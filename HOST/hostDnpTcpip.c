#include	"localLib.h"

extern  SHM_MEMORY	    *shmPtr;

extern  TASK_INFO	*taskPtr;

extern  RTC             *rtc;
extern  OPR_MSG         *opr;
extern  HISTORY_QUE     *hque;                      // CONSOLE 용 이벤트

extern  MPU_CONFIG      *mpuCFG;                    // MPU Config
extern  CONSOLE_INFO	*console;

extern  HOST_DCB        *hostDCB[MAX_HOST];             /* HARRIS, LANDIS, SICS HOST Structure */
extern	LINK_MSG		*linkCfg;

extern  int             termExec;

extern  int             wdtHostFlag[MAX_HOST];


extern  int     chkRcvFrameHOST( int hostid, int chid, int socketID, DNP_DATA_LINK *dnpData);
extern  int     checkDataLinkHOST( int hostid, int chid, int socketID, DNP_DATA_LINK *dnpData);
extern  int     checkAPPFrame_HOST(int hostid, int chid, int socketID);
extern  int     readDataLinkHOST(int hostid, int chid, int socketID, DNP_DATA_LINK *dnpData);


/*******************************************************************************
 *                                                                        
 * 모듈명:	_initNetServerHOST_DNP()
 *                                                                        
 ******************************************************************************/
int _initNetServerHOST_DNP(int hostid, int chid)
{
	int		flag = 1, length;
	
    HOST_DCB        *host;
    HOST_NET_ENTRY  *hostNet;
    MPU_NET_ENTRY   *mNet;
    
    host = (HOST_DCB *) hostDCB[hostid];                 /* 주장치 #1 속성정의 */
    
    if(host->socketServerFd != 0)   close(host->socketServerFd);
    
    /* ------------------------------------------------------------ */
    /*  SERVER-소켓의 Open : IPv4 기반, 연결지향형 소켓, TCP기반    */
    /* ------------------------------------------------------------ */
	if ((host->socketServerFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == ERROR)
	{
	    if(opr->hostDebug == hostid)   
	    Debug(console,"host%2d> *** Channel=%d [NET] socket 실패 \n",hostid+1, chid+1);
		return(ERROR);
	}
	
	if(opr->hostDebug == hostid)	
    Debug(console,"host%2d> Channel=%d...[NET] socket Open OK ...%d \n", hostid+1, chid+1, host->socketServerFd);

    /* ------------------------------------------------ */
    /*  SOCKET Option...                                */
    /* ------------------------------------------------ */
    setsockopt(host->socketServerFd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(int));
    setsockopt(host->socketServerFd, SOL_SOCKET, SO_KEEPALIVE, (char *)&flag, sizeof(int));
    //setsockopt(host->socketServerFd, SOL_SOCKET, SO_DONTROUTE, (char *)&flag, sizeof(int));
    //ioctl(host->socketServerFd, FIONBIO, (int)&flag);

    /* NON-BLOCK 설정 --> accept에서 block 되지 않는다. */
    flag = fcntl( host->socketServerFd, F_GETFL,0);
    fcntl( host->socketServerFd, F_SETFL, (flag | O_NONBLOCK));
    
    /* ------------------------------------------------ */    
    /*  소켓 구조체 초기화 ...                          */
    /* ------------------------------------------------ */    
    length = sizeof(struct sockaddr_in);
	bzero8248((byte *)&host->srvAddr, length);
	host->srvAddr.sin_family  = AF_INET;                          /* IPv4 기반의 인테넷 프로토콜 Family*/
	host->srvAddr.sin_port    = htons(host->tcpPort);             /* Port   정보의 Network Byte Order 로 변경함. */

    //host->srvAddr[chid].sin_addr.s_addr = htonl(INADDR_ANY);        /* IP주소 정보의 Network Byte Order 로 변경함. */

#if 1
    /* ------------------------------------------------ */
    /* CPU 타입과 HOST별 지정된 포트별 IP (CPU)지정     */
    /*  - mst_Net1 : Master-CPU의 Master 포트           */
    /*  - mst_Net2 : Master-CPU의 Slave 포트            */
    /*  - slv_Net1 : Slave-CPU의 Master 포트            */
    /*  - slv_Net2 : Slave-CPU의 Slave 포트             */
    /* ------------------------------------------------ */
	if(opr->cpuMode == MPU_A)
	{
	    hostNet = (HOST_NET_ENTRY *) &host->masterNetCfg[chid];
	    mNet = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[hostNet->netPort];
	    
	    host->srvAddr.sin_addr.s_addr = inet_addr(mNet->ipAddr);   /* Master-TASK : 지정된 NET 포트 의 IP 설정 */
	}
	else
	{   
	    hostNet = (HOST_NET_ENTRY *) &host->slaveNetCfg[chid];
	    mNet = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[hostNet->netPort];
	    
	    host->srvAddr.sin_addr.s_addr = inet_addr(mNet->ipAddr);   /* Master-TASK : 지정된 NET 포트 의 IP 설정 */
	}
#endif
	
    /* ------------------------------------------------ */    
    /*  소켓에 주소를 할당함...                         */
    /* ------------------------------------------------ */    
    length = sizeof(struct sockaddr_in);
	if (bind(host->socketServerFd, ( struct sockaddr *)&host->srvAddr, length) ==   ERROR)
	{
	   // hkkim 2026-06-24 오후 3:50:02   시작후 내용 확인이 불가하네..
	   // if(opr->hostDebug == hostid) 
	    { 
	        if(opr->cpuMode == MPU_A)
	        {   
	            hostNet = (HOST_NET_ENTRY *) &host->masterNetCfg[chid];
	            mNet = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[hostNet->netPort];
	            
	            Debug(console,"host%2d> *** Channel=%d /IP=%s [NET] bind 실패 ... socket = %d\n", hostid+1, chid+1, mNet->ipAddr, host->socketServerFd);
	            printf("host%2d> *** Channel=%d /IP=%s [NET] bind 실패 ... socket = %d\n", hostid+1, chid+1, mNet->ipAddr, host->socketServerFd);
	        }
	        else
	        {
	            hostNet = (HOST_NET_ENTRY *) &host->slaveNetCfg[chid];
	            mNet = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[hostNet->netPort];
	    
	            Debug(console,"host%2d> *** Channel=%d /IP=%s [NET] bind 실패 ... socket = %d\n", hostid+1, chid+1, mNet->ipAddr, host->socketServerFd);
	            printf("host%2d> *** Channel=%d /IP=%s [NET] bind 실패 ... socket = %d\n", hostid+1, chid+1, mNet->ipAddr, host->socketServerFd);	        
	        }     
	    }
		return(ERROR);
	}
	else // hkkim 2026-06-24 오후 3:51:22
	// if(opr->hostDebug == hostid) 
    {
        if(opr->cpuMode == MPU_A)
        {   
            hostNet = (HOST_NET_ENTRY *) &host->masterNetCfg[chid];
            mNet = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[hostNet->netPort];
	        // hkkim 2026-06-24 오후 3:51:26
	        Debug(console,"host%2d> Channel=%d...[NET] bind OK (ip=%s)... socket = %d \n", hostid+1, chid+1, mNet->ipAddr, host->socketServerFd);
	        printf("host%2d> Channel=%d...[NET] bind OK (ip=%s:%d)... socket = %d\r\n", hostid+1, chid+1, 
	                    mNet->ipAddr, host->tcpPort, host->socketServerFd);
	    }
	    else
	    {
	        hostNet = (HOST_NET_ENTRY *) &host->slaveNetCfg[chid];
	        mNet = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[hostNet->netPort];
	            
	        Debug(console,"host%2d> Channel=%d...[NET] bind OK (ip=%s)... socket = %d \n", hostid+1, chid+1, mNet->ipAddr, host->socketServerFd);
	        printf("host%2d> Channel=%d...[NET] bind OK (ip=%s:%d)... socket = %d\r\n", hostid+1, chid+1, 
	                    mNet->ipAddr,  host->tcpPort, host->socketServerFd);
	    }      
    }
    
    /* ------------------------------------------------ */    
    /*  연결요청 대기상태로 진입...                     */
    /* ------------------------------------------------ */    
	if (listen(host->socketServerFd, 5) == ERROR)
	{
	    if(opr->hostDebug == hostid)	Debug(console,"host%2d> *** Channel=%d...[NET] listen 실패 \n", hostid+1, chid+1);
		return(ERROR);
	}

    //if(opr->hostDebug == hostid)	
    //Debug(console,"host%2d> Channel=%d ... [NET] listen OK ... socket = %d \n", hostid+1, chid+1, host->socketServerFd);

    if(opr->hostDebug == hostid) 
    {
        if(opr->cpuMode == MPU_A)
        {   
            hostNet = (HOST_NET_ENTRY *) &host->masterNetCfg[chid];
            mNet = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[hostNet->netPort];
            
            Debug(console,"host%2d> Channel=%d...[NET] listen OK (ip=%s)... socket = %d \n", hostid+1, chid+1, mNet->ipAddr, host->socketServerFd);
	    }
	    else
	    {
	        hostNet = (HOST_NET_ENTRY *) &host->slaveNetCfg[chid];
	        mNet = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[hostNet->netPort];
	        
	        Debug(console,"host%2d> Channel=%d...[NET] listen OK (ip=%s)... socket = %d \n", hostid+1, chid+1, mNet->ipAddr, host->socketServerFd);
	    }      
    }
    
    
	return(host->socketServerFd);
	
	
}


/* ============================================================ */
/*	HOST 인터페이스용 THREAD...									*/
/* ============================================================ */
void    *hostTcpipComm(void *data)
{
	int     hostid, chid;
	int		length = sizeof(struct sockaddr_in);
	int     index;
	int		socketID;
	struct 	sockaddr_in	clientAddr;
	int		port;
	char	address[64];
	int		oldsec=0;
	int     retVal;
	int     hostTerminate=0;
	int     comFailCnt;
	int		rxcnt;
	char    buffer[256];
	
	HOST_DCB        *host;

    DNP_DATA_LINK   *dnpData;

    /* -------------------------------------------- */
    /*	클라이언트 정보를 전역 자료에 입력한다. 	*/
    /* -------------------------------------------- */
	memcpy(&socketID, data, sizeof(int)); 
	memcpy(&clientAddr, (char *)data+sizeof(int), length); 
	
	/* hostid 정보 추출... */
	index = sizeof(int) + length;
	memcpy(&hostid, (char *)data+index, sizeof(int)); 
	
	/* chid 정보 추출... */
	index = sizeof(int) + length + sizeof(int);
	memcpy(&chid, (char *)data+index, sizeof(int)); 
	
	/* -------------------------------------------- */
    /*	HOST 구조체 정보. 	                        */
    /* -------------------------------------------- */
	host = (HOST_DCB *) hostDCB[hostid];                 /* 주장치 #1 속성정의 */
	
	port = ntohs(clientAddr.sin_port);
	sprintf(address, "%s", inet_ntoa(clientAddr.sin_addr));
   	
   	printf("  >> HOST(%d)/CH(%d) DNP-TCPIP_THREAD Activated ...REMOTE: %s [Port=%d, Socket = %d] !\n", hostid+1, chid, address, port, socketID);

    /* ------------------------------------ */
    /* Network 연결후 - TASK 실행		    */
    /* ------------------------------------ */
    host->hostActive = HOST_NOT_READY;          /* HOST 통신모드 : 미지정 */   
	host->threadActive[chid] = SET;
    host->acceptFailCnt = 0;
    
    /* ------------------------------------ */
    /* THREAD 관리변수 초기화		        */
    /* ------------------------------------ */
    hostTerminate  = SET;
    comFailCnt     = 0;
    	        
    /* ------------------------------------ */
    /* DNP 송수신 구조체 초기화             */ 
    /* ------------------------------------ */
    dnpData   = (DNP_DATA_LINK *) &host->dataLinkFrame[chid];

    while(hostTerminate)
    {
        wdtHostFlag[hostid] = 0;
        pause(10);
        
        /* ---------------------------------------------------- */
		/*	CPU 이중화 : LOCAL-SLAVE ... 대기모드 				*/
		/*  - LINK 이상시 ... SLAVE CPU에서 통신하지 않음...    */
		/* ---------------------------------------------------- */
		if(opr->runMode == LOCAL_SLAVE)
	    {
	        if((opr->dualCpuSts == SET) && (linkCfg->online == RESET)) 
	        {
	            Debug(console, "host%d> DNP-TCPIP ...*Link Fail...return...!\n", hostid+1);
	            pause(1000);
	            host->hostActive = HOST_NOT_READY;          /* HOST 통신모드 : 미지정 */   
	            continue;
	        }
        }
              
        /* ---------------------------------------------------- */
        /*  MASTER 통신포트 : 상위 데이터 수신 처리 ...         */
        /* ---------------------------------------------------- */
        if((rxcnt = readDataLinkHOST( hostid, chid, socketID, dnpData)) > 0)
        {
            /* 접속오류 초기화 */
            host->acceptFailCnt  = 0;
            host->portResetCount = 0;
            comFailCnt = 0;
            
            if(opr->hostDebug == hostid)
            {
                Debug(console,"\n[host=%d/%d]-----------------------------------------[%02d/%02d-%02d:%02d:%02d]\n", hostid+1,chid,
      	            rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
      	        
      	        if(chid == MASTER_PORT)         DumpDNP_rcv(console, "RXM:", &dnpData->rcvFrame, rxcnt);
      	        else if(chid == SLAVE_PORT)     DumpDNP_rcv(console, "RXS:", &dnpData->rcvFrame, rxcnt);
      	        else                            
      	        {
      	            Debug(console, "host%d> *rcv Fail ... Invalid Chid = %d\n", hostid+1, chid);
      	            continue;
      	        }
      	                     
            }
            
            /* HOST 통신상태 Update... */
            host->runStatus = chid + 1 ;        // HOST 통신상태 : [0] 미정의, [1]주, [2]예비, [3] 통신이상
                
            dnpData->rcvCount = rxcnt;
                
            /* ---------------------------------------- */
            /*  HOST Online 이벤트 생성...              */
            /* ---------------------------------------- */
            if(host->online[chid] != 1)  
            {
                logEvent_MPU(shmPtr, ENT_HOST_ONLINE, hostid+1, chid, 0, hostid+1, NULL);  
                /* LOG File 저장 */
                sprintf(buffer, "HOST[%d] DNP-TCPIP ...%s", hostid+1, host->hostNameStr);
                LogFile_MPU (shmPtr, ENT_HOST_ONLINE, buffer, strlen(buffer));
            }
                
            host->online[chid] = 1;     // Online 
            
            /* ---------------------------------------- */
            /* 수신 DNP-Packet 처리...                  */
            /* ---------------------------------------- */
            retVal = chkRcvFrameHOST(hostid, chid, socketID, dnpData);
            if((retVal == FUNC_USER_DATA) || (retVal == FUNC_UNCONFIRM))
            {
                retVal = checkDataLinkHOST(hostid, chid, socketID, dnpData);
                if(retVal)
                {
                    checkAPPFrame_HOST( hostid, chid, socketID);
                }                    
            }

        } // MASTER PORT      
        else if(rxcnt == 0)
        {
            //if(opr->hostDebug == hostid)
            Debug(console,"host%02d> *DNP-TCPIP (chid=%d) read ...0 Connection...Closed...!\n", hostid+1, chid);
            pause(1000);
         
            /* ---------------------------------------- */
            /*  HOST Offline이벤트 생성...              */
            /* ---------------------------------------- */
            if(host->online[chid] != 0)  
            {   
                logEvent_MPU(shmPtr, ENT_HOST_OFFLINE, hostid+1, chid, 0, hostid+1, NULL);  
                /* LOG File 저장 */
                sprintf(buffer, "HOST[%d] DNP-TCPIP ...%s", hostid+1, host->hostNameStr);
                LogFile_MPU (shmPtr, ENT_HOST_OFFLINE, buffer, strlen(buffer));
            }
                                       
            /* HOST 통신상태 Update... */
            host->runStatus    = 3;                // HOST 통신상태 : [0] 미정의, [1]주, [2]예비, [3] 통신이상                                              
            host->online[chid] = 0;

            /* ---------------------------------------- */
  	        /* DNP-Thread 종료... Accept() 대기         */
  	        /* ---------------------------------------- */
  	        hostTerminate  = RESET;
            host->threadActive[chid] = RESET;
            host->hostActive = HOST_NOT_READY;          /* HOST 통신모드 : 미지정 */   
        }
            
        if(oldsec == rtc->sec)  continue;
        oldsec = rtc->sec;
        
        host->acceptFailCnt = 0;

        /* ------------------------------------------------ */
        /* HOST 통신이상시 처리...주/예비별 차별            */
        /* 주 포트 : Timeout 10초, 예비 포트 : Timeout 60초 */     
        /* ------------------------------------------------ */
        if(host->hostActive == HOST_ACTIVE_STS)
        {
            if(++comFailCnt > 10)  
            {
                /* ---------------------------------------- */
                /*  HOST Offline이벤트 생성...              */
                /* ---------------------------------------- */
                if(host->online[chid] != 0)  
                {   
                    logEvent_MPU(shmPtr, ENT_HOST_OFFLINE, hostid+1, chid, 0, hostid+1, NULL);  
                    /* LOG File 저장 */
                    sprintf(buffer, "HOST[%d] DNP-TCPIP ...%s", hostid+1, host->hostNameStr);
                    LogFile_MPU (shmPtr, ENT_HOST_OFFLINE, buffer, strlen(buffer));
                }
                
                /* HOST 통신상태 Update... */
                host->runStatus    = 3;                // HOST 통신상태 : [0] 미정의, [1]주, [2]예비, [3] 통신이상                                              
                host->online[chid] = 0;
                host->hostActive = HOST_NOT_READY;          /* HOST 통신모드 : 미지정 */   
            
                if(opr->hostDebug == hostid)    
                Debug(console,"host%02d) *DNP-TCPIP (chid=%d) Comm Fail...Re-initial...!\n", hostid+1, chid);
            
                /* ---------------------------------------- */
  	            /* DNP-Thread 종료... Accept() 대기         */
      	        /* ---------------------------------------- */
  	            hostTerminate  = RESET;
                host->threadActive[chid] = RESET;
            
            }      
        }
        else
        {              
            if(++comFailCnt > 60)   
            {   
                /* ---------------------------------------- */
                /*  HOST Offline이벤트 생성...              */
                /* ---------------------------------------- */
                if(host->online[chid] != 0)  
                {   
                    logEvent_MPU(shmPtr, ENT_HOST_OFFLINE, hostid+1, chid, 0, hostid+1, NULL);  
                    /* LOG File 저장 */
                    sprintf(buffer, "HOST[%d] DNP-TCPIP ...%s", hostid+1, host->hostNameStr);
                    LogFile_MPU (shmPtr, ENT_HOST_OFFLINE, buffer, strlen(buffer));
                }
                
                /* HOST 통신상태 Update... */
                host->runStatus    = 3;                // HOST 통신상태 : [0] 미정의, [1]주, [2]예비, [3] 통신이상                                              
                host->online[chid] = 0;
            	host->hostActive = HOST_NOT_READY;          /* HOST 통신모드 : 미지정 */   
            
                if(opr->hostDebug == hostid)    
                Debug(console,"host%02d) *DNP-TCPIP (chid=%d) Comm Fail...Re-initial...!\n", hostid+1, chid);
            
                /* ---------------------------------------- */
  	            /* DNP-Thread 종료... Accept() 대기         */
      	        /* ---------------------------------------- */
  	            hostTerminate  = RESET;
                host->threadActive[chid] = RESET;
                
            }
        }
    }
    
    /* ------------------------------------ */    
    /*  HOST-Socket Close...                */
    /* ------------------------------------ */
    if(host->online[chid] != 0)  
    {   
        logEvent_MPU(shmPtr, ENT_HOST_OFFLINE, hostid+1, chid, 0, hostid+1, NULL);  
        sprintf(buffer, "HOST[%d] DNP-TCPIP ...%s", hostid+1, host->hostNameStr);
        LogFile_MPU (shmPtr, ENT_HOST_OFFLINE, buffer, strlen(buffer));
    }
                
    host->online[chid] = 0;
    host->threadActive[chid] = RESET;   // THREAD 상태 종료 
    
    close(socketID);                    // THREAD Socket 종료
    
    Debug(console,">> HOST(%2d) : DNP-TCPIP Thread(ch=%d) Exit ...Close Port=%d, Socket = %d !\n", hostid+1, chid, port, socketID);
    
    /* ------------------------------------ */
    /* 접속종료되는 PORT 관리 (Max 4)       */
    /* ------------------------------------ */
    host->acceptPORT[chid] = 0;
    
    return (0);    
}



/* ============================================================ */
/*	HOST 인터페이스용 THREAD...									*/
/* ============================================================ */
int    hostDnp_tcpip_Thread(int hostid)
{
	int     chid, rcvChid;
	int		client, length = sizeof(struct sockaddr_in);
	int		port;
	int     index;
	int     portSum;
	int     taskCount;
	char	address[64];
	char	data[64];
    	
	pthread_t	pThread;
	
	struct sockaddr_in	clientPtr;
	
	HOST_DCB        *host;

	/* -------------------------------- */
	/*  HOST별 Index ...                */
	/* -------------------------------- */
	chid      = 0;                    // MASTER-NET
	taskCount = 0;
	
    host = (HOST_DCB *) hostDCB[hostid];                 /* 주장치 #1 속성정의 */
    
    host->hostActive = HOST_NOT_READY;          /* HOST 통신모드 : 미지정 */   
    host->threadActive[0] = RESET;
    host->threadActive[1] = RESET;                
    
    host->online[MASTER_PORT]      = 0;    // default OFFLINE
    host->online[SLAVE_PORT]       = 0;    // default OFFLINE
        
    host->iinRcvTick = 0;
    host->acceptFailCnt = 0;
    
    /* ------------------------------------ */
    /* 접속/종료되는 PORT 관리 (Max 4)      */
    /* ------------------------------------ */
    host->acceptPORT[0] = 0;
    host->acceptPORT[1] = 0;
     
    printf(" %s HOST[%d] DNP-TCPIP-THREAD Activated ... !\n", TARGET_NAME, hostid+1);

    /* ------------------------------------ */
    /*  HOST 서버 환경초기화                */
    /* ------------------------------------ */    
    if(_initNetServerHOST_DNP(hostid, chid) == ERROR)
    {
   	    printf("host%02d> *** NET SERVER Initial... Fail...\n", hostid+1);	
	    close(host->socketServerFd);
        termExec = NO;
    }
    
    opr->dnpHostEnb  = SET;     // HOST : DNP HOST 정의시
    
    opr->hostRestart[hostid] = RESET;
    host->portResetCount = 0;
    taskCount = 0;
    
	while(termExec)
	{
	    taskPtr->wdtCount = 0;
	    
	    wdtHostFlag[hostid] = 0; // ? 용도 못 찾음
		pause(1000);
		pause(1000);
		pause(1000);

        /* ------------------------------------ */
        /* HOST-DB 변경에 따른 재기동 ...            */
        /* 현 code에서는 사용하지 않고 있네...         */
        /* ------------------------------------ */
        if(opr->hostRestart[hostid] == SET)
        {
            Debug(console,"host%02d> *** HOST TASK Restart... DB Chg ...\n", hostid+1);
            pause(1000);
            termExec = RESET;
        }
           
        /* ------------------------------------ */        
        /* HOST  접속요청에따른  접속 허용      */
        /* ------------------------------------ */
    	length = sizeof(struct sockaddr_in);
	    if ((client = accept(host->socketServerFd, ( struct sockaddr *)&clientPtr, (socklen_t *) &length)) != ERROR)
        {
            /* 접속 Port, Address 정보 Assign... */
        	port = ntohs(clientPtr.sin_port);
    		sprintf(address, "%s", inet_ntoa(clientPtr.sin_addr));
    	
        	/* socket 정보 Assign... */
	    	memcpy(data, &client, sizeof(int));
			memcpy(&data[sizeof(int)], &clientPtr, length);
		    //sprintf(address, "%s", inet_ntoa(clientPtr.sin_addr));
		
    		/* hostid 정보 Assign... */
	    	index = sizeof(int)+ length;
			memcpy(&data[index], &hostid, sizeof(int));
		
    		/* ------------------------------------ */
	    	/* Access HOST 채널ID 확인              */
			/* ------------------------------------ */
            if(taskCount == 0)  rcvChid = MASTER_PORT; // 처음에 accept는 master 이고 
            else                rcvChid = SLAVE_PORT;  // 두번째 accpet는 slave 이군..  
            
            taskCount = (taskCount + 1) & 0x01;
        
            //if(opr->hostDebug == hostid)   
            Debug(console,"host%02d> DNP-TCPIP  Accept O.K...From %s:%d  (chid=%d)..., socket = %d\n", hostid+1, address, port, rcvChid, client);
        
            /* ------------------------------------ */
	    	/* HOST 해당 chid-Task 체크...             */
			/*  - 해당 TASK 가 없는경우...  실행          */
		    /* ------------------------------------ */
            if(host->threadActive[rcvChid] == RESET)
            {
                /* chid 정보 Assign... */
                index = sizeof(int)+ length + sizeof(int);
                memcpy(&data[index], &rcvChid, sizeof(int));
		    
		        /* ------------------------------------ */
   	            /* 접속되는 PORT 관리                   */
		        /* ------------------------------------ */
                host->acceptPORT[rcvChid] = port;           // 접속포트정보... 관리
                host->portResetCount = 0;
                
                Debug(console,"\n------------------------------------------------------------------\n");
                Debug(console,"host%02d> DNP-TCPIP ... Accept PORT .. MASTER = %4d, SLAVE = %4d\n", hostid+1, host->acceptPORT[0],host->acceptPORT[1]);
                Debug(console,"------------------------------------------------------------------\n");
            
            	/* ------------------------------------ */
        	    /*	통신 쓰레드 생성 (PCS Interface...)	   */
        	    /* data는 socket 정보와 host id           */	
	        	/* ------------------------------------ */
        	    if (pthread_create(&pThread, NULL, &hostTcpipComm, data) < 0)
	        	{
		    	    Debug(console,"host%02d> *** DNP-TCPIP Thread Create Fail...! ", hostid+1);
    	        	pause(1000);
    	    	
        	    	/* 프로그램 재기동... */
                    termExec = RESET;
    	        }
            }
            else
            {
                Debug(console,"host%02d> ==> DNP-TCPIP ... TASK-Already ACTIVE... %s (chid=%d)... port = %d, socket = %d\n", hostid+1, address, rcvChid, port, client);
        		pause(1000);
		
    		    /* ---------------------------------------- */
	    	    /* 2015년 4월 24일                          */
			    /* 유효 소켓이 남아있는 경우, CLOSE...      */
		        /* ---------------------------------------- */
		        close(client);
            }
                                 
        }
        else
        {	/* hkkim : accept가 실패한 경우                     */
            /* -------------------------------------------- */
            /* 접속되는 PORT 관리                           */
            /*  - 포트가 4개이상 활성화된 경우...Restart    */
            /* -------------------------------------------- */
            portSum = host->acceptPORT[0] + host->acceptPORT[1];
            
            if(opr->hostDebug == hostid)   
            Debug(console,"host%02d> port sum = %d, resetCount= %d .. MASTER.port = %4d, SLAVE.port = %4d\n", hostid+1, portSum, host->portResetCount, host->acceptPORT[0],host->acceptPORT[1]);
            
            if(portSum != 0) // 접속이 되어 있다.
            {
                /* HOST 통신시 초기화 됨 */
                if(++host->portResetCount > 10)
                {      
                    Debug(console,"host%02d> *** Residunt PORT ... HOST-PROCESS Timeout ... RESTART...\n", hostid+1);	
            	    close(host->socketServerFd);
        	    
            	    /* 프로그램 재기동... */
                    termExec = RESET;
                }
            }
                   
		}
		
        
	}/* while */
	
	Debug(console,"host%2d> *TCPIP-THREAD ......END...!\n", hostid+1);
    return (0);
}
