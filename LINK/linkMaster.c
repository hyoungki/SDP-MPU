
#include	"localLib.h"
#include    "external.h"
#include    "extfunc.h"

extern	int     termExec;

extern	TTY_DESC 	linkPort;
extern	TASK_INFO	*taskPtr;


extern  ICCP_SOE_DELETE_QUEUE   *iccpDelSOEQ;     // ICCP-HOST Delete SOEQ

static  byte    linkRxbuf[4096];
static  byte    linkTxbuf[4096];


static  int     scanDevNo = 0;
static  int     linkDevNo = 0;

extern  int	    checkDualCPU(int mode);
extern  void    resetMaster();
extern  void    setMaster();

extern  int     check_ICCP_deleteSOEQ();
extern  int     update_Device_DNP();


/*
* ======================================================
*   MASTER-CPU 메세지 구성
* ======================================================
*/
int	 makeLink_MASTER( byte *sndbuf)
{
    int     i;
    int     opcode;
    int     count, sndCount;
    int     dumpCount;
    int     dbSize;
    int     startPoint, reportPoint, pointNum;
    byte    front, rear, Qnum;
    word    chksum;
    
    SDP_DEVICE      *dev;
    POINT_BUF       *aiPoint;

    DB_ESIO_CONFIG  *dbESIO;
    DB_HOST_CONFIG  *dbHost;

    /* ---------------------------- */
	/*	OPCODE 처리 				*/
	/* ---------------------------- */
	if(opr->cpuChange)                  opcode = SIM_SYSTEM_CNTR;       // 절체 명령인가벼..
	else if(linkCfg->timeSyncReq)		opcode = SIM_TIME_DOWN;         // 0X11
	else if(linkCfg->chksumReq)		    opcode = SIM_CHKSUM_DUMP;	
	else if(linkCfg->mpuCfgDown)        opcode = SIM_MPUCFG_DOWN;       // MPU Config 전송요구
	else if(linkCfg->esioCfgDown)       opcode = SIM_ESIOCFG_DOWN;    	// ESIO Config 전송요구    
	else if(linkCfg->hostCfgDown)       opcode = SIM_HOST_DOWN;	        // HOST Config 전송요구
	else if(linkCfg->iccpCfgDown)	    opcode = SIM_ICCP_DOWN;	        // ICCP Config 전송요구
	else if(linkCfg->harrisCfgDown)	    opcode = SIM_HARRIS_DOWN;	    // HARRIS Config 전송요구
	else if(linkCfg->landisCfgDown)	    opcode = SIM_LANDIS_DOWN;	    // LANDIS Config 전송요구
	else if(linkCfg->modbusCfgDown)	    opcode = SIM_MODBUS_DOWN;	    // MODBUS Config 전송요구
    else if(linkCfg->scanCfgDown)	    opcode = SIM_SCAN_DOWN;         // SCAN Config 전송요구
    else if(linkCfg->deviceCfgDown)	    opcode = SIM_DEVICE_DOWN;       // DEVICE Config 전송요구
    else if(linkCfg->calptCfgDown)      opcode = SIM_CAL_POINT_DOWN;    // CAL-POINT Config 전송요구       
	else if(linkCfg->pointCfgDown)      opcode = SIM_POINT_DOWN;        // POINT Config 전송요구
	else
	{
	    /* ------------------------------------ */
	    /* 계전기 상태/아날로그 정보 연계       */
	    /* ------------------------------------ */
        for(i=0 ; i < MAX_DEVICE; i++)
        {
            if(++scanDevNo > MAX_DEVICE) 
            {
                scanDevNo = 0;
                // dev별로  point scan 하고...sim_sdp_status 하고  analog scan 하고...
                linkCfg->chgOpcode = (linkCfg->chgOpcode + 1) & 0x01;
                
                opcode = SIM_SDP_STATUS;       // 계전기 통신상태...
                break;
            }
            else
            {   //
                linkDevNo = scanDevNo - 1;      
                if(linkCfg->chgOpcode)  opcode = SIM_POINT_SCAN;    // 계전기 상태포인트 정보 
                else                    opcode = SIM_ANALOG_SCAN;   // 계전기 계측포인트 정보 
                
                dev = (SDP_DEVICE *) deviceCFG[linkDevNo];
                if(dev->scan)   break;    // scan 을 하면...
            }
        }

#if 0        
        /* -------------------------------------------- */
	    /* 1) 이중화 CPU : ICCP Delete SOE 정보를 연계..		*/
	    /* -------------------------------------------- */
	    if(opr->iccpEnbFlag == SET)
	    {	
		    if((opr->dualCpuSts == SET) && (linkCfg->iccpDelSOE))
		    {  
	    	    opcode = SIM_ICCP_DEL_SOE;          // ICCP Delete-SOE 정보    
	    	}
	    }
#endif
	    
        /* -------------------------------------------- */
	    /* 2) 이중화 CPU : 수집된 SOE 정보를 연계...    			*/
	    /* -------------------------------------------- */
	    if((opr->dualCpuSts == SET) && (opr->runMode == LOCAL_MASTER))
	    {  
	        front = linkCfg->front & 0xff;
	        rear  = linkCfg->rear & 0xff;
	       
	        if(front != rear)   opcode = SIM_EVENT_DUMP;      // 계전기 SOE 정보    
	    }
	}    

    /* -------------------------------------------- */
	/* 2019.07.27 SDP-A/B Reset 재기동 명령 Pass ... 	*/
	/* -------------------------------------------- */
	if(linkCfg->cntFlag == SET)  
	{
		if((opr->dualCpuSts == SET) && (linkCfg->online == SET))		opcode = SIM_POINT_CNTR;
		else
		{
			Debug(console, "master> *LINK Control Failed.... dualsts=%d, link-online = %d \n", opr->dualCpuSts, linkCfg->online);
			
			linkCfg->cntFlag  = RESET;
			linkCfg->cntPoint = 0;
            linkCfg->cntTCF   = 0;        // 제어상태, [0] TRIP, [1] CLOSE
		}
	}					    	
    
        
    /* -------------------------------- */                      
    /* 송신 Frame 구성                      */                      
    /* -------------------------------- */                      
    sndbuf[0] = 0x7e;                       // stx              
    sndbuf[1] = 0x7e;                       // stx              
    sndbuf[2] = 0;                          // size             
    sndbuf[3] = 0;                          // size        
    sndbuf[4] = opcode;                     // opcode          
    sndbuf[5] = linkCfg->sdpStatus;         // link status, PFR:0x80, DB-CHK:0x02, SOE:0x01
    sndbuf[6] = linkCfg->sndSeqNo;          // 수신 SeqNo       
    count = 7;                                                  


    switch(opcode)
    {
    case SIM_SYSTEM_CNTR:
        Debug(console,"Link-master> snd CPU-CHANGE....\n");
        break;
                
    case SIM_SDP_STATUS:        // 0x01 SDP 장치상태정보 Dump
        /* ---------------------------------------- */
        /*  SDP 상태정보 추출 및 전송               */
        /* ---------------------------------------- */
        sndbuf[count++] = mpuCFG->mpuStatus;            // Master-MPU 상태정보
        sndbuf[count++] = mpuCFG->mpuRackSts;           // Master-MPU RACK 상태정보
        sndbuf[count++] = mpuCFG->mpuRunSts;            // Master-MPU 모듈 상태정보
        
        /* ---------------------------------------- */
        /*  HOST & DEVICE 상태정보 추출 및 전송     */
        /* ---------------------------------------- */
        for(i=0; i < MAX_HOST; i++)         sndbuf[count++] = hostDCB[i]->runStatus;        // HOST# 통신 운영 상태
        for(i=0; i < MAX_DEVICE; i++)       sndbuf[count++] = deviceCFG[i]->online;         // 계전기# 통신 운영 상태
        for(i=0; i < MAX_DEV_POINT; i++)    sndbuf[count++] = devPtBuf[i]->status;          // DEVICE 포인트 운영 상태        
        
        if(opr->linkDebug)
    	Debug(console,"master> SDP_STATUS.... SND: sts = %02x, rack = %02x [RCV: sts = %02x, rack = %02x] \n", 
    	    mpuCFG->mpuStatus, mpuCFG->mpuRackSts, mpuCFG->rcvMpuSts, mpuCFG->rcvRackSts);
    	    
        break;
    
    case SIM_ICCP_DEL_SOE:                      // 0x42 ICCP Delete-SOE 정보 Dump
        front = iccpDelSOEQ->front & 0xff;
        rear  = iccpDelSOEQ->rear & 0xff;
    
        Qnum = (front - rear + 256) % 256;
        
        if(Qnum > 16)   Qnum = 16;
            
        sndbuf[count++] = Qnum;       // SOE 전송 개수

        if(Qnum > 0)
        {
            for(i=0; i< Qnum; i++)
            {
                /* ICCP-Delete SOE 이벤트 내용을 Copy */
                memcpy( &sndbuf[count], (byte *) &iccpDelSOEQ->queue[rear], sizeof(ICCP_SOEQ_ENTRY));
                count += sizeof(ICCP_SOEQ_ENTRY);

                rear = (rear + 1) & 0xff;
            }
        }      
        
        linkCfg->delReport = Qnum;
        if(opr->soeDebug)   
        Debug(console, "master> snd ICCP Delete-SOEQ Dump...count=%d\n", linkCfg->delReport);   
        break;
        
    case SIM_EVENT_DUMP:        // 0x02 SDP EVENT Dump
        if(opr->dualCpuSts != SET)
        {
            sndbuf[count++] = 0x00;     // 전송 Q-num
            break;
        }

		/* -------------------------------------------- */
		/*	CPU 이중화 : Master/Slave 간 이벤트 내용 공유			*/
		/* -------------------------------------------- */        
        if(opr->runMode == LOCAL_MASTER)
        {   
            front = linkCfg->front & 0xff;
            rear  = linkCfg->rear & 0xff;
        
            Qnum = (front - rear + 256) % 256;
            if(Qnum > 16)   Qnum = 16;			// 한번에 전송하는 SOE 이벤트 갯수 제한...
        
            sndbuf[count++] = Qnum;       // SOE 전송 개수

            for(i=0; i< Qnum; i++)
            {
                /* SOE 이벤트 내용을 Copy */
                memcpy( &sndbuf[count], (byte *) &linkCfg->linkSOE[rear], sizeof(MPU_SOEQ_ENTRY));
                count += sizeof(MPU_SOEQ_ENTRY);
                rear = (rear + 1) & 0xff;
            }      

            linkCfg->reportSOE = Qnum;
            
            if(opr->soeDebug)   
            Debug(console, "master> snd LINK-SOE Dump...count=%d\n", Qnum);         
            
            /* -------------------------------- */
            /* SOE 전송후 ... rear 포인트 증가 		*/
            /* -------------------------------- */
            linkCfg->rear = (linkCfg->rear + Qnum) & 0xff;   
        }
        else
        {
            sndbuf[count++] = 0x00;     // 전송 Q-num
            break;
        }
                
        break;
        
        
    case SIM_EVENT_ACK:         // 0x03 SDP EVENT-ACK
        break;
            
    case SIM_POINT_SCAN:        // 0x04 계전기 상태정보 Scan
        if(opr->runMode == LOCAL_MASTER)
        {    
            dev = (SDP_DEVICE *) deviceCFG[linkDevNo];

            /* ---------------------------------------- */
            /*  계전기 Online : 수신덴 상태데이터 전송  */
            /*         Offline: 계전기 버퍼내용 전송    */
            /* ---------------------------------------- */
            if((dev->online == SET) && (dev->init_status == SET))
            {	
	            sndbuf[7] = linkDevNo + 1;          // 계전기 번호, 1,2..64     
    	        sndbuf[8] = dev->stsByteCnt;        // 데이터 사이즈 (Byte 수)     
        	    count = 9;
            
                /* -------------------------------- */
                /* 계전기별 상태정보(BIT) 전송      */
                /* -------------------------------- */
            	memcpy( &sndbuf[count], &dev->cursts[0], dev->stsByteCnt);
            	count = count + dev->stsByteCnt;
            }
            else
            {
            	sndbuf[7] = linkDevNo+1;            // 계전기 번호, 1,2..64 
    	        sndbuf[8] = 0;                      // 데이터 사이즈 (Byte 수)     
        	    count = 9;
            }	
        }
        else
        {
            sndbuf[count++] = linkDevNo + 1;        // 계전기 번호, 1,2..64  
            sndbuf[count++] = 0;                    // 데이터 사이즈 (Byte 수)
        }
        break;
        
    case SIM_ANALOG_SCAN:       // 0x14 계전기 아날로그 정보 Scan
        if(opr->runMode == LOCAL_MASTER)
        {    
            dev = (SDP_DEVICE *) deviceCFG[linkDevNo];
            sndbuf[7] = linkDevNo+1;                // 계전기 번호, 1,2..64     
            sndbuf[8] = 0;                          // Dump 포인트 수      
            count = 9;

            /* ------------------------------------ */
       	    /* 2014.08.12 계측 포인트가 없는 경우.  */
       	    /* ------------------------------------ */
       	    if(dev->devAiPoint <= 0)    break;
        	        
            /* -------------------------------- */
            /* 계전기별 아날로그(Float) 전송    */
            /* -------------------------------- */
            dumpCount = 0;
            for(i=0; i < MAX_DEV_AI_POINT; i++)
            {
                aiPoint = (POINT_BUF *) &dev->aiPtBuf[i];
                if(aiPoint->config == 0)    continue;
                
                sndbuf[count++] = ( i >> 8) & 0xff;                 // 계측포인트 번호 (0,1,2,...)
                sndbuf[count++] = i & 0xff;                         // 계측포인트 번호 (0,1,2,...)
                
                memcpy( &sndbuf[count], &aiPoint->floatData, 4);    // 계측 데이터(Float)
                count += 4;
                dumpCount++;    
                
                /* 보고되는 아날로그 데이터 제한 */
                if(dumpCount > 128)  break;    
            }
            
            sndbuf[8] = dumpCount;                  // Dump 포인트 수      
        }
        else
        {
            sndbuf[7] = linkDevNo + 1 ;             // 계전기 번호, 1,2..64   
            sndbuf[8] = 0;                          // Dump 포인트 수  
            count = 9;
        }
        break;
    
    case SIM_CHKSUM_DUMP:       // 0x16 데이터베이스 Chksum Scan       
        chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
        sndbuf[count++] = (chksum >> 8) & 0xff;
        sndbuf[count++] = chksum & 0xff;
        break;
        
    case SIM_POINT_CNTR:        // 0x05 계전기 POINT 제어
        /* ---------------------------------------- */
        /*  CPU 이중화가 아닌경우...                */
        /* ---------------------------------------- */
        if(opr->dualCpuSts != SET)
        {
            sndbuf[count++] = 0;                // 대상 계전기 번호 [1..64]
            sndbuf[count++] = 0;                // 대상 포인트 [1..256]
            sndbuf[count++] = 0;                // 제어상태, [1] TRIP, [2] CLOSE
            break;
        }
        
        if((opr->cntrLinkPass == SET) || (opr->runMode == LOCAL_SLAVE))
        {
            sndbuf[7] = linkCfg->cntDev;        // 대상 계전기 번호 [1..64]
            sndbuf[8] = linkCfg->cntPoint;      // 대상 포인트 [1..256]
            sndbuf[9] = linkCfg->cntTCF;        // 제어상태, [0] TRIP, [1] CLOSE
            count = 10;
            
            if(opr->soeDebug)
		    Debug(console,"master> snd POINT-CONTROL ... dev=%d, point=%d, tcf=%d \n", linkCfg->cntDev, linkCfg->cntPoint, linkCfg->cntTCF);
		    
		    /* ------------------------ */
            /* 제어정보 Clear...        */  
            /* ------------------------ */          
            linkCfg->cntDev     = 0;
            linkCfg->cntPoint   = 0;
            linkCfg->cntTCF     = 0;
            
            linkCfg->cntFlag    = 0;
            linkCfg->cntHost    = 0;
            opr->cntrLinkPass   = 0;
        }    
        break;
        
    case SIM_TIME_DOWN:         // 0x11 Time Sync Down
        sndbuf[count++]  = (rtc->year >> 8) & 0xff;
        sndbuf[count++]  = rtc->year & 0xff;
        sndbuf[count++]  = rtc->month;  
        sndbuf[count++]  = rtc->day;  
        sndbuf[count++]  = rtc->hour;  
        sndbuf[count++]  = rtc->min;  
        sndbuf[count++]  = rtc->sec;  

       	//if(opr->linkDebug)
    	printf("master> TIME Sync Down : %4d/%2d/%2d %02d:%02d:%02d \n", rtc->year, rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
    	linkCfg->timeSyncReq = RESET;
        break;

                
    case SIM_MPUCFG_DOWN:       // 0x20 DB : MPU Config Down
        dbSize = sizeof(DB_MPU_CONFIG);
		memcpy( &sndbuf[count], (byte *) &rtudb->mpuConfig,  dbSize);
		count += dbSize;
		
		//if(opr->linkDebug)
		Debug(console,"master> snd MPU Config ... size = %d\n", dbSize);
        break;
        
    case SIM_ESIOCFG_DOWN:      // 0x22 DB : ESIO Config Down
        dbSize = sizeof(DB_ESIO_CONFIG);
        dbESIO = (DB_ESIO_CONFIG *) &rtudb->esioConfig[linkCfg->dbDownEsio - 1];
        
        sndbuf[count++] = linkCfg->dbDownEsio;                  // ESIO#  [1..5]
        sndbuf[count++] = 1;                                    // 전송 ESIO 갯수 (default = 1)
		memcpy( &sndbuf[count], (byte *) dbESIO,  dbSize);      // ESIO DB
		count += dbSize;
		
		//if(opr->linkDebug)
		Debug(console,"master> snd ESIO (%d) Config ... size = %d\n", linkCfg->dbDownEsio, dbSize);
        break;
        
    case SIM_HOST_DOWN:         // 0x24 DB : HOST Config Down
        dbSize = sizeof(DB_HOST_CONFIG) * MAX_DB_HOST;
        dbHost = (DB_HOST_CONFIG *) &rtudb->hostCfg[0];
		memcpy( &sndbuf[count], (byte *) dbHost,  dbSize);
		count += dbSize;
		
		//if(opr->linkDebug)
		Debug(console,"master> snd HOST Config ... size = %d\n", dbSize);
        break;
        
    case SIM_ICCP_DOWN:         // 0x26 DB : ICCP-HOST Config Down
        dbSize = sizeof(DB_ICCP_CONFIG);
		memcpy( &sndbuf[count], (byte *) &rtudb->iccpConfig,  dbSize);
		count += dbSize;
		
		//if(opr->linkDebug)
		Debug(console,"master> snd ICCP Config ... size = %d\n", dbSize);
        break;
        
    case SIM_HARRIS_DOWN:       // 0x28 DB : HARRIS Config Down
        dbSize = 32;
		memcpy( &sndbuf[count], (byte *) &rtudb->portdb[0],  dbSize);
		count += dbSize;
		
		//if(opr->linkDebug)
		Debug(console,"master> snd HARRIS Config ... size = %d\n", dbSize);
        break;
        
    case SIM_LANDIS_DOWN:       // 0x2A DB : LANDIS Config Down
        dbSize = 257;
		memcpy( &sndbuf[count], (byte *) &rtudb->chassisNum,  dbSize);
		count += dbSize;
		
		//if(opr->linkDebug)
		Debug(console,"master> snd LANDIS Config ... size = %d\n", dbSize);
        break;
        
    case SIM_MODBUS_DOWN:       // 0x2C DB : MODBUS Config Down
        dbSize = sizeof(DB_MODBUS_PROFILE) * MAX_DB_MODBUS_PROFILE;
		memcpy( &sndbuf[count], (byte *) &rtudb->modbusProfile[0],  dbSize);
		count += dbSize;
		
		//if(opr->linkDebug)
		Debug(console,"master> snd MODBUS Config ... size = %d\n", dbSize);
        break;
        
    case SIM_SCAN_DOWN:         // 0x30 DB : SCAN Config Down
        dbSize = sizeof(DB_SCAN_CONFIG) * MAX_DB_SCAN_PORT;
		memcpy( &sndbuf[count], (byte *) &rtudb->scanConfig[0],  dbSize);
		count += dbSize;
		
		//if(opr->linkDebug)
		Debug(console,"master> snd SCAN Config ... size = %d\n", dbSize);
        break;
        
    case SIM_DEVICE_DOWN:       // 0x32 DB : DEVICE Config Down
        dbSize = sizeof(DB_SDP_DEVICE) * 16;
        
        if(linkCfg->dbDownDevice >= 96) linkCfg->endDeviceFlag = SET;
        else                            linkCfg->endDeviceFlag = RESET;      
        
        sndbuf[count++] = linkCfg->dbDownDevice;    // 시작 계전기 번호 [1..64]
        
        if(linkCfg->endDeviceFlag == SET)   sndbuf[count++] = 16 + 0x80;                // 계전기 수
        else                                sndbuf[count++] = 16;                       // 계전기 수                 
		memcpy( &sndbuf[count], (byte *) &rtudb->deviceConfig[linkCfg->dbDownDevice - 1],  dbSize);
		count += dbSize;
		
		//if(opr->linkDebug)
		Debug(console,"master> snd DEVICE (st=%d, num=%d) Config ... size = %d\n", linkCfg->dbDownDevice, 16,  dbSize);
        break;
    
        
    case SIM_POINT_DOWN:        // 0x34 DB : POINT Config Down
        pointNum    = 16;
		startPoint  = linkCfg->dbDownPoint;
		reportPoint = startPoint + pointNum;
		
		/* ------------------------------------ */
		/* 실제 등록된 포인트 Check...          */
		/* ------------------------------------ */
		if(reportPoint >= MAX_DB_POINT)
		{
			pointNum = 	MAX_DB_POINT - linkCfg->dbDownPoint + 1;
			linkCfg->endPointFlag  = 0x80;	// point update flag...
		}	
		
		if(startPoint >= MAX_DB_POINT)
		{
			linkCfg->pointCfgDown = RESET;
			linkCfg->dbDownFlag   = RESET;
			Debug(console,"master> snd POINT-DB ...MAX start=%4d, num=%d ... pointcfg=%d, downFlag=%d... return \n", startPoint, pointNum, linkCfg->pointCfgDown, linkCfg->dbDownFlag);
			
			return(0);
		}
			
		linkCfg->dbDownIndex = pointNum;
		
		sndbuf[count++] = (startPoint >> 8) & 0xff;
		sndbuf[count++] = startPoint & 0xff;
		sndbuf[count++] = pointNum | linkCfg->endPointFlag; // endPointFlag 0x80 ->   16은 hex로 0x10 --> 0x90
		
        dbSize = sizeof(DB_POINT_BUF) * pointNum;
		memcpy( &sndbuf[count], (byte *) &rtudb->pointBuf[startPoint - 1],  dbSize);
		count += dbSize;
		
		//if(opr->linkDebug)
        Debug(console,"master> snd POINT-DB ...start=%4d, num=%d, size = %d\n", startPoint, pointNum, dbSize);
        break;
    
    case SIM_CAL_POINT_DOWN:        // 0x36 DB : CAL-POINT Config Down
        pointNum    = 16;
		startPoint  = linkCfg->dbDownCalpt;
		reportPoint = startPoint + pointNum;
		
		sndbuf[count++] = (startPoint >> 8) & 0xff;
		sndbuf[count++] = startPoint & 0xff;
		
		if(startPoint == 17)    sndbuf[count++] = pointNum | 0x80;
        else                    sndbuf[count++] = pointNum;
                 		
        dbSize = sizeof(DB_CAL_POINT) * pointNum;
		memcpy( &sndbuf[count], (byte *) &rtudb->calPointBuf[startPoint - 1],  dbSize);
		count += dbSize;
		
		if(opr->linkDebug)
        Debug(console,"master> snd CAL POINT-DB ...start=%4d, num=%d, size = %d\n", startPoint, pointNum, dbSize);
        break;
        
            
	default :
		break;	
	}


    /* -------------------------------- */                      
    /* 송신 Frame 구성                      */                      
    /* -------------------------------- */         	
    sndCount = count + 1;
                     
    sndbuf[2] = (sndCount >> 8) & 0xff;     /* Total size: MSB */
    sndbuf[3] = sndCount & 0xff;            /* Total size: LSB */
    sndbuf[count] = genlrc(sndbuf, count);
    count++;
    
    return(count);

}


/**
**
**/
int rcvHandler_MASTER(byte *sndbuf, byte *rxbuf, int rxcnt)
{
    int     i, index;
    int     opcode;
    int     count, sndCount;
    int     startPoint, anapoint;
    int     devNo, devNum;
    //int     devPt, pointSts;
    int     rcvQnum;     
    int     rcvAckCode, rcvEventNum;
    int     devid, rcvStsByte, dumpCount;
    int     cntrDev, cntrPoint, cntrTCF;
    int     deviceOffline;
    
    word    chksum, rcvsum;
    byte    *bfptr;
    float   *fptr;
    float   fdata;
    struct timeval  ctime;
    
    MPU_SOEQ_ENTRY  *rcvEvent;
    ICCP_SOEQ_ENTRY *delEvent;

    // hkkim
    MPU_SOEQ_ENTRY  rcvEventStruct;

    
    opcode = rxbuf[4] & 0x7f;           // 수신 OPCODE
    linkCfg->rcvStatus = rxbuf[5];      // 수신 MPU Status;
    linkCfg->rcvSeqNo  = rxbuf[6];      // 수신 Seqno



    
    /* ------------------------------------ */
    /* SLAVE-MPU 상태에 따른 OPCODE 처리    */
    /* ------------------------------------ */
    if(linkCfg->rcvStatus & LINK_PFR_BIT)           linkCfg->timeSyncReq = SET;
    else if(linkCfg->rcvStatus & LINK_DBCHK_BIT)    linkCfg->chksumReq   = SET;        
        
    /* -------------------------------- */                      
    /* 송신 Frame 구성                      */                      
    /* -------------------------------- */                      
    sndbuf[0] = 0x7e;                       // stx              
    sndbuf[1] = 0x7e;                       // stx              
    sndbuf[2] = 0;                          // size             
    sndbuf[3] = 0;                          // size        
    sndbuf[4] = opcode;                     // opcode          
    sndbuf[5] = linkCfg->sdpStatus;         // status           
    sndbuf[6] = linkCfg->rcvSeqNo;          // 수신 SeqNo       
    count = 7;                                                                                          

    switch(opcode)
    {
    case SIM_SYSTEM_CNTR: 
        Debug(console,"master> rcv CPU-CHHANGE....\n");
        opr->cpuChange = 0;
        
        if(opr->runMode == LOCAL_MASTER)    resetMaster();
        else                                setMaster();
                        
        return (0);
        break;
                
    case SIM_SDP_STATUS:        // 0x01 SDP 장치상태정보 Dump
        
        mpuCFG->rcvMpuSts  = rxbuf[7];       // 수신 MPU Status, 
        mpuCFG->rcvRackSts = rxbuf[8];       // 수신 MPU RACK - Status, 
        mpuCFG->rcvRunSts  = rxbuf[9];       // 수신 MPU 모듈 동작상태- Status, 
        
        if(opr->runMode == LOCAL_SLAVE)
        {
            /* ---------------------------------------- */
            /* MASTER 로부터 계전기 통신상태 Update...  */
            /* ---------------------------------------- */
            deviceOffline = 0;
            for(i=0; i < MAX_DEVICE; i++)       
            {
                if(deviceCFG[i]->scan == 0) continue;
                deviceCFG[i]->online = rxbuf[10 + MAX_HOST + i];                  // 계전기# 통신 운영 상태
                
                if(deviceCFG[i]->online == 0)   deviceOffline++;
            }
            
            if(deviceOffline )  opr->allDevOnline = RESET;
            else                opr->allDevOnline = SET;     
            
            /* ---------------------------------------- */
            /* MASTER 로부터 Device 포인트 정보 Update...  */
            /* ---------------------------------------- */    
            for(i=0; i < MAX_DEV_POINT; i++)    
            {
                devPtBuf[i]->status  = rxbuf[10 + MAX_HOST + MAX_DEVICE + i];     // DEVICE 포인트 운영 상태
                devPtBuf[i]->oldsts  = devPtBuf[i]->status;                         // 이전상태 Update
                
                /* ------------------------------------ */
                /*  ICCP-POINT Update...                */
                /* ------------------------------------ */
                //update_ICCP_info(shmPtr, (POINT_BUF *) &devPtBuf[i], (float) devPtBuf[i]->status, devPtBuf[i]->config);
                update_ICCP_info(shmPtr, (POINT_BUF *) devPtBuf[i], (float) devPtBuf[i]->status, devPtBuf[i]->config);
            }
            
            /* ---------------------------- */
            /* 수신된 Device 포인트 정보... Update */
            /* ---------------------------- */
            update_Device_DNP();
        }
        
        return (0);
        break;

    case SIM_ICCP_DEL_SOE:
        rcvQnum = rxbuf[7];     // 수신 SOE Queue
            
        if(opr->soeDebug)
        Debug(console,"master> rcv ICCP Delete-SOE Dump..... count = %d\n", rcvQnum);

        if(rcvQnum)
        {
            for(i=0; i < rcvQnum; i++)
            {   
                index    = i * sizeof(ICCP_SOEQ_ENTRY);
                delEvent = (ICCP_SOEQ_ENTRY  *) &rxbuf[8 + index];

                /* -------------------------------- */
                /*  ICCP Delete-SOE 정보 저장       */
                /* -------------------------------- */
                iccpShmDeleteSoeEntry (delEvent, 0);
                    
            }
        }
        
        /* -------------------------------- */
        /*  SOE-ACK 전송...                 */
        /* -------------------------------- */
        if(rcvQnum > 0)
        {     
            sndbuf[4] = SIM_ICCP_DEL_ACK + 0x80;   // opcode    
            sndbuf[count++] = 0x01;             // SOE Dump-ACK
            sndbuf[count++] = rcvQnum;          // rcv Q-num
            
            if(opr->soeDebug)
            Debug(console,"master> rcv ICCP Delete-SOE ACK send..... count = %d\n", rcvQnum);
            
        }
        else
        {
            sndbuf[4] = SIM_ICCP_DEL_ACK + 0x80;   // opcode    
            sndbuf[count++] = 0x00;             // SOE Dump-ACK
            sndbuf[count++] = 0x00;             // rcv Q-num
        }            
        break;

    case SIM_ICCP_DEL_ACK:
        rcvAckCode  = rxbuf[7];
        rcvEventNum = rxbuf[8];
        
        if((rcvAckCode == 0x01) && (rcvEventNum == linkCfg->delReport))
        {
            /* ICCP Delete-SOE 전송후 ... rear 포인트 증가 */
            iccpDelSOEQ->rear = (iccpDelSOEQ->rear + linkCfg->delReport ) & 0xff;

            check_ICCP_deleteSOEQ();
            linkCfg->delReport = 0;

            if(opr->soeDebug)   
            Debug(console, "master> rcv ICCP Delete-SOE ACK...count=%d   [front=%d/rear=%d]\n", rcvEventNum, iccpDelSOEQ->front, iccpDelSOEQ->rear);         
        }
        return (0);
        break;        
                
    case SIM_EVENT_DUMP:        // 0x02 SDP EVENT Dump
        if(opr->dualCpuSts != SET)
        {
            sndbuf[4] = SIM_EVENT_ACK + 0x80;   // opcode    
            sndbuf[count++] = 0x00;             // SOE Dump-ACK
            sndbuf[count++] = 0x00;             // rcv Q-num
            break;
        }
        
        if(opr->runMode == LOCAL_SLAVE)
        {
            rcvQnum = rxbuf[7];     // 수신 SOE Queue
            
            /* 수신 SOE 이벤트 수 Check...(MAX 16) */
            if(rcvQnum > 16)
            {
            	if(opr->soeDebug)
            	Debug(console,"master> *** rcv LINK-SOE Event.....Overflow ... count = %d\n", rcvQnum);
            	
            	sndbuf[4] = SIM_EVENT_ACK + 0x80;   // opcode    
                sndbuf[count++] = 0x00;             // SOE Dump-ACK
                sndbuf[count++] = 0x00;             // rcv Q-num
                break;
            }
            	
            if(opr->soeDebug)
            Debug(console,"master> rcv LINK-SOE Event..... count = %d\n", rcvQnum);

            if(rcvQnum)
            {
                for(i=0; i < rcvQnum; i++)
                {   
                    index    = i * sizeof(MPU_SOEQ_ENTRY);
                    rcvEvent = (MPU_SOEQ_ENTRY  *) &rxbuf[8 + index];

#if 1 // hslee
                      


                    /* -------------------------------- */
                    /*  LINK-SOE 정보 저장              */
                    /* -------------------------------- */
                    if(rcvEvent->eventCode == ENT_DEV_ONLINE)           set_Online_Device(shmPtr,rcvEvent);     
                    else if(rcvEvent->eventCode == ENT_DEV_OFFLINE)     set_Offline_Device(shmPtr,rcvEvent);            
                    else if(rcvEvent->eventCode == ENT_SOE)             store_MPU_SOE( shmPtr, rcvEvent);
                    else if(rcvEvent->eventCode == ENT_DEVICE_SOE)      Create_Device_SOE(shmPtr, rcvEvent);
                    else
                    {
                        logEvent_MPU(shmPtr, rcvEvent->eventCode, rcvEvent->devNo, rcvEvent->pointNo, rcvEvent->state, rcvEvent->esioNo, &rcvEvent->updateTime);
                    } 
#else
                     memcpy( &rcvEventStruct, &rxbuf[8 + index], sizeof(rcvEventStruct ) ) ;                     

                    /* -------------------------------- */
                    /*  LINK-SOE 정보 저장              */
                    /* -------------------------------- */
                    if(rcvEvent->eventCode == ENT_DEV_ONLINE)           set_Online_Device(shmPtr,rcvEvent);     
                    else if(rcvEvent->eventCode == ENT_DEV_OFFLINE)     set_Offline_Device(shmPtr,rcvEvent);            
                    else if(rcvEvent->eventCode == ENT_SOE)             store_MPU_SOE( shmPtr, rcvEvent);
                    else if(rcvEvent->eventCode == ENT_DEVICE_SOE)      Create_Device_SOE(shmPtr, rcvEvent);
                    else
                    {
                        logEvent_MPU(shmPtr, rcvEvent->eventCode, rcvEvent->devNo, rcvEvent->pointNo, rcvEvent->state, rcvEvent->esioNo, &rcvEvent->updateTime);
                    } 

#endif                               
                }
            }
        
            /* -------------------------------- */
            /*  SOE-ACK 전송...                 */
            /* -------------------------------- */
            if(rcvQnum > 0)
            {     
                sndbuf[4] = SIM_EVENT_ACK + 0x80;   // opcode    
                sndbuf[count++] = 0x01;             // SOE Dump-ACK
                sndbuf[count++] = rcvQnum;          // rcv Q-num
            }
            else
            {
                sndbuf[4] = SIM_EVENT_ACK + 0x80;   // opcode    
                sndbuf[count++] = 0x00;             // SOE Dump-ACK
                sndbuf[count++] = 0x00;             // rcv Q-num
            }    
        }
        else
        {
                sndbuf[4] = SIM_EVENT_ACK + 0x80;   // opcode    
                sndbuf[count++] = 0x00;             // SOE Dump-ACK
                sndbuf[count++] = 0x00;             // rcv Q-num
        }      
        break;
        
    case SIM_EVENT_ACK:         // 0x03 SDP EVENT-ACK
        rcvAckCode  = rxbuf[7];
        rcvEventNum = rxbuf[8];
        
        if(rcvAckCode == 0x01)
        {
            if(opr->soeDebug)   
            Debug(console, "master> rcv LINK-SOE ACK...count=%d   [front=%d/rear=%d]\n", rcvEventNum, linkCfg->front, linkCfg->rear);         
        }
        else if(rcvAckCode == 0x02)
        {
            if(opr->soeDebug)
		    Debug(console,"master> rcv POINT-CONTROL ACK ... dev=%d, point=%d, tcf=%d \n", linkCfg->cntDev, linkCfg->cntPoint, linkCfg->cntTCF);
		    
            /* ------------------------ */
            /* 제어정보 Clear...        */  
            /* ------------------------ */          
            linkCfg->cntDev     = 0;
            linkCfg->cntPoint   = 0;
            linkCfg->cntTCF     = 0;
            linkCfg->cntFlag    = 0;
            linkCfg->cntHost    = 0;
            opr->cntrLinkPass   = 0;
        }
        
#if 0
        if((rcvAckCode == 0x01) && (rcvEventNum == linkCfg->reportSOE))
        {
            /* SOE 전송후 ... rear 포인트 증가 */
            linkCfg->rear = (linkCfg->rear + rcvEventNum) & 0xff;        

            //if(opr->linkDebug)   
            Debug(console, "master> rcv LINK-SOE ACK...count=%d   [front=%d/rear=%d]\n", rcvEventNum, linkCfg->front, linkCfg->rear);         
        }
        
        if(rcvAckCode == 0x02)
        {
            //if(opr->linkDebug)
		    Debug(console,"master> rcv POINT-CONTROL ACK ... dev=%d, point=%d, tcf=%d \n", linkCfg->cntDev, linkCfg->cntPoint, linkCfg->cntTCF);
		    
            /* ------------------------ */
            /* 제어정보 Clear...        */  
            /* ------------------------ */          
            linkCfg->cntDev     = 0;
            linkCfg->cntPoint   = 0;
            linkCfg->cntTCF     = 0;
            linkCfg->cntFlag    = 0;
            linkCfg->cntHost    = 0;
            opr->cntrLinkPass   = 0;
        }
#endif
        
        return (0);
        break;
            
    case SIM_POINT_SCAN:        // 0x04 계전기 POINT 정보 Scan
        devid      = rxbuf[7] - 1;      // 계전기 번호     
        rcvStsByte = rxbuf[8];          // 데이터 사이즈 (Byte 수)     
        
        if(devid < 0)	
        {
        	Debug(console, "master> *Invalid POINT-SCAN devid = %d\n", devid);
        	break;
        }
        
        /* -------------------------------------------------------------------- */
        /*  LOCAL_SLAVE 모드 : Master CPU로부터 계전기별 상태데이터 Update...   */
        /*  LOCAL_MASTER모드 : Slave CPU에서 Master로 계전기별 상태데이터 전송  */
        /* -------------------------------------------------------------------- */
        if((opr->runMode == LOCAL_SLAVE) && (devid < MAX_DEVICE))
        {    
            /* ---------------------------------------- */
            /*  계전기별 상태데이터 Update...           */
            /* ---------------------------------------- */
            if(rcvStsByte > 0)  curStore_BIT_MPU( shmPtr, devid, &rxbuf[9], rcvStsByte);
          
            if(opr->linkDebug) 
            {
                Debug(console, "master>  : rcv STS(%02d) Data Updated...size= %2d \n", devid+1, rcvStsByte);
                DumpBuff(console," ", &rxbuf[9], rcvStsByte);
            }
                
            sndbuf[7] = devid + 1;           // 계전기 번호     
            sndbuf[8] = rcvStsByte;          // 데이터 사이즈 (Byte 수)     
            count = 9;
        }
        return (0);
        break;
        
    case SIM_ANALOG_SCAN:       // 0x14 계전기 아날로그 정보 Scan
       
        devid      = rxbuf[7] - 1;      // 계전기 번호     
        dumpCount  = rxbuf[8];          // 포인트 수     
        
        if(devid < 0)	
        {
        	Debug(console, "master> *Invalid ANALOG-SCAN devid = %d\n", devid);
        	break;
        }
        
        /* -------------------------------------------------------------------- */
        /*  LOCAL_SLAVE 모드 : Master CPU로부터 계전기별 계측데이터 Update...   */
        /*  LOCAL_MASTER모드 : Slave CPU에서 Master로 계전기별 계측데이터 전송  */
        /* -------------------------------------------------------------------- */
        if((opr->runMode == LOCAL_SLAVE) && (devid < MAX_DEVICE))
        {    
            bfptr = (byte *) &rxbuf[9];         // 계측데이터 시작지점...
            index = 0;
            
            /* ---------------------------------------- */
            /*  수신된 포인트 수만큼... Update...       */
            /* ---------------------------------------- */
            for(i=0; i< dumpCount; i++)
            {
                index = i * 6;
                anapoint = bfptr[index + 0] * 256 + bfptr[index + 1];   // 계측포인트 번호 (0,1,2,...)

#if 0  // hkkim            
                fptr = (float *) &bfptr[index + 2];                     // 계측 데이터(Float)                                      
                /* 기본 데이터 지정 */
                fdata = (float ) *fptr;

#else // hkkim

                  // in master    memcpy( &sndbuf[count], &aiPoint->floatData, 4);    // 계측 데이터(Float)

                  // 이렇게 해주면 fdata는 little endia 이고..
                   memcpy( &fdata, &bfptr[index + 2], 4);         
                   fptr = &fdata; 
                    
                                
            //    fptr = (float *) &bfptr[index + 2];                     // 계측 데이터(Float)                                      
            /* 기본 데이터 지정 */
            //    fdata = (float ) *fptr;
            //    fdata = 32.2;
            //     fptr = &fdata; 
#endif




                /* ---------------------------------------- */
                /* 2014.12.11 수신된 POINT 예외처리...      */
                /* ---------------------------------------- */
                if(anapoint >= MAX_DEV_AI_POINT)
                {
                    Debug(console,"master> rcv dev=%2d, *Analog Dump - *Invalid AI-Point (%d) \n", devid+1, anapoint);
                    break;
                }
                
                /* ------------------------------------ */               
                /* 계측포인트 데이터.. Update ...       */
                /* ------------------------------------ */      
                storeAI_FLOAT_MPU( shmPtr, devid, anapoint, fdata, (byte *)fptr);
            }
            
            if(opr->linkDebug) Debug(console, "master> rcv  ANA(%02d) Data Updated.(%f)..!\n", devid+1,fdata);
                
            sndbuf[7] = devid + 1;           // 계전기 번호     
            sndbuf[8] = dumpCount;           // 포인트 수 
            count = 9;                    
        }
        return (0);
        break;
    
    case SIM_CHKSUM_DUMP:       // 0x16 데이터베이스 Chksum Scan       
        rcvsum = rxbuf[7]*256 + rxbuf[8];
        chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);

        /* MASTER 재기동 Bit Clear... */        
        linkCfg->sdpStatus &= (~LINK_PFR_BIT);      
        
		linkCfg->chksumReq = RESET;
		//linkCfg->dbWriteDelay = RESET;      // delay off
		
		if(chksum != rcvsum)
		{
			Debug(console,"master> *** SLAVE Check DB Failed....!\n");
			Debug(console,"master> rcv CHKSUM  : rcv = %4x rom = %4x .... [%04d-%02d%02d-%02d:%02d:%02d] \n", rcvsum, chksum, rtc->year, rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
			
			linkCfg->mpuCfgDown     = SET;
			linkCfg->esioCfgDown    = SET;
			linkCfg->dbDownEsio     = 1;
			
			linkCfg->calptCfgDown   = SET;               // CAL-POINT Config 전송요구
            linkCfg->dbDownCalpt    = 1;    
        
			linkCfg->hostCfgDown    = SET;
			linkCfg->iccpCfgDown    = SET;
			linkCfg->harrisCfgDown  = SET;
			linkCfg->landisCfgDown  = SET;
			linkCfg->modbusCfgDown  = SET;
			linkCfg->scanCfgDown    = SET;
			linkCfg->deviceCfgDown  = SET;
			linkCfg->dbDownDevice   = 1;
			
			linkCfg->pointCfgDown   = SET;
			linkCfg->dbDownPoint    = 1;
			linkCfg->dbDownIndex    = 16;

			linkCfg->endPointFlag   = RESET;
			
			linkCfg->dbDownFlag     = SET;
			
		}
		return (0);
        break;
        
    case SIM_POINT_CNTR:        // 0x05 계전기 POINT 제어
        
        cntrDev     = rxbuf[7];             // 대상 계전기 번호 [1..64]
        cntrPoint   = rxbuf[8];             // 대상 포인트 [1..256]
        cntrTCF     = rxbuf[9];             // 제어상태, [1] TRIP, [2] CLOSE
            
        if(opr->soeDebug)
        Debug(console,"master> rcv Slave Control : dev=%d, point=%d, tcf=%2x\n",  cntrDev,cntrPoint, cntrTCF);

        /* ------------------------------------ */
        /* ESIO 제어정보 연계...                */
        /* ------------------------------------ */
        gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
        logEvent_MPU(shmPtr, ENT_LINK_CNTR, cntrDev, cntrPoint, cntrTCF, opr->cpuMode, &ctime);
            
        controlInfo_MPU( shmPtr, cntrDev, cntrPoint, cntrTCF, PASS_LINK_CNTR, &ctime);
            
        sndbuf[4] = SIM_EVENT_ACK + 0x80;   // opcode    
        sndbuf[count++] = 0x02;             // Control-ACK
        sndbuf[count++] = 0;                // rcv Q-num
        break;
        
    case SIM_TIME_DOWN:         // 0x11 Time Sync Down
        linkCfg->timeSyncReq = RESET;
        if(opr->linkDebug)
        Debug(console, "master> rcv Time Sync...! \n" );
        return (0);
        break;

    case SIM_TIME_UP:         // 0x11 Time Sync Down
        opr->year 	= rxbuf[7]*256 + rxbuf[8];
    	opr->month 	= rxbuf[9];
    	opr->day 	= rxbuf[10];
    	opr->hour	= rxbuf[11];
    	opr->min	= rxbuf[12];
    	opr->sec	= rxbuf[13];
    	opr->milisec = 0;

    	//if(opr->wdtDebug)
    	Debug(console, "master> rcv TIME Sync Down (from Slave) : %4d/%2d/%2d %02d:%02d:%02d \n",opr->year, opr->month, opr->day, opr->hour, opr->min, opr->sec);

		opr->rtcUpdateFlag = SET;
		
		/* TIME-SYNC 관련 이벤트 생성 */    	
    	logEvent_MPU(shmPtr, ENT_LOCAL_TIME, 0, 0, 0, opr->cpuMode, NULL);  
    	
        //opr->getClockNTP = SET; 
        //if(opr->wdtDebug)   
        //Debug(console, "master> GET NTP-Server .... TIME req...! \n");  
        
        sndbuf[4] = SIM_EVENT_ACK + 0x80;   // opcode    
        sndbuf[count++] = 0x11;             // TIME-SYNC UPLOAD -ACK
        sndbuf[count++] = 0;                
        break;
                
    case SIM_MPUCFG_DOWN:       // 0x20 DB : MPU Config Down
        linkCfg->mpuCfgDown = RESET;
        if(opr->linkDebug)
		Debug(console,"master> rcv MPU Config ...\n");
        return (0);
        break;
        
    case SIM_ESIOCFG_DOWN:      // 0x22 DB : ESIO Config Down
        devNo  = rxbuf[7];          // ESIO # [1..5]
        devNum = rxbuf[8];          // 전송 ESIO 갯수 (default = 1)
        
        if(opr->linkDebug)
		Debug(console,"master> rcv ESIO (%d) Config ...\n", linkCfg->dbDownEsio);
		
        linkCfg->dbDownEsio = devNo + 1;
        if((linkCfg->dbDownEsio > MAX_DB_ESIO) || (linkCfg->endEsioFlag == SET))
        {       
            linkCfg->esioCfgDown = RESET;
        }
        
        return (0);
        break;
        
    case SIM_HOST_DOWN:         // 0x24 DB : HOST Config Down
        linkCfg->hostCfgDown = RESET;
        
        if(opr->linkDebug)
		Debug(console,"master> rcv HOST Config ... \n");
        return (0);
        break;
        
    case SIM_ICCP_DOWN:         // 0x26 DB : ICCP-HOST Config Down
        linkCfg->iccpCfgDown = RESET;
        if(opr->linkDebug)
		Debug(console,"master> rcv ICCP Config ... \n");
        return (0);
        break;
        
    case SIM_HARRIS_DOWN:       // 0x28 DB : HARRIS Config Down
        linkCfg->harrisCfgDown = RESET;
        if(opr->linkDebug)
		Debug(console,"master> rcv HARRIS Config ...\n");
        return (0);
        break;
        
    case SIM_LANDIS_DOWN:       // 0x2A DB : LANDIS Config Down
        linkCfg->landisCfgDown = RESET;
        if(opr->linkDebug)
		Debug(console,"master> rcv LANDIS Config ... \n");
        return (0);
        break;
        
    case SIM_MODBUS_DOWN:       // 0x2C DB : MODBUS Config Down
        linkCfg->modbusCfgDown = RESET;
        if(opr->linkDebug)
		Debug(console,"master> rcv MODBUS Config ... \n");
        return (0);
        break;
        
    case SIM_SCAN_DOWN:         // 0x30 DB : SCAN Config Down
        linkCfg->scanCfgDown = RESET;
        if(opr->linkDebug)
		Debug(console,"master> rcv SCAN Config ... \n");
        return (0);
        break;
        
    case SIM_DEVICE_DOWN:       // 0x32 DB : DEVICE Config Down
        devNo  = rxbuf[7];      // 시작 계번기 번호 [1..64]
        devNum = rxbuf[8];      // 전송 계전기 수
        
        if(opr->linkDebug)
		Debug(console,"master> rcv DEVICE (st=%d, num=%d) Config ...\n", devNo, devNum);
		
        linkCfg->dbDownDevice = devNo + devNum;
        
        if((linkCfg->dbDownDevice >= MAX_DB_DEVICE) || (linkCfg->endDeviceFlag == SET))
        {       
            linkCfg->deviceCfgDown = RESET;
        }
        return (0);
        break;
        
    case SIM_POINT_DOWN:        // 0x34 DB : POINT Config Down
        startPoint = rxbuf[7]*256 + rxbuf[8];
		
		if(opr->linkDebug)
        Debug(console,"master> rcv POINT-DB ...start=%4d, num=%d \n", startPoint, rxbuf[9]);
        
        // 여기서 보낸 것을 update 하네..
        // 마스터는 dbDownPoint 가 1번부터 ?
		linkCfg->dbDownPoint = startPoint + linkCfg->dbDownIndex;

        /* ---------------------------------------- */
		/* 포인트 DB 전송 종료...                   */	
		/* ---------------------------------------- */	
		if((linkCfg->dbDownPoint >= MAX_DB_POINT) || (linkCfg->endPointFlag == 0x80))    
		{	
			linkCfg->pointCfgDown = RESET; // 0 인데..
			linkCfg->chksumReq    = SET;
			linkCfg->dbDownFlag   = RESET;
        
            //linkCfg->dbWriteDelay = SET;
            			
			Debug(console,"master> **** master DB-DOWN End...!\n");
		}
        return (0);
        break;
   
   case SIM_CAL_POINT_DOWN:        // 0x34 DB : POINT Config Down
        startPoint = rxbuf[7]*256 + rxbuf[8];
		
		if(opr->linkDebug)
        Debug(console,"master> rcv CAL POINT-DB ...start=%4d, num=%d \n", startPoint, rxbuf[9]);
        
		linkCfg->dbDownCalpt = startPoint + 16;         // 16 포인트씩 전송

        /* ---------------------------------------- */
		/* 포인트 DB 전송 종료...                   */	
		/* ---------------------------------------- */	
		if(linkCfg->dbDownCalpt >= MAX_DB_CAL_POINT)
		{	
			linkCfg->calptCfgDown = RESET;
			linkCfg->dbDownCalpt  = RESET;
		}
        return (0);
        break;
             
	default :
		break;	
	}


    /* -------------------------------- */                      
    /* 송신 Frame 구성                      */                      
    /* -------------------------------- */         	
    sndCount = count + 1;
                     
    sndbuf[2] = (sndCount >> 8) & 0xff;     /* Total size: MSB */
    sndbuf[3] = sndCount & 0xff;            /* Total size: LSB */
    sndbuf[count] = genlrc(sndbuf, count);
    count++;
    
    return(count);
    
    
}

/*   
*   LINK-MASTER : Client 접속 초기화
*/
int _initNetClient_LINK( )
{
    int     flag = 1;
    
    MPU_NET_ENTRY   *slaveNet1;
    
    /* Socket 종료후 재 초기화 */
	if(linkCfg->SocketFd != 0)
	{
		if(opr->linkDebug)
		Debug(console,"master> *** LINK: Alread Socket (%d) used... Socket Close \n", linkCfg->SocketFd);	
		close( linkCfg->SocketFd);  
  	}
  	              
    /* -------------------------------------------------- */
    /* 소켓의 Open : IPv4 기반, 연결지향형 소켓, TCP기반  */
    /* -------------------------------------------------- */
	if ((linkCfg->SocketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == ERROR)
	{
	    if(opr->linkDebug)
        Debug(console,"master> *** LINK: [NET] Socket Open Failed...\n");	
		return(ERROR);
	}
	
    if(opr->linkDebug)
    {
        Debug(console,"------------------------------------------ \n");
        Debug(console,"master> LINK: Socket Open OK ...%d \n", linkCfg->SocketFd);
        Debug(console,"------------------------------------------ \n");
    }
    
    /* ------------------------------------------------ */
    /*  SOCKET Option...                                */
    /* ------------------------------------------------ */
    setsockopt(linkCfg->SocketFd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(int));
    setsockopt(linkCfg->SocketFd, SOL_SOCKET, SO_KEEPALIVE, (char *)&flag, sizeof(int));
    
#if 1
    setsockopt(linkCfg->SocketFd, SOL_SOCKET, SO_DONTROUTE, (char *)&flag, sizeof(int));
    ioctl(linkCfg->SocketFd, FIONBIO, (int)&flag);
#endif
    
    /* ------------------------------------------------ */
    /* NON-BLOCK 설정 */
    /* ------------------------------------------------ */
    flag = fcntl( linkCfg->SocketFd, F_GETFL,0);
    fcntl( linkCfg->SocketFd, F_SETFL, (flag | O_NONBLOCK));
        
    /* ---------------------------------------- */
    /*	LINK Network => NET#2 사용				*/
    /* ---------------------------------------- */
    slaveNet1 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[0];
    //slaveNet1 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[1];
    	
    /* ---------------------------------------- */
    /* 소켓 구조체 초기화 ...                   */
    /* ---------------------------------------- */
	linkCfg->serv_addr.sin_family = AF_INET;                            /* IPv4 기반의 인테넷 프로토콜 Family */
	linkCfg->serv_addr.sin_port   = htons(LINK_NET_PORT);               /* Port   정보의 Network Byte Order 로 변경함. */
	linkCfg->serv_addr.sin_addr.s_addr = inet_addr(slaveNet1->ipAddr);	/* IP conversion & assign */;
	
	if(opr->linkDebug)  Debug(console, "master> LINK: TARGET : %s\n", slaveNet1->ipAddr);
	
	return (0);
}



/*
*   LINK-MASTER : Client 접속
*/
int _reConnectClient_LINK()
{
    MPU_NET_ENTRY   *slaveNet1;
    
    /* ---------------------------------------- */
    /*	LINK Network => NET#2 사용				*/
    /* ---------------------------------------- */
    slaveNet1 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[0];
    //slaveNet1 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[1];
    
    /* Image - Server Connection... */
    if(connect(linkCfg->SocketFd, (struct sockaddr *) &linkCfg->serv_addr, sizeof(linkCfg->serv_addr)) == -1)
    {
        if(opr->linkDebug)    
        Debug(console,"master> ***IP=%s, PORT=%d  connect Error...!\n", slaveNet1->ipAddr, LINK_NET_PORT);	
        
        linkCfg->connectStatus = 0;
        //close( linkCfg->SocketFd);
    }
    else
    {
        if(opr->linkDebug)    
        Debug(console,"master> IP=%s, PORT=%d connect OK... \n", slaveNet1->ipAddr, LINK_NET_PORT);	 
        
        linkCfg->connectStatus = SET;
        linkCfg->comFailTick = 0;
    }
            
	return(linkCfg->SocketFd);
}

/* -------------------------------------------------------- */
/*  return :  1   get vaild response                        */
/*  	      0   TCP Connection closed                     */
/*           -1  Timeout                                    */
/*           -2  select system error                        */
/* -------------------------------------------------------- */
int  get_responseLINK(int SocketFd, byte	*rxbuf)
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
		if((result = TKreadn(SocketFd, (char *) &rxbuf[0], 1, 20)) < 0)
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
	if((result =TKreadn(SocketFd, (char *) &rxbuf[1], 3, 20)) < 0)	return (-1);
	
	if(rxbuf[1] != 0x7E)	return (-1);		    
    
    reqSize = (rxbuf[2]*256 + rxbuf[3]) - 4;   /* except stx, size */ 

    /* read BODY */
    if((result =TKreadn(SocketFd, (char *) &rxbuf[4], reqSize, 20)) != reqSize)	
    {
        if(opr->linkDebug)
        Debug(console, "link> * Tail not Received... req=%d, rcv=%d\n", reqSize, result);     
        return (-1);
    }

    /* receive body */
    rxcnt = reqSize + 4;
    lrc = genlrc(rxbuf, rxcnt);
        
    if(lrc != 0)   
    {
        if(opr->linkDebug)    
        {
            Debug(console, "link> *LRC Error ...cal=%2x, rcv=%2x!\n", lrc, rxbuf[rxcnt-1]);
            DumpBuff(console,"*LRC: ", rxbuf, rxcnt);
        }                
        return(-1);
    }
    
    return (rxcnt);
}



/* ======================================================== */
/*   MASTER-CPU Main()                                      */
/* ======================================================== */
int linkMaster()
{
	int		oldsec;
	int     rxVal;
	int     runTick = 0;
	int		sendFlag;
	int     reConnectCount=0;
    //int     checkTick = 0;
    
    byte	*rxbuf, *txbuf;
	int		rxcnt, txcnt;
	char	buffer[128];
		
    printf(" %s LINK-MASTER PROCESS Activated ... !\n", TARGET_NAME);
 
 	rxbuf = (byte *) &linkRxbuf[0];
	txbuf = (byte *) &linkTxbuf[0];
	
    linkCfg->comFailTick = 0;
    linkCfg->chksumReq   = SET;
    linkCfg->timeSyncReq = SET;
    linkCfg->online      = 2;
    
    linkCfg->sdpStatus   = LINK_PFR_BIT;

    /* ------------------------------------ */
    /*  CPU 링크상태 확인....               */
    /* ------------------------------------ */
	opr->chkLinkOK   = SET;		
	//opr->initLinkSts = RESET;		// LINK 초기 접속 상태
	//opr->initLinkCount= 0;
	
    /* ------------------------------------ */
    /*  MASTER CLIENT initial...            */
    /* ------------------------------------ */
   	if (_initNetClient_LINK() == ERROR)
    { 
        printf("master> *** LINK: Network Initial... Fail...\n");	
        termExec = NO;
    }
    		
	oldsec  = rtc->sec;
	
	sendFlag = 0;
	
	while(termExec)
	{
        /* ---------------------------------------- */
	    /*  LINK 접속실패에 따른 재접속...          */
	    /* ---------------------------------------- */
	    while(linkCfg->connectStatus == RESET)
	    {
	        taskPtr->wdtCount = 0;
        
            if(opr->linkDebug)  Debug(console,"master> * LINK retry Connect....!\n");
                
            _reConnectClient_LINK();  

            pause(500);
            checkDualCPU(0); 
            
            /* -------------------------------------------- */
            /* LINK OFFLINE 표시                            */
            /* -------------------------------------------- */
            if(linkCfg->online == SET)    
            {
            	/* LOG File 저장 */
   				sprintf(buffer, "%s", ">> *** LINK OFFLINE ...!");
   				LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    				
                logEvent_MPU(shmPtr, ENT_LINK_OFFLINE, 0, 0, 0, opr->cpuMode, NULL);  
            }
	        
	        linkCfg->online = RESET;
			mpuCFG->rcvRunSts = 0;   
	        
            if(++reConnectCount > 5)
            {
                reConnectCount = 0;
                
                if(opr->linkDebug)  Debug(console,"master> * LINK: RE Initial...CLIENT....!\n");

                /* ---------------------------------------- */
                /*  장치별 TCPIP 초기화...                  */
                /* ---------------------------------------- */
                if (_initNetClient_LINK() == ERROR)
                { 
                    Debug(console,"master> *** LINK: Target No Response Re-Initial... Fail...\n");	
                }
            }

#if 0            
            /* -------------------------------------------- */
            /* CPU 이중화.... LINK 이상시 Default 구동 지정			*/
            /* -------------------------------------------- */
            if(opr->initLinkSts == SET)
           	{
            	if((opr->dualCpuSts == SET) && (opr->runMode == LOCAL_SLAVE) && (scuCfg->online == SET) && (scuCfg->remoteMode == AUTO_MODE))
          		{
          			if(opr->linkDebug)
          			Debug(console,"master> DUAL CPU...SLAVE Mode ...RESET sts %d\n", opr->initLinkCount);
          			if(++opr->initLinkCount > 5)	
          			{
          				setMaster();
          				opr->initLinkCount = 0;
          			}			
          		}
         	} 	  	
#endif
         	
	    }
	    
	    linkCfg->comFailTick = 0;
	    
	    /* ---------------------------------------- */
	    /*  LINK 접속성공시 ...          				*/
	    /* ---------------------------------------- */
	    while(linkCfg->connectStatus)
        {
            taskPtr->wdtCount = 0;
            pause(100);

			// TEST...
			if(opr->testWDTFlag)
			{
				if(opr->testWDTFlag == 1)	setMaster();
				if(opr->testWDTFlag == 2)	resetMaster();	
				opr->testWDTFlag = 0;	
			}
		            
            /* ------------------------------------ */
            /* 주기적으로 CPU 장착상태를 Check...   			*/
            /*	- SCU 스위치 상태 변화시...				*/
            /* ------------------------------------ */
	        if(linkCfg->cmdFlag == SET) checkDualCPU(1);    
            else                        
            {
                if(++runTick & 0x01)  checkDualCPU(0);    
            }
                            
    	    /* ------------------------------------- */
            /*  메세지 수신 & Response               */
    	    /* ------------------------------------- */	        	        
            if((rxcnt = get_responseLINK(linkCfg->SocketFd, rxbuf)) > 0)
            {
                linkCfg->comFailTick = 0;
	    
                /* -------------------------------------------- */
                /*  수신한 데이터에 대한 패켓 처리 ...          */    
                /* -------------------------------------------- */
                if(opr->linkDebug)
                {         
                    if(rxcnt > 256) DumpBuff(console,"LRX: ", rxbuf, 256);
                    else            DumpBuff(console,"LRX: ", rxbuf, rxcnt);
                }        

                txcnt = rcvHandler_MASTER(txbuf, rxbuf, rxcnt);
                if(txcnt > 7)   sendFlag = SET; 
                
                linkCfg->sndSeqNo++;
            
                /* -------------------------------------------- */
                /* LINK ONLINE 표시                             */
                /* -------------------------------------------- */
                if(linkCfg->online != SET)    
                {
                	/* LOG File 저장 */
    				sprintf(buffer, "%s", ">> LINK ONLINE ...!");
    				LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
                    logEvent_MPU(shmPtr, ENT_LINK_ONLINE, 0, 0, 0, opr->cpuMode, NULL); 
                    
                    /* Online 시 TIME-SYNC 	*/
                    if(opr->runMode == LOCAL_MASTER)	linkCfg->timeSyncReq = SET;
                }
	            linkCfg->online   = SET;
	            //opr->initLinkSts  = SET;		// LINK 초기접속 OK
	            //opr->initLinkCount= 0;
	               	
    	    }
    	    else
	        {
    	        if(rxcnt == 0)
    	        {
    	        	if(opr->linkDebug)  Debug(console, "master> *TCP/IP read ...0 Connection...Failed...!\n");
	            
	                /* CLIENT 종료후 재 접속 */
	                linkCfg->connectStatus = RESET;
	            }
	        
    	        /* -------------------------------------------- */
                /*  CPU 운영모드 : MASTER 모드                  */
                /* -------------------------------------------- */
                txcnt = makeLink_MASTER(txbuf);
                if(txcnt > 7)   sendFlag = SET;         
	        }

            /* ------------------------------------------------ */
            /*  SLAVE 송출데이터에 대한 패켓 처리 ...           */    
            /* ------------------------------------------------ */
            if(sendFlag == SET)
            {
                /* -------------------------------------------- */
                /* 전송메세지에 대한 출력                       */  
                /* -------------------------------------------- */     
                if(txcnt >= 5) 
                {
                    if(opr->linkDebug) 
                    {
                        if(txcnt > 256)     DumpBuff(console,"LTX: ", txbuf, 256);
                        else                DumpBuff(console,"LTX: ", txbuf, txcnt);    
                    }
                          
                    /* -------------------------------- */
                    /* Network Response...              */
                    /* -------------------------------- */    
                    if((rxVal = tkWriteTCP( linkCfg->SocketFd ,(byte *) txbuf, txcnt, 1000)) < 0 )
            	    {
                        if(opr->linkDebug)  Debug(console, "master> * *** net Send Error [%d %d]...socket = %d !\n", txcnt,  rxVal, linkCfg->SocketFd);		
	                }

                    if( rxVal != txcnt)
                    {
                        if(opr->linkDebug)  Debug(console, "master> * net Send Error [%d %d]...!\n", txcnt,  rxVal);
                    
                        /* CLIENT 종료후 재 접속 */
	                    linkCfg->connectStatus = RESET;
                    }                
                }
                
                if(linkCfg->chksumReq == SET)  pause(300);    
            }

            /* ------------------------------------ */
            /*  매초단위 Function ...               */
            /* ------------------------------------ */
            if(oldsec == rtc->sec)  continue;
            oldsec = rtc->sec;
        
            /* ------------------------------------ */
            /*  ICCP Delete SOE Check...            */
            /* ------------------------------------ */
            check_ICCP_deleteSOEQ();
            
            /* ------------------------------------ */
            /*  DB Check 주기 : 5분단위             */
            /* ------------------------------------ */
            if(((rtc->min % 5) == 0) && (oldsec == 0)) 
            {
                if(linkCfg->dbDownFlag == RESET)  linkCfg->chksumReq = SET;       // 1분단위 DB Check...    
            }     


            /* ------------------------------------ */
            /*  이중화 CPU 상태 Check...            */
            /*  - 양쪽다 MASTER => A-Master, B-Slave*/
            /*  - 양쪽다 SLAVE  => A-Master, B-Slave*/
            /* ------------------------------------ */
#if 0            
            if(opr->dualCpuSts == SET)
			{            	
	            if((opr->runMode == LOCAL_MASTER) && (scuCfg->remoteMode == AUTO_MODE))
    	        {       		
        	        if(mpuCFG->rcvMpuSts & MPU_ACT_BIT)
            	    {
                	    if(++checkTick > 3)
	                    {
    	                    checkTick = 0;      
        	                Debug(console, "==> rcvsts=%02x, MPU-A : Local MASTER ... MPU-B Slave...!\n", mpuCFG->rcvMpuSts);
            	            setMaster();
                	    }
	                }
    	            else    checkTick = 0;   
        	    }         
            	else if((opr->runMode == LOCAL_SLAVE) && (scuCfg->remoteMode == AUTO_MODE))
	            {
    	            if((mpuCFG->rcvMpuSts & MPU_ACT_BIT) == 0)
        	        {
            	        if(++checkTick > 3)
                	    {
                    	    checkTick = 0;      
	                        Debug(console, "==> rcvsts=%02x, MPU-A : Local SLAVE ... MPU-B Slave...!\n", mpuCFG->rcvMpuSts);
    	                    setMaster();
        	            }
            	    }
                	else    checkTick = 0;  
            	}
			}

#endif
			
	    	/* 콘솔종료시.... RESET-Master */
    		if(opr->userReset == SET)
	        {
	            resetMaster();
	            Debug(console,"link> **** USER Reset.... RESET Master !\n"); 
	            linkCfg->connectStatus = RESET;
    	        termExec = 0;       
	        }
	    
            /* ------------------------------------ */
            /*  LINK Timeout 처리...                */
            /* ------------------------------------ */
   	    	if(++linkCfg->comFailTick > 5)
   		    {
   		        if(opr->linkDebug)  Debug(console,"master> *** LINK Time Out ...Fail...\n");	
   		        
   		        mpuCFG->rcvRunSts = 0;
   		            
	            /* CLIENT 종료후 재 접속 */
                linkCfg->connectStatus = RESET;
       	    }
       	    		
       	 }
        
	}/* while */
	
	mpuCFG->rcvRunSts = 0;
	
	printf("LINK: MASTER ..... CLEAR....%02x !\n", linkCfg->cpuControl);
	resetMaster();

    return (0);
}
