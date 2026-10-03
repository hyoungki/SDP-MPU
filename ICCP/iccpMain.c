
#ifndef _WIN32
#include	"localLib.h"

#include "glbtypes.h"
#include "sysincs.h"
#include "mi_usr.h"
#include "iccpShm.h"

// 2026-04-02 오전 9:20:34
#include <netinet/tcp.h>  // TCP_KEEP... 상수 정의


SHM_DESC	    shmDesc = {-1, SHM_KEY, sizeof(SHM_MEMORY), NULL};
SHM_MEMORY	    *shmPtr = NULL;

int             termExec;

TASK_INFO	    *taskPtr= NULL;

RTC             *rtc    = NULL;
OPR_MSG         *opr    = NULL;
CONSOLE_INFO	*console= NULL;
LINK_MSG        *linkCfg= NULL;                 // CPU 이중화 구조체

MPU_CONFIG      *mpuCFG = NULL;                 // MPU Config
SCU_MSG         *scuCfg = NULL;                 // 이중화 절체장치(SCU) 구조체

ICCP_DCB        *iccpDCB = NULL;                // ICCP-HOST 구조체
ICCP_CONFIG     *iccpCFG = NULL;                // ICCP-HOST 구조체

ICCP_60870_DCB  *iccpInfo=NULL;                 // ICCP-HOST 참조용 : 모니터링 구조체

#if 0	// CHOIBC DELETE
ESIO_CONFIG     *esioCFG[MAX_ESIO];             // ESIO 장치 Config

HOST_DCB        *hostDCB[MAX_HOST];             // HOST 관련 구조체 : ICCP, DNP, HARRIS, LANDIS...
POINT_BUF       *pointCFG[MAX_DEV_POINT];       // SDP 포인트 Config 정보
SCAN_CONFIG     *scanCFG[MAX_SCAN_PORT];        // 하위계전기 SCAN Config
SDP_DEVICE      *deviceCFG[MAX_DEVICE];         // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)
#endif
	 
RTU_DATABASE    *rtudb  = NULL;                 // SDP 데이터베이스
HISTORY_QUE     *hque   = NULL;                 // CONSOLE 용 이벤트


#include <stdio.h>
#include "glbtypes.h"

typedef struct {
    ST_INT8  v1;
    ST_INT32 v2;
    ST_INT16 v3;
    ST_FLOAT v4;
} ALIGN_CHECK;

void check_alignment() {
    ALIGN_CHECK test;
    printf("--- Alignment Check on i.MX6SX ---\n");
    printf("Size of ALIGN_CHECK: %d\n", (int)sizeof(ALIGN_CHECK));
    printf("Offset of v1 (INT8) : %ld\n", (long)((char*)&test.v1 - (char*)&test));
    printf("Offset of v2 (INT32): %ld\n", (long)((char*)&test.v2 - (char*)&test));
    printf("Offset of v3 (INT16): %ld\n", (long)((char*)&test.v3 - (char*)&test));
    printf("Offset of v4 (FLOAT): %ld\n", (long)((char*)&test.v4 - (char*)&test));
}


/* *************************************************************************************
*	FUNCTION : sigHandler()
* **************************************************************************************/
void	SigHandler( int sig )
{
    //if(opr->wdtDebug)
	Debug(console,"iccp>> *****  ... signal generated (%2d)...!\n", sig);
    pause(100);
    
	switch(sig)
	{
	    case SIGTERM:
	        //if(opr->wdtDebug)   
	        Debug(console,"iccp> *SIG TERM Error ... !\n", sig); 
            pause(1000);
	        termExec = 0;
	        break;
	        
		case SIGBUS : 
		    //if(opr->wdtDebug)   
		    Debug(console,"iccp> *SIG BUS Error ... !\n", sig); 
            pause(1000);		        
            termExec = 0;
		    break;
		    
		case SIGSEGV: 
		    //if(opr->wdtDebug)   
		    Debug(console,"iccp> *SIG SEGV Error ... !\n", sig); 
            pause(1000);
            termExec = 0;		        
		    break;
		    
		case SIGPIPE: 
		    //if(opr->wdtDebug)   
		    Debug(console,"iccp> *SIG PIPE Error ... !\n", sig); 
            pause(1000);		        
			break;
			
		default : 
			//if(opr->wdtDebug)   
			Debug(console,"iccp> (%d) ... signal generated ...!\n", sig);
			break;
	}

}


//
// 모듈:	ClearEnvironment()
//
static void ClearEnv(void)
{
    /* -------------------------------- */
	/*  프로세스 정보를 정리한다        */
	/* -------------------------------- */
	UpdateProcessInfo(taskPtr, 0, getpid(), 0);

	// ICCP Shared memory lock file close
	iccpShmEnd ();
	// ICCP 타스크가 종료할때 정보를 초기화한다.
	iccpShmSetAssociationEnd ();

    /* -------------------------------- */
	/*  공유 메모리 설정 해제           */
	/* -------------------------------- */
	if (shmPtr != NULL)     ShmDetach(&shmDesc);
}
					    

//
// 모듈:	InitEnvironment()
//
static int InitEnv( void)
{
#ifdef	_PLATFORM_LINUX	// 2016.09 ChoiBC Test
    /* ------------------------------------ */
	/*  공유 메모리를 생성한다              */
	/* ------------------------------------ */
	if (ShmCreate(&shmDesc) < 0)
	{
		printf("iccp> *공유 메모리 Open 이상 \n");
		return(0);
	}
#endif
	// 공유 메모리 상태 확인
	if (ShmCheck(&shmDesc) < 0)
	{
		printf("iccp> *ERR_공유 메모리 상태 이상\n");
		return(0);
	}
	
	//printf("sim> SHM %p \n", shmDesc.address);
	
	// 포인터 변수 초기화
	shmPtr = (SHM_MEMORY *) shmDesc.address;
	taskPtr = (TASK_INFO *) &shmPtr->taskInfo[ICCP_PROCESS];

	rtc     = (RTC *)       &shmPtr->rtc;
	opr     = (OPR_MSG *)   &shmPtr->opr_msg;
    
    linkCfg = (LINK_MSG *)  &shmPtr->link_msg;
    
	rtudb   = (RTU_DATABASE *) &shmPtr->rtuDatabase;
    console = (CONSOLE_INFO *) &shmPtr->console; 
	hque    = (HISTORY_QUE *)  &shmPtr->localHistoryQ; 
	
    mpuCFG  = (MPU_CONFIG *) &shmPtr->mpuConfig;
    scuCfg 	= (SCU_MSG *)   &shmPtr->scu_msg;                   // 이중화절체장치(SCU) 구조체
    
    /* ICCP-INFO 구조체 정의 */
    iccpDCB = (ICCP_DCB *) &shmPtr->iccpDCB;                   // ICCP_DCB 
    iccpInfo= (ICCP_60870_DCB *) &shmPtr->iccpInfo;            // ICCP-HOST 참조용 : 모니터링 구조체
    
    /* ------------------------------------ */
    /* ESIO CFG : ESIO 구조체 (MMAX 5)      */
    /* HOST_DCB : 상위 HOST 구조체 (MMAX 8) */
    /* SCAN CONFIG : SCAN 구조체(MAX 16)    */
    /* 계전기 구조체 : 전체 계전기(MAX 64)  */
    /* 포인트 구조체 : 전체 포인트(4096)    */
    /* ------------------------------------ */ 
#if 0	// CHOIBC DELETE
    for(i=0; i< MAX_ESIO; i++)          esioCFG[i] = (ESIO_CONFIG *) &shmPtr->esioConfig[i];
    for(i=0; i< MAX_HOST; i++)          hostDCB[i] = (HOST_DCB *) &shmPtr->hostDCB[i];
    for(i=0; i< MAX_SCAN_PORT; i++)     scanCFG[i] = (SCAN_CONFIG *) &shmPtr->scanCFG[i];
    for(i=0; i< MAX_DEVICE; i++)        deviceCFG[i] = (SDP_DEVICE *) &shmPtr->deviceCFG[i];
	for(i=0; i< MAX_DEV_POINT; i++)     pointCFG[i] = (POINT_BUF *) &shmPtr->pointCFG[i];
	// ICCP_60870_DCB를 이용하기 때문에 pointCFG가 필요없다
#endif

	// ICCP Shared memory lock file open
	iccpShmInit (&shmPtr->iccpDCB);
	// ICCP 타스크가 시작할때 정보를 초기화한다.
	iccpShmSetAssociationInit ();
#if 0	// CHOIBC DELETE
	// ICCP COS Queue에 모든 Entry의 sending flag를 Reset한다.
	iccpShmResetSendingCosAllEntry ();
#endif	// CHOIBC DELETE

#if 0	// 2016.09 ChoiBC Test
	{
		ICCP_UNIT	*pAddr;
		// FEP_A
		pAddr = &shmPtr->iccpDCB.config.FEP_A;
		strcpy ((char *)pAddr->IPAddress, "127.0.0.1");
		strcpy ((char *)pAddr->P_Sel, "00 00 00 01");
		strcpy ((char *)pAddr->S_Sel, "00 01");
		strcpy ((char *)pAddr->T_Sel, "00 01");
		strcpy ((char *)pAddr->AP_Title, "1 3 5555 1");
		strcpy ((char *)pAddr->AE_Qualifier, "1");
		// FEP_B
		pAddr = &shmPtr->iccpDCB.config.FEP_B;
		strcpy ((char *)pAddr->IPAddress, "127.0.0.1");
		strcpy ((char *)pAddr->P_Sel, "00 00 00 02");
		strcpy ((char *)pAddr->S_Sel, "00 02");
		strcpy ((char *)pAddr->T_Sel, "00 02");
		strcpy ((char *)pAddr->AP_Title, "1 3 5555 2");
		strcpy ((char *)pAddr->AE_Qualifier, "2");
		// SDP_A
		pAddr = &shmPtr->iccpDCB.config.SDP_A;
		strcpy ((char *)pAddr->IPAddress, "127.0.0.1");
		strcpy ((char *)pAddr->P_Sel, "00 00 00 03");
		strcpy ((char *)pAddr->S_Sel, "00 03");
		strcpy ((char *)pAddr->T_Sel, "00 03");
		strcpy ((char *)pAddr->AP_Title, "1 3 5555 3");
		strcpy ((char *)pAddr->AE_Qualifier, "3");
		// SDP_B
		pAddr = &shmPtr->iccpDCB.config.SDP_B;
		strcpy ((char *)pAddr->IPAddress, "127.0.0.1");
		strcpy ((char *)pAddr->P_Sel, "00 00 00 04");
		strcpy ((char *)pAddr->S_Sel, "00 04");
		strcpy ((char *)pAddr->T_Sel, "00 04");
		strcpy ((char *)pAddr->AP_Title, "1 3 5555 4");
		strcpy ((char *)pAddr->AE_Qualifier, "4");
	}
	{
		ICCP_POINT_DEF *pPointDef = &shmPtr->iccpDCB.pointDef;
		ICCP_POINT_INFO *pI = &pPointDef->point[0];

		memset (pPointDef, 0, sizeof(*pPointDef));

		pI->devNo          = 1;
		pI->devPt          = 1;
		pI->config         = 1;
		pI->iccpPointType  = SDP_POINT_TYPE_SDI;
		pI->iccpPointIndex = 1;
		pPointDef->numPoint++;

		pI++;
		pI->devNo          = 1;
		pI->devPt          = 2;
		pI->config         = 1;
		pI->soe            = 1;
		pI->iccpPointType  = SDP_POINT_TYPE_SDI;
		pI->iccpPointIndex = 2;
		pPointDef->numPoint++;

		pI++;
		pI->devNo          = 1;
		pI->devPt          = 3;
		pI->config         = 1;
		pI->iccpPointType  = SDP_POINT_TYPE_SAI;
		pI->iccpPointIndex = 1;
		pPointDef->numPoint++;

		pI++;
		pI->devNo          = 1;
		pI->devPt          = 4;
		pI->config         = 1;
		pI->iccpPointType  = SDP_POINT_TYPE_SAI;
		pI->iccpPointIndex = 2;
		pPointDef->numPoint++;

		pI++;
		pI->devNo          = 1;
		pI->devPt          = 5;
		pI->config         = 1;
		pI->iccpPointType  = SDP_POINT_TYPE_SDO;
		pI->iccpPointIndex = 1;
		pPointDef->numPoint++;

		pI++;
		pI->devNo          = 1;
		pI->devPt          = 6;
		pI->config         = 1;
		pI->iccpPointType  = SDP_POINT_TYPE_SDO;
		pI->iccpPointIndex = 2;
		pPointDef->numPoint++;
	}
#endif	// 2016.09 ChoiBC Test

	//{
	//	int i;
	//	ICCP_POINT_DEF *pPointDef = &shmPtr->iccpDCB.pointDef;
	//	ICCP_POINT_INFO *pI = &pPointDef->point[0];

	//	printf ("========== ICCP Point DB \n");
	//	for (i=0; i<MAX_ICCP_POINT; i++, pI++)
	//	{
	//		if (pI->iccpPointType)
	//		{
	//			printf ("%d-%.3d : config(%d) devNo(%d) devPt(%.3d) soe(%d) dbIndex(%d)\n",
	//				pI->iccpPointType, pI->iccpPointIndex,
	//				pI->config, pI->devNo, pI->devPt, pI->soe, pI->dbIndex);
	//		}
	//	}
	//}
    /* ------------------------------------------------------------ */
	/* 프로세스 정보를 초기화한다                                   */
	/* ------------------------------------------------------------ */
	taskPtr->initial    = 1;
	taskPtr->wdtEnable  = 1;            
	
	/* ------------------------------------------------ */
	/*  PROCESS 호출시... Priority 지정 (-19 ~ 20)      */
	/* ------------------------------------------------ */
	nice(NICE_ICCP);
	
	UpdateProcessInfo(taskPtr, 1, getpid(), 1);
	
	return(1);
}

/*
* pI 에거 읽어서 pData에 넣는다.
*/
int getDeviceData (ICCP_POINT_INFO *pI, ICCP_POINT_DATA *pData)
{
	
	// SDP_POINT_TYPE_DEV은 연결된 dev가 없다.
	if (pI->iccpPointType == SDP_POINT_TYPE_DEV) // DevPoint 0 은 all-comm 이었지..
	{
		if (pI->iccpPointIndex >= 1 && pI->iccpPointIndex <= MAX_DEV_POINT)
		{
			pData->flag  = shmPtr->devPtBuf[pI->iccpPointIndex-1].config == 1 ? 0 : 1;
			pData->value = shmPtr->devPtBuf[pI->iccpPointIndex-1].status;
			memcpy (&pData->updateTime, &shmPtr->devPtBuf[pI->iccpPointIndex-1].updateTime,
				sizeof(pData->updateTime));
			return 1;
		}
		printf ("ICCP - getDeviceData() Invalid device iccpPointIndex(%d)\n", pI->iccpPointIndex);
		return 0;
	}
	if (pI->devNo >= 1 && pI->devNo <= MAX_DEVICE)
	{
		if (pI->iccpPointType == SDP_POINT_TYPE_SAI || pI->iccpPointType == SDP_POINT_TYPE_DAI ||
			pI->iccpPointType == SDP_POINT_TYPE_QAI || pI->iccpPointType == SDP_POINT_TYPE_TAI)
		{
			if (pI->devPt >= 1 && pI->devPt <= MAX_DEV_AI_POINT) // shmPtr->deviceCFG는 SDP_DEVICE 이다..
			{
				pData->flag  = shmPtr->deviceCFG[pI->devNo-1].online == 1 ? 0 : 1;
// hkkim
#if 0			
				pData->value = shmPtr->deviceCFG[pI->devNo-1].aiPtBuf[pI->devPt-1].floatData;
#else
                memcpy( (void *)&pData->value, (void *) &shmPtr->deviceCFG[pI->devNo-1].aiPtBuf[pI->devPt-1].floatData, sizeof(float));
#endif 			
			
				memcpy (&pData->updateTime, &shmPtr->deviceCFG[pI->devNo-1].aiPtBuf[pI->devPt-1].updateTime,
						sizeof(pData->updateTime));
				return 1;
			}
		}
		else
		{
			if (pI->devPt >= 1 && pI->devPt <= MAX_DEV_DI_POINT)	// Di
			{
				pData->flag  = shmPtr->deviceCFG[pI->devNo-1].online == 1 ? 0 : 1;
				pData->value = shmPtr->deviceCFG[pI->devNo-1].diPtBuf[pI->devPt-1].status;
				memcpy (&pData->updateTime, &shmPtr->deviceCFG[pI->devNo-1].diPtBuf[pI->devPt-1].updateTime,
						sizeof(pData->updateTime));
				return 1;
			}
		}
	}
	printf ("ICCP - getDeviceData() Invalid devNo(%d) devPt(%d)\n", pI->devNo, pI->devPt);
	return 0;
}

// 2026-04-02 오전 9:17:57 
void enable_keepAlive(int sock_fd)
{
    
    // Keep-alive 설정: 빠른 확인을 위해 짧게 설정
    int optval = 1;
    int idle = 10;  // 10초 동안 데이터 없으면 탐지 시작
    int intvl = 3;  // 3초 간격으로 재시도
    int cnt = 3;    // 3번 응답 없으면 종료

    setsockopt(sock_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
    setsockopt(sock_fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
    setsockopt(sock_fd, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(intvl));
    setsockopt(sock_fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));

    printf("Keep-alive enabled. Monitoring for 100 seconds...\n");
}

/* ------------------------------------------------------- */
/*  ICCP MAIN 프로그램 Start Routine ....                   */
/* ------------------------------------------------------- */
int	main(int argc, char **argv)
{
	time_t	oldSec, newSec;
	char	*appName = NULL;
	int		maxWaitTime = 100;
	int		check_iccpWDT=0;
	char	buffer[128];
	
	// 작업 환경을 초기화한다
	termExec = InitEnv();
	taskPtr->wdtCount = 0;
		
	/* Signal Handler initial */
	signal( SIGBUS,  SigHandler);
	signal( SIGSEGV, SigHandler);
	signal( SIGTERM, SigHandler);	
	signal( SIGPIPE, SigHandler);


//check_alignment();

	pause(100);

#if 0
    while(1)
    {
        taskPtr->wdtCount = 0;
        pause(1000);
    }
#endif
    
	/* ------------------------------------ */    
    /* CPU 링크상태 확인 후 PROCESS 기동    */
    /* ------------------------------------ */  
#ifndef	_PLATFORM_LINUX	// 2016.05.03 ChoiBC Test
    while(opr->chkLinkOK == RESET)
    {
        printf ("ICCP Initialization ... ready \n");
    	taskPtr->wdtCount = 0;
		pause(100);
    }
#endif

	/* ---------------------------------------------------- */
	/*	2020.08.05, FEP 통신 : MASTER 모드일때만 수행 				*/
	/* ---------------------------------------------------- */
	while(opr->runMode == LOCAL_SLAVE)
	{
		taskPtr->wdtCount = 0;
		
		if(opr->iccpDebug)	Debug(console,"  ICCP> ICCP-Stand By...SLAVE-MODE \n");
		pause(1000);
	}
	
	if (termExec)
	{
		if (opr->cpuMode == MPU_B)
			appName = APP_NAME_B;
		else
			appName = APP_NAME_A;
		
		/* Initialize ICCP Lite			*/
		/* cpuMode : A or B */
		if (miuInitIccp (appName, opr->cpuMode, &shmPtr->iccpDCB)) // 이 iccpDCBㅇ
		{
			printf ("ICCP Initialization error\n");
			termExec = 0;
		}
	}
	printf(" %s ICCP(60870-6) PROCESS Activated ... !\n",  TARGET_NAME);
    
	oldSec = time (NULL);
#ifdef	_PLATFORM_LINUX	// 2016.05.03 ChoiBC Test
	opr->iccpDebug = 1;
#endif

	while(termExec)
	{
	    taskPtr->wdtCount = 0;
		pause(100);

		/* ---------------------------------------------------- */
		/*	2020.08.05, FEP 통신 : MASTER 모드일때만 수행 				*/
		/* ---------------------------------------------------- */
		if(opr->runMode == LOCAL_SLAVE)
		{
			printf("ICCP> ICCP-TASK DeActivated in SLAVE MODE\r\n");
			termExec = 0;	
			continue;	
		}
		
		// 2026-05-23 오후 4:09:19 
		//continue ; 
		
		
		/* -------------------------------- */
	    /*	ICCP-HOST 통신 금지시....			*/
	    /* -------------------------------- */	
	    if(mpuCFG->iccpEnbFlag != SET)
	    {
	    	if(opr->iccpDebug)
			Debug(console,"  ICCP> ICCP-READY Status...Flag=%d \n", mpuCFG->iccpEnbFlag);
			
	    	pause(1000);
	    	continue;
	    }
	            
		// Check SOE Queue
		miuCheckEventQueue ();

#ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
		// Check Analog Value Changes
		miuCheckAnalogChanges ();		// 초단위로.... COA Check
#endif
		miuService (&maxWaitTime, appName);

		//if(oldsec == rtc->sec)	continue;
  //      oldsec = rtc->sec;	        
   
		newSec = time (NULL);
		if (newSec == oldSec)	continue;
		oldSec = newSec;

	    /* -------------------------------------------- */
	    /*	ICCP-HOST 통신 이상 Check...					*/
	    /* ICCP-HOST 와 통신이상시... Master/Slave 절체로 복구시도	*/
	    /* -------------------------------------------- */	
	    if(mpuCFG->hostWDT)
	    {
			if((iccpDCB->assocStatus) && (iccpDCB->commMaster))
			{
				check_iccpWDT = 0;
			}
			else
			{
				/* ---------------------------------------- */
				/*	이중화 상태에서.... 소규모 통신이상시... 절체			*/
				/* ---------------------------------------- */
				if((opr->cpuChgMode == CHECK_SDP_FULL) && (opr->dualCpuSts == SET) && (opr->runMode == LOCAL_MASTER) && (scuCfg->remoteMode == AUTO_MODE))
    			{ 
			    	/* 2020.06.03 SCU 통신상태 Check... */
			    	if(scuCfg->online == SET)
					{    		
						if(++check_iccpWDT > opr->iccpChkTime)	// 5분 check
						{
							check_iccpWDT = 0;
					
							Debug(console,"\n==================================\n");      
		        		    Debug(console,"wdt> ***ICCP-DeActive ... CPU-CHANGE (chkTime=%d)...Tick=%d\n", opr->iccpChkTime, check_iccpWDT);
        		    		Debug(console,"==================================\n");
			
							logEvent_MPU(shmPtr, ENT_CHANGE_CPU, CPU_CHG_ACTIVE, 0, 0, opr->cpuMode, NULL); 
			
            				opr->cpuChange = SET;
            			}
            		}				
				}
				
				if(opr->iccpDebug)	Debug(console,"wdt> *** ICCP-DeActive ...count= %d\n", check_iccpWDT);
			}
	    }
	    			
		static int count = 0;
		if (++count >= 10)		// every 10 seconds
		{
			if(opr->iccpDebug)
				Debug(console,"  ICCP> TASK Active...%d \n", oldSec);
				count = 0;
		}
                            		
	}/* while */

	// 작업 환경을 정리한다
	ClearEnv();

	if (appName)
		miuTerminate (appName);
	
	return (0);
}

#endif // _WIN32
