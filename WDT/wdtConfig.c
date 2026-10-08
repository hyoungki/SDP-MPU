/* 
 *****************************************************************************
 * SYSTEM   : ACE-VMECU-NET-V5 (철도청 VMECU : 공통제어부 )
 * FileName : CONFIG.C  
 * File 내용: 데이터베이스 관련 저장 및 초기화 관련 함수
 * Designed : TRATEK ... by LHS
 *****************************************************************************
*/   
#include	"localLib.h"
#include	"external.h"
//#include	"iccpShm.h"

static  short   diPtmax[MAX_HOST];
static  short   doPtmax[MAX_HOST];
static  short   aiPtmax[MAX_HOST];

extern  int             wdtid;  
extern  SHM_MEMORY	    *shmPtr;


/*
*   SDP-연산포인트 [상태] 포인트 초기화 
*/
int update_iccpInfo(int devNo, int devPt, POINT_BUF *ptBuf)
{
    int             pointInx;
    I60870_DATA     *iccpPt;
    
#if 0
typedef struct
{
    byte            devNo;          // SDP POINT : device 번호 [1..32] 
    word            devPt;          // SDP POINT : device 포인트 번호 [1..1024] 
    byte            devType;        // SDP POINT : devic TYPE      
        
    byte            config;         // Device Config Type, [0]미정의, [1] 정의
    byte            ptNameStr[40];  // SDP POINT : 포인트 이름
            
    int             reportFlag;     // ICCP-HOST Report Status...[0] X, [1] Report
        
	int				online;			// 0:online, 1:offline
	float			value;			// 포인트 정보 : 상태/계측 포인트 정보
	struct timeval	updateTime;		// 포인트 정보 Update Time
	
} __attribute__ ((packed)) I60870_DATA;
#endif
    
    pointInx = ptBuf->iccpIndex - 1;
    if((pointInx < 0) || (pointInx >= MAX_ICCP_POINT))   return (0);
        
    if(ptBuf->iccpType == SDP_POINT_TYPE_SDI)       
    {
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_sdi[pointInx];
        if(ptBuf->iccpIndex >= iccpInfo->maxIndex_sdi)  iccpInfo->maxIndex_sdi =  ptBuf->iccpIndex; 
    }
    else if(ptBuf->iccpType == SDP_POINT_TYPE_SDO)  
    {
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_sdo[pointInx];
        if(ptBuf->iccpIndex >= iccpInfo->maxIndex_sdo)  iccpInfo->maxIndex_sdo =  ptBuf->iccpIndex; 
    }
    else if(ptBuf->iccpType == SDP_POINT_TYPE_SAI)  
    {
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_sai[pointInx];
        if(ptBuf->iccpIndex >= iccpInfo->maxIndex_sai)  iccpInfo->maxIndex_sai =  ptBuf->iccpIndex; 
    }
    else if(ptBuf->iccpType == SDP_POINT_TYPE_DDI)  
    {
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_ddi[pointInx];    
        if(ptBuf->iccpIndex >= iccpInfo->maxIndex_ddi)  iccpInfo->maxIndex_ddi =  ptBuf->iccpIndex; 
    }
    else if(ptBuf->iccpType == SDP_POINT_TYPE_DAI)  
    {
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_dai[pointInx];    
        if(ptBuf->iccpIndex >= iccpInfo->maxIndex_dai)  iccpInfo->maxIndex_dai =  ptBuf->iccpIndex; 
    }
    else if(ptBuf->iccpType == SDP_POINT_TYPE_QDI)  
    {
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_qdi[pointInx];    
        if(ptBuf->iccpIndex >= iccpInfo->maxIndex_qdi)  iccpInfo->maxIndex_qdi =  ptBuf->iccpIndex; 
    }
    else if(ptBuf->iccpType == SDP_POINT_TYPE_QAI)  
    {
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_qai[pointInx];    
        if(ptBuf->iccpIndex >= iccpInfo->maxIndex_qai)  iccpInfo->maxIndex_qai =  ptBuf->iccpIndex; 
    }
    else if(ptBuf->iccpType == SDP_POINT_TYPE_TDI)  
    {
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_tdi[pointInx]; 
        if(ptBuf->iccpIndex >= iccpInfo->maxIndex_tdi)  iccpInfo->maxIndex_tdi =  ptBuf->iccpIndex; 
    }
    else if(ptBuf->iccpType == SDP_POINT_TYPE_TAI)  
    {
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_tai[pointInx];
        if(ptBuf->iccpIndex >= iccpInfo->maxIndex_tai)  iccpInfo->maxIndex_tai =  ptBuf->iccpIndex; 
    }
    else if(ptBuf->iccpType == SDP_POINT_TYPE_DEV)  
    {
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_dev[pointInx]; 
        if(ptBuf->iccpIndex >= iccpInfo->maxIndex_dev)  iccpInfo->maxIndex_dev =  ptBuf->iccpIndex; 
    }
    else    return(0);       
        
    iccpPt->devNo = ptBuf->devNo;
    iccpPt->devPt = ptBuf->devPt;
    iccpPt->devType = ptBuf->devType;
    iccpPt->config = SET;
    
    memcpy( (byte *) iccpPt->ptNameStr, (byte *) ptBuf->ptNameStr, 40);
    
    iccpPt->offline = SET;       // Offline
    iccpPt->value  = 0; 
    
    return (0);
}

/*
*   SDP-연산포인트 [상태] 포인트 초기화 
*/
int sdp_vdiPoint_initial(int devNo, int devPt, POINT_BUF *dPtBuf)
{
    //int     i;
    int     hostid;
    int     calPoint;
    word    dnpPoint;
    
    SDP_DEVICE      *dev=NULL;
    POINT_BUF       *ptBuf=NULL;             // 계전기 포인트 구조체
    HOST_DCB        *host=NULL;
    
    DIPOINT_INFO    *hostSts=NULL;
    CAL_POINT_BUF   *calBuf;
    //DIPOINT_INFO    *rtuSts=NULL;
    
    /* ------------------------------------ */
    /*  계전기 정보 추출                    */
    /* ------------------------------------ */
    dev   = (SDP_DEVICE *) deviceCFG[devNo - 1]; 
    ptBuf = (POINT_BUF *) &dev->diPtBuf[devPt - 1];

    /* ------------------------------------ */
    /* 계전기/POINT 구조체 정보 초기화      */
    /* ------------------------------------ */
    memcpy((byte *) ptBuf, (byte *) dPtBuf, sizeof(POINT_BUF));
    ptBuf->config = SET;
    
    dev->regDiPointNum++;	// 계전기별 DI 등록 포인트 수 

    /* ------------------------------------ */
    /*  연산포인트 그룹 지정...             */
    /* ------------------------------------ */
    calPoint = ptBuf->localIndex;                   // CAL-POINT 용 인덱스
    
    if(calPoint > 0)
    {
        calPoint = (calPoint - 1) & 0x1f;
        calBuf = (CAL_POINT_BUF *) calPtBuf[calPoint];    
        
        calBuf->devNo   = devNo;          // SDP POINT : device 번호 [1..32] 
        calBuf->devPt   = devPt;          // SDP POINT : device 포인트 번호 [1..1024] 
        calBuf->config  = SET;           
    }
          
    
    /* ------------------------------------ */
    /* DNP-HOST 별 상태포인트 정보 저장 ... */
    /* ------------------------------------ */            
    for(hostid = 0; hostid < MAX_HOST; hostid++)
    {
        host = (HOST_DCB *) hostDCB[hostid];
        dnpPoint = ptBuf->hostIndex[hostid];
                
        if((dnpPoint > 0) && (dnpPoint < MAX_DNP_DI_POINT))
        {
            /* HOST별 최대 포인트 등록 */
            if(dnpPoint >= diPtmax[hostid]) diPtmax[hostid] = dnpPoint;
            host->diPtNum = diPtmax[hostid];
                    
            /* HOST 별 상태포인트 구조체 초기화 */
            hostSts = (DIPOINT_INFO *) &host->stateInfo[dnpPoint-1];
            hostSts->devNo = devNo;     // 계전기 번호, 1...64                                         
            hostSts->devPt = devPt;     // 계전기내 포인트 번호, 1...4096
            hostSts->config= SET;   
        }                
    }

    return (0);
}

/*
*   SDP-연산포인트 [계측] 포인트 초기화 
*/
int sdp_vaiPoint_initial(int devNo, int devPt, POINT_BUF *dPtBuf)
{
    //int     i;
    int     hostid;
    int     calPoint;
    word    dnpPoint;
    
    SDP_DEVICE      *dev=NULL;
    POINT_BUF       *ptBuf=NULL;             // 계전기 포인트 구조체
    HOST_DCB        *host=NULL;
    
    AIPOINT_INFO    *hostAna=NULL;
    CAL_POINT_BUF   *calBuf;
    //DIPOINT_INFO    *rtuSts=NULL;
    
    /* ------------------------------------ */
    /*  계전기 정보 추출                    */
    /* ------------------------------------ */
    dev   = (SDP_DEVICE *) deviceCFG[devNo - 1]; 
    ptBuf = (POINT_BUF *) &dev->aiPtBuf[devPt - 1];

    /* ------------------------------------ */
    /* 계전기/POINT 구조체 정보 초기화      */
    /* ------------------------------------ */
    memcpy((byte *) ptBuf, (byte *) dPtBuf, sizeof(POINT_BUF));
    ptBuf->config = SET;
    
    dev->regAiPointNum++;	// 계전기별 AI 등록 포인트 수 

    /* ------------------------------------ */
    /*  연산포인트 그룹 지엉...             */
    /* ------------------------------------ */
    calPoint = ptBuf->localIndex;                   // CAL-POINT 용 인덱스
    
    if(calPoint > 0)
    {
        calPoint = (calPoint - 1) & 0x1f;
        calBuf = (CAL_POINT_BUF *) calPtBuf[calPoint];    
        
        calBuf->devNo = devNo;          // SDP POINT : device 번호 [1..32] 
        calBuf->devPt = devPt;          // SDP POINT : device 포인트 번호 [1..1024] 
        
        calBuf->config = SET;           
    }
          
    
    /* ------------------------------------ */
    /* DNP-HOST 별 상태포인트 정보 저장 ... */
    /* ------------------------------------ */            
    for(hostid = 0; hostid < MAX_HOST; hostid++)
    {
        host = (HOST_DCB *) hostDCB[hostid];
        dnpPoint = ptBuf->hostIndex[hostid];
                
        if((dnpPoint > 0) && (dnpPoint < MAX_DNP_DI_POINT))
        {
            /* HOST별 최대 포인트 등록 */
            if(dnpPoint >= aiPtmax[hostid]) aiPtmax[hostid] = dnpPoint;
            host->aiPtNum = aiPtmax[hostid];
                    
            /* HOST 별 상태포인트 구조체 초기화 */
            hostAna = (AIPOINT_INFO *) &host->analogInfo[dnpPoint-1];
            hostAna->devNo = devNo;     // 계전기 번호, 1...64                                         
            hostAna->devPt = devPt;     // 계전기내 포인트 번호, 1...4096
            hostAna->config= SET;   
        }                
    }

    return (0);
}

/*
*   SDP-계전기- 상태포인트 초기화 
*/
int sdp_diPoint_initial(int devNo, int devPt, POINT_BUF *dPtBuf)
{
    //int     i;
    int     hostid;
    word    dnpPoint;
    SDP_DEVICE      *dev=NULL;
    POINT_BUF       *ptBuf=NULL;             // 계전기 포인트 구조체
    HOST_DCB        *host=NULL;
    
    DIPOINT_INFO    *hostSts=NULL;
    //DIPOINT_INFO    *rtuSts=NULL;
    
    /* ------------------------------------ */
    /*  계전기 정보 추출                    */
    /* ------------------------------------ */
    dev   = (SDP_DEVICE *) deviceCFG[devNo - 1]; 
    ptBuf = (POINT_BUF *) &dev->diPtBuf[devPt - 1];

    /* ------------------------------------ */
    /* 계전기/POINT 구조체 정보 초기화      */
    /* ------------------------------------ */
    memcpy((byte *) ptBuf, (byte *) dPtBuf, sizeof(POINT_BUF));
    
    ptBuf->config = SET;
    dev->regDiPointNum++;	// 계전기별 DI 등록 포인트 수 

    /* ------------------------------------ */
    /* DNP-HOST 별 상태포인트 정보 저장 ... */
    /* ------------------------------------ */            
    for(hostid = 0; hostid < MAX_HOST; hostid++)
    {
        host = (HOST_DCB *) hostDCB[hostid];
        dnpPoint = ptBuf->hostIndex[hostid];
                
        if((dnpPoint > 0) && (dnpPoint < MAX_DNP_DI_POINT))
        {
            /* HOST별 최대 포인트 등록 */
            if(dnpPoint >= diPtmax[hostid]) diPtmax[hostid] = dnpPoint;
            host->diPtNum = diPtmax[hostid];
                    
            /* HOST 별 상태포인트 구조체 초기화 */
            hostSts = (DIPOINT_INFO *) &host->stateInfo[dnpPoint-1];
            hostSts->devNo = devNo;     // 계전기 번호, 1...64                                         
            hostSts->devPt = devPt;     // 계전기내 포인트 번호, 1...4096
            hostSts->config= SET;   
        }                
    }

    return (0);
}

/*
*   SDP-계전기- 제어포인트 초기화 
*/
int sdp_doPoint_initial(int devNo, int devPt, POINT_BUF *dPtBuf)
{
    //int     i;
    int     hostid;
    word    dnpPoint;
    SDP_DEVICE      *dev=NULL;
    POINT_BUF       *ptBuf=NULL;             // 계전기 포인트 구조체
    HOST_DCB        *host=NULL;
    
    CONTROL_INFO    *hostCntr=NULL;
    //DOPOINT_INFO    *rtuCntr=NULL;
    
    /* ------------------------------------ */
    /*  계전기 정보 추출                    */
    /* ------------------------------------ */
    dev   = (SDP_DEVICE *) deviceCFG[devNo - 1]; 
    ptBuf = (POINT_BUF *) &dev->doPtBuf[devPt - 1];

    /* ------------------------------------ */
    /* 계전기/POINT 구조체 정보 초기화      */
    /* ------------------------------------ */
    memcpy((byte *) ptBuf, (byte *) dPtBuf, sizeof(POINT_BUF));
    
    ptBuf->config = SET;

    /* ------------------------------------ */
    /* DNP-HOST 별 제어정보 저장 ...        */
    /* ------------------------------------ */
    for(hostid = 0; hostid < MAX_HOST; hostid++)
    {
        host = (HOST_DCB *) hostDCB[hostid];
        dnpPoint = ptBuf->hostIndex[hostid];
        
        if((dnpPoint > 0) && (dnpPoint < MAX_DNP_DO_POINT))    
        {
            if(dnpPoint >= doPtmax[hostid]) doPtmax[hostid] = dnpPoint;
            host->doPtNum = doPtmax[hostid];
            
            /* HOST 별 제어포인트 구조체 초기화 */
            hostCntr = (CONTROL_INFO *) &host->controlInfo[dnpPoint-1];
            hostCntr->devNo = devNo;                        // 계전기 번호, 1,2,..64
            hostCntr->devPt = devPt;                        // 계전기 포인트 번호, 1,2...64
            
            hostCntr->type  = CONTROL_POINT;                // 계전기 포인트 제어
            hostCntr->dbmax = ptBuf->pointMax;
            hostCntr->config= SET;
            hostCntr->cntrConfig = ptBuf->ptConfig;       // 제어포인트 속성 지정 (Pulse, Latch)
        }                
    }

#if 0    
    /* ------------------------------------ */
    /* SLAVE-RTU 제어포인트 정보 저장 ... */
    /* ------------------------------------ */   
    if(ptBuf->localIndex > 0)
    {
        /* HOST 별 상태포인트 구조체 초기화 */
        rtuCntr = (CONTROL_INFO *) &rtuDCB->controlInfo[ptBuf->localIndex - 1];
        rtuCntr->devNo = devNo;          // 물리적 계전기 번호, 1...64                               
        rtuCntr->devPt = devPt;          // 물리적 계전기내 포인트 번호, 1...4096
        
        rtuCntr->type  = CONTROL_POINT;                // 계전기 포인트 제어
        rtuCntr->dbmax = ptBuf->pointMax;
        rtuCntr->config= SET;
        rtuCntr->cntrConfig = ptBuf->ptConfig;       // 제어포인트 속성 지정 (Pulse, Latch)
    }
#endif
    
    return (0);
}

/*
*   SDP-계전기- 계측포인트 초기화 
*/
int sdp_aiPoint_initial(int devNo, int devPt, POINT_BUF *dPtBuf)
{
    //int     i;
    int     hostid;
    word    dnpPoint;
    SDP_DEVICE      *dev=NULL;
    POINT_BUF       *ptBuf=NULL;             // 계전기 포인트 구조체
    HOST_DCB        *host=NULL;
    
    AIPOINT_INFO    *hostAna=NULL;
    //AIPOINT_INFO    *rtuAna=NULL;
    
    /* ------------------------------------ */
    /*  계전기 정보 추출                    */
    /* ------------------------------------ */
    dev   = (SDP_DEVICE *) deviceCFG[devNo - 1]; 
    ptBuf = (POINT_BUF *) &dev->aiPtBuf[devPt - 1];

    /* ------------------------------------ */
    /* 계전기/POINT 구조체 정보 초기화      */
    /* ------------------------------------ */
    memcpy((byte *) ptBuf, (byte *) dPtBuf, sizeof(POINT_BUF));
    
    ptBuf->config = SET;
    dev->regAiPointNum++;	// 계전기별 AI 등록 포인트 수 

    /* ------------------------------------ */
    /* DNP-HOST 별 계측정보 저장 ...        */
    /* ------------------------------------ */
    for(hostid = 0; hostid < MAX_HOST; hostid++)
    {
        host = (HOST_DCB *) hostDCB[hostid];
        dnpPoint = ptBuf->hostIndex[hostid];
        
        if((dnpPoint > 0) && (dnpPoint < MAX_DNP_AI_POINT))        
        {
            if(dnpPoint >= aiPtmax[hostid]) aiPtmax[hostid] = dnpPoint;
            host->aiPtNum = aiPtmax[hostid];
            
            /* HOST 별 계측포인트 구조체 초기화 */
            hostAna = (AIPOINT_INFO *) &host->analogInfo[dnpPoint-1];
            hostAna->devNo = devNo; 
            hostAna->devPt = devPt; 
            hostAna->config= SET; 
        }                
    }

#if 0    
    /* ------------------------------------ */
    /* SLAVE-RTU 계측포인트 정보 저장 ...   */
    /* ------------------------------------ */   
    if(ptBuf->localIndex > 0)
    {
        /* HOST 별 상태포인트 구조체 초기화 */
        rtuAna = (AIPOINT_INFO *) &rtuDCB->analogInfo[ptBuf->localIndex - 1];
        rtuAna->devNo = devNo;          // 물리적 계전기 번호, 1...64                               
        rtuAna->devPt = devPt;          // 물리적 계전기내 포인트 번호, 1...4096
        
        rtuAna->config= SET;
    }
#endif
    
    return (0);
}

/*
*   SDP-계전기- Device 포인트 초기화 
*/
int sdp_devPoint_initial(int devNo, int devPt, POINT_BUF *dPtBuf)
{
    int     hostid;
    int     index;
    word    dnpPoint;
    POINT_BUF       *ptBuf=NULL;             // 계전기 포인트 구조체
    POINT_BUF       *diptBuf=NULL;
    HOST_DCB        *host=NULL;
    
    CONTROL_INFO    *hostCntr=NULL;
    DIPOINT_INFO    *hostSts=NULL;
    SDP_DEVICE      *dev=NULL;

//  2026-09-11 오후 2:35:47    
//  printf("DEV-TYPE [%d][%d]\r\n",devNo, devPt) ;
    
    /* ---------------------------------------- */
    /*  계전기 Type == DEVICE 인경우...         */
    /* ---------------------------------------- */
    /* RDEVPT : real Dev ?    CNTRPT : control   MCPUPT : */
    if(dPtBuf->ptType == RALLPT)        index = 0;            			// 가상 포인트 : 전체 계전기 통신상태

    // 지금 DB를 보면 devNo를 모두 1로 넣는데ㅔ         아니군..
    else if(dPtBuf->ptType == RDEVPT)   index = devNo;        			// 가상 포인트 : 개별 계전기 통신상태 , 계전기 번호,1,2,3...      
    // 다른 것들은 devNo가 중보하지 않고..ptType 가 중요하군.
    else if(dPtBuf->ptType == MCPUPT)   index = INDEX_AUTO_MANUAL;      // 가상 포인트 : 자동/수동 상태
    else if(dPtBuf->ptType == CNTRPT)   index = INDEX_CNTR_CHANGE;      // 가상 포인트 : 이중화 절체제어
    else
    { // 위 PtType를 제외하고..?
#ifdef	VITZRO_FEP_ENABLE    
		/* 비츠로시스 : 전체 Device-Point 정의 */
    	index = dPtBuf->iccpIndex;
    	if(index >= MAX_DEV_POINT)	 index = MAX_DEV_POINT - 1;
#else    		
        return (0);
#endif        
    }
     
    /* ------------------------------------ */                 
    /* 장치 포인트 구조체 초기화            */           
    /* ------------------------------------ */             
    ptBuf = (POINT_BUF *) devPtBuf[index];

    /* ------------------------------------ */
    /* 장치 포인트  구조체 정보 초기화      */
    /* ------------------------------------ */
    memcpy((byte *) ptBuf, (byte *) dPtBuf, sizeof(POINT_BUF));           
    
    // 이 것이 db의 포인트속성->CONFIG가 아니고 db가 설정되었다는 것이네
    ptBuf->config = SET; // config=SET 의 의미는  ptBuf가 의미가 있다는 것.

    /* ------------------------------------ */
    /*  계전기-DI 포인트 정보 Update        */
    /* ------------------------------------ */
    /* 이 point가 update되고 사용되는 것을 찾아보쟈..*/
    dev   = (SDP_DEVICE *) deviceCFG[devNo - 1]; 
    diptBuf = (POINT_BUF *) &dev->diPtBuf[devPt - 1];
    memcpy((byte *) diptBuf, (byte *) dPtBuf, sizeof(POINT_BUF));           
    diptBuf->config = SET;
    
    dev->regDiPointNum++;	// 계전기별 DI 등록 포인트 수 
 
    /* ------------------------------------ */
    /* 이중화 절체 포인트(제어)정보 저장 .. */
    /* ------------------------------------ */
    if(dPtBuf->ptType == CNTRPT)
    {   
        /* DNP-HOST 제어 구조체 정보 추가 */     
        for(hostid = 0; hostid < MAX_HOST; hostid++)
        {
            host = (HOST_DCB *) hostDCB[hostid];
            dnpPoint = ptBuf->hostIndex[hostid];
        
            if((dnpPoint > 0) && (dnpPoint < MAX_DNP_DO_POINT))    
            {
                if(dnpPoint >= doPtmax[hostid]) doPtmax[hostid] = dnpPoint;
                host->doPtNum = doPtmax[hostid];
            
                /* HOST 별 제어포인트 구조체 초기화 */
                hostCntr = (CONTROL_INFO *) &host->controlInfo[dnpPoint-1];
                hostCntr->devNo = devNo; 
                hostCntr->devPt = devPt; 
            
                hostCntr->type  = CONTROL_SYSTEM;               // 이중화 절체 포인트 제어
                
                hostCntr->dbmax = ptBuf->pointMax;
                hostCntr->config= SET;
                hostCntr->cntrConfig = ptBuf->ptConfig;         // 제어포인트 속성 지정 (Pulse, Latch)
                
                //printf("*** CNTRPT config... host=%d, dnpPoint=%d... dev=%d, pt=%d\n", hostid, dnpPoint, devNo, devPt);
            }
        }                
    }
    else
    {
	    /* ------------------------------------ */
	    /* DNP-HOST 별 상태포인트 정보 저장 ... */
    	/* ------------------------------------ */            
	    for(hostid = 0; hostid < MAX_HOST; hostid++)
    	{
	        host = (HOST_DCB *) hostDCB[hostid];
    	    dnpPoint = ptBuf->hostIndex[hostid];
                
	        if((dnpPoint > 0) && (dnpPoint < MAX_DNP_DI_POINT))
    	    {
	            /* HOST별 최대 포인트 등록 */
    	        if(dnpPoint >= diPtmax[hostid]) diPtmax[hostid] = dnpPoint;
        	    host->diPtNum = diPtmax[hostid];
                    
	            /* HOST 별 상태포인트 구조체 초기화 */
    	        hostSts = (DIPOINT_INFO *) &host->stateInfo[dnpPoint-1];
        	    hostSts->devNo = devNo;     // 계전기 번호, 1...64                                         
            	hostSts->devPt = devPt;     // 계전기내 포인트 번호, 1...4096
	            hostSts->config= SET;   
    	    }                
    	}
        	
    }
    	
    return (0);
}

/*
*   SDP-계전기- VIRTUAL Device 포인트 초기화 
* 2026-09-11 오후 4:28:12 sdp_devPoint_initial 에서 하는 것 말고는 여기서 처리하나..
*/
int sdp_virPoint_initial(int devNo, int devPt, POINT_BUF *dPtBuf)
{
    int     index;
	int     hostid;
    word    dnpPoint;
    HOST_DCB        *host=NULL;
    POINT_BUF       *ptBuf=NULL;             // 계전기 포인트 구조체
    POINT_BUF       *diptBuf=NULL;
    SDP_DEVICE      *dev=NULL;
    DIPOINT_INFO    *hostSts=NULL;

    /* ---------------------------------------- */
    /*  계전기 Type == 가상포인트 관련...       */
    /* ---------------------------------------- */
    if(dPtBuf->devType == SDP_STS_POINT)        
    {
        if(dPtBuf->ptType == MCPUPT)            index = INDEX_SDP_TIMESYNC;    // 가상 포인트 : SDP 시각동기 요청(상태) 
        else if(dPtBuf->ptType == SCPUPT)       index = INDEX_SDP_STS;         // 가상 포인트 : SDP-STATUS (상태) 
        else                                    index = INDEX_SDP_STS;         // 가상 포인트 : SDP-STATUS (상태)     
    }            
    else if(dPtBuf->devType == CPU_RUN_POINT)   index = INDEX_SDP_RUN_MODE;    // 가상 포인트 : CPU 동작정보 ,[0] 1계, [1] 2계 동작정보  
    else if(dPtBuf->devType == CPU_STS_POINT)  
    {
        if(dPtBuf->ptType == MCPUPT)            index = INDEX_SDP_RUN_A;        // 가상 포인트 : CPU 1계 상태정보, [0] 정상,[1] 이상
        else if(dPtBuf->ptType == SCPUPT)       index = INDEX_SDP_RUN_B;        // 가상 포인트 : CPU 2계 상태정보, [0] 정상,[1] 이상  
        else 									index = INDEX_SDP_RUN_A;        // 가상 포인트 : CPU 1계 상태정보, [0] 정상,[1] 이상	  
    }
    else if(dPtBuf->devType == RTU_RUN_POINT)   index = INDEX_RTU_MODE;         // 가상 포인트 : RTU 동작정보 ,[0] 1계, [1] 2계 동작정보 
    else if(dPtBuf->devType == RTU_STS_POINT)  
    {
        if(dPtBuf->ptType == MCPUPT)            index = INDEX_RTU_RUN_A;        // 가상 포인트 : RTU 1계 상태정보, [0] 정상,[1] 이상
        else if(dPtBuf->ptType == SCPUPT)       index = INDEX_RTU_RUN_B;        // 가상 포인트 : RTU 2계 상태정보, [0] 정상,[1] 이상  
        else 									index = INDEX_RTU_RUN_A;        // 가상 포인트 : RTU 1계 상태정보, [0] 정상,[1] 이상	
    }
    else if(dPtBuf->devType == CU_RUN_POINT)    index = INDEX_SCADA_MODE;       // 가상 포인트 : 전철제어반 동작정보 ,[0] 1계, [1] 2계 동작정보 
    else if(dPtBuf->devType == CU_STS_POINT)  
    {
        if(dPtBuf->ptType == MCPUPT)            index = INDEX_SCADA_RUN_A;      // 가상 포인트 : 전철제어반 1계 상태정보, [0] 정상,[1] 이상
        else if(dPtBuf->ptType == SCPUPT)       index = INDEX_SCADA_RUN_B;      // 가상 포인트 : 전철제어반 2계 상태정보, [0] 정상,[1] 이상
      	else 									index = INDEX_SCADA_RUN_A;      // 가상 포인트 : 전철제어반 1계 상태정보, [0] 정상,[1] 이상
    }
    else if(dPtBuf->devType == DIG_RUN_POINT)   index = INDEX_REMOTE_MODE;      // 가상 포인트 : 원격진단부 동작정보 ,[0] 1계, [1] 2계 동작정보 
    else if(dPtBuf->devType == DIG_STS_POINT)  
    {
        if(dPtBuf->ptType == MCPUPT)            index = INDEX_REMOTE_RUN_A;     // 가상 포인트 : 원격진단부 1계 상태정보, [0] 정상,[1] 이상
        else if(dPtBuf->ptType == SCPUPT)       index = INDEX_REMOTE_RUN_B;     // 가상 포인트 : 원격진단부 2계 상태정보, [0] 정상,[1] 이상
        else      								index = INDEX_REMOTE_RUN_A;     // 가상 포인트 : 원격진단부 1계 상태정보, [0] 정상,[1] 이상	
    }
    else if(dPtBuf->devType == EQM_RUN_POINT)   index = INDEX_ELECQ_MODE;       // 가상 포인트 : 전력품질부 동작정보 ,[0] 1계, [1] 2계 동작정보 
    else if(dPtBuf->devType == EQM_STS_POINT)  
    {
        if(dPtBuf->ptType == MCPUPT)            index = INDEX_ELECQ_RUN_A;      // 가상 포인트 : 전력품질부 상태정보, [0] 정상,[1] 이상
        else if(dPtBuf->ptType == SCPUPT)       index = INDEX_ELECQ_RUN_B;      // 가상 포인트 : 전력품질부 상태정보, [0] 정상,[1] 이상
       	else  									index = INDEX_ELECQ_RUN_A;      // 가상 포인트 : 전력품질부 상태정보, [0] 정상,[1] 이상
    }
    else if(dPtBuf->devType == IEC_RUN_POINT)   index = INDEX_61850_MODE;       // 가상 포인트 : 61850 동작정보 ,[0] 1계, [1] 2계 동작정보 
    else if(dPtBuf->devType == IEC_STS_POINT)  
    {
        if(dPtBuf->ptType == MCPUPT)            index = INDEX_61850_RUN_A;      // 가상 포인트 : 61850 1계 상태정보, [0] 정상,[1] 이상
        else if(dPtBuf->ptType == SCPUPT)       index = INDEX_61850_RUN_B;      // 가상 포인트 : 61850 2계 상태정보, [0] 정상,[1] 이상
        else    								index = INDEX_61850_RUN_A;      // 가상 포인트 : 61850 1계 상태정보, [0] 정상,[1] 이상	
    }
    else
    {
        return (0);
    }     
        
    /* 장치 포인트 구조체 초기화 */                        
    ptBuf = (POINT_BUF *) devPtBuf[index];

    /* ------------------------------------ */
    /* 계전기/POINT 구조체 정보 초기화      */
    /* ------------------------------------ */
    memcpy((byte *) ptBuf, (byte *) dPtBuf, sizeof(POINT_BUF));           
    ptBuf->config = SET;

    /* ------------------------------------ */
    /*  계전기-DI 포인트 정보 Update        */
    /* ------------------------------------ */
    dev   = (SDP_DEVICE *) deviceCFG[devNo - 1]; 
    diptBuf = (POINT_BUF *) &dev->diPtBuf[devPt - 1];
    memcpy((byte *) diptBuf, (byte *) dPtBuf, sizeof(POINT_BUF));           
    diptBuf->config = SET;
    
    dev->regDiPointNum++;	// 계전기별 DI 등록 포인트 수 

    /* ------------------------------------ */
    /* DNP-HOST 별 상태포인트 정보 저장 ... */
    /* ------------------------------------ */            
    for(hostid = 0; hostid < MAX_HOST; hostid++)
    {
        host = (HOST_DCB *) hostDCB[hostid];
        dnpPoint = ptBuf->hostIndex[hostid];
                
        if((dnpPoint > 0) && (dnpPoint < MAX_DNP_DI_POINT))
        {
            /* HOST별 최대 포인트 등록 */
            if(dnpPoint >= diPtmax[hostid]) diPtmax[hostid] = dnpPoint;
            host->diPtNum = diPtmax[hostid];
                    
            /* HOST 별 상태포인트 구조체 초기화 */
            hostSts = (DIPOINT_INFO *) &host->stateInfo[dnpPoint-1];
            hostSts->devNo = devNo;     // 계전기 번호, 1...64                                         
            hostSts->devPt = devPt;     // 계전기내 포인트 번호, 1...4096
            hostSts->config= SET;   
        }                
    }
        
    return (0);
}


/*----------------------------------------------------------------------------
* Function Name : pointParaConfig()
* 수행내용: 데이터베이스중 Runtime 포인트 정보를 DB에서 읽어 온다.
* ArgList :
*   1. ioid - 초기화 되고자 하는 모듈번호
*   2. buf  - 초기화 되는 데이터베이스상의 Pointer Address
* Return  :  
---------------------------------------------------------------------------- */   
void pointParaConfig()
{
    int     i;
    int     hostid;
    int     devNo, devPt;
    
    DB_POINT_BUF    *devPoint;      // DB용 Point 구조체
    POINT_BUF       dPtBuf;         // running 계전기 포인트 구조체
    SDP_DEVICE      *dev;           // running Device 구조체
    
    /* ------------------------------------ */
    /*  포인트 DB 초기화 .... Update 대기   */
    /*  - 장치 포인트 초기정보              */
    /* ------------------------------------ */
    for(devNo=0; devNo < MAX_DEVICE; devNo++)
    {
        /* -------------------------------------------- */
        /* 계전기 데이터베이스 초기화                   */
        /* -------------------------------------------- */
        dev   = (SDP_DEVICE *) deviceCFG[devNo];  
        dev->regDiPointNum = 0;
        dev->regAiPointNum = 0;
    }
        
    /* ------------------------------------ */
    /*  HOST 구조체 초기화                  */
    /* ------------------------------------ */    
    for(i=0; i< MAX_HOST; i++)
    {
        hostDCB[i]->diPtNum = 0;
        hostDCB[i]->doPtNum = 0;
        hostDCB[i]->aiPtNum = 0;
        hostDCB[i]->aoPtNum = 0;
        hostDCB[i]->devPtNum = 0;
        
        diPtmax[i] = 0;
        doPtmax[i] = 0;
        aiPtmax[i] = 0;
    }

    /* ------------------------------------ */
    /*  ICCP-INFO 구조체 초기화             */
    /* ------------------------------------ */ 
    bzero( iccpInfo, sizeof(ICCP_60870_DCB));
    
    opr->max_sdpPoint = 0;
    
    /* -------------------------------------------------------- */
    /*  데이터베이스 상의 포인트 DB 초기화...(max 4096)         */
    /*  rtudb에 DB_POINT를 SDP POINT로 옮기기 위해 local dPtBuf 를 이용
    /* -------------------------------------------------------- */
    for(i = 0; i < MAX_DBASE_POINT; i++)
    {
        /* DB 상의 포인트 구조체 ... */
        devPoint = (DB_POINT_BUF *) &rtudb->pointBuf[i];

        devNo = devPoint->devNo;                                        // 장치 ID [1..64]
        devPt = (devPoint->devPt[0]*256) + devPoint->devPt[1];          // 장치-포인트 [1...4096]
        
        /* ---------------------------------------- */
        /* 입력상의 오류 검증...                    */
        /* ---------------------------------------- */
        if((devNo < 1) || (devNo > MAX_DEVICE))         continue;
        if((devPt < 1) || (devPt > MAX_DEV_DI_POINT))   continue;
        if(devPoint->devType == NULL_DEV)   			continue;

        /* ------------------------------------------------ */
        /*  계전기 정보 추출 : 계전기별 포인트 구성정보     */
        /* ------------------------------------------------ */
        bzero((byte *) &dPtBuf, sizeof(POINT_BUF));
        
        dPtBuf.devNo = devNo;                           // SDP POINT : device 번호 [1..32] */
        dPtBuf.devPt = devPt;                           // SDP POINT : device 포인트 번호 [1..1024] */
        dPtBuf.devType = devPoint->devType;		        // SDP POINT : devic TYPE  */
     
        dPtBuf.ptType   = devPoint->ptType;             // SDP POINT : device 포인트 TYPE  */    
        dPtBuf.ptConfig = devPoint->ptConfig;           // SDP POINT : Point Config
        
        /* ICCP 관련 포인트 정보 추출 */
        dPtBuf.iccpType    = devPoint->iccpPointType;		                                    // SDP POINT : ICCP 포인트 TYPE, SDI/SDO/SAI/DDI/DAI/QDI/QAI/TDI/TAI/DEV
        dPtBuf.iccpIndex   = (devPoint->iccpPointIndex[0]*256) + devPoint->iccpPointIndex[1];   // SDP POINT : ICCP 포인트 인덱스, [0: 미지정, 1 ~ 1024]
        dPtBuf.iccpRes1    = devPoint->reserved1;
        dPtBuf.iccpRes2    = devPoint->reserved2;
        
        /* 계전기 관련 포인트 정보 추출 */
        dPtBuf.pointMax    = (devPoint->pointMax[0]*256) + devPoint->pointMax[1];          // SDP POINT :Point MAX Scale 값
        dPtBuf.pointOffset = (devPoint->pointOffset[0]*256) + devPoint->pointOffset[1];    // SDP POINT :Point Offset Scale 값
        dPtBuf.pointDelta  = devPoint->pointDelta;                                         // SDP POINT :Point Delts Scale 값
        
        dPtBuf.localIndex  = (devPoint->localIndex[0]*256) + devPoint->localIndex[1];      // SDP POINT : Local Index
        
        dPtBuf.modBase  = (devPoint->modBase[0]*256) + devPoint->modBase[1];               // SDP POINT :MODBUS base Address
        dPtBuf.modIndex = (devPoint->modIndex[0]*256) + devPoint->modIndex[1];             // SDP POINT :MODBUS index
        
        dPtBuf.dbport   = devPoint->port;            // SDP POINT :주장치 PORT 번호  [1..16]
        dPtBuf.dbpoint  = devPoint->point;           // SDP POINT :주장치 POINT 번호 [1..64] 
        
        for(hostid = 0; hostid < MAX_HOST; hostid++)
        {
            dPtBuf.hostIndex[hostid] = (devPoint->hostIndex[hostid][0]*256) + devPoint->hostIndex[hostid][1];     // SDP POINT : 상위 호스트 Index 번호 [1..1024]
        }

        /* 포인트/ON/OFF 이름 정보 */
        memcpy( (byte *) dPtBuf.ptNameStr, (byte *) devPoint->ptNameStr, 40);
        memcpy( (byte *) dPtBuf.onStr, (byte *) devPoint->onStr, 10);
        memcpy( (byte *) dPtBuf.offStr, (byte *) devPoint->offStr, 10);

        /* 이 것들은 내려오는것이 아니군..*/
        /* ICCP Mapping 포인트 Index... */
        dPtBuf.dbPtIndex = i;                   // 데이터베이스 상의 포인트 DB Index ...[0...4095]     
        
        dPtBuf.debounce = dPtBuf.pointMax & 0xff;
        dPtBuf.cntrTime = dPtBuf.pointMax;
        
        if(dPtBuf.debounce <= 5)   		dPtBuf.debounce = 5;   
        if(dPtBuf.cntrTime <= 500)   	dPtBuf.cntrTime = 500;   
        
        /* ------------------------------------------------ */
        /* 계전기/장치 TYPE에 따른 ... 포인트 초기화        */
        /* rtudb에 DB_POINT를 SDP POINT로 옮기기 위해 local dPtBuf 를 이용해서 위해서 복사하고 */
        /* 여기서 SDP 에 넣는다 */
        /* ------------------------------------------------ */
        
         /* devPoint는 rtuDB 상의 포인트 구조체 */
        if(devPoint->devType == DI_POINT)           sdp_diPoint_initial(devNo, devPt, &dPtBuf);
        else if(devPoint->devType == DO_POINT)      sdp_doPoint_initial(devNo, devPt, &dPtBuf);
        else if(devPoint->devType == AI_POINT)      sdp_aiPoint_initial(devNo, devPt, &dPtBuf);
        else if(devPoint->devType == VDI_POINT)     sdp_vdiPoint_initial(devNo, devPt, &dPtBuf);            // 연산포인트 : 상태
        else if(devPoint->devType == VAI_POINT)     sdp_vaiPoint_initial(devNo, devPt, &dPtBuf);            // 연산포인트 : 계측    
// devPoint 와 vritPoint는 다른 거군..        
        else if(devPoint->devType == DEV_POINT)     sdp_devPoint_initial(devNo, devPt, &dPtBuf);            // 장치 DEVICE 
        else                                        sdp_virPoint_initial(devNo, devPt, &dPtBuf);            // 장치 DEVICE 
        
        /* ------------------------------------------------ */
        /* ICCP-INFO 포인트 초기화                          */
        /* ------------------------------------------------ */
        update_iccpInfo(devNo, devPt, &dPtBuf);
        
        opr->max_sdpPoint++;    
    }
        
    Debug(console,"wdt> => POINT Config End...(CU Point = %2d)!\n", opr->max_sdpPoint);
    printf(" [*] ICCP-SDI = %3d   [*] ICCP-SAI = %3d    [*] ICCP-SDO = %3d \n", iccpInfo->maxIndex_sdi, iccpInfo->maxIndex_sai, iccpInfo->maxIndex_sdo);
    printf(" [*] ICCP-DDI = %3d   [*] ICCP-DAI = %3d  \n", iccpInfo->maxIndex_ddi, iccpInfo->maxIndex_dai);
    printf(" [*] ICCP-QDI = %3d   [*] ICCP-QAI = %3d  \n", iccpInfo->maxIndex_qdi, iccpInfo->maxIndex_qai);
    printf(" [*] ICCP-TDI = %3d   [*] ICCP-TAI = %3d  \n", iccpInfo->maxIndex_tdi, iccpInfo->maxIndex_tai);
    printf(" [*] ICCP-DEV = %3d \n", iccpInfo->maxIndex_dev);
     
}

    
/*----------------------------------------------------------------------------
* Function Name : calPointConfig()
* 수행내용: 데이터베이스중 연산 포인트 정보를 DB에서 읽어 온다.
* ArgList :
*   1. ioid - 초기화 되고자 하는 모듈번호
*   2. buf  - 초기화 되는 데이터베이스상의 Pointer Address
* Return  :  
---------------------------------------------------------------------------- */   
void calPointConfig()
{
    int     i;
    
    DB_CAL_POINT    *dbCalPt;           // DB - 연산포인트
    CAL_POINT_BUF   *calPt;             // 연산포인트 구조체

    /* -------------------------------------------------------- */
    /*  데이터베이스 상의 포인트 DB 초기화...(max 4096)         */
    /* -------------------------------------------------------- */
    for(i = 0; i < MAX_CAL_POINT; i++)
    {
        /* DB 상의 포인트 구조체 ... */
        dbCalPt = (DB_CAL_POINT *) &rtudb->calPointBuf[i];
        calPt   = (CAL_POINT_BUF *) calPtBuf[i];

        
        calPt->pointType = dbCalPt->pointType;          // SDP : 연산포인트 TYPE,     [0]NULL, [1] STATUS, [2]ANALOG, [3]CONTROL
        calPt->useFlag   = dbCalPt->useFlag;            // SDP : 연산포인트 사용유무, [0]사용않함, [1] 사용
        calPt->calcTime  = dbCalPt->calcTime;           // SDP : 연산포인트 연산주기, sec
        calPt->function  = dbCalPt->function;           // SDP : 연산포인트 Function#
        
        memcpy( (byte *) calPt->calString, dbCalPt->calString, 128);
        calPt->calString[127] = '\0';
        
        calPt->config = RESET;
        calPt->devNo  = RESET;
        calPt->devPt  = RESET;
        //calPt->devType= RESET;
        //calPt->localIndex= RESET;

#if 0
        // TEST...?????
        calPt->useFlag   = SET;
        calPt->config    = SET; 
        calPt->pointType = 1;
        calPt->calcTime  = 1;
        calPt->devNo     = 1;
        calPt->devPt     = i+1;
                  
        sprintf(calPt->calString, "%s", "([M01:S001] && [M02:S001])");
#endif
        
        calPt->status= RESET;
        calPt->floatData = 0;
    }
        
    Debug(console,"wdt> => CAL-POINT Config End...!\n");
    
}


/**************************************************************** 
*   FUNCTION : hostParaConfig() 
*   - DATABASE parameter initial 
****************************************************************/ 
void hostParaConfig() 
{ 
    int     i;
    int     hostid;
    
    DB_HOST_CONFIG		*dbHost;
    DB_HNET_ENTRY	    *dbHnet;
    
    HOST_DCB            *host;
    HOST_NET_ENTRY      *hostNet;

    /* ---------------------------------------- */
    /*  HOST 통신관련 파라메터 ...              */
    /* ---------------------------------------- */
    for(hostid = 0; hostid < MAX_HOST; hostid++)
    {
    	dbHost = (DB_HOST_CONFIG *) &rtudb->hostCfg[hostid];
        host = (HOST_DCB *) hostDCB[hostid];                 /* 주장치 #1 속성정의 */
        
        host->hostDualMode = dbHost->runMode;               // HOST Config : HOST 운영모드 : [0]사용안함, [1]개별, [2]이중화 
        host->hostProtocol = dbHost->protocol;              // HOST Config : HOST 통신 프로토콜 : HARRIS/LANDIS/DNP/MODBUS/IEC...
        host->hostComType  = dbHost->comMode;               // HOST Config : HOST 통신모드 : RS232/MODEM/RS485/TCPIP
        host->hostComSpeed = dbHost->comSpeed;              // HOST Config : HOST 통신속도 
        
        host->masterChan = dbHost->masterPort;              // HOST Config : HOST MASTER 통신포트
        host->slaveChan  = dbHost->slavePort;               // HOST Config : HOST SLAVE  통신포트
        
        /* VMEBUS-SIO Channel Index */
        host->vmeMstChan = host->masterChan % 8;            // SIO0 ~ SIO7
        host->vmeSlvChan = host->slaveChan % 8;             // SIO0 ~ SIO7
        
        host->soeClass   = dbHost->soeClass;                // HOST Config : SOE Class 지정 
        host->cosClass   = dbHost->cosClass;                // HOST Config : COS Class 지정 
        host->coaClass   = dbHost->coaClass;                // HOST Config : COA Class 지정 
        host->unsolMode  = dbHost->unsolite;      			/* UNsolite Event : 상위에서 내려옴. */
        //host->unsolEvent    = 0;
        
        host->comDelay   = dbHost->comDelay;               // HOST Config : HOST 통신 지연 (10ms)
        host->offCount   = dbHost->offCount;               // HOST Config : HOST 통신 Offline Count
        
        /* 2020.06.03 HOST별 TIME-SYNC 허용금지 */
        host->timeSyncDISB = dbHost->timeSyncDISB;            // HOST Config : HOST 통신 function
        if(host->timeSyncDISB > 0)	host->timeSyncDISB = SET;
        	
        host->chgMode    = dbHost->chgMode;                // HOST Config : HOST Change Mode;
        
        host->reserved1  = dbHost->reserved1;			// HOST TIME-SYNC 허용/금지
        host->reserved2  = dbHost->reserved2;
        
        host->hostid     = (dbHost->hostAddr[0]*256) + dbHost->hostAddr[1];     // HOST Config : HOST DNP - 센터 Address
        host->rtuAddr    = (dbHost->rtuAddr[0]*256) + dbHost->rtuAddr[1];       // HOST Config : HOST DNP - RTU Address
        host->tcpPort    = (dbHost->tcpipPort[0]*256) + dbHost->tcpipPort[1];   // HOST Config : HOST DNP - TCPIP Address
        
        memcpy((byte *)&host->hostNameStr[0], (byte *) &dbHost->hostNameStr[0], 20);
        
        /* ---------------------------- */
        /*  HOST - 주장치 Network 정보  */
        /* ---------------------------- */
        for(i=0;i < 2; i++)
        {
            dbHnet  = (DB_HNET_ENTRY *) &dbHost->masterNetCfg[i];
            hostNet = (HOST_NET_ENTRY *) &host->masterNetCfg[i];
            
            hostNet->netPort = dbHnet->netPort;                                         // Network# 사용 Port, [0] NET1 ~[7] NET8
            memcpy((byte *)&hostNet->ipAddr[0], (byte *) &dbHnet->ipAddr[0], 16);       // Network# IP-Address
        }
        
        /* ---------------------------- */
        /*  HOST - 예비장치 Network 정보*/
        /* ---------------------------- */
        for(i=0;i < 2; i++)
        {
            dbHnet  = (DB_HNET_ENTRY *) &dbHost->slaveNetCfg[i];
            hostNet = (HOST_NET_ENTRY *) &host->slaveNetCfg[i];
            
            hostNet->netPort = dbHnet->netPort;                                         // Network# 사용 Port, [0] NET1 ~[7] NET8
            memcpy((byte *)&hostNet->ipAddr[0], (byte *) &dbHnet->ipAddr[0], 16);       // Network# IP-Address
        }
        
        host->runStatus = 0;    // 0: 사용않함.
    }
    
    Debug(console,"wdt> => HOST Parameter init...!\n");
    
}


/**************************************************************** 
*   FUNCTION : iccpParaConfig() 
*   - ICCP HOST  parameter initial 
****************************************************************/ 
void iccpParaConfig() 
{ 
   // int     i;
    //int     devNo, devPt;
    //short   iccpType, iccpIndex;
    //short iccpRes1, iccpRes2;
    
    //DB_POINT_BUF    *devPoint;
    
    ICCP_CONFIG     *iccpCfg;
    DB_ICCP_CONFIG  *dbCfg;
    
    //ICCP_POINT_INFO *iccpPoint;
    //ICCP_POINT_DEF	*iccpDef;		// ICCP-HOST 운영 포인트 참조
    
    /* ---------------------------------------- */
    /*  ICCP-CONFIG 초기화 ...              */
    /* ---------------------------------------- */
    dbCfg   = (DB_ICCP_CONFIG *) &rtudb->iccpConfig;  
    iccpCfg = (ICCP_CONFIG *) &iccpDCB->config;  
    
    memcpy((byte *) iccpCfg, (byte *)dbCfg, sizeof(ICCP_CONFIG));

    //printf(" AP_Title[96]= %s ...  %s \n", rtudb->iccpConfig.FEP_A.AP_Title, iccpCfg->FEP_A.AP_Title); 
    //printf(" AP_Title[96]= %s ...  %s \n", rtudb->iccpConfig.FEP_B.AP_Title, iccpCfg->FEP_B.AP_Title); 
    //printf(" AP_Title[96]= %s ...  %s \n", rtudb->iccpConfig.SDP_A.AP_Title, iccpCfg->SDP_A.AP_Title); 
    //printf(" AP_Title[96]= %s ...  %s \n", rtudb->iccpConfig.SDP_B.AP_Title, iccpCfg->SDP_B.AP_Title); 

    Debug(console,"wdt> => ICCP-HOST Parameter init... %d   %d !\n", sizeof(ICCP_CONFIG), sizeof(DB_ICCP_CONFIG));
    
}


/**************************************************************** 
*   FUNCTION : scanParaConfig() 
*   - DATABASE parameter initial 
****************************************************************/ 
void scanParaConfig() 
{ 
    int     index;
    int     scanid;
    
    DB_SCAN_CONFIG		*dbScan;
    SCAN_CONFIG         *scan;

    for(scanid=0; scanid < MAX_SCAN_PORT; scanid++)
    {
        dbScan = (DB_SCAN_CONFIG *) &rtudb->scanConfig[scanid];
        scan   = (SCAN_CONFIG *) scanCFG[scanid];                
        
        scan->useFlag   = dbScan->useFlag;              // SDP SCAN : Channel 사용유무
        scan->targetID  = dbScan->targetID;             // ESIO Target ID, 0:사용않함, 1: SIO, 2:ESIO1, 3:ESIO2, 4:ESIO3, 5:ESIO4, 6:RTU, 7:MPU1, 8:MPU2
        
        scan->protocol  = dbScan->protocol;             // SDP SCAN : SCAN 통신 프로토콜 
        scan->comMode   = dbScan->comMode;              // SDP SCAN : SCAN 통신모드
        scan->comPort   = dbScan->comPort;			    // SDP SCAN : SCAN 통신포트
        scan->comSpeed  = dbScan->comSpeed;             // SDP SCAN : SCAN 통신속도

        scan->comDelay  = dbScan->comDelay;			    // SDP SCAN : SCAN 통신간격
        scan->offCount  = dbScan->offCount;			    // SDP SCAN : SCAN Offline Count
        scan->chgMode   = dbScan->chgMode;              // SDP SCAN : SCAN Change Mode
        
        scan->function1 = dbScan->function1;            // SDP SCAN : SCAN Function#1
        scan->function2 = dbScan->function2;            // SDP SCAN : SCAN Function#2
        scan->function3 = dbScan->function3;            // SDP SCAN : SCAN Function#3
        
        memcpy((byte *)&scan->scanNameStr[0], (byte *) &dbScan->scanNameStr[0], 20);       // SDP SCAN : SCAN Name String
        
        /* SCAN : 할당된 계전기 초기화 ... */    	
        scan->scanIndex  = 0;
        for(index=0; index< MAX_DEVICE; index++)    scan->scanDevice[index] = 0;
        
    }

    Debug(console,"wdt> => SCAN Parameter init...!\n");
    
    
}


/**************************************************************** 
*   FUNCTION : deviceParaConfig() 
*   - DATABASE parameter initial 
****************************************************************/ 
void deviceParaConfig() 
{ 
    int     ioid, scanid, esioid;

    DB_SDP_DEVICE	*dbMOD;
    SDP_DEVICE      *dev;
    SCAN_CONFIG     *scan;
    ESIO_CONFIG     *esio;

    for(ioid=0; ioid < MAX_DEVICE; ioid++)
    {
        /* -------------------------------------------- */
        /* 계전기 데이터베이스 초기화                   */
        /* -------------------------------------------- */
        dbMOD = (DB_SDP_DEVICE *) &rtudb->deviceConfig[ioid];
        dev   = (SDP_DEVICE *) deviceCFG[ioid];                
        
        memcpy((byte *)&dev->devNameStr[0], (byte *) &dbMOD->devNameStr[0], 40);       // SDP DEVICE : 계전기 Name String

        dev->scan         = dbMOD->useFlag;             // SDP DEVICE : 계전기 사용유무
        dev->comDevID     = dbMOD->comDevID;            // SDP DEVICE : 기능모듈내 계전기 ID 
        dev->comDevIndex  = dbMOD->comDevIndex;         // SDP DEVICE : 기능모듈내 계전기 Index    
        dev->scanPort     = dbMOD->scanPort;            // SDP DEVICE : 계전기 통신 포트
        dev->type         = dbMOD->type;           	    // SDP DEVICE : 계전기 TYPE
		dev->dualENB      = dbMOD->dualENB;			    // SDP DEVICE : 계전기 - 계전기 이중화 여부
		dev->modbusFileNo = dbMOD->modbusFileNo;		// SDP DEVICE : 계전기 - MODBUS 프로파일
		
		dev->devDiPoint = (dbMOD->di_ptnum[0]*256) + dbMOD->di_ptnum[1];       // SDP DEVICE : 장치별 DI 포인트 수 
		dev->devDoPoint = (dbMOD->do_ptnum[0]*256) + dbMOD->do_ptnum[1];       // SDP DEVICE : 장치별 DO 포인트 수 
		dev->devAiPoint = (dbMOD->ai_ptnum[0]*256) + dbMOD->ai_ptnum[1];       // SDP DEVICE : 장치별 AI 포인트 수 
		dev->devAoPoint = (dbMOD->ao_ptnum[0]*256) + dbMOD->ao_ptnum[1];       // SDP DEVICE : 장치별 AO 포인트 수 
		dev->devCntPoint= (dbMOD->cnt_ptnum[0]*256)+ dbMOD->cnt_ptnum[1];     // SDP DEVICE : 장치별 Count 포인트 수 
		
		dev->netPort= (dbMOD->netPort[0]*256) + dbMOD->netPort[1];      // SDP DEVICE : HOST TCPIP Port번호 */
		dev->function1 = dbMOD->function1;		                        // SDP DEVICE : SCAN Function#1
		
		/*  계전기 Network 정보 */
        memcpy((byte *)&dev->ipString1[0], (byte *) &dbMOD->ipString1[0], 16);       // SDP DEVICE : 계전기 IP-Address
        memcpy((byte *)&dev->ipString2[0], (byte *) &dbMOD->ipString2[0], 16);       // SDP DEVICE : 계전기 IP-Address

        /* -------------------------------------------- */
        /* 계전기 운영정보 초기화                       */
        /* -------------------------------------------- */
        dev->ioid   = ioid + 1 ;           			            /* Target IO Address */
        dev->hostid = 50;               			            /* CU Address */      
        
        if(dev->scan == 0)  dev->runStatus  = 0;            // 계전기 통신상태(사용않함) - 0
        else                dev->runStatus  = 2;            // 계전기 통신상태(이상) - 2      
        
        /* -------------------------------------------- */
        /* ESIO 별 계전기 할당 정보 초기화              */
        /* -------------------------------------------- */
        if((dev->scanPort > 0) && (dev->scanPort <= MAX_SCAN_PORT))
        {   
            scanid = dev->scanPort - 1;   
            scan = (SCAN_CONFIG *) scanCFG[scanid];
            
            scan->scanDevice[scan->scanIndex] = ioid + 1;
            scan->scanIndex = (scan->scanIndex + 1) & 0x3f;
            
            dev->targetID = scan->targetID;     // ESIO Target ID, 0:사용않함, 1: SIO, 2:ESIO1, 3:ESIO2, 4:ESIO3, 5:ESIO4, 6:RTU, 7:MPU1, 8:MPU2
			
			/* 대상 ESIO ID 지정 ... Default = 1 */
			if(dev->targetID == 0)	dev->targetID = 1;

            /* 계전기 정보 추가 */
            esioid = scan->targetID - 1;        // 0: 사용않함, 1: MPU, 2: ESIO1, 3: ESIO2, 4: ESI3, 5: ESIO4, 6: SIO (RTU)
            esio = (ESIO_CONFIG *) esioCFG[esioid];
            
            /* ESIO 계전기 리스트에 추가 */
            esio->scanDevice[esio->scanIndex] = ioid + 1;
            esio->scanIndex = (esio->scanIndex + 1) & 0x3f;
            esio->scanMaxNum = esio->scanIndex;       
                
        }        
        
        /* -------------------------------------------- */
		/* 	장치별 최대 포인트 제한 					*/
		/* -------------------------------------------- */
		if(dev->devDiPoint >= MAX_DEV_DI_POINT)		dev->devDiPoint = MAX_DEV_DI_POINT;		// 장치당 최대 DI 포인트 
		if(dev->devDoPoint >= MAX_DEV_DO_POINT)		dev->devDoPoint = MAX_DEV_DO_POINT;		// 장치당 최대 DO 포인트 
		if(dev->devAiPoint >= MAX_DEV_AI_POINT)		dev->devAiPoint = MAX_DEV_AI_POINT;		// 장치당 최대 AI 포인트 
		if(dev->devCntPoint >= MAX_DEV_COUNT_POINT)	dev->devCntPoint = MAX_DEV_COUNT_POINT;	// 장치당 최대 Binary Counter 포인트 
    }
    
    Debug(console,"wdt> => DEVICE Parameter init...!\n");
    
}



/****************************************************************
*   FUNCTION : sioInitial()
*   - SIO struct initial
****************************************************************/
void rtuCmdInitial()
{

#if 1    
    int rtuid;
    
    RTU    	 *rtu;
    
    /* ---------------------------------------- */
    /* HOST - HARRIS 프로토콜 command_size      */
    /* ---------------------------------------- */   
   	for(rtuid = 0; rtuid < MAX_HARRIS_RTU; rtuid++)
    {
   	    rtu = (RTU *) rtubuf[rtuid];

        bzero8248( (byte *) &rtu->cmdSize[0], 32);
	        
   	    rtu->cmdSize[0]  = 3;    /* Data Dump */
        rtu->cmdSize[3]  = 3;    /* Status Change Check */
   	    rtu->cmdSize[4]  = 4;    /* Status Change Dump */
       	rtu->cmdSize[5]  = 3;    /* Status Dump */
        rtu->cmdSize[6]  = 5;    /* control Point Arm */
   	    rtu->cmdSize[7]  = 5;    /* control Point Operate */
       	rtu->cmdSize[11] = 3;    /* Power Fail Reset */
        rtu->cmdSize[12] = 3;    /* Port Status Scan */
   	    rtu->cmdSize[17] = 8;    /* Time Syncronization */
       	rtu->cmdSize[18] = 4;    /* SOE Change dump */
        rtu->cmdSize[19] = 5;    /* Time Sync. Adjustment */
   	    rtu->cmdSize[30] = 3;    /* Locator Read */
    }
    
    Debug(console,"wdt> => RTU Command init...!\n");
#endif
    
}

/*----------------------------------------------------------------------------
* Function Name : harrisPortConfig()
* 수행내용: 데이터베이스중 포트 구성에 대한 초기화 함수
* ArgList :
*   1. buf  - 초기화 되는 데이터베이스상의 Pointer Address
* Return  :  
---------------------------------------------------------------------------- */    
void harrisPortConfig( int hostid, byte *buf)
{
#if 1      
    short   i;
    short   port = 0;
    short   seqno = 0;
    RTU     *rtu;
    static  byte PORT_TYPE[8] = {0,CAI,CAI,ANA,ANA,ACC};
	HOST_DCB *host;

  
	host = (HOST_DCB *) hostDCB[hostid];		/* HARRIS 주장치 속성정의 */
	
    /* -------------------------------- */
    /*  clear rtu buffer                */
    /* -------------------------------- */
    for(i = 0; i < MAX_HARRIS_RTU; i++)
    {
        rtu = (RTU *) rtubuf[i];
        rtu->type  = 0;
        rtu->id    = 0;
        rtu->dataPortCnt = 0;
        rtu->ciPortCnt   = 0;
    }

    /* -------------------------------- */
	/* HOST 별 RTU 구조체 정의          */
	/* -------------------------------- */
    rtu = (RTU *) rtubuf[0];
    rtu->id = buf[0];

    for(i = 0; i < MAX_HARRIS_PORT; i++)
    {
        /* RTU id */
        if(buf[i * 2] == 0)
        {
            /* portdb[i].rtuid = 255;*/
            portdb[i]->rtuid = 0;
            continue;
        }
        
        if(buf[i * 2] != rtu->id)
        {
            rtu->max_port = rtu->dataPortCnt + rtu->ciPortCnt;
            rtu->cmdSize[0] = 3 + rtu->dataPortCnt;
            rtu->cmdSize[5] = 3 + rtu->ciPortCnt;

            //printf("wdt> #1 rtu id = %d, ciPort=%d, dataPort=%d\n", rtu->id, rtu->ciPortCnt, rtu->dataPortCnt);
            rtu = (RTU *) rtubuf[++seqno];
            rtu->id = buf[i * 2];
            port = 0;
        }

        portdb[i]->rtuid      = seqno + 1;   	 /* write rtuid       */
        //host->portdb[i].rtuid      = buf[i * 2];   	 /* write rtuid       */
        portdb[i]->portStatus = PF;             /* power fail */
        portdb[i]->harrisPort = port++;         /* write harris port */
        portdb[i]->type       = PORT_TYPE[buf[i * 2 + 1]];

        if(portdb[i]->type == CAI)                                       	rtu->ciPortCnt++;
        else if((portdb[i]->type == ANA) || (portdb[i]->type == ACC))     	rtu->dataPortCnt++;

    } /* for(i) */

    rtu->max_port   = rtu->dataPortCnt + rtu->ciPortCnt;
    rtu->cmdSize[0] = 3 + rtu->dataPortCnt;
    rtu->cmdSize[5] = 3 + rtu->ciPortCnt;


#endif
    

} /* config */

// hkkim from linkMain
enum {
    SYSRUN_ERR_SYSTEM = -1000,
    SYSRUN_ERR_UNKNOWN = -1001
};


void  system_simple(const char *cmd)
{
    int ret = system(cmd);
    if (ret == -1) {
        printf("running system(%s) failed\r\n",cmd);
        //return SYSRUN_ERR_SYSTEM;   // system() 자체 실패
        return  ; 
    }

    if (WIFEXITED(ret)) {
        if (  WEXITSTATUS(ret) ) 
                printf( "running system(%s) exited with %d\r\n",cmd,WEXITSTATUS(ret)  );    
        return  ; 
   }

    if (WIFSIGNALED(ret)) {
        printf("running system(%s) failed with signal\r\n",cmd);
        //return -WTERMSIG(ret);      // -signal
        return  ; 
    }
        printf("running system(%s) failed with unkown reason\r\n",cmd);
    return SYSRUN_ERR_UNKNOWN;      // 드문 케이스 (정지/재개 등)
}
int check_interface_exists(const char *ifname) {
    char path[256];
    snprintf(path, sizeof(path), "/sys/class/net/%s", ifname);
    
    // 해당 디렉토리가 존재하면 인터페이스가 있는 것
    if (access(path, F_OK) == 0) {
        return 0;  // 존재함
    }
    return -1;  // 존재하지 않음
}


void down_eth(char *if_name)
{
       char buffer[100];
//            if ( ! check_interface_exists("eth0") )       
        {      
        snprintf(buffer, 100, "ifconfig  %s down\n", if_name);
   	    printf("WDT> %s\r\n",buffer);
   	    system_simple(buffer);
   	    pause(100);  
    	}
    
}

/*
*   NETWORK-Config 
*/
static void mpuNetwork_Initial()
{
    char    buffer[256];
    int     gw_flag  ;
    
    MPU_NET_ENTRY   *mpuNet1, *mpuNet2;
    MPU_NET_ENTRY   *mpuNet3, *mpuNet4;
    //MPU_NET_ENTRY   *mpuNet3;
    
    
    gw_flag=0;
    /* -------------------------------------------- */
    /*  MPU 실장모드에 따라서 각 Network IP 설정    */
    /* -------------------------------------------- */
    if(opr->cpuMode == MPU_A)
    {
        mpuNet1 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[0];
        mpuNet2 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[1];
        //mpuNet3 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[2];
        // hkkim
        mpuNet3 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[2];
        mpuNet4 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[3];
    
    }        
    else
    {
        mpuNet1 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[0];
        mpuNet2 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[1];
        //mpuNet3 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[2];
        // hkkim
        mpuNet3 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[2];
        mpuNet4 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[3];
    }
        

//    printf("[DB] Network#1 : IP=%s, Mask=%s \n", mpuNet1->ipAddr, mpuNet1->subMask);
//    printf("[DB] Network#2 : IP=%s, Mask=%s \n", mpuNet2->ipAddr, mpuNet2->subMask);
//    printf("[DB] Network#3 : IP=%s, Mask=%s \n", mpuNet3->ipAddr, mpuNet3->subMask);    
//    printf("[DB] Network#4 : IP=%s, Mask=%s \n", mpuNet4->ipAddr, mpuNet4->subMask);

    // 2026-07-15 오후 5:18:13 error가 나도 무시하면 되지.
             /* remove existing default gw */
             sprintf(buffer, "route del  default 2>  /dev/null");
   	         system_simple(buffer);
   	         pause(100); 


#ifdef  ROM_VERSION


    if(mpuNet1->useFlag == SET)      
    {       
        /* MASTER-CPU : eth0 Network...   */      
        
        if ( ! check_interface_exists("eth0") )       
        {      
            //    down_eth("eth0");
            sprintf(buffer, "ifconfig eth0 %s netmask %s up", mpuNet1->ipAddr, mpuNet1->subMask);
            printf("[CFG] Network#0 : %s\r\n", buffer);
    //      	system(buffer); 
       	    system_simple(buffer);
       	    pause(100);  
        	
        	/* 2026-07-15 오후 4:47:49  Default GW */
        	if ( mpuNet1->gwAddr[0] && ! gw_flag )
        	{
                 sprintf(buffer, "route add default gw %s dev eth0", mpuNet1->gwAddr);
                 printf("[CFG] Default GW : %s\r\n", buffer);
    //      	 system(buffer); 
       	         system_simple(buffer);
                 gw_flag =1 ;
       	         pause(100);     	        	    
        	}    	        	    	    	   	
       	}
       	else printf("[CFG] Network#0 not ready\r\n");   
    }
    else
    {
            /* MASTER-CPU : eth0 Network...   */  
            if ( ! check_interface_exists("eth0") )   
            {          

                sprintf(buffer, "ifconfig eth0 down\n");
                printf("[NO USE] Network#0 : %s", buffer);
       	        system_simple(buffer);
       	        pause(100);  
       	           	           	        
            }
        	else printf("[NO USE] Network#0 not ready\r\n");        
    } 
     	    
#endif
        
    /* MASTER-CPU : eth1 Network...   */ 
    if(mpuNet2->useFlag == SET)
    {            
        if ( ! check_interface_exists("eth1") ) 
        {
         //   down_eth("eth1");

            sprintf(buffer, "ifconfig eth1 %s netmask %s up", mpuNet2->ipAddr, mpuNet2->subMask);
            printf("[CFG] Network#1 : %s\r\n", buffer);
//      	system(buffer);  
   	        system_simple(buffer);
   	        pause(100);  
   	        
        	/* 2026-07-15 오후 4:47:49  Default GW */
        	if ( mpuNet2->gwAddr[0] && ! gw_flag )
        	{
                 sprintf(buffer, "route add default gw %s dev eth1", mpuNet2->gwAddr);
                 printf("[CFG] Default GW : %s\r\n", buffer);
    //      	 system(buffer); 
       	         system_simple(buffer);
                 gw_flag =1 ;
       	         pause(100);     	        	    
        	}   	           	           	        	        
   	    }
    	else printf("[CFG] Network#1 not ready\r\n");   	    
   	    
    }
    else  
    {
        /* MASTER-CPU : eth0 Network...   */            
         if ( ! check_interface_exists("eth1") ) 
        {
        sprintf(buffer, "ifconfig eth1 down\n");
        printf("[NO USE] Network#1 : %s", buffer);

   	    system_simple(buffer); 
   	    pause(100);  

        }
    	else printf("[NO USE] Network#1 not ready\r\n");  
                
    }
    
    /* MASTER-CPU : eth1 Network...   */ 
    if(mpuNet3->useFlag == SET)
    {            
        if ( ! check_interface_exists("eth2") ) 
        {
        //    down_eth("eth2");
        sprintf(buffer, "ifconfig eth2 %s netmask %s up", mpuNet3->ipAddr, mpuNet3->subMask);
        printf("[CFG] Network#2 : %s\r\n", buffer);
   	    system_simple(buffer);
   	    pause(100);  
   	    
    	/* 2026-07-15 오후 4:47:49  Default GW */
    	if ( mpuNet3->gwAddr[0] && ! gw_flag )
    	{
             sprintf(buffer, "route add default gw %s dev eth2", mpuNet3->gwAddr);
             printf("[CFG] Default GW : %s\r\n", buffer);
//      	 system(buffer); 
   	         system_simple(buffer);
             gw_flag =1 ;
   	         pause(100);     	        	    
    	}   	       	       	    
   	    }
    	else printf("[CFG] Network#2 not ready\r\n");      	    
    }
    else  
    {
        /* MASTER-CPU : eth0 Network...   */            
         if ( ! check_interface_exists("eth2") ) 
        {
           
        sprintf(buffer, "ifconfig eth2 down\n");
        printf("[NO USE] Network#2 : %s", buffer);

   	    system_simple(buffer);
   	    pause(100);  

        }
    	else printf("[NO USE] Network#2 not ready\r\n");  
                
    }
    
    /* MASTER-CPU : eth1 Network...   */ 
    if(mpuNet4->useFlag == SET)
    {            
         if ( ! check_interface_exists("eth3") ) 
        {
         //   down_eth("eth3");
        
        sprintf(buffer, "ifconfig eth3 %s netmask %s up", mpuNet4->ipAddr, mpuNet4->subMask);
        printf("[CFG] Network#4 : %s\r\n", buffer);
   	    system_simple(buffer); 
   	    pause(100);  
   	    
        	/* 2026-07-15 오후 4:47:49  Default GW */
        	if ( mpuNet4->gwAddr[0] && ! gw_flag )
        	{
                 sprintf(buffer, "route add default gw %s dev eth3", mpuNet4->gwAddr);
                 printf("[CFG] Default GW : %s\r\n", buffer);
    //      	 system(buffer); 
       	         system_simple(buffer);
                 gw_flag =1 ;
       	         pause(100);     	        	    
        	}   	       	       	    
        }
    	else printf("[CFG] Network#3 not ready\r\n");      	    
    }
    else  
    {
        /* MASTER-CPU : eth3 Network...   */            
         if ( ! check_interface_exists("eth3") ) 
        {
        
        sprintf(buffer, "ifconfig eth3 down\n");
        printf("[NO USE] Network#4 : %s", buffer);

   	    system_simple(buffer); 
   	    pause(100);  
             
        }
    	else printf("[NO USE] Network#3 not ready\r\n");               
             
                
    }    

    if(opr->cpuMode == MPU_A)
    {
        printf("\n------------------------------------------\n");
        printf("wdt> MPU-A ...Network Initial OK ...! \n");
        printf("------------------------------------------\n");
    }
    else
    {
        printf("\n------------------------------------------\n");
        printf("wdt> MPU-B ...Network Initial OK ...! \n");
        printf("------------------------------------------\n");
    }
    


}



/*
*   MPU 구성정보 Config 
*/
void mpuParaConfig()
{
    int i;
    DB_MPU_CONFIG		*dbMPU;
    DB_MNET_ENTRY	    *dbMnet;
    
    MPU_NET_ENTRY       *mpuNet;

    /* 데이터베이스 DB 포인터 초기화 */
    dbMPU = (DB_MPU_CONFIG *) &rtudb->mpuConfig;

    memcpy( (byte *) &mpuCFG->sdpNameStr[0], (byte *) &dbMPU->sdpNameStr[0], 20);   // MPU Parameter : 현장 이름
    
    /* ------------------------------------ */
    /* 주 MPU : Network 구성정보 초기화     */
    /* ------------------------------------ */
    //for(i=0; i < 3; i++)
    for(i=0; i <4 ; i++)
    {
        dbMnet = (DB_MNET_ENTRY *) &dbMPU->master_netCfg[i];
        mpuNet = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[i];
        
        mpuNet->useFlag = dbMnet->useFlag;
        memcpy( (byte *) &mpuNet->ipAddr[0], (byte *) &dbMnet->ipAddr[0], 16); 
        memcpy( (byte *) &mpuNet->gwAddr[0], (byte *) &dbMnet->gwAddr[0], 16); 
        memcpy( (byte *) &mpuNet->subMask[0], (byte *) &dbMnet->subMask[0], 16); 
     // hkkim ESIO 보낼때 깨저서 debug 하느라..   
     //   printf("MASTER[%d] IP : %s, GW : %s, Sub : %s \r\n",i, mpuNet->ipAddr,mpuNet->gwAddr,mpuNet->subMask);

    }
    
    /* ------------------------------------ */
    /* 부 MPU : Network 구성정보 초기화     */
    /* ------------------------------------ */
    for(i=0; i < 4; i++)
//    for(i=0; i < 3; i++)
    {
        dbMnet = (DB_MNET_ENTRY *) &dbMPU->slave_netCfg[i];
        mpuNet = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[i];
        
        mpuNet->useFlag = dbMnet->useFlag;
        memcpy( (byte *) &mpuNet->ipAddr[0], (byte *) &dbMnet->ipAddr[0], 16); 
        memcpy( (byte *) &mpuNet->gwAddr[0], (byte *) &dbMnet->gwAddr[0], 16); 
        memcpy( (byte *) &mpuNet->subMask[0], (byte *) &dbMnet->subMask[0], 16); 
       // printf("SLAVE[%d] IP : %s, GW : %s, Sub : %s \r\n",i, mpuNet->ipAddr,mpuNet->gwAddr,mpuNet->subMask);        
    }
    
    mpuCFG->dualMpu     = dbMPU->dualMpu;           // MPU Parameter : CPU 이중화 운영모드
    mpuCFG->dualModule  = dbMPU->dualModule;        // MPU Parameter : Module 이중화 운영모드 
    
    mpuCFG->scuUseFlag  = dbMPU->scuUseFlag;        // MPU Parameter : SCU 모듈 사용유무
    mpuCFG->mmiUseFlag  = dbMPU->mmiUseFlag;        // MPU Parameter : MMI 모듈 사용유, 표현은 ICCP 재기동 
    
    mpuCFG->statusDump  = dbMPU->statusDump;        // MPU Parameter : STATUS Dump 주기
    mpuCFG->analogDump  = dbMPU->analogDump;        // MPU Parameter : ANALOG Dump 주기
    mpuCFG->debounce    = dbMPU->debounce;          // MPU Parameter : Function 코드#1
    mpuCFG->dbCheck     = dbMPU->dbCheck;           // MPU Parameter : Function 코드#2
    mpuCFG->hostWDT     = dbMPU->hostWDT;           // MPU Parameter : Function 코드#3
    
    /* ICCP-HOST 통신연계 허용/금지 */
    mpuCFG->iccpEnbFlag = dbMPU->iccpFlag;          // (1) ICCP 허용, (0) ICCP 금지
    if(mpuCFG->iccpEnbFlag > 0)	mpuCFG->iccpEnbFlag = SET;
    else						mpuCFG->iccpEnbFlag = RESET;	
    
    opr->iccpEnbFlag = mpuCFG->iccpEnbFlag;
    
    mpuCFG->func1 = dbMPU->func1;                   // MPU Parameter : ICCP 절체시간 [Function 코드#1
    mpuCFG->func2 = dbMPU->func2;                   // MPU Parameter : Function 코드#2
    mpuCFG->func3 = dbMPU->func3;                   // MPU Parameter : Function 코드#3
    mpuCFG->func4 = dbMPU->func4;                   // MPU Parameter : Function 코드#4
    mpuCFG->func5 = dbMPU->func5;                   // MPU Parameter : Function 코드#5
    mpuCFG->func6 = dbMPU->func6;                   // MPU Parameter : Function 코드#6
    
    /* -------------------------------------------- */
    /*	2022.07.08 ICCP 통신이상에 따른 절체시간 지정 			*/
    /* -------------------------------------------- */
    if(mpuCFG->func1 == 0)	opr->iccpChkTime = 300;
    else					opr->iccpChkTime = mpuCFG->func1;
    if(opr->iccpChkTime < 30)	opr->iccpChkTime = 30;

    
    /* 하위 계전기 : 상태/아날로그 수집주기 */
    opr->cpuChgMode = mpuCFG->dualMpu;				// MPU Parameter : CPU 이중화 자동절체 
    opr->stsDumpPeriod = mpuCFG->statusDump;        // MPU Parameter : STATUS Dump 주기
    opr->anaDumpPeriod = mpuCFG->analogDump;        // MPU Parameter : ANALOG Dump 주기
    opr->calCalcFlag   = mpuCFG->dbCheck;			// MPU Parameter : 연산포인트 계산 허용주기
    opr->iccpResetENB  = mpuCFG->mmiUseFlag;		// MPU Parameter : ICCP 관제에서 SDP 재기동 혀용/금지
    
     
    if((opr->stsDumpPeriod <=0) || (opr->stsDumpPeriod > 300))  opr->stsDumpPeriod = 10;    // Default 10초  1초 아닌가 ???
    if((opr->anaDumpPeriod <=0) || (opr->anaDumpPeriod > 300))  opr->anaDumpPeriod = 10;    // Default 10초    
          
          
    Debug(console, "wdt> => MPU Parameter init...!\n");

    pause(100) ;

    mpuNetwork_Initial();
}


/*
*   ESIO 구성정보 Config 
*/
void esioParaConfig()
{
    int i, j;
    DB_ESIO_CONFIG		*dbESIO;
    DB_ENET_ENTRY	    *dbEnet;
    DB_PORT_ENTRY	    *dbEport;
    
    ESIO_CONFIG         *esioMod;
    ESIO_NET_ENTRY      *esioNet;
    ESIO_PORT_ENTRY     *esioPort;

    int   netCount ; 
    
#ifdef __ARM_ARCH__
    netCount = 4 ;
#else
    netCount =3 ;    

#endif 
    
    


    /* -------------------------------------------------------- */
    /*  ESIO# 구성정보 초기화 ...                               		*/
    /* -------------------------------------------------------- */
    for(i=0; i < MAX_ESIO; i++)
    {
        /* 데이터베이스 DB 포인터 초기화 */
        dbESIO = (DB_ESIO_CONFIG *) &rtudb->esioConfig[i];
        esioMod= (ESIO_CONFIG *) esioCFG[i];
        
        esioMod->useFlag    = dbESIO->useFlag;                // ESIO Parameter : ESIO 사용유무
        esioMod->targetID   = dbESIO->targetID;               // ESIO Parameter : ESIO Target Module-ID, 0:사용않함, 1: ESIO1, 2:ESIO2, 3:ESIO3, 4:ESIO4, 5:SIO
        esioMod->autoChgFlag= dbESIO->autoChgFlag;            // ESIO Parameter : 자동절체 Flag
        esioMod->comDelay   = dbESIO->comDelay;               // ESIO Parameter : MPU 통신 지연 (10ms)
        
        memcpy( (byte *) &esioMod->esioNameStr[0], (byte *) &dbESIO->esioNameStr[0], 20);   // ESIO 장치이름
        
        /* ------------------------------------ */
        /* 주 ESIO : Network 구성정보 초기화    */
        /* ------------------------------------ */
        // 2026-05-26 오후 7:34:39 hkkim
        for(j=0; j < netCount; j++)
        {
            // ROM-DB space
            dbEnet = (DB_ENET_ENTRY *) &dbESIO->mstNetConfig[j];
            // runtime DB
            esioNet= (ESIO_NET_ENTRY *)&esioMod->mstNetConfig[j];
        
            esioNet->useFlag = dbEnet->useFlag;
            memcpy( (byte *) &esioNet->ipAddr[0], (byte *) &dbEnet->ipAddr[0], 16); 
            memcpy( (byte *) &esioNet->gwAddr[0], (byte *) &dbEnet->gwAddr[0], 16); 
            memcpy( (byte *) &esioNet->subMask[0], (byte *) &dbEnet->subMask[0], 16); 
        }
    
        /* ------------------------------------ */
        /* 예비 ESIO : Network 구성정보 초기화  */
        /* ------------------------------------ */
        for(j=0; j < netCount; j++)
        {
            dbEnet = (DB_ENET_ENTRY *) &dbESIO->slvNetConfig[j];
            esioNet= (ESIO_NET_ENTRY *)&esioMod->slvNetConfig[j];
        
            esioNet->useFlag = dbEnet->useFlag;
            memcpy( (byte *) &esioNet->ipAddr[0], (byte *) &dbEnet->ipAddr[0], 16); 
            memcpy( (byte *) &esioNet->gwAddr[0], (byte *) &dbEnet->gwAddr[0], 16); 
            memcpy( (byte *) &esioNet->subMask[0], (byte *) &dbEnet->subMask[0], 16); 
        }
        
        /* ------------------------------------ */
        /* ESIO : PORT 구성정보 초기화          */
        /* ------------------------------------ */
        for(j=0; j < 8; j++)
        {
            dbEport = (DB_PORT_ENTRY *) &dbESIO->portConfig[j];
            esioPort= (ESIO_PORT_ENTRY *)&esioMod->portConfig[j];
        
            esioPort->useFlag   = dbEport->useFlag;             // PORT# 사용유무                                       
            esioPort->function  = dbEport->function;            // PORT# Function                                       
            esioPort->comMode   = dbEport->comMode;             // PORT# 통신모드, 0:RS232, 1:MODEM, 2:RS485, 3:TCPIP   
            esioPort->comSpeed  = dbEport->comSpeed;            // PORT# 통신속도, [0] 1200 ~ [7] 115200                
            
            memcpy( (byte *) &esioPort->portNameStr[0], (byte *) &dbEport->portNameStr[0], 20);     // PORT# 포트이름
        }  
        
        /* ------------------------------------ */
        /*  ESIO 할당 계전기 정보 초기화        */
        /* ------------------------------------ */
        esioMod->scanIndex  = 0;            // ESIO 계전기 Index
        esioMod->scanMaxNum = 0;            // ESIO 등록된 계전기 수
        for(j=0; j < MAX_DEVICE; j++)  esioMod->scanDevice[j] = 0;      // ESIO 계전기 리스트
        
        Debug(console,"wdt> => ESIO[%d] Parameter init...!\n", i+1); 
    }
    
}


#if 0
/*
 *  Generate chksum
 */
word acc_gensum(word initVal, byte *buf, int bfcnt)
{
    word   chksum = initVal;
    for(;bfcnt > 0; bfcnt--, buf++) chksum += *buf;
    return(chksum);
}


/*
*   ESIO 개별보드의 데이터베이스 Chksum 참조
*/
word calc_ESIODB_Chksum(int esioid)
{
    word    chksum = 0;
    byte    *bfptr;
    int     size;
    
    /* MPU Config 정보 */
    bfptr = (byte *) &rtudb->mpuConfig;
    size = sizeof(DB_MPU_CONFIG);
    chksum = acc_gensum(chksum, bfptr, size);
    
    /* 해당 ESIO# Config 정보 */
    bfptr = (byte *) &rtudb->esioConfig[esioid];
    size = sizeof(DB_ESIO_CONFIG);
    chksum = acc_gensum(chksum, bfptr, size);
    
    //printf("==> ESIO Config    Size = %d, chksum = %4x \n", size, chksum);
    
    /* MODBUS Profile 정보 */
    bfptr = (byte *) &rtudb->modbusProfile[0];
    size = sizeof(DB_MODBUS_PROFILE) * MAX_MODBUS_PROFILE;
    chksum = acc_gensum(chksum, bfptr, size);
    //printf("==> MODBUS Profile Size = %d, chksum = %4x \n", size, chksum);
    
    /* SCAN Config 정보 */
    bfptr = (byte *) &rtudb->scanConfig[0];
    size = sizeof(DB_SCAN_CONFIG) * MAX_SCAN_PORT;
    chksum = acc_gensum(chksum, bfptr, size);
    //printf("==> SCAN Config    Size = %d, chksum = %4x \n", size, chksum);
    
    /* 계전기 Config 정보 */
    bfptr = (byte *) &rtudb->deviceConfig[0];
    size = sizeof(DB_SDP_DEVICE) * MAX_DEVICE;
    chksum = acc_gensum(chksum, bfptr, size);
    //printf("==> DEVICE Config  Size = %d, chksum = %4x \n", size, chksum);
    
    /* 포인트 Config 정보 */
    bfptr = (byte *) &rtudb->pointBuf[0];
    size = sizeof(DB_POINT_BUF) * MAX_DBASE_POINT;
    chksum = acc_gensum(chksum, bfptr, size);
    //printf("==> POINT  Config  Size = %d, chksum = %4x \n", size, chksum);
    
    if(opr->mpuDebug)
    Debug(console, ">> ESIO-%02d : Calc... Database Chksum = %04x\n", esioid, chksum);
    
    /* ---------------------------------------- */
    /*  ESIO Check-sum ...                      */
    /* ---------------------------------------- */
    opr->esioChkSUM[esioid] = chksum;
    
    return (chksum);
}

#endif



/*----------------------------------------------------------------------------
* Function Name : readRTUdb()
* 수행내용: 데이터베이스내용의 Check 및 초기화 수행 함수
* ArgList :
* Return  :  
---------------------------------------------------------------------------- */    
int readRTUdb()
{
    word    chksum;
    word    romsum;

    if(ioctl(wdtid,PMU_WDT_CTR_CLR,NULL) < 0 )  Debug(console, "wdt> *** ioctl fail\n");
    /* --------------------------- */
    /*   Check chksum              */
    /* --------------------------- */
    chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
    chksum = chksum & 0xffff;
    romsum = rtudb->chksum[0] + rtudb->chksum[1]*256;

    printf("sizeof RTU_DATABASE = %d, chksum = %04x, romsum=%04x \n", sizeof(RTU_DATABASE), chksum, romsum);
    
    if(chksum != romsum)
    {
        printf("\n=============================\n");
        printf("[*] *** Chksum Error : %4x (%4x)\n", romsum ,chksum);
        printf("=============================\n");

        logEvent_MPU(shmPtr, ENT_ROMDB_FAIL, 0, 0, 0, opr->cpuMode, NULL); 

        //bzero8248(rtudb, sizeof(RTU_DATABASE));                
        
        romsum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
        rtudb->chksum[0] = romsum;
        rtudb->chksum[1] = romsum >> 8;

        dbFileWrite(opr, rtudb);
    }


    /* ---------------------------- */
    /* HARRIS Command Initialize    */
    /*   Read port_config           */
    /* ---------------------------- */
    rtuCmdInitial();
    pause(20);
    
    /* HOST#1 : HARRIS 적용 */
    harrisPortConfig(0, &rtudb->portdb[0]);         /* HOST #1 (HARRIS) Port Config */
    pause(20);

    /* ---------------------------- */
    /*   MPU 구성정보 Config        */
    /* ---------------------------- */
    mpuParaConfig();
    pause(20);
    
    /* ---------------------------- */
    /*   ESIO 구성정보 Config        */
    /* ---------------------------- */
    esioParaConfig();
    pause(20);
    
    /* ---------------------------- */
    /*   DNP-HOST Database Config   */
    /* ---------------------------- */
    hostParaConfig();
    pause(20);
    
    if(ioctl(wdtid,PMU_WDT_CTR_CLR,NULL) < 0 )  Debug(console, "wdt> *** ioctl fail\n");
        
    /* ---------------------------- */
    /*   ICCP-HOST Database Config   */
    /* ---------------------------- */
    iccpParaConfig();
    pause(20);

    /* ---------------------------- */
    /*   SCAN Database Config       */
    /* ---------------------------- */
    scanParaConfig();
    pause(20);
    
    /* ---------------------------- */
    /*   DEVICE Database Config       */
    /* ---------------------------- */
    deviceParaConfig();
    pause(20);
                
    /* ---------------------------- */
    /*   Read point_config          */
    /* ---------------------------- */
    calPointConfig();                       // 연산포인트 Config...
    pause(20);
                    
    /* ---------------------------- */
    /*   Read point_config          */
    /* ---------------------------- */
    pointParaConfig();                 
    pause(20);

    /* ---------------------------------------------------- */
    /*  ICCP-POINT Config... (ICCP- DB변경시... 재기동 확인 */
    /* ---------------------------------------------------- */
    iccpShmDcbInit (shmPtr);    

    if(ioctl(wdtid,PMU_WDT_CTR_CLR,NULL) < 0 )  Debug(console, "wdt> *** ioctl fail\n");
    
    printf("[*] RTU Database initial OK... ! \n");
    return (0);
}


/*
*	SDP DB 변경내역 참조
*/
int sdp_DBChange_Check()
{
	
    /* ------------------------------------------------ */
	/*  데이터베이스 변경시... 재구성 로드              */
	/* ------------------------------------------------ */
	if(opr->mpuCfgDown_OK == SET)
    {
        opr->mpuCfgDown_OK = RESET;
        mpuParaConfig();
    }		
    
    if(opr->esioCfgDown_OK == SET) // set by SIM
        
    {
        opr->esioCfgDown_OK = RESET;
        esioParaConfig();
        scanParaConfig();
        deviceParaConfig(); 
        
        /* ESIO-SCAN Process 재기동 ... */
        opr->esioRestart = SET;
        pause(100);
    }		  
    
    if(opr->hostCfgDown_OK == SET)
    {
        opr->hostCfgDown_OK = RESET;
        hostParaConfig();

        printf("hostParaConfig re-initial OK... ! \n");
        
        /* ESIO-SCAN Process 재기동 ... */
        //for(i=0; i < MAX_HOST; i++)     opr->hostRestart[i] = SET;
        //pause(100);
    }		    
    
    if(opr->devCfgDown_OK == SET)
    {
        opr->devCfgDown_OK = RESET;
        esioParaConfig();
        scanParaConfig();
        deviceParaConfig();
        
        /* ESIO-SCAN Process 재기동 ... */
        opr->esioRestart = SET;
        pause(100);
    }		
    
    if(opr->modbusCfgDown_OK == SET)
    {
        opr->modbusCfgDown_OK = RESET;
    }		
    
    if(opr->scanCfgDown_OK == SET)
    {
        opr->scanCfgDown_OK = RESET;
        esioParaConfig();
        scanParaConfig();
        deviceParaConfig();
        
        /* ESIO-SCAN Process 재기동 ... */
        opr->esioRestart = SET;
        pause(100);
    }		
    
    if(opr->pointCfgDown_OK == SET)
    {
        opr->pointCfgDown_OK = RESET;
        pointParaConfig();
        
        pause(100);
        
        /* ---------------------------------------------------- */
    	/*  ICCP-POINT Config... (ICCP- DB변경시... 재기동 확인 */
    	/* ---------------------------------------------------- */
    	iccpShmDcbInit (shmPtr);    
    }		
    
    if(opr->calPt_CfgDown_OK == SET)
    {
        opr->calPt_CfgDown_OK = RESET;
        calPointConfig();                       // 연산포인트 Config...
        pointParaConfig();
        pause(100);
    }
    
    return (0);
}

/* ............ end of " config.c " ....................................... */

