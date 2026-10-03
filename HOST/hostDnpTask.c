
#include	"localLib.h"
#include    "external.h"

extern  int     hostChanRead(int hostid, int chid, byte *rxbuf, int reqCount);

extern  int     hostVMERead(int hostid, int chid, byte *rxbuf);
extern  void    check_DNP_IIN(int hostid, int chid);
extern  int     sendAppFrame_HOST( int hostid, int chid, int socketID, DNP_APP_FRAME *sndAppDnp);
extern  int     vmeHostInitial(int hostid, int chid);
extern  int     vmeHostChanReset(int hostid, int chid);
extern  int     chkRcvFrameHOST( int hostid, int chid, int socketID, DNP_DATA_LINK *dnpData);
extern  int     checkDataLinkHOST( int hostid, int chid, int socketID, DNP_DATA_LINK *dnpData);
extern  int     checkAPPFrame_HOST(int hostid, int chid, int socketID);

extern  void    sendConfirmHOST( int hostid, int chid, int socketID, int code);

extern  void    sndACConfirm(int hostid, int chid, int socketID);

extern  TASK_INFO	    *taskPtr;
extern  int             termExec;

extern  int             wdtHostFlag[MAX_HOST];
extern  byte            vmeRxbuf[MAX_HOST][1024];


//#define CHECK  printf("%s.%s.%d\r\n",__FILE__,__FUNCTION__,__LINE__);

#define CHECK
/*****************************************
* handle response 
* 
* return :  1   get vaild response  
*  			0   TCP Connection closed
*           -1  Timeout
*           -2  select system error 
******************************************/
int  hostTCPIP_read(int hostid, int	hostSocketFd, byte	*rxbuf)
{
    int     i;
    int     result;
    int     size;
    int    length;
    int     frameCnt,left;
    int     dataLen;
    int		rxcnt;
    //int     reqSize;
    //byte    lrc;
    word    crc;

    rxcnt = 0;
    
    /* ---------------------------------------- */
    /*  READ MODBUS - Header                    */
    /* ---------------------------------------- */ 
	if((result =TKreadn(hostSocketFd, (char *) &rxbuf[0], 2, 100)) < 0)
   	{
        /*  such as timeout */
		if(result == -1) /* timeout */
    	{
        	//if(opr->hexDebug)
           	//Debug(console,"host%02d> *Header Read Timeout [%d]... \n", hostid, tid, result);			     			        
    	}
       	else
       	{
           	//if(opr->hexDebug) 
	    	//Debug(console,"host%02d> *Header Read fail [%d][%s] \n", hostid, tid, result, strerror(errno));
   	 	}
       	return result;
    }
        
    /* TCP/IP Socket Error 발생시 ... */
	if(result == 0)	return 0;

    if((rxbuf[0] != 0x05) || (rxbuf[1] != 0x64))    return (-1);
    rxcnt = 2;
        
    /* read Header */
	if((result =TKreadn(hostSocketFd, (char *) &rxbuf[2], 8, 100)) < 0)	return (-1);
	
	/*  check HEADER-CRC     */
    if(B013_ckdnpcrc(&rxbuf[0], 10) != 0)
    {
        crc = B013_mkdnpcrc( &rxbuf[0], 8);
        if(opr->hostDebug == hostid)
        {
        	Debug(console,"[%02d:%02d:%02d] ===========>\n", rtc->hour, rtc->min, rtc->sec);
            Debug(console,"hrcv%2d> *header - crc error cal=%04x, rcv=%02x-%02x!\n", hostid+1, crc, rxbuf[8],rxbuf[9]);
            //prn_rcv("hrcv-CRC:", &rxbuf[0], 10);
            DumpDNP_rcv(console, "hrcv-CRC:", &rxbuf[0], 10);
        }
        return (-1);
    }
    
    rxcnt  = 10;  

    length = rxbuf[2] - 5;      /* except control, src, des addr (5byte) */
    
    /* ------------------------------------ */
    /*  1) if CONFIRM frame received ...    */
    /*  2) if reset LINK frame received ... */
    /* ------------------------------------ */
    if(length <= 0)     
    {
        if(opr->hostDebug == hostid)  
            Debug(console,"hrcv%2d> no Data Length = %d\n", hostid+1, length);
        return (rxcnt);
    }
    else if(length > 255)     
    {
        //if(opr->hostDebug == hostid)  
        Debug(console,"hrcv%2d> *DNP rcv - Invalid Length = %d \n", hostid+1, length);
        return (-1);
    }
    
    frameCnt = (length / FRAME_LENGTH);
    left     = (length % FRAME_LENGTH);
    if(left)    frameCnt = frameCnt + 1;    
	
	/* Multi-user data block : execpt CRC ... */
    for(i = 0; i < frameCnt; i++)
    {
        if(length <= 0 )    break;

        if(length >= FRAME_LENGTH)  
        {
            dataLen = FRAME_LENGTH;
            size = dataLen + 2;
        }
        else                        
        {
            dataLen = length ;
            size    = dataLen + 2;
        }
        
        if((result =TKreadn(hostSocketFd, (char *) &rxbuf[rxcnt], size, 100)) < 0)	
        {
            if(opr->hostDebug == hostid)  
            Debug(console,"hrcv%2d> *Tail not Received.... %d\n", hostid+1, result);
            return (-1);
        }
                
        if(B013_ckdnpcrc(&rxbuf[rxcnt], size) != 0)
        {
            crc = B013_mkdnpcrc( &rxbuf[rxcnt], dataLen);
            if(opr->hostDebug == hostid)
            Debug(console,"hrcv%2d> *user data [%d] - crc error cal=%04x, rcv=%02x-%02x!\n",  hostid+1, i+1, crc, rxbuf[rxcnt + dataLen], rxbuf[rxcnt + dataLen + 1]);
            rxcnt = rxcnt + dataLen;
            return (-1);
        }

        rxcnt  = rxcnt + dataLen;   /* include crc-data */
        length = length - dataLen;
        
    }

    return (rxcnt);
	
	
}


/***************************************************
*   FUNCTION : readDataLink()
*   receive DATA-LINK with CRC
***************************************************/
int readDataLinkHOST(int hostid, int chid, int socketID, DNP_DATA_LINK *dnpData)
{
    int     i,size;
    int     rxcnt,vmecnt, rcvCount;
    //int     retVal;
    int     length;
    int     frameCnt,left;
    int     dataLen;
    int     desAddr, srcAddr;
    byte    *rxbuf, *vmedata;
    word    crc;
    DL_FRAME    *rcvData;
    //DNP_DEVICE  *dnp;
    HOST_DCB    *host;

    /* ------------------------------------ */
    /*  HOST 별 통신 Frame 설정             */
    /* ------------------------------------ */
    host = (HOST_DCB *) hostDCB[hostid];

    rxcnt   = 0;
    rcvCount = 0;

    /* ---------------------------------------------------- */
    /*  HOST - TCPIP 의 경우...                             */
    /* ---------------------------------------------------- */
    if(host->hostComType == COM_TCPIP)
    {
        rcvData = (DL_FRAME *) &dnpData->rcvFrame;
        rxbuf = (byte *) &rcvData->start1;

        /* ------------------------------------------------ */
        /*  VMEBUS 에서 수신된 Packet Data 수신...          */
        /* ------------------------------------------------ */
        rcvCount = hostTCPIP_read(hostid, socketID, rxbuf);
        
        /* Timeout 이나 Connection Fail... */
        if(rcvCount <= 0)   return (rcvCount);
            
        /* ------------------------ */
        /*  check DNP-ADDR ...      */
        /* ------------------------ */
        desAddr = rcvData->desAddr[0] + (rcvData->desAddr[1]*256);
        srcAddr = rcvData->srcAddr[0] + (rcvData->srcAddr[1]*256);
    
        if(desAddr == 0xffff)
        {
            if(opr->hostDebug == hostid) Debug(console,"hrcv%2d> Broadcast Addr .. !\n", hostid+1);
            dnpData->broadCast = SET;
        }
        else
        {
            dnpData->broadCast = RESET;

            if(desAddr != host->rtuAddr)
            {
                if(opr->hostDebug == hostid)    
                {
                	//prn_rcv("*RXM:", rxbuf, rcvCount);
                	DumpDNP_rcv(console, "RXM*:", rxbuf, rcvCount);
                }
                
                if(opr->hexDebug == hostid) 
                Debug(console,"hrcv%2d> *1 HOST Addr mismatch : rcv %4x, cu-host %4x.. !\n", hostid+1, desAddr, host->rtuAddr);
                return (-2);
            }
            else if(srcAddr != host->hostid)
            {
                if(opr->hostDebug == hostid)    
                {
                	//prn_rcv("*RXM:", rxbuf, rcvCount);
                	DumpDNP_rcv(console, "*RXM:", rxbuf, rcvCount);
                }
                if(opr->hexDebug == hostid)
                Debug(console,"hrcv%2d> *1 RTU  Addr mismatch : rcv %4x, cu-rtu %4x.. !\n", hostid+1, srcAddr, host->hostid);
                return (-2);
            }
        }
    
        return (rcvCount);
        
    }

    /* ---------------------------------------------------- */
    /*  HOST - ACE-VMECU-NET-V7 VMESIO 의 경우...           */
    /* ---------------------------------------------------- */
    rcvData = (DL_FRAME *) &dnpData->rcvFrame;

    rxcnt   = 0;
    rcvCount = 0;
    
    rxbuf = (byte *) &rcvData->start1;
    vmedata = (byte *) &vmeRxbuf[hostid][0];

    /* ------------------------------------------------ */
    /*  VMEBUS 에서 수신된 Packet Data 수신...          */
    /* ------------------------------------------------ */
        CHECK
    rcvCount = hostVMERead(hostid, chid, vmedata);
    
    //printf("host%d> vme read... %d\n",hostid, rcvCount);
     
    if(rcvCount <= 0)   return (0);
    
    
    
    /* check HEADER */
    if(vmedata[0]!= 0x05) return (0);
    if(vmedata[1]!= 0x64) return (0);
    rxcnt = 2;

    /*  check HEADER-CRC     */
    if(B013_ckdnpcrc(&vmedata[0], 10) != 0)
    {
        crc = B013_mkdnpcrc( &vmedata[0], 8);
        if(opr->hostDebug == hostid)
        {
        	Debug(console,"[%02d:%02d:%02d] ===========>\n", rtc->hour, rtc->min, rtc->sec);
            Debug(console,"hrcv%2d> *vme header - crc error cal=%04x, rcv=%02x-%02x!\n", hostid+1, crc, vmedata[8],vmedata[9]);
            //prn_rcv("hrcv-CRC:", &vmedata[0], 10);
            DumpDNP_rcv(console, "hrcv-CRC:", &vmedata[0], 10);
        }
        return (0);
    }
    
    rxcnt  = 10;  
    memcpy(rxbuf, vmedata, rxcnt);        
    
    /* ------------------------ */
    /*  check DNP-ADDR ...      */
    /* ------------------------ */
    desAddr = rcvData->desAddr[0] + (rcvData->desAddr[1]*256);
    srcAddr = rcvData->srcAddr[0] + (rcvData->srcAddr[1]*256);
    
    if(desAddr == 0xffff)
    {
        if(opr->hostDebug == hostid) Debug(console,"hrcv%2d> Broadcast Addr .. !\n", hostid+1);
        dnpData->broadCast = SET;
    }
    else
    {
        dnpData->broadCast = RESET;

        if(desAddr != host->rtuAddr)
        {
            if(opr->hexDebug == hostid)
            {       
                Debug(console,"hrcv%2d> *1 HOST Addr mismatch : rcv %4x, cu-host %4x.. !\n", hostid+1, desAddr, host->rtuAddr);
                prndat("*RCV: ", (char *) vmedata, rcvCount);
            }
            return (0);
        }
        else if(srcAddr != host->hostid)
        {
            if(opr->hexDebug == hostid)
            {      
                Debug(console,"hrcv%2d> *1 RTU  Addr mismatch : rcv %4x, cu-rtu %4x.. !\n", hostid+1, srcAddr, host->hostid);
                prndat("*RCV: ", (char *)vmedata, rcvCount);
            }
            return (0);
        }
    }
    
//NEXT_PROCESS_HOST:    
    rxcnt  = 10;                /* user data */
    vmecnt = rxcnt;
    length = vmedata[2] - 5;      /* except control, src, des addr (5byte) */

    /* ------------------------------------ */
    /*  1) if CONFIRM frame received ...    */
    /*  2) if reset LINK frame received ... */
    /* ------------------------------------ */
    if(length == 0)     
    {
        //memcpy(rxbuf, vmedata, rxcnt);
        if(opr->hostDebug == hostid)  Debug(console,"hrcv%2d> no Data Length = %d\n", hostid+1, length);
        return (rxcnt);
    }
    
    frameCnt = (length / FRAME_LENGTH);
    left     = (length % FRAME_LENGTH);
    if(left)    frameCnt = frameCnt + 1;

    /* Multi-user data block : execpt CRC ... */
    for(i = 0; i < frameCnt; i++)
    {
        if(length <= 0 )    break;

        if(length >= FRAME_LENGTH)  
        {
            dataLen = FRAME_LENGTH;
            size = dataLen + 2;
        }
        else                        
        {
            dataLen = length ;
            size    = dataLen + 2;
        }

        if(B013_ckdnpcrc(&vmedata[vmecnt], size) != 0)
        {
            crc = B013_mkdnpcrc( &vmedata[vmecnt], dataLen);
            if(opr->hostDebug == hostid)
            Debug(console,"hrcv%2d> *user data [%d] - crc error cal=%04x, rcv=%02x-%02x!\n",
                hostid+1, i+1, crc, vmedata[vmecnt + dataLen], vmedata[vmecnt + dataLen + 1]);
            rxcnt = rxcnt + dataLen;
            return (0);
        }

        memcpy(&rxbuf[rxcnt], &vmedata[vmecnt], dataLen);
        
        rxcnt  = rxcnt + dataLen;   /* include crc-data */
        length = length - dataLen;
        
        vmecnt += size;
    }

    return (rxcnt);

}

#if 0
==========================================================
/*
*   2016년 3월 5일 - 철도공항공사 영종역, 비츠로시스 HOST
*/
int check_UNSOLICT_HOST(int hostid, int chid , int socketID)
{
    int     i;
    int     count;
    byte    sndSeqNo;
    byte    front1, rear1, cosCount;
    byte    front2, rear2, soeCount;
    byte    *sndbuf;
    
    DNP_APP_FRAME   *sndAppDnp;
    
    HOST_DCB        *host;

    DNP_SOE_QUEUE   *soeQ;
    DNP_SOEQ_ENTRY  *entrySOE;
    DNP_COS_QUEUE   *cosQ;
    DNP_COSQ_ENTRY  *entryCOS;
    
    host = (HOST_DCB *) hostDCB[hostid];    
    
    /* -------------------------------------------------------- */
    /*  UNSOLICIT 메세지 허용여부                               */
    /* -------------------------------------------------------- */
    if(host->unsolEvent == 0)   return (0);

    /* ---------------------------------------- */
    /*  VAR=2, SOE 정보 추가                    */
    /* ---------------------------------------- */    
    soeQ = (DNP_SOE_QUEUE *) &host->dnpSOEQ;
    front2 = soeQ->front & 0xff;
    rear2  = soeQ->rear & 0xff;
    
    soeCount = (front2 - rear2 + 256) % 256;
    if(soeCount > 24)   soeCount = 24;
    
    /* ---------------------------------------- */
    /*  VAR=1, COS 정보 추가                    */
    /* ---------------------------------------- */  
    cosQ = (DNP_COS_QUEUE *) &host->dnpCOSQ;
    front1 = cosQ->front & 0xff;
    rear1  = cosQ->rear & 0xff;
    
    cosCount = (front1 - rear1 + 256) % 256;
    if(cosCount > 64)   cosCount = 64;
        
    if((cosCount == 0) && (soeCount == 0))  return (0);

    sndAppDnp = (DNP_APP_FRAME *) &host->sndUnsolFrame[chid];
    
    /* -------------------------------------------------------- */
    /* Make APPLICATION Message FORMAT :                        */
    /* -------------------------------------------------------- */
    check_DNP_IIN(hostid, chid);

    /* 데이터 포인트 */
    sndbuf = (byte *) &sndAppDnp->userData[0];
            
    count = 0;
    sndSeqNo = host->sndACseq + 0x1f;
    
    sndAppDnp->userData[0] = 0xc0 + sndSeqNo + CONFIRM_BIT;     /* AC */
    sndAppDnp->userData[1] = APP_RCV_UNSOLICIT;                 /* Function : 130 */
    sndAppDnp->userData[2] = host->rtuIIN[0];                   // IIN
    sndAppDnp->userData[3] = host->rtuIIN[1];                   // IIN
    count = 4;
    
    /* ---------------------------------------- */
    /*  VAR=1, COS 정보 추가                    */
    /* ---------------------------------------- */   
    if(cosCount > 0)
    {
        if(opr->soeDebug)
        Debug(console,"hsnd%2d> <--- Un-solicit COS-Q Report=%2d [front=%d, rear = %d] ... [%02d:%02d:%02d] \n", hostid+1, cosCount, 
            front1, rear1, rtc->hour, rtc->min, rtc->sec);
    
        /* ---------------------------------------------------------------- */
        /* QCode 7 : 8bit Single field Quantity, 포인트가 256 이하인 경우   */
        /* ---------------------------------------------------------------- */    
        if(host->diIndexWord == RESET)
        {            
            sndbuf[count++] = OBJ_BINARY_CHANGE;        // OBJ code             
            sndbuf[count++] = 1;                        // Variation #2
            sndbuf[count++] = 0x17;                     // QCode 7 : Single field Quantity, 포인트가 256 이하인 경우
            sndbuf[count++] = cosCount;                 // SOE Count Number
    
            for(i=0; i< cosCount; i++)
            {
                entryCOS = ( DNP_COSQ_ENTRY *) &cosQ->cosQueue[rear1];
                sndbuf[count++] = entryCOS->cosBuf[0];
                sndbuf[count++] = entryCOS->cosBuf[1];
                rear1 = (rear1 + 1) & 0xff;
            }
        }
        else
        {
            /* ---------------------------------------------------------------- */
            /* QCode 8 : 16bit Single field Quantity, 포인트가 256 이하인 경우  */
            /* ---------------------------------------------------------------- */    
            sndbuf[count++] = OBJ_BINARY_CHANGE;        // OBJ code             
            sndbuf[count++] = 1;                        // Variation #2
            sndbuf[count++] = 0x28;                     // QCode 8 : 16Bit Single field Quantity, 포인트가 256 이하인 경우
            sndbuf[count++] = cosCount;                 // SOE Count Number
            sndbuf[count++] = 0;                        // SOE Count Number
    
            for(i=0; i< cosCount; i++)
            {
                entryCOS = ( DNP_COSQ_ENTRY *) &cosQ->cosQueue[rear1];
                sndbuf[count++] = entryCOS->cosBuf[0];          // point 번호 : 하위 
                sndbuf[count++] = entryCOS->cosBuf[1];          // point 번호 : 상위 
                sndbuf[count++] = entryCOS->cosBuf[2];          // point state 
                                    
                rear1 = (rear1 + 1) & 0xff;
            }                                
        }
        
        /* AC-Confirm 시 Rear 정보 Update */
        host->cosReport = cosCount;
        host->cosRear   = rear1;
    }
    
    /* ---------------------------------------- */
    /*  VAR=2, SOE 정보 추가                    */
    /* ---------------------------------------- */    
    if(soeCount > 0)
    {
        if(opr->soeDebug)
        Debug(console,"hsnd%2d> <<<< Un-solicit SOE-Q Report=%2d [front=%d, rear = %d]   ... [%02d:%02d:%02d] \n", hostid+1, soeCount, 
                front2, rear2, rtc->hour, rtc->min, rtc->sec);      
    
        /* ---------------------------------------------------------------- */
        /* QCode 7 : 8bit Single field Quantity, 포인트가 256 이하인 경우   */
        /* ---------------------------------------------------------------- */    
        if(host->diIndexWord == RESET)
        {
            sndbuf[count++] = OBJ_BINARY_CHANGE;        // OBJ code             
            sndbuf[count++] = 2;                        // Variation #2
            sndbuf[count++] = 0x17;                     // QCode 7 : Single field Quantity, 포인트가 256 이하인 경우
            sndbuf[count++] = soeCount;                 // SOE Count Number
    
            for(i=0; i< soeCount; i++)
            {
                entrySOE = ( DNP_SOEQ_ENTRY *) &soeQ->soeQueue[rear2];
                sndbuf[count++] = entrySOE->soeBuf[0];
                sndbuf[count++] = entrySOE->soeBuf[1];
                sndbuf[count++] = entrySOE->soeBuf[2];
                sndbuf[count++] = entrySOE->soeBuf[3];
                sndbuf[count++] = entrySOE->soeBuf[4];
                sndbuf[count++] = entrySOE->soeBuf[5];
                sndbuf[count++] = entrySOE->soeBuf[6];
                sndbuf[count++] = entrySOE->soeBuf[7];
        
                rear2 = (rear2 + 1) & 0xff;
            }
        }
        else
        {
            /* ---------------------------------------------------------------- */
            /* QCode 8 : 16bit Single field Quantity, 포인트가 256 이하인 경우  */
            /* ---------------------------------------------------------------- */    
            sndbuf[count++] = OBJ_BINARY_CHANGE;        // OBJ code             
            sndbuf[count++] = 2;                        // Variation #2
            sndbuf[count++] = 0x28;                     // QCode 8 : Single field Quantity, 포인트가 256 이하인 경우
            sndbuf[count++] = soeCount;                 // SOE Count Number
            sndbuf[count++] = 0;                        // SOE Count Number
    
            for(i=0; i< soeCount; i++)
            {
                entrySOE = ( DNP_SOEQ_ENTRY *) &soeQ->soeQueue[rear2];
                sndbuf[count++] = entrySOE->soeBuf[0];          // point 번호 : 하위 
                sndbuf[count++] = entrySOE->soeBuf[1];          // point 번호 : 상위 
                sndbuf[count++] = entrySOE->soeBuf[2];          // point state 
            
                sndbuf[count++] = entrySOE->soeBuf[3];          // SOE Time 
                sndbuf[count++] = entrySOE->soeBuf[4];
                sndbuf[count++] = entrySOE->soeBuf[5];
                sndbuf[count++] = entrySOE->soeBuf[6];
                sndbuf[count++] = entrySOE->soeBuf[7];
                sndbuf[count++] = entrySOE->soeBuf[8];
        
                rear2 = (rear2 + 1) & 0xff;
            }
        }
                    
        /* AC-Confirm 시 Rear 정보 Update */
        host->soeReport = soeCount;
        host->soeRear = rear2;
    }
    
    /* ---------------------------------------- */
    /*  COS, SOE 정보가 있는 경우.... Write     */
    /* ---------------------------------------- */ 
    if(count > 4)
    {
                    
        if(opr->hexDebug == hostid)
        DumpBuff(console,"SND: ", &sndAppDnp->userData[0], count);
        
        sndAppDnp->sndCount = count;

        sendAppFrame_HOST( hostid, chid, socketID, sndAppDnp);
    }

    return (0);
    
}
==========================================================
#endif


/* ------------------------------------------------------- */
/*  HOST MAIN 프로그램 Start Routine ....             */
/* ------------------------------------------------------- */
int    hostDnpThread(int hostid)
{
    //int     i;
	int		oldsec = 0;
	int     retVal;
	int     rxcnt;
	int		runTick=0;
	int		powerReset=SET;
	//byte    front, rear;
	int     socketID=0;
	int		localMaster=0;
	
	int  loopCount ; 
	
	DNP_DATA_LINK   *dnpData;

    //DNP_APP_FRAME   *rcvAppDnp;
    //DNP_APP_FRAME   *sndAppDnp;
	HOST_DCB        *host;
	DNP_SOE_QUEUE   *dnpSOEQ;
	
	/* -------------------------------- */
	/*  HOST별 Index ...                */
	/* -------------------------------- */
    host = (HOST_DCB *) hostDCB[hostid];                 /* 주장치 #1 속성정의 */
    
    dnpSOEQ = (DNP_SOE_QUEUE *) &host->dnpSOEQ;
    dnpSOEQ->front = 0;
    dnpSOEQ->rear  = 0;
    
    dnpSOEQ->front1= 0;
    dnpSOEQ->rear1 = 0;
    
    host->sndTHseqno = 0;
    host->cosReport  = 0;
    host->cosRear    = 0;
    
    host->soeReport  = 0;
    host->soeRear    = 0;
            
    /* HOST 통신 초기상태 ...   */
    host->online[MASTER_PORT]      = 0;    // default OFFLINE
    host->online[SLAVE_PORT]       = 0;    // default OFFLINE
    host->comFailTick[MASTER_PORT] = 0;
    host->comFailTick[SLAVE_PORT]  = 0;

    /* ---------------------------------------------------- */
    /*  Default VME Channel Comm Parameter                  */
    /* ---------------------------------------------------- */
    host->vmeChan[MASTER_PORT]->protocolType = SCAN_DNP;
    host->vmeChan[MASTER_PORT]->cfgType      = host->hostComType;
    host->vmeChan[MASTER_PORT]->cfgSpeed     = host->hostComSpeed;
    
    host->vmeChan[SLAVE_PORT]->protocolType  = SCAN_DNP;
    host->vmeChan[SLAVE_PORT]->cfgType       = host->hostComType;
    host->vmeChan[SLAVE_PORT]->cfgSpeed      = host->hostComSpeed;

    //hostVMEInitial(hostid);    
 //   printf("Call vmeHostInitial %d %d\r\n",hostid, host->masterChan);
    vmeHostInitial(hostid, host->masterChan);  
//    printf("Call vmeHostInitial %d %d\r\n",hostid, host->slaveChan);
    vmeHostInitial(hostid, host->slaveChan);  
    
    vmeHostChanReset(hostid, MASTER_PORT);
    vmeHostChanReset(hostid, SLAVE_PORT);
    
    /* ------------------------ */
    /* SIO Board 를 사용함      */
    /* ------------------------ */
    opr->useSIOBoard = SET;
    
    /* ---------------------------------------------------- */
    /* ACE-RTU : 상위 HOST 통신포트 초기화                  */
    /* ---------------------------------------------------- */        
    if(host->hostDualMode == HOST_DUAL)             // 이중화 구조인 경우 
    {
        printf(" %s HOST[%d] DNP-THREAD *DUAL [%d/%d]*  Activated ... !\n", TARGET_NAME, hostid+1, host->masterChan, host->slaveChan);
    }
    else if(host->hostDualMode == HOST_SINGLE)      // 개별 구조인 경우 
    {
        printf(" %s HOST[%d] DNP-THREAD *SINGLE [%d/%d]*  Activated ... !\n", TARGET_NAME, hostid+1, host->masterChan, host->slaveChan);
    }        
    else         // 사용하지 않는 경우 
    {
        printf(" %s HOST[%d] DNP-THREAD *NOT Define [%d/%d]*  Activated ... !\n", TARGET_NAME, hostid+1, host->masterChan, host->slaveChan);
        
        while(termExec)
	    {
	        wdtHostFlag[hostid] = 0;
		    pause(1000);        
		    
		    if(opr->wdtDebug)
		    Debug(console,">> *HOST[%d] DNP-THREAD *NOT Define ... !\n", hostid);
        }		    
    }

    
    host->activePort = MASTER_PORT;
    opr->hostRestart[hostid] = RESET;
    
    opr->dnpHostEnb  = SET;     // HOST : DNP HOST 정의시
    
    powerReset = SET;
    
    
    loopCount = 0 ;
	while(termExec)
	{
	    taskPtr->wdtCount = 0;
	    wdtHostFlag[hostid] = 0;
		pause(20);

        // 고의적 sleep
//        if ( ++loopCount < 50 )  continue  ;
//        loopCount = 0 ;     
            
        
#if 0
		/* ---------------------------------------------------- */
		/*	CPU 이중화 : LOCAL-SLAVE ... 대기모드 				*/
		/* ---------------------------------------------------- */
		if(opr->runMode == LOCAL_SLAVE)	
		{
		    /* 2020.06.03 절체시 불필요한 HOST-Online 이벤트 발생 */
		    host->online[MASTER_PORT] = 0;
		    host->online[SLAVE_PORT]  = 0;
		    host->hostActive = HOST_NOT_READY;          /* HOST 통신모드 : 미지정 */   
		    pause(1000);
		    
		    /* ------------------------------------ */
            /* HOST-DB 변경에 따른 재기동 ...       */
            /* ------------------------------------ */
            if(opr->hostRestart[hostid] == SET)
            {
                Debug(console, "host%02d> *** HOST TASK Restart... DB Chg ...\n", hostid+1);
                pause(1000);
                termExec = RESET;
            }
        
        	localMaster = LOCAL_SLAVE;
        	
        	/* 초기 기동시... */
        	if((powerReset == SET) && (++runTick > 10))
        	{
        		runTick = 0;
        		vmeHostInitial(hostid, host->masterChan); 
				vmeHostInitial(hostid, host->slaveChan); 
        		vmeHostChanReset(hostid, MASTER_PORT);
				vmeHostChanReset(hostid, SLAVE_PORT);
				
        	}
        		
		    continue;
		}
		
		/* -------------------------------------------- */
		/*	SLAVE MODE => MASTER MODE 전환시...			*/
		/* -------------------------------------------- */
		if(localMaster == LOCAL_SLAVE)
		{
			pause(1000);
			Debug(console, "host%02d> *** Change MASTER : HOST Channel-RESET ...\n", hostid+1);
			vmeHostInitial(hostid, host->masterChan); 
			vmeHostInitial(hostid, host->slaveChan); 
			vmeHostChanReset(hostid, MASTER_PORT);
			vmeHostChanReset(hostid, SLAVE_PORT);
		}
		
		powerReset = 0;
		localMaster = LOCAL_MASTER;
#endif
					
        /* ---------------------------------------------------- */
        /*  MASTER 통신포트 : 상위 데이터 수신 처리 ...         */
        /* ---------------------------------------------------- */
        dnpData   = (DNP_DATA_LINK *) &host->dataLinkFrame[MASTER_PORT];

        CHECK
            
        if((rxcnt = readDataLinkHOST( hostid, MASTER_PORT, socketID, dnpData)) > 0)
        {
            host->comFailTick[MASTER_PORT]= 0;
            host->comFailTick[SLAVE_PORT] = 0;
            
            if(opr->hostDebug == hostid)
            {
                Debug(console,"\n-----------------------------------------[%02d/%02d-%02d:%02d:%02d]\n", 
        	        rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
                //prn_rcv("RXM:", &dnpData->rcvFrame, rxcnt);
                DumpDNP_rcv(console, "RXM:", &dnpData->rcvFrame, rxcnt);
            }

            /* HOST 통신상태 Update... */
            host->runStatus = MASTER_PORT + 1;
                        
            dnpData->rcvCount        = rxcnt;
            if(host->online[MASTER_PORT] != 1)  
            {
                logEvent_MPU(shmPtr, ENT_HOST_ONLINE, hostid+1, MASTER_PORT, 0, hostid+1, NULL);  
            }
            host->online[MASTER_PORT] = 1;
            
            /* ---------------------------- */
            /* check DATA-LINK Layer  ...   */
            /* ---------------------------- */
            retVal = chkRcvFrameHOST(hostid, MASTER_PORT, socketID, dnpData);
            if((retVal == FUNC_USER_DATA) || (retVal == FUNC_UNCONFIRM))
            {
                retVal = checkDataLinkHOST(hostid, MASTER_PORT, socketID, dnpData);
                if(retVal)
                {
                    checkAPPFrame_HOST( hostid, MASTER_PORT, socketID);
                }                    
            }
        } // MASTER PORT      
        CHECK        
        /* ---------------------------------------------------- */
        /*  SLAVE  통신포트 : 상위 데이터 수신 처리 ...         */
        /* ---------------------------------------------------- */        
        if(host->hostDualMode == HOST_DUAL)
        {
            dnpData   = (DNP_DATA_LINK *) &host->dataLinkFrame[SLAVE_PORT];
         CHECK       
            if((rxcnt = readDataLinkHOST( hostid, SLAVE_PORT, socketID, dnpData)) > 0)
            {
                host->comFailTick[MASTER_PORT]= 0;
                host->comFailTick[SLAVE_PORT] = 0;
                    
                if(opr->hostDebug == hostid)
                {
                    Debug(console,"\n-----------------------------------------[%02d/%02d-%02d:%02d:%02d]\n", 
        	            rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
                    //prn_rcv("RXS:", &dnpData->rcvFrame, rxcnt);
                    DumpDNP_rcv(console, "RXS:", &dnpData->rcvFrame, rxcnt);
                }
                
                /* HOST 통신상태 Update... */
                host->runStatus = SLAVE_PORT + 1;
            
                dnpData->rcvCount   = rxcnt;
                
                if(host->online[SLAVE_PORT] != 1)  
                {
                    logEvent_MPU(shmPtr, ENT_HOST_ONLINE, hostid+1, SLAVE_PORT, 0, hostid+1, NULL);    
                }
                host->online[SLAVE_PORT] = 1;
            
                /* ---------------------------- */
                /* check DATA-LINK Layer  ...   */
                /* ---------------------------- */
                retVal = chkRcvFrameHOST(hostid, SLAVE_PORT, socketID, dnpData);
                if((retVal == FUNC_USER_DATA) || (retVal == FUNC_UNCONFIRM))
                {
                    retVal = checkDataLinkHOST(hostid, SLAVE_PORT, socketID, dnpData);
                    if(retVal)
                    {
                        checkAPPFrame_HOST( hostid, SLAVE_PORT, socketID);
                    }                    
                }
            }   
        } // SLAVE PORT                 

         CHECK       
        if(oldsec == rtc->sec)  continue;
        oldsec = rtc->sec;
        CHECK
        /* ------------------------------------ */
        /* HOST-DB 변경에 따른 재기동 ...       */
        /* ------------------------------------ */
        if(opr->hostRestart[hostid] == SET)
        {
            printf("host%02d> *** HOST TASK Restart... DB Chg ...\n", hostid+1);
            pause(1000);
            termExec = RESET;
        }
        
        /* -------------------------------- */
        /* 상위 HOST별 통신상태 Check...    */
        /* -------------------------------- */
        if(++host->comFailTick[MASTER_PORT] > 30)   
        {
            host->comFailTick[MASTER_PORT] = 0;
            if(host->online[MASTER_PORT] != 0)  
            {
                logEvent_MPU(shmPtr, ENT_HOST_OFFLINE, hostid+1, MASTER_PORT, 0, hostid+1, NULL);    
            }
            host->online[MASTER_PORT] = 0;
            host->hostActive = HOST_NOT_READY;          /* HOST 통신모드 : 미지정 */   

            /* HOST 통신상태 Update... */
            host->runStatus = 3;                // HOST 통신상태 : 통신이상
            
            /* -------------------------------- */                        
            /* VME Channel Reinitial...         */
            /* VMECHAN 프로토콜 타입 지정       */
            /* -------------------------------- */       
            host->vmeChan[MASTER_PORT]->protocolType = SCAN_DNP;
            host->vmeChan[MASTER_PORT]->cfgType      = host->hostComType;
            host->vmeChan[MASTER_PORT]->cfgSpeed     = host->hostComSpeed;
 //  2026-02-01 오후 12:58:12          vmeHostChanReset 와 vmeHostInitial  순서변경
            if(opr->hostDebug == hostid)    Debug(console,">> DNP-HOST(%2d) : Master Comm Point initial...!\n", hostid+1);
                
            //hostVMEInitial(hostid);  
            //printf("Call vmeHostInitial %d %d\r\n",hostid, host->masterChan);            
            vmeHostInitial(hostid, host->masterChan);  
            vmeHostChanReset(hostid, MASTER_PORT);            
        
        }
         CHECK       
        /* DUAL 모드인 경우 : Master Port 가 OFFLINE 인 경우 */
        if(host->hostDualMode == HOST_DUAL)
        {
            if(++host->comFailTick[SLAVE_PORT] > 30)   
            {
                host->comFailTick[SLAVE_PORT] = 0;
                if(host->online[SLAVE_PORT] != 0)  
                {
                    logEvent_MPU(shmPtr, ENT_HOST_OFFLINE, hostid+1, SLAVE_PORT, 0, hostid+1, NULL);  
                }
                
                host->online[SLAVE_PORT] = 0;
				host->hostActive = HOST_NOT_READY;          /* HOST 통신모드 : 미지정 */   
				
                /* HOST 통신상태 Update... */
                host->runStatus = 3;                // HOST 통신상태 : 통신이상
            
                /* -------------------------------- */                        
                /* VME Channel Reinitial...         */
                /* VMECHAN 프로토콜 타입 지정       */
                /* -------------------------------- */       
                host->vmeChan[SLAVE_PORT]->protocolType = SCAN_DNP;
                host->vmeChan[SLAVE_PORT]->cfgType      = host->hostComType;
                host->vmeChan[SLAVE_PORT]->cfgSpeed     = host->hostComSpeed;
                
                vmeHostChanReset(hostid, SLAVE_PORT);
            
                if(opr->hostDebug == hostid)
                Debug(console,">> DNP-HOST(%2d) : Slave Comm Point initial...!\n", hostid+1);                           
        //    printf("Call vmeHostInitial %d %d\r\n",hostid, host->slaveChan);                    
                vmeHostInitial(hostid, host->slaveChan);  
                
            }
        }
                
	}/* while */

    return (0);
}


int    hostSerialTestThread(int hostid)
{
    //int     i;
	int		oldsec = 0;
	int     retVal;
	int     rxcnt;
	int		runTick=0;
	int		powerReset=SET;
	//byte    front, rear;
	int     socketID=0;
	int		localMaster=0;
	
	int  loopCount ; 
	
	DNP_DATA_LINK   *dnpData;

    //DNP_APP_FRAME   *rcvAppDnp;
    //DNP_APP_FRAME   *sndAppDnp;
	HOST_DCB        *host;
	DNP_SOE_QUEUE   *dnpSOEQ;
	
	
	char txbuf[50];
    int txcnt;	
    time_t now ; 
	
	/* -------------------------------- */
	/*  HOST별 Index ...                */
	/* -------------------------------- */
    host = (HOST_DCB *) hostDCB[hostid];                 /* 주장치 #1 속성정의 */
    
    dnpSOEQ = (DNP_SOE_QUEUE *) &host->dnpSOEQ;
    dnpSOEQ->front = 0;
    dnpSOEQ->rear  = 0;
    
    dnpSOEQ->front1= 0;
    dnpSOEQ->rear1 = 0;
    
    host->sndTHseqno = 0;
    host->cosReport  = 0;
    host->cosRear    = 0;
    
    host->soeReport  = 0;
    host->soeRear    = 0;
            
    /* HOST 통신 초기상태 ...   */
    host->online[MASTER_PORT]      = 0;    // default OFFLINE
    host->online[SLAVE_PORT]       = 0;    // default OFFLINE
    host->comFailTick[MASTER_PORT] = 0;
    host->comFailTick[SLAVE_PORT]  = 0;

    /* ---------------------------------------------------- */
    /*  Default VME Channel Comm Parameter                  */
    /* ---------------------------------------------------- */
    host->vmeChan[MASTER_PORT]->protocolType = SCAN_DNP;
    host->vmeChan[MASTER_PORT]->cfgType      = host->hostComType;
    host->vmeChan[MASTER_PORT]->cfgSpeed     = host->hostComSpeed;
    
    host->vmeChan[SLAVE_PORT]->protocolType  = SCAN_DNP;
    host->vmeChan[SLAVE_PORT]->cfgType       = host->hostComType;
    host->vmeChan[SLAVE_PORT]->cfgSpeed      = host->hostComSpeed;

    //hostVMEInitial(hostid);    
//    printf("Call vmeHostInitial %d %d\r\n",hostid, host->masterChan);
    vmeHostInitial(hostid, host->masterChan);  
//    printf("Call vmeHostInitial %d %d\r\n",hostid, host->slaveChan);
    vmeHostInitial(hostid, host->slaveChan);  
    
    vmeHostChanReset(hostid, MASTER_PORT);
    vmeHostChanReset(hostid, SLAVE_PORT);
    
    /* ------------------------ */
    /* SIO Board 를 사용함      */
    /* ------------------------ */
    opr->useSIOBoard = SET;
    
    /* ---------------------------------------------------- */
    /* ACE-RTU : 상위 HOST 통신포트 초기화                  */
    /* ---------------------------------------------------- */        
    if(host->hostDualMode == HOST_DUAL)             // 이중화 구조인 경우 
    {
        printf(" %s HOST[%d] Dual Serial TEST THREAD *DUAL [%d/%d]*  Activated ... !\n", TARGET_NAME, hostid+1, host->masterChan, host->slaveChan);
    }
    else if(host->hostDualMode == HOST_SINGLE)      // 개별 구조인 경우 
    {
        printf(" %s HOST[%d] Serial TEST DNP-THREAD *SINGLE [%d/%d]*  Activated ... !\n", TARGET_NAME, hostid+1, host->masterChan, host->slaveChan);
    }        
    else         // 사용하지 않는 경우 
    {
        printf(" %s HOST[%d] DNP-THREAD *NOT Define [%d/%d]*  Activated ... !\n", TARGET_NAME, hostid+1, host->masterChan, host->slaveChan);
        
        while(termExec)
	    {
	        wdtHostFlag[hostid] = 0;
		    pause(1000);        
		    
		    if(opr->wdtDebug)
		    Debug(console,">> *HOST[%d] DNP-THREAD *NOT Define ... !\n", hostid);
        }		    
    }

    
    host->activePort = MASTER_PORT;
    opr->hostRestart[hostid] = RESET;
    
    opr->dnpHostEnb  = SET;     // HOST : DNP HOST 정의시
    
    powerReset = SET;
    
    
    loopCount = 0 ;
	while(termExec)
	{
	    taskPtr->wdtCount = 0;
	    wdtHostFlag[hostid] = 0;
		pause(20);

        // 고의적 sleep
//        if ( ++loopCount < 50 )  continue  ;
//        loopCount = 0 ;     
            
//
    //PortWrite( tp, (char *) txbuf, txcnt);
 		bzero(txbuf, sizeof(txbuf));		
		time(&now);
        txcnt = sprintf(txbuf,"%d:%d %s",hostid+1,MASTER_PORT, ctime(&now));		 
        hostChanWrite( hostid, MASTER_PORT, 0, txbuf, txcnt);
        txcnt = sprintf(txbuf,"%d:%d %s",hostid+1,SLAVE_PORT, ctime(&now));	
        hostChanWrite( hostid, SLAVE_PORT, 0, txbuf, txcnt);
    
    if(opr->hostDebug == hostid)  
        DumpDNP_snd(console,"TXD:", txbuf, txcnt);
     
     sleep(1);   
        
#if 0
		/* ---------------------------------------------------- */
		/*	CPU 이중화 : LOCAL-SLAVE ... 대기모드 				*/
		/* ---------------------------------------------------- */
		if(opr->runMode == LOCAL_SLAVE)	
		{
		    /* 2020.06.03 절체시 불필요한 HOST-Online 이벤트 발생 */
		    host->online[MASTER_PORT] = 0;
		    host->online[SLAVE_PORT]  = 0;
		    host->hostActive = HOST_NOT_READY;          /* HOST 통신모드 : 미지정 */   
		    pause(1000);
		    
		    /* ------------------------------------ */
            /* HOST-DB 변경에 따른 재기동 ...       */
            /* ------------------------------------ */
            if(opr->hostRestart[hostid] == SET)
            {
                Debug(console, "host%02d> *** HOST TASK Restart... DB Chg ...\n", hostid+1);
                pause(1000);
                termExec = RESET;
            }
        
        	localMaster = LOCAL_SLAVE;
        	
        	/* 초기 기동시... */
        	if((powerReset == SET) && (++runTick > 10))
        	{
        		runTick = 0;
        		vmeHostInitial(hostid, host->masterChan); 
				vmeHostInitial(hostid, host->slaveChan); 
        		vmeHostChanReset(hostid, MASTER_PORT);
				vmeHostChanReset(hostid, SLAVE_PORT);
				
        	}
        		
		    continue;
		}
		
		/* -------------------------------------------- */
		/*	SLAVE MODE => MASTER MODE 전환시...			*/
		/* -------------------------------------------- */
		if(localMaster == LOCAL_SLAVE)
		{
			pause(1000);
			Debug(console, "host%02d> *** Change MASTER : HOST Channel-RESET ...\n", hostid+1);
			vmeHostInitial(hostid, host->masterChan); 
			vmeHostInitial(hostid, host->slaveChan); 
			vmeHostChanReset(hostid, MASTER_PORT);
			vmeHostChanReset(hostid, SLAVE_PORT);
		}
		
		powerReset = 0;
		localMaster = LOCAL_MASTER;
#endif
			
#if 0			// hkkim		
        /* ---------------------------------------------------- */
        /*  MASTER 통신포트 : 상위 데이터 수신 처리 ...         */
        /* ---------------------------------------------------- */
        dnpData   = (DNP_DATA_LINK *) &host->dataLinkFrame[MASTER_PORT];

        CHECK
            
        if((rxcnt = readDataLinkHOST( hostid, MASTER_PORT, socketID, dnpData)) > 0)
        {
            host->comFailTick[MASTER_PORT]= 0;
            host->comFailTick[SLAVE_PORT] = 0;
            
            if(opr->hostDebug == hostid)
            {
                Debug(console,"\n-----------------------------------------[%02d/%02d-%02d:%02d:%02d]\n", 
        	        rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
                //prn_rcv("RXM:", &dnpData->rcvFrame, rxcnt);
                DumpDNP_rcv(console, "RXM:", &dnpData->rcvFrame, rxcnt);
            }

            /* HOST 통신상태 Update... */
            host->runStatus = MASTER_PORT + 1;
                        
            dnpData->rcvCount        = rxcnt;
            if(host->online[MASTER_PORT] != 1)  
            {
                logEvent_MPU(shmPtr, ENT_HOST_ONLINE, hostid+1, MASTER_PORT, 0, hostid+1, NULL);  
            }
            host->online[MASTER_PORT] = 1;
            
            /* ---------------------------- */
            /* check DATA-LINK Layer  ...   */
            /* ---------------------------- */
            retVal = chkRcvFrameHOST(hostid, MASTER_PORT, socketID, dnpData);
            if((retVal == FUNC_USER_DATA) || (retVal == FUNC_UNCONFIRM))
            {
                retVal = checkDataLinkHOST(hostid, MASTER_PORT, socketID, dnpData);
                if(retVal)
                {
                    checkAPPFrame_HOST( hostid, MASTER_PORT, socketID);
                }                    
            }
        } // MASTER PORT      
        CHECK        
        /* ---------------------------------------------------- */
        /*  SLAVE  통신포트 : 상위 데이터 수신 처리 ...         */
        /* ---------------------------------------------------- */        
        if(host->hostDualMode == HOST_DUAL)
        {
            dnpData   = (DNP_DATA_LINK *) &host->dataLinkFrame[SLAVE_PORT];
         CHECK       
            if((rxcnt = readDataLinkHOST( hostid, SLAVE_PORT, socketID, dnpData)) > 0)
            {
                host->comFailTick[MASTER_PORT]= 0;
                host->comFailTick[SLAVE_PORT] = 0;
                    
                if(opr->hostDebug == hostid)
                {
                    Debug(console,"\n-----------------------------------------[%02d/%02d-%02d:%02d:%02d]\n", 
        	            rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
                    //prn_rcv("RXS:", &dnpData->rcvFrame, rxcnt);
                    DumpDNP_rcv(console, "RXS:", &dnpData->rcvFrame, rxcnt);
                }
                
                /* HOST 통신상태 Update... */
                host->runStatus = SLAVE_PORT + 1;
            
                dnpData->rcvCount   = rxcnt;
                
                if(host->online[SLAVE_PORT] != 1)  
                {
                    logEvent_MPU(shmPtr, ENT_HOST_ONLINE, hostid+1, SLAVE_PORT, 0, hostid+1, NULL);    
                }
                host->online[SLAVE_PORT] = 1;
            
                /* ---------------------------- */
                /* check DATA-LINK Layer  ...   */
                /* ---------------------------- */
                retVal = chkRcvFrameHOST(hostid, SLAVE_PORT, socketID, dnpData);
                if((retVal == FUNC_USER_DATA) || (retVal == FUNC_UNCONFIRM))
                {
                    retVal = checkDataLinkHOST(hostid, SLAVE_PORT, socketID, dnpData);
                    if(retVal)
                    {
                        checkAPPFrame_HOST( hostid, SLAVE_PORT, socketID);
                    }                    
                }
            }   
        } // SLAVE PORT                 
#endif 
         CHECK       
        if(oldsec == rtc->sec)  continue;
        oldsec = rtc->sec;
        CHECK
        /* ------------------------------------ */
        /* HOST-DB 변경에 따른 재기동 ...       */
        /* ------------------------------------ */
        if(opr->hostRestart[hostid] == SET)
        {
            printf("host%02d> *** HOST TASK Restart... DB Chg ...\n", hostid+1);
            pause(1000);
            termExec = RESET;
        }
        
#if  0 // hkkim        
        /* -------------------------------- */
        /* 상위 HOST별 통신상태 Check...    */
        /* -------------------------------- */
        if(++host->comFailTick[MASTER_PORT] > 30)   
        {
            host->comFailTick[MASTER_PORT] = 0;
            if(host->online[MASTER_PORT] != 0)  
            {
                logEvent_MPU(shmPtr, ENT_HOST_OFFLINE, hostid+1, MASTER_PORT, 0, hostid+1, NULL);    
            }
            host->online[MASTER_PORT] = 0;
            host->hostActive = HOST_NOT_READY;          /* HOST 통신모드 : 미지정 */   

            /* HOST 통신상태 Update... */
            host->runStatus = 3;                // HOST 통신상태 : 통신이상
            
            /* -------------------------------- */                        
            /* VME Channel Reinitial...         */
            /* VMECHAN 프로토콜 타입 지정       */
            /* -------------------------------- */       
            host->vmeChan[MASTER_PORT]->protocolType = SCAN_DNP;
            host->vmeChan[MASTER_PORT]->cfgType      = host->hostComType;
            host->vmeChan[MASTER_PORT]->cfgSpeed     = host->hostComSpeed;
            
            vmeHostChanReset(hostid, MASTER_PORT);

            if(opr->hostDebug == hostid)    Debug(console,">> DNP-HOST(%2d) : Master Comm Point initial...!\n", hostid+1);
                
            //hostVMEInitial(hostid);  
           //  printf("Call vmeHostInitial %d %d\r\n",hostid, host->masterChan);            
            vmeHostInitial(hostid, host->masterChan);  
            
        }
         CHECK       
        /* DUAL 모드인 경우 : Master Port 가 OFFLINE 인 경우 */
        if(host->hostDualMode == HOST_DUAL)
        {
            if(++host->comFailTick[SLAVE_PORT] > 30)   
            {
                host->comFailTick[SLAVE_PORT] = 0;
                if(host->online[SLAVE_PORT] != 0)  
                {
                    logEvent_MPU(shmPtr, ENT_HOST_OFFLINE, hostid+1, SLAVE_PORT, 0, hostid+1, NULL);  
                }
                
                host->online[SLAVE_PORT] = 0;
				host->hostActive = HOST_NOT_READY;          /* HOST 통신모드 : 미지정 */   
				
                /* HOST 통신상태 Update... */
                host->runStatus = 3;                // HOST 통신상태 : 통신이상
            
                /* -------------------------------- */                        
                /* VME Channel Reinitial...         */
                /* VMECHAN 프로토콜 타입 지정       */
                /* -------------------------------- */       
                host->vmeChan[SLAVE_PORT]->protocolType = SCAN_DNP;
                host->vmeChan[SLAVE_PORT]->cfgType      = host->hostComType;
                host->vmeChan[SLAVE_PORT]->cfgSpeed     = host->hostComSpeed;
                
                vmeHostChanReset(hostid, SLAVE_PORT);
            
                if(opr->hostDebug == hostid)
                Debug(console,">> DNP-HOST(%2d) : Slave Comm Point initial...!\n", hostid+1);                           
          //  printf("Call vmeHostInitial %d %d\r\n",hostid, host->slaveChan);                    
                vmeHostInitial(hostid, host->slaveChan);  
                
            }
        }
#endif 
                
	}/* while */

    return (0);
}
