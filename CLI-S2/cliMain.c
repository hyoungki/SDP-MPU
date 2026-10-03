//==============================================================================
//
// File:    cliMain.c
//
// CLI-S2 : CLI-ReadLine과 동일한 main()/환경 초기화. 명령 처리부(InputProcessing)만
//          cliDrv.c에서 argc/argv 디스패치 방식으로 바뀌었다.
//
//==============================================================================
#include	"localLib.h"
#include    "cli.h"

#include <readline/readline.h>
#include <readline/history.h>

extern void initialize_readline (void);
extern void InputProcessing(void);


int	        termExec;

SHM_DESC	shmDesc = {-1, SHM_KEY, sizeof(SHM_MEMORY), NULL};
SHM_MEMORY	*shmPtr = NULL;
TASK_INFO	*taskPtr = NULL;

RTC             *rtc    = NULL;
OPR_MSG         *opr    = NULL;
CONSOLE_INFO	*console= NULL;

LINK_MSG        *linkCfg= NULL;                 // CPU 이중화 통신 구조체
SCU_MSG         *scuCfg = NULL;                 // 통신제어장치(SCU) 구조체

ICCP_DCB        *iccpDCB = NULL;                // ICCP-HOST 구조체
ICCP_CONFIG     *iccpCFG = NULL;                // ICCP-HOST 구조체
ICCP_60870_DCB  *iccpInfo=NULL;                 // ICCP-HOST 데이터 : 모니터링 구조체

MPU_CONFIG      *mpuCFG  = NULL;                // MPU Config
ESIO_CONFIG     *esioCFG[MAX_ESIO];             // ESIO 장치 Config

HOST_DCB        *hostDCB[MAX_HOST];             // HOST 통신 구조체 : ICCP, DNP, HARRIS, LANDIS...
POINT_BUF       *devPtBuf[MAX_DEV_POINT];       // SDP 포인트 Config 정보
CAL_POINT_BUF   *calPtBuf[MAX_CAL_POINT];       // SDP 연산 포인트 Config 정보

SCAN_CONFIG     *scanCFG[MAX_SCAN_PORT];        // 통신포트별 SCAN Config
SDP_DEVICE      *deviceCFG[MAX_DEVICE];         // 자보호/장치-보조계전기 (GiPAM, HiMAP...)

RTU_DATABASE    *rtudb  = NULL;                 // SDP 데이터베이스
HISTORY_QUE     *hque   = NULL;                 // CONSOLE 용 이벤트


extern  void InputProcessing(void);


int     passTimeCLI;

/* THREAD 관련 정의 */
void            *debug_thread(int  *arg);
THREAD_ENTRY    debugTHREAD;


//
// 함수:	SigHandler()
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
// 함수:	ClearEnv()
//
void ClearEnv(void)
{
	// 공유 메모리 접속 해제
	if (shmPtr != NULL)
		ShmDetach(&shmDesc);

	printf("[*] Close Device ....4!\n");
}


//
// 함수:	InitEnv()
//
int InitEnv( void)
{
    int     i;

	// SIGNAL 처리 루틴 등록
	signal( SIGBUS,  SigHandler);
	signal( SIGSEGV, SigHandler);
	signal( SIGTERM, SigHandler);

	// 공유 메모리 상태 확인
	if (ShmCheck(&shmDesc) < 0)
	{
		printf("cli> *ERR_공유 메모리 상태 이상\n");
		return(0);
	}
	

	shmPtr = (SHM_MEMORY *) shmDesc.address;

	taskPtr = &shmPtr->taskInfo[CLI_PROCESS];

	rtc     = (RTC *)       &shmPtr->rtc;
	opr     = (OPR_MSG *)   &shmPtr->opr_msg;

    linkCfg = (LINK_MSG *)  &shmPtr->link_msg;
    scuCfg 	= (SCU_MSG *)   &shmPtr->scu_msg;

	rtudb   = (RTU_DATABASE *) &shmPtr->rtuDatabase;
    console = (CONSOLE_INFO *) &shmPtr->console;

	hque    = (HISTORY_QUE *)  &shmPtr->localHistoryQ;

    mpuCFG  = (MPU_CONFIG *) &shmPtr->mpuConfig;
    iccpDCB = (ICCP_DCB *) &shmPtr->iccpDCB;
    iccpCFG = (ICCP_CONFIG *) &shmPtr->iccpDCB.config;

    iccpInfo= (ICCP_60870_DCB *) &shmPtr->iccpInfo;

    for(i=0; i< MAX_ESIO; i++)      esioCFG[i] = (ESIO_CONFIG *) &shmPtr->esioConfig[i];
    for(i=0; i< MAX_HOST; i++)      hostDCB[i] = (HOST_DCB *) &shmPtr->hostDCB[i];
    for(i=0; i< MAX_SCAN_PORT; i++) scanCFG[i] = (SCAN_CONFIG *) &shmPtr->scanCFG[i];
    for(i=0; i< MAX_DEVICE; i++)    deviceCFG[i] = (SDP_DEVICE *) &shmPtr->deviceCFG[i];
	for(i=0; i< MAX_DEV_POINT; i++) devPtBuf[i] = (POINT_BUF *) &shmPtr->devPtBuf[i];
	for(i=0; i< MAX_CAL_POINT; i++) calPtBuf[i] = (CAL_POINT_BUF *) &shmPtr->calPtBuf[i];

	// PROCESS 우선순위 조정 (-19 ~ 20)
	i=nice(NICE_CLI);
	(void) i ;
	return(1);
}

#define COMPILE_TIME "CLI-S2 build"

//
// 함수:	DisplayLogo()
//
void DisplayLogo(void)
{
	ExecCommand("clear");

    printf("   (  )   /\\  _                (		\r\n");
    printf("    \\ |  (  \\( \\.(              )                     _____ \r\n");
    printf("  \\  \\ \\ `  `   ) \\            (  __                 / _   \\ \r\n");
    printf(" (_`    \\+  . x  ( .\\            \\/  \\____-----------/ (o)   \\_ \r\n");
    printf("- .-              \\+  ;          (  O                          \\____  \r\n");
    printf("(__                +- .( -'.- <.   \\____________  `              \\  /  \r\n");
    printf("(_____            ._._: <_ - <- _- _  VVVVVVV VV V\\               \\ /  \r\n");
    printf("  .    /./.+-  . .- /  +--  - .    (--_AAAAAAA__A_/                |   \r\n");
    printf("  (__ ' /x  / x _/ (                \\______________//_              \\_______\r\n");
    printf(" , x / ( '  . / .  /                                  \\___'         \\      /\r\n");
    printf("    /  /  _/ /    +                                       |           \\    /	\r\n");
    printf("   '  (__/                                               /             \\  /	\r\n");
    printf("                                                       /                \\	\r\n");

    printf("            [CLI-S2] argv-dispatch CLI  %s\r\n", COMPILE_TIME);

#ifdef __ARM_ARCH__
    printf("            ARCH : ARM\r\n");
#endif

#ifdef __PPC_ARCH__
    printf("            ARCH : PPC\r\n");
#endif

    printf("\r\n");
    printf("\r\n");
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

    debugTHREAD.entryPt    = debug_thread;
    debugTHREAD.arg        = 0;

    state = pthread_create( &debugTHREAD.tid, NULL, debugTHREAD.entryPt, &i);
   	if(state != 0)
    {
	    printf("CLI-S2> *Thread# [%s] Create Error ...!\n", debugTHREAD.name);
	    exit(1);
    }

	pause(100);

	return (0);
}

/* -------------------------------------------------------- */
/*  프로그램 MAIN 진입점                                     */
/* -------------------------------------------------------- */
int	main(int argc, char **argv)
{
	if ((termExec = InitEnv()) == 1)
	{
		DisplayLogo();
	}

	DisplayLogo();

    printf("CLI-S2>  start...!\n");

#ifdef  NETWORK_CONSOLE
	debug_thread_call();
#endif

	console->front = 0;
	console->rear  = 0;
	console->active = SET;

    passTimeCLI = 0;

	initialize_readline() ;

	while(termExec)
	{
		InputProcessing();
		usleep(100000);		/* 100 msec delay */
	}

    console->front = 0;
	console->rear  = 0;
	console->active = RESET;

	ClearEnv();

	return (0);
}
