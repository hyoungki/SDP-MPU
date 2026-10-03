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
#include    "extfunc.h"
// hkkim
#include    "execinfo.h"

SHM_DESC	    shmDesc = {-1, SHM_KEY, sizeof(SHM_MEMORY), NULL};
SHM_MEMORY	    *shmPtr = NULL;
int             termExec;

TASK_INFO	    *taskPtr= NULL;

RTC             *rtc    = NULL;
OPR_MSG         *opr    = NULL;
CONSOLE_INFO	*console= NULL;

LINK_MSG        *linkCfg= NULL;                 // CPU 이중화 구조체
SCU_MSG         *scuCfg = NULL;                 // 이중화 절체장치(SCU) 구조체

MPU_CONFIG      *mpuCFG  = NULL;                 // MPU Config
ESIO_CONFIG     *esioCFG[MAX_ESIO];             // ESIO 장치 Config

ICCP_DCB        *iccpDCB = NULL;                // ICCP-HOST 구조체
ICCP_CONFIG     *iccpCFG = NULL;                // ICCP-HOST 구조체

//RTU_CONFIG      *rtuDCB = NULL;                 // 하부 RTU 운영 구조체

HOST_DCB        *hostDCB[MAX_HOST];             // HOST 관련 구조체 : ICCP, DNP, HARRIS, LANDIS...
POINT_BUF       *devPtBuf[MAX_DEV_POINT];       // SDP 포인트 Config 정보
SCAN_CONFIG     *scanCFG[MAX_SCAN_PORT];        // 하위계전기 SCAN Config
SDP_DEVICE      *deviceCFG[MAX_DEVICE];         // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)
	 
RTU_DATABASE    *rtudb  = NULL;                 // SDP 데이터베이스
// hkkim
ESIO_RTU_DATABASE  esio_rtudb ; 

HISTORY_QUE     *hque   = NULL;                 // CONSOLE 용 이벤트

MPU_SOE_QUEUE   *mpuSOE = NULL;                 // MPU SOE Buffer
//MPU_COS_QUEUE   *mpuCOS = NULL;                 // MPU COS Buffer

PDPDTIME_DATE_TIME      dnpRTC;
PDPDTIME_MS_SINCE_70    dnpRTCInfo;

byte    bitcode[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

int     ThreadActive[MAX_ESIO];

int     devHero;
int     devExio;

byte    linkStatus = 0, cosSts = 0;
int     initCheck  = 0;
int     linkInitial;

static  int     stsRunTick, anaRunTick;

/* ---------------------------------------- */
/*  THREAD 관련 변수 정의                   */
/* ---------------------------------------- */
THREAD_ENTRY    ESIO_THREAD[MAX_ESIO];              // ESIO Thread 구조체 TASK


extern  void    *scanThread_ESIO(int *arg);                 // SCAN - ESIO TASK
extern  void    *scan_dnp_thread_TCP(int  *arg);             // SCAN - RTU 연계 DNP

extern  void    deviceComUpdate();
extern  void    systemStsUpdate();

//extern  int	    iccpShmInit (ICCP_DCB *iccpShm);
//extern  void    iccpShmEnd (void);


/* *************************************************************************************
*	FUNCTION : sigHandler()
* **************************************************************************************/
void	SigHandler( int sig )
{
	char    buffer[256];
    if(opr->wdtDebug)   Debug(console,"scan> ... signal generated (%2d)...!\n", sig);
    pause(100);
    
	switch(sig)
	{
    case SIGTERM:
        termExec = 0;
        if(opr->wdtDebug)   Debug(console,"scan> ... signal [SIGTERM] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scan> ... signal [SIGTERM] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   	
        pause(1000);     
        break;
	        
	case SIGBUS : 
	    termExec = 0;
        if(opr->wdtDebug)   Debug(console,"scan> ... signal [SIGBUS] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scan> ... signal [SIGBUS] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));  	
        pause(1000);     	    
	    break;
		    
	case SIGSEGV: 
	    termExec = 0;
        if(opr->wdtDebug)   Debug(console,"scan> ... signal [SIGSEGV] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scan> ... signal [SIGSEGV] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));  		
        pause(1000);     	    
	    break;
		    
	case SIGPIPE: 
        if(opr->wdtDebug)   Debug(console,"scan> ... signal [SIGPIPE] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scan> ... signal [SIGPIPE] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));  			
        pause(1000);     	    
		break;

	default : 
	    if(opr->wdtDebug)   Debug(console,"scan> ... signal[%2d] generated ...!\n", sig);
	    /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scan> ... signal[%2d] generated ...!", sig);
		break;
	}
    
}


// 프로그램 시작 시 베이스 주소 저장
static unsigned long base_address = 0;
void init_base_address(void) {
    FILE *fp = fopen("/proc/self/maps", "r");
    if (fp) {
        char line[256];
        if (fgets(line, sizeof(line), fp)) {
            // 첫 번째 LOAD 영역의 시작 주소
            sscanf(line, "%lx", &base_address);
        }
        fclose(fp);
    }
}



static void sigsegv_handler(int sig, siginfo_t *si, void *unused)
{

#if 0
    void *array[10];
    int size;
	char    buffer[256];
 
    termExec = 0;

    
    fprintf(stderr, "\n=== SCAN SIGSEGV 발생! ===\n");
    fprintf(stderr, "Fault address: %p\n", si->si_addr);
    
    fprintf(stderr, "\nBacktrace (크래시 시점의 호출 스택):\n");
    size = backtrace(array, 10);
    backtrace_symbols_fd(array, size, STDERR_FILENO);

#else 

    void *array[10];
    int size;
	char    buffer[256];
    int i ;
    ucontext_t *uc = (ucontext_t *)unused;

    unsigned long offset_pc ;
    unsigned long offset_lr ;
    
    termExec = 0;


    
    fprintf(stderr, "\n=== SCAN SIGSEGV 발생! ===\n");
    fprintf(stderr, "Fault address: %p\n", si->si_addr);
    
    fprintf(stderr, "PC: 0x%08lx\n", uc->uc_mcontext.arm_pc);
    fprintf(stderr, "LR: 0x%08lx\n", uc->uc_mcontext.arm_lr);
    fprintf(stderr, "SP: 0x%08lx\n", uc->uc_mcontext.arm_sp);    
    
    // 오프셋 계산
     offset_pc = uc->uc_mcontext.arm_pc - base_address;
     offset_lr = uc->uc_mcontext.arm_lr - base_address;
    
    fprintf(stderr, "\nTo decode base %x:\n",base_address);
    fprintf(stderr, "  addr2line -e YOUR_PROGRAM -f -p -C 0x%lx\n", offset_pc);
    fprintf(stderr, "  addr2line -e YOUR_PROGRAM  -p -C 0x%lx\n", offset_pc);
    fprintf(stderr, "  addr2line -e YOUR_PROGRAM -f -p -C 0x%lx\n", offset_lr);
        
    
        
    fprintf(stderr, "\nBacktrace (크래시 시점의 호출 스택):\n");
    size = backtrace(array, 10);
    // 각 주소를 16진수로 출력
    for (int i = 0; i < size; i++) {
        fprintf(stderr, "  [%d] %p\n", i, array[i]);
    }


#endif 

//    if(opr->wdtDebug)   Debug(console,"scan> ... signal generated (%2d)...!\n", sig);
//    pause(100);
    
	switch(sig)
	{
    case SIGTERM:
        termExec = 0;
        if(opr->wdtDebug)   Debug(console,"scan> ... signal [SIGTERM] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scan> ... signal [SIGTERM] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   	
        pause(1000);     
        break;
	        
	case SIGBUS : 
	    termExec = 0;
        if(opr->wdtDebug)   Debug(console,"scan> ... signal [SIGBUS] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scan> ... signal [SIGBUS] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));  	
        pause(1000);     	    
	    break;
		    
	case SIGSEGV: 
	    termExec = 0;
        if(opr->wdtDebug)   Debug(console,"scan> ... signal [SIGSEGV] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scan> ... signal [SIGSEGV] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));  		
        pause(1000);     	    
	    break;
		    
	case SIGPIPE: 
        if(opr->wdtDebug)   Debug(console,"scan> ... signal [SIGPIPE] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scan> ... signal [SIGPIPE] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));  			
        pause(1000);     	    
		break;

	default : 
	    if(opr->wdtDebug)   Debug(console,"scan> ... signal[%2d] generated ...!\n", sig);
	    /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scan> ... signal[%2d] generated ...!", sig);
		break;
	}


    
    _exit(1);
}


//
// 모듈:	ClearEnvironment()
//
void ClearEnv(void)
{
// 2026-05-19 오후 1:17:18
	UpdateProcessInfo(taskPtr, 0, getpid(), 0);    
    
    iccpShmEnd ();
    
	// 공유 메모리 설정 해제
	if (shmPtr != NULL)
		ShmDetach(&shmDesc);

	// 프로세스 정보를 정리한다
	//UpdateProcessInfo(taskPtr, 0, getpid(), 0);
			
}
					    

void copy_mpu_config(     DB_ESIO_MPU_CONFIG *dest,    DB_MPU_CONFIG *src)
{

        memcpy( &dest->sdpNameStr[0],  (byte *) &src->sdpNameStr[0], 20);                
        memcpy( (byte *)&dest->dualMpu,  (byte *) &src->dualMpu, 16);    
        // 배열의 이름은 주소...
        memcpy( (byte *)dest->master_netCfg,  (byte *) src->master_netCfg, sizeof(DB_MNET_ENTRY )*3  );  
        memcpy( (byte *)dest->slave_netCfg,  (byte *)  src->slave_netCfg, sizeof(DB_MNET_ENTRY )*3  );        

#if 0
        MPU_NET_ENTRY   *mNet1 ;  
     printf("sdpNameStr[%s]\r\n",dest->sdpNameStr);
           
    mNet1 = (MPU_NET_ENTRY *) &dest->master_netCfg[0];
    printf("[*] Master Network#1 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet1->useFlag, mNet1->ipAddr, mNet1->gwAddr, mNet1->subMask);

    mNet1 = (MPU_NET_ENTRY *) &dest->master_netCfg[1];
    printf("[*] Master Network#2 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet1->useFlag, mNet1->ipAddr, mNet1->gwAddr, mNet1->subMask);
    
    mNet1 = (MPU_NET_ENTRY *) &dest->master_netCfg[2];
    printf("[*] Master Network#3 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet1->useFlag, mNet1->ipAddr, mNet1->gwAddr, mNet1->subMask);
    
    
    mNet1 = (MPU_NET_ENTRY *) &dest->slave_netCfg[0];
    printf("[*] SLAVE Network#1 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet1->useFlag, mNet1->ipAddr, mNet1->gwAddr, mNet1->subMask);


    mNet1 = (MPU_NET_ENTRY *) &dest->slave_netCfg[1];
    printf("[*] SLAVE Network#2 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet1->useFlag, mNet1->ipAddr, mNet1->gwAddr, mNet1->subMask);
    
    mNet1 = (MPU_NET_ENTRY *) &dest->slave_netCfg[2];
    printf("[*] SLAVE Network#3 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet1->useFlag, mNet1->ipAddr, mNet1->gwAddr, mNet1->subMask);    

#endif 
}


// copy_common_fields ( &esio_rtudb, rtudb)
void copy_rtudb_for_esio(  ESIO_RTU_DATABASE *dest,   RTU_DATABASE  *src)
{    
    // mpu config 복사

    //    struct DB_ESIO_MPU_CONFIG mpu_config2;



           
    // 이 부분의 기존에 있는 부분이고..       
    // 각 필드를 개별적으로 복사
    memcpy(dest->esioConfig, src->esioConfig, sizeof(src->esioConfig));
    memcpy(dest->hostCfg, src->hostCfg, sizeof(src->hostCfg));
    memcpy(&dest->iccpConfig, &src->iccpConfig, sizeof(src->iccpConfig));
    memcpy(dest->portdb, src->portdb, sizeof(src->portdb));
    
    dest->chassisNum = src->chassisNum;
    memcpy(dest->chassis, src->chassis, sizeof(src->chassis));
    
    memcpy(dest->modbusProfile, src->modbusProfile, sizeof(src->modbusProfile));
    memcpy(dest->scanConfig, src->scanConfig, sizeof(src->scanConfig));
    memcpy(dest->deviceConfig, src->deviceConfig, sizeof(src->deviceConfig));
    memcpy(dest->pointBuf, src->pointBuf, sizeof(src->pointBuf));
    memcpy(dest->calPointBuf, src->calPointBuf, sizeof(src->calPointBuf));
    memcpy(dest->chksum, src->chksum, sizeof(src->chksum));
    
    
    // 이제 mpu Config를 복사하자.
    copy_mpu_config( &dest->esio_mpuConfig, &src->mpuConfig) ;
        

}


//
// 모듈:	InitEnvironment()
//
int InitEnv( void)
{
    int     i;
    
	// 공유 메모리 상태 확인
	if (ShmCheck(&shmDesc) < 0)
	{
		printf("scan> *ERR_공유 메모리 상태 이상\n");
		return(0);
	}
	
	// 포인터 변수 초기화
	shmPtr = (SHM_MEMORY *) shmDesc.address;
	taskPtr = (TASK_INFO *) &shmPtr->taskInfo[SCAN_PROCESS];

	rtc     = (RTC *)       &shmPtr->rtc;
	opr     = (OPR_MSG *)   &shmPtr->opr_msg;
    
    linkCfg = (LINK_MSG *)  &shmPtr->link_msg;                  // MPU 이중화 관련 구조체
    scuCfg 	= (SCU_MSG *)   &shmPtr->scu_msg;                   // 이중화절체장치(SCU) 구조체
    
	rtudb   = (RTU_DATABASE *) &shmPtr->rtuDatabase;
    console = (CONSOLE_INFO *) &shmPtr->console; 
    
	hque    = (HISTORY_QUE *)  &shmPtr->localHistoryQ; 

    mpuSOE  = (MPU_SOE_QUEUE *) &shmPtr->mpuSoeQueue;           // MPU SOE Buffer
    //mpuCOS  = (MPU_COS_QUEUE *) &shmPtr->mpuCosQueue;           // MPU COS Buffer
    
    mpuCFG  = (MPU_CONFIG *) &shmPtr->mpuConfig;                // MPU Network 구성정보
    
    iccpDCB  = (ICCP_DCB *) &shmPtr->iccpDCB;                   // ICCP_DCB 
    iccpCFG  = (ICCP_CONFIG *) &shmPtr->iccpDCB.config;         // MPU Network 구성정보
    
    //rtuDCB  = (RTU_CONFIG *) &shmPtr->rtuConfig;               // 하부 RTU 운영구조체
    
    /* ------------------------------------ */
    /*  ICCP 공유메모리 초기화              */
    /* ------------------------------------ */
    iccpShmInit (iccpDCB);
    
    /* ------------------------------------ */
    /* ESIO CFG : ESIO 구조체 (MMAX 5)      */
    /* ------------------------------------ */ 
    for(i=0; i< MAX_ESIO; i++)      esioCFG[i] = (ESIO_CONFIG *) &shmPtr->esioConfig[i];
    for(i=0; i< MAX_HOST; i++)      hostDCB[i] = (HOST_DCB *) &shmPtr->hostDCB[i];
    for(i=0; i< MAX_SCAN_PORT; i++) scanCFG[i] = (SCAN_CONFIG *) &shmPtr->scanCFG[i];
    for(i=0; i< MAX_DEVICE; i++)    deviceCFG[i] = (SDP_DEVICE *) &shmPtr->deviceCFG[i];
	for(i=0; i< MAX_DEV_POINT; i++) devPtBuf[i] = (POINT_BUF *) &shmPtr->devPtBuf[i];
    

	
    /* ------------------------------------------------------------ */
	/* 프로세스 정보를 초기화한다                                   */
	/* HOST Task 는 TCPIP 전송대기를 고려하여 WDT 기능을 삭제함...  */
	/* ------------------------------------------------------------ */
	taskPtr->initial    = 1;
	taskPtr->wdtEnable  = 1;            
	
	/* ------------------------------------------------ */
	/*  PROCESS 호출시... Priority 지정 (-19 ~ 20)      */
	/* ------------------------------------------------ */
	if( nice(NICE_SCAN) < 0)
	    printf("SCAN< fail to nice\r\n");
	
	
	UpdateProcessInfo(taskPtr, 1, getpid(), 1);
	
	return(1);
}


/*
*   SCAN-TASK 
*/
int esioTask_Call()
{
    int     esioid;      
    int	    state;
    int     index;
    
    ESIO_CONFIG *esio;
    
    /* ------------------------------------ */
    /*  SCAN THREAD 초기화 및 실행          */ 
    /* ------------------------------------ */       
    for(esioid=0; esioid < MAX_ESIO; esioid++)
    {
        /* esioid == 0  : SIO Board */
        /* esioid == 1  : 전력CU */
        /* esioid == 2  : 원격진단 */
        /* esioid == 3  : 전력품질 */
        /* esioid == 4  : IEC 61850 */
        /* esioid == 5  : RTU */

        /* SIO 통신 Thread....SKIP */
        if(esioid == 0)
        {
            ESIO_THREAD[esioid].options = 0;	
	        ESIO_THREAD[esioid].arg     = 0;
            continue;
        }
             
        esio = (ESIO_CONFIG *) esioCFG[esioid];
        
        ESIO_THREAD[esioid].tid        = 0;
        sprintf(ESIO_THREAD[esioid].name, "%s%d","ESIO", esioid+1);
        
        ESIO_THREAD[esioid].priority   = 0;
        ESIO_THREAD[esioid].options    = 0;       // 비실행 Flag
    
        /* ------------------------------------ */
		/* 	ACTIVE 상태에 따라서 Thread 실행 	*/
		/* ------------------------------------ */
		if(esio->useFlag == RESET)	continue;

		opr->scanWDT[esioid] = 0;
		
        esio->vmeNetConfig = SET;       // VME Config ...
        
        ESIO_THREAD[esioid].entryPt = scanThread_ESIO;     		// TCPIP 방식 MODULE-THREAD
        ESIO_THREAD[esioid].options = SET;	
	    ESIO_THREAD[esioid].arg     = 0;
    
      	index = esioid;      // Thread Index #1, 0,1,2...
      	
        state = pthread_create( &ESIO_THREAD[esioid].tid, NULL, ESIO_THREAD[esioid].entryPt, &index);
       	if(state != 0)
        {
	        printf("scan> *Thread# [%s] Create Error ...!\n", ESIO_THREAD[esioid].name);
    	    //exit(1);
            termExec = 0;
            return (0);
        }
	    pause(100);
    }
    return (0);
}

/*
*
*/
int display_ESIO_sts()
{
    int esioid;
    
    ESIO_CONFIG *esio;
    
    Debug(console, "\n--------------------------------------------------------\n");
    Debug(console, " No. SCAN  ONL  [ SND  RCV - PRE ]  CON  TARGET \n");
    for(esioid = 0; esioid < MAX_ESIO; esioid++)
    {
        esio = (ESIO_CONFIG *) esioCFG[esioid];
        
        Debug(console, "  %d.   %d    %d   [%4d %4d - %4d]   %d  %s \n", esioid+1, esio->useFlag, esio->online, 
            esio->sndCount, esio->rcvCount, esio->preRcvCount, esio->connectStatus, esio->targetAddr );
    }
    return (0);
}


/*
*   사용자 데이터 입력 시험 : ANALOG
*/
int testAnalogDATA( )
{
    float   fdata;
    int     devid, point;
    // hkkim
    unsigned int  U32 ; 
                            
    //Debug(console,"==> Test Analog Data Update....[%2d] !\n", rtc->sec);

 //   printf("testAnalogDATA in\r\n");
    
    // cli가 설정한 값을 가져옴.
    devid = opr->testDevid;
    point = opr->testPoint;

    // cli에서 내가 넣고,, 여기서 내가 빼니 문제는 보고 할때
    fdata = opr->testAIData;

    memcpy(&fdata, &opr->testAIData ,4);
    
    opr->testAnalogFLAG = RESET;


    // 지금 little ..
    memcpy(&U32, &fdata ,4); // U32 little ㅣ
    U32 = htonl(U32);    
    


    /* ------------------------------------ */
    /* DEVICE/POINT Assign into Buffer ...  */
    /* ------------------------------------ */
 //   printf("storeAI_FLOAT_MPU dev[%d] pt[%d] value[%f]\r\n", devid, point, fdata) ;
// hkkim storeAI_FLOAT_MPU 여기서 넣을 때  U32는 big endian
        storeAI_FLOAT_MPU(shmPtr, devid, point, fdata, (byte *)&U32);
//        storeAI_FLOAT_MPU(shmPtr, devid, point, fdata, (byte *)&fdata);

//    printf("testAnalogDATA out\r\n");
                
    return (0); 
}


/*
*   사용자 데이터 입력 시험 : STATUS
*/
int testStatusDATA()
{
    int     i;
    int     devid, point, state;
    int     millisecond;
    
    time_t  tm;
    struct  tm  cTime;
    
    MPU_SOEQ_ENTRY event;
    
    
    devid = opr->testDevid;
    point = opr->testPoint;
    state = opr->testState;
    
    opr->testStatusFLAG = RESET;
    
    for(i=0; i < 1; i++)
    { 
        /* ---------------------------------------- */
        /* MPU 전송 - SOE 이벤트 생성               */
        /* ---------------------------------------- */
        event.eventCode = ENT_SOE;                          // SOE 포인트 지정
        event.devNo     = devid + 1;                        // 계전기 번호 : 1 ~ 64
        event.pointNo   = point + 1;                        // 포인트 번호 : 1 ~ 1024
        event.state     = state;                            // 상태 
        event.esioNo    = opr->cpuMode;                     // 이벤트 발생주체 : [0] CPU-A, [1] CPU-B
        
        time(&tm);
        //localtm = localtime(&tm);
        localtime_r (&tm, &cTime);
                
        /* SOE 시각정보-timeval 정보 생성 */    
        gettimeofday(&event.updateTime, NULL);
        millisecond = (event.updateTime.tv_usec / 1000) % 1000;
    
        Debug(console,">> TEST-SOE: dev=%2d, point=%2d, state=%2x ... [%4d/%2d/%2d %02d:%02d:%02d - %d ms]\n", devid+1, point+1, state,
            cTime.tm_year + 1900, cTime.tm_mon + 1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, millisecond);
                
        /* -------------------------------- */
        /*  SOE 정보 저장 (for TEST)        */
        /* -------------------------------- */
        store_MPU_SOE(shmPtr, &event);
    }
                   
    /* -------------------------------- */
    /*  COS 정보 저장 (for TEST)        */
    /* -------------------------------- */
    //cosState = (state >> 7) & 0x01;
    //storeCOS_CU_DNP( devid, point, state);
    
    return (0); 
}                                    

/*
*   사용자 데이터 입력 시험 : STATUS
*/
int testDeviceDATA()
{
    int     devid, point, state;
    int     millisecond;
    
    time_t  tm;
    struct  tm  cTime;
    
    MPU_SOEQ_ENTRY event;
    
    
    devid = opr->testDevid;
    point = opr->testPoint;
    state = opr->testState;
    
    opr->testDeviceFLAG = RESET;
    
    /* ---------------------------------------- */
    /* MPU 전송 - SOE 이벤트 생성               */
    /* ---------------------------------------- */
    if(state == 1)  event.eventCode = ENT_DEV_ONLINE;     
    else            event.eventCode = ENT_DEV_OFFLINE;             
            
    event.devNo     = devid + 1;                        // 계전기 번호 : 1 ~ 64
    event.pointNo   = point + 1;                        // 포인트 번호 : 1 ~ 1024
    event.state     = state;                            // 상태 

    time(&tm);
    //localtm = localtime(&tm);
    localtime_r (&tm, &cTime);
                
    /* SOE 시각정보-timeval 정보 생성 */    
    gettimeofday(&event.updateTime, NULL);
    millisecond = (event.updateTime.tv_usec / 1000) % 1000;
    
    Debug(console,">> TEST-SOE: dev=%2d, point=%2d, state=%2x[%s] ... [%4d/%2d/%2d %02d:%02d:%02d - %d ms]\n", devid+1, point+1, state,
        (state == 1 ) ? "ONLINE" :"OFFLINE",
        cTime.tm_year + 1900, cTime.tm_mon + 1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, millisecond);
                
    /* -------------------------------- */
    /*  SOE 정보 저장 (for TEST)        */
    /* -------------------------------- */
    if(state == 1)  set_Online_Device(shmPtr, &event);
    else            set_Offline_Device(shmPtr, &event);    

    
    return (0); 
}                     

//static char scanModType[10][8]   = {{"*NOT "}, {" SIO "},{"ESIO1"}, {"ESIO2"}, {"ESIO3"}, {"ESIO4"}, {" RTU"}, {" MPU#1"}, {" MPU#2"}};
    
/*
* ============================================================
* ICCP - 제어명령 처리...
* ============================================================
*/
int check_ICCP_Control(ICCP_CONTROL_DATA *iccpCntr)
{
    int devNo, devPt, cntTCF, mpuChange;
    struct  timeval ctime;
    //char    buffer[256];
    
    //SDP_DEVICE      *dev;
    //ESIO_CONFIG     *esio;          // ESIO 구조체    

    //if(opr->soeDebug)
    //Debug(console, "> ICCP RUN State ... %d, FAB=%d\n", iccpDCB->assocStatus, iccpDCB->activeArIndex);
    
    if(opr->runMode == LOCAL_SLAVE)	return(0);		
        
    /* ---------------------------------------------------- */
    /*  ICCP 제어정보 추출 ...                              */
    /* ---------------------------------------------------- */        
    if(iccpCntr->cntrFlag != SET)   return (0);
        
    devNo = iccpCntr->cntrDev;          // ICCP Control 계전기 번호 ,           1 ~ 64
    devPt = iccpCntr->cntrPoint;        // ICCP Control 계전기-포인트 번호 ,    1 ~ 1024
    cntTCF= iccpCntr->cntrState;        // ICCP Control 상태 ,    1 : TRIP, 2: CLOSE
    
    mpuChange = iccpCntr->cntrType;		// 0: Point 제어, 1: MPU 절체
    
    iccpCntr->cntrDev   = 0;
    iccpCntr->cntrPoint = 0;
    iccpCntr->cntrState = 0;
    iccpCntr->cntrType  = 0;
    
    iccpCntr->cntrFlag  = 0;

    /* ---------------------------------------------------- */
    /*  ICCP 제어 : CPU 절체, 포인트 제어...                */
    /* ---------------------------------------------------- */
    if(mpuChange)
    {   
        if((opr->dualCpuSts == SET) && (scuCfg->remoteMode == AUTO_MODE))
        {  
            logEvent_MPU(shmPtr, ENT_ICCP_CNTR, devNo, devPt, cntTCF, opr->cpuMode, NULL);  
            logEvent_MPU(shmPtr, ENT_CHANGE_CPU, CPU_CHG_ICCP, 0, 0, opr->cpuMode, NULL);   
         
            //if(opr->soeDebug)
            Debug(console, "\n========================================\n");
            Debug(console, "scan> rcv ICCP-CNTR: *** MPU Change... !\n");
            Debug(console, "========================================\n");
            
            opr->cpuChange = SET;
            
        }
        else
        {
            //if(opr->soeDebug)
            Debug(console, "\n========================================\n");
            Debug(console, "scan> rcv ICCP-CNTR: *** MPU Change FAIL... dual=%d, auto=%d ... !\n", opr->dualCpuSts, scuCfg->remoteMode);
            Debug(console, "========================================\n");
        }      
    }
    else
    {
        //if(opr->soeDebug)
        {   
            Debug(console, "\n========================================\n");   
            Debug(console, "scan> rcv ICCP-CNTR: Dev=%d, Point=%d, TCF[TRIP=1, CLOSE=2] =%2x...\n", devNo, devPt, cntTCF);
            Debug(console, "========================================\n");
        }
        
        /* ------------------------------------------------ */
        /*  ICCP 수신 제어정보 유효성 Check...              */
        /* ------------------------------------------------ */    
        if((devNo <= 0) || (devNo > MAX_DEVICE))
        {
            Debug(console, "rcv> *ICCP Control Fail....*Invalid DEV-ID = %d\n", devNo);
            return (0);
        }     
    
        if((devPt <= 0) || (devPt > MAX_DEV_DO_POINT))
        {
            Debug(console, "rcv> *ICCP Control Fail....*Invalid POINT-NO = %d\n", devPt);
            return (0);
        }       
    
    	/* 포인트 제어상태 (TRIP/CLOSE) 비교 */
        if((cntTCF != 1) && (cntTCF != 2))
        {
            Debug(console, "rcv> *ICCP Control Fail....*Invalid Control TYPE[1/2] = %d\n", cntTCF);
            return (0);
        }   
      
        /* ------------------------------------ */
        /*  ICCP 포인트 제어정보 처리...        */
        /* ------------------------------------ */
        gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
        logEvent_MPU(shmPtr, ENT_ICCP_CNTR, devNo, devPt, cntTCF, opr->cpuMode, &ctime);   
        
        /* ------------------------------------ */
        /* ESIO 제어정보 연계...                */
        /* ------------------------------------ */
        controlInfo_MPU( shmPtr, devNo, devPt, cntTCF, PASS_ICCP_CNTR, &ctime);    
    }
                
    return (0);    
}


/*
*
*/
void    set_StatusDump()
{
    int i;
    SDP_DEVICE *dev;
    
    for(i=0; i < MAX_DEVICE; i++)
    {
        dev = (SDP_DEVICE *) deviceCFG[i];
        
        dev->statusDump = SET;
    }           
}

/*
* RTU : 계전기 상태정보 Update....
*/
int update_Device_Dump()
{
    int         ioid;
    SDP_DEVICE  *dev;
    
    stsRunTick++;   
    anaRunTick++;
    
    if(opr->runMode == LOCAL_SLAVE)	return(0);	
    	
    /* ---------------------------------------- */
    /* 계전기 상태정보 Update Flag...           */
    /* ---------------------------------------- */
    if(stsRunTick >= opr->stsDumpPeriod)
    {
        stsRunTick = 0;
        for(ioid=0; ioid< MAX_DEVICE; ioid++)
        {
            dev = (SDP_DEVICE *) deviceCFG[ioid];
            dev->statusDump = SET;
        }
    }
    
    /* ---------------------------------------- */
    /* 계전기 ANALOG 정보 Update Flag...        */
    /* ---------------------------------------- */
    if(anaRunTick >= opr->anaDumpPeriod)
    {
        anaRunTick = 0;
        for(ioid=0; ioid< MAX_DEVICE; ioid++)
        {
            dev = (SDP_DEVICE *) deviceCFG[ioid];
            if(dev->devAiPoint)		dev->analogDump = SET;
        }
    }
    
    return (1);         
}

void install_sighander(void)
{

init_base_address();
#if 0
	/* Signal Handler initial */
	signal( SIGBUS,  SigHandler);
	signal( SIGSEGV, SigHandler);
	signal( SIGTERM, SigHandler);	
	signal( SIGPIPE, SigHandler);
	
#else 	
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = sigsegv_handler;
    sigaction(SIGBUS, &sa, NULL);    

    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = sigsegv_handler;
    sigaction(SIGSEGV, &sa, NULL); 
    
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = sigsegv_handler;
    sigaction(SIGTERM, &sa, NULL); 
    
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = sigsegv_handler;
    sigaction(SIGPIPE, &sa, NULL);         

#endif     
}


int dbFileWriteESIO(OPR_MSG *opr, ESIO_RTU_DATABASE *localDB)
{
	//char	*buffer;                   
	int     dbsize;
	word	romsum;

    
#if  0
//#ifdef	ROM_VERSION
    FILE_DESC	romdbFile1 = {NULL, FIO_CREATE, ROMDB_STRING };
#else



    /* -------------------------------------------- */
    /*  MPU 동작모드에 따른 DB 화일 선택            */
    /* -------------------------------------------- */	
    FILE_DESC	romdbFile1;
    
    printf("Making  ESIO DB\r\n");
        
    if(opr->cpuMode == MPU_A)
    {
        romdbFile1.id   = NULL;
        romdbFile1.mode = FIO_NORMAL;   
        sprintf( romdbFile1.name, "%s", "/mnt/bin/SDP-ESIO-DB"); 
        
    }
    else if(opr->cpuMode == MPU_B)
    {
        romdbFile1.id   = NULL;
        romdbFile1.mode = FIO_NORMAL;  
        sprintf( romdbFile1.name, "%s", "/mnt/bin/SDP-ESIO-DB"); 
 //       sprintf( romdbFile1.name, "ESIO-%s", ROMDB_STRING2);
    }
    else
    {
        printf("[DB-WRITE] *** Invalid CPU-MODE = %d \n", opr->cpuMode);
        return (-1);
    }       
#endif

    
    /* 데이터베이스 저장 ... */
    romsum = gensum((byte *)localDB, sizeof(ESIO_RTU_DATABASE) - 2);
    localDB->chksum[0] = romsum;
    localDB->chksum[1] = romsum >> 8;
    
    printf("SCAN> ESIO_RTU_DATABASE CHKSIM [%02X:%02X]-[%04X]\r\n",    localDB->chksum[0] ,  localDB->chksum[1] ,romsum );
    
	dbsize = sizeof(ESIO_RTU_DATABASE);
	
	/* Database File #1 Open */
	if (FileOpen(&romdbFile1) < 0)
	{
		printf("[WR] %s Database File Open Error...!\n", romdbFile1.name);
		//romDBFail();
		/* Database File Create.... */
		//FileOpen(&romdbFile2);
		return (-1);
	}
	
    printf("[WR] %s Database File Open OK...(size=%d) !\n", romdbFile1.name, dbsize);   

    /* 내부변수의 내용을 Database화일로 Write 한다. */
    if(FileWrite(&romdbFile1, (char *) localDB, dbsize) < 0)
    {         
		return (-1);        
    }

    FileClose(&romdbFile1);  
    
    return (1);
    
}



/* ------------------------------------------------------- */
/*  SCAN MAIN 프로그램 Start Routine ....             */
/* ------------------------------------------------------- */
int	main(int argc, char **argv)
{
	int		i;
	int		oldsec;
	char	buffer[256];
	
	ICCP_CONTROL_DATA   *iccpCntr;
	
	// 작업 환경을 초기화한다
	termExec = InitEnv();
	
	taskPtr->wdtCount = 0;
		

    install_sighander();    

//2026-05-22 오후 6:31:47
	//pause(1000);// 밑으로 이동
	pause(1000);
	
	/* LOG File 저장 */
	sprintf(buffer, " %s SCAN-MAIN PROCESS Activated ... !", TARGET_NAME);
    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    
    printf(" %s SCAN PROCESS Activated ... !\n", TARGET_NAME);
    

//    printf("%s Updating esio_rtudb\r\n", TARGET_NAME);

    copy_rtudb_for_esio( &esio_rtudb, rtudb) ;
//2026-05-22 오후 6:31:44
    pause(1000);    
    //dbFileWrite(opr, rtudb);
    dbFileWriteESIO(opr, &esio_rtudb);

    pause(1000);
    
    
    
    
    
    opr->esioRestart  = 0;
    opr->allDevOnline = SET;        // 전체계전기 통신상태, 1 : Online, 0: Offline
    opr->scanInitial  = 30;
    
    //opr->testWDTFlag  = 0;
    
    /* -------------------------------------------- */
    /*  해당 ESIO-SCAN Thread 호출...               */
    /* -------------------------------------------- */
    esioTask_Call();
    
    oldsec  = rtc->sec;
    
    /* -------------------------------------------- */
    /*  ICCP-HOST Control 정보 초기화...            */
    /* -------------------------------------------- */
    iccpCntr = (ICCP_CONTROL_DATA *) &iccpDCB->cntrInfo;
    
    
    stsRunTick = 0;
    anaRunTick = 0;
    

 	while(termExec)
	{
        
//        printf("%s:%s:%d\r\n",__FILE__,__FUNCTION__,__LINE__);
	    taskPtr->wdtCount = 0;  // 이건 scan task 자체에 관한 것이고 opr->scanWDT[] 는 각  thread에 관한것
		pause(100);

        /* -------------------------------------------- */
        /*  ICCP 제어정보 추출.....                     */
        /* -------------------------------------------- */
        check_ICCP_Control(iccpCntr);
//          printf("%s:%s:%d\r\n",__FILE__,__FUNCTION__,__LINE__);      
   	    if(oldsec == rtc->sec)	continue;
        oldsec = rtc->sec;	        

        /* 초기 대기시간 ... */
        if(opr->scanInitial > 0)    opr->scanInitial--;
		
        /* -------------------------------------------- */
        /*  DEVICE 상태/계측 Dump : 해당 Flag 설정      */
        /* -------------------------------------------- */
        update_Device_Dump();
        
        if(opr->esioStsDebug)   display_ESIO_sts();
//        printf("%s:%s:%d\r\n",__FILE__,__FUNCTION__,__LINE__);
        /* -------------------------------------------- */
        /*  데이터베이스 변동에 따른 PROCESS 재기동     */
        /* -------------------------------------------- */
        if(opr->esioRestart == SET)
        {
            Debug(console, "scan>> ... *SCAN PROCESS RE-Start....!\n");
            taskPtr->wdtCount = 20;
            termExec = NO;
            //exit(1);
        }
//        printf("%s:%s:%d\r\n",__FILE__,__FUNCTION__,__LINE__);
		/* -------------------------------------------- */
	    /* THREAD 감시 : Internal WDT Flag......        	*/
	    /* -------------------------------------------- */
	    for(i=0; i< MAX_ESIO; i++)
	    {
	    	if(ESIO_THREAD[i].options == RESET)	continue;
            if(opr->scanWDT[i]++ > 30)
            {
            	printf("\n--------------------------------------------\n");
                printf("scan-main> **** ESIO-SCAN-Thread #%d ... WDT Failed...!!! [%04d/%02d/%02d %02d:%02d:%02d]\n", i+1, rtc->year, rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
                printf("--------------------------------------------\n");
                
                /* -------------------------------- */
       			/* LOG File 저장                    	*/
		        /* -------------------------------- */
    	   		sprintf(buffer, "*** ESIO-SCAN-MAIN : *[SCAN-Thread #%d...WDT Failed]...RESTART [%04d/%02d/%02d %02d:%02d:%02d] ", i+1, rtc->year, rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
   				LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));      	
      		
                pause(1000);
                termExec = 0;
            }   
        }
  //       printf("%s:%s:%d\r\n",__FILE__,__FUNCTION__,__LINE__);                       	

        /* ---------------------------------------- */
		/*	SLAVE 모드에서 VME Access 를 하지 않음  */
		/* ---------------------------------------- */
        if(opr->runMode == LOCAL_SLAVE)	continue;


        deviceComUpdate();          // 전체 계전기 통신상태 ... Check
        
        // 2026-02-10 오후 7:42:55 일단 여기에서 문제가 있는 듯
        systemStsUpdate();          // DEVICE 포인트 정보 Update
          
        
        /* -------------------------------------------- */
	    /* 사용자 상태지정.. CONSOLE ......             */
	    /* -------------------------------------------- */        
	    // 아래 flag는 어디서 clear 하지...  각 프로그램 안에서..reset 해준다.
        if(opr->testAnalogFLAG == SET)  testAnalogDATA();
        if(opr->testStatusFLAG == SET)  testStatusDATA();
        if(opr->testDeviceFLAG == SET)  testDeviceDATA();    
            
              
	}/* while */

	// 작업 환경을 정리한다
	ClearEnv();
	
	return (0);
}
