#include	"localLib.h"

extern  SHM_MEMORY	    *shmPtr;
extern  int             termExec;
extern  TASK_INFO	    *taskPtr;

extern  RTC             *rtc;
extern  OPR_MSG         *opr;
extern  RTU_DATABASE    *rtudb;                     // SDP 데이터베이스
extern  CONSOLE_INFO	*console;

extern  HISTORY_QUE     *hque;                      // CONSOLE 용 이벤트

extern  LINK_MSG        *linkCfg;                   // CPU 이중화 구조체
extern  SCU_MSG         *scuCfg;                    // 이중화 절체장치(SCU) 구조체

extern  ICCP_CONFIG     *iccpCFG;                   // ICCP-HOST 구조체
extern  MPU_CONFIG      *mpuCFG;                    // MPU Config

extern  MPU_SOE_QUEUE   *mpuSOE;                    // MPU SOE Buffer

extern  ESIO_CONFIG     *esioCFG[MAX_ESIO];         // ESIO 장치 Config

extern  HOST_DCB        *hostDCB[MAX_HOST];         // HOST 관련 구조체 : ICCP, DNP, HARRIS, LANDIS...
extern  POINT_BUF       *devPtBuf[MAX_DEV_POINT];   // SDP 포인트 Config 정보
extern  SCAN_CONFIG     *scanCFG[MAX_SCAN_PORT];    // 하위계전기 SCAN Config
extern  SDP_DEVICE      *deviceCFG[MAX_DEVICE];     // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)


// hkkim
extern ESIO_RTU_DATABASE  esio_rtudb ; 

/*----------------------------------------------------------------------------
* Function Name : deviceComUpdate()
* 수행내용: Device 장치별 통신상태정보 Update 함수
* ArgList :
* Return  :
---------------------------------------------------------------------------- */
void    deviceComUpdate()
{
    int     ioid;
    int     devPtIndex;
    //int     dnpPoint;
    int     anyOffline;

    SDP_DEVICE      *dev;
    POINT_BUF       *devPoint;
    //HOST_DCB        *host;
    MPU_SOEQ_ENTRY  devEvent;
    
    /* -------------------------------------------- */
    /* 전체 계전기 통신상태 Check                   */
    /* -------------------------------------------- */
    anyOffline = 0;
    devPtIndex = INDEX_ALL_DEVICE;                      // 전체 계전기 통신상태 포인트
    devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
    
    for(ioid = 0; ioid < MAX_DEVICE; ioid++)
    {
        dev      = (SDP_DEVICE *) deviceCFG[ioid];
        devPoint = (POINT_BUF *) devPtBuf[ioid+1];      // Device Point 정보 : 개별 계전기, 1,2 ~ 64
        
        /* -------------------------------------------- */
        /*  전체 계전기수는 112지만, 내부적으론 64...   */
        /*  - 추후 Device Point를 확장해야 함...        */
        /* -------------------------------------------- */
        if(ioid >= 64)  continue;

        if(dev->scan == 0)  
        {
            devPoint->status = 1;   // [1] : 통신이상, Offline
            continue;
        }
        
        if(dev->online == RESET)   
        {
            devPoint->status = 1;   // [1] : 통신이상, Offline
            anyOffline++;
        }
        else
        {
            devPoint->status = 0;   // [0] : 통신정상, Online   
        } 
        
        /* ------------------------------------ */
        /*  SDP에서 ICCP-POINT Update...        */
        /* ------------------------------------ */
        
        gettimeofday(&devPoint->updateTime, NULL);      
        update_ICCP_info(shmPtr, devPoint, (float) devPoint->status, devPoint->config);

    }           
    
    /* -------------------------------------------- */
    /* 전체 통신상태 DEVICE-POINT 정보 Update ...   */
    /* -------------------------------------------- */
    devPtIndex = INDEX_ALL_DEVICE;                      // 전체 계전기 통신상태 포인트
    devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
    
    if(anyOffline)
    {      
        if(opr->allDevOnline == SET)
        {
            if(opr->soeDebug)
                Debug(console,">> SOE : ALL Device Offline ... SET  offline count =%d\n", anyOffline);
            opr->allDevOnline = RESET;

            /* -------------------------------- */       
            /* DEVICE-SOE 생성                  */
            /* -------------------------------- */   
            devEvent.eventCode = ENT_DEVICE_SOE;
            devEvent.devNo   = devPtIndex + 1;          // Devie Point,,,1,2
            devEvent.pointNo = 0;
            devEvent.state   = 1;                       // 장치상태 : 이상
            devEvent.esioNo  = opr->cpuMode;            // 이벤트 발생주체 : [0] CPU-A, [1] CPU-B
            
            gettimeofday(&devEvent.updateTime, NULL);
            Create_Device_SOE(shmPtr, &devEvent);   // 통신이상
        }

        /* 전체계전기 통신상태 => 이상 */                    
        devPoint->status = 1;                       // 0 : 정상, 1: 이상
        
        /* ------------------------------------ */
        /*  SDP에서 ICCP-POINT Update...        */
        /* ------------------------------------ */
        gettimeofday(&devPoint->updateTime, NULL);
        update_ICCP_info(shmPtr, devPoint, (float) devPoint->status, devPoint->config);
    }
    else
    {
        if(opr->allDevOnline == RESET)
        {
            if(opr->soeDebug)
                    Debug(console,">> SOE : ALL Device Online ... offline count =%d\n", anyOffline);
            opr->allDevOnline = SET;
            
            /* -------------------------------- */       
            /* DEVICE-SOE 생성                  */
            /* -------------------------------- */   
            devEvent.eventCode = ENT_DEVICE_SOE;
            devEvent.devNo   = devPtIndex + 1;          // Devie Point,,,1,2
            devEvent.pointNo = 0;
            devEvent.state   = 0;                       // 장치상태 : 정상
            devEvent.esioNo  = opr->cpuMode;            // 이벤트 발생주체 : [0] CPU-A, [1] CPU-B
            
            gettimeofday(&devEvent.updateTime, NULL);
            Create_Device_SOE(shmPtr, &devEvent);   // 통신이상
            
        }
              
        /* 전체계전기 통신상태 => 정상 */                    
        devPoint->status = 0;                       // 0 : 정상, 1: 이상
        
        /* ------------------------------------ */
        /*  SDP에서 ICCP-POINT Update...        */
        /* ------------------------------------ */
        gettimeofday(&devPoint->updateTime, NULL);
        update_ICCP_info(shmPtr, devPoint, (float) devPoint->status, devPoint->config);
    }

}

/*
* Device Point : 수동/자동 상태, 0:자동, 1:수동
*/
void    check_AutoManual()
{
    int     devPtIndex;
    POINT_BUF   *devPoint;
        
    devPtIndex = INDEX_AUTO_MANUAL;                     // Device Point : 수동/자동 상태, 0:자동, 1:수동
    devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
    
    if(devPoint->config == 0)   return;
    
    if(scuCfg->online == SET)  
    {
        if(scuCfg->remoteMode == AUTO_MODE) 
        {
            /* 수동모드(1) => 자동모드(0) 변화 Check */
            devPoint->status = 0;       // 자동모드 : 0
            gettimeofday(&devPoint->updateTime, NULL);
        }
        else
        {
            /* 자동모드(0) => 수동모드(1) 변화 Check */
            devPoint->status = 1;       // 수동모드 : 0
            gettimeofday(&devPoint->updateTime, NULL);
        } 
    }
}

/*
* Device Point-70 : SDP 동작상태, 0:정상, 1:이상
*/
void    check_sdp_status()
{
    int     devPtIndex;
    
    POINT_BUF   *devPoint;
        
    devPtIndex = INDEX_SDP_STS;                         // Device Point-70 : SDP 동작상태, 0:정상, 1:이상
    devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
    
    if(devPoint->config == 0)   return;
    
    //printf("stsetm-check: sdp_status... %d\n", devPoint->status);
    
    /* ------------------------------------ */
    /*  SDP 정상/비정상 상태 정의... ????   */
    /* ------------------------------------ */
    devPoint->status = 0;                               // SDP 정상 : 0
    gettimeofday(&devPoint->updateTime, NULL);
    
}        


/*
* Device Point-71 : SDP 동작모드, 0:SDP-A 동작, 1:SDP-B 동작 
*/
void    check_sdp_runMode()
{
    int     devPtIndex;
    
    POINT_BUF   *devPoint;
        
    devPtIndex = INDEX_SDP_RUN_MODE;                    // Device Point-71 : SDP 동작모드, 0:SDP-A 동작, 1:SDP-B 동작  
    devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
    
    if(devPoint->config == 0)   return;
    
    /* ---------------------------------------- */
    /* 현재 Active 한 SDP 장치 지정...          */
    /* ---------------------------------------- */
    if(opr->cpuMode == MPU_A)       devPoint->status = 0;   // SDP-A 동작 : 0
    else if(opr->cpuMode == MPU_B)  devPoint->status = 1;   // SDP-B 동작 : 1 
    else    return;
                   
    gettimeofday(&devPoint->updateTime, NULL);

}        


/*
* Device Point-72 : SDP-A 동작상태, 0:정상, 1:이상  
* Device Point-73 : SDP-B 동작상태, 0:정상, 1:이상  
*/
void    check_sdp_runState()
{
    int     devPtIndex;
    
    POINT_BUF   *devPoint;

    if(opr->dualCpuSts == SET)  // CPU 이중화 여부 결정...
    {
        if(opr->cpuMode == MPU_A)
        {
            /* ---------------------------- */
            /* SDP-A 정상 Check             */
            /* ---------------------------- */
            devPtIndex = INDEX_SDP_RUN_A;                       // Device Point-72 : SDP-A 동작상태, 0:정상, 1:이상 
            devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보

            if(mpuCFG->mpuStatus & MPU_RUN_FAIL)    devPoint->status = 1;   // SDP-A 이상 : 1
            else                                    devPoint->status = 0;   // SDP-A 정상 : 0    
            gettimeofday(&devPoint->updateTime, NULL);    
            
            /* ---------------------------- */
            /* SDP-B 정상 Check             */
            /* ---------------------------- */
            devPtIndex = INDEX_SDP_RUN_B;                       // Device Point-73 : SDP-B 동작상태, 0:정상, 1:이상  
            devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
            
            if(mpuCFG->rcvMpuSts & MPU_RUN_FAIL)    devPoint->status = 1;   // SDP-B 이상 : 1
            else if(linkCfg->online != SET)         devPoint->status = 1;   // SDP-B 이상 : 1 , LINK 실패시
            else                                    devPoint->status = 0;   // SDP-B 정상 : 0    
            gettimeofday(&devPoint->updateTime, NULL);    
            
        }
        else
        {
            /* ---------------------------- */
            /* SDP-B 정상 Check             */
            /* ---------------------------- */
            devPtIndex = INDEX_SDP_RUN_B;                       // Device Point-73 : SDP-B 동작상태, 0:정상, 1:이상 
            devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보

            if(mpuCFG->mpuStatus & MPU_RUN_FAIL)    devPoint->status = 1;   // SDP-B 이상 : 1
            else                                    devPoint->status = 0;   // SDP-B 정상 : 0    
            gettimeofday(&devPoint->updateTime, NULL);    
            
            /* ---------------------------- */
            /* SDP-A 정상 Check             */
            /* ---------------------------- */
            devPtIndex = INDEX_SDP_RUN_A;                       // Device Point-72 : SDP-A 동작상태, 0:정상, 1:이상  
            devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
            
            if(mpuCFG->rcvMpuSts & MPU_RUN_FAIL)    devPoint->status = 1;   // SDP-A 이상 : 1
            else if(linkCfg->online != SET)         devPoint->status = 1;   // SDP-A 이상 : 1 , LINK 실패시
            else                                    devPoint->status = 0;   // SDP-A 정상 : 0    
            gettimeofday(&devPoint->updateTime, NULL);    
            
        }
        
    }
    else
    {
        if(opr->cpuMode == MPU_A)
        {
            /* ---------------------------- */
            /* SDP-A 정상,  SDPB 이상       */
            /* ---------------------------- */
            devPtIndex = INDEX_SDP_RUN_A;                       // Device Point-72 : SDP-A 동작상태, 0:정상, 1:이상 
            devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보

            devPoint->status = 0;   // SDP-A 정상 : 0
            gettimeofday(&devPoint->updateTime, NULL);
            
            devPtIndex = INDEX_SDP_RUN_B;                       // Device Point-73 : SDP-B 동작상태, 0:정상, 1:이상  
            devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보

            devPoint->status = 1;   // SDP-B 이상 : 1
            gettimeofday(&devPoint->updateTime, NULL);
            
        }
        else if(opr->cpuMode == MPU_B)
        {
            /* ---------------------------- */
            /* SDP-A 이상,  SDP-B 정상      */
            /* ---------------------------- */
            devPtIndex = INDEX_SDP_RUN_A;                       // Device Point-72 : SDP-A 동작상태, 0:정상, 1:이상 
            devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보

            devPoint->status = 1;   // SDP-A 이상 : 1
            gettimeofday(&devPoint->updateTime, NULL);
            
            devPtIndex = INDEX_SDP_RUN_B;                       // Device Point-73 : SDP-B 동작상태, 0:정상, 1:이상  
            devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보

            devPoint->status = 0;   // SDP-B 정상 : 0
            gettimeofday(&devPoint->updateTime, NULL);
            
        }
    }

}        

/*
* Device Point-74 : RTU 동작모드, 0:SDP-A 동작, 1: SDP-B 동작  
*/
void    check_rtu_status()
{
    int     devPtIndex;
    //int		devNo, devPt;
    
    POINT_BUF   *devPoint;
    //POINT_BUF   *diptBuf;
    ESIO_CONFIG *esio;
    //SDP_DEVICE	*dev;

    /* ---------------------------------------- */
    /*  RTU 동작모드 ...                        */
    /* ---------------------------------------- */        
    devPtIndex = INDEX_RTU_MODE;                        // Device Point-74 : RTU 동작모드, 0:SDP-A 동작, 1: SDP-B 동작  
    devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
    
    if(opr->cpuMode == MPU_A)       devPoint->status = 0;   // SDP-A 동작 : 0
    else if(opr->cpuMode == MPU_B)  devPoint->status = 1;   // SDP-B 동작 : 1 
    gettimeofday(&devPoint->updateTime, NULL);

    /* ---------------------------------------- */
    /*  RTU 동작상태 ...                        */
    /* ---------------------------------------- */             
    esio = (ESIO_CONFIG *) esioCFG[ESIO_RTU];
    if(opr->cpuMode == MPU_A)
    {   
        devPtIndex = INDEX_RTU_RUN_A;                       // Device Point-75 : RTU#1 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
      	
        if(esio->online == SET)     devPoint->status = 0;   // 정상: 0
        else                        devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
        
        
        devPtIndex = INDEX_RTU_RUN_B;                       // Device Point-76 : RTU#2 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
        
        /* 상대방 MPU 동작상태 비교 : RTU 부  */
        if(mpuCFG->rcvRunSts & 0x20)        devPoint->status = 0;   // 정상: 0 
        else                                devPoint->status = 1;   // 이상: 1   
        gettimeofday(&devPoint->updateTime, NULL);
        
    }
    else
    {   
        devPtIndex = INDEX_RTU_RUN_B;                       // Device Point-76 : RTU#2 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
      
        if(esio->online == SET)     devPoint->status = 0;   // 정상: 0
        else                        devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);

        devPtIndex = INDEX_RTU_RUN_A;                       // Device Point-75 : RTU#1 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
        
        /* 상대방 MPU 동작상태 비교 : RTU 부  */
        if(mpuCFG->rcvRunSts & 0x20)        devPoint->status = 0;   // 정상: 0 
        else                                devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);

    }       
}                        

/*
* Device Point-77 : SCADA 동작모드, 0:SDP-A 동작, 1: SDP-B 동작  
* 이 놈이 ICCP와 HOST로 언제 넘어가니..?
*/
void    check_scada_status()
{
    int     devPtIndex;
    
    POINT_BUF   *devPoint;
    ESIO_CONFIG *esio;
    

    /* ---------------------------------------- */
    /*  SCADA 동작모드 ...                        */
    /* ---------------------------------------- */        
    devPtIndex = INDEX_SCADA_MODE;                      // Device Point-77 : SCADA 동작모드, 0:SDP-A 동작, 1: SDP-B 동작  
    devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
    
    if(opr->cpuMode == MPU_A)       devPoint->status = 0;   // SDP-A 동작 : 0
    else if(opr->cpuMode == MPU_B)  devPoint->status = 1;   // SDP-B 동작 : 1 

    // 정렬이 필요하군..        
    gettimeofday(&devPoint->updateTime, NULL);
        
    /* ---------------------------------------- */
    /*  RTU 동작상태 ...                        */
    /* ---------------------------------------- */             
    esio = (ESIO_CONFIG *) esioCFG[ESIO_SCADA]; //ESIO_SCADA =1 
    if(opr->cpuMode == MPU_A)
    {   
        devPtIndex = INDEX_SCADA_RUN_A;                     // Device Point-78 : SCADA#1 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
      
        if(esio->online == SET)     devPoint->status = 0;   // 정상: 0
        else                        devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
        
        devPtIndex = INDEX_SCADA_RUN_B;                     // Device Point-79 : SCADA#2 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
        
        /* 상대방 MPU 동작상태 비교 : ESIO#1-전력감시부 */
        if(mpuCFG->rcvRunSts & 0x02)        devPoint->status = 0;   // 정상: 0 
        else                                devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
    }
    else
    {   
        devPtIndex = INDEX_SCADA_RUN_B;                     // Device Point-79 : SCADA#2 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
      
        if(esio->online == SET)     devPoint->status = 0;   // 정상: 0
        else                        devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
        
        devPtIndex = INDEX_SCADA_RUN_A;                     // Device Point-78 : SCADA#1 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
        
        /* 상대방 MPU 동작상태 비교 : ESIO#1-전력감시부 */
        if(mpuCFG->rcvRunSts & 0x02)        devPoint->status = 0;   // 정상: 0 
        else                                devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
    }       
}                        

/*
* Device Point-80 : 원격진단부 동작모드, 0:SDP-A 동작, 1: SDP-B 동작  
*/
void    check_remote_status()
{
    int     devPtIndex;
    
    POINT_BUF   *devPoint;
    ESIO_CONFIG *esio;
    

    /* ---------------------------------------- */
    /*  원격진단부 동작모드 ...                 */
    /* ---------------------------------------- */        
    devPtIndex = INDEX_REMOTE_MODE;                     // Device Point-80 : 원격진단부 동작모드, 0:SDP-A 동작, 1: SDP-B 동작  
    devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
    
    if(opr->cpuMode == MPU_A)       devPoint->status = 0;   // SDP-A 동작 : 0
    else if(opr->cpuMode == MPU_B)  devPoint->status = 1;   // SDP-B 동작 : 1 
    gettimeofday(&devPoint->updateTime, NULL);

    /* ---------------------------------------- */
    /*  원격진단부 동작상태 ...                 */
    /* ---------------------------------------- */             
    esio = (ESIO_CONFIG *) esioCFG[ESIO_REMOTE];
    if(opr->cpuMode == MPU_A)
    {   
        devPtIndex = INDEX_REMOTE_RUN_A;                    // Device Point-81 : 원격진단부#1 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
      
        if(esio->online == SET)     devPoint->status = 0;   // 정상: 0
        else                        devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
        
        devPtIndex = INDEX_REMOTE_RUN_B;                     // Device Point-82 : 원격진단부#2 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
        
        /* 상대방 MPU 동작상태 비교 : ESIO#2-원격진단부 */
        if(mpuCFG->rcvRunSts & 0x04)        devPoint->status = 0;   // 정상: 0 
        else                                devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
    }
    else
    {   
        devPtIndex = INDEX_REMOTE_RUN_B;                     // Device Point-82 : 원격진단부#2 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
      
        if(esio->online == SET)     devPoint->status = 0;   // 정상: 0
        else                        devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
        
        devPtIndex = INDEX_REMOTE_RUN_A;                     // Device Point-81 : 원격진단부#1 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
        
        /* 상대방 MPU 동작상태 비교 : ESIO#2-원격진단부 */
        if(mpuCFG->rcvRunSts & 0x04)        devPoint->status = 0;   // 정상: 0 
        else                                devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
    }       
}                        


/*
* Device Point-83 : 전력품질부 동작모드, 0:SDP-A 동작, 1: SDP-B 동작  
*/
void    check_elecq_status()
{
    int     devPtIndex;
    
    POINT_BUF   *devPoint;
    ESIO_CONFIG *esio;
    

    /* ---------------------------------------- */
    /*  전력품질부 동작모드 ...                 */
    /* ---------------------------------------- */        
    devPtIndex = INDEX_ELECQ_MODE;                      // Device Point-83 : 전력품질부 동작모드, 0:SDP-A 동작, 1: SDP-B 동작  
    devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
    
    if(opr->cpuMode == MPU_A)       devPoint->status = 0;   // SDP-A 동작 : 0
    else if(opr->cpuMode == MPU_B)  devPoint->status = 1;   // SDP-B 동작 : 1 
    gettimeofday(&devPoint->updateTime, NULL);

    /* ---------------------------------------- */
    /*  전력품질부 동작상태 ...                 */
    /* ---------------------------------------- */             
    esio = (ESIO_CONFIG *) esioCFG[ESIO_ELECQ];
    if(opr->cpuMode == MPU_A)
    {   
        devPtIndex = INDEX_ELECQ_RUN_A;                    // Device Point-84 : 전력품질부#1 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
      
        if(esio->online == SET)     devPoint->status = 0;   // 정상: 0
        else                        devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
        
        devPtIndex = INDEX_ELECQ_RUN_B;                     // Device Point-85 : 전력품질부#2 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
        
        /* 상대방 MPU 동작상태 비교 : ESIO#3-전력품질부 */
        if(mpuCFG->rcvRunSts & 0x08)        devPoint->status = 0;   // 정상: 0 
        else                                devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
    }
    else
    {   
        devPtIndex = INDEX_ELECQ_RUN_B;                     // Device Point-85 : 전력품질부#2 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
      
        if(esio->online == SET)     devPoint->status = 0;   // 정상: 0
        else                        devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
        
        devPtIndex = INDEX_ELECQ_RUN_A;                     // Device Point-84 : 전력품질부#1 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
        
        /* 상대방 MPU 동작상태 비교 : ESIO#3-전력품질부 */
        if(mpuCFG->rcvRunSts & 0x08)        devPoint->status = 0;   // 정상: 0 
        else                                devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
        
    }       
}                        


/*
* Device Point-86 : 61850 동작모드, 0:SDP-A 동작, 1: SDP-B 동작  
*/
void    check_61850_status()
{
    int     devPtIndex;
    
    POINT_BUF   *devPoint;
    ESIO_CONFIG *esio;
    

    /* ---------------------------------------- */
    /*  61850 동작모드 ...                 */
    /* ---------------------------------------- */        
    devPtIndex = INDEX_61850_MODE;                      // Device Point-86 : 61850 동작모드, 0:SDP-A 동작, 1: SDP-B 동작  
    devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
    
    if(opr->cpuMode == MPU_A)       devPoint->status = 0;   // SDP-A 동작 : 0
    else if(opr->cpuMode == MPU_B)  devPoint->status = 1;   // SDP-B 동작 : 1 
    gettimeofday(&devPoint->updateTime, NULL);

    /* ---------------------------------------- */
    /*  61850 동작상태 ...                 */
    /* ---------------------------------------- */             
    esio = (ESIO_CONFIG *) esioCFG[ESIO_61850];
    if(opr->cpuMode == MPU_A)
    {   
        devPtIndex = INDEX_61850_RUN_A;                     // Device Point-87 : 61850#1 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
      
        if(esio->online == SET)     devPoint->status = 0;   // 정상: 0
        else                        devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
        
        devPtIndex = INDEX_61850_RUN_B;                     // Device Point-88 : 61850#2 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
        
        /* 상대방 MPU 동작상태 비교 : ESIO#4-61850 */
        if(mpuCFG->rcvRunSts & 0x10)        devPoint->status = 0;   // 정상: 0 
        else                                devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
    }
    else
    {   
        devPtIndex = INDEX_61850_RUN_B;                     // Device Point-88 : 61850#2 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
      
        if(esio->online == SET)     devPoint->status = 0;   // 정상: 0
        else                        devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
        
        devPtIndex = INDEX_61850_RUN_A;                     // Device Point-87 : 61850#1 동작상태, 0:정상, 1:이상
        devPoint = (POINT_BUF *) devPtBuf[devPtIndex];      // Device Point 정보
        
        /* 상대방 MPU 동작상태 비교 : ESIO#4-61850 */
        if(mpuCFG->rcvRunSts & 0x10)        devPoint->status = 0;   // 정상: 0 
        else                                devPoint->status = 1;   // 이상: 1    
        gettimeofday(&devPoint->updateTime, NULL);
    }       
    
}                        


/*
* =================================================
*  Device 포인트 ... SOE 생성 
* =================================================
*/
int check_Device_SOE()
{
    int index;
    int hostid, dnpPoint;
    
    HOST_DCB    *host;
    POINT_BUF   *devPoint;
    MPU_SOEQ_ENTRY  devEvent;
    
    /* ---------------------------------------------------------------- */
    /*  자동/수동(65) ~ 61850-B 동작(88) 까지의 SOE 정보 생성           */
    /* ---------------------------------------------------------------- */
    for(index = INDEX_AUTO_MANUAL; index <= INDEX_61850_RUN_B; index++)
    {
        devPoint = (POINT_BUF *)   devPtBuf[index];     // SDP 포인트 Config 정보   
        if(devPoint->config != SET) continue;

        /* ------------------------------------ */
        /* ICCP TIME-SYNC 포인트 .... skip      */
        /*  RTC 설정시... 이벤트 생성...        */
        /* ------------------------------------ */
        if(index == INDEX_SDP_TIMESYNC) continue;       // SDP 시각동기 명령
        if(index == INDEX_CNTR_CHANGE) continue;        // SDP 이중화 절체

#if 1            
        /* ------------------------------------ */
        /* DNP HOST : Device 포인트 Update ...  */
        /* ------------------------------------ */
        for(hostid=0; hostid < MAX_HOST; hostid++)
        {
            host = (HOST_DCB *) hostDCB[hostid];
            if(host->hostDualMode == HOST_NOT_USE)	continue;
        
            dnpPoint = devPoint->hostIndex[hostid] - 1;         // SDP POINT : 상위 호스트#1 Index 번호 [1..1024]
            if((dnpPoint < 0) || (dnpPoint >= MAX_DNP_DI_POINT))    continue;         

            /* DNP HOST - 0: 이상, 1: 정상 */
            if(devPoint->status == 1)   host->sts_pointData[dnpPoint] = PT_FLAG_STS_ON  | PT_FLAG_ONLINE;
            else                        host->sts_pointData[dnpPoint] = PT_FLAG_STS_OFF | PT_FLAG_ONLINE;        
        }

        //printf(">> system Update... (%d) config=%d, %s \n", index, devPoint->config, devPoint->ptNameStr);
#endif
        
        /* ------------------------------------ */
        /*  SDP에서 ICCP-POINT Update...        */
        /* ------------------------------------ */
        update_ICCP_info(shmPtr, devPoint, (float) devPoint->status, devPoint->config); 
        
        /* -------------------------------------------- */                   
        /* DEVICE 포인트 상태변화 Check...              */
        /* -------------------------------------------- */  
        if(devPoint->oldsts != devPoint->status)
        {
            /* -------------------------------- */       
            /* DEVICE-SOE 생성                  */
            /* -------------------------------- */   
            devEvent.eventCode = ENT_DEVICE_SOE;
            devEvent.devNo   = index + 1;           // Devie Point,,,1,2
            devEvent.pointNo = 0;
            devEvent.state   = devPoint->status;    // 장치상태 : 이상
            devEvent.esioNo  = opr->cpuMode;            // 이벤트 발생주체 : [0] CPU-A, [1] CPU-B
            
            gettimeofday(&devEvent.updateTime, NULL);
            
            /* DEVICE-SOE 생성 */
            if(opr->devSoeENBTick > 5)	// 초기부팅후... 초기 이벤트 생성않함.
          	{  	
            	Create_Device_SOE(shmPtr, &devEvent);   // 통신이상
            }
        }
        
        devPoint->oldsts = devPoint->status;                  
    }
 
    return (0);
}


/*
* =================================================
*  시스템 디바이스 포인트 정보 Update 
* - Device 포인트 정보를 내부 DI 모듈부로 연계
* =================================================
*/
int update_Device_Info()
{
    int index;
    int		devNo, devPt;
    int hostid, dnpPoint;
    
    HOST_DCB    *host;
    POINT_BUF   *devPoint;
    POINT_BUF   *diptBuf;
    SDP_DEVICE	*dev;
    
    /* ---------------------------------------------------------------- */
    /*  전체 Device 포인트 정보 추출           */
    /* ---------------------------------------------------------------- */

    for(index = 0; index < MAX_DEV_POINT; index++) 
//    for(index = 0; index <= MAX_DEV_POINT; index++) //  == 이 있어서 buffer overflow 발생했음.

    {
        devPoint = (POINT_BUF *) devPtBuf[index];     // SDP 포인트 Config 정보   
        if(devPoint->config != SET) continue;

        /* ------------------------------------ */
        /* ICCP TIME-SYNC 포인트 .... skip      */
        /* INDEX_CNTR_CHANGE 포인트 .... skip      */
        /* ------------------------------------ */
        if(index == INDEX_SDP_TIMESYNC) continue;       // SDP 시각동기 명령
        if(index == INDEX_CNTR_CHANGE) continue;        // SDP 이중화 절체

        /* 상태포인트 정보...추출 */
      	devNo = devPoint->devNo - 1;
      	devPt = devPoint->devPt - 1;
      	
      	if((devNo < 0) || (devNo >= MAX_DEVICE))		continue;
   		if((devPt < 0) || (devPt >= MAX_DEV_DI_POINT))	continue;
      		
      	dev      = (SDP_DEVICE *) deviceCFG[devNo];
      	diptBuf  = (POINT_BUF *) &dev->diPtBuf[devPt];

		/* ------------------------------------ */ 
        /* 내부 DEVIE 정보  Data Assign...         */
        /* ------------------------------------ */ 
      	if(diptBuf->config != SET)	continue;
	
		diptBuf->status = devPoint->status;
		
        /* ------------------------------------ */ 
        /* DNP HOST Data Assign...              */
        /*  => DNP-HOST 별 상태포인트 정보 저장 */
        /* ------------------------------------ */ 
      
        if(opr->dnpHostEnb == SET)
        {
	        /* ------------------------------------ */
   	     	/* DNP HOST : Device 포인트 Update ...  */
        	/* ------------------------------------ */
	        for(hostid=0; hostid < MAX_HOST; hostid++)
    	    {
        	    host = (HOST_DCB *) hostDCB[hostid];
	            if(host->hostDualMode == HOST_NOT_USE)	continue;
    	      	if(host->hostProtocol != HOST_DNP)      continue;  
        
        	    dnpPoint = devPoint->hostIndex[hostid] - 1;         // SDP POINT : 상위 호스트#1 Index 번호 [1..1024] 
        	                                                        // MAX_DNP_DI_POINT  2048
            	if((dnpPoint < 0) || (dnpPoint >= MAX_DNP_DI_POINT))    continue;         

//                if ( dnpPoint > 0)
//                    Debug(console,"INX[%d] H[%d] PT[%d\r\n", index ,hostid, dnpPoint);
	            /* DNP HOST - 0: 이상, 1: 정상 */
    	        if(devPoint->status == 1)   host->sts_pointData[dnpPoint] = PT_FLAG_STS_ON  | PT_FLAG_ONLINE;
        	    else                        host->sts_pointData[dnpPoint] = PT_FLAG_STS_OFF | PT_FLAG_ONLINE;        
        	}
		}
		
    }
 
    return (0);
}



/*----------------------------------------------------------------------------
* Function Name : systemStsUpdate()
* 수행내용: 이중화시스템 관련 상태정보 Update
* ArgList :
* Return  :
---------------------------------------------------------------------------- */
void systemStsUpdate()
{
	// 여기서 죽는 문제가 발생하니..일단 처리하자.
	
	/* 초기 기동시.... DEVICE-SOE 발생하지 않음 */
	if(++opr->devSoeENBTick > 8)	opr->devSoeENBTick = 8;
		
	/* DEV-TEST 인 경우... SKIP */
	if(opr->devTestFlag == 0)
	{		    
	    check_AutoManual();         // Device Point-65 : 수동/자동 상태, 0:자동, 1:수동

    	check_sdp_status();         // Device Point-70 : SDP 동작상태, 0:정상, 1:이상  

    	check_sdp_runMode();        // Device Point-71 : SDP 동작모드, 0:SDP-A 동작, 1:SDP-B 동작 
    
    	check_sdp_runState();       // Device Point-72, 73 : SDP-A/B 동작모드, 0:정상, 1:이상 
    
    	check_rtu_status();         // Device Point-74,75,76 : RTU 동작모드, 0:SDP-A 동작, 1: SDP-B 동작     
    	check_scada_status();       // Device Point-77,78,79 : 전철제어반 동작모드, 0:SDP-A 동작, 1: SDP-B 동작
    	check_remote_status();      // Device Point-80,81,82 : 원격진단 동작모드, 0:SDP-A 동작, 1: SDP-B 동작   
    
    	check_elecq_status();       // Device Point-83,84,85 : 전력품질 동작모드, 0:SDP-A 동작, 1: SDP-B 동작  
    	check_61850_status();       // Device Point-86,87,88 : 61850 동작모드, 0:SDP-A 동작, 1: SDP-B 동작       


	}


    /* ---------------------------------------- */
    /*  DEVICE 포인트 Update. 추출 ...         */
    /* ---------------------------------------- */

#if 1  // 이 것 땀시 죽는데.. buffer overflow... 
    update_Device_Info(); // 여기서..처리해주는 구나..devPoint->host 로.
    /* ---------------------------------------- */
    /*  DEVICE 포인트 SOE 정보 추출 ...         */
    /* ---------------------------------------- */
#endif    
    check_Device_SOE(); // 65 번이후.
    
}
        