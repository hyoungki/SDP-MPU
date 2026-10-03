#include	"localLib.h"
#include    "external.h"
#include    "extfunc.h"

extern  SHM_MEMORY	    *shmPtr;
extern	int             termExec;

extern	TTY_DESC 	linkPort;
extern	TASK_INFO	*taskPtr;

extern  PDPDTIME_DATE_TIME      dnpRTC;
extern  PDPDTIME_MS_SINCE_70    dnpRTCInfo;

extern  ICCP_SOE_DELETE_QUEUE   *iccpDelSOEQ;     // ICCP-HOST Delete SOEQ

static  byte    slvRxbuf[4096];
static  byte    slvTxbuf[4096];

extern  int     get_responseLINK(int SocketFd, byte	*rxbuf);
extern  int	    checkDualCPU(int mode);
extern  void    resetMaster();
extern  void    setMaster();

extern  int     check_ICCP_deleteSOEQ();
extern  int     update_Device_DNP();


//extern  void setMaster();

/*******************************************************************************
 *                                                                        
 * 모듈명:	_initNetServer_LINK()
 *                                                                        
 ******************************************************************************/
int _initNetServer_LINK(void)
{
	int		flag = 1, length;
	//char	address[64];
	//int		port;
    
    MPU_NET_ENTRY   *masterNet1;
    
    /* Socket 종료후 재 초기화 */
	if(linkCfg->SocketFd != 0)
	{
		if(opr->linkDebug)
		Debug(console,"slave> *** LINK: Alread Socket (%d) used... Socket Close \n", linkCfg->SocketFd);	
		close( linkCfg->SocketFd);  
  	}
  	
    /* ------------------------------------------------------------ */
    /*  SERVER-소켓의 Open : IPv4 기반, 연결지향형 소켓, TCP기반    */
    /* ------------------------------------------------------------ */
	if ((linkCfg->SocketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == ERROR)
	{
	    if(opr->linkDebug)
        Debug(console,"slave> *** [NET] Socket Open Failed...\n");	
		return(ERROR);
	}
	
    if(opr->linkDebug)
    {
        Debug(console,"------------------------------------------ \n");
        Debug(console,"slave> socket Open OK ...%d \n", linkCfg->SocketFd);
    }

    /* ---------------------------------------- */
    /*  SOCKET Option...                        */
    /* ---------------------------------------- */
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
    masterNet1 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[0];
    //masterNet1 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[1];
    
    /* ---------------------------------------- */
    /* 소켓 구조체 초기화 ...                   */
    /* ---------------------------------------- */
    length = sizeof(struct sockaddr_in);
	bzero((char *)&linkCfg->serv_addr, length);
	
	linkCfg->serv_addr.sin_family = AF_INET;                            /* IPv4 기반의 인테넷 프로토콜 Family */
	linkCfg->serv_addr.sin_port   = htons(LINK_NET_PORT);               /* Port   정보의 Network Byte Order 로 변경함. */
	linkCfg->serv_addr.sin_addr.s_addr = inet_addr(masterNet1->ipAddr);	/* IP conversion & assign */;
	linkCfg->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);             /* IP주소 정보의 Network Byte Order 로 변경함. */
	
	if(opr->linkDebug)  Debug(console, "slave> TARGET : %s\n", masterNet1->ipAddr);
	
    /* ------------------------------------------------ */    
    /*  소켓에 주소를 할당함...                         */
    /* ------------------------------------------------ */    
    length = sizeof(struct sockaddr_in);
	if (bind(linkCfg->SocketFd, ( struct sockaddr *)&linkCfg->serv_addr, length) == ERROR)
	{
	    if(opr->linkDebug) Debug(console,"slave> ***[NET] bind 실패 \n");
		return(ERROR);
	}
	if(opr->linkDebug) Debug(console,"slave> [NET] bind OK \n");

    /* ------------------------------------------------ */    
    /*  연결요청 대기상태로 진입...                     */
    /* ------------------------------------------------ */    
	if (listen(linkCfg->SocketFd, 5) == ERROR)
	{
	    if(opr->linkDebug)	Debug(console,"slave> ***[NET] listen 실패 \n");
		return(ERROR);
	}
    
    return(linkCfg->SocketFd);
    
#if 0
    if(opr->linkDebug)	Debug(console,"slave> [NET] listen OK \n");

    /* ------------------------------------------------ */    
    /*  ACCEPT() 진입...                                */
    /* ------------------------------------------------ */    
	length = sizeof(struct sockaddr_in);
    if ((linkCfg->SocketID = accept(linkCfg->SocketFd, ( struct sockaddr *)&linkCfg->clientPtr, (socklen_t *) &length)) == ERROR)
    {
        return (ERROR);
    }
    
    
    port = ntohs(linkCfg->clientPtr.sin_port);
   	sprintf(address, "%s", inet_ntoa(linkCfg->clientPtr.sin_addr));
    		
   	//if(opr->linkDebug)   
   	Debug(console,"slave> accept O.K...%s... port = %d, socket = %d\n", address, port, linkCfg->SocketID);

	return(linkCfg->SocketID);
#endif
	
}


/**
**
**/
int rcvHandler_SLAVE(byte *sndbuf, byte *rxbuf, int rxcnt)
{

    int     i;
    byte    opcode1;
    int     index;
    int     esioId;
    int     count, sndCount;
    int     rcvAckCode, rcvEventNum;
    //int     devPt, pointSts;
    int     anapoint, dataLen, dbSize;
    int     devid,devNum, rcvStsByte, dumpCount;
    int     startPt, ptNum;
    int     cntrDev, cntrPoint, cntrTCF;
    int     deviceOffline;
    
    byte    front, rear;
    
    byte    rcvQnum, Qnum;
    word    rcvsum, chksum;
    byte    *bfptr;
    float   *fptr;
    float   fdata;
    struct  timeval ctime;
        
    SDP_DEVICE      *dev;
    POINT_BUF       *aiPoint;
    ESIO_CONFIG     *esio;          // ESIO 구조체    
    MPU_SOEQ_ENTRY  *rcvEvent;
    ICCP_SOEQ_ENTRY *delEvent;
    
        
    opcode1 = rxbuf[4] & 0x7f;           // 수신 OPCODE
    
    linkCfg->rcvStatus = rxbuf[5];      // 수신 MPU Status;
    linkCfg->rcvSeqNo  = rxbuf[6];       // 수신 Seqno

    /* MASTER 재기동시.... DB CHK...    */    
    if(linkCfg->rcvStatus & LINK_PFR_BIT)   linkCfg->sdpStatus |= LINK_DBCHK_BIT;

#if 0    
    /* -------------------------------------------- */
    /* 1) 이중화 CPU : ICCP Delete SOE 정보를 연계..*/
    /* -------------------------------------------- */
    if(opr->iccpEnbFlag == SET)
 	{   	
	    if((opr->dualCpuSts == SET) && (linkCfg->iccpDelSOE))
    	{  
        	if(opcode1 != SIM_EVENT_ACK)
	        {      
    	        if(opcode1 != SIM_ICCP_DEL_ACK)  opcode1 = SIM_ICCP_DEL_SOE;          // ICCP Delete-SOE 정보    
        	}
    	}
    }
#endif
    	
    /* ------------------------------------ */
	/* 1) 이중화 CPU : SOE Cheak...         */
	/* ------------------------------------ */
	if(opr->dualCpuSts == SET)
    {
        if(opr->runMode == LOCAL_MASTER)
	    {  
	        if(opcode1 != SIM_EVENT_ACK)
	        {        
                front = linkCfg->front & 0xff;
       	        rear  = linkCfg->rear & 0xff;   
       	        
                if(linkCfg->timeSyncReq == SET)     opcode1 = SIM_TIME_UP;
                else if(front != rear)              opcode1 = SIM_EVENT_DUMP;      // 계전기 SOE 정보  
                      
            }
	    }
	    else  // LOCAL_SLAVE
	    {
	        if(opcode1 != SIM_EVENT_ACK)
	        {
	            if(linkCfg->timeSyncReq == SET)     opcode1 = SIM_TIME_UP;
	            else if(linkCfg->cntFlag == SET)    opcode1 = SIM_POINT_CNTR;    // 계전기 제어정보
	        }
	    }
	           
	}    
    
    /* -------------------------------------------- */
	/* 2019.07.27 SDP-A/B Reset 재기동 명령 Pass ... 	*/
	/* -------------------------------------------- */
	if(opcode1 != SIM_EVENT_ACK)
	{
		if(linkCfg->cntFlag == SET)  
		{
			if((opr->dualCpuSts == SET) && (linkCfg->online == SET))		opcode1 = SIM_POINT_CNTR;
			else
			{
				Debug(console, "slave> *LINK Control Failed.... dualsts=%d, link-online = %d \n", opr->dualCpuSts, linkCfg->online);
			
				linkCfg->cntFlag  = RESET;
				linkCfg->cntPoint = 0;
        	    linkCfg->cntTCF   = 0;        // 제어상태, [0] TRIP, [1] CLOSE
			}
		}					 
    	else if(opr->cpuChange)	opcode1 = SIM_SYSTEM_CNTR;
   	}
        
    /* -------------------------------- */                      
    /* 송신 Frame 구성                      */                      
    /* -------------------------------- */                      
    sndbuf[0] = 0x7e;                       // stx              
    sndbuf[1] = 0x7e;                       // stx              
    sndbuf[2] = 0;                          // size             
    sndbuf[3] = 0;                          // size        
    sndbuf[4] = opcode1;                     // opcode          
    sndbuf[5] = linkCfg->sdpStatus;         // status           
    sndbuf[6] = linkCfg->rcvSeqNo;          // 수신 SeqNo       
    count = 7;                                         

    dataLen = rxcnt - 8;
    
    switch(opcode1)
    {
    case SIM_SYSTEM_CNTR:
        Debug(console,"slave> rcv CPU-CHHANGE....\n");
        opr->cpuChange = 0;
        
        if(opr->runMode == LOCAL_MASTER)    resetMaster();
        else                                setMaster();
            
        break;
              
    case SIM_SDP_STATUS:        // 0x01 SDP 장치상태정보 Dump
        mpuCFG->rcvMpuSts  = rxbuf[7];       // 수신 MPU Status, 
        mpuCFG->rcvRackSts = rxbuf[8];       // 수신 MPU RACK - Status,
        mpuCFG->rcvRunSts  = rxbuf[9];       // 수신 MPU 모듈- 동작 Status,  
        
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
                devPtBuf[i]->status  = rxbuf[10 + MAX_HOST + MAX_DEVICE + i];       // DEVICE 포인트 운영 상태
                devPtBuf[i]->oldsts  = devPtBuf[i]->status;                         // 이전상태 Update
                
                /* ------------------------------------ */
                /*  ICCP-POINT Update...                */
                /* ------------------------------------ */
                update_ICCP_info(shmPtr, (POINT_BUF *) devPtBuf[i], (float) devPtBuf[i]->status, devPtBuf[i]->config);
    
            }
            
            /* ---------------------------- */
            /* 수신된 Device 포인트 정보... Update */
            /* ---------------------------- */
            update_Device_DNP();
        }
                
        /* ---------------------------------------- */
        /*  SDP 상태정보 추출 및 전송               */
        /* ---------------------------------------- */
        sndbuf[count++] = mpuCFG->mpuStatus;            // Slave-MPU 상태정보
        sndbuf[count++] = mpuCFG->mpuRackSts;           // Slave-MPU RACK 상태정보
        sndbuf[count++] = mpuCFG->mpuRunSts;            // Slave-MPU 모듈 동작상태정보
        
        /* ---------------------------------------- */
        /*  HOST & DEVICE 상태정보 추출 및 전송     */
        /* ---------------------------------------- */
        for(i=0; i < MAX_HOST; i++)         sndbuf[count++] = hostDCB[i]->runStatus;    // HOST# 통신 운영 상태
        for(i=0; i < MAX_DEVICE; i++)       sndbuf[count++] = deviceCFG[i]->online;     // 계전기# 통신 운영 상태
        for(i=0; i < MAX_DEV_POINT; i++)    sndbuf[count++] = devPtBuf[i]->status;          // DEVICE 포인트 운영 상태       
        
        if(opr->linkDebug)
    	Debug(console,"slave> SDP_STATUS.... SND: sts = %02x, rack = %02x [RCV: sts = %02x, rack = %02x] \n", 
    	    mpuCFG->mpuStatus, mpuCFG->mpuRackSts, mpuCFG->rcvMpuSts, mpuCFG->rcvRackSts);
    	
        break;

    case SIM_ICCP_DEL_SOE:
        if(linkCfg->iccpDelSOE == 0)
        {      
            rcvQnum = rxbuf[7];     // 수신 SOE Queue
            
            if(opr->soeDebug)
            Debug(console,"slave> rcv ICCP Delete-SOE Dump..... count = %d\n", rcvQnum);

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
            }
            else
            {
                sndbuf[4] = SIM_ICCP_DEL_ACK + 0x80;   // opcode    
                sndbuf[count++] = 0x00;             // SOE Dump-ACK
                sndbuf[count++] = 0x00;             // rcv Q-num
            } 
        }
        else
        {
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
            Debug(console, "slave> snd ICCP Delete-SOEQ Dump...count=%d\n", linkCfg->delReport);   
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
            Debug(console, "slave> rcv ICCP Delete-SOE ACK...count=%d   [front=%d/rear=%d]\n", rcvEventNum, iccpDelSOEQ->front, iccpDelSOEQ->rear);         
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
        
        /* -------------------------------------------- */
		/*	CPU 이중화 : Master/Slave 간 이벤트 내용 공유			*/
		/* -------------------------------------------- */ 
        if(opr->runMode == LOCAL_MASTER)
        {        
            front = linkCfg->front & 0xff;
            rear  = linkCfg->rear & 0xff;
        
            Qnum = (front - rear + 256) % 256;
            if(Qnum > 16)   Qnum = 16;				// 한번에 전송하는 SOE 이벤트 갯수 제한...
        
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
            Debug(console, "slave> snd LINK-SOE Dump...count=%d\n", Qnum);   
            
            /* -------------------------------- */
            /* SOE 전송후 ... rear 포인트 증가 		*/
            /* -------------------------------- */
            linkCfg->rear = (linkCfg->rear + Qnum) & 0xff;         
        }
        else if(opr->runMode == LOCAL_SLAVE)
        {
            rcvQnum = rxbuf[7];     // 수신 SOE Queue
            
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
            Debug(console,"slave> rcv LINK-SOE Event..... count = %d\n", rcvQnum);

            if(rcvQnum)
            {
                for(i=0; i < rcvQnum; i++)
                {   
                    index    = i * sizeof(MPU_SOEQ_ENTRY);
                    rcvEvent = (MPU_SOEQ_ENTRY  *) &rxbuf[8 + index];
                    
                    /* -------------------------------- */
                    /*  LINK-SOE 정보 저장              */
                    /* -------------------------------- */
                    if(rcvEvent->eventCode == ENT_DEV_ONLINE)           set_Online_Device(shmPtr,rcvEvent);     
                    else if(rcvEvent->eventCode == ENT_DEV_OFFLINE)     set_Offline_Device(shmPtr,rcvEvent);            
                    else if(rcvEvent->eventCode == ENT_SOE)             store_MPU_SOE( shmPtr, rcvEvent);
                    else if(rcvEvent->eventCode == ENT_DEVICE_SOE)      Create_Device_SOE(shmPtr, rcvEvent);
                    else
                    {
                    	//printf("slave> rcv Event... code = %d, dev=%d, point=%d, state=%d \n", rcvEvent->eventCode, rcvEvent->devNo, rcvEvent->pointNo, rcvEvent->state);
                    	
                        logEvent_MPU(shmPtr, rcvEvent->eventCode, rcvEvent->devNo, rcvEvent->pointNo, rcvEvent->state, rcvEvent->esioNo, &rcvEvent->updateTime);
                    }            
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
        break;
        
    case SIM_EVENT_ACK:         // 0x03 SDP EVENT-ACK
        rcvAckCode  = rxbuf[7];
        rcvEventNum = rxbuf[8];

		if(rcvAckCode == 0x01)
        {
            if(opr->soeDebug)   
            Debug(console, "slave> rcv LINK-SOE ACK...count=%d   [front=%d/rear=%d]\n", rcvEventNum, linkCfg->front, linkCfg->rear);         
        }
        else if(rcvAckCode == 0x02)
        {
            if(opr->soeDebug)
		    Debug(console,"slave> rcv POINT-CONTROL ACK ... dev=%d, point=%d, tcf=%d \n", linkCfg->cntDev, linkCfg->cntPoint, linkCfg->cntTCF);
		    
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
        else if(rcvAckCode == 0x11)
        {
        	if(opr->soeDebug)
		         Debug(console,"slave> rcv TIME-SYNC Upload ACK ... \n");
		    linkCfg->timeSyncReq = 0;
        }
#if 0        
        if((rcvAckCode == 0x01) && (rcvEventNum == linkCfg->reportSOE))
        {
            /* SOE 전송후 ... rear 포인트 증가 */
            linkCfg->rear = (linkCfg->rear + rcvEventNum) & 0xff;        

            //if(opr->linkDebug)   
            Debug(console, "slave> rcv LINK-SOE ACK...count=%d   [front=%d/rear=%d]\n", rcvEventNum, linkCfg->front, linkCfg->rear);         
        }
        
        if(rcvAckCode == 0x02)
        {
            //if(opr->soeDebug)
		    Debug(console,"slave> rcv POINT-CONTROL ACK ... dev=%d, point=%d, tcf=%d \n", linkCfg->cntDev, linkCfg->cntPoint, linkCfg->cntTCF);
		    
            /* ------------------------ */
            /* 제어정보 Clear...        */  
            /* ------------------------ */          
            linkCfg->cntDev     = 0;
            linkCfg->cntPoint   = 0;
            linkCfg->cntTCF     = 0;
            linkCfg->cntFlag    = 0;
            linkCfg->cntHost    = 0;
            
            opr->cntrLinkPass 	= 0;
        }
        
        if(rcvAckCode == 0x11)
        {
        	//if(opr->linkDebug)
		    Debug(console,"slave> rcv TIME-SYNC Upload ACK ... \n");
		    linkCfg->timeSyncReq = 0;
        }
#endif        
        return (0);
        break;
            
    case SIM_POINT_SCAN:        // 0x04 계전기 POINT 정보 Scan
        devid      = rxbuf[7] - 1;      // 계전기 번호     
        rcvStsByte = rxbuf[8];          // 데이터 사이즈 (Byte 수)     
 
        //hkkim
         if(opr->linkDebug) 
          Debug(console, "slave> SIM_POINT_SCAN SCAN devId[%d] rcvStsByte[%d]\n", devid, rcvStsByte  );
        
        if(devid < 0)	
        {
        	Debug(console, "slave> *Invalid POINT-SCAN devid = %d\n", devid);
        	break;
        }
        	
        /* -------------------------------------------------------------------- */
        /*  LOCAL_SLAVE 모드 : Master CPU로부터 계전기별 상태데이터 Update...   */
        /*  LOCAL_MASTER모드 : Slave CPU에서 Master로 계전기별 상태데이터 전송  */
        /* -------------------------------------------------------------------- */
        if((opr->runMode == LOCAL_SLAVE) && (devid < MAX_DEVICE))
        {    
            dev = (SDP_DEVICE *) deviceCFG[devid];
            
            /* ---------------------------------------- */
            /*  계전기별 상태데이터 Update...           */
            /* ---------------------------------------- */
            if(rcvStsByte > 0)  curStore_BIT_MPU( shmPtr, devid, &rxbuf[9], rcvStsByte);
          
            if(opr->linkDebug) 
            {
                Debug(console, "slave>  : rcv STS(%02d) Data Updated...size= %2d \n", devid+1, rcvStsByte);
                DumpBuff(console," ", &rxbuf[9], rcvStsByte);
            }
                
            sndbuf[7] = devid + 1;           // 계전기 번호     
            sndbuf[8] = rcvStsByte;          // 데이터 사이즈 (Byte 수)     
            count = 9;
        }
        else
        {
            dev = (SDP_DEVICE *) deviceCFG[devid];

            /* ---------------------------------------- */
            /*  계전기 Online : 수신덴 상태데이터 전송  */
            /*         Offline: 계전기 버퍼내용 전송    */
            /* ---------------------------------------- */
            if((dev->online == SET) && (dev->init_status == SET))
            {	
	            sndbuf[7] = devid + 1;              // 계전기 번호     
    	        sndbuf[8] = dev->stsByteCnt;    	// 데이터 사이즈 (Byte 수)     
        	    count = 9;
        	    
            	memcpy( &sndbuf[count], &dev->cursts[0], dev->stsByteCnt);
            	count = count + dev->stsByteCnt;
            }
            else
            {
            	sndbuf[7] = devid + 1;              // 계전기 번호     
    	        sndbuf[8] = 0;    	                // 데이터 사이즈 (Byte 수)     
        	    count = 9;
            }	            
        }    
                
        break;
        
    case SIM_ANALOG_SCAN:       // 0x14 계전기 아날로그 정보 Scan
   
             
        devid      = rxbuf[7] - 1;      // 계전기 번호     
        dumpCount  = rxbuf[8];          // 포인트 수     
  
  
 //        Debug(console, "slave> ANALOG SCAN devId[%d] DumpCount[%d]\n", devid, dumpCount  );
        
        if(devid < 0)	
        {
        	Debug(console, "slave> *Invalid ANALOG-SCAN devid = %d\n", devid);
        	break;
        }
        
        /* -------------------------------------------------------------------- */
        /*  LOCAL_SLAVE 모드 : Master CPU로부터 계전기별 계측데이터 Update...   */
        /*  LOCAL_MASTER모드 : Slave CPU에서 Master로 계전기별 계측데이터 전송  */
        /* -------------------------------------------------------------------- */
        if((opr->runMode == LOCAL_SLAVE) && (devid < MAX_DEVICE))
        {    
            dev = (SDP_DEVICE *) deviceCFG[devid];
            
            bfptr = (byte *) &rxbuf[9];         // 계측데이터 시작지점...
            index = 0;
            
            /* ---------------------------------------- */
            /*  수신된 포인트 수만큼... Update...       */
            /* ---------------------------------------- */
            for(i=0; i< dumpCount; i++)
            {
                index = i * 6;
                anapoint = bfptr[index + 0] * 256 + bfptr[index + 1];   // 계측포인트 번호 (0,1,2,...)
#if 0            
                fptr = (float *) &bfptr[index + 2];                     // 계측 데이터(Float)                                      
                /* 기본 데이터 지정 */
                fdata = (float ) *fptr;

#else // hkkim

                  // in master    memcpy( &sndbuf[count], &aiPoint->floatData, 4);    // 계측 데이터(Float)
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
                    Debug(console,"link> rcv dev=%2d, *Analog Dump - *Invalid AI-Point (%d) \n", devid+1, anapoint);
                    break;
                }
                
                /* ------------------------------------ */               
                /* 계측포인트 데이터.. Update ...       */
                /* ------------------------------------ */      
              //  Debug(console,"dev[%d] Pt[%d] F[%f]\n", devid+1, anapoint,fdata);
              // hkkim 잠시 대기 storeAI_FLOAT_MPU 여기에서 죽네.
                storeAI_FLOAT_MPU( shmPtr, devid, anapoint, fdata, (byte *)fptr);
            }
            
            if(opr->linkDebug) Debug(console, "slave : ANA(%02d) Data Updated (%f)...!\n", devid+1, fdata);
                
            sndbuf[7] = devid + 1;           // 계전기 번호     
            sndbuf[8] = dumpCount;           // 포인트 수 
            count = 9;                    
        }
        else    // MASTER 인경우...
        {
            dev = (SDP_DEVICE *) deviceCFG[devid];
            
            sndbuf[7] = devid + 1;          // 계전기 번호     
            sndbuf[8] = 0;                  // 포인트 수 
            count = 9;

            dumpCount = 0;
            /* ---------------------------------------- */
            /*  계전기 계측데이터 ... 송신메세지 구성   */
            /* ---------------------------------------- */
            for(i=0; i < MAX_DEV_AI_POINT; i++)
            {
                aiPoint = (POINT_BUF *) &dev->aiPtBuf[i];
                if(aiPoint->config == 0)    continue;
                
                sndbuf[count++] = ( i >> 8) & 0xff;         // 계측포인트 번호 (0,1,2,...)
                sndbuf[count++] = i & 0xff;                 // 계측포인트 번호 (0,1,2,...)
                
                memcpy( &sndbuf[count], &aiPoint->floatData, 4);     // 계측 데이터(Float)
                count += 4;
                dumpCount++;    
                
                /* 보고되는 아날로그 데이터 제한 */
                if(dumpCount > 128)  break;    
            }
            
            sndbuf[8] = dumpCount;                          // Dump 포인트 수      
        }            
        break;
    
    case SIM_CHKSUM_DUMP:       // 0x16 데이터베이스 Chksum Scan       
        rcvsum = rxbuf[7]*256 + rxbuf[8];
        chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
        
        //if(opr->linkDebug)
        Debug(console, "slave> ===> rcv chksum = %04x, romsum=%04x !\n", rcvsum, chksum);
        if((rcvsum == chksum) && (linkCfg->dbChanged == SET))
        {
            linkCfg->dbChanged = 0;
            
            chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
            
            Debug(console, "slave> ======> DATABASE Write... chksum = %04x !\n", chksum);
            rtudb->chksum[0] = chksum;
            rtudb->chksum[1] = chksum >> 8;
            
            dbFileWrite(opr, rtudb);
        }      
        
        linkCfg->sdpStatus &= (~LINK_DBCHK_BIT);          // DB-Chksum  Clear
        sndbuf[5] = linkCfg->sdpStatus;         // status     
        
        sndbuf[count++] = (chksum >> 8) & 0xff;
        sndbuf[count++] = chksum & 0xff;
        
        if(opr->linkDebug)
    	Debug(console,"slave> rcv CHKSUM  : rcv = %4x rom = %4x .... [%04d-%02d%02d-%02d:%02d:%02d] \n", rcvsum, chksum, rtc->year, rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
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
        
        /* ---------------------------------------- */
        /*  SLAVE-CPU 로부터 제어정보 수신시...     */
        /* ---------------------------------------- */
        if(opr->cntrLinkPass == SET)
        {
            sndbuf[7] = linkCfg->cntDev;        // 대상 계전기 번호 [1..64]
            sndbuf[8] = linkCfg->cntPoint;      // 대상 포인트 [1..256]
            sndbuf[9] = linkCfg->cntTCF;        // 제어상태, [0] TRIP, [1] CLOSE
            count = 10;
            
            if(opr->soeDebug)
		    Debug(console,"slave> snd POINT-CONTROL ... dev=%d, point=%d, tcf=%d \n", linkCfg->cntDev, linkCfg->cntPoint, linkCfg->cntTCF);
        }    
        else
        {
            cntrDev     = rxbuf[7];             // 대상 계전기 번호 [1..64]
            cntrPoint   = rxbuf[8];             // 대상 포인트 [1..256]
            cntrTCF     = rxbuf[9];             // 제어상태, [1] TRIP, [2] CLOSE
            
            /* ------------------------------------ */
            /* ESIO 제어정보 연계...                */
            /* ------------------------------------ */
            gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
            logEvent_MPU(shmPtr, ENT_LINK_CNTR, cntrDev, cntrPoint, cntrTCF, opr->cpuMode, &ctime);
            
            controlInfo_MPU( shmPtr, cntrDev, cntrPoint, cntrTCF, PASS_LINK_CNTR, &ctime);    
            
            sndbuf[4] = SIM_EVENT_ACK + 0x80;   // opcode    
            sndbuf[count++] = 0x02;             // Control-ACK
            sndbuf[count++] = 0;                // rcv Q-num
        }      
        break;
        
    case SIM_TIME_DOWN:         // 0x11 Time Sync Down
        opr->year 	= rxbuf[7]*256 + rxbuf[8];
    	opr->month 	= rxbuf[9];
    	opr->day 	= rxbuf[10];
    	opr->hour	= rxbuf[11];
    	opr->min	= rxbuf[12];
    	opr->sec	= rxbuf[13];
    	opr->milisec = 0;

    	linkCfg->sdpStatus &= (~LINK_PFR_BIT);          // PFR Clear
    	sndbuf[5] = linkCfg->sdpStatus;         // status     
    	
    	//if(opr->linkDebug)
    	Debug(console, "slave> rcv TIME Sync Down (from master): %4d/%2d/%2d %02d:%02d:%02d \n",opr->year, opr->month, opr->day, opr->hour, opr->min, opr->sec);

		opr->rtcUpdateFlag = SET;
		
		/* TIME-SYNC 관련 이벤트 생성 */    	
    	logEvent_MPU(shmPtr, ENT_LOCAL_TIME, 0, 0, 0, opr->cpuMode, NULL);  
    	
        //opr->getClockNTP = SET;    
        
        //if(opr->linkDebug)  
        //Debug(console, "slave> GET NTP-Server .... TIME req...! \n");  
        
    	sndbuf[count++] = 0;
    	sndbuf[count++] = 0;
        break;

    case SIM_TIME_UP:         // 0x11 Time Sync Down
        sndbuf[count++]  = (rtc->year >> 8) & 0xff;
        sndbuf[count++]  = rtc->year & 0xff;
        sndbuf[count++]  = rtc->month;  
        sndbuf[count++]  = rtc->day;  
        sndbuf[count++]  = rtc->hour;  
        sndbuf[count++]  = rtc->min;  
        sndbuf[count++]  = rtc->sec;  

       	//if(opr->linkDebug)
    	Debug(console, "slave> TIME Sync Upload : %4d/%2d/%2d %02d:%02d:%02d \n", rtc->year, rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
    	linkCfg->timeSyncReq = 0;
        break;
                
    case SIM_MPUCFG_DOWN:       // 0x20 DB : MPU Config Down
    	
    	dbSize = sizeof(DB_MPU_CONFIG);
    	
    	/* 데이터베이스 저장 ... */
    	if(dataLen >= dbSize)   dataLen = dbSize;
    	memcpy( (byte *) &rtudb->mpuConfig, &rxbuf[7], dataLen);
        linkCfg->dbChanged = SET;
        //dbFileWrite(rtudb);
        
        /* DB 초기화 */
        opr->mpuCfgDown_OK = SET;
        
        chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
        //if(opr->linkDebug)
    	Debug(console,"slave> rcv MCU Config Down : rcv =%d ... chksum = %04x\n", dataLen, chksum);
    	
        break;
        
    case SIM_ESIOCFG_DOWN:      // 0x22 DB : ESIO Config Down   , ESIO 모두를 한번에 주는 것이 아니고 1개씩이군...
        dataLen = rxcnt - 10;
        esioId  = rxbuf[7] - 1;           // ESIO # [1..5]
        devNum = rxbuf[8];
        
    	dbSize = sizeof(DB_ESIO_CONFIG);
    	
    	/* 데이터베이스 저장 ... */
    	if(dataLen >= dbSize)   dataLen = dbSize;
    	memcpy( (byte *) &rtudb->esioConfig[esioId], &rxbuf[9], dataLen);
    	linkCfg->dbChanged = SET;
        //dbFileWrite(rtudb);

        /* --------------------------------- */
        /* 해당 ESIO SCAN Config-Down Flag   */
        /* --------------------------------- */   
        esio = (ESIO_CONFIG *) esioCFG[esioId];
        esio->esioCfgDown = SET;
        
    	sndbuf[count++] = esioId + 1;       // 시작 ESIO#  [1..5]
    	sndbuf[count++] = devNum;           // 전송 ESIO 갯수
    	
    	chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
    	//if(opr->linkDebug)
    	Debug(console,"slave> rcv ESIO (%d) Config Down : rcv =%d ... chksum = %04x\n", esioId+1, dataLen, chksum);
    	
    	/* 마지막 ESIO 구성정보 수신후... 재기동 */
        if(esioId == 5)
        {       
            opr->esioCfgDown_OK = SET;      // ESIO Config Down .... Re Initial 
        }
        
        break;
        
    case SIM_HOST_DOWN:         // 0x24 DB : HOST Config Down
        
    	
    	dbSize = sizeof(DB_HOST_CONFIG) * MAX_DB_HOST;
    	
    	/* 데이터베이스 저장 ... */
    	if(dataLen >= dbSize)   dataLen = dbSize;
    	memcpy( (byte *) &rtudb->hostCfg[0], &rxbuf[7], dataLen);
    	linkCfg->dbChanged = SET;
        //dbFileWrite(rtudb);
        
        opr->hostCfgDown_OK = SET;  
        
        chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
        //if(opr->linkDebug)
    	Debug(console,"slave> rcv HOST Config Down : rcv =%d ... chksum = %04x\n", dataLen, chksum);
        break;
        
    case SIM_ICCP_DOWN:         // 0x26 DB : ICCP-HOST Config Down
        
    	dbSize = sizeof(DB_ICCP_CONFIG);
    	
    	/* 데이터베이스 저장 ... */
    	if(dataLen >= dbSize)   dataLen = dbSize;
    	memcpy( (byte *) &rtudb->iccpConfig, &rxbuf[7], dataLen);
    	linkCfg->dbChanged = SET;
        //dbFileWrite(rtudb);
        
        opr->iccpCfgDown_OK = SET;  
        
        chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
        //if(opr->linkDebug)
    	Debug(console,"slave> rcv ICCP Config Down : rcv =%d ... chksum = %04x\n", dataLen, chksum);
    	
        break;
        
    case SIM_HARRIS_DOWN:       // 0x28 DB : HARRIS Config Down
        
    	
    	dbSize = 32;
    	
    	/* 데이터베이스 저장 ... */
    	if(dataLen >= dbSize)   dataLen = dbSize;
    	memcpy( (byte *) &rtudb->portdb[0], &rxbuf[7], dataLen);
    	linkCfg->dbChanged = SET;
        //dbFileWrite(rtudb);
        
        chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
        //if(opr->linkDebug)
    	Debug(console,"slave> rcv HARRIS Config Down : rcv =%d ... chksum = %04x \n", dataLen, chksum);
        break;
        
    case SIM_LANDIS_DOWN:       // 0x2A DB : LANDIS Config Down
        
    	dbSize = 257;
    	
    	/* 데이터베이스 저장 ... */
    	if(dataLen >= dbSize)   dataLen = dbSize;
    	memcpy( (byte *) &rtudb->chassisNum, &rxbuf[7], dataLen);
    	linkCfg->dbChanged = SET;
        //dbFileWrite(rtudb);
        
        chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
        //if(opr->linkDebug)
    	Debug(console,"slave> rcv LANDIS Config Down : rcv =%d ... chksum = %04x \n", dataLen, chksum);
    	
        break;
        
    case SIM_MODBUS_DOWN:       // 0x2C DB : MODBUS Config Down
        
    	dbSize = sizeof(DB_MODBUS_PROFILE) * MAX_DB_MODBUS_PROFILE;
    	
    	/* 데이터베이스 저장 ... */
    	if(dataLen >= dbSize)   dataLen = dbSize;
    	memcpy( (byte *) &rtudb->modbusProfile[0], &rxbuf[7], dataLen);
    	linkCfg->dbChanged = SET;
    	
    	/* --------------------------------- */
        /* 해당 ESIO SCAN Config-Down Flag   */
        /* --------------------------------- */            
        for(i=0; i < MAX_ESIO; i++)
        {
            esio = (ESIO_CONFIG *) esioCFG[i];
            esio->modbusCfgDown = SET;     
        }
        
        //dbFileWrite(rtudb);
        chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
        //if(opr->linkDebug)
    	Debug(console,"slave> rcv MODBUS Config Down : rcv =%d ... chksum = %04x \n", dataLen, chksum);
    	
        break;
        
    case SIM_SCAN_DOWN:         // 0x30 DB : SCAN Config Down
       
    	dbSize = sizeof(DB_SCAN_CONFIG) * MAX_DB_SCAN_PORT;
    	
    	/* 데이터베이스 저장 ... */
    	if(dataLen >= dbSize)   dataLen = dbSize;
    	memcpy( (byte *) &rtudb->scanConfig[0], &rxbuf[7], dataLen);
    	linkCfg->dbChanged = SET;
        //dbFileWrite(rtudb);
        
        opr->scanCfgDown_OK = SET;          // SCAN 구조체 초기화
        
        /* --------------------------------- */
        /* 해당 ESIO SCAN Config-Down Flag   */
        /* --------------------------------- */            
        for(i=0; i < MAX_ESIO; i++)
        {
            esio = (ESIO_CONFIG *) esioCFG[i];
            esio->scanCfgDown = SET;     
        }
        
        chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
		//if(opr->linkDebug)
    	Debug(console,"slave> rcv SCAN Config Down : rcv =%d ... chksum =%04x \n", dataLen, chksum);
    	                
        break;
        
    case SIM_DEVICE_DOWN:       // 0x32 DB : DEVICE Config Down
        devid  = rxbuf[7];
        devNum = rxbuf[8];
        dataLen = rxcnt - 10;
        
        
    	dbSize = sizeof(DB_SDP_DEVICE) * devNum;
    	
    	/* 데이터베이스 저장 ... */
    	if(dataLen >= dbSize)   dataLen = dbSize;
    	memcpy( (byte *) &rtudb->deviceConfig[devid-1], &rxbuf[9], dataLen);
    	linkCfg->dbChanged = SET;
        //dbFileWrite(rtudb);
        
        if(rxbuf[8] & 0x80)
        {	
            opr->devCfgDown_OK = SET; 
            
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
        
    	sndbuf[count++] = devid;            // 시작 계전기 번호 [1..64]
    	sndbuf[count++] = devNum;           // 전송 계전기 수
    	
    	chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
    	//if(opr->linkDebug)
    	Debug(console,"slave> rcv DEVICE[%d-%d] Config Down : rcv =%d ... chksum = %04x \n", devid, devid+devNum -1 , dataLen, chksum);
    	
        break;
        
    case SIM_POINT_DOWN:        // 0x34 DB : POINT Config Down
        startPt = (rxbuf[8] + rxbuf[7] * 256) - 1; // 4081-1 = 4080 부터 16 하면 4095  4080+16 =4096
        ptNum   = rxbuf[9] & 0x7f;
        dataLen = rxcnt - 11;
        
        if(ptNum > 16)  ptNum = 16;
            
        if((startPt + ptNum) > MAX_DB_POINT)	// MAX_DB_POINT 4096	갯수 확인용 16.32...4096
        // if((startPt + ptNum) >= MAX_DB_POINT)	// MAX_DB_POINT 4096	    
		{
			Debug(console,"slave> Point DB Down : *** Invalid Point num = %d, ST[%d]-NUM[%d]\n", startPt + ptNum,startPt,ptNum);
			return 0;
		}	
		
		
        dbSize = sizeof(DB_POINT_BUF) * ptNum;
    	
    	/* 데이터베이스 저장 ... */
    	if(dataLen >= dbSize)   dataLen = dbSize;
    	memcpy( (byte *) &rtudb->pointBuf[startPt], &rxbuf[10], dataLen);
    	linkCfg->dbChanged = SET;
        //dbFileWrite(rtudb);
        
        /* 포인트 전송종료 Check ... */
        if(rxbuf[9] & 0x80)
        {
            opr->pointCfgDown_OK  = SET;        // POINT 구성정보 변경표시...

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
        
        sndbuf[count++] =  rxbuf[7];        // 시작 포인트 번호 [1...4095]
        sndbuf[count++] =  rxbuf[8];  
        sndbuf[count++] =  rxbuf[9];        // 전송 포인트 수
        
        chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
        //if(opr->linkDebug) 
        Debug(console,"slave> rcv Point Down : start=%4d, ptnum=%d ... chksum = %04x \n", startPt + 1, ptNum, chksum);
       
        break;

    case SIM_CAL_POINT_DOWN:        // 0x34 DB : POINT Config Down
        startPt = (rxbuf[8] + rxbuf[7] * 256) - 1;
        ptNum   = rxbuf[9] & 0x7f;
        dataLen = rxcnt - 11;
        
        if(ptNum > 16)  ptNum = 16;
            
        if((startPt + ptNum) > MAX_DB_CAL_POINT)		    
		{
			Debug(console,"slave> CAL Point DB Down : *** Invalid Point num = %d\n", startPt + ptNum);
			return 0;
		}	
		
		
        dbSize = sizeof(DB_CAL_POINT) * ptNum;
    	
    	/* 데이터베이스 저장 ... */
    	if(dataLen >= dbSize)   dataLen = dbSize;
    	memcpy( (byte *) &rtudb->calPointBuf[startPt], &rxbuf[10], dataLen);
    	linkCfg->dbChanged = SET;
        //dbFileWrite(rtudb);
        
        /* 포인트 전송종료 Check ... */
        if(rxbuf[9] & 0x80)
        {
            opr->calPt_CfgDown_OK  = SET;        // POINT 구성정보 변경표시...
        }
        
        sndbuf[count++] =  rxbuf[7];        // 시작 포인트 번호 [1...4095]
        sndbuf[count++] =  rxbuf[8];  
        sndbuf[count++] =  rxbuf[9];        // 전송 포인트 수
        
        chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
        //if(opr->linkDebug) 
        Debug(console,"slave> rcv CAL-Point Down : start=%4d, ptnum=%d ... chksum = %04x \n", startPt + 1, ptNum, chksum );
       
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
        
 //      Debug(console,"rcvHandler_SLAVE  out\r\n");     
        
    return (count);
}


/*
* =======================================================================
*   SLAVE-CPU 주처리 프로세서
* =======================================================================
*/
int linkSlave()
{
	int     rxVal;
	int		oldsec;
    //int     checkTick=0;
	int		length = sizeof(struct sockaddr_in);
    char	address[64];
	int		port;
	int     reConnectCount=0;
	int     runtick=0;
	
    byte	*rxbuf, *txbuf;
	int		rxcnt, txcnt;
	char	buffer[128];
    
    printf(" %s LINK-SLAVE PROCESS Activated ... !\n", TARGET_NAME);        
 
  	rxbuf = (byte *) &slvRxbuf[0];
	txbuf = (byte *) &slvTxbuf[0];
	
    linkCfg->comFailTick = 0;
    linkCfg->online      = 2;
    
    linkCfg->sdpStatus = (LINK_PFR_BIT + LINK_DBCHK_BIT);
    
    /* ------------------------------------ */
    /*  CPU 링크상태 확인....               */
    /* ------------------------------------ */
	opr->chkLinkOK   = SET;		
	//opr->initLinkSts = RESET;		// LINK 초기 접속 상태
	//opr->initLinkCount = 0;
	    
    oldsec = rtc->sec;
    
    /* ---------------------------------------- */
    /*  장치별 TCPIP 초기화...                  */
    /* ---------------------------------------- */
    if (_initNetServer_LINK() == ERROR)
    { 
        Debug(console,"slave> *** SERVER Re-Initial... Fail...\n");	
        termExec = RESET;
    }
                    
	while(termExec)
	{
	    taskPtr->wdtCount = 0;
    	pause(20);

        /* ---------------------------------------- */
	    /*  LINK 접속실패에 따른 재접속...          */
	    /* ---------------------------------------- */
	    while(linkCfg->connectStatus == RESET)
	    {
	        taskPtr->wdtCount = 0;
		    pause(20);
		
  			if(linkCfg->online == SET)    
  			{
  				/* LOG File 저장 */
   				sprintf(buffer, "%s", ">> *** LINK OFFLINE ...!");
   				LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
   				
  			    logEvent_MPU(shmPtr, ENT_LINK_OFFLINE, 0, 0, 0, opr->cpuMode, NULL); 
  			}
  			
		    linkCfg->online = RESET;
			mpuCFG->rcvRunSts = 0;    
            
            if(opr->linkDebug)  Debug(console,"slave> * retry Connect....!\n");
            
            pause(500);
            
            checkDualCPU(0); 
            
            /* ------------------------------------------------ */    
            /*  ACCEPT() 진입...                                */
            /* ------------------------------------------------ */    
	        length = sizeof(struct sockaddr_in);
            if ((linkCfg->SocketID = accept(linkCfg->SocketFd, ( struct sockaddr *)&linkCfg->clientPtr, (socklen_t *) &length)) != ERROR)
            {
                port = ntohs(linkCfg->clientPtr.sin_port);
   	            sprintf(address, "%s", inet_ntoa(linkCfg->clientPtr.sin_addr));
    		
   	            //if(opr->linkDebug)   
   	            Debug(console,"slave> accept O.K...%s... port = %d, socket = %d\n", address, port, linkCfg->SocketID);
            
                linkCfg->connectStatus = SET;
            }
            else
            {
                if(++reConnectCount > 5)
                {
                    reConnectCount = 0;
                
                    /* Socket 종료후 재 초기화 */
                    close( linkCfg->SocketFd);  

                    if(opr->linkDebug)  Debug(console,"slave> * RE Initial...SERVER....!\n");

                    /* ---------------------------------------- */
                    /*  장치별 TCPIP 초기화...                  */
                    /* ---------------------------------------- */
                    if (_initNetServer_LINK() == ERROR)
                    { 
                        Debug(console,"slave> *** SERVER Re-Initial... Fail...\n");	
                    }
                }
            }   

#if 0            
            /* -------------------------------------------- */
            /* CPU 이중화.... LINK 이상시 Default 구동 지정			*/
            /* -------------------------------------------- */
            if(opr->initLinkSts == SET)
           	{
            	if((opr->dualCpuSts == SET) && (opr->runMode == LOCAL_MASTER) && (scuCfg->online == SET) && (scuCfg->remoteMode == AUTO_MODE))
          		{
          			if(opr->linkDebug)
          			Debug(console,"slvae> DUAL CPU...MASTER Mode ... RESET sts %d\n", opr->initLinkCount);
          			if(++opr->initLinkCount > 5)	
          			{
          				resetMaster();
          				opr->initLinkCount = 0;
          			}			
          		}
         	} 	  	
#endif
         	
        }
                
		linkCfg->comFailTick = 0;
    	
    	while(linkCfg->connectStatus)
    	{
    	    taskPtr->wdtCount = 0;
    	    pause(20);

			// TEST...
			if(opr->testWDTFlag)
			{
				if(opr->testWDTFlag == 1)	setMaster();
				if(opr->testWDTFlag == 2)	resetMaster();	
				opr->testWDTFlag = 0;	
			}
		
            /* ------------------------------------ */
            /* 주기적으로 CPU 장착상태를 Check...   */
            /* ------------------------------------ */
            if(++runtick > 5)
            {      
                runtick = 0;
    	        if(linkCfg->cmdFlag == SET) checkDualCPU(1);    	 
                else                        checkDualCPU(0);    
            }
                    
            /* ------------------------------------- */
            /*  메세지 수신 & Response               */
        	/* ------------------------------------- */	        	        
            if((rxcnt = get_responseLINK(linkCfg->SocketID, rxbuf)) > 0)
            {
                linkCfg->comFailTick = 0;
	            
                /* ----------------------------------------- */
                /*  수신한 데이터에 대한 패켓 처리 ...                  */    
                /* ----------------------------------------- */
                if(opr->linkDebug)
                {         
                    if(rxcnt > 256) DumpBuff(console,"LRX: ", rxbuf, 256);
                    else            DumpBuff(console,"LRX: ", rxbuf, rxcnt);
                }                                            

                txcnt = rcvHandler_SLAVE(txbuf, rxbuf, rxcnt);
                
                if(linkCfg->online != SET)    
                {
                	/* LOG File 저장 */
	   				sprintf(buffer, "%s", ">> LINK ONLINE ...!");
   					LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
   				
                    logEvent_MPU(shmPtr, ENT_LINK_ONLINE, 0, 0, 0, opr->cpuMode, NULL); 
                    
                    /* Online 시 TIME-SYNC 	*/
                    if(opr->runMode == LOCAL_MASTER)	linkCfg->timeSyncReq = SET;
                }
                
	            linkCfg->online = SET;
			    //opr->initLinkSts = SET;		// LINK 초기접속 OK
			    //opr->initLinkCount = 0;
			        
                /* -------------------------------------------- */
                /* 전송메세지에 대한 출력                       */  
                /* -------------------------------------------- */     
                if(txcnt >= 5) 
                {
                    /* -------------------------------- */
                    /* Network Response...              */
                    /* -------------------------------- */    
                    if((rxVal = tkWriteTCP( linkCfg->SocketID ,(byte *) txbuf, txcnt, 1000)) < 0 )
                	{
                        if(opr->linkDebug)  Debug(console, "slave> * *** net Send Error [%d %d]...socket = %d !\n", txcnt,  rxVal, linkCfg->SocketID);		
	                }   
	            
                    if(opr->linkDebug) 
                    {
                        if(txcnt > 256)     DumpBuff(console,"LTX: ", txbuf, 256);
                        else                DumpBuff(console,"LTX: ", txbuf, txcnt);    
                    }
    
                    if( rxVal != txcnt)
                    {
                        if(opr->linkDebug)  
                        Debug(console, "slave> * net Send Error [%d %d]...!\n", txcnt,  rxVal);
                        
			            /* -------------------------------- */
			            /* SERVER 종료후 재 초기화          */
			            /* -------------------------------- */
                        linkCfg->connectStatus = RESET;
                    }                
                }         	
        	}
	        else
	        {
    	        if(rxcnt == 0)
        	    {
	            	if(opr->linkDebug)
	                Debug(console, "slave> *TCP/IP read ...0 Connection...Failed...!\n");
	                
	                /* -------------------------------- */
	                /* SERVER 종료후 재 초기화          */
			        /* -------------------------------- */
                    linkCfg->connectStatus = RESET;
	            }
	            
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

#if 0            
            /* ------------------------------------ */
            /*  이중화 CPU 상태 Check...            */
            /*  - 양쪽다 MASTER => A-Master, B-Slave*/
            /*  - 양쪽다 SLAVE  => A-Master, B-Slave*/
            /* ------------------------------------ */
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
                	        resetMaster();
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
	                        resetMaster();
    	                }
        	        }
            	    else    checkTick = 0;  
            	}
			}
#endif
			
    		/* ------------------------------------ */
	    	/* 콘솔종료시.... RESET-Master          */
		    /* ------------------------------------ */
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
   	    	    if(opr->linkDebug)
   		        Debug(console,"slave> *** LINK Time Out ...Fail...\n");	
   		        
   		        /* -------------------------------- */
	            /* SERVER 종료후 재 초기화          */
                /* -------------------------------- */
                mpuCFG->rcvRunSts = 0;
                
                linkCfg->connectStatus = RESET;
   		        
       	    }		
        }
        
	}/* while */

    Debug(console, "LINK: SLAVE ..... CLEAR....%02x !\n", linkCfg->cpuControl);
    mpuCFG->rcvRunSts = 0;
    //resetMaster();
    
    return (0);
}


