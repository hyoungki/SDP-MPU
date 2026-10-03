
#include	"localLib.h"

TTY_DESC        scuPort	 = {4, 0, NULL, ASYNC_860T, COM_MODEM, 9600,   8,  1, 0, 'N',    FID_SCU_PORT,   {0, 50000}};  

SHM_DESC	    shmDesc = {-1, SHM_KEY, sizeof(SHM_MEMORY), NULL};
SHM_MEMORY	    *shmPtr = NULL;
int             termExec;

TASK_INFO	    *taskPtr= NULL;

RTC             *rtc    = NULL;
OPR_MSG         *opr    = NULL;
CONSOLE_INFO	*console= NULL;

LINK_MSG        *linkCfg= NULL;                 // CPU 이중화 구조체
SCU_MSG         *scuCfg = NULL;                 // 이중화 절체장치(SCU) 구조체

MPU_CONFIG      *mpuCFG = NULL;                 // MPU Config
ESIO_CONFIG     *esioCFG[MAX_ESIO];             // ESIO 장치 Config

HOST_DCB        *hostDCB[MAX_HOST];             // HOST 관련 구조체 : ICCP, DNP, HARRIS, LANDIS...
POINT_BUF       *devPtBuf[MAX_DEV_POINT];       // SDP 포인트 Config 정보
SCAN_CONFIG     *scanCFG[MAX_SCAN_PORT];        // 하위계전기 SCAN Config
SDP_DEVICE      *deviceCFG[MAX_DEVICE];         // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)
	 
RTU_DATABASE    *rtudb  = NULL;                 // SDP 데이터베이스
HISTORY_QUE     *hque   = NULL;                 // CONSOLE 용 이벤트

byte    bitcode[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

static char remoteStr[2][8] = {{"MANUAL"}, {"REMOTE"}};
static char chanStr[2][8]   = {{"CHAN-A"}, {"CHAN-B"}};


static  byte    txbuf[256];
static  byte    rxbuf[256];
        
static  int     scuInitial = RESET;

/* *************************************************************************************
*	FUNCTION : sigHandler()
* **************************************************************************************/
void	SigHandler( int sig )
{
	char    buffer[256];
	
    if(opr->wdtDebug)   Debug(console,"scu> ... signal generated (%2d)...!\n", sig);
    pause(100);
    
	switch(sig)
	{
    case SIGTERM:
        termExec = 0;
        if(opr->wdtDebug)   Debug(console,"scu> ... signal [SIGTERM] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scu> ... signal [SIGTERM] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        pause(1000);    
        break;
	        
	case SIGBUS : 
	    termExec = 0;
        if(opr->wdtDebug)   Debug(console,"scu> ... signal [SIGBUS] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scu> ... signal [SIGBUS] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        pause(1000);    	    
	    break;
		    
	case SIGSEGV: 
	    termExec = 0;
        if(opr->wdtDebug)   Debug(console,"scu> ... signal [SIGSEGV] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scu> ... signal [SIGSEGV] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        pause(1000);   	    
	    break;
		    
	case SIGPIPE: 
        if(opr->wdtDebug)   Debug(console,"scu> ... signal [SIGPIPE] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scu> ... signal [SIGPIPE] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        pause(1000);  	    
		break;

	default : 
	    if(opr->wdtDebug)	Debug(console,"scu> ... signal[%2d] generated ...!\n", sig);
	    /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scu> ... signal[%2d] generated ...!\n", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        pause(1000);  	
		break;
	}
    
}


//
// 모듈:	ClearEnvironment()
//
void ClearEnv(void)
{
    //Debug(console,"link exit...!\n");
    PortClose(&scuPort);

	// 공유 메모리 설정 해제
	if (shmPtr != NULL) ShmDetach(&shmDesc);

	// 프로세스 정보를 정리한다
	UpdateProcessInfo(taskPtr, 0, getpid(), 0);
			
}
					    

//
// 모듈:	InitEnvironment()
//
int InitEnv( void)
{
    int     i;
    
    /* -------------------------------------------- */
	/*  공유 메모리 상태 확인                       */
	/* -------------------------------------------- */
	if (ShmCheck(&shmDesc) < 0)
	{
		printf("scu> *ERR_공유 메모리 상태 이상\n");
		return(0);
	}
	
	/* -------------------------------------------- */
	/*  포인터 변수 초기화                          */
	/* -------------------------------------------- */
	shmPtr = (SHM_MEMORY *) shmDesc.address;
	taskPtr = (TASK_INFO *) &shmPtr->taskInfo[SCU_PROCESS];

	rtc     = (RTC *)       &shmPtr->rtc;
	opr     = (OPR_MSG *)   &shmPtr->opr_msg;
    
    linkCfg = (LINK_MSG *)  &shmPtr->link_msg;                  // MPU 이중화 관련 구조체
    scuCfg 	= (SCU_MSG *)   &shmPtr->scu_msg;                   // 이중화절체장치(SCU) 구조체
    
	rtudb   = (RTU_DATABASE *) &shmPtr->rtuDatabase;
    console = (CONSOLE_INFO *) &shmPtr->console; 
    
	hque    = (HISTORY_QUE *)  &shmPtr->localHistoryQ; 

    mpuCFG  = (MPU_CONFIG *) &shmPtr->mpuConfig;                // MPU Network 구성정보
    
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
	/* ------------------------------------------------------------ */
	taskPtr->initial    = 1;
	taskPtr->wdtEnable  = 1;            
	
	/* ------------------------------------------------ */
	/*  PROCESS 호출시... Priority 지정 (-19 ~ 20)      */
	/* ------------------------------------------------ */
	nice(NICE_SCU);
	
	UpdateProcessInfo(taskPtr, 1, getpid(), 1);
	
	return(1);
}


/*
*/
int	 makeScuCmd( byte   *sndbuf)
{
	int     count=0;
	int		sndCount;
	int		opcode;
	byte    status;
	
	/* ---------------------------- */
	/*	OPCODE 처리 				*/
	/* ---------------------------- */
	if(scuCfg->pfrReset)        opcode = SCU_PFR_CLEAR;
	else if(scuCfg->cntrFlag)   opcode = SCU_CONTROL;
	else                        opcode = SCU_STATUS_DUMP;
	
	status = scuCfg->sndSeqno & 0x0f;
	
	if(scuCfg->cpuRestart)  status |= 0x80;         // MPU 재기동 상태
	if(scuCfg->activeFlag)  status |= 0x10;         // SCU 제어권 허용상태

	/* ---------------------------- */
    /* 	송신 Frame 구성      		*/
    /* ---------------------------- */
    sndbuf[0] = 0x7e;            /* stx          */
    sndbuf[1] = 0;               /* size         */
    sndbuf[2] = scuCfg->address; /* CPU Address  */
    sndbuf[3] = opcode;          /* opcode*/
    sndbuf[4] = status;               /* CPU Status  */
    
    count = 5;

	switch(opcode)
	{
    case SCU_GPOLL:
        sndbuf[count++] = 0;
        break;	
        
    case SCU_STATUS_DUMP:           // 이중화 절체 상태 요청
        sndbuf[count++] = 0;
        break;	
        
    case SCU_CONTROL:               // 이중화 절체 명령
        sndbuf[count++] = scuCfg->chanCmd[0];
        sndbuf[count++] = scuCfg->chanCmd[1];
        sndbuf[count++] = scuCfg->chanCmd[2];
        sndbuf[count++] = scuCfg->chanCmd[3];
        sndbuf[count++] = scuCfg->chanCmd[4];
        sndbuf[count++] = scuCfg->chanCmd[5];
        sndbuf[count++] = scuCfg->chanCmd[6];
        sndbuf[count++] = scuCfg->chanCmd[7];
        break;	
        
    case SCU_PFR_CLEAR:             // PFR Reset
        sndbuf[count++] = 0;
        break;	
        
	default :
		break;	
	}
	
    sndCount = count + 1;
                     
    sndbuf[1] = sndCount;       /* Total size: MSB */
    sndbuf[count] = genlrc(sndbuf, count);
    count++;
    return(count);
        	
}


/*
*/
int rcvWithSCU(TTY_DESC *tp, byte *rxbuf)
{
    int     rcvCount;
    int     rcvTimeout;
    int     reqSize, rcvSize;
    byte    lrc;

    rcvTimeout = 0;

    /* receive 1'st STX */
    do {
        if(asyncRead( opr, tp, &rxbuf[0], 1) == 0) return(0);
        if(rcvTimeout++ > 50) return (0);
    } while(rxbuf[0] != 0x7e);
    
    /* receive size */
    if(asyncRead( opr, tp, &rxbuf[1], 1) == 0)
    {
        if(opr->scuDebug)   Debug(console,"scu> Size not received ...!\n");
        return(0);
    }
    
    reqSize = rxbuf[1] - 2;   /* except stx, size */ 

    /* receive body */
    if((rcvSize = asyncRead( opr, tp, &rxbuf[2], reqSize)) != reqSize)
    {
        if(opr->scuDebug)    
            Debug(console,"link> *Tail not received ...req=%2d, rcv=%2d!\n", reqSize, rcvSize);
        return(0);
    }
    
    rcvCount = rcvSize + 2;
    lrc = genlrc(rxbuf, rcvCount);
        
    if(lrc == 0)   return (rcvCount);
    else    
    {
        if(opr->scuDebug)    
        {
            Debug(console,"scu> *LRC Error ...cal=%2x, rcv=%2x!\n", lrc, rxbuf[rcvCount-1]);
            DumpBuff(console,"*LRC: ", rxbuf, rcvCount);
        }                
        return(0);
    }
    
}

/*
*   이중화절체장치(SCU) 수신메세지 처리...
*/
int rcvHandler_SCU(byte *rxbuf, int rxcnt)
{
    int i;
    int opcode;
    int status;
    
    opcode = rxbuf[3];
    status = rxbuf[4];      // 이중화절체모듈 절체상태
    
    /* ------------------------------------ */
    /*  이중화 절체모듈의 절체상태변화...   */
    /* ------------------------------------ */
    scuCfg->status     = status & 0xf0;         // except Seq.No
    scuCfg->changeFlag = scuCfg->status ^ scuCfg->oldsts;
    scuCfg->oldsts     = scuCfg->status;
    
    /* 초기 SCU  initial */
    if(scuInitial == RESET)
    {
        scuInitial = SET;
        scuCfg->oldsts = scuCfg->status;
        scuCfg->changeFlag = 0;
        printf(">>> SCU : initial Status Update...  %02x\n", scuCfg->status);
     
        if(status & 0x40)   scuCfg->remoteMode = AUTO_MODE;           // 자동모드
        else                scuCfg->remoteMode = MANUAL_MODE;         // 수동모드

        if(status & 0x20)   scuCfg->chanMode = SYSTEM_B;             // Chan-B 상태
        else                scuCfg->chanMode = SYSTEM_A;           // Chan-A 상태
        return (0);
    }    
        
    
    // printf("rcv scu sts .... %2x\n", status);
    
    /* ------------------------------------ */  
    /* SCU 절체상태 변화 발생시...          */
    /* ------------------------------------ */  
    if(scuCfg->changeFlag)  
    {
        linkCfg->cmdFlag = SET;
        if(opr->scuDebug)   
        Debug(console, "scu> Change Flag... SET (%02x)\n", scuCfg->status);
    }
        
    /* ------------------------------------ */
    /*  이중화 절체장치 상태정보            */
    /* ------------------------------------ */
    scuCfg->rcvSeqno    = status & 0x0f;
    
    if(status & 0x80)   scuCfg->pfrReset = SET;
    else                scuCfg->pfrReset = RESET;     

    /* ------------------------------------ */
    /*  SCU 상태 : 수동 / 자동 모드         */
    /* ------------------------------------ */
    if(status & 0x40)   
    {
        if(scuCfg->remoteMode == MANUAL_MODE)   
        {
            logEvent_MPU(shmPtr, ENT_CHG_AUTO, opr->cpuMode, 0, 0, opr->cpuMode, NULL); 
            
            if(opr->scuDebug)   Debug(console,"scu> AUTO-MODE.... set !\n");
        }
        scuCfg->remoteMode = AUTO_MODE;           // 자동모드
        
        /* ------------------------------------ */
        /*  SCU 상태 : MPU-A / MPU-B 모드       */
        /* ------------------------------------ */    
        if(status & 0x20)   scuCfg->chanMode = SYSTEM_B;             // Chan-B 상태
        else                scuCfg->chanMode = SYSTEM_A;           // Chan-A 상태
    }
    else
    {
        if(scuCfg->remoteMode == AUTO_MODE)   
        {
            logEvent_MPU(shmPtr, ENT_CHG_MANUAL, opr->cpuMode, 0, 0, opr->cpuMode, NULL); 
            if(opr->scuDebug)   Debug(console,"scu> MANUAL-MODE.... set !\n");
        }
        scuCfg->remoteMode = MANUAL_MODE;         // 수동모드
    
        /* ------------------------------------ */
        /*  SCU 상태 : MPU-A / MPU-B 모드       */
        /* ------------------------------------ */    
        if(status & 0x20)   
        {
            if(scuCfg->chanMode == SYSTEM_A)   
            {
                logEvent_MPU(shmPtr, ENT_MAN_CHG_B, opr->cpuMode, 0, 0, opr->cpuMode, NULL);  
                if(opr->scuDebug)   Debug(console,"scu> Manual CPU-B.... set !\n");
            }
            scuCfg->chanMode = SYSTEM_B;             // Chan-B 상태
        }
        else
        {
            if(scuCfg->chanMode == SYSTEM_B)   
            {
                logEvent_MPU(shmPtr, ENT_MAN_CHG_A, opr->cpuMode, 0, 0, opr->cpuMode, NULL);  
                if(opr->scuDebug)   Debug(console,"scu> Manual CPU-A.... set !\n");
            }
            scuCfg->chanMode = SYSTEM_A;           // Chan-A 상태
        }
    }    
                
	switch(opcode)
	{
    case SCU_GPOLL:
        break;	
        
    case SCU_STATUS_DUMP:           // 이중화 절체 상태 요청
        
        for(i=0; i < 8; i++)    scuCfg->chanSts[i] = rxbuf[5 + i];
        
        if(opr->scuDebug)
        Debug(console,"scu> rcv[%02d/%02d] Remote sts=%s, Channel sts=%s : %d %d %d %d - %d %d %d %d\n", 
            scuCfg->sndSeqno, scuCfg->rcvSeqno, remoteStr[scuCfg->remoteMode], chanStr[scuCfg->chanMode],
            scuCfg->chanSts[0],scuCfg->chanSts[1],scuCfg->chanSts[2],scuCfg->chanSts[3],
            scuCfg->chanSts[4],scuCfg->chanSts[5],scuCfg->chanSts[6],scuCfg->chanSts[7]);
        
        scuCfg->cpuRestart = 0;     // CU Reset Flag Clear ...
        break;	
        
    case SCU_CONTROL:               // 이중화 절체 명령
        scuCfg->cntrFlag = RESET;
        break;	
        
    case SCU_PFR_CLEAR:             // PFR Reset
        break;	
        
	default :
		break;	
	}

    scuCfg->sndSeqno = (scuCfg->sndSeqno + 1) & 0x0f;

    return (0);
}

/*
* CPU 모드와 절체기 상태에 따른 제어
*/
int check_chanSelect()
{
    int i;
    
    scuCfg->cntrFlag = RESET;
    
    /* -------------------------------------------- */
    /* 절체상태에 따른 채널 변경...                 */
    /*  - 수동모드인 경우... 제어 않함              */
    /*  - CPU모드와 절체상태가 일치하는 경우...     */
    /* -------------------------------------------- */
    if(scuCfg->remoteMode == MANUAL_MODE)   return (0);
    if(opr->runMode == LOCAL_SLAVE)         return (0);        

    if((opr->cpuMode == MPU_A) && (scuCfg->chanMode == SYSTEM_A))     return(0);
    if((opr->cpuMode == MPU_B) && (scuCfg->chanMode == SYSTEM_B))      return(0);    
    
    /* -------------------------------------------- */
    /* 절체상태에 따른 채널 변경...                 */
    /* -------------------------------------------- */
    if((opr->cpuMode == MPU_A) && (scuCfg->chanMode == SYSTEM_B))       // Chan-B 동작시 
    {
        for(i=0; i < 8; i++)    scuCfg->chanCmd[i] = SYSTEM_A;             // 시스템-A 절체
        scuCfg->cntrFlag = SET;
        
        if(opr->scuDebug)
        Debug(console,"scu> *** Change Channel... SYSTEM-A !\n");  
        
    }
    else if((opr->cpuMode == MPU_B) && (scuCfg->chanMode == SYSTEM_A)) // Chan-A 동작시 
    {
        for(i=0; i < 8; i++)    scuCfg->chanCmd[i] = SYSTEM_B;            // 시스템-B 절체
        scuCfg->cntrFlag = SET;
        
        if(opr->scuDebug)
        Debug(console,"scu> *** Change Channel... SYSTEM-B !\n");  
        
    }    
    
    return (0);
}


/* ------------------------------------------------------- */
/*  HOST MAIN 프로그램 Start Routine ....             */
/* ------------------------------------------------------- */
int	main(int argc, char **argv)
{
	int		oldsec;
	int		rcvaddr;
	byte    txcnt, rxcnt;    
    char    buffer[256];
	
	// 작업 환경을 초기화한다
	termExec = InitEnv();
	
	taskPtr->wdtCount = 0;
		
	/* Signal Handler initial */
	signal( SIGBUS,  SigHandler);
	signal( SIGSEGV, SigHandler);
	signal( SIGTERM, SigHandler);	

    /* SCU 통신포트 초기화 ... */
    PortInitial((TTY_DESC *) &scuPort);

    scuCfg->activeFlag = SET;       // SCU 제어권 허용
    scuCfg->cpuRestart = SET;       // MPU 재기동 ...
    
    /* ------------------------------------ */   
    /*  CPU Address 설정...                 */
    /* ------------------------------------ */ 
    if(opr->cpuMode == MPU_A)   scuCfg->address = MPU_A + 1;        // CPU Address => 1
    else                        scuCfg->address = MPU_B + 1;        // CPU Address => 2

	/* LOG File 저장 */
	sprintf(buffer, " %s SCU-MAIN PROCESS Activated ... !", TARGET_NAME);
    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    			
    printf(" %s SCU-MAIN PROCESS Activated ... !\n", TARGET_NAME);
 
	oldsec = rtc->sec;
    scuInitial = RESET;   
    
    //scuCfg->remoteMode  = 0x0f;
    //scuCfg->chanMode    = 0x0f;

  	while(termExec)
	{
	    taskPtr->wdtCount = 0;
		pause(100);

        /* ------------------------------------ */
        /*  이중화절체모듈 : 송신메세지         */
        /* ------------------------------------ */
        if(scuCfg->sendFlag)
        {   
            scuCfg->sendFlag = RESET; 
            if((txcnt = makeScuCmd(txbuf)) > 0)
            {	
                PortWrite( (TTY_DESC *) &scuPort, txbuf, txcnt);
                if(opr->scuDebug)  DumpBuff(console,"TXM: ", txbuf, txcnt);
            }
		}
			            
        if((rxcnt = rcvWithSCU(&scuPort, rxbuf)) > 0)
       	{
       	    rcvaddr = rxbuf[2];
       	    
       	    /* -------------------------------- */
       	    /*  수신메세지 : Address 비교       */
       	    /* -------------------------------- */
       	    if(rcvaddr != scuCfg->address)
       	    {
       	        if(opr->scuDebug)
       	        Debug(console,"scu> *rcv misMatch Address Error .... rcv=%d, cpu=%d\n", rcvaddr, scuCfg->address);     
       	        continue;
       	    }
       	        
   	        scuCfg->comFailTick = 0;
           	if(opr->scuDebug)  DumpBuff(console,"RXM: ", rxbuf, rxcnt);
            
            if(scuCfg->online == RESET)     
            {
                logEvent_MPU(shmPtr, ENT_SCU_ONLINE, opr->cpuMode, 0, 0, opr->cpuMode, NULL);  
                
                /* LOG File 저장 */
    			sprintf(buffer, "%s", ">> SCU ONLINE ...!");
    			LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
            }
           	scuCfg->online = SET;    
        
            /* -------------------------------- */
       	    /*  이중화 절체모듈 수신메세지      */
       	    /* -------------------------------- */
           	rcvHandler_SCU(rxbuf, rxcnt);
           	
   	    }
       	
       	
        if(oldsec == rtc->sec)  continue;
        oldsec = rtc->sec;

        scuCfg->sendFlag = SET;
        
        /* ---------------------------------------- */
        /*  절체모듈 - 절체상태 비교...             */
        /* ---------------------------------------- */
        if(scuCfg->online)  check_chanSelect();
        
        /* ---------------------------------------- */
        /*  절체모듈 통신이상 Check...              */
        /* ---------------------------------------- */
        if(++scuCfg->comFailTick > 5)
        {
            scuCfg->comFailTick = 0;
            scuCfg->cntrFlag = RESET;
            
            if(opr->scuDebug)  
            Debug(console,"scu> *SCU Comm Failed....!\n");
            
            if(scuCfg->online == SET)
            {
                Debug(console,"scu> *SCU Comm Failed....!\n");
                logEvent_MPU(shmPtr, ENT_SCU_OFFLINE, opr->cpuMode, 0, 0, opr->cpuMode, NULL);    
                
                /* LOG File 저장 */
    			sprintf(buffer, "%s", ">> *** SCU OFFLINE ...!");
    			LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
            }
            
            scuCfg->online = 0;
            
            PortClose((TTY_DESC *) &scuPort);  pause(100);
            PortInitial((TTY_DESC *) &scuPort); pause(100);
        }    
        
	}/* while */
    
	// 작업 환경을 정리한다
	ClearEnv();
	
	return (0);
}
