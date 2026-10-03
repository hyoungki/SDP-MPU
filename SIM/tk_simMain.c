
#include	"localLib.h"
#include    "external.h"

#include <stddef.h>

SHM_DESC	    shmDesc = {-1, SHM_KEY, sizeof(SHM_MEMORY), NULL};
SHM_MEMORY	    *shmPtr = NULL;

int             termExec;

TASK_INFO	*taskPtr= NULL;

RTC             *rtc    = NULL;
OPR_MSG         *opr    = NULL;
CONSOLE_INFO	*console= NULL;
LINK_MSG        *linkCfg= NULL;                 // CPU 이중화 구조체
SCU_MSG         *scuCfg = NULL;                 // 이중화 절체장치(SCU) 구조체

MPU_CONFIG      *mpuCFG = NULL;                 // MPU Config
ESIO_CONFIG     *esioCFG[MAX_ESIO];             // ESIO 장치 Config
ICCP_DCB        *iccpDCB = NULL;                // ICCP-HOST 구조체

HOST_DCB        *hostDCB[MAX_HOST];             // HOST 관련 구조체 : ICCP, DNP, HARRIS, LANDIS...
POINT_BUF       *devPtBuf[MAX_DEV_POINT];       // SDP 포인트 Config 정보
SCAN_CONFIG     *scanCFG[MAX_SCAN_PORT];        // 하위계전기 SCAN Config
SDP_DEVICE      *deviceCFG[MAX_DEVICE];         // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)
	 
RTU_DATABASE    *rtudb  = NULL;                 // SDP 데이터베이스
HISTORY_QUE     *hque   = NULL;                 // CONSOLE 용 이벤트


struct  sockaddr_in	srvAddr;
struct  sockaddr_in	clientPtr;

int     simTask_Active;
int     socketServerFd;         /* for Server */
int     simSocketID = 0;

static  byte    simRxbuf[MAX_SIMULATOR_BUF];
static  byte    simTxbuf[MAX_SIMULATOR_BUF]; // 4096


/* *************************************************************************************
*	FUNCTION : sigHandler()
* **************************************************************************************/
void	SigHandler( int sig )
{
	char    buffer[256];
	
	if(opr->wdtDebug)   Debug(console,"<sim> *****  ... signal generated (%2d)...!\n", sig);
    pause(100);
    
	switch(sig)
	{
	    case SIGTERM:
	        if(opr->wdtDebug)   Debug(console,"sim> *SIG TERM Error ... !\n", sig); 
            /* -------------------------------- */
        	/* LOG File 저장                    */
	        /* -------------------------------- */
    	    sprintf(buffer, "sim> *SIG TERM Error (%d) ... !", sig); 
      		LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        	pause(1000);     
	        termExec = 0;
	        break;
	        
		case SIGBUS : 
		    if(opr->wdtDebug)   Debug(console,"sim> *SIG BUS Error (%d)  ... !\n", sig); 
            /* -------------------------------- */
        	/* LOG File 저장                    */
	        /* -------------------------------- */
    	    sprintf(buffer, "sim> *SIG BUS Error  (%d) ... !", sig); 
      		LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        	pause(1000);     	        
            termExec = 0;
		    break;
		    
		case SIGSEGV: 
		    if(opr->wdtDebug)   Debug(console,"sim> *SIG SEGV Error (%d)  ... !\n", sig); 
            /* -------------------------------- */
        	/* LOG File 저장                    */
	        /* -------------------------------- */
    	    sprintf(buffer, "sim> *SIG SEGV Error  (%d) ... !", sig); 
      		LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        	pause(1000); 
            termExec = 0;		 
            simTask_Active = 0;       
		    break;
		    
		case SIGPIPE: 
		    if(opr->wdtDebug)   Debug(console,"sim> *SIG PIPE Error (%d)  ... !\n", sig); 
            /* -------------------------------- */
        	/* LOG File 저장                    */
	        /* -------------------------------- */
    	    sprintf(buffer, "sim> *SIG PIPE Error  (%d) ... !", sig); 
      		LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        	pause(1000);         
			break;
			
		default : 
			if(opr->wdtDebug)   Debug(console,"sim> ... signal generated  (%d) ...!\n", sig);
			/* -------------------------------- */
        	/* LOG File 저장                    */
	        /* -------------------------------- */
    	    sprintf(buffer, "sim> ... signal generated  (%d) ...!", sig);
      		LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   		
        	pause(1000); 	
			break;
	}

}


/*******************************************************************************
 *                                                                        
 * 모듈명:	_clearNetServer()
 *                                                                        
 ******************************************************************************/

void _clearNetServer(void)
{
	if (socketServerFd != ERROR) close(socketServerFd);
	Debug(console,"sim> ....NET 서버 종료 \n");
}


//
// 모듈:	ClearEnvironment()
//
void ClearEnv(void)
{
    /* -------------------------------- */
    /*  SERVER Socket Close ...         */
    /* -------------------------------- */
     _clearNetServer();
    
    iccpShmEnd ();
    
    /* -------------------------------- */
	/*  공유 메모리 설정 해제           */
	/* -------------------------------- */
	if (shmPtr != NULL)     ShmDetach(&shmDesc);

    /* -------------------------------- */
	/*  프로세스 정보를 정리한다        */
	/* -------------------------------- */
	UpdateProcessInfo(taskPtr, 0, getpid(), 0);
			
}
					    

//
// 모듈:	InitEnvironment()
//
int InitEnv( void)
{
    int  i;
    
	// 공유 메모리 상태 확인
	if (ShmCheck(&shmDesc) < 0)
	{
		printf("sim> *ERR_공유 메모리 상태 이상\n");
		return(0);
	}
	
	//printf("sim> SHM %p \n", shmDesc.address);
	
	// 포인터 변수 초기화
	shmPtr = (SHM_MEMORY *) shmDesc.address;
	taskPtr = (TASK_INFO *) &shmPtr->taskInfo[SIM_PROCESS];

	rtc     = (RTC *)       &shmPtr->rtc;
	opr     = (OPR_MSG *)   &shmPtr->opr_msg;
    
    linkCfg = (LINK_MSG *)  &shmPtr->link_msg;
    
	rtudb   = (RTU_DATABASE *) &shmPtr->rtuDatabase;
    console = (CONSOLE_INFO *) &shmPtr->console; 
	hque    = (HISTORY_QUE *)  &shmPtr->localHistoryQ; 
	
    mpuCFG  = (MPU_CONFIG *) &shmPtr->mpuConfig;
    scuCfg 	= (SCU_MSG *)   &shmPtr->scu_msg;                   // 이중화절체장치(SCU) 구조체
    iccpDCB = (ICCP_DCB *) &shmPtr->iccpDCB;                    // ICCP_DCB 
    
    /* ------------------------------------ */
    /*  ICCP 공유메모리 초기화              */
    /* ------------------------------------ */
    iccpShmInit (iccpDCB);
    
    /* ------------------------------------ */
    /* ESIO CFG : ESIO 구조체 (MMAX 5)      */
    /* HOST_DCB : 상위 HOST 구조체 (MMAX 8) */
    /* SCAN CONFIG : SCAN 구조체(MAX 16)    */
    /* 계전기 구조체 : 전체 계전기(MAX 64)  */
    /* 포인트 구조체 : 전체 포인트(4096)    */
    /* ------------------------------------ */ 
    for(i=0; i< MAX_ESIO; i++)          esioCFG[i] = (ESIO_CONFIG *) &shmPtr->esioConfig[i];
    for(i=0; i< MAX_HOST; i++)          hostDCB[i] = (HOST_DCB *) &shmPtr->hostDCB[i];
    for(i=0; i< MAX_SCAN_PORT; i++)     scanCFG[i] = (SCAN_CONFIG *) &shmPtr->scanCFG[i];
    for(i=0; i< MAX_DEVICE; i++)        deviceCFG[i] = (SDP_DEVICE *) &shmPtr->deviceCFG[i];
	for(i=0; i< MAX_DEV_POINT; i++)     devPtBuf[i] = (POINT_BUF *) &shmPtr->devPtBuf[i];
	
    /* ------------------------------------------------------------ */
	/* 프로세스 정보를 초기화한다                                   */
	/* SIM Task 는 TCPIP 전송대기를 고려하여 WDT 기능을 삭제함...  */
	/* ------------------------------------------------------------ */
	taskPtr->initial    = 1;
	taskPtr->wdtEnable  = 0;            
	
	/* ------------------------------------------------ */
	/*  PROCESS 호출시... Priority 지정 (-19 ~ 20)      */
	/* ------------------------------------------------ */
	nice(NICE_SIM);
	
	UpdateProcessInfo(taskPtr, 1, getpid(), 1);
	
	return(1);
}




/*******************************************************************************
 *                                                                        
 * 모듈명:	_initNetpServer()
 *                                                                        
 ******************************************************************************/
int _initNetServerSIM(void)
{
	int		flag = 1, length;
	char	address[64];
	int		port;
    
    /* ------------------------------------------------------------ */
    /*  SERVER-소켓의 Open : IPv4 기반, 연결지향형 소켓, TCP기반    */
    /* ------------------------------------------------------------ */
	if ((socketServerFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == ERROR)
	{
	    if(opr->simDebug)   Debug(console,"sim> ***[NET] socket 실패 \n");
		return(ERROR);
	}
	
	if(opr->simDebug)	Debug(console,"sim> socket Open OK ...%d \n", socketServerFd);

    /* ------------------------------------------------ */
    /*  SOCKET Option...                                */
    /* ------------------------------------------------ */
    setsockopt(socketServerFd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(int));
    setsockopt(socketServerFd, SOL_SOCKET, SO_KEEPALIVE, (char *)&flag, sizeof(int));
    setsockopt(socketServerFd, SOL_SOCKET, SO_DONTROUTE, (char *)&flag, sizeof(int));
    
    //ioctl(socketServerFd, FIONBIO, (int)&flag);

    /* ------------------------------------------------ */    
    /*  소켓 구조체 초기화 ...                          */
    /* ------------------------------------------------ */    
    length = sizeof(struct sockaddr_in);
	bzero((char *)&srvAddr, length);
	srvAddr.sin_family  = AF_INET;                      /* IPv4 기반의 인테넷 프로토콜 Family*/
	srvAddr.sin_port    = htons(SIMULATOR_NET_PORT);    /* Port   정보의 Network Byte Order 로 변경함. */
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);        /* IP주소 정보의 Network Byte Order 로 변경함. */

    /* ------------------------------------------------ */    
    /*  소켓에 주소를 할당함...                         */
    /* ------------------------------------------------ */    
    length = sizeof(struct sockaddr_in);
	if (bind(socketServerFd, ( struct sockaddr *)&srvAddr, length) == ERROR)
	{
	    if(opr->simDebug) Debug(console,"sim> ***[NET] bind 실패 \n");
		return(ERROR);
	}
	if(opr->simDebug) Debug(console,"sim> [NET] bind OK \n");

    /* ------------------------------------------------ */    
    /*  연결요청 대기상태로 진입...                     */
    /* ------------------------------------------------ */    
	if (listen(socketServerFd, 5) == ERROR)
	{
	    if(opr->simDebug)	Debug(console,"sim> ***[NET] listen 실패\r\n");
		return(ERROR);
	}

    if(opr->simDebug)	Debug(console,"sim> [NET] listen OK \n");

// 2026-06-02 오후 5:18:04   socket 과 accept 분리
//    return socketServerFd ;

    /* ------------------------------------------------ */    
    /*  ACCEPT() 진입...                                */
    /* ------------------------------------------------ */    

#if 1    
	length = sizeof(struct sockaddr_in);
    if ((simSocketID = accept(socketServerFd, ( struct sockaddr *)&clientPtr, (socklen_t *) &length)) == ERROR)
    {
        return (ERROR);
    }
    
    
    port = ntohs(clientPtr.sin_port);
   	sprintf(address, "%s", inet_ntoa(clientPtr.sin_addr));
    		
   	if(opr->simDebug)   
   	Debug(console,"sim> accept O.K...%s... port = %d, socket = %d\n", address, port, simSocketID);

 
	return(simSocketID);
	
#endif	
}

int tk_initNetServerSIM(void)
{
	int		flag = 1, length;
	char	address[64];
	int		port;
    
    /* ------------------------------------------------------------ */
    /*  SERVER-소켓의 Open : IPv4 기반, 연결지향형 소켓, TCP기반    */
    /* ------------------------------------------------------------ */
	if ((socketServerFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == ERROR)
	{
	    if(opr->simDebug)   Debug(console,"sim> ***[NET] socket 실패 \n");
		return(ERROR);
	}
	
	if(opr->simDebug)	Debug(console,"sim> socket Open OK ...%d \n", socketServerFd);

    /* ------------------------------------------------ */
    /*  SOCKET Option...                                */
    /* ------------------------------------------------ */
    setsockopt(socketServerFd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(int));
    setsockopt(socketServerFd, SOL_SOCKET, SO_KEEPALIVE, (char *)&flag, sizeof(int));
    setsockopt(socketServerFd, SOL_SOCKET, SO_DONTROUTE, (char *)&flag, sizeof(int));
    
    //ioctl(socketServerFd, FIONBIO, (int)&flag);

    /* ------------------------------------------------ */    
    /*  소켓 구조체 초기화 ...                          */
    /* ------------------------------------------------ */    
    length = sizeof(struct sockaddr_in);
	bzero((char *)&srvAddr, length);
	srvAddr.sin_family  = AF_INET;                      /* IPv4 기반의 인테넷 프로토콜 Family*/
	srvAddr.sin_port    = htons(SIMULATOR_NET_PORT);    /* Port   정보의 Network Byte Order 로 변경함. */
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);        /* IP주소 정보의 Network Byte Order 로 변경함. */

    /* ------------------------------------------------ */    
    /*  소켓에 주소를 할당함...                         */
    /* ------------------------------------------------ */    
    length = sizeof(struct sockaddr_in);
	if (bind(socketServerFd, ( struct sockaddr *)&srvAddr, length) == ERROR)
	{
	    if(opr->simDebug) Debug(console,"sim> ***[NET] bind 실패 \n");
		return(ERROR);
	}
	if(opr->simDebug) Debug(console,"sim> [NET] bind OK \n");

    /* ------------------------------------------------ */    
    /*  연결요청 대기상태로 진입...                     */
    /* ------------------------------------------------ */    
	if (listen(socketServerFd, 5) == ERROR)
	{
	    if(opr->simDebug)	Debug(console,"sim> ***[NET] listen 실패\r\n");
		return(ERROR);
	}

    if(opr->simDebug)	Debug(console,"sim> [NET] listen OK \n");

// 2026-06-02 오후 5:18:04   socket 과 accept 분리
    return socketServerFd ;

}


/*
*   SIMULATOR 상태정보 추출 - 이벤트 유무
*/
void checkSIM_sts()
{
    /* SDP Event 갯수 check... */
    opr->simEventNum = (opr->simFront - opr->simRear + HISTORY_QUE_MAX) % HISTORY_QUE_MAX;
    if(opr->simEventNum > 0)    opr->simStatus = 0x01;
    else                        opr->simStatus = 0x00;     
    
    //printf("sim> CU Event num ... %d %d => %d\n", opr->simFront, opr->simRear,opr->simEventNum);
            
}

static char scanModType[10][8]   = {{"*NOT "}, {" SIO "},{"ESIO1"}, {"ESIO2"}, {"ESIO3"}, {"ESIO4"}, {" RTU"}, {" MPU#1"}, {" MPU#2"}};


/*
*
*/
int check_device_point(POINT_BUF *diPoint)
{
    int iccpPoint = -1;
    
    /* ---------------------------------------- */
    /*  계전기 Type == DEVICE 인경우...         */
    /* ---------------------------------------- */
    if(diPoint->ptType == RALLPT)        iccpPoint = 0;                  // 가상 포인트 : 전체 계전기 통신상태
    else if(diPoint->ptType == RDEVPT)   iccpPoint = diPoint->devNo;     // 가상 포인트 : 개별 계전기 통신상태 , 계전기 번호,1,2,3...      
    else if(diPoint->ptType == MCPUPT)   iccpPoint = INDEX_AUTO_MANUAL;  // 가상 포인트 : 자동/수동 상태
    else if(diPoint->ptType == CNTRPT)   iccpPoint = INDEX_CNTR_CHANGE;  // 가상 포인트 : 이중화 절체제어
    else if(diPoint->devType == SDP_STS_POINT)        
    {
        if(diPoint->ptType == MCPUPT)            iccpPoint = INDEX_SDP_TIMESYNC;    // 가상 포인트 : SDP 시각동기 요청(상태) 
        else if(diPoint->ptType == SCPUPT)       iccpPoint = INDEX_SDP_STS;         // 가상 포인트 : SDP-STATUS (상태) 
        else                                    iccpPoint = INDEX_SDP_STS;         // 가상 포인트 : SDP-STATUS (상태)     
    }            
    else if(diPoint->devType == CPU_RUN_POINT)   iccpPoint = INDEX_SDP_RUN_MODE;    // 가상 포인트 : CPU 동작정보 ,[0] 1계, [1] 2계 동작정보  
    else if(diPoint->devType == CPU_STS_POINT)  
    {
        if(diPoint->ptType == MCPUPT)            iccpPoint = INDEX_SDP_RUN_A;        // 가상 포인트 : CPU 1계 상태정보, [0] 정상,[1] 이상
        else if(diPoint->ptType == SCPUPT)       iccpPoint = INDEX_SDP_RUN_B;        // 가상 포인트 : CPU 2계 상태정보, [0] 정상,[1] 이상    
    }
    else if(diPoint->devType == RTU_RUN_POINT)   iccpPoint = INDEX_RTU_MODE;         // 가상 포인트 : RTU 동작정보 ,[0] 1계, [1] 2계 동작정보 
    else if(diPoint->devType == RTU_STS_POINT)  
    {
        if(diPoint->ptType == MCPUPT)            iccpPoint = INDEX_RTU_RUN_A;        // 가상 포인트 : RTU 1계 상태정보, [0] 정상,[1] 이상
        else if(diPoint->ptType == SCPUPT)       iccpPoint = INDEX_RTU_RUN_B;        // 가상 포인트 : RTU 2계 상태정보, [0] 정상,[1] 이상  
    }
    else if(diPoint->devType == CU_RUN_POINT)    iccpPoint = INDEX_SCADA_MODE;       // 가상 포인트 : 전철제어반 동작정보 ,[0] 1계, [1] 2계 동작정보 
    else if(diPoint->devType == CU_STS_POINT)  
    {
        if(diPoint->ptType == MCPUPT)            iccpPoint = INDEX_SCADA_RUN_A;      // 가상 포인트 : 전철제어반 1계 상태정보, [0] 정상,[1] 이상
        else if(diPoint->ptType == SCPUPT)       iccpPoint = INDEX_SCADA_RUN_B;      // 가상 포인트 : 전철제어반 2계 상태정보, [0] 정상,[1] 이상
    }
    else if(diPoint->devType == DIG_RUN_POINT)   iccpPoint = INDEX_REMOTE_MODE;      // 가상 포인트 : 원격진단부 동작정보 ,[0] 1계, [1] 2계 동작정보 
    else if(diPoint->devType == DIG_STS_POINT)  
    {
        if(diPoint->ptType == MCPUPT)            iccpPoint = INDEX_REMOTE_RUN_A;     // 가상 포인트 : 원격진단부 1계 상태정보, [0] 정상,[1] 이상
        else if(diPoint->ptType == SCPUPT)       iccpPoint = INDEX_REMOTE_RUN_B;     // 가상 포인트 : 원격진단부 2계 상태정보, [0] 정상,[1] 이상
    }
    else if(diPoint->devType == EQM_RUN_POINT)   iccpPoint = INDEX_ELECQ_MODE;       // 가상 포인트 : 전력품질부 동작정보 ,[0] 1계, [1] 2계 동작정보 
    else if(diPoint->devType == EQM_STS_POINT)  
    {
        if(diPoint->ptType == MCPUPT)            iccpPoint = INDEX_ELECQ_RUN_A;      // 가상 포인트 : 전력품질부 상태정보, [0] 정상,[1] 이상
        else if(diPoint->ptType == SCPUPT)       iccpPoint = INDEX_ELECQ_RUN_B;      // 가상 포인트 : 전력품질부 상태정보, [0] 정상,[1] 이상
    }
    else if(diPoint->devType == IEC_RUN_POINT)   iccpPoint = INDEX_61850_MODE;       // 가상 포인트 : 61850 동작정보 ,[0] 1계, [1] 2계 동작정보 
    else if(diPoint->devType == IEC_STS_POINT)  
    {
        if(diPoint->ptType == MCPUPT)            iccpPoint = INDEX_61850_RUN_A;      // 가상 포인트 : 61850 1계 상태정보, [0] 정상,[1] 이상
        else if(diPoint->ptType == SCPUPT)       iccpPoint = INDEX_61850_RUN_B;      // 가상 포인트 : 61850 2계 상태정보, [0] 정상,[1] 이상
    }
    
    return (iccpPoint);    
}

/**
**  SIMULATOR - 응답메세지 구성
**/
int mkResponseSIM( byte *txbuf, byte *rxbuf, int rxcnt)
{
    int     i;
    int     iccpIndex=0;
    short   opcode, ioid;
    short   scanid, targetID;
    short   count, sndCount, rcvAckNum;
    short   point, stspoint, anapoint, rptpoint;
    short   dataLen, dbSize;
    short   cntrPoint, cntrType, cntrSts;
    short   esioId, esioNum;
    short   devid, devNum;
    short   startPt, ptNum;
    //short	diDumpPt, aiDumpPt, reportSum;
    short	dumpPoint;
    
    word    rear, eventNum;
    byte    *bfptr;
    float   aiData, *aiPtr;
    char    buffer[256];
    struct  timeval ctime;
    
    SCAN_CONFIG     *scan;          // SCAN 구조체    
    SDP_DEVICE      *dev;           // 계전기 구조체
    ESIO_CONFIG     *esio;          // ESIO 구조체    
    POINT_BUF       *relayPoint;    // 계전기-포인트 구조체
    POINT_BUF       *diPoint;
    SYSLOG_FORM     *event;         // EVENT-Q 구조체
    MPU_SOEQ_ENTRY  mpuEvent;      // MPU SOE-Q Event
    
    DB_MPU_CONFIG   *dbMPU;         // DB-MPU Config
    DB_ESIO_CONFIG  *dbESIO;        // DB-ESIO Config
    DB_ICCP_CONFIG  *dbICCP;        // DB ICCP Config
    DB_HOST_CONFIG  *dbHOST;
    DB_SCAN_CONFIG	*dbScan;
    DB_SDP_DEVICE	*dbDev;
    DB_MODBUS_PROFILE   *dbModbus;

    /* -------------------------------- */
    /* OPCODE Check...                  */    
    /* -------------------------------- */
    opcode = rxbuf[4] & 0x7f;
    opr->rcvSimSeq = rxbuf[6];
    
    dataLen = rxcnt - 8;            // except 8 Byte : STX(2)/SIZE(2)/OPCODE(1)/Control(1)/SeqNo(1)/LRC*(1) */

#if 0
    /* -------------------------------- */
    /*  이벤트 전송                     */
    /* -------------------------------- */    
    if((opcode != SIM_EVENT_ACK) && (opr->simEventNum > 0))
    {
        opr->rcvSimSeq++;
        opcode = SIM_EVENT_DUMP;
    }
#endif
         
    /* -------------------------------- */
    /* 송신 Frame 구성                  */
    /* -------------------------------- */
    txbuf[0] = 0x7e;                        // stx
    txbuf[1] = 0x7e;                        // stx
    txbuf[2] = 0;                           // size
    txbuf[3] = 0;                           // size 
    txbuf[4] = opcode;                      // opcode
    txbuf[5] = opr->simStatus;              // status 
    txbuf[6] = opr->rcvSimSeq;              // 수신 SeqNo
    count = 7;


   	if(opr->simDebug)   
   	    Debug(console,"sim> OPCODE [%d]\r\n", opcode);

	switch(opcode)
	{
    case SIM_SDP_STATUS:        // 0x01 SDP 장치상태정보 Dump
        /* ---------------------------------------- */
        /*  SDP 상태정보 추출 및 전송               */
        /* ---------------------------------------- */
        if(opr->cpuMode == MPU_A)
        {
            txbuf[count++] = mpuCFG->mpuStatus;         // Master-MPU 상태정보
            txbuf[count++] = mpuCFG->mpuRackSts;        // Master-MPU RACK 상태정보
            txbuf[count++] = mpuCFG->rcvMpuSts;         // Slave-MPU 상태정보
            txbuf[count++] = mpuCFG->rcvRackSts;        // Slave-MPU RACK 상태정보
        }
        else
        {
            txbuf[count++] = mpuCFG->rcvMpuSts;         // Master-MPU 상태정보
            txbuf[count++] = mpuCFG->rcvRackSts;        // Master-MPU RACK 상태정보
            txbuf[count++] = mpuCFG->mpuStatus;         // Slave-MPU 상태정보
            txbuf[count++] = mpuCFG->mpuRackSts;        // Slave-MPU RACK 상태정보
        }
                
        /* ---------------------------------------- */
        /*  HOST & DEVICE 상태정보 추출 및 전송     */
        /* ---------------------------------------- */
        // HOST 통신상태 : [0] 미정의, [1]주, [2]예비, [3] 통신이상 
        
        if(iccpDCB->assocStatus)
    	{    
        	if(iccpDCB->commMaster)
        	{
        		if(opr->cpuMode == MPU_A)	hostDCB[7]->runStatus = 1;
        		else						hostDCB[7]->runStatus = 2;		
        	}
        	else
        	{
        		if(opr->cpuMode == MPU_A)	hostDCB[7]->runStatus = 1;
        		else						hostDCB[7]->runStatus = 2;		
        	}  
        }
        else
        {
        	hostDCB[7]->runStatus = 3;
        }
        
        for(i=0; i < MAX_HOST; i++)     
        {
            txbuf[count++] = hostDCB[i]->runStatus;         // HOST# 통신 운영 상태
        }
        
        for(i=0; i < MAX_DEVICE; i++)   
        {
            txbuf[count++] = deviceCFG[i]->runStatus;       // 계전기# 통신 운영 상태
        }
        
            
        break;
        
    case SIM_EVENT_DUMP:        // 0x02 SDP EVENT Dump
        
        /* SIMULATOR 연계 : EVENT Check... */
        eventNum = opr->simEventNum;
        if(eventNum > 32)   eventNum = 32;
        
        if(opr->simDebug)
        Debug(console,"sim> snd EVENT dump : reported index=%d, Event count=%d ... \n", opr->simRear, eventNum);
        
        /* ---------------------------------------- */
        /*  EVENT 정보 추출 및 전송                 */
        /* ---------------------------------------- */
        txbuf[count++] = eventNum;     						// Event Report 수  
        
        for(i=0; i < eventNum; i++)
        {
            rear = (opr->simRear + i) & HISTORY_QUE_MASK;
            event = (SYSLOG_FORM *) &hque->queue[rear];
            
            memcpy( &txbuf[count], (byte *) event, sizeof(SYSLOG_FORM));
            count += sizeof(SYSLOG_FORM);
        }
        
        opr->simReport = eventNum;

        break;
        
    case SIM_EVENT_ACK:         // 0x03 SDP EVENT-ACK
            
        rcvAckNum = rxbuf[7];
        
        if(opr->simDebug)
        Debug(console,"sim> rcv EVENT ACK : Rcv= %d, Report = %d\n", rcvAckNum, opr->simReport);
        
        if(opr->simReport == rcvAckNum) 
        {
            opr->simRear = (opr->simRear + rcvAckNum ) & HISTORY_QUE_MASK;
            opr->simReport =0;
        }
        
        
        return (0);
        break;
               
    case SIM_POINT_SCAN:        // 0x04 계전기 상태 POINT 정보 Scan
        ioid = (rxbuf[7] - 1) & 0x3f;
        
        // 2026-06-01 오후 4:19:29
   	    if(opr->simDebug)   
   	    Debug(console,"sim> SIM_POINT_SCAN dev[%d]\r\n", ioid+1);
        
        dev  = (SDP_DEVICE *) deviceCFG[ioid];
 
 
        
        
        /* 수신 계전기번호 Check...[0...63] */
		if((ioid < 0) || (ioid >= MAX_DEVICE))
        {
            if(opr->simDebug)
    	    Debug(console, "sim> *rcv POINT SCAN : Invalid Device ID = %d \n", rxbuf[7]);
            return (0);
        }

        txbuf[count++] = ioid + 1;		        // Module No
        txbuf[count++] = 0;				        // Index = 8, 전송되는 point Num
        txbuf[count++] = 0;				        // Index = 9, 전송되는 point Num
        
        stspoint = 0;	
        anapoint = 0;
        dumpPoint= 0;
        
        /* --------------------------------------- */
        /* 상태포인트 정보 : 포인트당 4바이트 표현 */
        /* --------------------------------------- */
        for(point=0; point< MAX_DEV_DI_POINT; point++)  // DI가 1024 인데...
        {
            relayPoint = (POINT_BUF *) &dev->diPtBuf[point];    
            rptpoint = point + 1;
            
            if(relayPoint->config == SET)  // 이건 DB 의 config가 아니고.. 내가 처리했다는
            {
                txbuf[count++]  = 0;	                        // 포인트 Type -   [0] 상태포인트, [1] 아날로그포인트
                txbuf[count++]  = relayPoint->config;	        // 포인트 Config - [0] 사용않함,   [1] 사용
                
            	txbuf[count++]  = (rptpoint >> 8) & 0xff;	    // MSB - 상태포인트 지정 
            	txbuf[count++]  = rptpoint & 0xff;	            // LSB - 상태포인트 
            	
                txbuf[count++]  = 0;
                txbuf[count++]  = 0;
                txbuf[count++]  = 0;
                txbuf[count++]  = relayPoint->status & 0xff;
                stspoint++;
                
                //printf("sim>> status point... %d... sts = %d\n", rptpoint, relayPoint->status);
            }
        }

#if 0


printf("dev           : %p, align = %lu\n",        dev, (unsigned long)dev % 4);

printf("aiPtBuf       : %p, align = %lu\n",        dev->aiPtBuf, (unsigned long)dev->aiPtBuf % 4);

printf("floatData[0]  : %p, align = %lu\n",        &dev->aiPtBuf[0].floatData, 
        (unsigned long)&dev->aiPtBuf[0].floatData % 4);
        

printf("sizeof(POINT_BUF) = %zu\n", sizeof(POINT_BUF));
printf("sizeof(aiPtBuf[0]) = %zu\n", sizeof(dev->aiPtBuf[0]));

// i=0 일때와 i=1 일때 주소 차이 확인
printf("&aiPtBuf[0] = %p\n", &dev->aiPtBuf[0]);
printf("&aiPtBuf[1] = %p\n", &dev->aiPtBuf[1]);
printf("floatData[0] align = %lu\n",        (unsigned long)&dev->aiPtBuf[0].floatData % 4);
printf("floatData[1] align = %lu\n",        (unsigned long)&dev->aiPtBuf[1].floatData % 4);
       
#endif        
  


#if 1        
        /* --------------------------------------- */
        /* 계측포인트 정보 : 포인트당 4바이트 표현 */
        /* --------------------------------------- */
//        for(point=0; point< 4; point++)
        for(point=0; point< MAX_DEV_AI_POINT; point++)
        {
//            struct timespec t1, t2;        
//            long elapsed;                
        

#if 1
//            clock_gettime(CLOCK_MONOTONIC, &t1); 
             relayPoint = (POINT_BUF *) &dev->aiPtBuf[point];
             rptpoint = point + 1;
            
            if(relayPoint->config == SET)
            {
                txbuf[count++]  = 1;	                        // 포인트 Type -   [0] 상태포인트, [1] 아날로그포인트
                txbuf[count++]  = relayPoint->config;	        // 포인트 Config - [0] 사용않함,   [1] 사용
                
                txbuf[count++]  = (rptpoint >> 8) & 0xff;	    // MSB - 아날로그 포인트 지정 
            	txbuf[count++]  = rptpoint & 0xff;	            // LSB - 아날로그 포인트 
            	
            	/* FLOAT 형으로 데이터 보고... */
            	// test 
            	relayPoint->floatData = point*10 ;
            	  
            	bfptr = (byte *) &relayPoint->floatData;
            	
#if 0 // 기존  fvalue 값이 big endian 이었다.            	
                txbuf[count++]  = (bfptr[0]) & 0xff;
                txbuf[count++]  = (bfptr[1]) & 0xff;
                txbuf[count++]  = (bfptr[2]) & 0xff;
                txbuf[count++]  = (bfptr[3]) & 0xff;
                anapoint++;
#else 
                txbuf[count++]  = (bfptr[3]) & 0xff;
                txbuf[count++]  = (bfptr[2]) & 0xff;
                txbuf[count++]  = (bfptr[1]) & 0xff;
                txbuf[count++]  = (bfptr[0]) & 0xff;
                anapoint++;

#endif
                
                if(opr->simDebug)
                Debug(console, "dev%2d> ANA point = %2d, data= %4.2f\n", ioid+1, point+1, relayPoint->floatData);
            
//            clock_gettime(CLOCK_MONOTONIC, &t2);
//            elapsed = (t2.tv_sec - t1.tv_sec) * 1000000000L
//                    + (t2.tv_nsec - t1.tv_nsec);
//            printf("old way printf time: %ld ns\n", elapsed); 
            // 2000 ~ 2333ns  나오고  가장작ㅇ을때 1000ns 나옴            
            
            }
#else

            char *ptrChar ;
            size_t offset;
            float   fvalue ;
            unsigned char     config ;
            
//            POINT_BUF   local_PointBuf;        
 
// aiPtBuf[i] 자체를 char*로 직접 접근
              ptrChar = (char *)&dev->aiPtBuf[ point ];

// floatData가 구조체 내 몇 번째 바이트인지 offset 계산
              offset = offsetof(POINT_BUF, floatData);
              memcpy(&fvalue, ptrChar + offset, sizeof(float));

              fvalue = point*10;
// config 가 구조체내  몇 번째 바이트인지 offset 계산
              offset = offsetof(POINT_BUF, config);
              memcpy(&config, ptrChar + offset, sizeof(byte));
//              printf("[%3d]\r\n", point+1, fvalue);
//            printf("[%3d] %4.2f ", i+1, fvalue);
            
//             memcpy(&local_PointBuf, ptrChar, sizeof(POINT_BUF));


            // old way
//             clock_gettime(CLOCK_MONOTONIC, &t1);           
//            printf("[%3d]. %d-%4.2f\r\n", point+1, config, fvalue );     

            
            rptpoint = point + 1;
           // printf("[%3d]. %d-%4.2f - count[%d]\r\n", point+1, config, fvalue,count );           
            
            if( config == SET)
            {
                txbuf[count++]  = 1;	                        // 포인트 Type -   [0] 상태포인트, [1] 아날로그포인트
                txbuf[count++]  = config;	                    // 포인트 Config - [0] 사용않함,   [1] 사용
                
                txbuf[count++]  = (rptpoint >> 8) & 0xff;	    // MSB - 아날로그 포인트 지정 
            	txbuf[count++]  = rptpoint & 0xff;	            // LSB - 아날로그 포인트 
            	
            	/* FLOAT 형으로 데이터 보고... */
            	bfptr = (byte *) &fvalue;
            	
#if 0 // 기존  fvalue 값이 big endian 이었다.            	
                txbuf[count++]  = (bfptr[0]) & 0xff;
                txbuf[count++]  = (bfptr[1]) & 0xff;
                txbuf[count++]  = (bfptr[2]) & 0xff;
                txbuf[count++]  = (bfptr[3]) & 0xff;
                anapoint++;
#else 
                txbuf[count++]  = (bfptr[3]) & 0xff;
                txbuf[count++]  = (bfptr[2]) & 0xff;
                txbuf[count++]  = (bfptr[1]) & 0xff;
                txbuf[count++]  = (bfptr[0]) & 0xff;
                anapoint++;

#endif                 
                
                
                if(opr->simDebug)
                Debug(console, "dev%2d> ANA point = %2d, data= %4.2f  Count[%d]\n", ioid+1, point+1, fvalue,count);
//                Debug(console, "dev%2d> ANA point = %2d, data= %4.2f  Count[%d]\n", ioid+1, point+1, relayPoint->floatData,count);
            }

  
#endif          
        }
                

                   
#endif 
		if(opr->simDebug)
        Debug(console, "=> rcv Device(%2d) Scan...dump init \n", ioid + 1);
		
        /* -------------------------------- */
        /*  전송되는 포인트 수 표시         */
        /* -------------------------------- */
        dumpPoint = stspoint + anapoint;
        txbuf[8] = (dumpPoint >> 8) & 0xff;
        txbuf[9] = dumpPoint & 0xff;
        
        if(opr->simDebug)
        Debug(console, "=> rcv Device(%2d) Scan...STS(%2d/%2d)-%d, ana(%2d/%2d)-%d\n", ioid + 1, dev->devDiPoint, dev->regDiPointNum, stspoint, dev->devAiPoint, dev->regAiPointNum, anapoint);
        break;
        
    case SIM_POINT_CNTR:        // 0x05 계전기 POINT 제어

        /* 수신되는 제어정보 참조 */
        ioid 	  = (rxbuf[7] - 1) & 0x3f;
        cntrPoint = (rxbuf[8] - 1) & 0xff;
        cntrType  = rxbuf[9];                       // 제어상태정보 : [0] TRIP, [1]CLOSE
        
        /* 수신 계전기번호 Check...[0...63] */
		if((ioid < 0) || (ioid >= MAX_DEVICE))
        {
            //if(opr->simDebug)
    	    Debug(console, "sim> *rcv POINT Control : Invalid Device ID = %d \n", rxbuf[7]);
            return (0);
        }
        
		if((cntrType != 0) && (cntrType != 1))
		{
			Debug(console, "sim> *** Invalid POINT-CNTR... Dev=%d, Point=%2d, Control=%2x\n", ioid+1, cntrPoint+1, cntrType);
			return 0;
		}	
		
		/* ---------------------------------------- */
		/*  계전기 정보 추출 ...                    */
		/* ---------------------------------------- */
        dev = (SDP_DEVICE *) deviceCFG[ioid];
        scanid = dev->scanPort - 1;             // 계전기 할당 scanid [1 ~ 16]

        /* ---------------------------------------- */
        /* SCAN-ID Check... */
        /* ---------------------------------------- */
        if((scanid < 0) || (scanid >= MAX_SCAN_PORT))
        {
            Debug(console, "sim> *** [dev=%2d/point=%2d/stat=%d] rcv Control Fail => Invalid SCAN-ID = %d... \n", ioid+1, cntrPoint+1, cntrType, scanid);   
            txbuf[count++] = ioid + 1;		    // 제어 모듈   [1..64] */
       	    txbuf[count++] = cntrPoint + 1;		// 제어 포인트 [1..1024]
       	    txbuf[count++] = cntrType;		    // 제어상태정보 : [0] TRIP, [1]CLOSE
       	    txbuf[count++] = 0;                 // 제어상태 : [0] 이상, [1] 실행 
       	    break;
        }         
                			  
        /* ---------------------------------------- */
        /* SCAN 할당 Target Board Check...          */
        /* ---------------------------------------- */
        scan = (SCAN_CONFIG *) scanCFG[scanid];
        targetID = scan->targetID;              // SDP SCAN : ESIO Target Module-ID, 0:사용않함, 1: MPU, 2: ESIO1, 3:ESIO2, 4:ESIO3, 5:ESIO4, 6:SIO
        
        if((targetID <= 0) || (targetID > SDP_MPU_2))
        {
            Debug(console, "sim> *** [dev=%2d/point=%2d/stat=%d] rcv Control Fail => Invalid TARGET-ID = %d... \n", ioid+1, cntrPoint+1, cntrType, targetID);   
            txbuf[count++] = ioid + 1;		    // 제어 모듈   [1..64] */
       	    txbuf[count++] = cntrPoint + 1;		// 제어 포인트 [1..1024]
       	    txbuf[count++] = cntrType;		    // 제어상태정보 : [0] TRIP, [1]CLOSE
       	    txbuf[count++] = 0;                 // 제어상태 : [0] 이상, [1] 실행 
       	    break;
        }         
        
        if(opr->soeDebug)
        Debug(console, "sim> rcv POINT-CNTR : Dev=%d, Point=%d, TCF[TRIP=0, CLOSE=1] =%2x...SCAN-ID=%d, Target=%s \n", ioid+1, 
            cntrPoint+1, cntrType, scanid+1, scanModType[targetID]);
            
        /* ------------------------------------ */
        /* ESIO 제어정보 연계...                */
        /* ------------------------------------ */
        gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
        logEvent_MPU(shmPtr, ENT_SIM_CNTR, ioid+1, cntrPoint+1, cntrType+1, opr->cpuMode, &ctime);
        
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "<-- SIM> CNTR: Dev=%d, PT=%d, TCF[TRIP=0, CLOSE=1]= %2x...SCAN-ID=%d, Target=%s ", 
            ioid+1, cntrPoint+1, cntrType, scanid+1, scanModType[targetID]);
        LogFile_MPU (shmPtr, ENT_SIM_CNTR, buffer, strlen(buffer));            

		controlInfo_MPU( shmPtr, ioid + 1, cntrPoint + 1, cntrType + 1, PASS_SIM_CNTR, &ctime);
                                
       	txbuf[count++] = ioid + 1;		    // 제어 모듈   [1..64] */
       	txbuf[count++] = cntrPoint + 1;		// 제어 포인트 [1..1024]
       	txbuf[count++] = cntrType;		    // 제어상태정보 : [0] TRIP, [1]CLOSE
       	txbuf[count++] = SET;               // 제어상태 : [0] 이상, [1] 실행
        break;
        
        
    case SIM_SYSTEM_CNTR:       // 0x06 SYSTEM 절체 제어
        //if(opr->simDebug)
        Debug(console, "sim> rcv SYSTEM Change ...!\n");
        
        cntrSts = 0;
        
        if((opr->dualCpuSts == SET) && (scuCfg->remoteMode == AUTO_MODE))
        {    
            opr->cpuChange = SET;    
            cntrSts = SET;
            
            logEvent_MPU(shmPtr, ENT_CHANGE_CPU, CPU_CHG_SIM, 0, 0, opr->cpuMode, NULL);
        
            /* -------------------------------- */
            /* LOG File 저장                    */
            /* -------------------------------- */
            sprintf(buffer, "<-- USER> CPU CHANGE cmd ... " );
            LogFile_MPU (shmPtr, ENT_CHANGE_CPU, buffer, strlen(buffer));        
        }         
        txbuf[count++] = cntrSts;       // 이중화 절체 제어상태, [0] 실패, [1] 실행
        txbuf[count++] = 0;             // Reserved
                
        break;
   
    case SIM_MANUAL_STATUS:     //0x07 수동기입 - 상태포인트 
        
        /* 수신되는 제어정보 참조 */
        ioid 	  = rxbuf[7];                       // 계전기 번호, 1,2,...64
        cntrPoint = rxbuf[8] *256 + rxbuf[9];       // 포인트 번호, 1,2...1024
        cntrType  = rxbuf[10];                      // 포인트 상태, [0] TRIP, [1] CLOSE
        
        //if(opr->simDebug)
        Debug(console, "sim> rcv MANUAL-STATUS : dev=%2d, point=%3d... state=%d\n", ioid, cntrPoint, cntrType);

        /* DEVIE 포인트 Check.... */
        dev = (SDP_DEVICE *) deviceCFG[ioid-1];
        diPoint = (POINT_BUF *) &dev->diPtBuf[cntrPoint -1 ];
        
        if(diPoint->devType == DI_POINT)
        { 
            /* ---------------------------------------- */
            /*  수동기입-상태포인트 SOE 생성 ...        */
            /* ---------------------------------------- */    
            mpuEvent.eventCode = ENT_SOE;           // 계전기 포인트 SOE 정보
            mpuEvent.devNo     = ioid;              // SDP POINT : device 번호 [1..32] 
            mpuEvent.pointNo   = cntrPoint;         // SDP POINT : device 포인트 번호 [1..1024] 
            mpuEvent.state     = cntrType;
            mpuEvent.esioNo    = opr->cpuMode;
            gettimeofday(&mpuEvent.updateTime, NULL);       //정보 갱신시간 추출...timeval  Form  
                
            store_MPU_SOE( shmPtr, &mpuEvent);
        }
        else if(diPoint->devType >= DEV_POINT)
        {
            iccpIndex = check_device_point(diPoint);
            if(iccpIndex >= 0)
            {
                /* ---------------------------------------- */
                /*  수동기입-상태포인트 SOE 생성 ...        */
                /* ---------------------------------------- */    
                mpuEvent.eventCode = ENT_DEVICE_SOE;    
                mpuEvent.devNo     = ioid;              // SDP POINT : device 번호 [1..32] 
                mpuEvent.pointNo   = iccpIndex+1;       // SDP POINT : device 포인트 번호 [1..1024] 
                mpuEvent.state     = cntrType;
                gettimeofday(&mpuEvent.updateTime, NULL);       //정보 갱신시간 추출...timeval  Form  
                
                store_MPU_Device( shmPtr, &mpuEvent);
            }
                 
            Debug(console, "sim>   rcv DEVICE-Point...point=%d, sts=%d\n", iccpIndex, cntrType );
        }
              
        
        txbuf[count++] = rxbuf[7];		    // 계전기 번호   [1..64] */
   	    txbuf[count++] = rxbuf[8];		    // 포인트 번호 - MSB [1..1024]
   	    txbuf[count++] = rxbuf[9];		    // 포인트 번호 - LSB [1..1024]
   	    txbuf[count++] = rxbuf[10];		    // 포인트 상태 - [0] TRIP, [1]CLOSE
   	    txbuf[count++] = 1;                 // 제어상태 : [0] 이상, [1] 실행 
       	    
        break;

    case SIM_MANUAL_ANALOG:     //0x08 수동기입 - 계측포인트 
        ioid 	  = (rxbuf[7] - 1) & 0x3f;                  // 계전기 번호, 1,2,...64
        cntrPoint = (rxbuf[8] *256 + rxbuf[9]) - 1;         // 포인트 번호, 1,2...1024

#if 0        
        aiPtr     = (float *) &rxbuf[10];                   // 포인트 데이터(Float)
        aiData    = *aiPtr;
#else
        memcpy (&aiData, &rxbuf[10] ,sizeof(float) ) ;
        aiPtr = &aiPtr ;        
#endif         
        //if(opr->simDebug)
        Debug(console, "sim> rcv MANUAL-ANALOG : dev=%2d, point=%3d... data=%4.2f\n", ioid + 1, cntrPoint + 1, aiData);
        
        /* ------------------------------------ */               
        /* 계측포인트 데이터.. Update ...       */
        /* ------------------------------------ */      
        storeAI_FLOAT_MPU( shmPtr, ioid, cntrPoint, aiData, (byte *)aiPtr);
            
        txbuf[count++] = rxbuf[7];		    // 계전기 번호   [1..64] */
   	    txbuf[count++] = rxbuf[8];		    // 포인트 번호 - MSB [1..1024]
   	    txbuf[count++] = rxbuf[9];		    // 포인트 번호 - LSB [1..1024]
   	    txbuf[count++] = rxbuf[10];		    // 포인트 데이터 (Float)
   	    txbuf[count++] = rxbuf[11];		    // 포인트 데이터 (Float)
   	    txbuf[count++] = rxbuf[12];		    // 포인트 데이터 (Float)
   	    txbuf[count++] = rxbuf[13];		    // 포인트 데이터 (Float)

   	    txbuf[count++] = 1;                 // 제어상태 : [0] 이상, [1] 실행 
        break;
                     
    case SIM_TIME_DOWN:         // 0x11 Time Sync Down
       	opr->year 	= (rxbuf[7] * 256) + rxbuf[8];
    	opr->month 	= rxbuf[9];
    	opr->day 	= rxbuf[10];
    	opr->hour	= rxbuf[11];
    	opr->min	= rxbuf[12];
    	opr->sec	= rxbuf[13];
    	opr->milisec = 0;

		opr->rtcUpdateICCP = SET;		// ICCP-HOST Time-Sync 정보 
    	opr->rtcUpdateFlag = SET;
    	
		/* TIME-SYNC 관련 이벤트 생성 */    	
    	logEvent_MPU(shmPtr, ENT_SIM_TIMESET, 0, 0, 0, opr->cpuMode, NULL);  
    	
    	sprintf(buffer, "<-- SIM> Time SYNC ... rcv : %4d/%2d/%2d %02d:%02d:%02d", opr->year, opr->month, opr->day, opr->hour, opr->min, opr->sec);
        LogFile_MPU (shmPtr, ENT_SIM_TIMESET, buffer, strlen(buffer));     
        
    	//if(opr->simDebug)
    	printf("sim> rcv TIME Sync Down : %4d/%2d/%2d %02d:%02d:%02d \n", opr->year, opr->month, opr->day, opr->hour, opr->min, opr->sec);
        break;
        
    case SIM_TIME_UP:           // 0x12 Time Sync Up
        txbuf[count++]  = (rtc->year >> 8) & 0xff;
        txbuf[count++]  = rtc->year & 0xff;
        txbuf[count++]  = rtc->month;  
        txbuf[count++]  = rtc->day;  
        txbuf[count++]  = rtc->hour;  
        txbuf[count++]  = rtc->min;  
        txbuf[count++]  = rtc->sec;  
        
        //if(opr->simDebug)
    	Debug(console, "sim> rcv TIME Sync UP : %4d/%2d/%2d %02d:%02d:%02d \n", rtc->year, rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
        break;
        
    case SIM_MPUCFG_DOWN:       // 0x20 DB : MPU Config Down


   		dbSize = sizeof(DB_MPU_CONFIG);
		dbMPU = (DB_MPU_CONFIG *) &rtudb->mpuConfig;
		
        if(dataLen != dbSize)   // 받은 것 size 가 틀리면..
        {
            if(opr->simDebug)
    	    Debug(console, "sim> *rcv MPU Config : Invalid Size ... rcv=%d, db=%d \n", dataLen, dbSize);
    	    return (0);
        }

        /* ---------------------------- */        
    	/* 데이터베이스 저장 ...        */
    	/* ---------------------------- */ 
    	memcpy( (byte *) dbMPU, &rxbuf[7], dataLen);
        dbFileWrite(opr, rtudb);
        
        /* ------------------------------------ */
        /*  ICCP 포인트 제어정보 처리...        */
        /* ------------------------------------ */
        gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
        logEvent_MPU(shmPtr, ENT_DB_UPDATE, 0, SIM_MPUCFG_DOWN, 0, 0, &ctime);   
        
        sprintf(buffer, "<-- SIM> Database Down [ SIM_MPUCFG_DOWN ]");
        LogFile_MPU (shmPtr, ENT_DB_UPDATE, buffer, strlen(buffer));     
            
        //if(opr->simDebug)   
        Debug(console, "sim> rcv MPU Config-DB Down ... %d\n", dataLen); 
        
        opr->mpuCfgDown_OK = SET;
        
        /* CPU 이중화 DB 연계 */
        linkCfg->mpuCfgDown = SET;
        
        /* --------------------------------- */
        /* 해당 ESIO SCAN Config-Down Flag   */
        /* --------------------------------- */            
        for(i=0; i < MAX_ESIO; i++)
        {
            esio = (ESIO_CONFIG *) esioCFG[i];
            esio->mpuCfgDown = SET;     
        }
            
        break;
         
    case SIM_MPUCFG_UP:         // 0x21 DB : MPU Config UP
#if 1        
        dbSize = sizeof(DB_MPU_CONFIG);
		dbMPU = (DB_MPU_CONFIG *) &rtudb->mpuConfig;
						
        memcpy(&txbuf[count], (byte *) dbMPU, dbSize);
        count = count + dbSize;    
#else
        dbSize = sizeof(DB_MPU_CONFIG2);
        struct DB_MPU_CONFIG2 mpu_config2;



        memcpy( &mpu_config2.sdpNameStr[0],  (byte *) &rtudb->sdpNameStr[0], 20);                

        // 배열의 이름은 주소...
        memcpy( (byte *)mpu_config2.master_netCfg,  (byte *) rtudb->master_netCfg, sizeof(DB_MNET_ENTRY )*3  );  
        memcpy( (byte *)mpu_config2.slave_netCfg,  (byte *)  rtudb->slave_netCfg, sizeof(DB_MNET_ENTRY )*3  );  

        memcpy( (byte *)&mpu_config2.dualMpu,  (byte *) &rtudb->dualMpu, 16);        

        // 새로추가된..항목
        memcpy( &mpu_config2.master_netCfg[3],  (byte *)&shmPtr->master_netCfg4, sizeof(DB_MNET_ENTRY ) );          
        memcpy( &mpu_config2.slave_netCfg[3],  (byte *)&shmPtr->master_netCfg4, sizeof(DB_MNET_ENTRY )  );         
    
        memcpy(&txbuf[count], (byte *)&mpu_config2.sdpNameStr[0], dbSize);
        count = count + dbSize;   

#endif 
        
        if(opr->simDebug)   Debug(console, "sim> MPU Config-DB Upload ...snd=%d\n", dbSize); 
        break;
               
    case SIM_ESIOCFG_DOWN:      // 0x22 DB : ESIO Config Down
        
        esioId  = rxbuf[7] - 1;
        esioNum = rxbuf[8];
        dbSize  = sizeof(DB_ESIO_CONFIG);
		dbESIO  = (DB_ESIO_CONFIG *) &rtudb->esioConfig[esioId];
		
		/* 수신 ESIO 번호 Check...[1...5] */
		if((esioId < 0) || (esioId > MAX_DB_ESIO))
        {
            if(opr->simDebug)
    	    Debug(console, "sim> *rcv ESIO Config : Invalid ESIO ID ... rcv=%d \n", rxbuf[7]);
            return (0);
        }
        		    
        if(dataLen != (dbSize + 2))   
        {
            if(opr->simDebug)
    	    Debug(console, "sim> *rcv ESIO(%d) Config : Invalid Size ... rcv=%d, db=%d \n",  esioId+1, dataLen, dbSize);
    	    return (0);
        }

        /* ---------------------------- */        
    	/* 데이터베이스 저장 ...        */
    	/* ---------------------------- */ 
    	memcpy( (byte *) dbESIO, &rxbuf[9], dbSize);
        
        txbuf[count++] = esioId + 1;
            
        //if(opr->simDebug)   
        Debug(console, "sim> rcv ESIO(%d) Config-DB Down ... %d\n",  esioId+1, dbSize); 

// 내용은 정상인데...
//dispDB_ESIO_CONFIG( (DB_ESIO_CONFIG*) &rxbuf[9]);
//dispDB_ESIO_CONFIG( (DB_ESIO_CONFIG*) &rxbuf[9]);

        /* ESIO Config Down 종료후 초기화... */    
        if(esioNum & 0x80)
        {
            dbFileWrite(opr, rtudb);
            
            /* ------------------------------------ */
        	/*  Database 변경 이력 ...        */
	        /* ------------------------------------ */
    	    gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
        	logEvent_MPU(shmPtr, ENT_DB_UPDATE, 0, SIM_ESIOCFG_DOWN, 0, 0, &ctime);   
        
        	sprintf(buffer, "<-- SIM> Database Down [ SIM_ESIOCFG_DOWN ]");
        	LogFile_MPU (shmPtr, ENT_DB_UPDATE, buffer, strlen(buffer));     
        
            //if(opr->simDebug)   
            Debug(console, "sim> rcv ESIO(%d) Config-Inital.. \n",  esioId+1); 
            opr->esioCfgDown_OK = SET;      // ESIO Config Down .... Re Initial 
            
            /* CPU 이중화 DB 연계 */
            linkCfg->dbDownEsio  = 1;
            linkCfg->esioCfgDown = SET;
            
            /* --------------------------------- */
            /* 해당 ESIO SCAN Config-Down Flag   */
            /* --------------------------------- */            
            for(i=0; i < MAX_ESIO; i++)
            {
                esio = (ESIO_CONFIG *) esioCFG[i];
                if(esio->online != SET) continue;
           
                // 차이가 뭐냐 ???
                esio->esioCfgDown = SET;
                esio->dbDownEsio  = 1;
            }
        
        }
        
        break;
        
    case SIM_ESIOCFG_UP:        // 0x23 DB : ESIO Config UP
        esioId = rxbuf[7] - 1;
        esioNum = rxbuf[8];
        
        /* 수신 ESIO 번호 Check...[1...5] */
		if((esioId < 0) || (esioId > MAX_DB_ESIO))
        {
            if(opr->simDebug)
    	    Debug(console, "sim> *rcv ESIO Config : Invalid ESIO ID ... rcv=%d \n", rxbuf[7]);
            return (0);
        }
        
        dbSize = sizeof(DB_ESIO_CONFIG);
		dbESIO = (DB_ESIO_CONFIG *) &rtudb->esioConfig[esioId];
		
		txbuf[count++] = esioId + 1;        // 전송 ESIO 모듈번호
		txbuf[count++] = 1;                 // 전송갯수 (default= 1)
		      
        memcpy(&txbuf[count], (byte *) dbESIO, dbSize);
        count = count + dbSize;    
        
        //if(opr->simDebug)   
        Debug(console, "sim> ESIO(%d) Config-DB Upload ...snd=%d\n", esioId+1, dbSize); 
            
        break;
        
    case SIM_HOST_DOWN:         // 0x24 DB : HOST Config Down
   		dbSize = sizeof(DB_HOST_CONFIG) * MAX_DB_HOST;
		dbHOST = (DB_HOST_CONFIG *) &rtudb->hostCfg[0];
		
        if(dataLen != dbSize)   
        {
            if(opr->simDebug)
    	    Debug(console, "sim> *rcv HOST Config : Invalid Size ... rcv=%d, db=%d \n", dataLen, dbSize);
    	    return (0);
        }

        /* ---------------------------- */        
    	/* 데이터베이스 저장 ...        */
    	/* ---------------------------- */ 
    	memcpy( (byte *) dbHOST, &rxbuf[7], dataLen);
        dbFileWrite(opr, rtudb);
        
        
        /* ------------------------------------ */
       	/*  Database 변경 이력 ...        */
        /* ------------------------------------ */
   	    gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
       	logEvent_MPU(shmPtr, ENT_DB_UPDATE, 0, SIM_HOST_DOWN, 0, 0, &ctime);   
        
        sprintf(buffer, "<-- SIM> Database Down [ SIM_HOST_DOWN ]");
       	LogFile_MPU (shmPtr, ENT_DB_UPDATE, buffer, strlen(buffer));  
        	
        		
        opr->hostCfgDown_OK = SET;  
        
        /* CPU 이중화 DB 연계 */
        linkCfg->hostCfgDown  = SET;
        
        /* --------------------------------- */
        /* 해당 ESIO SCAN Config-Down Flag   */
        /* --------------------------------- */            
        for(i=0; i < MAX_ESIO; i++)
        {
            esio = (ESIO_CONFIG *) esioCFG[i];
            esio->hostCfgDown = SET;     // sio는 이걸 안 보나봐..
        }
           
        //if(opr->simDebug)   
        Debug(console, "sim> rcv HOST Config-DB Down ... %d\n", dataLen); 
            
        break;
        
    case SIM_HOST_UP:           // 0x25 DB : HOST Config UP
                
        dbSize = sizeof(DB_HOST_CONFIG) * MAX_DB_HOST;
        dbHOST = (DB_HOST_CONFIG *) &rtudb->hostCfg[0];
		
        memcpy(&txbuf[count], (byte *) dbHOST, dbSize);
        count = count + dbSize;    
        
        //if(opr->simDebug)   
        Debug(console, "sim> HOST Config-DB Upload ...snd=%d\n", dbSize); 
        break;
        
        
    case SIM_ICCP_DOWN:         // 0x26 DB : ICCP-HOST Config Down
        dbSize = sizeof(DB_ICCP_CONFIG);
		dbICCP = (DB_ICCP_CONFIG *) &rtudb->iccpConfig;
		
        if(dataLen != dbSize)   
        {
            if(opr->simDebug)
    	    Debug(console, "sim> *rcv HOST-ICCP Config : Invalid Size ... rcv=%d, db=%d \n", dataLen, dbSize);
    	    return (0);
        }

        /* ---------------------------- */        
    	/* 데이터베이스 저장 ...        */
    	/* ---------------------------- */ 
    	memcpy( (byte *) dbICCP, &rxbuf[7], dataLen);
        dbFileWrite(opr, rtudb);
        
        /* ------------------------------------ */
       	/*  Database 변경 이력 ...        */
        /* ------------------------------------ */
   	    gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
       	logEvent_MPU(shmPtr, ENT_DB_UPDATE, 0, SIM_ICCP_DOWN, 0, 0, &ctime); 
       	
       	sprintf(buffer, "<-- SIM> Database Down [ SIM_ICCP_DOWN ]");
        LogFile_MPU (shmPtr, ENT_DB_UPDATE, buffer, strlen(buffer));  
        
        
        opr->iccpCfgDown_OK = SET;  
        
        /* CPU 이중화 DB 연계 */
        linkCfg->iccpCfgDown  = SET;
        
        /* --------------------------------- */
        /* 해당 ESIO SCAN Config-Down Flag   */
        /* --------------------------------- */            
        for(i=0; i < MAX_ESIO; i++)
        {
            esio = (ESIO_CONFIG *) esioCFG[i];
            esio->iccpCfgDown = SET;     
        }
            
        //if(opr->simDebug)   
        Debug(console, "sim> rcv HOST-ICCP Config-DB Down ... %d\n", dataLen); 
            
        break;
        
    case SIM_ICCP_UP:           // 0x27 DB : ICCP-HOST Config UP
        dbSize = sizeof(DB_ICCP_CONFIG);
        dbICCP = (DB_ICCP_CONFIG *) &rtudb->iccpConfig;
		
        memcpy(&txbuf[count], (byte *) dbICCP, dbSize);
        count = count + dbSize;    
        
        if(opr->simDebug)   Debug(console, "sim> HOST Config-DB Upload ...snd=%d\n", dbSize); 
        break;
        
    case SIM_HARRIS_DOWN:       // 0x28 DB : HARRIS Config Down
        
        if(dataLen != 32)
        {
            if(opr->simDebug)
    	    Debug(console, "sim> *rcv HARRIS CONFIG : Invalid Size ... rcv=%d \n", dataLen);
    	    return (0);
        }
        
        memcpy(&rtudb->portdb[0], &rxbuf[7], dataLen);  /* 32x2x3 port */
        
        /* 데이터베이스 저장 ... */
        dbFileWrite(opr, rtudb);
        
        /* ------------------------------------ */
       	/*  Database 변경 이력 ...        */
        /* ------------------------------------ */
   	    gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
       	logEvent_MPU(shmPtr, ENT_DB_UPDATE, 0, SIM_HARRIS_DOWN, 0, 0, &ctime); 
       	
       	sprintf(buffer, "<-- SIM> Database Down [ SIM_HARRIS_DOWN ]");
        LogFile_MPU (shmPtr, ENT_DB_UPDATE, buffer, strlen(buffer));  
        
        /* CPU 이중화 DB 연계 */
        linkCfg->harrisCfgDown  = SET;
        
        /* --------------------------------- */
        /* 해당 ESIO SCAN Config-Down Flag   */
        /* --------------------------------- */            
        for(i=0; i < MAX_ESIO; i++)
        {
            esio = (ESIO_CONFIG *) esioCFG[i];
            esio->harrisCfgDown = SET;     
        }
        
        //if(opr->simDebug)   
        Debug(console, "sim> rcv HARRIS DB Down ... rcv=%d\n", dataLen);   
        break;
        
    case SIM_HARRIS_UP:         // 0x29 DB : HARRIS Config UP
        dbSize = 32;
        memcpy(&txbuf[count], &rtudb->portdb[0], dbSize);
        count = count + dbSize;     
        
        if(opr->simDebug)   Debug(console, "sim> HARRIS DB Upload ...snd=%d\n",dbSize );         
        break;
        
    case SIM_LANDIS_DOWN:       // 0x2A DB : LANDIS Config Down
        if(dataLen != 257)
        {
            if(opr->simDebug)
    	    Debug(console, "sim> *rcv LANDIS CONFIG : Invalid Size ... rcv=%d \n", dataLen);
    	    return (0);
        }
        
        memcpy(&rtudb->chassisNum, &rxbuf[7], dataLen);
        
        /* 데이터베이스 저장 ... */
        dbFileWrite(opr, rtudb);
        
        /* CPU 이중화 DB 연계 */
        linkCfg->landisCfgDown  = SET;
        
        /* --------------------------------- */
        /* 해당 ESIO SCAN Config-Down Flag   */
        /* --------------------------------- */            
        for(i=0; i < MAX_ESIO; i++)
        {
            esio = (ESIO_CONFIG *) esioCFG[i];
            esio->landisCfgDown = SET;     
        }
        
        //if(opr->simDebug)   
        Debug(console, "sim> rcv LANDIS DB Down ... rcv=%d\n", dataLen);
        break;
        
    case SIM_LANDIS_UP:         // 0x2B DB : LANDIS Config UP
   		dbSize = 256 + 1;
        memcpy(&txbuf[count], &rtudb->chassisNum, dbSize);
        count = count + dbSize; 
        
        //if(opr->simDebug)   
        Debug(console, "sim> LANDIS DB Upload ... snd=%d\n", dbSize);
        break;
        
    case SIM_MODBUS_DOWN:       // 0x2C DB : MODBUS Config Down
		dbSize = sizeof(DB_MODBUS_PROFILE) * MAX_DB_MODBUS_PROFILE;
		
		if(dataLen != dbSize)   
        {
            //if(opr->simDebug)
    	    Debug(console, "sim> *rcv MODBUS PROFILE : Invalid Size ... rcv=%d, db=%d \n", dataLen, dbSize);
    	    return (0);
        }

		dbModbus = (DB_MODBUS_PROFILE *) &rtudb->modbusProfile[0];
        	
        memcpy( (byte *) dbModbus, &rxbuf[7], dbSize);
        
        /* 데이터베이스 저장 ... */
        dbFileWrite(opr, rtudb);
        
        /* ------------------------------------ */
       	/*  Database 변경 이력 ...        */
        /* ------------------------------------ */
   	    gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
       	logEvent_MPU(shmPtr, ENT_DB_UPDATE, 0, SIM_MODBUS_DOWN, 0, 0, &ctime); 
       	
       	sprintf(buffer, "<-- SIM> Database Down [ SIM_MODBUS_DOWN ]");
        LogFile_MPU (shmPtr, ENT_DB_UPDATE, buffer, strlen(buffer));  
        
        /* CPU 이중화 DB 연계 */
        linkCfg->modbusCfgDown  = SET;
        opr->modbusCfgDown_OK   = SET;     
        
        /* --------------------------------- */
        /* 해당 ESIO SCAN Config-Down Flag   */
        /* --------------------------------- */            
        for(i=0; i < MAX_ESIO; i++)
        {
            esio = (ESIO_CONFIG *) esioCFG[i];
            esio->modbusCfgDown = SET;     
        }
        
        //if(opr->simDebug)   
        Debug(console, "sim> MODBUS Config-DB Down ... rcv=%d\n", dbSize);   
        break;
        
    case SIM_MODBUS_UP:         // 0x2D DB : MODBUS Config UP
        dbSize = sizeof(DB_MODBUS_PROFILE) * MAX_DB_MODBUS_PROFILE;

		dbModbus = (DB_MODBUS_PROFILE *) &rtudb->modbusProfile[0];
		
        memcpy(&txbuf[count], (byte *) dbModbus, dbSize);
        count = count + dbSize;   
        
        //if(opr->simDebug)   
        Debug(console, "sim> MODBUS Config-DB Upload ... snd=%d\n", dbSize);   
        break;
                  
    case SIM_SCAN_DOWN:         // 0x30 DB : SCAN Config Down
		dbSize = sizeof(DB_SCAN_CONFIG) * MAX_DB_SCAN_PORT;
		
		if(dataLen != dbSize)   
        {
            if(opr->simDebug)
    	    Debug(console, "sim> *rcv SCAN CONFIG : Invalid Size ... rcv=%d, db=%d \n", dataLen, dbSize);
    	    return (0);
        }

		dbScan = (DB_SCAN_CONFIG *) &rtudb->scanConfig[0];
        memcpy( (byte *) dbScan, &rxbuf[7], dbSize);
        
        dbFileWrite(opr, rtudb);                 // 데이터베이스 저장 ...
        
        /* ------------------------------------ */
       	/*  Database 변경 이력 ...        */
        /* ------------------------------------ */
   	    gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
       	logEvent_MPU(shmPtr, ENT_DB_UPDATE, 0, SIM_SCAN_DOWN, 0, 0, &ctime); 
       	
       	sprintf(buffer, "<-- SIM> Database Down [ SIM_SCAN_DOWN ]");
        LogFile_MPU (shmPtr, ENT_DB_UPDATE, buffer, strlen(buffer));  
        
        opr->scanCfgDown_OK = SET;          // SCAN 구조체 초기화
        
        /* CPU 이중화 DB 연계 */
        linkCfg->scanCfgDown  = SET;
        
        /* --------------------------------- */
        /* 해당 ESIO SCAN Config-Down Flag   */
        /* --------------------------------- */            
        for(i=0; i < MAX_ESIO; i++)
        {
            esio = (ESIO_CONFIG *) esioCFG[i];
            esio->scanCfgDown = SET;     
        }
        
        //if(opr->simDebug)   
        Debug(console, "sim> SCAN CONFIG-DB Down ... rcv=%d\n", dataLen);           
        break;
        
    case SIM_SCAN_UP:           // 0x31 DB : SCAN Config UP
        dbSize = sizeof(DB_SCAN_CONFIG) * MAX_DB_SCAN_PORT;

        dbScan = (DB_SCAN_CONFIG *) &rtudb->scanConfig[0];
        memcpy(&txbuf[count], (byte *) dbScan, dbSize);
        count = count + dbSize;   
        
        //if(opr->simDebug)   
        Debug(console, "sim> SCAN CONFIG-DB Upload ... snd=%d\n", dbSize);   
        break;
        
    case SIM_DEVICE_DOWN:       // 0x32 DB : DEVICE Config Down
        devid  = rxbuf[7] - 1;
        devNum = rxbuf[8] & 0x7f;;
        
        dbSize = sizeof(DB_SDP_DEVICE) * devNum;
        
        /* 수신 계전기 번호 Check...[0...63] */
		if((devid < 0) || ((devid + devNum) > MAX_DB_DEVICE))
        {
            //if(opr->simDebug)
    	    Debug(console, "sim> *rcv DEVICE Config : Invalid ID ... rcv= %d, num=%d \n", rxbuf[6], rxbuf[7]);
            return (0);
        }
        
        if(dataLen != (dbSize + 2))   
        {
            //if(opr->simDebug)
    	    Debug(console, "sim> *rcv DEVICE CONFIG : Invalid Size ... rcv=%d, db=%d \n", dataLen, dbSize);
    	    return (0);
        }

        dbDev = (DB_SDP_DEVICE *) &rtudb->deviceConfig[devid];
        memcpy( (byte *) dbDev, &rxbuf[9], dbSize);
        
        /* 포인트 전송종료 Check ... */
        if(rxbuf[8] & 0x80)
        {	
        	Debug(console, "sim> rcv DEVICE CONFIG-DB Down ... \n", dbSize);  
        	
            /* 데이터베이스 저장 ... */
            dbFileWrite(opr, rtudb);
            
            /* ------------------------------------ */
       		/*  Database 변경 이력 ...        */
	        /* ------------------------------------ */
   		    gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
       		logEvent_MPU(shmPtr, ENT_DB_UPDATE, 0, SIM_DEVICE_DOWN, 0, 0, &ctime); 
       	
       		sprintf(buffer, "<-- SIM> Database Down [ SIM_DEVICE_DOWN ]");
        	LogFile_MPU (shmPtr, ENT_DB_UPDATE, buffer, strlen(buffer));  
        
            opr->devCfgDown_OK = SET; 
            
            /* CPU 이중화 DB 연계 */
            linkCfg->dbDownDevice   = 1;
            linkCfg->deviceCfgDown  = SET;
			
            /* --------------------------------- */
            /* 해당 ESIO SCAN Config-Down Flag   */
            /* --------------------------------- */            
            for(i=0; i < MAX_ESIO; i++)
            {
                esio = (ESIO_CONFIG *) esioCFG[i];
                esio->dbDownDevice  = 1;       // 시작 계전기 번호 [1..64]
                esio->deviceCfgDown = SET;
            }
        }
        
        
        txbuf[count++]  = devid + 1;
        txbuf[count++]  = devNum;
        
        //if(opr->simDebug)   
        Debug(console, "sim> DEVICE(%d) CONFIG-DB Down ... rcv=%d\n", devid + 1, dbSize);           
            
        break;
        
    case SIM_DEVICE_UP:         // 0x33 DB : DEVICE Config UP
        devid  = rxbuf[7] - 1;
        devNum = rxbuf[8] & 0x7f;
        
        /* 수신 계전기 번호 Check...[0...63] */
		if((devid < 0) || ((devid + devNum) > MAX_DB_DEVICE))
        {
            if(opr->simDebug)
    	    Debug(console, "sim> *rcv DEVICE Config : Invalid ID ... rcv= %d, num=%d \n", rxbuf[6], rxbuf[7]);
            return (0);
        }
        
        dbSize = sizeof(DB_SDP_DEVICE) * devNum;
        dbDev = (DB_SDP_DEVICE *) &rtudb->deviceConfig[devid];
        
        txbuf[count++]  = devid + 1;
        txbuf[count++]  = devNum;
        
        memcpy(&txbuf[count], (byte *) dbDev, dbSize);
        count = count + dbSize;   
        
        //if(opr->simDebug)   
        Debug(console, "sim> DEVICE(%d) CONFIG-DB Upload ... snd=%d\n", devid+1, dbSize);   
        break;
        
    case SIM_POINT_DOWN:        // 0x34 DB : POINT Config Down
		startPt = (rxbuf[8] + rxbuf[7] * 256) - 1;
        ptNum   = rxbuf[9] & 0x7f;
        if(ptNum > 16)  ptNum = 16;

        if((startPt < 0) || ((startPt + ptNum) > MAX_DB_POINT))
        {
            if(opr->simDebug)
    	    Debug(console, "sim> *rcv POINT Config : Invalid Index ... start=%d, num=%d \n", startPt +1, ptNum);
            return (0);
        }
        
        dbSize = sizeof(DB_POINT_BUF) * ptNum;
        
        if(dataLen != (dbSize + 3))   
        {
            if(opr->simDebug)
    	    Debug(console, "sim> *rcv POINT CONFIG : Invalid Size ... rcv=%d, db=%d \n", dataLen, dbSize);
    	    return (0);
        }
        
    	    
 
    
        
        
        bfptr   = (byte *) &rtudb->pointBuf[startPt];
        memcpy( (byte *) bfptr, &rxbuf[10], dbSize);


    	{
    	word chksum, rcvsum ;
    	chksum = gensum (&rxbuf[10], dbSize);
    	rcvsum = gensum ((byte *) bfptr, dbSize);
    	
    	//if(opr->mpuDebug)   
        Debug(console,"sim> rcv Point Down : start=%4d, ptnum=%d, size = %d  check=%4x/%4x\n", startPt , ptNum, dbSize, chksum, rcvsum);
        }    



        
        /* 데이터베이스 저장 ... */
        //dbFileWrite(opr, rtudb);
        
        txbuf[count++]  = rxbuf[7]; 
        txbuf[count++]  = rxbuf[8]; 
        txbuf[count++]  = rxbuf[9];
         
        //if(opr->simDebug)   
        Debug(console, "sim> Point Down : start=%d, ptnum=%d\n", startPt + 1, ptNum);
        
        /* 포인트 전송종료 Check ... */
        if(rxbuf[9] & 0x80)
        {	
        	Debug(console, "sim> Point Down OK .... Point Initial...Start  [%d]\n", startPt + 1 );	
        	
        	/* 데이터베이스 저장 ... */
            dbFileWrite(opr, rtudb);
        	
        	/* ------------------------------------ */
       		/*  Database 변경 이력 ...        */
	        /* ------------------------------------ */
   		    gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
       		logEvent_MPU(shmPtr, ENT_DB_UPDATE, 0, SIM_POINT_DOWN, 0, 0, &ctime); 
       		
       		sprintf(buffer, "<-- SIM> Database Down [ SIM_POINT_DOWN ]");
        	LogFile_MPU (shmPtr, ENT_DB_UPDATE, buffer, strlen(buffer));  
        	
        	linkCfg->endPointFlag = 0x80;
        	
        	/* -------------------------------------------- */
            /*  이중화 구성시... 포인트 DB 전송...          */
            /* -------------------------------------------- */
            linkCfg->dbDownPoint  = startPt + 1;
            linkCfg->dbDownIndex  = ptNum;
            
            linkCfg->pointCfgDown = SET;
            
        	/* --------------------------------- */
            /* 해당 ESIO SCAN Config-Down Flag   */
            /* --------------------------------- */            
            for(i=0; i < MAX_ESIO; i++)
            {
                esio = (ESIO_CONFIG *) esioCFG[i];
                if(esio->online != SET) continue;
           
                esio->endPointFlag = 0x80;
                esio->dbDownPoint  = startPt + 1;
                //esio->dbDownIndex  = ptNum;
                
                esio->pointCfgDown  = SET;
            }
            
            opr->pointCfgDown_OK  = SET;        // POINT 구성정보 변경표시...
        }
        else
        {
            /* --------------------------------- */
            /* 해당 ESIO SCAN Config-Down Flag   */
            /* --------------------------------- */            
            for(i=0; i < MAX_ESIO; i++)
            {
                esio = (ESIO_CONFIG *) esioCFG[i];
                if(esio->online != SET) continue;
           
                esio->endPointFlag = RESET;
            }
        }
        
        break;
        
    case SIM_POINT_UP:          // 0x35 DB : POINT Config UP
        startPt = (rxbuf[8] + rxbuf[7] * 256) - 1;
        ptNum   = rxbuf[9] & 0x7f;
        if(ptNum > 16)  ptNum = 16;
            
       	if((startPt < 0) || ((startPt + ptNum) > MAX_DB_POINT))
        {
            if(opr->simDebug)
    	    Debug(console, "sim> *rcv POINT Up : Invalid Index ... start=%d, num=%d \n", startPt +1, ptNum);
            return (0);
        }
        
        bfptr   = (byte *) &rtudb->pointBuf[startPt];            
        
        dbSize = sizeof(DB_POINT_BUF) * ptNum;
        
        txbuf[count++] = rxbuf[7];  /* startPT MSB */
        txbuf[count++] = rxbuf[8];  /* startPT LSB */
        txbuf[count++] = rxbuf[9];  /* Point Num */        
        
        memcpy(&txbuf[count], bfptr, dbSize);
        count = count + dbSize;     
        
        //if(opr->simDebug)   
        Debug(console, "sim> Point Upload : start=%d, ptnum=%d... size=%d\n", startPt, ptNum, dbSize);
        break;

    case SIM_CAL_POINT_DOWN:        // 0x36 DB : CAL POINT Config Down
		startPt = (rxbuf[8] + rxbuf[7] * 256) - 1;
        ptNum   = rxbuf[9] & 0x7f;
        if(ptNum > 16)  ptNum = 16;

        if((startPt < 0) || ((startPt + ptNum) > MAX_DB_CAL_POINT))
        {
            //if(opr->simDebug)
    	    Debug(console, "sim> *rcv CAL-POINT Config : Invalid Index ... start=%d, num=%d \n", startPt +1, ptNum);
            return (0);
        }
        
        dbSize = sizeof(DB_CAL_POINT) * ptNum;
        
        if(dataLen != (dbSize + 3))   
        {
            //if(opr->simDebug)
    	    Debug(console, "sim> *rcv CAL-POINT CONFIG : Invalid Size ... rcv=%d, db=%d \n", dataLen, dbSize);
    	    return (0);
        }
        
        bfptr   = (byte *) &rtudb->calPointBuf[startPt];
        memcpy( (byte *) bfptr, &rxbuf[10], dbSize);
        
        /* 데이터베이스 저장 ... */
        //dbFileWrite(opr, rtudb);
        
        txbuf[count++]  = rxbuf[7]; 
        txbuf[count++]  = rxbuf[8]; 
        txbuf[count++]  = rxbuf[9];
         
        //if(opr->simDebug)   
        //Debug(console, "sim> CAL-Point Down : start=%d, ptnum=%d\n", startPt + 1, ptNum);
        
        /* 포인트 전송종료 Check ... */
        if(rxbuf[9] & 0x80)
        {	
        	Debug(console, "sim> CAL-Point Down OK .... Point Initial...Start\n");	
        	
        	/* 데이터베이스 저장 ... */
            dbFileWrite(opr, rtudb);
        
        	/* ------------------------------------ */
       		/*  Database 변경 이력 ...        */
	        /* ------------------------------------ */
   		    gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
       		logEvent_MPU(shmPtr, ENT_DB_UPDATE, 0, SIM_CAL_POINT_DOWN, 0, 0, &ctime); 
       		
       		sprintf(buffer, "<-- SIM> Database Down [ SIM_CAL_POINT_DOWN ]");
        	LogFile_MPU (shmPtr, ENT_DB_UPDATE, buffer, strlen(buffer));  
        	
        	opr->calPt_CfgDown_OK = SET;
        	
        	linkCfg->dbDownCalpt    = 1;    
        	linkCfg->calptCfgDown   = SET;               // CAL-POINT Config 전송요구
        	
        	/* --------------------------------- */
            /* 해당 ESIO SCAN Config-Down Flag   */
            /* --------------------------------- */            
            for(i=0; i < MAX_ESIO; i++)
            {
                esio = (ESIO_CONFIG *) esioCFG[i];
                esio->dbDownCalpt    = 1;    
        	    esio->calptCfgDown   = SET;               // CAL-POINT Config 전송요구
            }
        }
        break;

    case SIM_CAL_POINT_UP:          // 0x37 DB : CAL POINT Config UP
        startPt = (rxbuf[8] + rxbuf[7] * 256) - 1;
        ptNum   = rxbuf[9] & 0x7f;
        if(ptNum > 16)  ptNum = 16;
            
       	if((startPt < 0) || ((startPt + ptNum) > MAX_DB_CAL_POINT))
        {
            //if(opr->simDebug)
    	    Debug(console, "sim> *rcv CAL-POINT Up : Invalid Index ... start=%d, num=%d \n", startPt +1, ptNum);
            return (0);
        }
        
        bfptr   = (byte *) &rtudb->calPointBuf[startPt];            
        
        dbSize = sizeof(DB_CAL_POINT) * ptNum;
        
        txbuf[count++] = rxbuf[7];  /* startPT MSB */
        txbuf[count++] = rxbuf[8];  /* startPT LSB */
        txbuf[count++] = rxbuf[9];  /* Point Num */        
        
        memcpy(&txbuf[count], bfptr, dbSize);
        count = count + dbSize;     
        
        //if(opr->simDebug)   
        Debug(console, "sim> CAL-Point Upload : start=%d, ptnum=%d... size=%d\n", startPt, ptNum, dbSize);
        break;
                                		
	default :
	    if(opr->simDebug)
		Debug(console, "sim> *** Invalid Opcode = %2x\n", opcode);
		return (0);
	}

    /* -------------------------------- */
    /* 송신 Frame 구성                  */
    /* -------------------------------- */
    sndCount = count + 1;
                     
    txbuf[2] = (sndCount >> 8) & 0xff;     /* Total size: MSB */
    txbuf[3] = sndCount & 0xff;            /* Total size: LSB */
    txbuf[count] = genlrc(txbuf, count);
    count++;
    
    return(count);
                    
}

/* -------------------------------------------------------- */
/*  SIMULATOR - Packet 수신부                               */
/* -------------------------------------------------------- */
/*  return :  1   get vaild response                        */
/*  	      0   TCP Connection closed                     */
/*           -1  Timeout                                    */
/*           -2  select system error                        */
/* -------------------------------------------------------- */

#if  1
int  get_responseSIM(int SocketFd, byte	*rxbuf)
{
    int     result;
    word    errCount;
    int		rxcnt;
    int     reqSize;
    byte    lrc;

    rxcnt = 0;
    errCount = 0;
    
    /* ---------------------------------------- */
    /*  READ MODBUS - Header                    */
    /* ---------------------------------------- */ 
    do{
        // 100->1000
		if((result = TKreadn(SocketFd, (char *) &rxbuf[0], 1, 1000)) < 0)
    	{
        	return result;
        }
        
        if(++errCount > 50)	return (-1);
       	usleep(100);
        
    }while(rxbuf[0] != 0x7E);

    /* ------------------------------------ */
    /*  TCP/IP Socket Error 발생시 ...      */
    /* ------------------------------------ */
	if(result == 0)	return 0;

    /* read STX, SIZE */
	if((result =TKreadn(SocketFd, (char *) &rxbuf[1], 3, 100)) < 0)	return (-1);
	
	if(rxbuf[1] != 0x7E)	return (-1);		    
    
    reqSize = (rxbuf[2]*256 + rxbuf[3]) - 4;   /* except stx, size */ 

    if(reqSize > MAX_SIMULATOR_BUF)
    {
        if(opr->simDebug)
        Debug(console, "sim> * Invalid ... REQ-Size=%d\n", reqSize);   
        DumpBuff(console,"*RCV: ", rxbuf, 4);
        
        reqSize = 100;  
    }
          
    /* read BODY */
    if((result =TKreadn(SocketFd, (char *) &rxbuf[4], reqSize, 100)) != reqSize)	
    {
        if(opr->simDebug)
        Debug(console, "sim> * Tail not Received... req=%d, rcv=%d\n", reqSize, result);   
        DumpBuff(console,"*RCV: ", rxbuf, rxcnt);  
        return (-1);
    }

    /* receive body */
    rxcnt = reqSize + 4;
    lrc = genlrc(rxbuf, rxcnt);
        
#if 1
    if(lrc != 0)   
    {
        lrc = genlrc(rxbuf, rxcnt-1);
        if(opr->simDebug)    
        {
            Debug(console, "sim> *LRC Error ...cal=%2x, rcv=%2x!\n", lrc, rxbuf[rxcnt-1]);
            DumpBuff(console,"*LRC: ", rxbuf, rxcnt);
        }                
        return(-1);
    }
#endif
    
    return (rxcnt);
}
#else
// 2026-06-01 오후 4:09:36
int  tk_get_responseSIM(int SocketFd, byte	*rxbuf)
{
    int     result;
    word    errCount;
    int		rxcnt;
    int     reqSize;
    byte    lrc;

    rxcnt = 0;
    errCount = 0;
    
    /* ---------------------------------------- */
    /*  READ MODBUS - Header                    */
    /* ---------------------------------------- */ 
    do{
		if((result = tk_net_read(SocketFd, (char *) &rxbuf[0], 1, 1000)) < 0)
    	{
        	 //return result; // 아 여기서 빠진다.
            if (result != TK_TIMEOUT)
                return result ; 
        }
        
        if( result ==0 ) return 0; 
        
        
        if(++errCount > 50)	return (-1);
       	
       	// tk_net_read에서 wait 하기 때문에 필요 없다.
       	//usleep(100);
        
    }while(rxbuf[0] != 0x7E);

    /* ------------------------------------ */
    /*  TCP/IP Socket Error 발생시 ...      */
    /* ------------------------------------ */
	if(result == 0)	return 0; /* EOF 처리 */

    /* read STX, SIZE */
	if((result =tk_net_read(SocketFd, (char *) &rxbuf[1], 3, 1000)) < 0)	return (-1);
	
	if(rxbuf[1] != 0x7E)	return (-1);		    
    
    reqSize = (rxbuf[2]*256 + rxbuf[3]) - 4;   /* except stx, size */ 

    if(reqSize > MAX_SIMULATOR_BUF)
    {
        if(opr->simDebug)
        Debug(console, "sim> * Invalid ... REQ-Size=%d\n", reqSize);   
        DumpBuff(console,"*RCV: ", rxbuf, 4);
        
        reqSize = 100;  
    }
          
    /* read BODY */
    if((result =tk_net_read(SocketFd, (char *) &rxbuf[4], reqSize, 1000)) != reqSize)	
    {
        if(opr->simDebug)
        Debug(console, "sim> * Tail not Received... req=%d, rcv=%d\n", reqSize, result);   
        DumpBuff(console,"*RCV: ", rxbuf, rxcnt);  
        return (-1);
    }

    /* receive body */
    rxcnt = reqSize + 4;
    lrc = genlrc(rxbuf, rxcnt);
        
#if 1
    if(lrc != 0)   
    {
        lrc = genlrc(rxbuf, rxcnt-1);
        if(opr->simDebug)    
        {
            Debug(console, "sim> *LRC Error ...cal=%2x, rcv=%2x!\n", lrc, rxbuf[rxcnt-1]);
            DumpBuff(console,"*LRC: ", rxbuf, rxcnt);
        }                
        return(-1);
    }
#endif
    
    return (rxcnt);
}
#endif 

void dispDB_ESIO_CONFIG( DB_ESIO_CONFIG *db_esio)
{
    DB_ENET_ENTRY*  mNet1 ; 

    printf("usage %d\r\n",db_esio->useFlag);
    printf("targetID %d\r\n",db_esio->targetID);    
    printf("autoChgFlag %d\r\n",db_esio->autoChgFlag);    
    printf("comDelay %d\r\n",db_esio->comDelay);        
    printf("esioNameStr %s\r\n",db_esio->esioNameStr);  
    
    mNet1 = (DB_ENET_ENTRY *) &db_esio->mstNetConfig[0];
    printf("[1]use = %d, %s/%s/%s\n",   mNet1->useFlag, mNet1->ipAddr, mNet1->gwAddr, mNet1->subMask);    

    mNet1 = (DB_ENET_ENTRY *) &db_esio->mstNetConfig[1];
    printf("[2]use = %d, %s/%s/%s\n",   mNet1->useFlag, mNet1->ipAddr, mNet1->gwAddr, mNet1->subMask);    
    
    mNet1 = (DB_ENET_ENTRY *) &db_esio->mstNetConfig[2];
    printf("[3]use = %d, %s/%s/%s\n",   mNet1->useFlag, mNet1->ipAddr, mNet1->gwAddr, mNet1->subMask);    
    
    mNet1 = (DB_ENET_ENTRY *) &db_esio->mstNetConfig[3];
    printf("[4]use = %d, %s/%s/%s\n",   mNet1->useFlag, mNet1->ipAddr, mNet1->gwAddr, mNet1->subMask);            
    
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


    
    fprintf(stderr, "\n=== SIM SIGSEGV 발생! ===\n");
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
        
         printf("sim> ... signal [SIGTERM] generated (%2d)...!\n", sig);
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
            

         printf("sim> ... signal [SIGBUS] generated (%2d)...!\n", sig);            
            
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

         printf("sim> ... signal [SIGSEGV] generated (%2d)...!\n", sig);                
            
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "scan> ... signal [SIGSEGV] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));  		
        pause(1000);     	    
	    break;
		    
	case SIGPIPE: 
        if(opr->wdtDebug)   Debug(console,"scan> ... signal [SIGPIPE] generated (%2d)...!\n", sig);

         printf("sim> ... signal [SIGPIPE] generated (%2d)...!\n", sig); 

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



/* ------------------------------------------------------- */
/*  SIMULATOR MAIN 프로그램 Start Routine ....             */
/* ------------------------------------------------------- */

#if 0 // hslee
int	main(int argc, char **argv)
{
	//int     threadActive;
	int     rxVal;
	int     comFailTick;
	int     oldsec;
	int     simRcvPacket;
	
	byte	*rxbuf, *txbuf;
	int		rxcnt, txcnt;
	char	buffer[128];
	
	// 작업 환경을 초기화한다
	termExec = InitEnv();
	taskPtr->wdtCount = 0;
		
#if 1		
    install_sighander();
#else 		
		
	/* Signal Handler initial */
	signal( SIGBUS,  SigHandler);
	signal( SIGSEGV, SigHandler);
	signal( SIGTERM, SigHandler);	
	signal( SIGPIPE, SigHandler);
#endif 

#if 0
    rxbuf = (byte *) &simRxbuf[0];
	txbuf = (byte *) &simTxbuf[0];
#else
    rxbuf = (byte *) calloc( 1024 , 15) ;
    if ( rxbuf == NULL)
    {
                fprintf(stderr, "SIM> calloc failed (%zu * %zu bytes): %s\n",
                1024, 15, strerror(errno));
                exit(1) ;
    } 
        
	txbuf = (byte *) calloc( 1024 , 15) ;

    if ( txbuf == NULL)
    {
                fprintf(stderr, "SIM> calloc failed (%zu * %zu bytes): %s\n",
                1024, 15, strerror(errno));
                exit(1) ;
    } 

#endif 	
	
    opr->simDebug   = 0;
			
	pause(1000);
	
	/* ------------------------------------ */    
    /* CPU 링크상태 확인 후 HOST-TASK  기동 */
    /* ------------------------------------ */  
    
    // 이것이 master만 sim에 연결하기 때문인가 ???
    
    while(opr->chkLinkOK == RESET)
    {
    	taskPtr->wdtCount = 0;
		pause(100);
	    if(opr->simDebug)
		  Debug(console,"SIM> *check LINK Status  ...! \n" );
    }
    
    pause(1000);

	/* LOG File 저장 */
	sprintf(buffer, " %s SIMULATOR PROCESS Activated ... !", TARGET_NAME);
    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    
    printf(" %s SIMULATOR-SERVER PROCESS Activated [socket=%d, port=%d]... !\n", TARGET_NAME, socketServerFd, SIMULATOR_NET_PORT);
    
    simRcvPacket = RESET;
    oldsec = rtc->sec;
    
	while(termExec)
	{
	    taskPtr->wdtCount = 0;
		pause(100);

        /* ------------------------------------ */
        /* SIMULATOR Network 초기화... 			*/
        /* ------------------------------------ */
        while(_initNetServerSIM() == ERROR) 
		{
		    close(socketServerFd);
    	    simTask_Active = RESET;
    	    pause(1000);
            
    	    continue;
		}
		
		if(opr->simDebug)
		Debug(console," %s SIM-TCPIP(SERVER) Activated ....Socket = %d \n", TARGET_NAME, simSocketID);
		
		simTask_Active = SET;
		comFailTick  = 0;
    	
    	while(simTask_Active)
    	{
    	    taskPtr->wdtCount = 0;
    	    pause(100);
    	    
            /* ------------------------------------- */
            /*  메세지 수신 & Response               */
    	    /* ------------------------------------- */	        	        
   	    	if((rxcnt = get_responseSIM(simSocketID, rxbuf)) > 0)
    	    {
                comFailTick = 0;
	        
                /* ----------------------------------------- */
                /*  수신한 데이터에 대한 패켓 처리 ... */    
                /* ----------------------------------------- */
                if(opr->simDebug)
                {	    
                	if(rxcnt > 200)	DumpBuff(console,"SIM-RX: ", rxbuf, 200);
                	else			DumpBuff(console,"SIM-RX: ", rxbuf, rxcnt);
                }	
            	
            	simRcvPacket = SET;        
    	    }
	        else
	        {
    	        if(rxcnt == 0)
    	        {
	            	if(opr->simDebug)
	                Debug(console, "sim> *TCP/IP read ...0, Simulatior Closed!\n");
	                simTask_Active = RESET;
	                close(socketServerFd); // 여기서는 accepted 만 닫는데... listen 은 어디서 닫나 ?
	                //2026-06-01 오후 5:24:13
	                continue ;
	            }
#if 0	            
	            else 
	            {
	            	//if(opr->simDebug)
	                Debug(console, "sim> read Timeout or Error!\n");
	                simTask_Active = RESET;
	                close(socketServerFd);	            
	            }
	            // 2026-06-01 오후 4:46:11
	            continue ;
#endif 	     
                else if ( timeout )
                {    
            		/* ------------------------------------- */
        	        /*  CLIENT - Timeout Check...			 */
        	        /* ------------------------------------- */	     
            		if(++comFailTick > 10)
        	    	{
        		    	comFailTick = 0;
        			
            			if(opr->simDebug)	
        	    		Debug(console, "sim> *CLIENT.. No Response...reConnect!\n");
        	            simTask_Active = RESET;
        	            
        	            close(socketServerFd);
        		    }
                }
       
	        }

            /* ---------------------------------------- */
            /*  수신한 데이터에 대한 패켓 처리 ...      */    
            /* ---------------------------------------- */
            if(simRcvPacket == SET)
            {
                simRcvPacket = RESET;
                txcnt = mkResponseSIM(txbuf, rxbuf, rxcnt);
        
                /* -------------------------------------------- */
                /* 전송메세지에 대한 출력                       */  
                /* -------------------------------------------- */     
                if(txcnt >= 5) 
                {
                    /* -------------------------------- */
                    /* Network Response...              */
                    /* -------------------------------- */    
                    if((rxVal = tkWriteTCP( simSocketID ,(byte *) txbuf, txcnt, 1000)) < 0 )
                	{
                        if(opr->simDebug)  Debug(console, "sim> * *** net Send Error [%d %d]...socket = %d !\n", txcnt,  rxVal, simSocketID);		
	                }
	                
                    if(opr->simDebug) 
                    {
                    	if(txcnt > 200)	DumpBuff(console,"SIM-TX: ", txbuf, 200);
                    	else			DumpBuff(console,"SIM-TX: ", txbuf, txcnt);	
                   	}

                    if( rxVal != txcnt)
                    {
                        if(opr->simDebug)  
                        Debug(console, "sim> * net Send Error [%d %d]...!\n", txcnt,  rxVal);
                        simTask_Active = RESET;
                        close(socketServerFd);
                    }                
                }
            }
        	 
    	    if(oldsec == rtc->sec)	continue;
            oldsec = rtc->sec;	        
            
            /* -------------------------------- */
            /*  SIMULATOR 상태정보 추출         */
            /* -------------------------------- */    
            checkSIM_sts();   
    
	
		    	    
    	}
    	
                            		
	}/* while */

	// 작업 환경을 정리한다
	ClearEnv();
	
	return (0);
}

#else // socket + accpet 분리
 
/* ------------------------------------------------------- */
/*  SIMULATOR MAIN 프로그램 Start Routine ....             */
/* ------------------------------------------------------- */
int	main(int argc, char **argv)
{
	//int     threadActive;
	int     rxVal;
	int     comFailTick;
	int     oldsec;
	int     simRcvPacket;
	
	byte	*rxbuf, *txbuf;
	int		rxcnt, txcnt;
	char	buffer[128];
	
	int socket_fd ;
	int accepted_fd ;
	
	struct sockaddr_in cli_addr_out ;
	int accept_debug;
	
	// 작업 환경을 초기화한다
	termExec = InitEnv();
	taskPtr->wdtCount = 0;
		
	
    install_sighander();

    // 2026-06-02 오후 5:33:16
    rxbuf = (byte *) calloc( 1024 , 15) ;
    if ( rxbuf == NULL)
    {
                fprintf(stderr, "SIM> calloc failed (%zu * %zu bytes): %s\n",
                1024, 15, strerror(errno));
                exit(1) ;
    } 
        
	txbuf = (byte *) calloc( 1024 , 15) ;

    if ( txbuf == NULL)
    {
                fprintf(stderr, "SIM> calloc failed (%zu * %zu bytes): %s\n",
                1024, 15, strerror(errno));
                exit(1) ;
    } 


	
    opr->simDebug   = 0;
			
	pause(1000);
	
	/* ------------------------------------ */    
    /* CPU 링크상태 확인 후 HOST-TASK  기동 */
    /* ------------------------------------ */  
    
    // 이것이 master만 sim에 연결하기 때문인가 ???
    
    while(opr->chkLinkOK == RESET)
    {
    	taskPtr->wdtCount = 0;
		pause(100);
	    if(opr->simDebug)
		  Debug(console,"SIM> *check LINK Status  ...! \n" );
    }
    
    pause(1000);

	/* LOG File 저장 */
	sprintf(buffer, " %s SIMULATOR PROCESS Activated ... !", TARGET_NAME);
    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    
    printf(" %s SIMULATOR-SERVER PROCESS Activated [socket=%d, port=%d]... !\n", TARGET_NAME, socketServerFd, SIMULATOR_NET_PORT);
    
    simRcvPacket = RESET;
    oldsec = rtc->sec;
    
    socket_fd=-1 ; 
    /* open socket */
    socket_fd = tk_initNetServerSIM() ;
    
    if ( socket_fd < 0)
    {
            printf("SIM> open socket error!  [%s]\n", sterrror(errno) );    
            exit -1 ; 
    }
    accepted_fd = -1 ;
    
	while(termExec)
	{
	    taskPtr->wdtCount = 0;
		pause(100);

        /* ------------------------------------ */
        /* accpet			*/
        /* ------------------------------------ */
        
          if(opr->simDebug)  accept_debug =1 ; 
          else accept_debug =0 ;
                      
        if (accepted_fd < 0 )
        {
            accepted_fd = tk_accept_with_timeout( socket_fd, 1000,   // accpet 대기 시간
                                                  *cli_addr_out, // 상대편 주소 담는 곳 
                                                  accept_debug    // 개발시 debug enable 
                                                   ) ;
                                                   
            if ( accepted_fd > 0)
            {

        		if(opr->simDebug)
        		    Debug(console," %s SIM-TCPIP(SERVER) Activated ....Socket = %d  listefd\n", TARGET_NAME, accepted_fd,socket_fd );
        		
		         simTask_Active = SET;
		         comFailTick  = 0;
    	
    	         while(simTask_Active)
    	        {
    	                taskPtr->wdtCount = 0;
    	                pause(100);
    	    
                        /* ------------------------------------- */
                        /*  메세지 수신 & Response               */
                	    /* ------------------------------------- */	        	        
   	    	            if((rxcnt = get_responseSIM(accepted_fd, rxbuf)) > 0)
    	                {
                            comFailTick = 0;
	        
                            /* ----------------------------------------- */
                            /*  수신한 데이터에 대한 패켓 처리 ... */    
                            /* ----------------------------------------- */
                            if(opr->simDebug)
                            {	    
                            	if(rxcnt > 200)	DumpBuff(console,"SIM-RX: ", rxbuf, 200);
                            	else			DumpBuff(console,"SIM-RX: ", rxbuf, rxcnt);
                            }	
            	
            	            simRcvPacket = SET;     /* 수신 data 있어요...*/   
    	               }
	                   else
	                   {
                	        if(rxcnt == 0)
                	        {
            	            	if(opr->simDebug)
            	                Debug(console, "sim> *TCP/IP read ...0, Simulatior Closed!\n");
            	                simTask_Active = RESET;
            	                close(accepted_fd); // 여기서는 accepted 만 닫는데... listen 은 어디서 닫나 ?
            	                accepted_fd = -1;
            	                //2026-06-01 오후 5:24:13
            	                continue ;
            	            }
                            
                            // systemerr
            	            else 
            	            {
            	            	//if(opr->simDebug)
            	                Debug(console, "sim> read Timeout or Error!\n");
            	                simTask_Active = RESET;
            	                close(socketServerFd);	            
            	            }
            	            // 2026-06-01 오후 4:46:11
            	            continue ;
 	            
            	        }
            
                        /* ---------------------------------------- */
                        /*  수신한 데이터에 대한 패켓 처리 ...      */    
                        /* ---------------------------------------- */
                        if(simRcvPacket == SET)
                        {
                            simRcvPacket = RESET;
                            txcnt = mkResponseSIM(txbuf, rxbuf, rxcnt);
                    
                            /* -------------------------------------------- */
                            /* 전송메세지에 대한 출력                       */  
                            /* -------------------------------------------- */     
                            if(txcnt >= 5) 
                            {
                                /* -------------------------------- */
                                /* Network Response...              */
                                /* -------------------------------- */    
                                if((rxVal = tkWriteTCP( accepted_fd ,(byte *) txbuf, txcnt, 1000)) < 0 )
                            	{
                                    if(opr->simDebug)  Debug(console, "sim> * *** net Send Error [%d %d]...socket = %d !\n", txcnt,  rxVal, simSocketID);		
            	                }
            	                
                                if(opr->simDebug) 
                                {
                                	if(txcnt > 200)	DumpBuff(console,"SIM-TX: ", txbuf, 200);
                                	else			DumpBuff(console,"SIM-TX: ", txbuf, txcnt);	
                               	}

                                if( rxVal != txcnt)
                                {
                                    if(opr->simDebug)  
                                    Debug(console, "sim> * net Send Error [%d %d]...!\n", txcnt,  rxVal);
                                    simTask_Active = RESET;
            	                    close(accepted_fd); // 여기서는 accepted 만 닫는데... listen 은 어디서 닫나 ?
            	                    accepted_fd = -1;
                                }                
                            }
                        }
        	 
                	    if(oldsec == rtc->sec)	continue;
                        oldsec = rtc->sec;	        
            
                        /* -------------------------------- */
                        /*  SIMULATOR 상태정보 추출         */
                        /* -------------------------------- */    
                        checkSIM_sts();   
    
                		/* ------------------------------------- */
            	        /*  CLIENT - Timeout Check...			 */
            	        /* ------------------------------------- */	     
                		if(++comFailTick > 10)
            	    	{
            		    	comFailTick = 0;
            			
                			if(opr->simDebug)	
            	    		Debug(console, "sim> *CLIENT.. No Response...reConnect!\n");
            	            simTask_Active = RESET;
            	            
            	            close(socketServerFd);
            		    }	
		    	    
    	        } //     	         while(simTask_Active)



            
            } //accpeted> 0
            // 실패하는 경우 timeout 과 system Error
            else 
            {
                if ( accepted_fd ==TK_SYS_ERR ) 
                {
                    Debug(console,"SIM>  listefd error while accepting [%s]\n",socket_fd, strerror(errno) ) ;
                    termExec = 0;
                    close (socket_fd);
                    continue ; 
                }
            }
                
        }    
        else 
        { 
               close(accepted_fd);
               accepted_fd=-1; 
         
        }    
		            if(opr->simDebug)
		            Debug(console," %s SIM-TCPIP(SERVER) Activated ....Socket = %d \n", TARGET_NAME, simSocketID);
		
		            simTask_Active = SET;
		            comFailTick  = 0;
    	
    	            while(simTask_Active)
    	            {
    	                taskPtr->wdtCount = 0;
    	                pause(100);
    	    
                        /* ------------------------------------- */
                        /*  메세지 수신 & Response               */
                	    /* ------------------------------------- */	        	        
            	    	if((rxcnt = get_responseSIM(simSocketID, rxbuf)) > 0)
    	                {
                            comFailTick = 0;
            	        
                            /* ----------------------------------------- */
                            /*  수신한 데이터에 대한 패켓 처리 ... */    
                            /* ----------------------------------------- */
                            if(opr->simDebug)
                            {	    
                            	if(rxcnt > 200)	DumpBuff(console,"SIM-RX: ", rxbuf, 200);
                            	else			DumpBuff(console,"SIM-RX: ", rxbuf, rxcnt);
                            }	
            	
            	            simRcvPacket = SET;        
    	                 }
	                    else
	                    {
                	        if(rxcnt == 0)
                	        {
            	            	if(opr->simDebug)
            	                Debug(console, "sim> *TCP/IP read ...0, Simulatior Closed!\n");
            	                simTask_Active = RESET;
            	                close(socketServerFd); // 여기서는 accepted 만 닫는데... listen 은 어디서 닫나 ?
            	                //2026-06-01 오후 5:24:13
            	                continue ;
            	            }
                        #if 0	            
                        	            else 
                        	            {
                        	            	//if(opr->simDebug)
                        	                Debug(console, "sim> read Timeout or Error!\n");
                        	                simTask_Active = RESET;
                        	                close(socketServerFd);	            
                        	            }
                        	            // 2026-06-01 오후 4:46:11
                        	            continue ;
                        #endif 	            
        	        }

            /* ---------------------------------------- */
            /*  수신한 데이터에 대한 패켓 처리 ...      */    
            /* ---------------------------------------- */
            if(simRcvPacket == SET)
            {
                simRcvPacket = RESET;
                txcnt = mkResponseSIM(txbuf, rxbuf, rxcnt);
        
                /* -------------------------------------------- */
                /* 전송메세지에 대한 출력                       */  
                /* -------------------------------------------- */     
                if(txcnt >= 5) 
                {
                    /* -------------------------------- */
                    /* Network Response...              */
                    /* -------------------------------- */    
                    if((rxVal = tkWriteTCP( simSocketID ,(byte *) txbuf, txcnt, 1000)) < 0 )
                	{
                        if(opr->simDebug)  Debug(console, "sim> * *** net Send Error [%d %d]...socket = %d !\n", txcnt,  rxVal, simSocketID);		
	                }
	                
                    if(opr->simDebug) 
                    {
                    	if(txcnt > 200)	DumpBuff(console,"SIM-TX: ", txbuf, 200);
                    	else			DumpBuff(console,"SIM-TX: ", txbuf, txcnt);	
                   	}

                    if( rxVal != txcnt)
                    {
                        if(opr->simDebug)  
                        Debug(console, "sim> * net Send Error [%d %d]...!\n", txcnt,  rxVal);
                        simTask_Active = RESET;
                        close(socketServerFd);
                    }                
                }
            }
        	 
    	    if(oldsec == rtc->sec)	continue;
            oldsec = rtc->sec;	        
            
            /* -------------------------------- */
            /*  SIMULATOR 상태정보 추출         */
            /* -------------------------------- */    
            checkSIM_sts();   
    
    		/* ------------------------------------- */
	        /*  CLIENT - Timeout Check...			 */
	        /* ------------------------------------- */	     
    		if(++comFailTick > 10)
	    	{
		    	comFailTick = 0;
			
    			if(opr->simDebug)	
	    		Debug(console, "sim> *CLIENT.. No Response...reConnect!\n");
	            simTask_Active = RESET;
	            
	            close(accepted_fd);
	            accepted_fd =-1;
                simTask_Active = RESET;    	
		    			    	        	                   
            }    
    	    
    	        
    	    continue;                
                                                                   
        }                 


                            		
	}/* while(termExec) */

	// 작업 환경을 정리한다
	ClearEnv();
	
	return (0);

        
    }



#endif 
