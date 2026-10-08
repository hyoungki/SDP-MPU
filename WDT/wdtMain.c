/*
 * ==============================================================
 * System TARGET : ACE Control SDP-2000 Ver 1.0   
 * Target CPU    : MPC8248, VME6U
 * Main Factors  : 스마트급전용 SDP 자장치 
 *     - MPU 이중화 구성
 *     - ESOP/SIO 연계 : 전력감시, 원격진단, 전력품질, 고장점 등
 *     - 통합 시뮬레이터 : 2013/07/08 V8.2 적용 
 * --------------------------------------------------------------
 * System DESIGN : SANE-SYSTEM   .... by  Lee Ho-Sang
 * Initial-DATA  : 2016,03,25
 * Last Updated  : 2016,03,25
 * ==============================================================
 */


#include	"localLib.h"
//#include    "iccpShm.h"
#include    "smbus.h"
#include    "i2c-dev.h"

/* 2026-09-03 : updateHistoryQ()/init_fram() 에서 offsetof(HISTORY_QUE, queue)를
 * 쓰는데, 이 매크로는 <stddef.h>에 정의돼 있다. localLib.h 체인이 이걸 항상
 * 끌어들여준다고 보장할 수 없어서(실제로 이 파일에서는 안 끌려와서 빌드
 * 에러가 났음) 명시적으로 포함한다. */
#include    <stddef.h>

/* 2026-09-30 : RTC backup 유지시간이 짧은(약 5분) 보드 대응.
 * 주기적으로 현재 시각을 비휘발성 파일에 저장하고, 부팅 시 RTC 가 리셋된
 * 상태면 그 값으로 system time + RTC 를 복구한다.
 * 설계: design/20260930_time_backup.md */
#include    "time_backup.h"

//typedef unsigned char   bool;           // wdtParser.cpp 참조

int	            termExec;

SHM_DESC	    shmDesc = {-1, SHM_KEY, sizeof(SHM_MEMORY), NULL};

SHM_MEMORY	    *shmPtr = NULL;

TASK_INFO	    *taskPtr= NULL;

RTC             *rtc    = NULL;
OPR_MSG         *opr    = NULL;
CONSOLE_INFO	*console= NULL;

LINK_MSG        *linkCfg= NULL;                 // CPU 이중화 구조체
SCU_MSG         *scuCfg = NULL;                 // 이중화 절체장치(SCU) 구조체

ICCP_DCB        *iccpDCB = NULL;                // ICCP-HOST 구조체
ICCP_CONFIG     *iccpCFG = NULL;                // ICCP-HOST 구조체

ICCP_60870_DCB  *iccpInfo=NULL;                 // ICCP-HOST 참조용 : 모니터링 구조체

MPU_CONFIG      *mpuCFG  = NULL;                 // MPU Config
ESIO_CONFIG     *esioCFG[MAX_ESIO];             // ESIO 장치 Config

HOST_DCB        *hostDCB[MAX_HOST];             // HOST 관련 구조체 : ICCP, DNP, HARRIS, LANDIS...

RTU             *rtubuf[MAX_HARRIS_RTU];        /* HARRIS RTU Structure */
PORT_DB         *portdb[MAX_HARRIS_PORT];       /* HARRIS #1 PORT Structure */
	
POINT_BUF       *devPtBuf[MAX_DEV_POINT];       // SDP 디바이스 포인트 Config 정보
CAL_POINT_BUF   *calPtBuf[MAX_CAL_POINT];       // SDP 연산 포인트 Config 정보

SCAN_CONFIG     *scanCFG[MAX_SCAN_PORT];        // 하위계전기 SCAN Config
SDP_DEVICE      *deviceCFG[MAX_DEVICE];         // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)

//RTU_CONFIG      *rtuDCB = NULL;                 // 하부 RTU 운영 구조체
	 
RTU_DATABASE    *rtudb  = NULL;                 // SDP 데이터베이스
HISTORY_QUE     *hque   = NULL;                 // CONSOLE 용 이벤트

MPU_SOE_QUEUE   *mpuSOE = NULL;                 // MPU SOE Buffer
//MPU_COS_QUEUE   *mpuCOS = NULL;                 // MPU COS Buffer

HISTORY_QUE      localHQ;                       // MPU History-Q
volatile HISTORY_QUE      *framHque;

// hkkim for imx6sx 
#if 0

MPC860IO_DESC    ledPort1 = {0, NULL, 0, "/dev/i2c-1", O_RDWR|O_NDELAY};    // 상단부 LED : RUN/MS/ACT...
MPC860IO_DESC    ledPort2 = {0, NULL, 0, "/dev/i2c-1", O_RDWR|O_NDELAY};    // 하단부 LED : M1/S1.....

MPC860IO_DESC    devInput  = {0, NULL, 0, "/dev/i2c-0", O_RDWR|O_NDELAY};    // LINK 부 Output .....
MPC860IO_DESC    devOutput = {0, NULL, 0, "/dev/i2c-0", O_RDWR|O_NDELAY};    // LINK 부 Output .....

#else 

MPC860IO_DESC    ledPort1 = {0, NULL, 0, "/dev/i2c-3", O_RDWR|O_NDELAY};    // 상단부 LED : RUN/MS/ACT...
MPC860IO_DESC    ledPort2 = {0, NULL, 0, "/dev/i2c-3", O_RDWR|O_NDELAY};    // 하단부 LED : M1/S1.....

MPC860IO_DESC    devInput  = {0, NULL, 0, "/dev/i2c-1", O_RDWR|O_NDELAY};    // LINK 부 Output .....
MPC860IO_DESC    devOutput = {0, NULL, 0, "/dev/i2c-1", O_RDWR|O_NDELAY};    // LINK 부 Output .....

#endif 

#define P2_IO_INPUT_I2C_ADDR        0x1E
#define P2_IO_OUTPUT_I2C_ADDR       0x18
#define SWITCH_I2C_ADDR             0x1A

#define FRONT_LED_H__I2C_ADDR       0x1A
#define FRONT_LED_L_I2C_ADDR        0x1E

MPC860IO_DESC    rtcPort  = {0, NULL, 0, "/dev/rtc0", O_RDWR};

/* ---------------------------------------- */
/*  VMEBUS 관련 변수 초기화                 */
/* ---------------------------------------- */
void    *vmebus_ptr ;
int     vme_fd; 

VME_SIODCB      *vmeSioDCB[MAX_VME_SIO];
VME_SIODCB      tempVmeSioDCB[MAX_VME_SIO];

static  word    vmeRunTick[MAX_VME_SIO];        //모듈동작 이상
static  word    vmeFailTick[MAX_VME_SIO];       //모듈동작 이상
static  word    vmeFailType[MAX_VME_SIO];       //모듈장착 이상
static  word    vmeActCount[MAX_VME_SIO];

static  short   ledRunTick=0;
static  short   iccpRunTick = 0;

/* ---------------------------------------- */
/*  RTC 시각 백업 (time_backup) 관련        */
/* ---------------------------------------- */
/* 이 시각보다 오래된 RTC 값이면 "backup 방전으로 RTC 가 리셋된 것" 으로 간주한다.
 * time_backup.h 의 TIME_BACKUP_MIN_VALID_TIME(2023-11-15)은 갱신되지 않아
 * 시간이 갈수록 무의미해지므로, 그 값을 쓰지 않고 여기서 직접 관리한다.
 * ==> 릴리스마다 빌드 시점 근처로 올려줄 것.
 *     1756000000 = 2025-08-24 */
#define WDT_TIME_MIN_VALID      ((time_t)1756000000)

/* 마지막 백업 저장 이후 몇 번의 시간(hour) 변화가 지나면 다시 저장할지.
 * 정확한 12시간 계산이 아니라 hour 변화 횟수만 센다. */
#define WDT_TIME_BACKUP_HOURS   12

static  int     timeBackupHourCnt = 0;      // 마지막 저장 이후 hour 변화 횟수

extern  int	    readClock();
extern  int	    writeClock( int year, int month, int day, int hour, int min, int sec, int week);
extern  int     readRTUdb();
extern  int     sdp_DBChange_Check();

extern  void    mpuParaConfig();
extern  void    esioParaConfig();
extern  void    hostParaConfig(); 
extern  void    scanParaConfig();
extern  void    deviceParaConfig();
extern  void    pointParaConfig(); 
extern  void    calPointConfig();         // 연산포인트 Config...

extern  int     get_NTP_Info();

extern  void    read_RTC1340();
extern  int	    rtc_TimeUpdate();

/* -------------------------------------------- */
/*  External -Delcaration                       */
/* -------------------------------------------- */
extern  void    parserInit(void);
extern  void    sdp_Calculate_Point();

extern void lan_gpio_init(void) ;


static char cpuModeStr[2][12]  = {{"SINGLE-CPU"}, {"DUAL-CPU"}};
static char cpuStsStr[2][12]  = {{"MPU-A"}, {"MPU-B"}};
/*

PROCESS_DESC	prcTable[MAX_PROCESS] = 
            {
				{"SIM",    "",  0, 0},              // Simulator 통신 Process 
                {"SCU",    "",  0, 0},              // 이중화절체장치(SCU) 통신 Process
                {"LINK",   "",  0, 0},              // MPU 이중화 LINK Process 
                {"SCAN",   "",  0, 1},              // SCAN Process 
                {"ICCP",   "",  0, 2},              // ICCP(60870-6) Process 
                {"HOST",   "0", 0, 2},              // HOST#1  Process 
                {"HOST",   "1", 0, 2},              // HOST#2  Process 
                {"HOST",   "2", 0, 2},              // HOST#3  Process 
                {"HOST",   "3", 0, 2},              // HOST#4  Process 
    	    };
    	    
*/    	    




/* ---------------------------------------- */
/*  SDP-자장치 : PROCESS 관련 구조체 정의  ??????  */
/* ---------------------------------------- */
PROCESS_DESC	prcTable[MAX_PROCESS] = 
            {
				{"SIM",    "",  0, 0},              // Simulator 통신 Process 								
                {"SCU",    "",  0, 0},              // 이중화절체장치(SCU) 통신 Process
                {"LINK",   "",  0, 0},              // MPU 이중화 LINK Process 
                {"SCAN",   "",  0, 1},              // SCAN Process 
                {"ICCP",   "",  0, 2},              // ICCP(60870-6) Process 
                {"HOST",   "0", 0, 2},              // HOST#1  Process                
                {"HOST",   "1", 0, 2},              // HOST#2  Process 
                {"HOST",   "2", 0, 2},              // HOST#3  Process 
                {"HOST",   "3", 0, 2},              // HOST#4  Process 
  
    	    };

void    *mram_ptr;
int     mram_fd;                // SVME-860 Device Driver 

int     wdtid;                      

/* --------------------------------------------------------------------
 * 2026-09-03 : FRAM이 없는 신규 보드용 History.Bin 파일 백업
 *  (design/history-fram-to-file.md 참고)
 *
 *  기존에는 init_fram() 이 /dev/mem 을 mmap 해서 mram_ptr 이 FRAM
 *  물리주소를 직접 가리켰고, framHque 는 "mram_ptr + 0x1000" 위치를
 *  HISTORY_QUE 구조체로 캐스팅한 것이었다(즉 framHque 읽기/쓰기가
 *  그대로 FRAM 읽기/쓰기였음). 신규 보드는 FRAM이 없으므로:
 *   - histBuf   : FRAM을 대신하는 실제 저장 공간(그냥 RAM). framHque
 *                 가 최종적으로 이 변수의 주소를 가리키게 된다.
 *   - histFd    : /mnt/bin/History.Bin 의 파일 디스크립터. WDT
 *                 프로세스가 살아있는 동안 open 상태를 유지하며,
 *                 이벤트가 생길 때마다(updateHistoryQ) pwrite 로
 *                 바뀐 부분만 파일에 반영한다 - FRAM이 바이트 단위로
 *                 즉시 쓰였던 것과 동일한 효과를 파일에서도 낸다.
 *                 -1이면 "이번 부팅에는 파일을 못 열어서 못 쓴다"는
 *                 뜻이고, 이 경우 framHque 는 기존 폴백 그대로
 *                 &localHQ(휘발성 RAM)를 가리키게 된다.
 * -------------------------------------------------------------------- */
static HISTORY_QUE   histBuf;
static int           histFd = -1;

//
// 모듈:	SigHandler()
//
void SigHandler(int sig)
{
	char    buffer[256];
	
	
    //if(opr->wdtDebug)
           Debug(console,"wdt> %4d/%2d/%2d-%02d:%02d:%02d ... signal generated (%2d)...!\n", rtc->year, rtc->month,rtc->day,rtc->hour, rtc->min, rtc->sec, sig);
    pause(100);
    
	switch(sig)
	{
    case SIGTERM:
        termExec = 0;
        //if(opr->wdtDebug)
           Debug(console,"wdt> ... signal [SIGTERM] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "wdt> ... signal [SIGTERM] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        pause(1000);     
        break;
	        
	case SIGBUS : 
	    termExec = 0;
      //  if(opr->wdtDebug) 
              Debug(console,"wdt> ... signal [SIGBUS] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "wdt> ... signal [SIGBUS] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        pause(1000);     	    
	    break;
		    
	case SIGSEGV: 
	    termExec = 0;
       // if(opr->wdtDebug) 
              Debug(console,"wdt> ... signal [SIGSEGV] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "wdt> ... signal [SIGSEGV] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        pause(1000);     	     	    
	    break;
		    
	case SIGPIPE: 
      //  if(opr->wdtDebug) 
              Debug(console,"wdt> ... signal [SIGPIPE] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "wdt> ... signal [SIGPIPE] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        pause(1000);     	 	    
		break;

	default : 
	    //if(opr->wdtDebug)
	           Debug(console,"wdt> ... signal[%2d] generated ...!\n", sig);
	    /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "wdt> ... signal[%2d] generated ...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        pause(1000);   	
		break;
	}
	
}

int  write_led( int fd, unsigned char data)
{
    char buf[3];
    buf[0]=0x1;
    buf[1]= ~data ;
    int rc; 
    if ( (rc=write( fd, buf, 2) ) != 2)
    {
        perror("change mode\r\n");
        return 0;
          
    }

    return rc;
      
}


/*
* MPU 내부 이벤트용 Memory 초기화
*/
#if 0
/* --------------------------------------------------------------------
 * 2026-09-03 : 예전 FRAM(비휘발성 메모리) 기반 구현. 신규 보드는 FRAM이
 * 없어서 아래 History.Bin 파일 기반 구현으로 대체했다. 예전 방식이
 * 남아있는 보드용으로 참고할 수 있도록 지우지 않고 #if 0 으로 남겨둔다.
 * (design/history-fram-to-file.md 참고)
 * -------------------------------------------------------------------- */
int  init_fram_OLD_FRAM_VERSION(void)
{
    printf("[INFO] init_fram return just NOK\r\n");
    return NOK ;
    /* ------------------------------------ */
    /*  RED_HAT: MEMORY DEVICE 초기화       */
    /* ------------------------------------ */
    if ((mram_fd = open("/dev/mem", O_RDWR) ) < 0)
    {
        printf("[ERROR] wdt> *open /dev/fram ... Error ! \n");
        return (NOK);
    }        
    
    /* ------------------------------------ */ 
    /* FRAM Backup Memory 초기화 (512K)      */
    /* ------------------------------------ */
    if(( mram_ptr = ( char  *)mmap(0, MRAM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mram_fd, (MRAM_START_ADDRESS )) ) < 0)    
    {            
        printf("wdt> *FRAM Memory Mapping ... Error ! \n");
        return (NOK);            
    }

    printf("wdt> FRAM (at %8X, size %d Kbyte) Mapping OK\r\n",MRAM_START_ADDRESS, MRAM_SIZE/1024 ) ;

    return (OK);

}
#endif

/* --------------------------------------------------------------------
 * 2026-09-03 : FRAM 대신 /mnt/bin/History.Bin 파일로 History-Queue를
 * 영속화하는 신규 보드용 구현. 함수 이름과 반환값(OK/NOK)은 예전과
 * 그대로 유지해서 이 함수를 호출하는 init_variable() 쪽은 손댈 필요가
 * 없게 했다. (design/history-fram-to-file.md "채택안 상세 설계 2절" 참고)
 * -------------------------------------------------------------------- */
int  init_fram(void)
{
    struct stat st;

    /* ------------------------------------------------------------ */
    /* (1) History.Bin 열기 - 없으면 새로 만든다(O_CREAT).           */
    /*     0644 : 소유자 rw, 그룹/기타 r  (실행 권한 불필요)         */
    /* ------------------------------------------------------------ */
    histFd = open(HIST_FILE_PATH, O_RDWR | O_CREAT, 0644);
    if (histFd < 0)
    {
        printf("[ERROR] wdt> *open %s ... Error ! \n", HIST_FILE_PATH);
        return (NOK);
    }

    /* ------------------------------------------------------------ */
    /* (2) 파일 크기가 HISTORY_QUE 크기와 다르면(최초 생성 직후이거나 */
    /*     예전 버전/손상된 파일) 0으로 초기화해서 새로 쓴다.        */
    /*     크기가 맞으면 기존 내용을 그대로 histBuf 로 읽어들인다    */
    /*     - 이게 재부팅 후 이력이 살아남는 지점이다.                */
    /* ------------------------------------------------------------ */
    if (fstat(histFd, &st) < 0)
    {
        printf("[ERROR] wdt> *fstat %s ... Error ! \n", HIST_FILE_PATH);
        close(histFd);
        histFd = -1;
        return (NOK);
    }

    if (st.st_size != (off_t) sizeof(HISTORY_QUE))
    {
        /* 파일이 방금 새로 생성됐거나(크기 0), 예전 버전/손상된 파일이다 */
        printf("wdt> History.Bin size mismatch(file=%ld, need=%d) -> re-create\r\n",
               (long) st.st_size, (int) sizeof(HISTORY_QUE));

        bzero8248((byte *) &histBuf, sizeof(HISTORY_QUE));

        if (pwrite(histFd, &histBuf, sizeof(HISTORY_QUE), 0) != (ssize_t) sizeof(HISTORY_QUE))
        {
            printf("[ERROR] wdt> *History.Bin initial write fail ! \n");
            close(histFd);
            histFd = -1;
            return (NOK);
        }
        fsync(histFd);
    }
    else
    {
        if (pread(histFd, &histBuf, sizeof(HISTORY_QUE), 0) != (ssize_t) sizeof(HISTORY_QUE))
        {
            printf("[ERROR] wdt> *History.Bin read fail ! \n");
            close(histFd);
            histFd = -1;
            return (NOK);
        }
    }

    printf("wdt> History.Bin (%s, size %d Kbyte) Mapping OK\r\n",
           HIST_FILE_PATH, (int) (sizeof(HISTORY_QUE) / 1024));

    return (OK);

}

/*
*   VMEBUS : SIO 관련 메모리 초기화...
*/
int  init_vmebus(int sioid)
{
    /* -------------------------------------------- */
    /*  VME Backup Memory 초기화                    */
    /*  - SIO Module Size : 0x20000                 */
    /* -------------------------------------------- */
    if((vmebus_ptr = ( char  *) mmap(0, VMESIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vme_fd, (VMESIO_START_ADDRESS + (sioid*VMESIO_SIZE))) ) < 0)
    {            
        printf("wdt> *VME Memory Mapping ... Error ! \n");
        return (NOK);            
    }
    
    //printf("host> MAP VME_AREA : %p ... OK ! \n",vmebus_ptr );
    printf("wdt> VME (at %8X, size %d Kbyte) Mapping OK\r\n",VMESIO_START_ADDRESS, VMESIO_SIZE/1024 ) ;    
    return (OK);
}    

/*
*   VMEBUS & MRAM 초기화 
*/
int	init_variable()
{
    int     sioid;
    word    chksum;
    int     size1;
    int		diff;
    
    
    /* -------------------------------------------- */
    /*  VMEBUS MEMORY DEVICE 초기화                 */
    /* -------------------------------------------- */
    if ((vme_fd = open("/dev/mem", O_RDWR) ) < 0)
    {
        printf("host> *open VME_AREA : /dev/mem ... Error ! \n");
        return (NOK);
    }        
    
    /* -------------------------------------------- */
    /*  VME SIO# 구조체 포인트 초기화...            */
    /* -------------------------------------------- */
    for (sioid = 0; sioid < MAX_VME_SIO; sioid++)
    {
        /* -------------------------------- */
        /* 1. 외부 메모리 포인트 초기화 ... */
        /* -------------------------------- */
        if(init_vmebus(sioid) == OK)
        {
            /* SIO보드 초기화...내부 변수 초기화에 따른 변수 */             
    	    vmeSioDCB[sioid] = (VME_SIODCB *)(  vmebus_ptr);   
	        size1 = sizeof(VME_SIODCB);
    	    //printf("wdt> VMEBUS INIT-OK... VME SIO(%d)  Address : %p, size=%d\n", sioid, vmeSioDCB[sioid], size1);  

            vmeSioDCB[sioid]->sioAccess = 0;
            vmeSioDCB[sioid]->mpuAccess = 0;
            vmeSioDCB[sioid]->initReq   = 0;   
            vmeSioDCB[sioid]->initEnd   = 0;    
            vmeSioDCB[sioid]->resetFlag = 0;         
        }
        else
        {
            size1 = sizeof(VME_SIODCB);
	        printf("wdt> **** VMEBUS(size=%d) INIT-Fail... VME SIO(%d)\n", size1, sioid); 

            /* SIO보드 초기화...내부 변수 초기화에 따른 변수 */             
	        vmeSioDCB[sioid]  = (VME_SIODCB *) &tempVmeSioDCB;   

            vmeSioDCB[sioid]->sioAccess = 0;
            vmeSioDCB[sioid]->mpuAccess = 0;
            vmeSioDCB[sioid]->initReq   = 0;   
            vmeSioDCB[sioid]->initEnd   = 0;    
            vmeSioDCB[sioid]->resetFlag = 0;          	    
        }    
    }
    
    /* ------------------------------- */
    /* 1. 외부 메모리 포인트 초기화 ... */
    /* -------------------------------- */

    if(init_fram() == OK)
    {
  	    /* ---------------------------------------- */
        /* 운영 HISTORY ...변수 초기화              */ 
        /* ---------------------------------------- */  
        /* 2026-09-03 : FRAM 물리주소(mram_ptr+0x1000) 대신, init_fram()이
         * History.Bin 파일에서 이미 읽어들여 놓은 RAM 버퍼 histBuf 를
         * 가리키게 한다. (design/history-fram-to-file.md 참고) */
        framHque    = (HISTORY_QUE *) &histBuf;
  	    memcpy8248((byte *)hque, (byte *)framHque, sizeof(HISTORY_QUE));
  	    
  	    printf("\n=======================================================\n");
  	    printf("wdt> sizeof [SYSLOG_FORM] = %d byte, HQ Max = %d \n", sizeof(SYSLOG_FORM), HISTORY_QUE_MAX);
	    printf("wdt> History-FRAM initial...OK... HQueue Size = %6d / %04x \n", sizeof(HISTORY_QUE), sizeof(HISTORY_QUE));  	 
	    printf("=======================================================\n");
    }
    else
    {
        framHque    = (HISTORY_QUE *)&localHQ;
        bzero8248((byte *)framHque, sizeof(HISTORY_QUE)); 
	    printf("wdt> *FRAM History-Q initial...Error - HistoryQ \n"); 
    }

    /* ------------------------------------------------ */
    /*  2. History-Queue Check-Sum                      */
    /* ------------------------------------------------ */    
    chksum = gensum((byte *) framHque, sizeof(HISTORY_QUE) - 2);
         
    /* database check */
    if(framHque->chksum != chksum)
    {
        printf("wdt> *HISTORY-Q FRAM Check-sum Fail [rom = %04x, cal = %04x] ->Cleared... \n", framHque->chksum, chksum);
        bzero8248((byte *)framHque, sizeof(HISTORY_QUE));
        
        chksum = gensum((byte *) framHque, sizeof(HISTORY_QUE) - 2);
        framHque->chksum = chksum;
    
        bzero8248((byte *)hque, sizeof(HISTORY_QUE));
        chksum = gensum((byte *) hque, sizeof(HISTORY_QUE) - 2);

        hque->chksum = chksum;
        printf("wdt> *HISTORY-Q FRAM Check-sum Write OK ... chksum = %04x  \n", chksum);
    }

#if 0
typedef struct
{
	TASK_INFO	    taskInfo[MAX_PROCESS + 2];
	
	RTC             rtc; 
	OPR_MSG         opr_msg;                            
	
	CONSOLE_INFO	console;

    /* -------------------------------- */
	/* ICCP 운영 버퍼 Queue             */
	/* -------------------------------- */
    ICCP_DCB        iccpDCB;                            // ICCP-HOST 운영 버퍼
    ICCP_60870_DCB  iccpInfo;                           // ICCP-HOST 참조용 : 모니터링 구조체
    
    MPU_CONFIG      mpuConfig;                          // MPU Config
    ESIO_CONFIG     esioConfig[MAX_ESIO];               // ESIO 장치 Config
    
    LINK_MSG        link_msg;                           // CPU 이중화 구조체
    SCU_MSG         scu_msg;                            // 이중화 절체장치 구조체 

    /* -------------------------------- */
	/* HOST 운영 버퍼 Queue             */
	/* -------------------------------- */
    HOST_DCB        hostDCB[MAX_HOST];                  // HOST 관련 구조체 : DNP, HARRIS, LANDIS...
    
    /* HARRIS-HOST 데이터 구조 */
    RTU             rtubuf[MAX_HARRIS_RTU];         /* HARRIS RTU Structure */
	PORT_DB         portdb[MAX_HARRIS_PORT];        /* HARRIS #1 PORT Structure */
		
    /* -------------------------------- */
	/* 계전기 SCAN 버퍼 Queue           */
	/* -------------------------------- */
	SCAN_CONFIG     scanCFG[MAX_SCAN_PORT];             // 하위계전기 SCAN Config
	SDP_DEVICE      deviceCFG[MAX_DEVICE];              // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)

	POINT_BUF       devPtBuf[MAX_DEV_POINT];            // 장치 포인트용 구조체
	CAL_POINT_BUF   calPtBuf[MAX_CAL_POINT];            // 연산 포인트용 구조체
    RTU_DATABASE    rtuDatabase;                        // SDP 데이터베이스
    
    /* -------------------------------- */
	/* 시스템 운영 이벤트 Queue         */
	/* -------------------------------- */
	MPU_SOE_QUEUE   mpuSoeQueue;
	//MPU_COS_QUEUE   mpuCosQueue;
    HISTORY_QUE     localHistoryQ;                      // CONSOLE 용 LOCAL Event (SOE...)

} __attribute__ ((packed)) SHM_MEMORY;
#endif

	printf("\n----------------------------------------------\n");
    printf("size of TASK_INFO       = %d\n", sizeof(TASK_INFO) *  (MAX_PROCESS+2));
    printf("size of RTC             = %d\n", sizeof(RTC));
    printf("size of OPR_MSG         = %d\n", sizeof(OPR_MSG));
    printf("size of CONSOLE_INFO    = %d\n", sizeof(CONSOLE_INFO));
    printf("size of ICCP_DCB    	= %d\n", sizeof(ICCP_DCB));
    printf("size of ICCP_60870_DCB	= %d\n", sizeof(ICCP_60870_DCB));
    printf("size of MPU_CONFIG      = %d\n", sizeof(MPU_CONFIG));
    printf("size of ESIO_CONFIG     = %d\n", sizeof(ESIO_CONFIG) * MAX_ESIO);
    printf("size of LINK_MSG     	= %d\n", sizeof(LINK_MSG));
    printf("size of SCU_MSG     	= %d\n", sizeof(SCU_MSG));
    printf("size of HOST_DCB     	= %d\n", sizeof(HOST_DCB)*MAX_HOST);
    printf("size of RTU-DB     		= %d\n", sizeof(RTU)*MAX_HARRIS_RTU);
    printf("size of PORT-DB     	= %d\n", sizeof(PORT_DB)*MAX_HARRIS_PORT);
    printf("size of SCAN_CONFIG     = %d\n", sizeof(SCAN_CONFIG) * MAX_SCAN_PORT);
    printf("size of SDP_DEVICE      = %d\n", sizeof(SDP_DEVICE) * MAX_DEVICE);
    printf("size of DEV_POINT_BUF   = %d\n", sizeof(POINT_BUF) * MAX_DEV_POINT);
    printf("size of CAL_POINT_BUF   = %d\n", sizeof(CAL_POINT_BUF) * MAX_CAL_POINT);
    printf("size of RTU_DATABASE   	= %d\n", sizeof(RTU_DATABASE));
    printf("size of MPU_SOE_QUEUE	= %d\n", sizeof(MPU_SOE_QUEUE));
    printf("size of HISTORY_QUE     = %d\n", sizeof(HISTORY_QUE));

    printf("\n----------------------------------------------\n");
    diff = 13719188 - sizeof(SHM_MEMORY);
    printf("wdt> Shared Memory.... size = %d byte (13,719,188 byte) => diff = %d\n", sizeof(SHM_MEMORY), diff);
    printf("----------------------------------------------\n");
        
            
    hque->front = framHque->front;
    hque->chksum= framHque->chksum;
    
    
    /* SIMULATOR 이벤트 초기화 */
    opr->simFront = hque->front;
    opr->simRear  = hque->front;
    opr->simOverlab = RESET;
    
    mpuCFG->mpuStatus = 0;              

    /* ---------------------------------------- */
    /*  연산포인트용 Local 초기화 호출...       */
    /* ---------------------------------------- */
    parserInit();        
    
    return (0);
        
}


//
// 모듈: SetProcessInfo()
//
void SetProcessInfo(void)
{
	int		idx;
	TASK_INFO	    *taskPtr;
	PROCESS_DESC	*dscPtr;

	// Clear Process Information
	memset((char *)shmPtr->taskInfo, 0, sizeof(TASK_INFO)*MAX_PROCESS);

	// Initialize Process Information
	taskPtr = shmPtr->taskInfo;
	
	dscPtr = prcTable;
	for (idx = 0; idx < MAX_PROCESS; idx++, taskPtr++, dscPtr++)
	{
		if (!strlen(dscPtr->name))  continue;

		sprintf(taskPtr->name, "%s", dscPtr->name);
		taskPtr->define = 1;
		taskPtr->check = dscPtr->check;
		taskPtr->wdtCount = 0;
	}
	
}

//
// 모듈: TerminateProcess()
//
void TerminateProcess(void)
{
	int		idx;
	TASK_INFO	*taskPtr;

	if (shmPtr == NULL)
		return;
    
	// Terminate Process
	taskPtr = shmPtr->taskInfo;
	for (idx = 0; idx < MAX_PROCESS; idx++, taskPtr++)
	{
		//if (!prcPtr->st.defined || !prcPtr->st.active || prcPtr->pid <= 1)
		//	continue;
		KillProcess(taskPtr->pid, taskPtr->check);
	}
}

//
// 모듈: CheckProcess()
//
int start_process_first ; 

void CheckProcess(void)
{
	int		idx;
	int		retVal;
	char	buffer[256];
	
	TASK_INFO	*taskPtr;
	PROCESS_DESC	*dscPtr;

	taskPtr = (TASK_INFO *) shmPtr->taskInfo;
	dscPtr = prcTable;
	
	for (idx = 0; idx < MAX_PROCESS; idx++, taskPtr++, dscPtr++)
	{
		if (!taskPtr->define)   continue;

        /* -------------------------------- */
        /* PROCESS 초기 기동시 ...          */
        /* -------------------------------- */
		if (!taskPtr->active || IsProcessActive(taskPtr->pid) < 0)
	    {    // 처음 이거나 종료된 경우 다시 살린다.
 			if (dscPtr->waitCount > 0)  dscPtr->waitCount--;
			else
			{   // dscPtr->waitCount ==0 이면..여기네..
			    //if(opr->wdtDebug)   
			    //Debug(console,"wdt>> [*] Process Start ... %s \n", dscPtr->name);
			        
				//StartProcess(dscPtr->name, dscPtr->argument);
				//dscPtr->waitCount = WDT_WAIT_COUNT;
				
				retVal = StartProcess(dscPtr->name, dscPtr->argument);
				if ( ! start_process_first ) //아..process마다 필요하군..
				{    
				    printf("wdt> *** TASK start : [ %s ] Return sts... %d\n", taskPtr->name, retVal);				
				   
				}else 
				    printf("wdt> *** TASK Restart : [ %s ] Return sts... %d\n", taskPtr->name, retVal);
				
				/* --------------------------------------------- */
				/*	2020.05.15  TASK 실행이상시...					*/
				/* --------------------------------------------- */
				if(retVal == -1)
				{
					printf("\n----------------------------------------\n");
					printf("[ERROR] wdt> *** [%s] TASK Restart... FAIL : REBOOT.. \n", taskPtr->name);
					printf("\n----------------------------------------\n");
					
					/* LOG File 저장 */
    				sprintf(buffer, "%s", "=====================================================");
    				LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    
					sprintf(buffer, ">> wdt: *** [%s] TASK-Restart Fail...MPU-REBOOT ! ", taskPtr->name);
					LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
					  
					sprintf(buffer, "%s", "=====================================================");
    				LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
			
					opr->wdtEnable = 0;
					return;
				}

                // ?  이렇게 되면 종료 확인후 20 초인데....					
				dscPtr->waitCount = WDT_WAIT_COUNT;
				dscPtr->waitCount = 20;
				
			}
		}
		else //  이미 잘 돌고 있다..
		{
			dscPtr->waitCount = 0; // ? 이게 무슨 동작이지...이렇게 되면 바로 시작인데...
			if (taskPtr->wdtEnable) // 각 proces가 돌면 0 으로 된다.
			{
				taskPtr->wdtCount++;
				//Debug(console,"WDT> Task-%s, count=%d\n", taskPtr->name, taskPtr->wdtCount);
				if (taskPtr->wdtCount > 10)  // 10초 동안 대기 하면 계속 이상ㅎ안지 확인
				{
				    Debug(console,"\n===================================\n");
                    Debug(console,"wdt> [*] Process Re-Start ... [%s] \n", taskPtr->name);
                    Debug(console,"===================================\n");
					KillProcess(taskPtr->pid, taskPtr->check);
					pause(100);
					
					//StartProcess(dscPtr->name, dscPtr->argument);
					//termExec = 0;
				}
			}
			else	
			    taskPtr->wdtCount = 0;	// Reset WDT Check Counter
		}
	}
	
	start_process_first=1;
	
}

//
// 모듈:	DisplayLogo()
//
void DisplayLogo(void)
{
	//ExecCommand("clear");
	pause(1000);
	printf("\n\n");
	printf("////////////////////////////////////////////////////////\n");  pause(100);
	printf("//        %s CPU Module            //\n", TARGET_NAME); pause(100);
	printf("//        [*] Designed by ACE Control System           //\n");  pause(100);
	printf("//        [*] Version   %s                   //\n", VERSION_STRING);pause(100);
	printf("//        [*] %s     //\n", SIMULATOR_NAME);  pause(100);
	printf("//        [*] Update Date ...%s              //\n", DATE_STRING);pause(100);
	printf("////////////////////////////////////////////////////////\n\n");  pause(100);
	
	pause(1000);
	    
 }

//
// 모듈:	ClearEnvironment()
//
void ClearEnv(void)
{
    // 수행중인 프로세스들을 종료시킨다
	TerminateProcess();
	
	opr->stscode = 0;
	opr->stscode1= 0;
	
	/* -------------------------------- */
    /* 상단부 전면 LED 상태표출 ...     */
    /* -------------------------------- */
    write_led(ledPort1.id, opr->stscode);
    write_led(ledPort2.id, opr->stscode1);
    
	pause(100);
	
    close(ledPort1.id);
    close(ledPort2.id);
    close(devOutput.id);
    close(rtcPort.id);
    
	close(mram_fd);

	/* 2026-09-03 : WDT 종료 시 History.Bin fd도 정리한다.
	 * (마지막 내용은 updateHistoryQ/clearHistoryQ 에서 이미 fsync
	 * 까지 마친 상태이므로 여기서는 fd만 닫으면 된다.) */
	if (histFd >= 0)
	{
		close(histFd);
		histFd = -1;
	}

	iccpShmEnd ();
	    
	// 공유 메모리를 제거한다
	ShmDelete(&shmDesc);
	
	printf("wdt> *WDT PROCESS.... EXIT ...!\n");

}

/*  polarity invertion */
int  set_polarity(int fd,  unsigned char data)
{
    
    char buf[3];
    int rc;
    
    /* chage to output  */
    buf[0]= 0x02 ;// register address 
    buf[1]= data ; // all to out 
    
    if ( (rc=write( fd, buf, 2) ) != 2)
    {
        perror("change ploartiy step1\r\n");
        return -1;
          
    }
    
    /* verify */
    buf[0]=0x02 ;
    
    /* set register */
    if ( (rc=write( fd, buf, 1) ) != 1)
    {
        perror("change ploartiy step2\r\n");
        return -1;
          
    }     
    /* read */
     if ( (rc=read( fd, buf, 1) ) != 1)
    {
        perror("change ploartiy step3\r\n");
        return -1;
          
    }     
       
    if ( ( unsigned char)buf[0] != data)   
     {
        printf("change ploartiy step4 R[%x]-W[%x]\r\n", buf[0], data);
        return -1;
          
    }       
    return 0 ;  
}

int  chage_mode( int fd)
{
    char buf[3];
    int rc;
    


    /* chage to output  */
    buf[0]= 0x03 ;// register address 
    buf[1]= 0x00 ; // all to out 
    
    if ( (rc=write( fd, buf, 2) ) != 2)
    {
        perror("change ploartiy\r\n");
        return 0;
          
    }
    

    /* chage polarity */
    buf[0]= 0x02 ;// register address 
    buf[1]= 0x00 ; // all to out 
    
    if ( (rc=write( fd, buf, 2) ) != 2)
    {
        perror("change ploartiy\r\n");
        return 0;
          
    }

    return 0;
}
					    

//
// 모듈:	InitEnvironment()
//
int InitEnv( void)
{
    int i;
    
	// SIGNAL 처리 루틴 설정
	signal( SIGBUS,  SigHandler);
	signal( SIGSEGV, SigHandler);
	signal( SIGTERM, SigHandler);

	/* -------------------------------------------- */
	/* 디바이스 LED#1 초기화 - IO Device Open ...   */
	/* -------------------------------------------- */
    if((ledPort1.id = open( ledPort1.name, ledPort1.attr)) <0)  
	{
        printf("wdt> *Device Open Error ... LED#1(%s) \n", ledPort1.name);
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    if (ioctl(ledPort1.id,  I2C_SLAVE         ,FRONT_LED_H__I2C_ADDR) < 0) 
    {
        printf("Failed to acquire bus access and/or talk to slave 0.\n");
        exit(1);
    }

    /* OUTPUT 속성 변경 */
    chage_mode( ledPort1.id);
    
    /* -------------------------------------------- */
	/* 디바이스 LED#2 초기화 - IO Device Open ...   */
	/* -------------------------------------------- */
    if((ledPort2.id = open( ledPort2.name, ledPort2.attr)) <0)  
	{
        printf("wdt> *Device Open Error ... LED#2(%s) \n", ledPort2.name);
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    if (ioctl(ledPort2.id,  I2C_SLAVE         ,FRONT_LED_L_I2C_ADDR) < 0) 
    {
        printf("Failed to acquire bus access and/or talk to slave 0.\n");
        //exit(1);
        termExec = 0;
        return (0);
    }

    /* OUTPUT 속성 변경 */
    chage_mode( ledPort2.id);

    /* ---------------------------------------------------- */
    /*  MPU P2-IO-INPUT  :                                  */
    /* ---------------------------------------------------- */
    if((devInput.id = open( devInput.name, devInput.attr)) <0)  
	{
        printf("wdt> *Device Open Error ... P2-INPUT (%s) \n", devInput.name);
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    if (ioctl(devInput.id,  I2C_SLAVE , P2_IO_INPUT_I2C_ADDR) < 0) 
    {
        printf("wdt> *Failed to acquire bus access and/or talk to slave 0... P2-INPUT \n");
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    if( set_polarity(devInput.id, 0x00) <0 )
    {
        printf("wdt> *set_polarity fail ... P2-INPUT  \n");
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    
    /* ---------------------------------------------------- */
    /*  MPU P2-IO-OUTPUT  :  CPU 동작상태 Clear...          */
    /* ---------------------------------------------------- */
    if((devOutput.id = open( devOutput.name, devOutput.attr)) <0)  
	{
        printf("link> *Device Open Error ... P2-OUTPUT (%s) \n", devOutput.name);
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    if (ioctl(devOutput.id,  I2C_SLAVE , P2_IO_OUTPUT_I2C_ADDR) < 0) 
    {
        printf("link> *Failed to acquire bus access and/or talk to slave 0... P2-OUTPUT \n");
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    /* OUTPUT 속성 변경 */
    chage_mode( devOutput.id);
    
    /* -------------------------------------------- */
	/* 디바이스 RTC 초기화 - IO Device Open ...     */
	/* -------------------------------------------- */
   	if((rtcPort.id = open( rtcPort.name, rtcPort.attr)) < 0)
    {
        printf("wdt> *Device Open Error ... RTC(%s) \n", rtcPort.name);
        //exit(1);
        termExec = 0;
        return (0);
    }

    printf("size of SHM-Memory ... %d\n", sizeof(SHM_MEMORY));
    
    /* ------------------------------------ */
	/*  공유 메모리를 생성한다              */
	/* ------------------------------------ */
	if (ShmCreate(&shmDesc) < 0)
	{
		printf("wdt> *공유 메모리 Open 이상 \n");
		return(0);
	}

    /* ------------------------------------ */
	/*  공유 메모리 상태 확인               */
	/* ------------------------------------ */
	if (ShmCheck(&shmDesc) < 0)
	{
		printf("wdt> *ERR_공유 메모리 상태 이상\n");
		return(0);
	}
	
	/* ------------------------------------ */
	/* 공유 메모리상의 포인터 변수 초기화   */      
	/* ------------------------------------ */
	shmPtr = (SHM_MEMORY *) shmDesc.address;
	taskPtr = (TASK_INFO *) &shmPtr->taskInfo[WDT_PROCESS];
	
	rtc     = (RTC *)       &shmPtr->rtc;
	opr     = (OPR_MSG *)   &shmPtr->opr_msg;
    
    linkCfg = (LINK_MSG *)  &shmPtr->link_msg;                  // MPU 이중화 관련 구조체
    scuCfg 	= (SCU_MSG *)   &shmPtr->scu_msg;                   // 이중화절체장치(SCU) 구조체
    
	rtudb   = (RTU_DATABASE *) &shmPtr->rtuDatabase;
    console = (CONSOLE_INFO *) &shmPtr->console; 
    
    mpuSOE  = (MPU_SOE_QUEUE *) &shmPtr->mpuSoeQueue;           // MPU SOE Buffer
    //mpuCOS  = (MPU_COS_QUEUE *) &shmPtr->mpuCosQueue;           // MPU COS Buffer
    
	hque    = (HISTORY_QUE *)  &shmPtr->localHistoryQ; 

    mpuCFG  = (MPU_CONFIG *) &shmPtr->mpuConfig;                // MPU Network 구성정보
    
    iccpDCB = (ICCP_DCB *) &shmPtr->iccpDCB;                   // ICCP_DCB 
    iccpCFG = (ICCP_CONFIG *) &shmPtr->iccpDCB.config;         // MPU Network 구성정보
    
    /* ICCP-INFO 구조체 정의 */
    iccpInfo= (ICCP_60870_DCB *) &shmPtr->iccpInfo;            // ICCP-HOST 참조용 : 모니터링 구조체
    
    //rtuDCB  = (RTU_CONFIG *) &shmPtr->rtuConfig;               // 하부 RTU 운영구조체
    
    /* ------------------------------------ */
    /*  ICCP 공유메모리 초기화              */
    /* ------------------------------------ */
    iccpShmInit (iccpDCB);
    
    /* ------------------------------------ */
    /* ESIO CFG : ESIO 구조체 (MMAX 5)      */
    /* ------------------------------------ */ 
    for(i=0; i< MAX_ESIO; i++)   
	{
	    esioCFG[i] = (ESIO_CONFIG *) &shmPtr->esioConfig[i];
	    bzero((byte *) esioCFG[i], sizeof(ESIO_CONFIG));
	    
	    esioCFG[i]->online = 2;			// ESIO 구조체 초기화 
	    esioCFG[i]->chksumReq   = SET;
    	esioCFG[i]->timeSyncReq = SET;
	}
	
	/* ------------------------------------ */
    /* HOST_DCB : 상위 HOST 구조체 (MMAX 8) */
    /* ------------------------------------ */
    for(i=0; i< MAX_HOST; i++)   
	{
	    hostDCB[i] = (HOST_DCB *) &shmPtr->hostDCB[i];
	    bzero((byte *) hostDCB[i], sizeof(HOST_DCB));
	}
	
	/* ------------------------------------ */
    /* HARRIS_DCB : 상위 HOST 구조체 (MMAX 8) */
    /* ------------------------------------ */
    for(i=0; i< MAX_HARRIS_RTU; i++)   
	{
	    rtubuf[i] = (RTU *) &shmPtr->rtubuf[i];
	    bzero((byte *) rtubuf[i], sizeof(RTU));
	}
	
	/* ------------------------------------ */
    /* HARRIS_DCB : 상위 HOST 구조체 (MMAX 8) */
    /* ------------------------------------ */
    for(i=0; i< MAX_HARRIS_PORT; i++)   
	{
	    portdb[i] = (PORT_DB *) &shmPtr->portdb[i];
	    bzero((byte *) portdb[i], sizeof(PORT_DB));
	}
	

    /* ------------------------------------ */
    /* SCAN CONFIG : SCAN 구조체(MAX 16)    */
    /* ------------------------------------ */
    for(i=0; i< MAX_SCAN_PORT; i++)   
	{
	    scanCFG[i] = (SCAN_CONFIG *) &shmPtr->scanCFG[i];
	    bzero((byte *) scanCFG[i], sizeof(SCAN_CONFIG));
	}
	
	/* ------------------------------------ */
    /* 계전기 구조체 : 전체 계전기(MAX 64)  */
    /* ------------------------------------ */    
    for(i=0; i< MAX_DEVICE; i++)   
	{
	    deviceCFG[i] = (SDP_DEVICE *) &shmPtr->deviceCFG[i];
	    bzero((byte *) deviceCFG[i], sizeof(SDP_DEVICE));
	}

    /* ------------------------------------ */
    /* 디바이스 포인트 : 100                */
    /* ------------------------------------ */   
	for(i=0; i< MAX_DEV_POINT; i++)   
	{
	    devPtBuf[i] = (POINT_BUF *) &shmPtr->devPtBuf[i];
	    bzero((byte *) devPtBuf[i], sizeof(POINT_BUF));
	}
	
	/* ------------------------------------ */
    /* 연산 포인트 구조체 : 32              */
    /* ------------------------------------ */   
	for(i=0; i< MAX_CAL_POINT; i++)   
	{
	    calPtBuf[i] = (CAL_POINT_BUF *) &shmPtr->calPtBuf[i];
	    bzero((byte *) calPtBuf[i], sizeof(CAL_POINT_BUF));
	}		
	
	/* ------------------------------------ */
    /* 운영 Buffer Clear....                */
    /* ------------------------------------ */
    bzero( opr,     sizeof(OPR_MSG));
    bzero( rtc,     sizeof(RTC));
    bzero( linkCfg, sizeof(LINK_MSG));
    bzero( rtudb,   sizeof(RTU_DATABASE));
    bzero( console, sizeof(CONSOLE_INFO));
    bzero( hque,    sizeof(HISTORY_QUE));
    bzero( mpuCFG,  sizeof(MPU_CONFIG));
    
    bzero( mpuSOE,  sizeof(MPU_SOE_QUEUE));
    //bzero( mpuCOS,  sizeof(MPU_COS_QUEUE));
    
    
    opr->lineDebug  = 0xff;
    opr->hostDebug  = 0xff;
    opr->hexDebug   = 0xff; 
    opr->esioDebug  = 0xff;
    opr->calDebug   = 0xff;

    opr->dnpDebug   = 0xff;
    opr->msgDebug   = 0xff; 
    opr->mpuDebug   = 0;
        
	// PROCESS 관련 정보를 초기화한다
	SetProcessInfo();
	
	/* ------------------------------------------------ */
	/*  PROCESS 호출시... Priority 지정 (-19 ~ 20)      */
	/* ------------------------------------------------ */
	nice(NICE_WDT);

	// 프로세스 정보를 초기화한다
	taskPtr->initial = 1;
	UpdateProcessInfo(taskPtr, 1, getpid(), 0);

	return(1);
}


/*
*   CONSOLE 제어기 운영 Event....FRAM 영역으로 Copy
*/
int updateHistoryQ()
{
    word    chksum;
    word    rear, front;
    int		updateCount;
    volatile SYSLOG_FORM   *src, *des;
    SYSLOG_FORM  temp;
    off_t   recOffset;              /* 2026-09-03 : 이번에 바뀐 큐 슬롯의 History.Bin 상 위치 */

    opr->nramAccess = SET;
    
    rear  = framHque->front & HISTORY_QUE_MASK;
    front = hque->front & HISTORY_QUE_MASK;
	updateCount = 0;
	
	if(opr->wdtDebug)
	Debug(console,">> HQ update... front = %d / rear = %d \n", front, rear);
	
    while(front != rear)
    {        
    	/* ---------------------------------------- */
       	/*  제어기 WDT & System-REBOOT Check        */
	    /* ---------------------------------------- */
        if(opr->wdtEnable == SET)
	    {
            if(ioctl(wdtid,PMU_WDT_CTR_CLR,NULL) < 0 )  Debug(console, "wdt> *** WDT ioctl fail\n");
        }      
        
        if(opr->wdtDebug)   
        Debug(console,"wdt> Update HISTORY-EVENT ... fram=%d, Que=%d\n", rear, front);
            
        des = (SYSLOG_FORM *) &framHque->queue[rear];
        src = (SYSLOG_FORM *) &hque->queue[rear];

        /* CPU 내부 Backup Memory Write... */     
        memcpy8248((byte *) &temp, (byte *)src, sizeof(SYSLOG_FORM));
        memcpy8248((byte *) des, (byte *) &temp, sizeof(SYSLOG_FORM));

        /* --------------------------------------------------------- */
        /* 2026-09-03 : 예전에는 위 memcpy8248 한 줄로 끝이었다(des가  */
        /* FRAM 물리주소를 직접 가리켰으므로 memcpy 자체가 FRAM 쓰기  */
        /* 였음). 지금은 des가 RAM(histBuf) 이라서, 방금 바뀐 이      */
        /* 슬롯 하나만 골라 History.Bin 의 같은 offset 에 pwrite로    */
        /* 즉시 반영해줘야 파일이 FRAM처럼 "즉시 영속"된다. 44KB      */
        /* 전체를 매번 다시 쓰지 않고 딱 이 슬롯(sizeof(SYSLOG_FORM)  */
        /* 바이트)만 쓰므로 이벤트가 몰려도 flash 부담이 크지 않다.   */
        /* --------------------------------------------------------- */
        if (histFd >= 0)
        {
            recOffset = (off_t) offsetof(HISTORY_QUE, queue) + (off_t) rear * (off_t) sizeof(SYSLOG_FORM);
            /* pwrite()는 _FORTIFY_SOURCE 빌드에서 반환값을 반드시 확인하도록
             * 강제된다(warn_unused_result). 실패해도 WDT를 멈출 정도는
             * 아니므로(다음 이벤트에서 다시 시도됨) Debug 로그만 남긴다. */
            if (pwrite(histFd, (void *) des, sizeof(SYSLOG_FORM), recOffset) < 0)
            {
                Debug(console, "wdt> *** History.Bin record write fail (rear=%d)\n", rear);
            }
        }
     
        rear = (rear + 1) & HISTORY_QUE_MASK;
        
        /* 무한 Update ... 방지 */
        if(++updateCount > 16)	break;
  	}
  	      
    framHque->front     = rear;
    framHque->overlab   = hque->overlab;
        
    chksum = gensum((byte *) framHque, sizeof(HISTORY_QUE) - 2);
    framHque->chksum = chksum;

    /* --------------------------------------------------------------- */
    /* 2026-09-03 : 위에서 갱신한 헤더(front/overlab)와 맨 끝 체크섬도  */
    /* 파일에 반영한다. front/overlab/clear 는 구조체 맨 앞(offset 0)  */
    /* 이라 한 번에 쓰고, chksum 은 구조체 맨 끝에 있어 따로 쓴다.     */
    /* fsync 까지 호출하는 이유는, WDT가 재부팅 직전 남기는            */
    /* ENT_CU_RESTART 같은 이벤트가 바로 이 경로를 타기 때문에 -       */
    /* "전원이 실제로 끊기기 직전 이벤트가 디스크에 남는가"가 이       */
    /* 기능의 존재 이유라 durability를 우선했다. 이벤트가 매우         */
    /* 빈번한 현장이라면 이 fsync 호출 빈도를 낮추는 걸 고려할 수      */
    /* 있다(design/history-fram-to-file.md 참고).                     */
    /* --------------------------------------------------------------- */
    if (histFd >= 0)
    {
        if (pwrite(histFd, (void *) framHque, offsetof(HISTORY_QUE, queue), 0) < 0)
        {
            Debug(console, "wdt> *** History.Bin header write fail\n");
        }
        if (pwrite(histFd, (void *) &framHque->chksum, sizeof(word), sizeof(HISTORY_QUE) - sizeof(word)) < 0)
        {
            Debug(console, "wdt> *** History.Bin chksum write fail\n");
        }
        fsync(histFd);
    }

    opr->nramAccess = RESET;

    return (rear);
    
}

/*
*   CONSOLE 제어기 운영 Event....CLEAR
*/
void clearHistoryQ()
{
    word    chksum;

    Debug(console,"wdt> *HISTORY FRAM Event Cleared.... !\n");

    opr->nramAccess = SET;
    
    /* ---------------------------------------- */
   	/*  제어기 WDT & System-REBOOT Check        */
   	/* ---------------------------------------- */
    if(opr->wdtEnable == SET)
   	{
        if(ioctl(wdtid,PMU_WDT_CTR_CLR,NULL) < 0 )  Debug(console, "wdt> *** WDT ioctl fail\n");
    }      
        
    bzero((byte *) framHque, sizeof(HISTORY_QUE));
        
    chksum = gensum((byte *) framHque, sizeof(HISTORY_QUE) - 2);
    framHque->chksum = chksum;

    /* 2026-09-03 : "이력 전체 지우기"는 관리자가 가끔 하는 동작이라 */
    /* 44KB 전체를 통째로 다시 써도 부담이 없다. updateHistoryQ()   */
    /* 처럼 슬롯 단위로 쪼갤 필요 없이 한 번에 pwrite + fsync.       */
    if (histFd >= 0)
    {
        if (pwrite(histFd, (void *) framHque, sizeof(HISTORY_QUE), 0) < 0)
        {
            Debug(console, "wdt> *** History.Bin clear write fail\n");
        }
        fsync(histFd);
    }

    opr->nramAccess = RESET;

    hque->front   = framHque->front & HISTORY_QUE_MASK;
    hque->overlab = framHque->overlab;
    hque->clear   = 0;
}




/*
*   VMECU 전면판 상태 LED
*/
void mpu_LED_Update()
{
    
    /* -------------------------------- */
    /* CPU 운영모드 동작상태 확인...    */
    /* -------------------------------- */
    if(opr->runMode == LOCAL_SLAVE)   
    {
        if(ledRunTick++ > 5)
        {
            ledRunTick = 0;
            opr->stscode ^= LED_RUN_BIT;    /* RUN led */
            
            /* -------------------------------- */
            /*  ESIO 모듈 통신상태 표시         */
            /* -------------------------------- */  
            if(esioCFG[ESIO_SCADA]->online == SET)  opr->stscode1 ^= LED_ESIO_SCADA;    
            else                                    opr->stscode1 &= (~LED_ESIO_SCADA);
    
            if(esioCFG[ESIO_REMOTE]->online == SET) opr->stscode1 ^= LED_ESIO_REMOTE;    
            else                                    opr->stscode1 &= (~LED_ESIO_REMOTE); 
    
            if(esioCFG[ESIO_ELECQ]->online == SET)  opr->stscode1 ^= LED_ESIO_ELECQ;    
            else                                    opr->stscode1 &= (~LED_ESIO_ELECQ);              
        
            if(esioCFG[ESIO_61850]->online == SET)  opr->stscode1 ^= LED_ESIO_61850;    
            else                                    opr->stscode1 &= (~LED_ESIO_61850);     
    
            if(esioCFG[ESIO_RTU]->online == SET)    opr->stscode1 ^= LED_ESIO_RTU;    
            else                                    opr->stscode1 &= (~LED_ESIO_RTU); 
        
        }
    }
    else
    {       
        opr->stscode ^= LED_RUN_BIT;    /* RUN led */
        
        /* -------------------------------- */
        /*  ESIO 모듈 통신상태 표시         */
        /* -------------------------------- */  
        if(esioCFG[ESIO_SCADA]->online == SET)  opr->stscode1 ^= LED_ESIO_SCADA;    
        else                                    opr->stscode1 &= (~LED_ESIO_SCADA);
    
        if(esioCFG[ESIO_REMOTE]->online == SET) opr->stscode1 ^= LED_ESIO_REMOTE;    
        else                                    opr->stscode1 &= (~LED_ESIO_REMOTE); 
    
        if(esioCFG[ESIO_ELECQ]->online == SET)  opr->stscode1 ^= LED_ESIO_ELECQ;    
        else                                    opr->stscode1 &= (~LED_ESIO_ELECQ);              
        
        if(esioCFG[ESIO_61850]->online == SET)  opr->stscode1 ^= LED_ESIO_61850;    
        else                                    opr->stscode1 &= (~LED_ESIO_61850);     
    
        if(esioCFG[ESIO_RTU]->online == SET)    opr->stscode1 ^= LED_ESIO_RTU;    
        else                                    opr->stscode1 &= (~LED_ESIO_RTU); 
    }
    
    /* -------------------------------- */
    /* ICCP 운영상태 동작상태 확인...   */
    /* -------------------------------- */
    if(iccpDCB->assocStatus)
    {    
        if(iccpDCB->commMaster)     opr->stscode ^= LED_D1_BIT;    
        else                        
        {
            if(iccpRunTick++ > 5)
            {
                iccpRunTick = 0;
                opr->stscode ^= LED_D1_BIT;    /* RUN led */
            }
        }
    }
    else
        opr->stscode &= (~LED_D1_BIT);    
        
    /* -------------------------------- */
    /*  이중화 CPU모듈 통신상태 표시    */
    /* -------------------------------- */   
    if(linkCfg->online == SET)  opr->stscode1 |= LED_LINK_ONLINE;         // MST-LED ON
    else                        opr->stscode1 &= (~LED_LINK_ONLINE);      // MST-LED ON            
        
    /* -------------------------------- */
    /*  SCU모듈 통신상태 표시           */
    /* -------------------------------- */       
    if(scuCfg->online)  opr->stscode1 |= LED_SCU_ONLINE;
    else                opr->stscode1 &= (~LED_SCU_ONLINE);    
 
    /* -------------------------------- */
    /* 상단부 전면 LED 상태표출 ...     */
    /* -------------------------------- */
    write_led(ledPort1.id, opr->stscode);
    write_led(ledPort2.id, opr->stscode1);
           
	
}

/*
*   ESIO 관련 초기화
*/
void    WDT_sdp_initial()
{
    int sioid;
    ESIO_CONFIG     *esio;
    
    for(sioid = 0; sioid < MAX_ESIO; sioid++)
    {
        esio   = (ESIO_CONFIG *) esioCFG[sioid];       // ESIO 장치 Config 
           
        esio->rackInstall   = 3;        // RACK 실장상태 : 이상
        esio->sioRunFail    = 3;        // SIO 동작상태 : 정상
        esio->online        = 0;    
    }
    
    mpuCFG->mpuRackSts  = 0;
    mpuCFG->mpuRunSts   = 0;
    
    for(sioid = 0; sioid < MAX_VME_SIO; sioid++)
   	{ 
    	vmeRunTick[sioid] = 0;        //모듈동작 이상
		vmeFailTick[sioid] = 0;       //모듈동작 이상
		vmeFailType[sioid] = 0;       //모듈장착 이상
		vmeActCount[sioid] = 0;
	}
	
}


/*
*   FUNCTION : hostDCBInitial()
*   - 계전기 장치 구조체 ... 초기화
*/
void WDT_hostDCBInitial()
{
    int i, j;
    
    VME_CHAN_DCB    *vmeChan;
    HOST_DCB        *host;
    DNP_ANA_INPUT   *anaPoint;
    
    VME_SIODCB      *dnpHostSIO;
    
    // dnpHostSIO  개발 VME board를 지시하는 변수..
    dnpHostSIO = (VME_SIODCB *) vmeSioDCB[0];
    
    /* --------------------------- */
    /*    VME CHAN pointer init    */
    /* --------------------------- */
    for(i = 0; i < 8; i++)
    {
        // vmeChan 이 각 Channel 별 위치 변수
        vmeChan = (VME_CHAN_DCB *) &dnpHostSIO->vmeChan[i];
        vmeChan->rxFront = 0;
        vmeChan->rxRear  = 0;
        vmeChan->rxCount = 0;
        vmeChan->rxFlag  = 0;        
        
        vmeChan->txCount = 0;
        vmeChan->txFlag  = 0;
    }    
    
    /* --------------------------- */
    /*    HOST 별 운영정보 init    */
    /* --------------------------- */        
    for(i = 0; i < MAX_HOST; i++)
    {
        host = (HOST_DCB *) hostDCB[i];                 /* 주장치 #1 속성정의 */
        
        host->id = i;
        host->initial	= SET;

        /* DNP initialize ... */
        host->sndFlag    = RESET;
        host->sndLinkSts = RESET;
        host->rcvLinkSts = RESET;           // LINK RESET 상태 
        host->rcvEndOk   = SET;

        host->diIndexWord = RESET;          // DI Point 최대 수 : 256보다 큰경우 WORD 처리 
        host->aiIndexWord = RESET;          // AI Point 최대 수 : 256보다 큰경우 WORD 처리 

        host->online[MASTER_PORT]      = 0;    // default OFFLINE
        host->online[SLAVE_PORT]       = 0;    // default OFFLINE
        
        /* ---------------------------------------------------- */
        /*  상위 HOST별 DI/AI 포인트 수에 따른 데이터 처리      */
        /* ---------------------------------------------------- */
        if(host->diPtNum > 255) host->diIndexWord    = SET;
        if(host->aiPtNum > 255) host->aiIndexWord    = SET; 
            
        /* HOST 미지정시... */
        if(host->hostDualMode == HOST_NOT_USE)  continue;
        if(host->hostComType == COM_TCPIP)      continue;    

        /* ------------------------------------ */
        /* Default VME Channel 지정             */
        /* ------------------------------------ */
        host->vmeChan[0] = (VME_CHAN_DCB *) &dnpHostSIO->vmeChan[host->vmeMstChan];       /* MASTER 포트 VME 초기화 */  
        host->vmeChan[1] = (VME_CHAN_DCB *) &dnpHostSIO->vmeChan[host->vmeSlvChan];       /* SLAVE  포트 VME 초기화 */   
        
        if(host->hostProtocol == HOST_IEC_101)
        {
            host->vmeChan[0]->protocolType = SCAN_IEC;        // HOST 별 프로토콜 타입 지정 
            host->vmeChan[1]->protocolType = SCAN_IEC;
            host->vmeChan[0]->cfgParity    = PARITY_EVEN;
            host->vmeChan[1]->cfgParity    = PARITY_EVEN;
            
            host->vmeChan[0]->cfgData      = DATA_8_BIT;
            host->vmeChan[1]->cfgData      = DATA_8_BIT;
            host->vmeChan[0]->cfgStopbit   = STOP_1_BIT;
            host->vmeChan[1]->cfgStopbit   = STOP_1_BIT;
        }
        else if(host->hostProtocol == HOST_DNP)
        {
            host->vmeChan[0]->protocolType = SCAN_DNP;        // HOST 별 프로토콜 타입 지정 
            host->vmeChan[1]->protocolType = SCAN_DNP;
            host->vmeChan[0]->cfgParity    = PARITY_NONE;
            host->vmeChan[1]->cfgParity    = PARITY_NONE;
            
            host->vmeChan[0]->cfgData      = DATA_8_BIT;
            host->vmeChan[1]->cfgData      = DATA_8_BIT;
            host->vmeChan[0]->cfgStopbit   = STOP_1_BIT;
            host->vmeChan[1]->cfgStopbit   = STOP_1_BIT;
        }
        else if(host->hostProtocol == HOST_MODBUS)
        {
            host->vmeChan[0]->protocolType = SCAN_MODBUS;        // HOST 별 프로토콜 타입 지정 
            host->vmeChan[1]->protocolType = SCAN_MODBUS;
            host->vmeChan[0]->cfgParity    = PARITY_NONE;
            host->vmeChan[1]->cfgParity    = PARITY_NONE;
            
            host->vmeChan[0]->cfgData      = DATA_8_BIT;
            host->vmeChan[1]->cfgData      = DATA_8_BIT;
            host->vmeChan[0]->cfgStopbit   = STOP_1_BIT;
            host->vmeChan[1]->cfgStopbit   = STOP_1_BIT;
        }
        else if(host->hostProtocol == HOST_HARRIS)
        {
            host->vmeChan[0]->protocolType = SCAN_HARRIS;        // HOST 별 프로토콜 타입 지정 
            host->vmeChan[1]->protocolType = SCAN_HARRIS;
            host->vmeChan[0]->cfgParity    = PARITY_ODD;
            host->vmeChan[1]->cfgParity    = PARITY_ODD;
            
            host->vmeChan[0]->cfgData      = DATA_7_BIT;
            host->vmeChan[1]->cfgData      = DATA_7_BIT;
            host->vmeChan[0]->cfgStopbit   = STOP_1_BIT;
            host->vmeChan[1]->cfgStopbit   = STOP_1_BIT;
        }
        else
        {
            host->vmeChan[0]->protocolType = SCAN_ASYNC;        // HOST 별 프로토콜 타입 지정 
            host->vmeChan[1]->protocolType = SCAN_ASYNC;
            host->vmeChan[0]->cfgParity    = PARITY_NONE;
            host->vmeChan[1]->cfgParity    = PARITY_NONE;
            
            host->vmeChan[0]->cfgData      = DATA_8_BIT;
            host->vmeChan[1]->cfgData      = DATA_8_BIT;
            host->vmeChan[0]->cfgStopbit   = STOP_1_BIT;
            host->vmeChan[1]->cfgStopbit   = STOP_1_BIT;
        }
            
        host->vmeChan[0]->cfgType      = host->hostComType;
        host->vmeChan[1]->cfgType      = host->hostComType;

        host->vmeChan[0]->cfgSpeed     = host->hostComSpeed;
        host->vmeChan[1]->cfgSpeed     = host->hostComSpeed;
                        
        for(j=0; j < MAX_DNP_AI_POINT; j++)
        {
            anaPoint = (DNP_ANA_INPUT *) &hostDCB[i]->ana_pointData[j];
            anaPoint->pointData = 0;
        }

    }
    
    //Debug(console,"wdt> => HOST DNP initial...!\n");
}


/*
*   SDP 장치별 - SIO/ESIO 보드 장착 Check...
*/
void vmeModule_Check()
{
    int     sioid;
    byte    rackStatus;
    char    buffer[256];
    VME_SIODCB      *vmeSIO;
    ESIO_CONFIG     *esio;
    
    /* -------------------------------------------- */
	/*	VME Module 상태 Check (MAX_VME_SIO : 5)      */
	/* -------------------------------------------- */
    for(sioid = 0; sioid < MAX_VME_SIO; sioid++)
    {    
        vmeSIO = (VME_SIODCB *) vmeSioDCB[sioid];            // VME SPACE
        esio   = (ESIO_CONFIG *) esioCFG[sioid];             // ESIO 장치 Config 
        
        /* ESIO# 을 사용하지 않는경우... */
        if(esio->useFlag == RESET)
        {
            esio->rackInstall  = RESET;     // RACK 실장상태 : 이상
            esio->sioRunFail   = RESET;     // SIO 동작상태 : 정상
            vmeFailType[sioid] = 0;
            vmeFailTick[sioid] = 0;
            vmeRunTick[sioid] = 0;
            continue;
        }
        
        /* -------------------------------------------- */
        /*  ESIO# 장착상태 : Valid Info Check ..0x1234  */
        /* -------------------------------------------- */                  
        if(vmeSIO->sioValid != 0x1234)
        {
            vmeFailType[sioid]++;

            /* ---------------------------------------- */
            /*  보드별 장착상태 Check...                */  
            /* ---------------------------------------- */          
            if(vmeFailType[sioid] > 5)
            {
            	if(opr->wdtDebug)
	            Debug(console,"wdt> *Invalid SIO(%d) TYPE (Valid=0x1234) : SIO=%04x\n", sioid+1, vmeSIO->sioValid);

                if(esio->rackInstall == SET)
                {
                    /* ------------------------------------------------ */
                    /*  MPU 기동 Event...                               */
                    /* ------------------------------------------------ */
                    logEvent_MPU(shmPtr, ENT_VME_UNINSTALL, sioid+1, 0, 0, sioid+1, NULL);     // MPU 재기동 Event...
                    Debug(console,"wdt> check VME : *Invalid SIO(%d) TYPE (Valid=0x1234) : SIO=%04x\n", sioid+1, vmeSIO->sioValid);
                    
                    /* LOG File 저장 */
                    sprintf(buffer, "*ESIO[%d] Invalid TYPE...(%04x)  ", sioid+1, vmeSIO->sioValid);
                    LogFile_MPU (shmPtr, ENT_VME_UNINSTALL, buffer, strlen(buffer));
                }
                     
                esio->rackInstall  = RESET;    // // RACK 실장상태 : 이상
                esio->sioRunFail   = SET;    // SIO 동작상태 : 이상
                vmeFailType[sioid] = 0;
                
                if(sioid == 0)  esio->online = RESET;     // SIO의 경우... 
            }
            
            continue;
        }
        else
        {   
            if(esio->rackInstall == RESET)
            {
                /* ------------------------------------------------ */
                /*  MPU 기동 Event...                               */
                /* ------------------------------------------------ */
                logEvent_MPU(shmPtr, ENT_VME_INSTALL, sioid+1, 0, 0, sioid+1, NULL);     // MPU 재기동 Event...
                Debug(console,"wdt> check VME : Valid SIO(%d) TYPE (Valid=0x1234) : SIO=%04x\n", sioid+1, vmeSIO->sioValid);
                
                /* LOG File 저장 */
                sprintf(buffer, "ESIO[%d] TYPE...(%04x)  ", sioid+1, vmeSIO->sioValid);
                LogFile_MPU (shmPtr, ENT_VME_INSTALL, buffer, strlen(buffer));
            }   
            
            esio->rackInstall  = SET;    // RACK 실장상태 : 정상
            vmeFailType[sioid] = 0;
            
            if(sioid == 0)  esio->online = SET;     // SIO의 경우... 
        } 
        
        /* -------------------------------------------- */
        /*  ESIO# 동작상태 : MPU/SIO Count Check ...    */
        /* -------------------------------------------- */
        if(vmeActCount[sioid] != vmeSIO->sioCount)  // 맨 처음에는...1번은 발생하네..
        {
            if(opr->wdtDebug)
            Debug(console,"wdt> *Invalid SIO(%d) RUN ...: MPU=%04d / SIO=%04d\n", sioid+1, vmeActCount[sioid], vmeSIO->sioCount);

            vmeFailTick[sioid]++;
            vmeRunTick[sioid] = 0;
            
            /* ---------------------------------------- */
            /*  보드별 장착상태 Check...                */  
            /* ---------------------------------------- */          
            if(vmeFailTick[sioid] > 5)
            {
                if(esio->sioRunFail == RESET)
                {
                    /* ------------------------------------------------ */
                    /*  MPU 기동 Event...                               */
                    /* ------------------------------------------------ */
                    logEvent_MPU(shmPtr, ENT_VME_OFFLINE, sioid+1, 0, 0, sioid+1, NULL);     // MPU 재기동 Event...
                    Debug(console,"wdt> check VME : *Offline SIO(%d) \n", sioid+1);
                    
                    /* LOG File 저장 */
                    sprintf(buffer, "*ESIO[%d] Offline ", sioid+1);
                    LogFile_MPU (shmPtr, ENT_VME_OFFLINE, buffer, strlen(buffer));
                }
                esio->sioRunFail   = SET;    // SIO 동작상태 : 이상
                vmeFailTick[sioid] = 0;
                
                if(sioid == 0)  esio->online = RESET;     // SIO의 경우...
            }
        }
        else
        {   
            vmeRunTick[sioid]++;
            
            if(vmeRunTick[sioid] > 5)
            {
                vmeRunTick[sioid] = 0;
                if(esio->sioRunFail == SET)
                {
                    /* ------------------------------------------------ */
                    /*  MPU 기동 Event...                               */
                    /* ------------------------------------------------ */
                    logEvent_MPU(shmPtr, ENT_VME_ONLINE, sioid+1, 0, 0, sioid+1, NULL);     // MPU 재기동 Event...
                    Debug(console,"wdt> check VME : Online SIO(%d) \n", sioid+1);
                    
                    /* LOG File 저장 */
                    sprintf(buffer, "ESIO[%d] Online ", sioid+1);
                    LogFile_MPU (shmPtr, ENT_VME_ONLINE, buffer, strlen(buffer));
                }
                
                esio->sioRunFail   = RESET;      // SIO 동작상태 : 정상
            }   

            vmeFailTick[sioid] = 0;
            if(sioid == 0)  esio->online = SET;     // SIO의 경우... 
        } 
            
        /* ---------------------------------------- */
        /*  MPU 동작상태 => SIO 연계                */
        /* ---------------------------------------- */
        vmeActCount[sioid]++;  // 이건 local 변수지..
        vmeSIO->activeCount = vmeActCount[sioid];     // VME Read/Write Check
        
        /* ---------------------------------------- */
        /*  MPU 동작모드 => SIO 연계                */
        /* ---------------------------------------- */
        if(opr->runMode == LOCAL_MASTER)    vmeSIO->localMaster = SET;                      // SIO 보드 : Active 상태정보... 
        else                                vmeSIO->localMaster = RESET;                    // SIO 보드 : Active 상태정보...      
            
    }
    
    /* ------------------------------------------------ */
    /*  SDP - RACK 실장상태 표시....                    */
    /* ------------------------------------------------ */
    rackStatus = 0;
    if(esioCFG[0]->rackInstall)     rackStatus = 0x01;      // SIO 모듈 장착 상태
    if(esioCFG[1]->rackInstall)     rackStatus |= 0x02;     // ESIO#1 : 전력제어부 장착 상태    
    if(esioCFG[2]->rackInstall)     rackStatus |= 0x04;     // ESIO#2 : 원격진단부 장착 상태    
    if(esioCFG[3]->rackInstall)     rackStatus |= 0x08;     // ESIO#3 : 전력품질부 장착 상태            
    if(esioCFG[4]->rackInstall)     rackStatus |= 0x10;     // ESIO#4 : 61850      장착 상태        

    if(esioCFG[5]->online)          rackStatus |= 0x20;     // RTU : 장착 상태 => ONLINE 상태    
    if(scuCfg->online)              rackStatus |= 0x40;     // SCU : 장착 상태 => ONLINE 상태  
        
    mpuCFG->mpuRackSts = rackStatus;          // Master-MPU RACK 상태정보    
    
    
}

/*
*   SDP 장치별 - ESIO Network 구성정보 참조...
*   VME 를 통해 Network 구성정보를 보낸다.---> 처음 DB가 틀린 경우 이것으로 DB를 받는다 
*/
void WDT_ESIO_Config()
{
    int         i,j;
    int         sioid;
    int         k;
    
    VME_SIODCB      *vmeSIO;
    ESIO_CONFIG     *esio;
    ESIO_NET_ENTRY  *enetCfg;
    
    int   netCount ; 
    
#ifdef __ARM_ARCH__
    netCount = 4 ;
#else
    netCount =3 ;    
#endif 
    

    /* ------------------------------------------------ */
	/*	MPU에서 ESIO Config - Network 정보를 초기화 함  */
	/* ------------------------------------------------ */
    for(sioid = 1; sioid < MAX_VME_SIO; sioid++)  // SIO 제외하고
    {    
        // VME 영역
        vmeSIO = (VME_SIODCB *) vmeSioDCB[sioid];
        // esio 영역
        esio   = (ESIO_CONFIG *) esioCFG[sioid];             // ESIO 장치 Config 
        
        //printf("ESIO[%d] ... %d %d %d \n", sioid, esio->useFlag, esio->rackInstall, esio->sioRunFail);
        
        if(esio->useFlag == RESET)  	continue;  // 사용안하고
        if(esio->rackInstall == RESET)  continue;  // 장착이 안 되 있고
        if(esio->sioRunFail == SET)  	continue;  // run 이 안 되고 있음.  

#if 0
        if(sioid != 1)  continue;       // 시험용...
#endif
            
      //  printf("WDT> ESIO[%d] ... VME config Check.... !\n", sioid);
         
        /* MPU 모드 지정 */
        vmeSIO->mpuMode = opr->cpuMode;  // mastr or slave
                  
        /* -------------------------------------------- */
        /*  ESIO 보드와 비정상 통신시...                */   
        /* -------------------------------------------- */ 
        // 연결이 안 된 경우만..
        if((esio->connectStatus == RESET) || (esio->online != SET))
        {
            //if((vmeSIO->netChangeReq == 0x1234) || (esio->vmeNetConfig == SET))
               
            if(vmeSIO->netChangeReq == 0x1234) // esio가 요청을 하면 
            {
                /* Network 정보 ...Update */
                // 2026-05-26 오후 4:20:23 
                for(i=0; i < netCount; i++)
                {
                    if(opr->cpuMode == MPU_A)   enetCfg = (ESIO_NET_ENTRY *) &esio->mstNetConfig[i]; 
                    else                        enetCfg = (ESIO_NET_ENTRY *) &esio->slvNetConfig[i]; 
            
                    vmeSIO->useFlag[i] = enetCfg->useFlag;

// endian 땀시                    
//                   printf("ipAddr %s\r\n",enetCfg->ipAddr);
//                   printf("gwAddr %s\r\n",enetCfg->gwAddr);
//                   printf("subMask %s\r\n",enetCfg->subMask);                   

// esio로 넘길 때 byt 단위 문제로..
#ifdef __ARM_ARCH__  // MPU-ARM <->ESIO-ARM        
                    for(j=0; j <16; j++) // byte  단위로 쓴다.
                    {   // 
                        vmeSIO->ipAddr[i][j] = enetCfg->ipAddr[j];
                        vmeSIO->gwAddr[i][j] = enetCfg->gwAddr[j];
                        vmeSIO->subMask[i][j]= enetCfg->subMask[j];                                      
                    }
#else  // MPU-ARM <-> ESIO-PPC
 
                    for(k=0,j=0; k<8 ;j+=2,k++) // byte  단위로 쓴다.
                    {   // 
                        vmeSIO->ipAddr[i][k] = enetCfg->ipAddr[j+1] +  (enetCfg->ipAddr[j] << 8) ;
//                        printf("%04X-%02X:%02X - ", vmeSIO->ipAddr[i][k],  enetCfg->ipAddr[j ],enetCfg->ipAddr[j+1]  ) ;
                        vmeSIO->gwAddr[i][k] = enetCfg->gwAddr[j+1] +  (enetCfg->gwAddr[j] << 8) ;
                        vmeSIO->subMask[i][k]= enetCfg->subMask[j+1] +  (enetCfg->subMask[j] << 8) ;                                       
                    }

#endif 

                    
#if 0                    
                    printf("WDT_ESIO[%d]_Config[%d]\r\n",sioid,i);                    
                    printf("ipADDR ") ;
                    
                    for(j=0; j <16; j++) // byte  단위로 쓴다.
                   {                           
                        printf("[%c]" ,   vmeSIO->ipAddr[i][j]);                        
                    }                    
                    printf("gwADDR ") ;                    
                    for(j=0; j <16; j++) // byte  단위로 쓴다.
                   {                           
                        printf("[%c]" ,   vmeSIO->gwAddr[i][j]);                        

                    }   
                    printf("subMask ") ;  
                    for(j=0; j <16; j++) // byte  단위로 쓴다.
                   {                           
                        printf("[%c]" ,   vmeSIO->subMask[i][j]);                        

                    }            
                   printf("\r\n"); 
                               
#endif 
                    
                    
                }
                
                Debug(console,"wdt> ESIO_CONFIG : the num of NET %d\r\n",netCount);
                
                if(vmeSIO->netChangeReq == 0x1234)      Debug(console,"wdt> *** ESIO: VME REQ... ESIO(%d) Network Initial...[%02d:%02d:%02d] \n", sioid, rtc->hour, rtc->min, rtc->sec);
                else                                    Debug(console,"wdt> *** MPU : ESIO(%d) Network Initial...[%02d:%02d:%02d] \n", sioid, rtc->hour, rtc->min, rtc->sec);
                
                esio->vmeNetConfig  	= 0;
                vmeSIO->netChangeReq 	= 0; //요청은 지우고 
                vmeSIO->netCfgChange 	= 0x1234;   // 써다.     
                    
            }   
                
        }
        // 중간에 param 이 변경ㅇ된 경우
    }
        
}

unsigned char   read_i2c ( int fd )
{
    char buf[3];
    buf[0]=0x0;
    int rc; 
    
    /* set register */
    if ( (rc=write( fd, buf, 1) ) != 1)
    {
        perror("set addr\r\n");
        return 0;
    }

    /*read */
    if ( (rc=read ( fd, buf, 1) ) != 1)
    {
        perror("set addr\r\n");
        return 0;
    }

   return  ( unsigned char) buf[0];
      
}

/*
*   CPU 이중화 상태 Check...
*/
int	checkCPU_mode()
{

	/* -------------------------------------------- */
	/*  이중화 CPU 상태 Check...I2C Read            */
	/* -------------------------------------------- */
    linkCfg->cpuStatus =  read_i2c (devInput.id);
	
    //Debug(console,"wdt> CPU status = %02x ...\n",linkCfg->cpuStatus);
    printf("wdt> CPU status = %02x ...\n",linkCfg->cpuStatus);
    
    /* ---------------------------------------- */
  	/*  MASTER-CPU 의 경우...                   */
    /* ---------------------------------------- */
  	if(linkCfg->cpuStatus & SYS_MASTER_BIT)  
    {
        opr->cpuMode = MPU_A;

        printf("\n=======================================\n");
        printf("   **** MPU-MODE : MPU-A  \n");
        printf("=======================================\n");
    }
    else 
    {
        opr->cpuMode = MPU_B;

        printf("\n=======================================\n");
        printf("   **** MPU-MODE : MPU-B  \n");
        printf("=======================================\n");
    }
    
    return (1);        
}


static	int	cpu_change_count = 0;


/*
*   MPU RACK 동작상태 Check....
*/
void update_MPU_status()
{
    int     i;
    int     chgOnline, chgInstall;
    byte    runStatus=0;
    char	buffer[256];
    
    ESIO_CONFIG     *esio;
    
    /* -------------------------------------------------------- */
    /*  SDP RACK 동작상태 정보                                  */
    /*  0: SIO, 1: ESIO1, 2:ESIO2; 3:ESIO3, 4:ESIO4, 5: RTU     */
    /* -------------------------------------------------------- */
    runStatus = 0;
 
    if(esioCFG[0]->rackInstall)     runStatus = 0x01;       // SIO 모듈 동작상태
    
    if(esioCFG[1]->online == SET)   runStatus |= 0x02;     // ESIO#1 : 전력제어부 장착 상태    
    if(esioCFG[2]->online == SET)   runStatus |= 0x04;     // ESIO#2 : 원격진단부 장착 상태    
    if(esioCFG[3]->online == SET)   runStatus |= 0x08;     // ESIO#3 : 전력품질부 장착 상태            
    if(esioCFG[4]->online == SET)   runStatus |= 0x10;     // ESIO#4 : 61850      장착 상태        

    if(esioCFG[5]->online == SET)   runStatus |= 0x20;     // RTU : 장착 상태     
    if(scuCfg->online == SET)       runStatus |= 0x40;     // SCU : 장착 상태   
            
    mpuCFG->mpuRunSts = runStatus;

    if(opr->checkDebug)
    {
        Debug(console, "\n-----------------------------------------------------\n");
        if(opr->runMode == LOCAL_MASTER)    Debug(console," MPU Status ==> %s [ ACTIVE ] \n", cpuStsStr[opr->cpuMode]);
        else                                Debug(console," MPU Status ==> %s [ STAND-BY ] \n", cpuStsStr[opr->cpuMode]);  
        
        Debug(console," MPU RUN  Status    = %02x [rcvSTS = %02x]\n", mpuCFG->mpuRunSts, mpuCFG->rcvRunSts);  
        Debug(console," MPU LINK Status    = %02x [online = %d  ]\n", linkCfg->cpuStatus, linkCfg->online);  
        Debug(console," SDP CPU Mode (DUAL)= %s / online = %d\n", cpuModeStr[opr->dualCpuSts], linkCfg->online);   
        Debug(console," SCU Channel Status = %02x [Remote=%d] [Chan=%d] \n", scuCfg->status, scuCfg->remoteMode, scuCfg->chanMode);      
        Debug(console," ICCP Comm Status   = AssocSts = %d, Active = %d\n", iccpDCB->assocStatus, iccpDCB->commMaster); 
        Debug(console, "-----------------------------------------------------\n");      
        Debug(console,"wdt> RACK-Install : %d - %d %d %d %d - %d \n", esioCFG[0]->rackInstall, esioCFG[1]->rackInstall, 
            esioCFG[2]->rackInstall, esioCFG[3]->rackInstall, esioCFG[4]->rackInstall, esioCFG[5]->rackInstall);
        Debug(console,"wdt> ESIO RUN-Fail: %d - %d %d %d %d - %d \n", esioCFG[0]->sioRunFail, esioCFG[1]->sioRunFail, 
            esioCFG[2]->sioRunFail, esioCFG[3]->sioRunFail, esioCFG[4]->sioRunFail, esioCFG[5]->sioRunFail);        
        Debug(console,"wdt> ESIO Online  : %d - %d %d %d %d - %d \n\n", esioCFG[0]->online, esioCFG[1]->online, 
            esioCFG[2]->online, esioCFG[3]->online, esioCFG[4]->online, esioCFG[5]->online);    
        
        Debug(console, "wdt> check ESIO ...cpu change mode=%d,  Tick=%d, dual=%d, remote=%d\n", opr->cpuChgMode, opr->cpuChgTick, opr->dualCpuSts, scuCfg->remoteMode );    
        Debug(console, "wdt> SDP DB Change-Flag : MPU=%d, ESIO=%d, HOST=%d, SCAN=%d, DEVICE=%d, POINT=%d, MDBUS=%d \n",
            opr->mpuCfgDown_OK, opr->esioCfgDown_OK,opr->hostCfgDown_OK,opr->scanCfgDown_OK, 
            opr->devCfgDown_OK, opr->pointCfgDown_OK, opr->modbusCfgDown_OK);
    }

#if 0
    /* ---------------------------------------------------------------- */
    /* ICCP Active status 에 따른 이중화 CPU 자동절체 							*/
    /* - 2019-05-24 : ICCP 통신상태에 따른 SDP 절체기능 => 중지						*/
    /* ---------------------------------------------------------------- */
    if((opr->cpuChgMode == CHECK_SDP_FULL) && (opr->dualCpuSts == SET) && (opr->runMode == LOCAL_SLAVE) && (scuCfg->remoteMode == AUTO_MODE))
    { 
    	if((iccpDCB->assocStatus) && (iccpDCB->commMaster) && (linkCfg->online == SET))
    	{
    		Debug(console,"\n==================================\n");      
            Debug(console,"wdt> *** ICCP-Active ... CPU-CHANGE ... %d\n");
            Debug(console,"==================================\n");
			
			logEvent_MPU(shmPtr, ENT_CHANGE_CPU, CPU_CHG_ACTIVE, 0, 0, opr->cpuMode, NULL); 
			
            opr->cpuChange = SET;
            pause(1000);
    	}	
  	}
#endif
  	  	
    /* ---------------------------------------------------------------- */
    /* 이중화 CPU 자동절체 : CPU 절체 허용시, 자동/LOCAL_MASTER         */
    /* ---------------------------------------------------------------- */
    if((opr->cpuChgMode == CHECK_SDP_FULL) && (opr->dualCpuSts == SET) && (opr->runMode == LOCAL_MASTER) && (scuCfg->remoteMode == AUTO_MODE))
    { 
    	/* 2020.06.03 SCU 통신상태 Check... */
    	if(scuCfg->online != SET)	return;
    	
    	/* ------------------------------------------------ */
    	/*	CPU Active 절체후 .. 20초 뒤... Change Check 		*/
    	/* ------------------------------------------------ */
        if(opr->cpuChgTick > 0)
        {
            opr->cpuChgTick--;
            return ;
        }
              
        /* -------------------------------- */
        /* SIO/ESIO/RTU 통신이상시 .. 절체  */
        /* -------------------------------- */
        chgOnline = RESET;
        //for(i=0; i < MAX_ESIO; i++)
        for(i=0; i < MAX_ESIO - 1; i++)		// 2020.06.01, RTU Online Check... 제외 
        {
            esio   = (ESIO_CONFIG *) esioCFG[i];             // ESIO 장치 Config 
            if(esio->useFlag == RESET)  	continue;
            if(esio->autoChgFlag == RESET)  continue;		// 자동절체 Flag 가 OFF 시
            if(esio->online == 0)   chgOnline = i+1;      
        }
    
        /* -------------------------------- */
        /* SIO/ESIO 미장착시... 절체        */
        /* -------------------------------- */
        chgInstall = RESET;
        for(i=0; i < MAX_VME_SIO; i++)
        {
            esio   = (ESIO_CONFIG *) esioCFG[i];             // ESIO 장치 Config 
            if(esio->useFlag == RESET)  	continue;
            if(esio->autoChgFlag == RESET)  continue;		// 자동절체 Flag 가 OFF 시	
            if(esio->rackInstall == RESET)  chgInstall = i+1;   
        }
        
        /* -------------------------------- */
        /* SIO/ESIO 통신이상/미장착시... 자동 절체   */
        /* -------------------------------- */
        if( chgOnline + chgInstall)
        {        
            logEvent_MPU(shmPtr, ENT_CHANGE_CPU, CPU_CHG_ESIO, 0, 0, opr->cpuMode, NULL); 
                    
            if(chgOnline)
            {   
                Debug(console,"\n==================================\n");      
                Debug(console,"wdt> *** ESIO OFFLINE CPU-CHANGE ... %d\n", chgOnline);
                Debug(console,"==================================\n");
                
                /* LOG File 저장 */
    			sprintf(buffer, "%s", "*** CPU Change : ESIO OFFLINE Change ...");
    			LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
            }
            
            if(chgInstall)
            {   
                Debug(console,"\n==================================\n");      
                Debug(console,"wdt> *** ESIO NOT-INSTALL  CPU-CHANGE ... %d\n", chgInstall);
                Debug(console,"==================================\n");
                
                /* LOG File 저장 */
    			sprintf(buffer, "%s", "*** CPU Change : ESIO NOT-INSTALL Change ...");
    			LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
            }
                                
            opr->cpuChange = SET;
            opr->cpuChgTick = 20;		// 20초 뒤에 자동절체 Check...
        }
                    
    }      
}

/*
*   ESIO Time Update...
*/
void    update_SystemTime()
{
    int i;
 
    SDP_DEVICE      *dev;
    ESIO_CONFIG     *esio;
    
     /* ------------------------------------ */
	/* MPU Time 변경후  모듈 초기화  ....   */
	/* ------------------------------------ */
    for(i = 0; i < MAX_DEVICE; i++)
    {
        dev = (SDP_DEVICE *) deviceCFG[i];

        dev->timeSyncReq = SET;
        dev->statusDump  = SET;
    }

    /* ------------------------------------ */
	/* MPU Time 변경후  ESIO 초기화  ....   */
	/* ------------------------------------ */
    for(i = 0; i < MAX_ESIO; i++)
    {
        esio = (ESIO_CONFIG *) esioCFG[i];
        esio->timeSyncReq = SET;
    }

}

// hkkim
/*
* VME 버스상의 SIO 모듈 Time-Sync
*/
static void vmeTimeSync()
{
	int	year;
	
	/* ---------------------------------------- */
	/*	SLAVE 모드에서 VME Access 를 하지 않음  */
	/* ---------------------------------------- */
	if(opr->runMode == LOCAL_SLAVE)     return;

    /* VME BUS 상의 SIO보드 - Time Sync...  */        
	year = (rtc->year % 100) + 2000;
    vmeSioDCB[0]->year  = year;
    vmeSioDCB[0]->month = rtc->month;
    vmeSioDCB[0]->day   = rtc->day;
    vmeSioDCB[0]->week  = rtc->week;
    vmeSioDCB[0]->hour  = rtc->hour;
    vmeSioDCB[0]->min   = rtc->min;
    vmeSioDCB[0]->sec   = rtc->sec;
    
    vmeSioDCB[0]->rtcUpdateFlag = 0x1234;
    
    if(opr->wdtDebug)
	Debug(console,"[vme Time-Sync] %04d:%02d:%02d-%02d-%02d:%02d:%02d ...\n", 	year, rtc->month, rtc->day, rtc->week, rtc->hour, rtc->min, rtc->sec);

}


/* ======================================================== */
/*  현재 system time 을 백업파일에 저장한다.                */
/*  - RTC backup 유지시간(약 5분)이 짧아, 장시간 무전원 후   */
/*    부팅하면 RTC 가 1970 으로 리셋된다. 그때 복구할 값을    */
/*    미리 비휘발성 파일에 남겨두는 것이 목적.               */
/*  - system time 자체가 무효(복구 실패 상태)면 저장하지      */
/*    않는다. 무효값으로 덮어쓰면 다음 부팅 때 복구할 수단이  */
/*    아예 없어지기 때문.                                    */
/*  - 성공 시 hour 카운터를 리셋한다. 실패하면 리셋하지       */
/*    않으므로 다음 hour 변화에서 자동 재시도된다.            */
/* ======================================================== */
static void wdt_TimeBackupSave(const char *reason)
{
    time_t  now = time(NULL);

    if(now < WDT_TIME_MIN_VALID)
    {
        Debug(console, "wdt> time-backup skip (%s) : system time invalid (%ld)\n", reason, (long)now);
        return;
    }

    if(time_backup_save(TIME_BACKUP_DEFAULT_PATH) < 0)
    {
        Debug(console, "WDT-ERR> *** time-backup save FAIL (%s) : %s\n", reason, strerror(errno));
        return;
    }

    timeBackupHourCnt = 0;
    Debug(console, "wdt> time-backup saved (%s) : %ld\n", reason, (long)now);
}


/* ======================================================== */
/*  SDP MAIN 프로그램 Start Routine ....                    */
/* ======================================================== */
int	main(int argc, char **argv)
{
	int		oldsec, oldhour;
	int     size, retVal;
	int     ntpSyncFail;
	word    oldFront;
    char    buffer[256];
    
    int     devPtIndex;
    int		rtcCheckTick;
    
    POINT_BUF   *devPoint;
    MPU_SOEQ_ENTRY  devEvent;

    int rc; 
//    char exePath[100];

//extern int get_cwd(  char *exePath ) ;
//get_cwd(exePath) ;
//       printf("dir : %s\r\n" ,  exePath ) ;

#ifndef __ARM_ARCH__
    //#error ARCH is PPC
#else 
   //  #err ARCH is ARM
#endif 



#ifdef __PPC_ARCH__
    printf("ARCH is PPC\r\n");
#endif 

#ifdef __ARM_ARCH__
    printf("ARCH is ARM\r\n");
#endif 


    if ((rc= check_already_running("WDT")) < 0)
    {
        // printf("WDT-ERR>> already running..\r\n");
        exit(0);
        
    }

   

    lan_gpio_init();


    /* ------------------------------------------------ */
    /*  RTC backup 방전 대비 시각 복구                  */
    /*  - HW RTC 의 backup 유지시간이 약 5분뿐이므로,   */
    /*    장시간 무전원 후에는 RTC 가 1970 으로 리셋됨.  */
    /*  - 반드시 InitEnv() 보다 먼저 호출해야 한다:      */
    /*    /dev/rtc0 는 한 프로세스만 open 할 수 있는데   */
    /*    InitEnv() 가 열고 나면 종료까지 점유하므로,    */
    /*    그 뒤에는 RTC 재설정이 EBUSY 로 실패한다.      */
    /*  - 공유메모리가 아직 없으므로 printf 로 기록.     */
    /*  - 반환값 -1 은 "RTC 무효 + 백업파일 없음" 인     */
    /*    최초 기동에서도 나오는 정상 상황이므로,        */
    /*    프로세스를 중단시키지 않고 로그만 남긴다.      */
    /* ------------------------------------------------ */
    if(time_backup_sync_on_boot(TIME_BACKUP_DEFAULT_PATH, WDT_TIME_MIN_VALID) < 0)
    {
        printf("WDT> *** time-backup restore FAIL : %s\n", strerror(errno));
    }
    else
    {
        time_t  bootTime = time(NULL);
        printf("WDT> time-backup check OK ... %s", ctime(&bootTime));
    }


    /* ------------------------------------------------ */
	/*  작업 환경을 초기화한다                          */
	/* ------------------------------------------------ */
	if ((termExec = InitEnv()) == 1) // 정상이면 1 , 실패면 0 ...
	{
		DisplayLogo();
	}
    else 
    {
        printf("WDT> [ERROR] InitEnv failed\r\n");
        exit(0 );
    }
        
    
    /* ------------------------------------------------ */
    /*  하드웨어 WDT 기능을 설정함...                   */
    /* ------------------------------------------------ */
    opr->wdtEnable = 1;

    // 2026-08-24 오후 7:39:21  여기서 WDT 실패시 exit하는 이유는 WDT가 구현이 안 되있나 ???
    if ((wdtid=open("/dev/esio_wdt",O_RDWR|O_NDELAY)) < 0)  
	{
		printf("wdt> ***/dev/wdt can't open file\n");
		opr->wdtEnable = 0;
	}
    else
    {
	    if(ioctl(wdtid,PMU_WDT_CTR_ON ,  NULL) < 0 )
		{ 
		    printf("wdt> ioctl fail\n");
		    opr->wdtEnable = 0;    
        }
    }
    
	
    /* 내부변수 초기화 */
    init_variable();

/* hhkim */
   opr->endian = check_endian() ;
    sleep(1);
    
    
    
    /* ------------------------------------------------ */
    /*  MPU 동작모드 - MPU-A/B 결정                     */
    /* ------------------------------------------------ */
    checkCPU_mode();
    
    ioctl(wdtid, PMU_WDT_CTR_CLR, NULL);   /* WDT Clear.... */
    
    size = sizeof(RTU_DATABASE); // 아래 출력에 사용된다.
    if(dbFileRead(opr, rtudb) < 0) // opr의 용도는 ????
    {
        printf("\n=======================================\n");
        printf("[ERROR] wdt> *** RTUDB Read failed...!\n");
        printf("=======================================\n");        
        opr->romDBFail = 1;
        
        bzero(rtudb, sizeof(RTU_DATABASE));
        
        dbFileWrite(opr, rtudb);
        
        readRTUdb();            // DB 초기화...  ROMDB->runTimeDB
    }
    else
    {   
        printf("\n=======================================\n");
        printf("wdt> *** RTUDB Read OK... size = %d !\n", size);
        printf("=======================================\n");
        pause(100);
        
        readRTUdb();            // DB 초기화...ROMDB->runTimeDB
    }

    /* ------------------------------------------------ */
    /*  LOCAL Controller TIME 초기화 ....               */
    /*  - RTC Chip 값을 읽어서... System Clock 로       */
    /* ------------------------------------------------ */
	read_RTC1340();
	writeClock(rtc->year, rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec, rtc->week);
	printf("read RTC  : %4d/%02d/%02d-w(%d)-%02d:%02d:%02d\n", rtc->year,rtc->month, rtc->day, rtc->week, rtc->hour, rtc->min, rtc->sec);

	oldsec = rtc->sec;
	// 2026-09-30 오후 5:25:26 by recommned from Claude
	oldhour = rtc->hour;
	
	
	opr->stscode = 0;
	ntpSyncFail  = 0;

	/* LOG File 저장 */
	sprintf(buffer, " %s WDT-MAIN PROCESS Activated ... !", TARGET_NAME);
    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    
    printf(" %s WDT-MAIN PROCESS Activated ... !\n", TARGET_NAME);
	
    ioctl(wdtid, PMU_WDT_CTR_CLR, NULL);   /* WDT Clear.... */
    
    /* ------------------------------------------------ */
    /*  MPU 기동 Event...                               */
    /* ------------------------------------------------ */
    logEvent_MPU(shmPtr, ENT_UNDER_LINE, 0, 0, 0, opr->cpuMode, NULL); 
    logEvent_MPU(shmPtr, ENT_CU_RESTART, 0, 0, 0, opr->cpuMode, NULL);     // MPU 재기동 Event...

    /* LOG File 저장 */
    sprintf(buffer, "%s", "=====================================================");
    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    
    
    // 2026-08-25 오후 1:59:45 왜?  재기동일까...아...죽엇다 살아서...
    if(opr->cpuMode == MPU_A)       sprintf(buffer, "%s", "SDP-A System 재기동---------------------");
    else if(opr->cpuMode == MPU_B)  sprintf(buffer, "%s", "SDP-B System 재기동---------------------");    
    LogFile_MPU (shmPtr, ENT_CU_RESTART, buffer, strlen(buffer));
    
    sprintf(buffer, "%s", "=====================================================");
    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    
    /* ------------------------------------------------ */
    /*  SDP  구조체 변수 초기화 ....                    */
    /*  - MPU/ESIO 구조체 정보 초기화                   */
    /*  - HOST VMESIO 포인트 지정 & ASYNC PORT 지정     */
    /* ------------------------------------------------ */
    WDT_sdp_initial();
    WDT_hostDCBInitial();
    
    rtcCheckTick = 0;
    
	while(termExec)
	{
	    readClock();
	    pause(100);

        /* ---------------------------------------- */
        /* NTP Time GET...                          */
        /* ---------------------------------------- */
        if(opr->getClockNTP == SET)
	    {
	        retVal = get_NTP_Info();        // get NTP-Time Update
	        if(retVal == 1)
	        {
	        	rtc_TimeUpdate();           // RTC Update...
	            opr->getClockNTP = 0;
                update_SystemTime();        // ESIO & DEV Time Update.. 아래랑 비교하면 SIO가 없다.
	        }
	        else
	        {
	            if(++ntpSyncFail > 5)
	            {
	                Debug(console, "[ERROR] wdt> *** NTP TIME-SYNC ....Fail...\n");
	                ntpSyncFail = 0;
	                opr->getClockNTP = 0;
	            }
	        }       
	    }
	          
	    /* ---------------------------------------- */
        /* DNP-HOST 시각동기 ...                    */
        /* ---------------------------------------- */
        if(opr->rtcUpdateFlag)
        {
            opr->rtcUpdateFlag = 0;
            
            // update hwclock & ESIO,dev set updateFlag            
            writeClock( opr->year, opr->month, opr->day, opr->hour, opr->min, opr->sec, opr->week);
            // update rtc
            readClock();
            //update_SystemTime();        // ESIO Time Update...
            vmeTimeSync();                // SIO time update

            /* 상위시스템 시각동기로 시각이 갱신됨 => 백업파일에도 반영  */
            wdt_TimeBackupSave("rtc-update");


#ifdef	VITZRO_FEP_ENABLE
#else
            /* ---------------------------------------- */
            /*  ICCP-HOST : SDP 시각변경 Report         	*/
            /* - 비츠로시스 FEP 의 경우 : SOE 정보를 생성하지 않음 	*/
            /* ---------------------------------------- */
            if((opr->iccpEnbFlag == SET) && (opr->rtcUpdateICCP == SET))
          	{  	
          		opr->rtcUpdateICCP = 0;		// ICCP-HOST Time-Sync 정보 
          		
	            devPtIndex = ICCP_SDP_TIMESYNC;                         // ICCP-Device 포인트 = 69
    	        devPoint   = (POINT_BUF *) devPtBuf[devPtIndex];      	// Device Point 정보
        	    devPoint->status = 1;   // 요청: 1   
            	//devPoint->status = (devPoint->status + 1) & 0x01;   // 요청: 1    
	            gettimeofday(&devPoint->updateTime, NULL);

    	        devEvent.eventCode = ENT_DEVICE_SOE;
        	    devEvent.devNo   = devPtIndex + 1;           // Devie Point,,,1,2
	            devEvent.pointNo = 0;
    	        devEvent.state   = devPoint->status;    // 장치상태 : 이상
        	    devEvent.esioNo  = opr->cpuMode;            // 이벤트 발생주체 : [0] CPU-A, [1] CPU-B
            
	            memcpy( &devEvent.updateTime, &devPoint->updateTime, sizeof(struct timeval));
            
    	        Debug(console, "wdt> ===> ICCP TIME-SYNC REQ [RTC]....state=%d\n", devPoint->status);
        	    Create_Device_SOE(shmPtr, &devEvent);   // ICCP : SDP TIME-SYNC 전송
         	}               
#endif         	
        }


	    /* ---------------------------------------- */
       	/*  제어기 WDT & System-REBOOT Check        */
	    /* ---------------------------------------- */
        if(opr->wdtEnable == SET)
	    {
            if(ioctl(wdtid,PMU_WDT_CTR_CLR,NULL) < 0 )  Debug(console, "wdt> *** WDT ioctl fail\n");
        }      
	    
	    /* ---------------------------------------- */
       	/* 시스템 이벤트  내용 저장 ...             */
   	    /* ---------------------------------------- */
   	    if(opr->eventLogging == RESET)
   	    {   
       		/* -------------------------------- */
           	/* 운영 콘솔 이벤트  내용 저장 ...       */
       	    /* -------------------------------- */
           	oldFront = framHque->front & HISTORY_QUE_MASK;
       	    if(hque->clear == 255)              clearHistoryQ();
    	    else if(oldFront != hque->front)    updateHistoryQ(); 

	    }
	    
	    mpu_LED_Update();               // MPU LED 상태 Update...
	    
        /* -------------------------------- */
        /*  초단위 처리루틴                 */
        /* -------------------------------- */          
        if(oldsec == rtc->sec)   
        {
        	/* RTC 값이 고정되는 현상 방지.... RESTART */
        	if(++rtcCheckTick > 60)
        	{
        		sprintf(buffer, "%s", "wdt> *** RTC Not Changed ... RESTART !");
    			LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    			pause(1000);
    			
    			printf("wdt> *** RTC Fixed... %d  RESTART \n", rtcCheckTick);
    			opr->wdtEnable = RESET;
        		//termExec = 0;	
        	}
        		
        	continue;
        }
        
        oldsec = rtc->sec;
		rtcCheckTick = 0;
		
		/* -------------------------------- */
	    /* VMEBUS 상의 기능모듈부 동작 Check...		*/
	    /* -------------------------------- */
		vmeModule_Check();              // ESIO RACK 실장정보 Check...       
		
		/* -------------------------------- */
		/* ICCP-SDP-RESET 명령 처리...			*/
		/* -------------------------------- */
		if((opr->iccpResetENB != 0) && (opr->iccpResetFlag == SET))
		{
			pause(1000);
			opr->wdtEnable = RESET;
			opr->iccpResetFlag = RESET;
			
			printf("\n=============================================\n");
    		printf("    rcv>> SDP-RESET Control... REBOOT..!       \n");
    		printf("=============================================\n");
    
            // 2026-10-07 오후 3:27:56
            system("reboot");
    
		}
	
		/* -------------------------------- */
		/* 연산포인트 계산기능 수행 : 지정 주기별 연산 		*/
		/* -------------------------------- */
		if(opr->calCalcFlag)
		{			
        	if((oldsec % opr->calCalcFlag) == 0)   sdp_Calculate_Point();
        }
        
        /* -------------------------------- */
        /* MPU 상태정보 Update...           */
        /* -------------------------------- */
        update_MPU_status();


//#ifdef  VME_NETWORK_INITIAL
        WDT_ESIO_Config();              // ESIO Network 구성정보
//#endif
        
        /* ------------------------------------------------ */
        /*  MFCU-PROCESS 동작상태 감시                      */
        /* ------------------------------------------------ */                           
		CheckProcess();					// 프로세스 동작 상태 감시
		
		/* ------------------------------------------------ */
		/*  데이터베이스 변경시... 재구성 로드              */
		/* ------------------------------------------------ */
		sdp_DBChange_Check();
		
        /* -------------------------------- */
        /*  시간단위 처리루틴               */
        /* -------------------------------- */
        if(oldhour == rtc->hour)   continue;
        oldhour = rtc->hour;

        /* 매시간마다 주기적 Time Sync ... */
        // set updateFlag for ESIO & DEV
        update_SystemTime();

        /* ---------------------------------------------------- */
        /*  RTC 시각 백업 : 마지막 저장 후 12시간 경과 시       */
        /*  - 보통은 상위시스템이 1시간 주기로 rtcUpdateFlag 를 */
        /*    set 하므로 여기까지 오지 않는다.                  */
        /*    (저장할 때마다 timeBackupHourCnt 가 0 이 됨)       */
        /*  - 상위시스템이 없는 환경에서 장시간 aging 을 하기    */
        /*    위한 경로.                                        */
        /*  - hour 변화 횟수만 세므로 정확히 12시간은 아니다     */
        /*    (부팅 시각에 따라 최초 1회는 11~12시간).           */
        /* ---------------------------------------------------- */
        if(++timeBackupHourCnt >= WDT_TIME_BACKUP_HOURS)
        {
            wdt_TimeBackupSave("12h-period");
        }
	}

    /* -------------------------------------------- */
    /* CPU 동작 중지 표시... Shutdown/live OFF      */
    /* -------------------------------------------- */
    linkCfg->cpuControl = 0x03;
    write_led(devOutput.id, linkCfg->cpuControl);  
    
    printf("\n============================\n");
    printf("    *** WDT Disable...!       \n");
    printf("============================\n");
    
    if(ioctl(wdtid,PMU_WDT_CTR_OFF, NULL) < 0 )  printf("wdt> *** ioctl fail\n");
    
	/*  작업 환경을 정리한다    */
	ClearEnv();

    return (0);	
}
