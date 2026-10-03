//==============================================================================
//
// 파일:    cliMain.c
//
//==============================================================================
#include	"localLib.h"
#include    "cli.h"

int	        termExec;

SHM_DESC	shmDesc = {-1, SHM_KEY, sizeof(SHM_MEMORY), NULL};
SHM_MEMORY	*shmPtr = NULL;
TASK_INFO	*taskPtr = NULL;

RTC             *rtc    = NULL;
OPR_MSG         *opr    = NULL;
CONSOLE_INFO	*console= NULL;

LINK_MSG        *linkCfg= NULL;                 // CPU 이중화 구조체
SCU_MSG         *scuCfg = NULL;                 // 이중화 절체장치(SCU) 구조체

ICCP_DCB        *iccpDCB = NULL;                // ICCP-HOST 구조체
ICCP_CONFIG     *iccpCFG = NULL;                // ICCP-HOST 구조체
ICCP_60870_DCB  *iccpInfo=NULL;                 // ICCP-HOST 참조용 : 모니터링 구조체

MPU_CONFIG      *mpuCFG  = NULL;                // MPU Config
ESIO_CONFIG     *esioCFG[MAX_ESIO];             // ESIO 장치 Config

HOST_DCB        *hostDCB[MAX_HOST];             // HOST 관련 구조체 : ICCP, DNP, HARRIS, LANDIS...
POINT_BUF       *devPtBuf[MAX_DEV_POINT];       // SDP 포인트 Config 정보
CAL_POINT_BUF   *calPtBuf[MAX_CAL_POINT];       // SDP 연산 포인트 Config 정보

SCAN_CONFIG     *scanCFG[MAX_SCAN_PORT];        // 하위계전기 SCAN Config
SDP_DEVICE      *deviceCFG[MAX_DEVICE];         // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)

//RTU_CONFIG      *rtuDCB = NULL;                 // 하부 RTU 운영 구조체
	 
RTU_DATABASE    *rtudb  = NULL;                 // SDP 데이터베이스
HISTORY_QUE     *hque   = NULL;                 // CONSOLE 용 이벤트


extern  void InputProcessing(void);


int     passTimeCLI;

/* THREAD 관련 변수 정의 */
void            *debug_thread(int  *arg);
THREAD_ENTRY    debugTHREAD;

//
// 모듈:	SigHandler()
//
void SigHandler(int sig)
{
    if(opr->wdtDebug)   Debug(console,"cli> ... signal generated (%2d)...!\n", sig);
    pause(100);
    
	switch(sig)
	{
    case SIGTERM:
        termExec = 0;
        if(opr->wdtDebug)   Debug(console,"cli> ... signal [SIGTERM] generated (%2d)...!\n", sig);
        pause(1000);     
        break;
	        
	case SIGBUS : 
	    termExec = 0;
        if(opr->wdtDebug)   Debug(console,"cli> ... signal [SIGBUS] generated (%2d)...!\n", sig);
        pause(1000);     	    
	    break;
		    
	case SIGSEGV: 
	    termExec = 0;
        if(opr->wdtDebug)   Debug(console,"cli> ... signal [SIGSEGV] generated (%2d)...!\n", sig);
        pause(1000);     	    
	    break;
		    
	case SIGPIPE: 
        if(opr->wdtDebug)   Debug(console,"cli> ... signal [SIGPIPE] generated (%2d)...!\n", sig);
        pause(1000);     	    
		break;

	default : 
	    if(opr->wdtDebug)   Debug(console,"cli> ... signal[%2d] generated ...!\n", sig);
		break;
	}
}


//
// 모듈:	ClearEnvironment()
//
void ClearEnv(void)
{
	// 공유 메모리 설정 해제
	if (shmPtr != NULL)
		ShmDetach(&shmDesc);

    //PortClose(&consolePort);
	printf("[*] Close Device ....4!\n");
}
					    

//
// 모듈:	InitEnvironment()
//
int InitEnv( void)
{
    int     i;
    
	// SIGNAL 처리 루틴 설정
	signal( SIGBUS,  SigHandler);
	signal( SIGSEGV, SigHandler);
	signal( SIGTERM, SigHandler);

	// 공유 메모리 상태 확인
	if (ShmCheck(&shmDesc) < 0)
	{
		printf("cli> *ERR_공유 메모리 상태 이상\n");
		return(0);
	}
	
	// 포인터 변수 초기화
	shmPtr = (SHM_MEMORY *) shmDesc.address;
	
	taskPtr = &shmPtr->taskInfo[CLI_PROCESS];

	rtc     = (RTC *)       &shmPtr->rtc;
	opr     = (OPR_MSG *)   &shmPtr->opr_msg;
    
    linkCfg = (LINK_MSG *)  &shmPtr->link_msg;                  // MPU 이중화 관련 구조체
    scuCfg 	= (SCU_MSG *)   &shmPtr->scu_msg;                   // 이중화절체장치(SCU) 구조체
    
	rtudb   = (RTU_DATABASE *) &shmPtr->rtuDatabase;
    console = (CONSOLE_INFO *) &shmPtr->console; 
    
	hque    = (HISTORY_QUE *)  &shmPtr->localHistoryQ; 

    mpuCFG  = (MPU_CONFIG *) &shmPtr->mpuConfig;                // MPU Network 구성정보
    iccpDCB = (ICCP_DCB *) &shmPtr->iccpDCB;                   // ICCP_DCB 
    iccpCFG = (ICCP_CONFIG *) &shmPtr->iccpDCB.config;         // MPU Network 구성정보
    
    /* ICCP-INFO 구조체 정의 */
    iccpInfo= (ICCP_60870_DCB *) &shmPtr->iccpInfo;            // ICCP-HOST 참조용 : 모니터링 구조체
    
    //rtuDCB  = (RTU_CONFIG *) &shmPtr->rtuConfig;               // 하부 RTU 운영구조체
    
    /* ------------------------------------ */
    /* ESIO CFG : ESIO 구조체 (MMAX 5)      */
    /* ------------------------------------ */ 
    for(i=0; i< MAX_ESIO; i++)      esioCFG[i] = (ESIO_CONFIG *) &shmPtr->esioConfig[i];
    for(i=0; i< MAX_HOST; i++)      hostDCB[i] = (HOST_DCB *) &shmPtr->hostDCB[i];
    for(i=0; i< MAX_SCAN_PORT; i++) scanCFG[i] = (SCAN_CONFIG *) &shmPtr->scanCFG[i];
    for(i=0; i< MAX_DEVICE; i++)    deviceCFG[i] = (SDP_DEVICE *) &shmPtr->deviceCFG[i];
	for(i=0; i< MAX_DEV_POINT; i++) devPtBuf[i] = (POINT_BUF *) &shmPtr->devPtBuf[i];
	for(i=0; i< MAX_CAL_POINT; i++) calPtBuf[i] = (CAL_POINT_BUF *) &shmPtr->calPtBuf[i];
    
    
    /* ------------------------------------------------ */
	/*  PROCESS 호출시... Priority 지정 (-19 ~ 20)      */
	/* ------------------------------------------------ */
	nice(NICE_CLI);
	
	return(1);
}

//
// 모듈:	DisplayLogo()
//
#define COMPILE_TIME "2026-05-29 오후 3:53:31"
void DisplayLogo(void)
{
	ExecCommand("clear");
#if 0
	printf("\t\t////////////////////////////////////////////////////////////\n");
	printf("\t\t//                                                        //\n");
	printf("\t\t//        %s : CLI - USER Debugger     //\n", TARGET_NAME);
	printf("\t\t//        [*] Designed by ACE Control System                     //\n");
	printf("\t\t//        [*] Version   %s                       //\n", VERSION_STRING);
	printf("\t\t//        [*] %s         //\n", SIMULATOR_NAME);  
	printf("\t\t//        [*] Update Date ...%s                   //\n", DATE_STRING);
	printf("\t\t//                                                        //\n");
	printf("\t\t////////////////////////////////////////////////////////////\n\n");

#endif 

    printf("   (  )   /\\  _                (		\r\n");
    printf("    \\ |  (  \\( \\.(              )                     _____ \r\n");
    printf("  \\  \\ \\ `  `   ) \\            (  __                 / _   \\ \r\n");
    printf(" (_`    \\+  . x  ( .\            \\/  \\____-----------/ (o)   \\_ \r\n");
    printf("- .-              \\+  ;          (  O                          \\____  \r\n");
    printf("(__                +- .( -'.- <.   \\____________  `              \\  /  \r\n");
    printf("(_____            ._._: <_ - <- _- _  VVVVVVV VV V\\               \\ /  \r\n");
    printf("  .    /./.+-  . .- /  +--  - .    (--_AAAAAAA__A_/                |   \r\n");
    printf("  (__ ' /x  / x _/ (                \\______________//_              \\_______\r\n");
    printf(" , x / ( '  . / .  /                                  \\___'         \\      /\r\n");
    printf("    /  /  _/ /    +                                       |           \\    /	\r\n");
    printf("   '  (__/                                               /             \\  /	\r\n");
    printf("                                                       /                \\	\r\n");

    printf("\r\n");   
    printf("\r\n");
#ifdef __ARM_ARCH__
    printf("ARCH : ARM  %s\r\n",COMPILE_TIME);
#endif 

#ifdef __PPC_ARCH__
    printf("ARCH : PPC  %s\r\n",COMPILE_TIME);
#endif 
	
 }

/*
*   NETWORK Debug MAIN() 
*/
int debug_thread_call()
{
    int     i;     
    int	    state;

    i = 0;
    
    debugTHREAD.tid        = i;
    sprintf(debugTHREAD.name, "%s%d","DEBUG", i);
    debugTHREAD.priority   = 0;
    debugTHREAD.options    = 0;
    
    debugTHREAD.entryPt    = debug_thread;  // MODBUS-THREAD
    debugTHREAD.arg        = 0;
    
    state = pthread_create( &debugTHREAD.tid, NULL, debugTHREAD.entryPt, &i);
   	if(state != 0)
    {
	    printf("CLI> *Thread# [%s] Create Error ...!\n", debugTHREAD.name);
	    exit(1);
    }
	
	pause(100);
	
	return (0);
}

void dispDB_SIZE(void)
{
#if 0    
typedef struct
        {
        //      DB_MPU_CONFIG2  기존 ESIO 와 호환을 위한 것..       
        DB_ESIO_MPU_CONFIG		esio_mpuConfig;					        // SDP : MPU Network 구성정보 & Parameter
        DB_ESIO_CONFIG      esioConfig[MAX_DB_ESIO];            // SDP : ESIO Network 구성정보 & Parameter, MAX 6
		DB_HOST_CONFIG 		hostCfg[MAX_DB_HOST];		        // SDP : HOST 구성정보 & 운영파라메터, MAX 8

        DB_ICCP_CONFIG      iccpConfig;                         // SDP : ICCP HOST 구성정보 & Parameter
        
        byte    portdb[32];                			            // SDP : HARRIS-HOST 구성정보
        byte    chassisNum;                 			        // SDP : LANDIS Chassis Number
        byte    chassis[16][16];            			        // SDP : LANDIS RTU 구성정보
        
        DB_MODBUS_PROFILE	modbusProfile[MAX_DB_MODBUS_PROFILE];  // SDP : MODBUS 프로파일 구성정보, MAX 16
        
        DB_SCAN_CONFIG	scanConfig[MAX_DB_SCAN_PORT];	            // SDP : 계전기 통신포트 구성, MAX 32
        DB_SDP_DEVICE   deviceConfig[MAX_DB_DEVICE];	            // SDP : 계전기별 운영 구성, MAX 112
        DB_POINT_BUF    pointBuf[MAX_DB_POINT]; 	            	// SDP : 운영 포인트 구성, MAX 4096
        
        DB_CAL_POINT        calPointBuf[MAX_DB_CAL_POINT];         	// SDP : 연산포인트 구성, MAX 32
		
		byte    chksum[2];
        
	    } __attribute__ ((packed)) ESIO_RTU_DATABASE;
#endif 	    
	    printf("SIZEOF(ESIO_RTU_DATABASE)  %d\r\n",sizeof(ESIO_RTU_DATABASE ))  ;
	    printf("SIZEOF(DB_ESIO_MPU_CONFIG) %d \r\n",sizeof(DB_ESIO_MPU_CONFIG ))  ;
        printf("MAX_DB_ESIO %d\r\n",MAX_DB_ESIO);
	    printf("SIZEOF(DB_ESIO_CONFIG) %d \r\n",sizeof(DB_ESIO_CONFIG ))  ;
	    printf("SIZEOF(DB_HOST_CONFIG) %d \r\n",sizeof(DB_HOST_CONFIG ))  ;
        printf("MAX_DB_HOST %d\r\n",MAX_DB_HOST);

	    printf("SIZEOF(DB_ICCP_CONFIG) %d \r\n",sizeof(DB_ICCP_CONFIG ))  ;
	    printf("SIZEOF(DB_MODBUS_PROFILE) %d \r\n",sizeof(DB_MODBUS_PROFILE ))  ;	    	    	    

	    printf("SIZEOF(DB_SCAN_CONFIG)  %d\r\n",sizeof(DB_SCAN_CONFIG ))  ;
	    printf("SIZEOF(DB_SDP_DEVICE)  %d\r\n",sizeof(DB_SDP_DEVICE ))  ;
	    printf("SIZEOF(DB_POINT_BUF)  %d\r\n",sizeof(DB_POINT_BUF ))  ;
	    printf("SIZEOF(DB_CAL_POINT)  %d\r\n",sizeof(DB_CAL_POINT ))  ;
  	       
    
}
 
	
/* ------------------------------------------------------- */
/*  신호제어기 MAIN 프로그램 Start Routine ....             */
/* ------------------------------------------------------- */
int	main(int argc, char **argv)
{
	
	// 작업 환경을 초기화한다
	if ((termExec = InitEnv()) == 1)
	{
		DisplayLogo();
	}
	
	DisplayLogo();
		
    printf("...CLI start...!\n");


dispDB_SIZE();

#ifdef  NETWORK_CONSOLE
    /* ------------------------------------------------ */
    /*  ROM Version 경우 : Network Console TASK 실행    */
    /* ------------------------------------------------ */
	debug_thread_call();
#endif
	
	console->front = 0;
	console->rear  = 0;
	console->active = SET;
	
    passTimeCLI = 0;
    
	while(termExec)
	{
		InputProcessing();
		usleep(100000);		/* 100 msec delay */

	}

    console->front = 0;
	console->rear  = 0;
	console->active = RESET;
	
	// 작업 환경을 정리한다
	ClearEnv();
	
	return (0);
}
