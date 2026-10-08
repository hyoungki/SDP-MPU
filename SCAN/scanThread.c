#include	"localLib.h"
#include    "external.h"
#include    "extfunc.h"

extern  SHM_MEMORY	    *shmPtr;
extern  int             termExec;
extern  TASK_INFO	    *taskPtr;

extern  PDPDTIME_DATE_TIME      dnpRTC;
extern  PDPDTIME_MS_SINCE_70    dnpRTCInfo;

/* ---------------------------------------- */
/*  THREAD 관련 변수 정의                   */
/* ---------------------------------------- */
extern  THREAD_ENTRY    ESIO_THREAD[MAX_ESIO];              // ESIO Thread 구조체 TASK
extern  int     ThreadActive[MAX_ESIO];

static  byte    esioSndBuf[MAX_ESIO][4096];
static  byte    esioRcvBuf[MAX_ESIO][4096];


extern ESIO_RTU_DATABASE  esio_rtudb ; 

/*******************************************************************************
 *                                                                        
 * 모듈명:	_initNetpServer()
 *                                                                        
 ******************************************************************************/
#if 0
int _initNetClient_SCAN( ESIO_CONFIG *esio, int esioid)
{
    int     flag = 1;
    char	buffer[128];
    
    ESIO_NET_ENTRY  *esioNet1;
    
    /* ------------------------------------------------ */
    /* 2019.06.21 ... CLIENT 경우...소켓중첩 방지... 			*/
    /*	- ESIO SCAN 허용/금지시... 소켓이 중첩되어 통신이상 처리 방지		*/
    /* ------------------------------------------------ */
    // 2026-05-19 오후 1:22:09 
    if(esio->SocketFd) 
    {     
        esio->SocketFd = -1 ; 
        close( esio->SocketFd);    

    }
	//pause(1000);    	
            
            
            
            
    /* -------------------------------------------------- */
    /* 소켓의 Open : IPv4 기반, 연결지향형 소켓, TCP기반  */
    /* -------------------------------------------------- */
	if ((esio->SocketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == ERROR)
	{
	    //if(opr->esioDebug == esioid)
        Debug(console,"esio%d> *** [NET] Socket Open Failed...\n", esioid);	
        
        sprintf(buffer, "esio%d> *** [NET-INIT] Socket Open Failed => TASK Restart ... %s ", esioid, esio->esioNameStr);
        LogFile_MPU (shmPtr, ENT_ESIO_OFFLINE, buffer, strlen(buffer));
                
        pause(100);
        termExec = NO;
        
		return(ERROR);
	}
	
  //  if(opr->esioDebug == esioid)
    {
        Debug(console,"esio%d> socket-ID  Open OK ...%d \n", esioid, esio->SocketFd);
    }
    
    /* ------------------------------------------------ */
    /*  SOCKET Option...                                */
    /* ------------------------------------------------ */
    setsockopt(esio->SocketFd, SOL_SOCKET, SO_REUSEADDR, (const void  *)&flag, sizeof(int));
    setsockopt(esio->SocketFd, SOL_SOCKET, SO_KEEPALIVE, (const void *)&flag, sizeof(int));
    
#if 1
    setsockopt(esio->SocketFd, SOL_SOCKET, SO_DONTROUTE, (const void *)&flag, sizeof(int));
//    ioctl(esio->SocketFd, FIONBIO, (int)&flag);
#endif
    
    /* ------------------------------------------------ */
    /* NON-BLOCK 설정 */
    /* ------------------------------------------------ */
//    flag = fcntl( esio->SocketFd, F_GETFL,0);
 //   fcntl( esio->SocketFd, F_SETFL, (flag | O_NONBLOCK));
        
    /* ------------------------------------------------ */
    /* 대상 ESIO# 주소정보 추출...                      */
    /* ------------------------------------------------ */
    if((esioid + 1) == SDP_RTU)
    {
        esioNet1 = (ESIO_NET_ENTRY *) &esio->mstNetConfig[0];
    }
    else
    {            
        if(opr->cpuMode == MPU_A)       esioNet1 = (ESIO_NET_ENTRY *) &esio->mstNetConfig[0];
        else if(opr->cpuMode == MPU_B)  esioNet1 = (ESIO_NET_ENTRY *) &esio->slvNetConfig[0]; 
        else                            esioNet1 = (ESIO_NET_ENTRY *) &esio->mstNetConfig[0];       
    }
    
    /* ---------------------------------------- */
    /* 소켓 구조체 초기화 ...                   */
    /*  - ESIO_SCAN_NET_PORT : 9700 + ESIO-ID   */
    /* ---------------------------------------- */
	esio->serv_addr.sin_family = AF_INET;                                   /* IPv4 기반의 인테넷 프로토콜 Family */
	esio->serv_addr.sin_port   = htons(ESIO_SCAN_NET_PORT + esioid);        /* Port   정보의 Network Byte Order 로 변경함. */
	esio->serv_addr.sin_addr.s_addr = inet_addr(esioNet1->ipAddr);	        /* IP conversion & assign */;
	
	// if(opr->esioDebug == esioid)
	Debug(console, "esio%d> INIT Client... TARGET : %s, Socket= %d, PORT = %d \n", esioid, esioNet1->ipAddr, esio->SocketFd, ESIO_SCAN_NET_PORT + esioid);
	
	// 위에서 non-blocking을 했는데
    /* Image - Server Connection... */
    if(connect(esio->SocketFd, (struct sockaddr *) &esio->serv_addr, sizeof(esio->serv_addr)) == -1)
    {
        printf("connect failed : %s\r\n",strerror(errno)) ;
        close( esio->SocketFd);  
        esio->SocketFd = -1 ; 
      
  

        
      //   if(opr->esioDebug == esioid)    
        Debug(console,"esio%d> *** Connect Error , IP=%s, PORT=%d  !\n", esioid, esioNet1->ipAddr, ESIO_SCAN_NET_PORT + esioid);
        esio->connectStatus = 0;
        // 2026-05-19 오후 1:23:15
        return - 1; 
    }
    else
    {
        sprintf(esio->targetAddr, "%s", inet_ntoa(esio->serv_addr.sin_addr));
        
        //if(opr->esioDebug == esioid)
        Debug(console,"------------------------------------------ \n");
        Debug(console,"esio%d> IP=%s, PORT=%d, socket=%d  connect OK... %s \n", esioid, esioNet1->ipAddr, ESIO_SCAN_NET_PORT + esioid, esio->SocketFd, esio->targetAddr);	 
        Debug(console,"------------------------------------------ \n");
        
        esio->connectStatus = SET;
        esio->comFailTick = 0;
        pause(1000);
        
        return(esio->SocketFd);
    }
            
            
	
	
	
	
	//return (esio->SocketFd);
	
}
#endif 
static int _initNetClient_SCAN( ESIO_CONFIG *esio, int esioid)
{
    int     flag = 1;
    int     retVal ; 
    char	buffer[128];
    char curTime[80] ;       
    
    
    ESIO_NET_ENTRY  *esioNet1;
    
    /* ------------------------------------------------ */
    /* 2019.06.21 ... CLIENT 경우...소켓중첩 방지... 			*/
    /*	- ESIO SCAN 허용/금지시... 소켓이 중첩되어 통신이상 처리 방지		*/
    /* ------------------------------------------------ */
    
    // STEP1
    // 2026-05-19 오후 1:22:09 
    if(esio->SocketFd) 
    {     
        esio->SocketFd = -1 ; 
        close( esio->SocketFd);    

    }
    
    
    // STEP2
    /* find target address */
    /* ------------------------------------------------ */
    /* 대상 ESIO# 주소정보 추출...                      */
    /* ------------------------------------------------ */
    if((esioid + 1) == SDP_RTU)
    {
        esioNet1 = (ESIO_NET_ENTRY *) &esio->mstNetConfig[0];
    }
    else
    {            
        if(opr->cpuMode == MPU_A)       esioNet1 = (ESIO_NET_ENTRY *) &esio->mstNetConfig[0];
        else if(opr->cpuMode == MPU_B)  esioNet1 = (ESIO_NET_ENTRY *) &esio->slvNetConfig[0]; 
        else                            esioNet1 = (ESIO_NET_ENTRY *) &esio->mstNetConfig[0];       
    }
    
    /* ---------------------------------------- */
    /* 소켓 구조체 초기화 ...                   */
    /*  - ESIO_SCAN_NET_PORT : 9700 + ESIO-ID   */
    /* ---------------------------------------- */
	esio->serv_addr.sin_family = AF_INET;                                   /* IPv4 기반의 인테넷 프로토콜 Family */
	esio->serv_addr.sin_port   = htons(ESIO_SCAN_NET_PORT + esioid);        /* Port   정보의 Network Byte Order 로 변경함. */
	esio->serv_addr.sin_addr.s_addr = inet_addr(esioNet1->ipAddr);	        /* IP conversion & assign */;
	
	if(opr->esioDebug == esioid)
	Debug(console, "esio%d> try to connet TARGET[%s], Socket= %d, PORT = %d \n", esioid, esioNet1->ipAddr, esio->SocketFd, ESIO_SCAN_NET_PORT + esioid);
	
    
	//pause(1000);    	
            
    // STEP3  tk_connect                
    /* 소켓의 Open : IPv4 기반, 연결지향형 소켓, TCP기반  */
    /* 내부적으로 Delay 및 timeout 수행 */
	if (( retVal = tk_connect (AF_INET, SOCK_STREAM, IPPROTO_TCP ,
					(const struct sockaddr *) &esio->serv_addr, sizeof( esio->serv_addr),
					2000, 2, 500, &esio->SocketFd )) < 0 )
										
	{
	    // 실패
         if(opr->esioDebug == esioid)
         //  	Debug(console,"Err-Dev[%02d]> Socket Open Failed...\n", devid+1);	
          {
           tk_print_curTime(curTime, 80) ;            
           Debug(console,"Err> ESIO[%02d] : Socket[%d], %s connection to %s:%d Failed\r\n",
            	 	 esioid, esio->SocketFd,   esio->esioNameStr,          	 	          	 	             	 	 
            	 	 inet_ntoa( esio->serv_addr.sin_addr ),
            	 	 ntohs(esio->serv_addr.sin_port) )  ;	
           Debug(console,"%s at %s\r\n",strerror(errno), curTime);          
         }       
         
	    //if(opr->esioDebug == esioid)
        // Debug(console,"esio%d> *** [NET] Socket Open Failed...\n", esioid);	
        sprintf(buffer, "esio%d> *** [NET-INIT] Connection Failed ... %s ", esioid, esio->esioNameStr);                
//        sprintf(buffer, "esio%d> *** [NET-INIT] Connection Failed => TASK Restart ... %s ", esioid, esio->esioNameStr);
        LogFile_MPU (shmPtr, ENT_ESIO_OFFLINE, buffer, strlen(buffer));
                
     //   pause(100);
     
         if ( retVal == TK_SYS_ERR)
              termExec = NO;         
                                    
		return(-1);
	}  

    // 성공 
  
  
  
#if 0   
            
    /* -------------------------------------------------- */
    /* 소켓의 Open : IPv4 기반, 연결지향형 소켓, TCP기반  */
    /* -------------------------------------------------- */
	if ((esio->SocketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == ERROR)
	{
	    //if(opr->esioDebug == esioid)
        Debug(console,"esio%d> *** [NET] Socket Open Failed...\n", esioid);	
        
        sprintf(buffer, "esio%d> *** [NET-INIT] Socket Open Failed => TASK Restart ... %s ", esioid, esio->esioNameStr);
        LogFile_MPU (shmPtr, ENT_ESIO_OFFLINE, buffer, strlen(buffer));
                
        pause(100);
        termExec = NO;
        
		return(ERROR);
	}
	
  //  if(opr->esioDebug == esioid)
    {
        Debug(console,"esio%d> socket-ID  Open OK ...%d \n", esioid, esio->SocketFd);
    }
#endif     
    
    
    // STEP4 set option
    
    /* ------------------------------------------------ */
    /*  SOCKET Option...                                */
    /* ------------------------------------------------ */
    setsockopt(esio->SocketFd, SOL_SOCKET, SO_REUSEADDR, (const void  *)&flag, sizeof(int));
    setsockopt(esio->SocketFd, SOL_SOCKET, SO_KEEPALIVE, (const void *)&flag, sizeof(int));
    
#if 1
    setsockopt(esio->SocketFd, SOL_SOCKET, SO_DONTROUTE, (const void *)&flag, sizeof(int));
//    ioctl(esio->SocketFd, FIONBIO, (int)&flag);
#endif
    
    /* ------------------------------------------------ */
    /* NON-BLOCK 설정 */
    /* ------------------------------------------------ */
    flag = fcntl( esio->SocketFd, F_GETFL,0);
    fcntl( esio->SocketFd, F_SETFL, (flag | O_NONBLOCK));
        

	
	// STEP5 set flags
	 esio->connectStatus = SET;
     esio->comFailTick = 0;
     
    // STEP6 
    return esio->SocketFd ;
     
	
#if 0	
	// 위에서 non-blocking을 했는데
    /* Image - Server Connection... */
    if(connect(esio->SocketFd, (struct sockaddr *) &esio->serv_addr, sizeof(esio->serv_addr)) == -1)
    {
        printf("connect failed : %s\r\n",strerror(errno)) ;
        close( esio->SocketFd);  
        esio->SocketFd = -1 ; 
      
  

        
      //   if(opr->esioDebug == esioid)    
        Debug(console,"esio%d> *** Connect Error , IP=%s, PORT=%d  !\n", esioid, esioNet1->ipAddr, ESIO_SCAN_NET_PORT + esioid);
        esio->connectStatus = 0;
        // 2026-05-19 오후 1:23:15
        return - 1; 
    }
    else
    {
        sprintf(esio->targetAddr, "%s", inet_ntoa(esio->serv_addr.sin_addr));
        
        //if(opr->esioDebug == esioid)
        Debug(console,"------------------------------------------ \n");
        Debug(console,"esio%d> IP=%s, PORT=%d, socket=%d  connect OK... %s \n", esioid, esioNet1->ipAddr, ESIO_SCAN_NET_PORT + esioid, esio->SocketFd, esio->targetAddr);	 
        Debug(console,"------------------------------------------ \n");
        
        esio->connectStatus = SET;
        esio->comFailTick = 0;
        pause(1000);
        
        return(esio->SocketFd);
    }
#endif             
            
	
	
	
	
	//return (esio->SocketFd);
	
}


/*
*/
int _reConnectClient_SCAN(ESIO_CONFIG *esio, int esioid)
{
    ESIO_NET_ENTRY  *esioNet1;
    
    /* ------------------------------------------------ */
    /* 대상 ESIO# 주소정보 추출...                      */
    /* ------------------------------------------------ */
    if((esioid + 1) == SDP_RTU)
    {
        esioNet1 = (ESIO_NET_ENTRY *) &esio->mstNetConfig[0];
    }
    else
    {            
        if(opr->cpuMode == MPU_A)       esioNet1 = (ESIO_NET_ENTRY *) &esio->mstNetConfig[0];
        else if(opr->cpuMode == MPU_B)  esioNet1 = (ESIO_NET_ENTRY *) &esio->slvNetConfig[0]; 
        else                            esioNet1 = (ESIO_NET_ENTRY *) &esio->mstNetConfig[0];       
    }
    
    /* Image - Server Connection... */
    if(connect(esio->SocketFd, (struct sockaddr *) &esio->serv_addr, sizeof(esio->serv_addr)) == -1)
    {
        if(opr->esioDebug == esioid)    
        Debug(console,"esio%d> *** Connect Error , IP=%s, PORT=%d  !\n", esioid, esioNet1->ipAddr, ESIO_SCAN_NET_PORT + esioid);
        esio->connectStatus = 0;
    }
    else
    {
        sprintf(esio->targetAddr, "%s", inet_ntoa(esio->serv_addr.sin_addr));
        
        //if(opr->esioDebug == esioid)
        Debug(console,"------------------------------------------ \n");
        Debug(console,"esio%d> IP=%s, PORT=%d, socket=%d  connect OK... %s \n", esioid, esioNet1->ipAddr, ESIO_SCAN_NET_PORT + esioid, esio->SocketFd, esio->targetAddr);	 
        Debug(console,"------------------------------------------ \n");
        
        esio->connectStatus = SET;
        esio->comFailTick = 0;
        pause(1000);
        
        return(esio->SocketFd);
    }
            
    return (0);	
}


/* -------------------------------------------------------- */
/*  return :  1   get vaild response                        */
/*  	      0   TCP Connection closed                     */
/*           -1  Timeout                                    */
/*           -2  select system error                        */
/* -------------------------------------------------------- */
int  get_response_ESIO(int SocketFd, int esioid, byte	*rxbuf)
{
    int     result;
    word    errCount;
    int		rxcnt;
    int     reqSize;

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
        
        if(++errCount > 100)	return (-1);
       	usleep(100);
        
    }while(rxbuf[0] != 0x05);



    /* ------------------------------------ */
    /*  TCP/IP Socket Error 발생시 ...      */
    /* ------------------------------------ */
	if(result == 0)	return 0;

    /* read STX, SIZE */
	if((result =TKreadn(SocketFd, (char *) &rxbuf[1], 3, 100)) < 0)	return (-2);
	if(rxbuf[1] != 0x64)	                        return (-2);	
    
    rxcnt = 4;
    reqSize = (rxbuf[2]*256 + rxbuf[3]) - 4;   
     
    /* read BODY */
    if((result =TKreadn(SocketFd, (char *) &rxbuf[rxcnt], reqSize, 1000)) != reqSize)	
    {
        if(opr->esioDebug == esioid)
        Debug(console, "esio%d> * Tail not Received... req=%d, rcv=%d\n", esioid, reqSize, result);     
        return (-2);
    }

    /* receive body */
    rxcnt = reqSize + 4;
        
    return (rxcnt);
}

int  tk_get_response_ESIO(int SocketFd, int esioid, byte	*rxbuf)
{
    int     result;
    word    errCount;
    int		rxcnt;
    int     reqSize;
   
    
   int    timeout ;
   struct timeval start ;  

    rxcnt = 0;
    errCount = 0;
    
    bzero(rxbuf, 10) ;
    /* ---------------------------------------- */
    /*  READ MODBUS - Header    */
    /*   엥    일단 0x05 0x64                                           */
    /* ---------------------------------------- */ 
    // STEP1  Check 0x05     
    reqSize=1 ; /* 05 */
    timeout = 2000 ; 
    do{
           // 1 byte일때는 의미가 없다.
           // gettimeofday( &start, NULL) ;  


		if((result = tk_net_readn(SocketFd, (char *) &rxbuf[0], reqSize, timeout)) <=0)
		{
	        // step 1  timeout
	        if(result == TK_TIMEOUT ) /* timeout -1 */
	        {
                if(opr->esioDebug == esioid)
                Debug(console,"SCAN> esio%02d, MAGIC Number Read Timeout [%d]... \n", esioid, result);		
                return TK_TIMEOUT ;  	     			        
	        }

	        // step2  EOF, 0
	        if(result == TK_SOCKET_EOF ) 
	        {

                if(opr->esioDebug == esioid)
                Debug(console,"SCAN> esio%02d, Peer close retVal[%d]... \n", esioid, result);			            	       		     			        
	        	return TK_SOCKET_EOF ; 
	        }
            else // system error 
            {

		        Debug(console,"SCAN-ERR> esio%02d, Socket Err [%s] \n", esioid, strerror(errno));
		        return TK_SYS_ERR ; 
            } 		    
		    

	 				    
		}

       // reqSize = reqSize - result  ;
       reqSize = 0;  

        
    }while( reqSize > 0 );




    if ( rxbuf[0] != 0x05 )
    {	

                if(opr->esioDebug == esioid)
                Debug(console,"SCAN> esio%02d, MAGIC Number Err [%02X]\r\n", esioid, rxbuf[0] );	
  
            	return TK_DNP_PROTOCOL_ERR ; 
    
    }    

    // STEP2  Check 0x05 
    reqSize=3 ; /*  06 SIZE1 SIZE2 */  
    timeout = 2000 ;     

    
    do{
          gettimeofday( &start, NULL) ;  


		if((result = tk_net_readn(SocketFd, (char *) &rxbuf[1], reqSize, timeout)) <=0)
		{
	        // step 1  timeout
	        if(result == TK_TIMEOUT ) /* timeout -1 */
	        {
                if(opr->esioDebug == esioid)
                Debug(console,"SCAN> esio%02d, MAGIC Number Read Timeout [%d]... \n", esioid, result);		
                return TK_TIMEOUT ;  	     			        
	        }

	        // step2  EOF, 0
	        if(result == TK_SOCKET_EOF ) 
	        {

                if(opr->esioDebug == esioid)
                Debug(console,"SCAN> esio%02d, Peer close retVal[%d]... \n", esioid, result);			            	       		     			        
	        	return TK_SOCKET_EOF ; 
	        }
            else // system error 
            {

		        Debug(console,"SCAN-ERR> esio%02d, Socket Err [%s] \n", esioid, strerror(errno));
		        return TK_SYS_ERR ; 
            } 		    
		    
		    
		}

        reqSize = reqSize - result  ;
	    
	    timeout -= elapsed_msec(start);        
        if ( timeout <= 0)   return TK_TIMEOUT ;  
        
    }while( reqSize > 0 );


	if(rxbuf[1] != 0x64)	                        return ( TK_DNP_PROTOCOL_ERR );	
    
 
     // STEP3  read body    
    rxcnt = reqSize = (rxbuf[2]*256 + rxbuf[3]) - 4;             
    /* SIZE 오류는 확인하지 않나 ??? */ 
    if ( reqSize > 4090)
    {
                if(opr->esioDebug == esioid)
                Debug(console,"SCAN> esio%02d, Size overflow [%02d]\r\n", esioid, reqSize );	  
            	return TK_DNP_PROTOCOL_ERR ;         
        
    }       
        
    timeout = 2000 ;     
    
    do{
          gettimeofday( &start, NULL) ;  

		if((result = tk_net_readn(SocketFd, (char *) &rxbuf[4], reqSize, timeout)) <=0)
		{
	        // step 1  timeout
	        if(result == TK_TIMEOUT ) /* timeout -1 */
	        {
                if(opr->esioDebug == esioid)
                Debug(console,"SCAN> esio%02d, * Tail not Received... req=%d, rcv [%d]... \n", esioid, rxcnt, reqSize);		
                return TK_TIMEOUT ;  	     			        
	        }

	        // step2  EOF, 0
	        if(result == TK_SOCKET_EOF ) 
	        {

                if(opr->esioDebug == esioid)
                Debug(console,"SCAN> esio%02d, Peer close retVal[%d]... \n", esioid, result);			            	       		     			        
	        	return TK_SOCKET_EOF ; 
	        }
            else // system error 
            {

		        Debug(console,"SCAN-ERR> esio%02d, Socket Err [%s] \n", esioid, strerror(errno));
		        return TK_SYS_ERR ; 
            } 		    
		    
		    
		}

        reqSize = reqSize - result  ;
	    
	    timeout -= elapsed_msec(start);        
        if ( timeout <= 0)   return TK_TIMEOUT ;  
        

        
    }while( reqSize > 0 );    
     
 #if 0    
    /* read BODY */
    if((result =TKreadn(SocketFd, (char *) &rxbuf[rxcnt], reqSize, 1000)) != reqSize)	
    {
        if(opr->esioDebug == esioid)
        Debug(console, "esio%d> * Tail not Received... req=%d, rcv=%d\n", esioid, reqSize, result);     
        return (-2);
    }
#endif 
    /* receive body */
    rxcnt = rxcnt  + 4;
        
    return (rxcnt);
}



/* ==================================================================== */
/*  ESIO 데이터 수신부....                                              */
/*  - 전력감시(ESIO1),원격진단(ESIO2), 전력품질(ESIO3)                  */
/* ==================================================================== */
int rcvHandler_ESIO(int esioid, byte *sndbuf, byte *rxbuf, int rxcnt)
{
    int     i, index;
    int     opcode;
    int     count, sndCount;
    int     startPoint, rcvPoint;
    int     devNo, devNum;
    int     eventNum, dumpPoint, soe_size;
    int     anapoint, stsByteCnt;
    int     dataLen, reqDataLen;
    int     rcvCntrDEV, rcvCntrPT, rcvCntrTCF, rcvCntrSTS;
            
    byte    devSts;
    byte    *bfptr;
    float   *fptr;
    float   fdata;
    word    chksum, rcvsum;
    //byte    soeTime[6];
    
    SDP_DEVICE      *dev;
    ESIO_CONFIG     *esio;

// 2026-03-06 오후 2:44:22
    MPU_SOEQ_ENTRY  event;
    MPU_SOEQ_ENTRY_PACKED *event_packed;

 
 
 	// 2026-10-07 오후 4:15:03 
	struct timeval  tv ;
	char  buf[100];
    
    esio = (ESIO_CONFIG *) esioCFG[esioid];
    
    opcode = rxbuf[5] & 0x7f;           // 수신 OPCODE
    
    esio->rcvStatus = rxbuf[6];         // 수신 ESIO Status 정보;
    esio->rcvSeqNo  = rxbuf[7];         // 수신 Seqno

    /* ------------------------------------ */
    /* SLAVE-MPU 상태에 따른 OPCODE 처리    */
    /* ------------------------------------ */
    if(esio->rcvStatus & 0xc0)           esio->timeSyncReq = SET;
    else if(esio->rcvStatus & 0x20)      esio->chksumReq   = SET;        
                           
    count = 0;
    
  // 수신 OPCODE   
    switch(opcode)
    {
    case SIM_SDP_GPOLL:             // 0x00 SDP G-POLL    
{        
        devNo     = rxbuf[8] - 1;
        devSts    = rxbuf[9];
        
        if((devNo < 0) || (devNo >= MAX_DEVICE))
        {
            Debug(console,"esio%d> *rcv GPOLL ...*Invlaid Dev=%d \n", esioid, devNo + 1);
            return (0);
        }     
        
        /* ---------------------------------------- */
        /*  계전기별 Online 상태 Update...          */
        /* ---------------------------------------- */
        dev = (SDP_DEVICE *) deviceCFG[devNo];
        
        if((opr->esioDebug == esioid) || (opr->dnpDebug == devNo))
        // esioid는 +1이 없네 위에서 내려오나 ???
        Debug(console,"esio%d> rcv GPOLL ...Dev= %02d \n", esioid, devNo + 1);	
        
        /* ------------------------------------ */
        /*  SLAVE 모드시....continue            */
        /* ------------------------------------ */
        if(opr->runMode == LOCAL_SLAVE) 
        {
            dev->oldRcvCount = 0;
            dev->comRcvCount = 0;
            dev->comSndCount = 0;
            dev->rcvIIN[0] = 0;
            dev->rcvIIN[1] = 0;
            break;
        }
        
        if(devSts & 0x01)   
        {
            dev->online     = 1;
            dev->runStatus  = 1;            // Simulator : 계전기 통신정상
        }
        else                
        {
            dev->online     = 0;
            dev->runStatus  = 2;            // Simulator : 계전기 통신이상
        }
        
        dev->rcvIIN[0] = rxbuf[10];
        dev->rcvIIN[1] = rxbuf[11];
        dev->comSndCount = (rxbuf[12]*256) + rxbuf[13];
        dev->comRcvCount = (rxbuf[14]*256) + rxbuf[15];
        dev->oldRcvCount = (rxbuf[16]*256) + rxbuf[17]; 
        
        
        	    
        return (0);
        break;
}
                
    case SIM_EVENT_DUMP:            // 0x02 SDP EVENT Dump,  요거 밑에서 올려 주나보나..
    {
        eventNum = rxbuf[8];
        dataLen  = rxcnt - 10;      // 순수 SOE 정보만...
        
        /* -------------------------------------------- */
        /*  ESIO 수신 SOE Size Check    ....            */
        /* -------------------------------------------- */
        // 2026-03-06 오후 2:44:54 
        soe_size = sizeof(MPU_SOEQ_ENTRY_PACKED);
        reqDataLen = eventNum * soe_size;


            Debug(console, "esio%d> *rcv SOE-DUMP Count : event num=%d (size=%d), rcv=%d, req=%d !\n", esioid, 
                eventNum,soe_size,dataLen, reqDataLen);

        
        // 2026-05-29 오후 4:48:29 
        if(dataLen != reqDataLen)
        {
            Debug(console, "esio%d> *rcv SOE-DUMP Count Fail : event num=%d (size=%d), rcv=%d, req=%d !\n", esioid, 
                eventNum,soe_size,dataLen, reqDataLen);
            
            sndbuf[0] = 0x05;                       // 전송 구분자#1                
            sndbuf[1] = 0x64;                       // 전송 구분자#2  
            sndbuf[2] = 0;                          // size             
            sndbuf[3] = 0;                          // size        
            sndbuf[4] = esioid;                     // 대상 ESIO Address, [1..5]
            sndbuf[5] = SIM_EVENT_ACK;              // Opcode
            sndbuf[6] = mpuCFG->mpuStatus;          // MPU 상태정보 status, MASTER/SLAVE, SDP-A/SDP-B     
            sndbuf[7] = esio->rcvSeqNo;             // ESIO 수신 SeqNo       
            sndbuf[8] = eventNum;                   // EVENT 갯수
            sndbuf[9] = 1;                          // 수신 실패      
            count = 10;
        
            Debug(console,"esio%d> *send EVENT-ACK Fail ...Num=%d.. .!\n", esioid, eventNum);    
            break;
        }
        
        if(opr->soeDebug)
        Debug(console,"esio%d> rcv EVENT...Num=%d.. .!\n", esioid, eventNum);
            
        /* -------------------------------------------- */
        /*  ESIO 수신 SOE 정보 => MPU SOE 정보 Update   */
        /* -------------------------------------------- */
        for(i = 0; i < eventNum; i++)
        {
          
            /* 2026-03-05 오후 7:46:44 esio 에서 time_t 확인하자. */
            event_packed = (MPU_SOEQ_ENTRY_PACKED *) &rxbuf[9 + i*soe_size];
            
            // 여기서 packed->unpacked로 복사 하고 ..timeval endian 처리 

            bzero( &event, sizeof( MPU_SOEQ_ENTRY )) ;
            event.eventCode    = event_packed->eventCode ;
            event.devNo        = event_packed->devNo ;
// 2026-05-29 오후 4:34:42  0-->1

// 2026-06-08 오후 2:24:45 신기하네 ARM-ARM도에서도 잘 나오는데 ntoh 때문인가.

#if  __PPC_ARCH__         
            event.pointNo      = htons ( event_packed->pointNo) ;
 // void ntoh_timeval_safe(struct timeval *out, const void *in_src)           
            ntoh_timeval_safe(&event.updateTime, (const void *)&event_packed->updateTime  )   ;    

#else
            
            event.pointNo      =  ( event_packed->pointNo) ;
            memcpy(&event.updateTime, (const void *)&event_packed->updateTime,  sizeof(struct timeval) )   ;    
         //   printf("ARM-ARM Convesion\r\n");
#endif 
            event.state        = event_packed->state ;                                                
      
 // 자 출력한번 해주자...
         if(opr->soeDebug)
         {   
	        gettimeofday(&tv, NULL);
            // 2026-10-07 오후 4:16:11 
		    Debug(console,"\r\n%s : ", sprt_timeval (buf, sizeof(buf), &tv));            
            Debug(console,"eventCode[%d], devNo[%d], pointNo[%d], State[%d]\r\n", 
             event.eventCode, event.devNo , event.pointNo , event.state  ); 
            print_timeval( event.updateTime);
          }              

           
            
            event.esioNo = opr->cpuMode;           // 이벤트 발생주체 : [0] CPU-A, [1] CPU-B
            
            if(event.eventCode == ENT_DEV_ONLINE)          set_Online_Device(shmPtr,&event);     
            else if(event.eventCode == ENT_DEV_OFFLINE)    set_Offline_Device(shmPtr,&event);            
            else if(event.eventCode == ENT_SOE)            store_MPU_SOE( shmPtr, &event);
                 
        }
        
        /* ------------------------------------ */
        /*  EVENT-ACK Message ...               */
        /* ------------------------------------ */
        if(eventNum)
        {            
            sndbuf[0] = 0x05;                       // 전송 구분자#1                
            sndbuf[1] = 0x64;                       // 전송 구분자#2  
            sndbuf[2] = 0;                          // size             
            sndbuf[3] = 0;                          // size        
            sndbuf[4] = esioid;                     // 대상 ESIO Address, [1..5]
            sndbuf[5] = SIM_EVENT_ACK;              // Opcode
            sndbuf[6] = mpuCFG->mpuStatus;          // MPU 상태정보 status, MASTER/SLAVE, SDP-A/SDP-B     
            sndbuf[7] = esio->rcvSeqNo;             // ESIO 수신 SeqNo       
            sndbuf[8] = eventNum;                   // EVENT 갯수
            sndbuf[9] = 0;                          // 수신 성공      
            count = 10;
            
            if(opr->soeDebug)
            Debug(console,"esio%d> snd EVENT-ACK ...Num=%d.. .!\n", esioid, eventNum);
            
        }
        break;
    }
    case SIM_POINT_SCAN:        // 0x04 계전기 POINT 정보 Scan
        devNo     = rxbuf[8] - 1;
        devSts    = rxbuf[9]; // 아. 포인트 상태가 아니고 계전기 상태이다.
        
        if((devNo < 0) || (devNo >= MAX_DEVICE))
        {
            if((opr->esioDebug == esioid) || (opr->dnpDebug == devNo))
            Debug(console,"esio%d> *rcv STATUS Dump ...*Invlaid Dev=%d \n", esioid, devNo + 1);
            dev->statusDump  = RESET;
            dev->init_status = SET;     // 초기 상태덤프 후 ... 이벤트 발생
            dev->stsByteCnt  = 0;
            return (0);
        }     
        
        /* ---------------------------------------- */
        /*  계전기별 Online 상태 Update...          */
        /* ---------------------------------------- */
        dev = (SDP_DEVICE *) deviceCFG[devNo];
        if(devSts & 0x01)   
        {
            dev->online     = 1;
            dev->runStatus  = 1;            // Simulator : 계전기 통신정상
        }
        else                
        {
            dev->online     = 0;
            dev->runStatus  = 2;            // Simulator : 계전기 통신이상
        }
        
        dev->rcvIIN[0] = rxbuf[10];
        dev->rcvIIN[1] = rxbuf[11];
        dev->comSndCount = (rxbuf[12]*256) + rxbuf[13];
        dev->comRcvCount = (rxbuf[14]*256) + rxbuf[15];
        dev->oldRcvCount = (rxbuf[16]*256) + rxbuf[17]; 
        
        /* ---------------------------------------- */
        /*  계전기별 상태데이터 Update...           */
        /* ---------------------------------------- */
        stsByteCnt= rxbuf[18];          // 상태 덤프 Byte 수
        dataLen   = rxcnt - 20;         // 순수 상태 데이터 수...
        
        if(dataLen != stsByteCnt)
        {
        	if((opr->esioDebug == esioid) || (opr->dnpDebug == devNo))
            Debug(console, "esio%d> *rcv STATUS-DUMP Fail : mismatch count, rcv=%d, req=%d !\n", esioid, dataLen, stsByteCnt);
            dev->statusDump  = RESET;
            dev->init_status = SET;     // 초기 상태덤프 후 ... 이벤트 발생
            dev->stsByteCnt  = 0;
            return (0);
        }
        
        /* 상태정보 Update... */
        if((stsByteCnt > 0) && (stsByteCnt <= MAX_DI_DATA_NUM))  
        {
        	if((opr->esioDebug == esioid) || (opr->dnpDebug == devNo))
        	{
            	Debug(console, "esio%d> rcv STATUS Dump ... dev= %02d [size= %2d] \n", esioid, devNo+1, stsByteCnt);
	            //DumpBuff(console," ", &rxbuf[19], stsByteCnt);
    	    }
    	    
            curStore_BIT_MPU( shmPtr, devNo, &rxbuf[19], stsByteCnt);
            
            /* -------------------------------------------------------- */
            /* ESIO 수신 ...장치별 상태정보 Update...                   */
            /* -------------------------------------------------------- */
            memcpy(dev->cursts, (char *) &rxbuf[19], stsByteCnt);
            dev->stsByteCnt = stsByteCnt;
        }
        else if(stsByteCnt > MAX_DI_DATA_NUM)
        {
        	if((opr->esioDebug == esioid) || (opr->dnpDebug == devNo))
        	Debug(console, "esio%d> *** rcv STATUS Dump OVER... dev= %02d, size= %2d [max=%d]\n", esioid, devNo+1, stsByteCnt, MAX_DI_DATA_NUM);
        	dev->stsByteCnt = 0;
        }	  
        else
        {
            dev->statusDump  = RESET;
            dev->init_status = SET;     // 초기 상태덤프 후 ... 이벤트 발생
            dev->stsByteCnt  = 0;
        }
             
        return (0);    
        break;
        
    case SIM_ANALOG_SCAN:       // 0x14 계전기 아날로그 정보 Scan
        {
        devNo     = rxbuf[8] - 1;
        devSts    = rxbuf[9];
        
        if((devNo < 0) || (devNo >= MAX_DEVICE))
        {
            if((opr->esioDebug == esioid) || (opr->dnpDebug == devNo))
            Debug(console,"esio%d> *rcv ANALOG Dump ...*Invlaid Dev=%d \n", esioid, devNo +1);
            return (0);
        }     
        
        /* ---------------------------------------- */
        /*  계전기별 Online 상태 Update...          */
        /* ---------------------------------------- */
        dev = (SDP_DEVICE *) deviceCFG[devNo];
        if(devSts & 0x01)   
        {
            dev->online     = 1;
            dev->runStatus  = 1;            // Simulator : 계전기 통신정상
        }
        else                
        {
            dev->online     = 0;
            dev->runStatus  = 2;            // Simulator : 계전기 통신이상
        }
        
        dev->rcvIIN[0] = rxbuf[10];
        dev->rcvIIN[1] = rxbuf[11];
        dev->comSndCount = (rxbuf[12]*256) + rxbuf[13];
        dev->comRcvCount = (rxbuf[14]*256) + rxbuf[15];
        dev->oldRcvCount = (rxbuf[16]*256) + rxbuf[17]; 
            
        /* ---------------------------------------- */
        /*  수신된 포인트 수만큼... Update...       */
        /* ---------------------------------------- */
        dumpPoint = rxbuf[18]*256 + rxbuf[19];
        
        dataLen   = rxcnt - 20;         // 순수 계측 데이터 수...
        dataLen   = rxcnt - 21;         // 순수 계측 데이터 수...
        
        if(dataLen != (dumpPoint * 6))
        {
        	if((opr->esioDebug == esioid) || (opr->dnpDebug == devNo))
            Debug(console, "esio%d> *rcv ANALOG-DUMP Fail : mismatch count, dump=%d, rcv=%d, req=%d !\n", esioid, dumpPoint, dataLen, (dumpPoint * 6));
            return (0);
        }
            
        bfptr = (byte *) &rxbuf[20];         // 계측데이터 시작지점...
        index = 0;
        
        for(i=0; i< dumpPoint; i++)
         {
            index = i * 6;
            anapoint = bfptr[index + 0] * 256 + bfptr[index + 1];   // 계측포인트 번호 (0,1,2,...)


            

//2026-05-29 오후 12:43:02 1->0
#ifdef __ARM_ARCH__    //PPC 방식    ARM-PPC  ..코멘트 오류  여기가 ARM 이고
        // memcpy ..가 좋은데..
        #if 0
                fptr = (float *) &bfptr[index + 2];                     // 계측 데이터(Float)                                      
                /* 기본 데이터 지정 */
                fdata = (float ) *fptr;
        #else
                memcpy(&fdata, &bfptr[index + 2],4);
                fptr = &fdata;                

//            	    if((opr->esioDebug == esioid) || (opr->dnpDebug == devNo))
//                                    Debug(console,"esio[%d] ARM-Coversion PT[%d] :  %05.2f\r\n", esioid , anapoint, fdata);     

        
        #endif 

#else // hkkim  정광영과 이것으로 시험했으니 이겟 PPC 일듯.
                {
                    float a ;                                                    
                    a=BITS_TO_FLOAT (bfptr[index + 5],bfptr[index + 4],bfptr[index + 3],bfptr[index + 2]);
            	    
//            	    if((opr->esioDebug == esioid) || (opr->dnpDebug == devNo))
//                                    Debug(console,"esio[%d] PPC-Conversion PT[%d] :  %05.2f\r\n", esioid , anapoint, a);                 
                    fdata=a ; 
                    fptr = &fdata;                 
                }
                 
#endif



            /* ---------------------------------------- */
            /* 2014.12.11 수신된 POINT 예외처리...      */
            /* ---------------------------------------- */
            if(anapoint >= MAX_DEV_AI_POINT)
            {
            	if((opr->esioDebug == esioid) || (opr->dnpDebug == devNo))
                Debug(console,"esio%d> rcv dev=%2d, *Analog Dump - *Invalid AI-Point (%d) \n", esioid, devNo+1, anapoint);
                break;
            }
                
            /* ------------------------------------ */               
            /* 계측포인트 데이터.. Update ...       */
            /* ------------------------------------ */      
            storeAI_FLOAT_MPU( shmPtr, devNo, anapoint, fdata, (byte *)fptr);
        }
        if((opr->esioDebug == esioid) || (opr->dnpDebug == devNo))                               
        Debug(console,"esio%d> rcv ANALOG DUMP ...dev= %02d [dump=%02d] \n", esioid, devNo+1, dumpPoint);
        return (0);    
        break;
        }
    case SIM_POINT_CNTR:        // 0x05 계전기 POINT 제어
        rcvCntrDEV = rxbuf[8];              // 대상 계전기 번호 [1..64]
        rcvCntrPT  = rxbuf[9];              // 대상 포인트 [1..256]
        rcvCntrTCF = rxbuf[10];             // 제어상태, [1] TRIP, [2] CLOSE
        rcvCntrSTS = rxbuf[11];             // 제어상태, [0] 실패, [1] 성공
        
        if(opr->soeDebug)
        Debug(console,"esio%d> rcv Point Control ...rcv, [Dev=%d, point=%d, TCF=%d] => [STS=%d]\n", esioid, rcvCntrDEV, rcvCntrPT, rcvCntrTCF, rcvCntrSTS);
        
        /* ------------------------------------ */
        /*  ESIO 계전기-제어 Clear...           */   
        /* ------------------------------------ */    
        esio->cntDev    = 0;
        esio->cntPoint  = 0;
        esio->cntTCF    = 0;
        esio->cntFlag   = 0;
        return (0);
        break;

    case SIM_TIME_DOWN:         // 0x11 Time Sync Down
    {
        esio->timeSyncReq = RESET;
        esio->chksumReq   = SET; 
        
        if(opr->esioDebug == esioid) 
    	Debug(console, "esio%d> rcv TIME Sync Down ... \n", esioid);
        return (0);
        break;
    }                
    case SIM_CHKSUM_DUMP:       // 0x16 데이터베이스 Chksum Scan       
    {
        rcvsum = rxbuf[8]*256 + rxbuf[9];

#if 0
        chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
#else
        chksum = gensum((byte *)&esio_rtudb, sizeof(ESIO_RTU_DATABASE) - 2);        
#endif 


        
        if(opr->esioDebug == esioid) 
	    {	
	        Debug(console,"----------------------------------------------\n");    
		    Debug(console,"esio%d> rcv Chksum = %4x, romsum = %4x\n", esioid, chksum, rcvsum);
		}
		
		esio->chksumReq = RESET; // 주기적으로 RESET이 되니 일단 틀려도 여기서 RESET 해주네..
		
		if(chksum != rcvsum)
		{
			Debug(console,"esio%d> *** ESIO(%d) Check DB Failed....!\n", esioid, esioid);
			Debug(console,"esio%d> rcv CHKSUM  : rcv = %4x rom = %4x ....\n", esioid, rcvsum, chksum);
			
			esio->mpuCfgDown     = SET;
			esio->esioCfgDown    = SET;
			esio->dbDownEsio     = 1;
			
			esio->calptCfgDown   = SET;         // CAL-POINT Config 전송요구
            esio->dbDownCalpt    = 1;    
            
            esio->hostCfgDown    = SET;
			esio->iccpCfgDown    = SET;
			esio->harrisCfgDown  = SET;
			esio->landisCfgDown  = SET;
			esio->modbusCfgDown  = SET;
			esio->scanCfgDown    = SET;
			esio->deviceCfgDown  = SET;
			esio->dbDownDevice   = 1;           // 1번 계전기 번호
			
			esio->pointCfgDown   = SET;
			esio->dbDownPoint    = 1;
			esio->dbDownIndex    = 16;

			esio->endPointFlag   = RESET;
			esio->endDeviceFlag  = RESET;
			
			esio->dbDownFlag     = SET;
		}
		return (0);
        break;
    }
        
    case SIM_RESTART_ESIO:      // 0x18 ESIO Restart 제어
        if(opr->esioDebug == esioid) 
    	Debug(console,"esio%d> rcv ESIO-RESTART  ... \n", esioid);
        return (0);
        break;    


    case SIM_MPUCFG_DOWN:      // 0x22 DB : ESIO Config Down
        esio->mpuCfgDown = RESET;
        
        // if(opr->esioDebug == esioid) 
    	printf("esio%d> rcv MPU Config DB Down ... \n", esioid);
    	Debug(console,"esio%d> rcv MPU Config DB Down ... \n", esioid);
        return (0);
        break;
                
    case SIM_ESIOCFG_DOWN:      // 0x22 DB : ESIO Config Down
        devNo  = rxbuf[8];          // ESIO # [1..5]
        devNum = rxbuf[9];          // 전송 ESIO 갯수 (default = 1)
        
        esio->dbDownEsio = devNo + 1;
        if((esio->dbDownEsio > MAX_DB_ESIO) || (esio->endEsioFlag == SET))
        {       
            esio->esioCfgDown = RESET;
        }
        
        if(opr->esioDebug == esioid) 
    	Debug(console,"esio%d> rcv ESIO (%d) DB Down ... \n", esioid, devNo);
        return (0);
        break;

    case SIM_HOST_DOWN:         // 0x24 DB : HOST Config Down
        esio->hostCfgDown = RESET;
        
		if(opr->esioDebug == esioid) 
    	Debug(console,"esio%d> rcv HOST Config Down ... \n", esioid);
        return (0);
        break;
        
    case SIM_ICCP_DOWN:         // 0x26 DB : ICCP-HOST Config Down
        esio->iccpCfgDown = RESET;
        if(opr->esioDebug == esioid) 
    	Debug(console,"esio%d> rcv ICCP-HOST Config Down ... \n", esioid);
        return (0);
        break;
        
    case SIM_HARRIS_DOWN:       // 0x28 DB : HARRIS Config Down
        esio->harrisCfgDown = RESET;
        if(opr->esioDebug == esioid) 
    	Debug(console,"esio%d> rcv HARRIS-HOST Config Down ... \n", esioid);
        return (0);
        break;
        
    case SIM_LANDIS_DOWN:       // 0x2A DB : LANDIS Config Down
        esio->landisCfgDown = RESET;
        if(opr->esioDebug == esioid) 
    	Debug(console,"esio%d> rcv LANDIS-HOST Config Down ... \n", esioid);
        return (0);
        break;
                
    case SIM_MODBUS_DOWN:       // 0x2C DB : MODBUS Config Down
        esio->modbusCfgDown = RESET;
        
        if(opr->esioDebug == esioid) 
    	Debug(console,"esio%d> rcv MODBUS DB Down ... \n", esioid);
        return (0);
        break;
        
    case SIM_SCAN_DOWN:         // 0x30 DB : SCAN Config Down
        esio->scanCfgDown = RESET;
        
        if(opr->esioDebug == esioid) 
    	Debug(console,"esio%d> rcv SCAN DB Down ... \n", esioid);
        return (0);
        break;
        
    case SIM_DEVICE_DOWN:       // 0x32 DB : DEVICE Config Down
        devNo  = rxbuf[8];      // 시작 계번기 번호 [1..64]
        devNum = rxbuf[9];      // 전송 계전기 수
        
        esio->dbDownDevice = devNo + devNum;
        
        if(opr->esioDebug == esioid) 
    	Debug(console,"esio%d> rcv DEVICE DB Down ... %d \n", esioid, devNo);
    	
        if((esio->dbDownDevice >= MAX_DB_DEVICE) || (esio->endDeviceFlag == 0x80))
        {       
            esio->deviceCfgDown = RESET;
            esio->endDeviceFlag = 0;
        }
        return (0);
        break;
        
    case SIM_POINT_DOWN:        // 0x34 DB : esio 에서 rx  POINT Config Down 처리
        startPoint = rxbuf[8]*256 + rxbuf[9];
		rcvPoint   = rxbuf[10];
		
		if(opr->esioDebug == esioid) 
    	Debug(console,"esio%d> rcv POINT DB Down ... start=%d , end = %d\n", esioid, startPoint, startPoint + rcvPoint);
    	
		esio->dbDownPoint = startPoint + rcvPoint;
		
		if(opr->esioDebug == esioid) 
    	Debug(console,"esio%d> esio->dbDownPoint =%d\n", esioid, esio->dbDownPoint);



		/* 포인트 DB 전송 종료... */		
		if((esio->dbDownPoint >= MAX_DB_POINT) || (esio->endPointFlag == 0x80))    
		{	

            // esio->endPointFlag 이건  sim에서 내렬 올때....즉 이때는 1회만 .
            // esio->dbDownPoint >= MAX_DB_POINT 이건 checksump으,로
			esio->pointCfgDown = RESET;
			esio->endPointFlag = 0;
			
			esio->chksumReq    = SET;
			esio->dbDownFlag   = RESET;
			
			if(opr->esioDebug == esioid) 
			Debug(console,"esio%d>  rcv ESIO POINT-DB DOWN End...!\n", esioid);
		
		    msleep(500);
		}
        return (0);
        break;

   case SIM_CAL_POINT_DOWN:        // 0x34 DB : POINT Config Down
        startPoint = rxbuf[8]*256 + rxbuf[9];
		
		if(opr->esioDebug == esioid) 
    	Debug(console,"esio%d> rcv CAL-POINT DB Down ... start=%d , end = %d\n", esioid, startPoint, rxbuf[10]);
    	
		esio->dbDownCalpt = startPoint + 16;         // 16 포인트씩 전송

        /* ---------------------------------------- */
		/* 포인트 DB 전송 종료...                   */	
		/* ---------------------------------------- */	
		if(esio->dbDownCalpt >= MAX_DB_CAL_POINT)
		{	
			esio->calptCfgDown = RESET;
			esio->dbDownCalpt  = RESET;
		}
        return (0);
        break;
                
	default :
		break;	
	}


    /* -------------------------------- */                      
    /* 송신 Frame 구성                  */                      
    /* -------------------------------- */         	
    sndCount = count + 1;                   // Packet 내 Address ~ DF 까지의 데이터 수
                     
    sndbuf[2] = (sndCount >> 8) & 0xff;     /* Total size: MSB */
    sndbuf[3] = sndCount & 0xff;            /* Total size: LSB */

    sndbuf[count] = genlrc(sndbuf, count);
    count++;
    
    return(count);
    
}

extern void copy_rtudb_for_esio(  ESIO_RTU_DATABASE *dest,   RTU_DATABASE  *src) ;
/*
*   ESIO - SCAN TASK : MASTER-CPU 메세지 구성
*/
int	 makeLink_ESIO(ESIO_CONFIG *esio, int esioid,  byte *sndbuf)
{
    int     opcode;
    int     count, sndCount;
    int     dbSize, sndDevNo=0;
    int     startPoint, reportPoint, pointNum;
    int     comDevid, devNum;
    word    chksum;
    
    SDP_DEVICE      *dev;
    DB_ESIO_CONFIG  *dbESIO;
    DB_HOST_CONFIG  *dbHost;
    
    /* ---------------------------- */
	/*	OPCODE 처리 				*/
	/* ---------------------------- */
	if(opr->runMode == LOCAL_MASTER)
    {	    
    	if(esio->timeSyncReq)		    opcode = SIM_TIME_DOWN;
        else if(esio->cntFlag == SET)   opcode = SIM_POINT_CNTR;        // ESIO# 제어정보 전송 	    
        // 데이터베이스 전송관련     
	    else if(esio->chksumReq)		opcode = SIM_CHKSUM_DUMP;	
	    else if(esio->mpuCfgDown)       opcode = SIM_MPUCFG_DOWN;    	// MPU Config 전송요구        
    	else if(esio->esioCfgDown)      opcode = SIM_ESIOCFG_DOWN;    	// ESIO Config 전송요구    
    	else if(esio->hostCfgDown)      opcode = SIM_HOST_DOWN;	        // HOST Config 전송요구
    	else if(esio->iccpCfgDown)	    opcode = SIM_ICCP_DOWN;	        // ICCP Config 전송요구
    	else if(esio->harrisCfgDown)	opcode = SIM_HARRIS_DOWN;	    // HARRIS Config 전송요구
	    else if(esio->landisCfgDown)	opcode = SIM_LANDIS_DOWN;	    // LANDIS Config 전송요구            
	    else if(esio->modbusCfgDown)	opcode = SIM_MODBUS_DOWN;	    // MODBUS Config 전송요구
        else if(esio->scanCfgDown)	    opcode = SIM_SCAN_DOWN;         // SCAN Config 전송요구
        else if(esio->deviceCfgDown)	opcode = SIM_DEVICE_DOWN;       // DEVICE Config 전송요구
        else if(esio->calptCfgDown)     opcode = SIM_CAL_POINT_DOWN;    // CAL-POINT Config 전송요구     
    	else if(esio->pointCfgDown)     opcode = SIM_POINT_DOWN;        // POINT Config 전송요구
	    else
    	{
	        /* ------------------------------------ */
	        /* 계전기 상태/아날로그 정보 연계       */
    	    /* ------------------------------------ */
            if(++esio->scanIndex >= esio->scanMaxNum) 
            {
                esio->scanIndex = 0;
                esio->chgOpcode = (esio->chgOpcode + 1) & 0x01;
            }

            sndDevNo = esio->scanDevice[esio->scanIndex] & 0x3f;
        
            if((sndDevNo <= 0) || (sndDevNo > MAX_DEVICE))
            {
                opcode = SIM_SDP_GPOLL;
            }
            else
            {                   
                dev = (SDP_DEVICE *) deviceCFG[sndDevNo - 1];
                comDevid = dev->comDevID;
                
                /* ------------------------------------------------ */
                /*  계전기별 상태/계측정보 수집...                  */
                /*  - 상태Dump : 초기 & 상태Dump 주기시             */      
                /* ------------------------------------------------ */         
                if(dev->scan == 0)      opcode = SIM_SDP_GPOLL;     
                else
                { 
                    if(dev->devDiPoint)
                    {
                        if(dev->init_status == RESET)   opcode = SIM_POINT_SCAN;    // 계전기 상태포인트 정보 
                        else
                        {
                            if(dev->statusDump == SET)  opcode = SIM_POINT_SCAN;    // 계전기 상태포인트 정보 
                            else if(dev->devAiPoint)    opcode = SIM_ANALOG_SCAN;   // 계전기 계측포인트 정보   
                            else                        opcode = SIM_SDP_GPOLL;  
                        }
                    }
                    else
                    {
                        if(dev->devAiPoint)     opcode = SIM_ANALOG_SCAN;   // 계전기 계측포인트 정보    
                        else                    opcode = SIM_SDP_GPOLL;                                      
                    }
                }
            }    
	    
	    }    
    }
    else
    {
        /* ------------------------------------------------------------ */
        /*  LOCAL-SLAVE 인 경우....                                     */
        /* ------------------------------------------------------------ */
        if(esio->timeSyncReq)		    opcode = SIM_TIME_DOWN;
        else if(esio->chksumReq)		opcode = SIM_CHKSUM_DUMP;	
        else if(esio->mpuCfgDown)       opcode = SIM_MPUCFG_DOWN;    	// MPU Config 전송요구        
    	else if(esio->esioCfgDown)      opcode = SIM_ESIOCFG_DOWN;    	// ESIO Config 전송요구    
    	else if(esio->hostCfgDown)      opcode = SIM_HOST_DOWN;	        // HOST Config 전송요구
    	else if(esio->iccpCfgDown)	    opcode = SIM_ICCP_DOWN;	        // ICCP Config 전송요구
    	else if(esio->harrisCfgDown)	opcode = SIM_HARRIS_DOWN;	    // HARRIS Config 전송요구
	    else if(esio->landisCfgDown)	opcode = SIM_LANDIS_DOWN;	    // LANDIS Config 전송요구            
	    else if(esio->modbusCfgDown)	opcode = SIM_MODBUS_DOWN;	    // MODBUS Config 전송요구
        else if(esio->scanCfgDown)	    opcode = SIM_SCAN_DOWN;         // SCAN Config 전송요구
        else if(esio->deviceCfgDown)	opcode = SIM_DEVICE_DOWN;       // DEVICE Config 전송요구
        else if(esio->calptCfgDown)     opcode = SIM_CAL_POINT_DOWN;    // CAL-POINT Config 전송요구     
    	else if(esio->pointCfgDown)     opcode = SIM_POINT_DOWN;        // POINT Config 전송요구.  2026-08-27 오후 1:42:29 
        else    
       	{
       	    /* ------------------------------------ */
	        /* 계전기 상태/아날로그 정보 연계       */
    	    /* ------------------------------------ */
            if(++esio->scanIndex >= esio->scanMaxNum) 
            {
                esio->scanIndex = 0;
                esio->chgOpcode = (esio->chgOpcode + 1) & 0x01;
            }
                         
            sndDevNo = esio->scanDevice[esio->scanIndex] & 0x3f;
            
       	    opcode = SIM_SDP_GPOLL; 
       	    comDevid = 0;
	    }    
    }     
    
    if ( opcode == SIM_MPUCFG_DOWN ||  opcode == SIM_ESIOCFG_DOWN ||  opcode == SIM_HOST_DOWN ||  opcode == SIM_ICCP_DOWN ||  opcode == SIM_LANDIS_DOWN || \
     opcode == SIM_MODBUS_DOWN ||  opcode == SIM_SCAN_DOWN ||  opcode == SIM_DEVICE_DOWN ||  opcode == SIM_POINT_DOWN ||  opcode == SIM_CAL_POINT_DOWN     )
     {
        copy_rtudb_for_esio( &esio_rtudb, rtudb) ;

    /* 데이터베이스 저장 ... */
        
         chksum = gensum((byte *)&esio_rtudb, sizeof(ESIO_RTU_DATABASE) - 2);
        printf("SCAN> ESIO_RTU_DATABASE CHKSIM [%04X]\r\n",chksum );

     }


        // dbFileWriteESIO(opr, &esio_rtudb);

    
    /* -------------------------------- */                      
    /* 송신 Frame 구성                      */                      
    /* -------------------------------- */                      
    sndbuf[0] = 0x05;                       // 전송 구분자#1                
    sndbuf[1] = 0x64;                       // 전송 구분자#2  
    sndbuf[2] = 0;                          // size             
    sndbuf[3] = 0;                          // size        
    sndbuf[4] = esioid;                     // Address ...[1] ESIO1, [2] ESIO2, [3] ESIO3, [4] ESIO4, [5] RTU
    sndbuf[5] = opcode + 0x80;              // opcode          
    sndbuf[6] = mpuCFG->mpuStatus;          // MPU 상태정보 status, MASTER/SLAVE, SDP-A/SDP-B           
    sndbuf[7] = esio->sndSeqNo;             // 송신 SeqNo       
    count = 8;                                                  

    esio->sndOpcode = opcode & 0x7f;
    
    switch(opcode)
    {
    case SIM_SDP_GPOLL:             // 0x00 SDP G-POLL        
        if(opr->esioDebug == esioid)                                    
        Debug(console,"esio%d> snd GPOLL ...dev= %d[com=%d] \n", esioid, sndDevNo, comDevid);
        sndbuf[count++] = sndDevNo;     // 대상 계전기 번호, 1,2,3...
        break;
        
    case SIM_POINT_SCAN:                // 0x04 계전기 상태 POINT 정보 Scan
        if(opr->esioDebug == esioid)                                    
        Debug(console,"esio%d> snd STATUS DUMP ...dev= %02d [com=%02d] \n", esioid, sndDevNo, comDevid);
        sndbuf[count++] = sndDevNo;     // 대상 계전기 번호, 1,2,3...
        break;
        
    case SIM_ANALOG_SCAN:               // 0x14 계전기 아날로그 정보 Scan
        if(opr->esioDebug == esioid)                                    
        Debug(console,"esio%d> snd ANALOG DUMP ...dev= %d[com=%d] \n", esioid, sndDevNo, comDevid);
        sndbuf[count++] = sndDevNo;     // 대상 계전기 번호, 1,2,3...
        break;
    
    case SIM_POINT_CNTR:                // 0x05 계전기 POINT 제어
        sndbuf[count++] = esio->cntDev;        // 대상 계전기 번호 [1..64]
        sndbuf[count++] = esio->cntPoint;      // 대상 포인트 [1..256]
        sndbuf[count++] = esio->cntTCF;        // 제어상태, [1] TRIP, [2] CLOSE
        
        if(opr->soeDebug)
        Debug(console,"esio%d> snd Point Control ...[Dev=%d, point=%d, TCF=%d] \n", esioid, esio->cntDev, esio->cntPoint, esio->cntTCF);
        break;
        
    case SIM_TIME_DOWN:                 // 0x11 Time Sync Down
        sndbuf[count++]  = (rtc->year >> 8) & 0xff;
        sndbuf[count++]  = rtc->year & 0xff;
        sndbuf[count++]  = rtc->month;  
        sndbuf[count++]  = rtc->day;  
        sndbuf[count++]  = rtc->hour;  
        sndbuf[count++]  = rtc->min;  
        sndbuf[count++]  = rtc->sec;  

       	if(opr->esioDebug == esioid)
    	Debug(console, "esio%d> snd TIME-Sync Down : %4d/%2d/%2d %02d:%02d:%02d \n", esioid, rtc->year, rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
        break;

    case SIM_CHKSUM_DUMP:               // 0x16 데이터베이스 Chksum Scan   

#if 0
        chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
#else

        chksum = gensum((byte *)&esio_rtudb, sizeof(ESIO_RTU_DATABASE) - 2);
#endif         
        sndbuf[count++] = (chksum >> 8) & 0xff;
        sndbuf[count++] = chksum & 0xff;

        
        if(opr->esioDebug == esioid)                                  
        Debug(console,"esio%d> snd CHKSUM req...chksum = %04x \n", esioid, chksum);
        break;

    case SIM_RESTART_ESIO:       // 0x16 데이터베이스 Chksum Scan   
        if(opr->esioDebug == esioid) 
    	Debug(console,"esio%d> cmd ESIO-Restart... \n", esioid);
    	sndbuf[count++] = 0;   
        break;        

    case SIM_MPUCFG_DOWN:      // 0x20 DB : MPU Config Down
        
            
#if 0		
        dbSize = sizeof(DB_MPU_CONFIG);
		memcpy( &sndbuf[count], (byte *) &rtudb->mpuConfig,  dbSize);
		count += dbSize;
#else
// update 될 경우... 일단 조금 있다가..
        extern void copy_mpu_config(     DB_ESIO_MPU_CONFIG *dest,    DB_MPU_CONFIG *src) ;
        copy_mpu_config(  &esio_rtudb.esio_mpuConfig, &rtudb->mpuConfig);
         //copy_mpu_config( &dest->esio_mpuConfig, &src->mpuConfig) ;
        
        dbSize = sizeof(DB_ESIO_MPU_CONFIG);
        //printf("SIM_MPUCFG_DOWN size[%d]\r\n",dbSize );    
		// 구조체의 이름은 주소가 아니라 첫 항목이다.
		memcpy( &sndbuf[count], (byte *) &esio_rtudb.esio_mpuConfig,  dbSize);
		count += dbSize;

#endif 
		
		if(opr->esioDebug == esioid) 
		Debug(console,"esio%d> snd MPU Config ... size = %d\n", esioid, dbSize);
        break;
                        
    case SIM_ESIOCFG_DOWN:      // 0x22 DB : ESIO Config Down
        dbSize = sizeof(DB_ESIO_CONFIG);
        dbESIO = (DB_ESIO_CONFIG *) &rtudb->esioConfig[esio->dbDownEsio - 1];
        
		sndbuf[count++] = esio->dbDownEsio;                     // ESIO#  [1..5]
        sndbuf[count++] = 1;                                    // 전송 ESIO 갯수 (default = 1)
		memcpy( &sndbuf[count], (byte *) dbESIO,  dbSize);      // ESIO DB
		count += dbSize;
		
		if(opr->esioDebug == esioid) 
		Debug(console,"esio%d> snd ESIO (%d) Config ... size = %d\n", esioid, esio->dbDownEsio, dbSize);
        break;

    case SIM_HOST_DOWN:         // 0x24 DB : HOST Config Down
        dbSize = sizeof(DB_HOST_CONFIG) * MAX_DB_HOST;
        dbHost = (DB_HOST_CONFIG *) &rtudb->hostCfg[0];
		memcpy( &sndbuf[count], (byte *) dbHost,  dbSize);
		count += dbSize;
		
		if(opr->esioDebug == esioid) 
		Debug(console,"esio%d> snd HOST Config... size = %d\n", esioid, dbSize);
        break;
        
    case SIM_ICCP_DOWN:         // 0x26 DB : ICCP-HOST Config Down
        dbSize = sizeof(DB_ICCP_CONFIG);
		memcpy( &sndbuf[count], (byte *) &rtudb->iccpConfig,  dbSize);
		count += dbSize;
		
		if(opr->esioDebug == esioid) 
		Debug(console,"esio%d> snd ICCP Config... size = %d\n", esioid, dbSize);
        break;
        
    case SIM_HARRIS_DOWN:       // 0x28 DB : HARRIS Config Down
        dbSize = 32;
		memcpy( &sndbuf[count], (byte *) &rtudb->portdb[0],  dbSize);
		count += dbSize;
		
		if(opr->esioDebug == esioid) 
		Debug(console,"esio%d> snd HARRIS Config... size = %d\n", esioid, dbSize);
        break;
        
    case SIM_LANDIS_DOWN:       // 0x2A DB : LANDIS Config Down
        dbSize = 257;
		memcpy( &sndbuf[count], (byte *) &rtudb->chassisNum,  dbSize);
		count += dbSize;
		
		if(opr->esioDebug == esioid) 
		Debug(console,"esio%d> snd LANDIS Config... size = %d\n", esioid, dbSize);
        break;
                
    case SIM_MODBUS_DOWN:       // 0x2C DB : MODBUS Config Down
        dbSize = sizeof(DB_MODBUS_PROFILE) * MAX_DB_MODBUS_PROFILE;
		memcpy( &sndbuf[count], (byte *) &rtudb->modbusProfile[0],  dbSize);
		count += dbSize;
		
		if(opr->esioDebug == esioid) 
		Debug(console,"esio%d> snd MODBUS Config ... size = %d\n", esioid, dbSize);
        break;
        
    case SIM_SCAN_DOWN:         // 0x30 DB : SCAN Config Down
        dbSize = sizeof(DB_SCAN_CONFIG) * MAX_DB_SCAN_PORT;
		memcpy( &sndbuf[count], (byte *) &rtudb->scanConfig[0],  dbSize);
		count += dbSize;
		
		if(opr->esioDebug == esioid) 
		Debug(console,"esio%d> snd SCAN Config ... size = %d\n", esioid, dbSize);
        break;
        
    case SIM_DEVICE_DOWN:       // 0x32 DB : DEVICE Config Down
        devNum = 16;

        /* END-계전기  표시 */
        if(esio->dbDownDevice >= 96)    esio->endDeviceFlag = SET;
        else                            esio->endDeviceFlag = RESET;    

        dbSize = sizeof(DB_SDP_DEVICE) * 16;
        
        sndbuf[count++] = esio->dbDownDevice;               // 시작 계전기 번호 [1..64]
        
        if(esio->endDeviceFlag == SET)  sndbuf[count++] = 16 + 0x80;                // 계전기 수
        else                            sndbuf[count++] = 16;                       // 계전기 수                 
		memcpy( &sndbuf[count], (byte *) &rtudb->deviceConfig[esio->dbDownDevice - 1],  dbSize);
		count += dbSize;
		
		if(opr->esioDebug == esioid) 
		Debug(console,"esio%d> snd DEVICE (st=%d, num=%d) Config ... size = %d\n", esioid, esio->dbDownDevice, devNum,  dbSize);
        break;
        
    case SIM_POINT_DOWN:        // 0x34 DB : POINT Config Down
        pointNum    = 16;
		startPoint  = esio->dbDownPoint;
		reportPoint = startPoint + pointNum;
		
		/* ------------------------------------ */
		/* 실제 등록된 포인트 Check...          */
		/* ------------------------------------ */
		if(reportPoint >= MAX_DB_POINT)
		{
			pointNum = 	MAX_DB_POINT - esio->dbDownPoint + 1;
			
			esio->endPointFlag  = 0x80;	// point update flag...
            Debug(console,"esio%d> set  POINT-DB endPointFlag\n", esioid);

		}	

        esio->dbDownIndex = pointNum;
        
		sndbuf[count++] = (startPoint >> 8) & 0xff;
		sndbuf[count++] = startPoint & 0xff;
		sndbuf[count++] = pointNum | esio->endPointFlag;
		
        dbSize = sizeof(DB_POINT_BUF) * pointNum;
		memcpy( &sndbuf[count], (byte *) &rtudb->pointBuf[startPoint - 1],  dbSize);
        // 2026-05-23 오후 3:31:53
    	chksum = gensum (&sndbuf[count], dbSize);

		count += dbSize;
		
		

		
	//	if(opr->esioDebug == esioid) 

		
		if(opr->esioDebug == esioid) 
            if ( esio->endPointFlag & 0x80)
                Debug(console,"esio%d> snd Last POINT-DB ...start=%4d, num=%d, size = %d chksum [%04x]\n", esioid, startPoint, pointNum, dbSize,chksum);
            else 
                 Debug(console,"esio%d> snd POINT-DB ...start=%4d, num=%d, size = %d,   chksum [%04x]\n", esioid, startPoint, pointNum, dbSize,chksum);               
        break;



//        Debug(console,"esio%d> snd POINT-DB ...start=%4d, num=%d, size = %d\n", esioid, startPoint, pointNum, dbSize);
//        break;

    case SIM_CAL_POINT_DOWN:        // 0x36 DB : CAL-POINT Config Down
        pointNum    = 16;
		startPoint  = esio->dbDownCalpt;
		reportPoint = startPoint + pointNum;
		
		sndbuf[count++] = (startPoint >> 8) & 0xff;
		sndbuf[count++] = startPoint & 0xff;
		
		if(startPoint == 17)    sndbuf[count++] = pointNum | 0x80;
        else                    sndbuf[count++] = pointNum;
                 		
        dbSize = sizeof(DB_CAL_POINT) * pointNum;
		memcpy( &sndbuf[count], (byte *) &rtudb->calPointBuf[startPoint - 1],  dbSize);
		count += dbSize;
		
		if(opr->esioDebug == esioid) 
        Debug(console,"esio%d> snd CAL POINT-DB ...start=%4d, num=%d, size = %d\n", esioid, startPoint, pointNum, dbSize);
        break;
                
	default :
	    Debug(console, "esio%d> *snd Invalid Opcode ... %02x\n", esioid, opcode); 
	    return (0);
		break;	
	}


    /* -------------------------------- */                      
    /* 송신 Frame 구성                  */                      
    /* -------------------------------- */   
    sndCount = count + 1;                   // Packet 내 Address ~ DF 까지의 데이터 수
    sndbuf[2] = (sndCount >> 8) & 0xff;     /* Total size: MSB */
    sndbuf[3] = sndCount & 0xff;            /* Total size: LSB */

    sndbuf[count] = genlrc(sndbuf, count);
    count++;
    
    return(count);

}


/* ==================================================== */
/*  SCAN THREAD Start Routine ....                      */
/* ==================================================== */
void    *scanThread_ESIO(int  *arg)
{
    int     i,esioid;
    int     oldsec;
    int     reConnectCount;
    int		taskDelay;
    int     sendFlag, rxVal;
    int		resetCount, readFail;
    int		opcode, devNo;
    
    
    byte    *rxbuf, *txbuf;
    int     rxcnt, txcnt;
    char    buffer[256];
    
    ESIO_NET_ENTRY  *esioNet;
    ESIO_CONFIG     *esio;
        
	/* ------------------------------------ */
	/* THREAD Argment Assign ...            */
	/* ------------------------------------ */
	esioid = (int ) *arg;                                   // 해당 ESIO ID, 0,1,2..
    esio = (ESIO_CONFIG *) esioCFG[esioid];
    esioNet = (ESIO_NET_ENTRY *) &esio->mstNetConfig[0];    // ESIO 접속 Network 정보
    
    if(esio->comDelay <= 0) taskDelay = 100;
	else                    taskDelay = esio->comDelay * 10;
    
    printf(" %s ESIO-SCAN[%d] PROCESS Activated ...(Target=%s, Delay = %d ms) !\n", TARGET_NAME, esioid, esioNet->ipAddr, taskDelay);

    ThreadActive[esioid] = SET;
    resetCount = 0;
    readFail   = 0;
    
    txbuf = (byte *) &esioSndBuf[esioid][0];        // ESIO# 송신 Buffer Pointer
    rxbuf = (byte *) &esioRcvBuf[esioid][0];        // ESIO# 수신 Buffer Pointer
    rxcnt = 0;
    txcnt = 0;
    
    esio->SocketFd = -1 ; 
    esio->connectStatus = RESET;    

#if 0    
    /* ------------------------------------ */
    /*  MASTER CLIENT initial...            */
    /* ------------------------------------ */
   	if (_initNetClient_SCAN( esio, esioid) == ERROR)
    { 
        printf("esio%d> *** Network Initial(Client)... Fail...\n", esioid);	
        termExec = NO;
    }
    esio->connectStatus = SET;        
#endif     
    
    /* ------------------------------------ */
    /*  ESIO# : SCAN Device 정보            */
    /* ------------------------------------ */
    printf("=> ESIO-SCAN[%d] device Num = %02d ... ", esioid, esio->scanMaxNum);
    for(i=0; i< esio->scanMaxNum; i++)  printf(" %2d", esio->scanDevice[i]);
    printf("\n");
    
    reConnectCount  = 0;
    sendFlag        = 0;
    
    esio->comFailTick = 0;
    // 기본 flag set
    esio->chksumReq   = SET;
    esio->timeSyncReq = SET;
    //esio->online      = 2;	// 초기 ESIO 통신상태
//  2026-05-19 오후 1:24:34  위로이동     
//    esio->connectStatus = RESET;
    
    oldsec = rtc->sec;
    
	while(ThreadActive[esioid])
	{
	    pause(taskDelay);
		opr->scanWDT[esioid] = 0;
				                                
        /* ---------------------------------------- */
	    /*  ESIO 접속실패에 따른 재접속...          */
	    /* ---------------------------------------- */
	    if(esio->connectStatus == RESET)
	    {
          //  if(opr->esioDebug == esioid)  
          //  Debug(console,"esio%d> * Client retry Connect....socket = %d !\n", esioid , esio->SocketFd );
            
            /* LINK OFFLINE 표시 */
            if(esio->online == SET)
            {
            	Debug(console, ">> *ESIO(%d) Offline.... %s \n", esioid, esio->esioNameStr);
                logEvent_MPU(shmPtr, ENT_ESIO_OFFLINE, esioid+1, 0, 0, opr->cpuMode, NULL);   
                
                sprintf(buffer, ">> ESIO (%d) Offline ... %s ", esioid, esio->esioNameStr);
                LogFile_MPU (shmPtr, ENT_ESIO_OFFLINE, buffer, strlen(buffer));
            }
            
            esio->online = RESET;
            // 1000->200
            pause(200);

                   if (_initNetClient_SCAN(esio, esioid) < 0 )
                   { 
                        if(opr->esioDebug == esioid)
                        Debug(console, "esio%d> *** Network Initial(Client)... Fail...\n", esioid);	                    
                        // 2026-05-19 오후 1:27:28 
                      //  pause(1000);
                          
                          //  TK_SYS_ERR 이건    _initNetClient_SCAN 에서 처리하고 
                          //  termExec = NO; // thread종료가 아니고. SCAN  종룓네..

        				/* -------------------------------------------- */
        				/*	지속적으로 ESIO 와 링크가 되지 않는 경우...				*/
        				/* -------------------------------------------- */
        				resetCount++;
        				if(resetCount > 60) // 연속으로 60번 connect가 정상적으로 실패한 경우...
        				{
        					if((opr->runMode == LOCAL_SLAVE) && (esio->autoChgFlag == SET))
        					{
        						Debug(console, "esio%d> * *** SCAN Network Conection Failed... RE-BOOT ! \n", esioid);
        						
        						sprintf(buffer, "esio%d> *** SLAVE-Network Cnnection Failed... RE-BOOT !", esioid);
        						LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
        						pause(1000);
        			
        						opr->wdtEnable = RESET;
        					}
        				}
                    }            
                   else esio->connectStatus = SET ;
            
#if 0       
{     
            /* ------------------------------------ */
            /* ESIO 와의 접속 시도...               */  
            /* ------------------------------------ */  
            if(_reConnectClient_SCAN(esio, esioid) == 0)
            {
                if(++reConnectCount > 5)
                {
                    reConnectCount = 0;
                
                    if(opr->esioDebug == esioid)  
                    {
                        Debug(console,"------------------------------------------ \n");
                        Debug(console,"esio%d> * RE Initial...CLIENT....(reset=%d) !\n", esioid, resetCount);
                        Debug(console,"------------------------------------------ \n");
                    }

                    /* ---------------------------------------- */
                    /*  장치별 TCPIP 초기화...                  */
                    /* ---------------------------------------- */
                    if (_initNetClient_SCAN(esio, esioid) == ERROR)
                    { 
                        //if(opr->esioDebug == esioid)
                        Debug(console, "esio%d> *** Network Initial(Client)... Fail...\n", esioid);	
                        termExec = NO;
                    }
    
    				/* -------------------------------------------- */
    				/*	지속적으로 ESIO 와 링크가 되지 않는 경우...				*/
    				/* -------------------------------------------- */
    				resetCount++;
    				if(resetCount > 60)
    				{
    					if((opr->runMode == LOCAL_SLAVE) && (esio->autoChgFlag == SET))
    					{
    						Debug(console, "esio%d> * *** SCAN Network Conection Failed... RE-BOOT ! \n", esioid);
    						
    						sprintf(buffer, "esio%d> *** SLAVE-Network Cnnection Failed... RE-BOOT !", esioid);
    						LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    						pause(1000);
    			
    						opr->wdtEnable = RESET;
    					}
    				}
                }
    	        
	        }
}	        
#endif 	        
	    	continue;
	    }
    	
    	if(opr->runMode == LOCAL_MASTER)		resetCount = 0;
	    
	    
	    {		 	  	
	            /* ---------------------------------------- */
    	        /*  ESIO 송신 메세지 처리                        */
        	    /* ---------------------------------------- */
	            txcnt = makeLink_ESIO(esio, esioid, txbuf);
    	        if(txcnt > 7) 
    	        {      
    	                sendFlag = SET;      
                	    /* ESIO# 송수신 통신상태 확인용... */            
        	            esio->sndCount++;  
            	        if(esio->sndCount > 1000)
                	    {
                    	    esio->preRcvCount = esio->rcvCount;
                        	esio->sndCount = 1;
                        	esio->rcvCount = 0;
                        }
                        
                        /* 아래가 여기에 와야지. */
                        
                }
        }

        /* ---------------------------------------- */
        /*  ESIO 전송 데이터에 대한 패켓 처리 ...   */    
        /* ---------------------------------------- */
        if(sendFlag == SET)
        {
            sendFlag = RESET;
            
            if(txcnt >= 5) 
            {
                if(opr->esioDebug == esioid)
                {
                    if(txcnt > 256)     DumpBuff(console,"ETX: ", txbuf, 256);
                    else                DumpBuff(console,"ETX: ", txbuf, txcnt);    
                }
                          
                /* -------------------------------- */
                /* Network Response...              */
                /* -------------------------------- */    
                if((rxVal = tkWriteTCP( esio->SocketFd ,(byte *) txbuf, txcnt, 1000)) < 0 )
            	{
                   if(opr->esioDebug == esioid)  
                    Debug(console, "esio%d> * *** net Send Error [%d %d]...socket = %d !\n", esioid, txcnt,  rxVal, esio->SocketFd);		
                    //esio->connectStatus = RESET;
	            }

                if( rxVal != txcnt)
                {
                   if(opr->esioDebug == esioid)  
                    Debug(console, "esio%d> * net Send Error [%d %d]...!\n", esioid, txcnt,  rxVal);
                    
                    //esio->connectStatus = RESET;
                }                
            }
            
            
            if(esio->sndOpcode == SIM_CHKSUM_DUMP)  pause(200);
                
        }
	    
	    pause(500);
	    
	    //2026-05-22 오후 6:54:49
	    // 밀린다.. 
	    // 2026-05-23 오전 11:23:09  tk_get_response로 변경하고 지운다.
	    //pause(1000);
	    
	    
	    bzero(rxbuf, 10);
	    /* ------------------------------------- */
        /*  메세지 수신 & Response               */
    	/* ------------------------------------- */	        	        
        if((rxcnt = tk_get_response_ESIO(esio->SocketFd, esioid, rxbuf)) > 0)
        {
            esio->comFailTick = 0;
            esio->netCfgCount = 0;
	    
            /* ---------------------------------------- */
            /*  수신한 데이터에 대한 패켓 처리 ...              */    
            /* ---------------------------------------- */
            if(opr->esioDebug == esioid)  
            {         
                if(rxcnt > 256) DumpBuff(console,"ERX: ", rxbuf, 256);
                else            DumpBuff(console,"ERX: ", rxbuf, rxcnt);
            }        

			/* Reset Clear... */
            opcode = rxbuf[5] & 0x7f;           // 수신 OPCODE
            if(opcode == SIM_TIME_DOWN)		resetCount = 0;
            else if ((opcode == SIM_SDP_GPOLL) || (opcode == SIM_POINT_SCAN) || (opcode == SIM_ANALOG_SCAN))
            {
            	devNo = rxbuf[8] - 1;
            	if(opr->dnpDebug == devNo)
            	{
            		if(rxcnt > 256) DumpBuff(console,"ERX: ", rxbuf, 256);
                	else            DumpBuff(console,"ERX: ", rxbuf, rxcnt);
            	}	
            }
            
            txcnt = rcvHandler_ESIO(esioid, txbuf, rxbuf, rxcnt);
            if(txcnt > 7)  // 2026-05-22 오후 6:05:39 
            {
                 sendFlag = SET;  // ???? 엥 있다 ..eventDump.        
                                /* ---------------------------------------- */
                /*  ESIO 전송 데이터에 대한 패켓 처리 ...   */    
                /* ---------------------------------------- */
                if(sendFlag == SET)
                {
                    sendFlag = RESET;
                    
                    if(txcnt >= 5) 
                    {
                        if(opr->esioDebug == esioid)
                        {
                            if(txcnt > 256)     DumpBuff(console,"ETX: ", txbuf, 256);
                            else                DumpBuff(console,"ETX: ", txbuf, txcnt);    
                        }
                                  
                        /* -------------------------------- */
                        /* Network Response...              */
                        /* -------------------------------- */    
                        if((rxVal = tkWriteTCP( esio->SocketFd ,(byte *) txbuf, txcnt, 1000)) < 0 )
                    	{
                           if(opr->esioDebug == esioid)  
                            Debug(console, "esio%d> * *** net Send Error [%d %d]...socket = %d !\n", esioid, txcnt,  rxVal, esio->SocketFd);		
                            //esio->connectStatus = RESET;
        	            }

                        if( rxVal != txcnt)
                        {
                           if(opr->esioDebug == esioid)  
                            Debug(console, "esio%d> * net Send Error [%d %d]...!\n", esioid, txcnt,  rxVal);
                            
                            //esio->connectStatus = RESET;
                        }                
                    }
                    
                    
                    if(esio->sndOpcode == SIM_CHKSUM_DUMP)  pause(200);
                        
                }
                
                
                
            }		
            	
            /* ESIO# 송수신 통신상태 확인용... */           
            esio->sndSeqNo++;
            esio->rcvCount++;
            
            if(esio->online != SET)
            {
            	Debug(console, ">> ESIO(%d) Online.... %s \n", esioid, esio->esioNameStr);
            	
                logEvent_MPU(shmPtr, ENT_ESIO_ONLINE, esioid+1, 0, 0, opr->cpuMode, NULL);    
                
                sprintf(buffer, ">> ESIO (%d) Online ... %s ", esioid, esio->esioNameStr);
                LogFile_MPU (shmPtr, ENT_ESIO_ONLINE, buffer, strlen(buffer));
            }
            
	        esio->online = SET;
	               	
    	}
	    else
	    {
	        /* ---------------------------------------- */
            /*  ESIO 접속이상에 따른 처리               */
            /* ---------------------------------------- */
    	    if(rxcnt == 0)
    	    {
	        	//if(opr->esioDebug == esioid)  
	        	Debug(console, "esio%d> *** TCP/IP read ...0 Connection...Failed...!\n", esioid);
	            esio->connectStatus = RESET;
	            
	            
	            
	        }
	        else if(rxcnt == -2)
	      	{
	      		Debug(console, "esio%d> *TCP/IP read Fail ...%d !\n", esioid, readFail);
	      		if(++readFail > 10)
	      		{
	      			readFail = 0;
	      			esio->connectStatus = RESET;
	      		}		
	      	}
#if 0	      	
	     	else // timeout 걸려서ㅣㅣ나오념...아 이상하다.
	     	{		 	  	
	            /* ---------------------------------------- */
    	        /*  ESIO 송신 메세지 처리                        */
        	    /* ---------------------------------------- */
	            txcnt = makeLink_ESIO(esio, esioid, txbuf);
    	        if(txcnt > 7)   sendFlag = SET;      

        	    /* ESIO# 송수신 통신상태 확인용... */            
	            esio->sndCount++;  
    	        if(esio->sndCount > 1000)
        	    {
            	    esio->preRcvCount = esio->rcvCount;
                	esio->sndCount = 1;
                	esio->rcvCount = 0;
                }
            }
#endif                        
	    }


        
	    
        /* ------------------------------------ */
        /* 초단위 정보 처리 ...                 */ 
        /* ------------------------------------ */       
        if(oldsec == rtc->sec)  continue;
        oldsec = rtc->sec;
        
        if(oldsec == 0) 
        {
            if(esio->dbDownFlag == RESET)  esio->chksumReq = SET;       // 1분단위 DB Check...    
        }     
        
        /* ------------------------------------ */
        /* ESIO# 정보연계 Timeout 초기화 ...    */ 
        /* ------------------------------------ */ 
        if(++esio->comFailTick > 5)
        {
            if(opr->esioDebug == esioid)
            Debug(console, "esio%d> **** ESIO-SCAN TIME-OUT... Re Initialize...!\n", esioid);
                    
            esio->comFailTick = 0;
            esio->connectStatus = RESET;
        }
        
	}/* while */

    ThreadActive[esioid] = 0;
    
    printf(">> *ESIO[%d] Thread.... exit !\n", esioid);
    
    return (0);
    
}


