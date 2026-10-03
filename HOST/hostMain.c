
#include	"localLib.h"

TTY_DESC        hostPort1 = {0, 0, NULL, ASYNC_860T, COM_MODEM, 9600,   8,  1, 0, 'N',    FID_SCAN1_PORT,   {0, 50000}};
TTY_DESC        hostPort2 = {0, 0, NULL, ASYNC_860T, COM_MODEM, 9600,   8,  1, 0, 'N',    FID_SCAN2_PORT,   {0, 50000}};
TTY_DESC        hostPort3 = {0, 0, NULL, ASYNC_860T, COM_MODEM, 9600,   8,  1, 0, 'N',    FID_SCAN3_PORT,   {0, 50000}};
TTY_DESC        hostPort4 = {0, 0, NULL, ASYNC_860T, COM_MODEM, 9600,   8,  1, 0, 'N',    FID_SCAN4_PORT,   {0, 50000}};
TTY_DESC        hostPort5 = {0, 0, NULL, ASYNC_860T, COM_MODEM, 9600,   8,  1, 0, 'N',    FID_SCAN5_PORT,   {0, 50000}};


extern  int    hostDnp_tcpip_Thread(int hostid);       // TCPIP DNP
extern  int    hostDnpThread(int hostid);
extern  int    hostHarrisThread(int hostid);


extern int hostSerialTestThread(int hostid) ;

THREAD_ENTRY	hostTHREAD[MAX_HOST];


TASK_INFO	    *taskPtr= NULL;
int             termExec;
int             wdtHostFlag[MAX_HOST];

SHM_DESC	    shmDesc = {-1, SHM_KEY, sizeof(SHM_MEMORY), NULL};
SHM_MEMORY	    *shmPtr = NULL;

RTC             *rtc    = NULL;
OPR_MSG         *opr    = NULL;
CONSOLE_INFO	*console= NULL;

LINK_MSG        *linkCfg= NULL;                 // CPU 이중화 구조체
SCU_MSG         *scuCfg = NULL;                 // 이중화 절체장치(SCU) 구조체

ICCP_CONFIG     *iccpCFG = NULL;                // ICCP-HOST 구조체
MPU_CONFIG      *mpuCFG  = NULL;                // MPU Config
ESIO_CONFIG     *esioCFG[MAX_ESIO];             // ESIO 장치 Config

HOST_DCB        *hostDCB[MAX_HOST];             // HOST 관련 구조체 : ICCP, DNP, HARRIS, LANDIS...
RTU             *rtubuf[MAX_HARRIS_RTU];        /* HARRIS RTU Structure */
PORT_DB         *portdb[MAX_HARRIS_PORT];       /* HARRIS #1 PORT Structure */

POINT_BUF       *devPtBuf[MAX_DEV_POINT];       // SDP 포인트 Config 정보
SCAN_CONFIG     *scanCFG[MAX_SCAN_PORT];        // 하위계전기 SCAN Config
SDP_DEVICE      *deviceCFG[MAX_DEVICE];         // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)
	 
RTU_DATABASE    *rtudb  = NULL;                 // SDP 데이터베이스
HISTORY_QUE     *hque   = NULL;                 // CONSOLE 용 이벤트


byte            vmeRxbuf[MAX_HOST][1024];

PDPDTIME_DATE_TIME      dnpRTC;
PDPDTIME_MS_SINCE_70    dnpRTCInfo;

/* ---------------------------------------- */
/*  VMEBUS 관련 변수 초기화                 */
/* ---------------------------------------- */
void    *vmebus_ptr ;
int     vme_fd; 

VME_SIODCB      *vmeSioDCB;
//VME_CHAN_DCB    *vmeChanDCB[SIO_CHANNEL_MAX];


VME_SIODCB      tempVmeSioDCB;
//VME_CHAN_DCB    tempVmeChanDCB[SIO_CHANNEL_MAX];

byte    bitcode[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

word    hostLEDsts[MAX_HOST] = {0x0080, 0x0100, 0x0200, 0x0400, 0x0800};


/*
*   VME Channel WRITE :
*/
void sioThreadReset( )
{
    vmeSioDCB->initReq = 0x1234;
    vmeSioDCB->initEnd = 0x4321;
    
    Debug(console,"\n===================================\n");
    Debug(console,"===> SIO Thread Re-Start...!\n");
    Debug(console,"===================================\n");
}

/*
*   VME Channel WRITE :
*/
void vmeHostChanReset(int hostid, int chid)
{
    HOST_DCB        *hst;
    VME_CHAN_DCB    *vmeChan;
    
    hst = (HOST_DCB *) hostDCB[hostid];
    vmeChan = hst->vmeChan[chid];
    
    vmeChan->initReq = 0x1234;
}

/*
*/
void modemDelay(int  msec)
{
    struct timeval timeDelay;    

    timeDelay.tv_sec = 0;
    timeDelay.tv_usec = msec * 1000;
    
    select( 1, NULL, NULL, NULL, &timeDelay);
}

/*
*   VME Channel WRITE :
*/
int hostChanWrite(int hostid, int chid, int socketID,  byte *sndbuf, int sndCount)
{
    //int         timeout, mode;
    //char        ctsSts;
    int         rxVal;
    HOST_DCB    *host;
    //TTY_DESC    *tp;
    
    /* ---------------------------------------------------- */
	/*	CPU 이중화 : LOCAL-SLAVE ... 대기모드 				*/
	/* ---------------------------------------------------- */
	//if(opr->runMode == LOCAL_SLAVE)	return (0);  
		
    host = (HOST_DCB *) hostDCB[hostid];
    
    if(host->hostComType == COM_TCPIP)
    {
        if((rxVal = tkWriteTCP( socketID ,(byte *) sndbuf, sndCount, 100)) < 0 )
        {
            if(opr->hostDebug == hostid)  Debug(console,"host%02d> *** Channel=%d  net Send Error [%d/%d]...socket = %d !\n", hostid+1, chid+1, sndCount,  rxVal, socketID);		
        }
    }
    else
    {        
        /* ---------------------------------------------------- */
        /*  RTU-TYPE : ACE-LBS-6U... ASYNC  PORT Comm Parameter */
        /*  RTU-TYPE : ACE-VMECU ... VMESIO PORT Comm Parameter */
        /* ---------------------------------------------------- */
        vmeChanWrite(hostid, vmeSioDCB, host->vmeChan[chid], sndbuf, sndCount);
    }
    
    return (sndCount);
    
}

/*
*   VME Channel READ :
*/
int hostChanRead(int hostid, int chid, byte *rxbuf, int reqCount)
{
    int rcvCount;
    HOST_DCB  *host;

#if 0
	/* ---------------------------------------------------- */
	/*	CPU 이중화 : LOCAL-SLAVE ... 대기모드 				*/
	/* ---------------------------------------------------- */
	if(opr->runMode == LOCAL_SLAVE)	return (0);  
#endif
		
    host = (HOST_DCB *) hostDCB[hostid];
    rcvCount = vmeChanRead(hostid, vmeSioDCB, host->vmeChan[chid], rxbuf, reqCount);
    return (rcvCount);
    
}

/*
*   VME Channel READ :
*/
int hostVMERead(int hostid, int chid, byte *rxbuf)
{
    int     i;
    int     rcvCount;
	int	    timeout = 0;

    HOST_DCB        *hst;
    VME_CHAN_DCB    *vmeChan;
    
#if 0
    /* ---------------------------------------------------- */
	/*	CPU 이중화 : LOCAL-SLAVE ... 대기모드 				*/
	/* ---------------------------------------------------- */
	if(opr->runMode == LOCAL_SLAVE)	return (0);  
#endif
		
    hst = (HOST_DCB *) hostDCB[hostid];
    
    // 각 channel
    vmeChan = (VME_CHAN_DCB *) hst->vmeChan[chid];
    
    // vmeSioDBC는 전체 SIO
    while(vmeSioDCB->mpuAccess + vmeSioDCB->sioAccess)  
    {
        usleep(1000);
        if(++timeout > 20)  
        {
            //Debug(console,"vme%02d> read ready timeout...\n", chid);
            return (0);
        }            
    }

    if(vmeChan->rxFlag)
    {
        vmeSioDCB->mpuAccess = 1;
        rcvCount = vmeChan->rxCount;
        
        if ( rcvCount > VME_CHAN_SIZE ) 
        {
                rcvCount= VME_CHAN_SIZE ;
                Debug(console,"host%02d> *** Channel=%d  rcvCount Err[%d] !\n", hostid+1, chid+1, vmeChan->rxCount);		                       
        }

        for(i=0; i< rcvCount; i++)      rxbuf[i] = vmeChan->rcvbuf[i];
        vmeChan->rxFlag     = 0;
        vmeChan->rxCount    = 0;
        
        vmeSioDCB->mpuAccess = 0;
        
        return(rcvCount);
    }
        
    return(0);    
}


/* *************************************************************************************
*	FUNCTION : sigHandler()
* **************************************************************************************/
void	SigHandler( int sig )
{
	char    buffer[256];
    Debug(console,"host> ... signal generated (%2d)...!\n", sig);
    pause(100);
    
	switch(sig)
	{
    case SIGTERM:
        termExec = 0;
        if(opr->wdtDebug)   Debug(console,"host> ... signal [SIGTERM] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "host> ... signal [SIGTERM] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));      	
        pause(1000);     
        break;
	        
	case SIGBUS : 
	    termExec = 0;
        if(opr->wdtDebug)   Debug(console,"host> ... signal [SIGBUS] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "host> ... signal [SIGBUS] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));      	
        pause(1000);     	    
	    break;
		    
	case SIGSEGV: 
	    termExec = 0;
        if(opr->wdtDebug)   Debug(console,"host> ... signal [SIGSEGV] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "host> ... signal [SIGSEGV] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));    	
        pause(1000);     	    
	    break;
		    
	case SIGPIPE: 
        if(opr->wdtDebug)   Debug(console,"host> ... signal [SIGPIPE] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "host> ... signal [SIGPIPE] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   	
        pause(1000);     	    
		break;

	default : 
	    if(opr->wdtDebug)	Debug(console,"host> ... signal[%2d] generated ...!\n", sig);
	    /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "host> ... signal[%2d] generated ...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
	    pause(1000);	
		break;
	}
        // hkkim 2025-12-09 오후 12:42:39
    termExec = 0 ;
	
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
    void *array[10];
    int size;
	char    buffer[256];
    int i ;
    ucontext_t *uc = (ucontext_t *)unused;

    unsigned long offset_pc ;
    unsigned long offset_lr ;
    
    termExec = 0;


    
    fprintf(stderr, "\n=== HOST  SIGSEGV 발생! ===\n");
    fprintf(stderr, "Fault address: %p\n", si->si_addr);
    
    fprintf(stderr, "PC: 0x%08lx\n", uc->uc_mcontext.arm_pc);
    fprintf(stderr, "LR: 0x%08lx\n", uc->uc_mcontext.arm_lr);
    fprintf(stderr, "SP: 0x%08lx\n", uc->uc_mcontext.arm_sp);    
    
    // 오프셋 계산
     offset_pc = uc->uc_mcontext.arm_pc - base_address;
     offset_lr = uc->uc_mcontext.arm_lr - base_address;
    
    fprintf(stderr, "\nTo decode:\n");
    fprintf(stderr, "  addr2line -e YOUR_PROGRAM -f -p -C 0x%lx\n", offset_pc);
    fprintf(stderr, "  addr2line -e YOUR_PROGRAM -p -C 0x%lx\n", offset_pc);
    fprintf(stderr, "  addr2line -e YOUR_PROGRAM -f -p -C 0x%lx\n", offset_lr);
        
    
        
    fprintf(stderr, "\nBacktrace (크래시 시점의 호출 스택):\n");
    size = backtrace(array, 10);
    // 각 주소를 16진수로 출력
    for (int i = 0; i < size; i++) {
        fprintf(stderr, "  [%d] %p\n", i, array[i]);
    }


    backtrace_symbols_fd(array, size, STDERR_FILENO);



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
    
    // hkkim
    sleep(1);
    
    _exit(1);
}

void install_sighander(void)
{
// 2026-09-02 오후 2:50:11 comment out :  base_addr 
unsigned long init_base ;

    init_base_address();
//    printf("base_address=%x\r\n",base_address);    
    init_base = init_base_address_safe();
//   printf("init_base_address=%x\r\n",base_address);
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

//
// 모듈:	ClearEnvironment()
//
void ClearEnv(int hostid)
{
	// 공유 메모리 설정 해제
	if (shmPtr != NULL)
		ShmDetach(&shmDesc);

	// 프로세스 정보를 정리한다
	UpdateProcessInfo(taskPtr, 0, getpid(), hostid+1);
			
}
					    

//
// 모듈:	InitEnvironment()
//
int InitEnv( int hostid)
{
    int     i;
    
	// 공유 메모리 상태 확인
	if (ShmCheck(&shmDesc) < 0)
	{
		printf("host> *ERR_공유 메모리 상태 이상\n");
		return(0);
	}
	
	// 포인터 변수 초기화
	shmPtr = (SHM_MEMORY *) shmDesc.address;
	taskPtr = (TASK_INFO *) &shmPtr->taskInfo[HOST1_PROCESS + hostid];

	rtc     = (RTC *)       &shmPtr->rtc;
	opr     = (OPR_MSG *)   &shmPtr->opr_msg;
    
    linkCfg = (LINK_MSG *)  &shmPtr->link_msg;                  // MPU 이중화 관련 구조체
    scuCfg 	= (SCU_MSG *)   &shmPtr->scu_msg;                   // 이중화절체장치(SCU) 구조체
    
	rtudb   = (RTU_DATABASE *) &shmPtr->rtuDatabase;
    console = (CONSOLE_INFO *) &shmPtr->console; 
    
	hque    = (HISTORY_QUE *)  &shmPtr->localHistoryQ; 

    mpuCFG  = (MPU_CONFIG *) &shmPtr->mpuConfig;                // MPU Network 구성정보
    iccpCFG  = (ICCP_CONFIG *) &shmPtr->iccpDCB.config;                // MPU Network 구성정보
    
    /* ------------------------------------ */
    /* ESIO CFG : ESIO 구조체 (MMAX 5)      */
    /* ------------------------------------ */ 
    for(i=0; i< MAX_ESIO; i++)      esioCFG[i] = (ESIO_CONFIG *) &shmPtr->esioConfig[i];
    for(i=0; i< MAX_HOST; i++)      hostDCB[i] = (HOST_DCB *) &shmPtr->hostDCB[i];
    for(i=0; i< MAX_SCAN_PORT; i++) scanCFG[i] = (SCAN_CONFIG *) &shmPtr->scanCFG[i];
    for(i=0; i< MAX_DEVICE; i++)    deviceCFG[i] = (SDP_DEVICE *) &shmPtr->deviceCFG[i];
	for(i=0; i< MAX_DEV_POINT; i++) devPtBuf[i] = (POINT_BUF *) &shmPtr->devPtBuf[i];
	
	/* ------------------------------------ */
    /* HARRIS_DCB : 상위 HOST 구조체 (MMAX 8) */
    /* ------------------------------------ */
    for(i=0; i< MAX_HARRIS_RTU; i++)   	rtubuf[i] = (RTU *) &shmPtr->rtubuf[i];
    for(i=0; i< MAX_HARRIS_PORT; i++)   portdb[i] = (PORT_DB *) &shmPtr->portdb[i];
	
    /* ------------------------------------------------------------ */
	/* 프로세스 정보를 초기화한다                                   */
	/* HOST Task 는 TCPIP 전송대기를 고려하여 WDT 기능을 삭제함...  */
	/* ------------------------------------------------------------ */
	taskPtr->initial    = 1;
	taskPtr->wdtEnable  = 1;            
	
	/* ------------------------------------------------ */
	/*  PROCESS 호출시... Priority 지정 (-19 ~ 20)      */
	/* ------------------------------------------------ */
	nice(NICE_HOST);
	
	UpdateProcessInfo(taskPtr, 1, getpid(), hostid+1);
	
	return(1);
}


/*
*   VMEBUS : SIO 관련 메모리 초기화...
*/
int  init_vmebus(void)
{
    // 아직 VMEBUS 구현미정... ?????
    //return (NOK);
    
    /* ------------------------------------ */
    /*  VMEBUS MEMORY DEVICE 초기화         */
    /* ------------------------------------ */
    if ((vme_fd = open("/dev/mem", O_RDWR) ) < 0)
    {
        printf("host> *open VME_AREA : /dev/mem ... Error ! \n");
        return (NOK);
    }        

    /* -------------------------------------------- */
    /*  VME Backup Memory 초기화                    */
    /*  - SIO Module Size : 0x20000                 */
    /* -------------------------------------------- */
    if((vmebus_ptr = ( char  *) mmap(0, VMESIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vme_fd, (VMESIO_START_ADDRESS)) ) < 0)
    {            
        printf("host> *VME Memory Mapping ... Error ! \n");
        return (NOK);            
    }
    
    //printf("host> MAP VME_AREA : %p ... OK ! \n",vmebus_ptr );
    
    return (OK);
}    

/*
*/
int	init_variable(int hostid)
{
    int     size1;

    /* -------------------------------- */
    /* 1. 외부 메모리 포인트 초기화 ... */
    /* -------------------------------- */
    if(init_vmebus() == OK)
    {
        /* SIO보드 초기화...내부 변수 초기화에 따른 변수 */             
	    vmeSioDCB  = (VME_SIODCB *)(  vmebus_ptr);   
	    size1 = sizeof(VME_SIODCB);

        //printf("host> HOST(%d) VMEBUS INIT-OK... VME SIO  Address : %p, size=%d\n", hostid, vmeSioDCB, size1);  
        
        vmeSioDCB->sioAccess = 0;
        vmeSioDCB->mpuAccess = 0;
        vmeSioDCB->initReq   = 0;   
        vmeSioDCB->initEnd   = 0;    
        vmeSioDCB->resetFlag = 0;          	    
        
#if 0
        for(i=0; i< SIO_CHANNEL_MAX; i++)
        {
            size2 = sizeof(VME_CHAN_DCB);
            vmeChanDCB[i] = (VME_CHAN_DCB *) (vmebus_ptr + size1 + (i*size2));   	    
        }	               
#endif
        
    }
    else
    {
        size1 = sizeof(VME_SIODCB);
	    printf("host> **** VMEBUS (size=%d) Mapping Error...Error \n", size1); 

        /* SIO보드 초기화...내부 변수 초기화에 따른 변수 */             
	    vmeSioDCB  = (VME_SIODCB *) &tempVmeSioDCB;   

        vmeSioDCB->sioAccess = 0;
        vmeSioDCB->mpuAccess = 0;
        vmeSioDCB->initReq   = 0;   
        vmeSioDCB->initEnd   = 0;    
        vmeSioDCB->resetFlag = 0;          	    

#if 0        
        for(i=0; i< SIO_CHANNEL_MAX; i++)
        {
            size2 = sizeof(VME_CHAN_DCB);
            vmeChanDCB[i] = (VME_CHAN_DCB *) &tempVmeChanDCB[i];   	    
        }	                
#endif        
    }
    
    return (0);
}

/*
*   FUNCTION : hostDCBInitial()
*   - 계전기 장치 구조체 ... 초기화
*   - VMEBUS 포인터 ... WDT
*/
void hostDCBInitial(int hostid)
{
    HOST_DCB        *host;

    /* --------------------------- */
    /*    HOST 별 운영정보 init    */
    /* --------------------------- */        
    host = (HOST_DCB *) hostDCB[hostid];                 /* 주장치 #1 속성정의 */
    
    host->id = hostid;
    host->initial	= SET;

    /* DNP initialize ... */
    host->sndFlag    = RESET;
    host->sndLinkSts = RESET;
    host->rcvLinkSts = RESET;           // LINK RESET 상태 
    host->rcvEndOk   = SET;

    host->diIndexWord = RESET;          // DI Point 최대 수 : 256보다 큰경우 WORD 처리 
    host->aiIndexWord = RESET;          // AI Point 최대 수 : 256보다 큰경우 WORD 처리 

    /* ---------------------------------------------------- */
    /*  상위 HOST별 DI/AI 포인트 수에 따른 데이터 처리      */
    /* ---------------------------------------------------- */
    if(host->diPtNum > 255) host->diIndexWord    = SET;
    if(host->aiPtNum > 255) host->aiIndexWord    = SET; 
        
    /* HOST 미지정시... */
    if(host->hostDualMode == HOST_NOT_USE)  return;
    if(host->hostComType == COM_TCPIP)      return;

    /* ---------------------------------------------------- */
    /*  RTU-TYPE : ACE-LBS-6U... ASYNC  PORT Comm Parameter */
    /*  RTU-TYPE : ACE-VMECU ... VMESIO PORT Comm Parameter */
    /* ---------------------------------------------------- */
    /* ------------------------------------ */
    /* Default VME Channel 지정             */
    /* ------------------------------------ */
    host->vmeChan[0] = (VME_CHAN_DCB *) &vmeSioDCB->vmeChan[host->vmeMstChan];   /* MASTER 포트 VME 초기화 */  
    host->vmeChan[1] = (VME_CHAN_DCB *) &vmeSioDCB->vmeChan[host->vmeSlvChan];    /* SLAVE  포트 VME 초기화 */   
    
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

    printf("2026.02.01 hostDCBInitialP[%d] initial...!\n", hostid+1);
    //Debug(console,"host> => HOST(%d) DNP initial...!\n", hostid+1);
}



/*
*   FUNCTION : vmeHostInitial()
*   - 계전기 장치 구조체 ... 초기화
*/
// vmeHostInitial 쓰고 SIO에서 확인하는 과정은 ?
#define CHECK_IN  printf("%s:%d-%s\r\n",__FUNCTION__,__LINE__,__FILE__);
void vmeHostInitial(int hostid, int chid)
{
   
    // 아...이거군....
    chid = chid & 0x07;

//    PR_COLOR_YELLOW
//    printf("hostid[%d], chid[%d]\r\n",hostid,chid);
//    CHECK_IN
//    PR_RESET   
//    PR_RESET
    
    VME_CHAN_DCB    *vmeChan;
    HOST_DCB        *host;

    host    = (HOST_DCB *) hostDCB[hostid];                 /* 주장치 #1 속성정의 */
    vmeChan = (VME_CHAN_DCB *) &vmeSioDCB->vmeChan[chid];
    
    vmeChan->rxFront = 0;
    vmeChan->rxRear  = 0;
    vmeChan->rxCount = 0;
    vmeChan->rxFlag  = 0;        
        
    vmeChan->txCount = 0;
    vmeChan->txFlag  = 0;
    
    if(host->hostProtocol == HOST_IEC_101)
    {
        vmeChan->protocolType = SCAN_IEC;           // HOST 별 프로토콜 타입 지정 
        vmeChan->cfgParity    = PARITY_EVEN;
        
        vmeChan->cfgData      = DATA_8_BIT;
        vmeChan->cfgStopbit   = STOP_1_BIT;
    }
    else if(host->hostProtocol == HOST_DNP)
    {
        vmeChan->protocolType = SCAN_DNP;           // HOST 별 프로토콜 타입 지정 
        vmeChan->cfgParity    = PARITY_NONE;
        
        vmeChan->cfgData      = DATA_8_BIT;
        vmeChan->cfgStopbit   = STOP_1_BIT;
    }
    else if(host->hostProtocol == HOST_MODBUS)
    {
        vmeChan->protocolType = SCAN_MODBUS;        // HOST 별 프로토콜 타입 지정 
        vmeChan->cfgParity    = PARITY_NONE;
        vmeChan->cfgData      = DATA_8_BIT;
        vmeChan->cfgStopbit   = STOP_1_BIT;
    }
    else if(host->hostProtocol == HOST_HARRIS)
    {
        vmeChan->protocolType = SCAN_HARRIS;        // HOST 별 프로토콜 타입 지정 
        vmeChan->cfgParity    = PARITY_ODD;
        vmeChan->cfgData      = DATA_7_BIT;
        vmeChan->cfgStopbit   = STOP_1_BIT;
    }
    else
    {
        vmeChan->protocolType = SCAN_ASYNC;        // HOST 별 프로토콜 타입 지정 
        vmeChan->cfgParity    = PARITY_NONE;
        vmeChan->cfgData      = DATA_8_BIT;
        vmeChan->cfgStopbit   = STOP_1_BIT;
    }
            
    vmeChan->cfgType      = host->hostComType;
    vmeChan->cfgSpeed     = host->hostComSpeed;
//    PR_COLOR_YELLOW    
    // hkkim comment out                    
    //printf("host> => HOST(%d) Channel(%d) initial...!\n", hostid+1, chid);
    //printf("host> => HOST(%d) Ch(%d) initial...Speed[%d]!\n", hostid+1, chid+1,  vmeChan->cfgSpeed  );
    //PR_RESET   
    //PR_RESET   
}


/*
* VME 버스상의 SIO 모듈 Time-Sync
*/
void vmeTimeSync()
{
	int	year;
	
	/* ---------------------------------------- */
	/*	SLAVE 모드에서 VME Access 를 하지 않음  */
	/* ---------------------------------------- */
	if(opr->runMode == LOCAL_SLAVE)     return;

    /* VME BUS 상의 SIO보드 - Time Sync...  */        
	year = (rtc->year % 100) + 2000;
    vmeSioDCB->year  = year;
    vmeSioDCB->month = rtc->month;
    vmeSioDCB->day   = rtc->day;
    vmeSioDCB->week  = rtc->week;
    vmeSioDCB->hour  = rtc->hour;
    vmeSioDCB->min   = rtc->min;
    vmeSioDCB->sec   = rtc->sec;
    
    vmeSioDCB->rtcUpdateFlag = 0x1234;
    
    if(opr->wdtDebug)
	Debug(console,"[vme Time-Sync] %04d:%02d:%02d-%02d-%02d:%02d:%02d ...\n", 	year, rtc->month, rtc->day, rtc->week, rtc->hour, rtc->min, rtc->sec);
    
}
    
void make_seg_fault(void)
{
    
    	// segment fault 시험용
	volatile char *pchar ;
	
	pchar =0;
	*pchar =1 ;
}

#define HOST_NULL           0           // HOST Protocol : 미지정, 사용않함
#define HOST_HARRIS         1           // HOST Protocol : HARRIS 
#define HOST_LANDIS         2           // HOST Protocol : LANDIS 
#define HOST_DNP            3           // HOST Protocol : DNP 
#define HOST_MODBUS         4           // HOST Protocol : MODBUS 
#define HOST_ICCP	        5           // HOST Protocol : ICCP-60870 
#define HOST_IEC_101	    6           // HOST Protocol : IEC60870-101 
#define HOST_RESERVE1       7           // HOST Protocol : Reserved
#define HOST_RESERVE2       8           // HOST Protocol : Reserved


char *HOST_PROTOCOL_NAME[10] =
{
   "NULL",
   "HARRIS",
   "LANDIS",    
   "DNP",
   "MODBUS",             
   "ICCP",
   "IEC_101",
   "RESERVED1",
   "RESERVED2"
   "ERROR"      
} ;
/* ------------------------------------------------------- */
/*  HOST MAIN 프로그램 Start Routine ....             */
/* ------------------------------------------------------- */
int	main(int argc, char **argv)
{
    int     hostid;
    int		oldsec=0;
    char	buffer[128];
	
	HOST_DCB *host;
	


#if 0 // 2026-03-04 오전 10:40:34 hkkim	
    /* Signal Handler initial */
	signal( SIGBUS,  SigHandler);
	signal( SIGSEGV, SigHandler);
	signal( SIGTERM, SigHandler);	
	
	signal( SIGPIPE, SigHandler);
	
#else
    install_sighander();    
#endif 	
	
	/* ------------------------------------ */  
	/*  HOST ID 추출...                     */
	/* ------------------------------------ */  
	hostid = atoi(argv[1]);

#ifdef __ARM_PLATFORM__
    #warning "ARM PLATFORM Selected"
#endif 

//	printf("HOST> HOST-MAIN Call ... argc=%d, argv[]= %s %s, hostid=%d \n", argc, argv[0], argv[1], hostid);


    


	// 작업 환경을 초기화한다
	termExec = InitEnv(hostid);
	
	/* 내부변수 초기화 */
    init_variable(hostid);

	/* LOG File 저장 */
	sprintf(buffer, " %s HOST-MAIN (hostid=%d) PROCESS Activated ... !", TARGET_NAME, hostid);
    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    
    pause(1000);

	/* ------------------------------------ */    
    /* CPU 링크상태 확인 후 HOST-TASK  기동 */
    /* ------------------------------------ */  
    while(opr->chkLinkOK == RESET)
    {
    	taskPtr->wdtCount = 0;
		pause(100);
		//Debug(console,"host%02d> *check LINK Status HOST ...! \n", hostid+1 );
    }
    
    taskPtr->wdtCount   = 0;
    
    /* ------------------------------------ */
    /* 상위 호스트관련 변수 초기화          */
    /* ------------------------------------ */
    host = (HOST_DCB *) hostDCB[hostid];                 /* 주장치 #1 속성정의 */
	hostDCBInitial(hostid);
	
	/* ------------------------------------ */
    /* HOST채널 미정의... MMI 상 OFF 처리   */
    /* ------------------------------------ */
    opr->hostRestart[hostid] = 0;

//    make_seg_fault();



    /* ------------------------------------------------ */
    /* HOST Protocol 타입에 따른 분기 ...               */
    /*   - HARRIS 인경우    : HARRIS-Task 호출...       */
    /*   - DNP(TCPIP)인경우 : DNP-TCPIP-Task 호출...    */
    /*   - DNP(ASYNC)인경우 : DNP-Task 호출...          */
    /*   - 기타(MODBUS, IEC): 미정의 Task               */
    /* ------------------------------------------------ */
    // hkkim

    if(host->hostDualMode != HOST_NOT_USE) // DB상 운영모드
    {   
        printf("HOST> hostid [%d] protocol %s\r\n", hostid ,HOST_PROTOCOL_NAME [ host->hostProtocol ] );
        
        if(host->hostProtocol == HOST_HARRIS)           hostHarrisThread(hostid);  // MODBUS-THREAD  
        else if(host->hostProtocol == HOST_DNP)     
        {
            /* ---------------------------------------- */
            /* TCPIP 인 경우 : 각각의 SERVER 프로그램   */
            /* ---------------------------------------- */
            if(host->hostComType == COM_TCPIP)
            {
                hostDnp_tcpip_Thread(hostid);      // RS232 : DNP-HOST 실행
            }    
            else
            {    
                hostDnpThread(hostid);     // RS232 : DNP-HOST 실행
            }
        }
        
        // hkkim
        else if(host->hostProtocol == HOST_RESERVE1)  // 예비 1... serial test
        {            
              hostSerialTestThread(hostid);
        }
        
        else
        {
            while(termExec)
        	{
        	    taskPtr->wdtCount = 0;
	        	pause(100);

                if(oldsec == rtc->sec)  continue;
	        	oldsec = rtc->sec;
		
		        /* ------------------------------------ */
                /* HOST-DB 변경에 따른 재기동 ...       */
                /* ------------------------------------ */
                if(opr->hostRestart[hostid] == SET)
                {
                    Debug(console,"host%02d> *** HOST TASK Restart... DB Chg ...\n", hostid+1);
                    pause(1000);
                    termExec = RESET;
                }
        
                if(opr->hostDebug == hostid)
                Debug(console,"host%02d> *Invalid PROTOCOL-Type HOST ...! \n", hostid+1 );
            }
        }
    }    
    else  
    {
        while(termExec)
    	{
    	    taskPtr->wdtCount = 0;
	    	pause(100);

            if(oldsec == rtc->sec)  continue;
	    	oldsec = rtc->sec;
		
		    /* ------------------------------------ */
            /* HOST-DB 변경에 따른 재기동 ...       */
            /* ------------------------------------ */
            if(opr->hostRestart[hostid] == SET)
            {
                Debug(console,"host%02d> *** HOST TASK Restart... DB Chg ...\n", hostid+1);
                pause(1000);
                termExec = RESET;
            }
                
            if(opr->hostDebug == hostid)
            Debug(console,"host%02d> *HOST NO-Define ...! \n", hostid+1 );
        }
    }
    
	// 작업 환경을 정리한다
	ClearEnv(hostid);
	
	return (0);
}
