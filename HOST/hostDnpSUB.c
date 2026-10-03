/* ======================================================================================== */
/*  철도청 통합 CU - 상위 DNP 공통 Library                                                  */
/* ======================================================================================== */
/*  Designed : Sanesys.Co.Ltd, LEE HO-SANG                                                  */
/*  Updated  : 2009. 04. 10                                                                 */
/* ======================================================================================== */
/*      - int sendDataLink_HOST(int hostid, int chid, byte thHeader, byte *src, int length) */
/*      - int sendAppFrame_HOST( int hostid, int chid, DNP_APP_FRAME *sndAppDnp)            */
/*      - int sndACConfirm(int hostid, int chid)                                            */
/*      - int funcAPP_read(int hostid, int chid)                                            */
/*      - int funcAPP_write(int hostid, int chid)                                           */
/*      - int funcAPP_select(int hostid, int chid)                                          */
/*      - int funcAPP_operate(int hostid, int chid)                                         */
/*      - int funcAPP_delayMeasure(int hostid, int chid)                                    */
/*      - int checkAPPFrame_HOST(int hostid, int chid )                                     */
/*      - int checkDataLinkHOST( int hostid, int chid, DNP_DATA_LINK *dnpData)              */
/*      - int chkRcvFrameHOST( int hostid, int chid, DNP_DATA_LINK *dnpData)                */
/*      -                                                                                   */
/*      -                                                                                   */
/* ======================================================================================== */

#include    "localLib.h"
#include    "external.h"

extern  int     hostChanWrite(int hostid, int chid, int socketID, byte *sndbuf, int sndCount);
extern  int     readDataLinkHOST(int hostid, int chid, int socketID, DNP_DATA_LINK *dnpData);
extern  int     chkRcvFrameHOST( int hostid, int chid, int socketID, DNP_DATA_LINK *dnpData);

extern  void    check_DNP_IIN(int hostid, int chid);
extern  int     get_binary_input(int hostid, int chid, byte *sndbuf, int variation);
extern  int     get_binary_change(int hostid, int chid, byte *sndbuf, int variation);
extern  int     get_analog_input(int hostid,  int chid, byte *sndbuf, int variation);
extern  int     get_analog_change(int hostid, int chid, byte *sndbuf, int variation);
extern  int     get_class_read(int hostid, int chid, byte *sndbuf, int variation);
extern  int     set_TimeDate(int hostid, int chid, int *dataPos);
extern  void    sendConfirmHOST( int hostid, int chid, int socketID, int code);


static char scanModType[10][8]   = {{"*NOT "}, {" SIO "},{"ESIO1"}, {"ESIO2"}, {"ESIO3"}, {"ESIO4"}, {" RTU"}, {" MPU#1"}, {" MPU#2"}};

/***************************************************
*   FUNCTION : sendDataLink_HOST()
***************************************************/
int sendDataLink_HOST(int hostid, int chid, int socketID, byte thHeader, byte *src, int length)
{
    int     i,j;
    //int     opcode;
    int     frameCnt;
    int     dataLen,bufPos;
    //int     sndCount;
    int     txcnt, rxcnt;
    int     retryCnt;
    byte    *txbuf;
    byte    control;
    word    crc;
    
    DL_FRAME        *sndData;
    DNP_DATA_LINK   *dnpData;
    HOST_DCB        *host;
    //TTY_DESC         *tp;

    host = (HOST_DCB *) hostDCB[hostid];  
    //tp   = (TTY_DESC *) host->ttyPort[chid];
            
    //dnpData  = (DNP_DATA_LINK *) &dataLinkFrame[hostid];
    dnpData  = (DNP_DATA_LINK *) &host->dataLinkFrame[chid];
    sndData  = (DL_FRAME *) &dnpData->sndFrame;

    control  = (host->sndFcbBit + host->sndFcvBit) + 0xc0 + FUNC_USER_DATA;
    control  = PRM_BIT + FUNC_UNCONFIRM;    

    txbuf = (byte *) &sndData->start1;

    /* ------------------------ */
    /*  Make HEADER frame ...   */
    /* ------------------------ */
    txcnt = 0;
    txbuf[txcnt++] = 0x05;
    txbuf[txcnt++] = 0x64;
    txbuf[txcnt++] = length + 5 + 1;        /* include header, TH,AC */
    txbuf[txcnt++] = control;               /* control */
    txbuf[txcnt++] = host->hostid & 0xff;
    txbuf[txcnt++] = (host->hostid >> 8) & 0xff;    
    txbuf[txcnt++] = host->rtuAddr & 0xff;
    txbuf[txcnt++] = (host->rtuAddr >> 8) & 0xff;
    crc = B013_mkdnpcrc(txbuf, txcnt);

    txbuf[txcnt++] = crc & 0xff;
    txbuf[txcnt++] = (crc >> 8) & 0xff;

    bufPos   = 0;
    frameCnt = (length / FRAME_LENGTH) + 1;

    /* -------------------------------------------- */
    /*  Make USER-DATA Block Multi send Block ....  */
    /* -------------------------------------------- */
    for(i=0; i< frameCnt; i++)
    {
        if(length <= 0) break;

        if(i==0)
        {
            if(length >= FRAME_LENGTH)  dataLen = FRAME_LENGTH-1;
            else    dataLen = length;

            txbuf[txcnt+ 0] = thHeader;
            for(j=0; j < dataLen; j++)  txbuf[txcnt+1+j] = src[bufPos++];

            length = length - dataLen;

            crc = B013_mkdnpcrc( &txbuf[txcnt], dataLen+1);
            txcnt = txcnt + dataLen + 1;
            txbuf[txcnt++] = crc & 0xff;
            txbuf[txcnt++] = (crc >> 8) & 0xff;
        }
        else
        {
            if(length >= FRAME_LENGTH)  dataLen = FRAME_LENGTH;
            else    dataLen = length;

            for(j=0; j < dataLen; j++)  txbuf[txcnt+j] = src[bufPos++];

            length = length - dataLen;

            crc = B013_mkdnpcrc( &txbuf[txcnt], dataLen);
            txcnt = txcnt + dataLen;
            txbuf[txcnt++] = crc & 0xff;
            txbuf[txcnt++] = (crc >> 8) & 0xff;
        }
    }

    retryCnt = 3;
    control  = control & 0x0f;

    if(opr->hostDebug == hostid)       
    {
        if(chid == MASTER_PORT)         DumpDNP_snd(console,"TXM:", txbuf, txcnt);
        else if(chid == SLAVE_PORT)     DumpDNP_snd(console,"TXS:", txbuf, txcnt);
    }

    /* ------------------------------------ */
    /*  DataLink 데이터 전송 ....Send ...   */
    /* ------------------------------------ */
    for(i=0; i< retryCnt; i++)
    {
        //if(PortWrite((TTY_DESC *) tp, (char *) txbuf, txcnt) > 0)
        if(hostChanWrite(hostid, chid, socketID, txbuf, txcnt) > 0)
        {
            /* ------------------------------------ */
            /*  DATA-LINK ACK 확인...               */
            /* ------------------------------------ */                
            if(control == FUNC_UNCONFIRM)
            {
                if(host->sndFcbBit) host->sndFcbBit = 0;
                else                host->sndFcbBit = FCB_BIT;
                return (OK);
            }
            else
            {
                if((rxcnt = readDataLinkHOST( hostid, chid, socketID, dnpData)) > 0)
                {
                    dnpData->rcvCount = rxcnt;
                    if(opr->hostDebug == hostid)   
                    {
                        if(chid == MASTER_PORT) DumpDNP_rcv(console, "RXM:", &dnpData->rcvFrame, rxcnt);
                        else                    DumpDNP_rcv(console, "RXS:", &dnpData->rcvFrame, rxcnt);
                    }
                
                    /* check Function Code ... */
                    if(chkRcvFrameHOST(hostid, chid, socketID, dnpData) == RCV_ACK)
                    {
                        if(host->sndFcbBit)  host->sndFcbBit = 0;
                        else                host->sndFcbBit = FCB_BIT;
                        return (OK);
                    }
                }
            }
                            
        }/**/

    }

    if(opr->hostDebug == hostid)
    Debug(console,"*** host [%2d] Channel LinkFail ...!\n", hostid+1);
    return (NOK);
    
}


/***************************************************
*   FUNCTION : sendAppFrame_HOST()
*   send User Layer : make send frame & send
***************************************************/
int sendAppFrame_HOST( int hostid, int chid, int socketID, DNP_APP_FRAME *sndAppDnp)
{
    int     i;

    int     length;
    //int     opcode;
    int     retVal;
    int     frameCnt, left;
    int     sndPos;
    int     dataLen;
    byte    control,thHeader;
    HOST_DCB *host;

    host = (HOST_DCB *) hostDCB[hostid];  
        
    /* ---------------------------- */
    /*  check Multi frame ...       */
    /* ---------------------------- */
    length   = sndAppDnp->sndCount;           /* App user data length */
    frameCnt = (length / APP_SEND_LEN);
    left     = (length % APP_SEND_LEN);

    if(left) frameCnt = frameCnt + 1;

    sndAppDnp->sndFrame    = frameCnt;
    sndPos = 0;

    if(length <= 0)
    {
        if(opr->hexDebug == hostid)
        Debug(console,"snd%2d> App send size (%d) error .... !\n", hostid+1,length);
        return (0);
    }

    control  = 0;
    thHeader = 0;
    sndPos   = 0;

    for(i=0; i< frameCnt; i++)
    {
        if(length <= 0) break;

        if(frameCnt == 1)               dataLen = length;
        else if(length >= APP_SEND_LEN) dataLen = APP_SEND_LEN;
        else                            dataLen = length;

        /* check Frame-Control Bit */
        if(i == 0)              control = TH_FIR_BIT;
        else                    control = 0;
        if(i == (frameCnt - 1)) control = control | TH_FIN_BIT;

        host->sndTHseqno = (host->sndTHseqno + 1) & 0x3f;
        thHeader  = control + host->sndTHseqno;
        //retVal = sendDataLink_HOST( tp, host, chid, thHeader, &sndAppDnp->userData[sndPos], dataLen);
        retVal = sendDataLink_HOST( hostid, chid, socketID, thHeader, &sndAppDnp->userData[sndPos], dataLen);

        if(retVal == OK)
        {
            //host->sndTHseqno = (host->sndTHseqno + 1) & 0x3f;
            //if(opr->hexDebug == chid)
            //Debug(console,"hsnd%2d> [%d] frame send OK.... !\n", hostid+1, i);
        }
        else
        {
            if(opr->hexDebug == hostid)
            Debug(console,"hsnd%2d> *** [%d] frame send failed ... !\n", hostid+1, i);
            return (NOK);
        }

        sndPos = sndPos + dataLen;
        length = length - dataLen;
        
        pause(100);
    }

    host->sndFlag    = SET;
    
    return (OK);
}


/***************************************************
    FUNCTION : sndACConfirm()
***************************************************/
void    sndACConfirm(int hostid, int chid, int socketID)
{
    int     txcnt;
    byte    *txbuf;
    byte    control, thHeader, acHeader;
    word    crc;
    DL_FRAME        *sndData;
    DNP_DATA_LINK   *dnpData;
    HOST_DCB        *host;
    //TTY_DESC        *tp;

    host = (HOST_DCB *) hostDCB[hostid];    
    //tp   = (TTY_DESC *) host->ttyPort[chid];
    
    //dnpData  = (DNP_DATA_LINK *) &dataLinkFrame[hostid];
    dnpData  = (DNP_DATA_LINK *) &host->dataLinkFrame[chid];
    sndData  = (DL_FRAME *) &dnpData->sndFrame;

    //control  = (host->sndFcbBit + host->sndFcvBit) + 0xc0 + FUNC_UNCONFIRM;
    control  = 0x40 + FUNC_UNCONFIRM;

    txbuf = (byte *) &sndData->start1;

    /* check Frame-Control Bit */
    host->sndTHseqno = (host->sndTHseqno + 1) & 0x3f;
    thHeader = TH_FIN_BIT + TH_FIR_BIT + host->sndTHseqno;
    acHeader = AC_FIN_BIT + AC_FIR_BIT + host->rcvACseq;
    
    /* ------------------------ */
    /*  Make HEADER frame ...   */
    /* ------------------------ */
    txcnt = 0;
    txbuf[0] = 0x05;
    txbuf[1] = 0x64;
    txbuf[2] = 3 + 5 + 2;                 /* include header, TH,AC,FC */
    txbuf[3] = control;               /* control */
    txbuf[4] = host->hostid & 0xff;
    txbuf[5] = (host->hostid >> 8) & 0xff;    
    txbuf[6] = host->rtuAddr & 0xff;
    txbuf[7] = (host->rtuAddr >> 8) & 0xff;
    crc = B013_mkdnpcrc(txbuf, 8);

    txbuf[8] = crc & 0xff;
    txbuf[9] = (crc >> 8) & 0xff;
    
    txbuf[10] = thHeader;   /* TH */
    txbuf[11] = acHeader;   /* AC */
    txbuf[12] = 0x0;        /* FC : confirm */
    txbuf[13] = 0x10;       /* IIN */
    txbuf[14] = 0x0;        /* IIN */
    
    crc = B013_mkdnpcrc( &txbuf[10], 5);
    txbuf[15] = crc & 0xff;
    txbuf[16] = (crc >> 8) & 0xff;
    txcnt = 17;

    //PortWrite( tp, (char *) txbuf, txcnt);
    hostChanWrite( hostid, chid, socketID, txbuf, txcnt);
    
    //if(opr->hexDebug == chid)   Debug(console,"acfm%2d> AC Confirm ...!\n", chid);
    if(opr->hostDebug == hostid)  DumpDNP_snd(console,"APPCFM:", txbuf, txcnt);
    
    pause(100);
}




/*
*
*/
int funcAPP_read(int hostid, int chid)
{
    int     rcvCount;
    int     dataPos, sndPos, count;
    //byte    acHeader, requestCON;
    //int     object,variation,qualifier;
    //int     indexSize, qcode;
    
    DNP_APP_FRAME   *rcvAppDnp;
    DNP_APP_FRAME   *sndAppDnp;
    HOST_DCB        *host;

    DNP_SOE_QUEUE   *soeQ;  
    DNP_COS_QUEUE   *cosQ;  
    
    host = (HOST_DCB *) hostDCB[hostid];        
    
    //rcvAppDnp = (DNP_APP_FRAME *) &rcvAppFrame[hostid];
    //sndAppDnp = (DNP_APP_FRAME *) &sndAppFrame[hostid];
    rcvAppDnp = (DNP_APP_FRAME *) &host->rcvAppFrame[chid];
    sndAppDnp = (DNP_APP_FRAME *) &host->sndAppFrame[chid];
    rcvCount  = rcvAppDnp->rcvSize;

    dataPos = 2;  /* execpt AC & FC area */             
    
    /* -------------------------------------------------------- */
    /* Make APPLICATION Message FORMAT :                        */
    /* -------------------------------------------------------- */
    check_DNP_IIN(hostid, chid);
        
    sndPos = 0;
    sndAppDnp->userData[0] = 0xc0 + host->rcvACseq;     /* AC */
    sndAppDnp->userData[1] = APP_RCV_RESPONSE;          /* Function */
    sndAppDnp->userData[2] = host->rtuIIN[0];           // IIN
    sndAppDnp->userData[3] = host->rtuIIN[1];           // IIN
    sndPos = 4;
        
    /* check Multi-Obj */
    while(dataPos < rcvCount)
    {
        host->rcvObj    = rcvAppDnp->userData[dataPos++];
        host->rcvVar    = rcvAppDnp->userData[dataPos++];
        host->rcvQcode  = (rcvAppDnp->userData[dataPos]) & 0x0f;
        host->rcvIndex  = (rcvAppDnp->userData[dataPos] >> 4) & 0x07;
        dataPos++;

        switch(host->rcvQcode)         /* data size check ... */
        {
            case 0:     /* Range: 8bit Start & Stop */
            case 3:     /* Range: 8bit Abs Start & Stop */    
                host->rcvStart   = rcvAppDnp->userData[dataPos++];
                host->rcvStop    = rcvAppDnp->userData[dataPos++];
                host->rcvDataPos = dataPos;
                break;
        
            case 1:     /* Range: 16bit Start & Stop */
            case 4:     /* Range: 16bit Abs Start & Stop */    
                host->rcvStart  = rcvAppDnp->userData[dataPos+0] 
                                  + ((rcvAppDnp->userData[dataPos+1] << 8) & 0xff00);
                host->rcvStop   = rcvAppDnp->userData[dataPos+2] 
                                  + ((rcvAppDnp->userData[dataPos+3] << 8) & 0xff00);
                dataPos += 4;                                  
                host->rcvDataPos = dataPos;    
                break;
        
            case 2:     /* Range: 32bit Start & Stop */
            case 5:     /* Range: 32bit Abs Start & Stop */    
                host->rcvStart   = rcvAppDnp->userData[dataPos+0] 
                                  + ((rcvAppDnp->userData[dataPos+1] << 8) & 0x0000ff00) 
                                  + ((rcvAppDnp->userData[dataPos+2] << 16) & 0x00ff0000)
                                  + ((rcvAppDnp->userData[dataPos+3] << 24) & 0xff000000);

                host->rcvStop    = rcvAppDnp->userData[dataPos+4] 
                                  + ((rcvAppDnp->userData[dataPos+5] << 8) & 0x0000ff00) 
                                  + ((rcvAppDnp->userData[dataPos+6] << 16) & 0x00ff0000)
                                  + ((rcvAppDnp->userData[dataPos+7] << 24) & 0xff000000);
                dataPos += 8; 
                host->rcvDataPos = dataPos;                                                       
                break;
        
            case 6:     /* All Object : No Range */
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                host->rcvDataPos = dataPos;
                break;

            case 7:     /* Range : data quantity */             
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                host->rcvQnum    = rcvAppDnp->userData[dataPos++];   /* quentity number */
                host->rcvDataPos = dataPos;
                
                //if(opr->hexDebug == hostid)
                //Debug(console,"hrcv%2d> Single Field Quqntity...Qualifier=%2d, Qnum=%2d\n", hostid+1, host->rcvQcode, host->rcvQnum);
                break;
    
            case 8:
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                host->rcvQnum    = rcvAppDnp->userData[dataPos+0] 
                                  + ((rcvAppDnp->userData[dataPos+1] << 8) & 0xff00);
                dataPos += 2;                                   
                host->rcvDataPos = dataPos;
                
                //if(opr->hexDebug == hostid)
                //    Debug(console,"hrcv%2d> Single Field Quqntity...Qualifier=%2d, Qnum=%2d\n", hostid+1, host->rcvQcode, host->rcvQnum);            
                break;
    
            case 9:
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                host->rcvQnum    = rcvAppDnp->userData[dataPos+0] 
                                  + ((rcvAppDnp->userData[dataPos+1] << 8) & 0x0000ff00) 
                                  + ((rcvAppDnp->userData[dataPos+2] << 16) & 0x00ff0000)
                                  + ((rcvAppDnp->userData[dataPos+3] << 24) & 0xff000000);
                dataPos += 4;                                   
                host->rcvDataPos = dataPos;
                
                //if(opr->hexDebug == hostid)
                //    Debug(console,"hrcv%2d> Single Field Quqntity...Qualifier=%2d, Qnum=%2d\n", hostid+1, host->rcvQcode, host->rcvQnum);             
                break;                        
    
            case 11:    // Free Format Qualifier 
                if(opr->hexDebug == hostid)
                    Debug(console,"hrcv%2d> READ FREE Format Quqntity...Qualifier=%2d\n", hostid+1, host->rcvQcode); 
                break;        
    
            default:
                break;
        }/*switch */

        if(opr->hexDebug == hostid)
        Debug(console,"hrcv%2d> -----> READ   [%02x] Func=%2d, OBJ=%2d, VAR=%2d, Index=%2d, QCode=%2d...Start=%2d, Stop=%2d, DataPos=%2d\n", 
            hostid+1, host->rcvACseq, host->rcvACfunc, host->rcvObj, host->rcvVar, host->rcvIndex, host->rcvQcode, 
            host->rcvStart, host->rcvStop, host->rcvDataPos);

        switch(host->rcvObj)
        {        
        case OBJ_BINARY_INPUT:          // Single-BIT Binary Input 
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> BINARY_INPUT  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);        
            count = get_binary_input(hostid, chid, &sndAppDnp->userData[sndPos], host->rcvVar);
            sndPos += count;
            
            /* ---------------------------------------- */
            /*  UN-SOLICIT 메세지 전송 포트 지정...     */
            /*  - 이중화 포트중 Active Port 설정        */
            /* ---------------------------------------- */
            host->activePort = chid;
            
            break;
            
        case OBJ_BINARY_CHANGE:         // Binary Input Change Without Time
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> BINARY_CHANGE  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            count = get_binary_change(hostid, chid, &sndAppDnp->userData[sndPos], host->rcvVar);
            sndPos += count;
            
            /* 이벤트 정보 전송에 따른 HOST-ACK 요구 */
            //if((host->rcvVar == 1) && (host->cosReport > 0)) sndAppDnp->userData[0] = 0xc0 + host->rcvACseq + CONFIRM_BIT;     /* AC */
            //if((host->rcvVar == 1) && (host->soeReport > 0)) sndAppDnp->userData[0] = 0xc0 + host->rcvACseq + CONFIRM_BIT;     /* AC */

            /* 이벤트 정보 전송에 따른 HOST-ACK 요구 */
            if((host->soeReport + host->cosReport) > 0) 
            {
                sndAppDnp->userData[0] = 0xc0 + host->rcvACseq + CONFIRM_BIT;     /* AC */  
                    
                if(opr->soeDebug)
                Debug(console,"host%2d> Binary Change response...[soe=%d, cos=%d]\n", hostid+1, host->soeReport, host->cosReport);
            }
            
            /* ---------------------------------------- */
            /*  UN-SOLICIT 메세지 전송 포트 지정...     */
            /*  - 이중화 포트중 Active Port 설정        */
            /* ---------------------------------------- */
            host->activePort = chid;
                        
            break;
            
        case OBJ_BINARY_OUTPUT:         // Binary Output 
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> BINARY_OUTPUT VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            break;
        case OBJ_CONTROL_RELAY:         // Control Relay Output
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> CONTROL_RELAY  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            break;
        case OBJ_BINARY_COUNTER:        // Binary Counter 
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> BINARY_COUNTER  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            break;
        case OBJ_FROZEN_COUNTER:        // 32-Bit Frozen Counter
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> FROZEN COUNTER  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            break;
        case OBJ_COUNTER_CHANGE:        // 32-Bit Counter Change Event Without Time
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> COUNTER_CHANGE  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            break;
        case OBJ_FC_CHANGE:             // 32-Bit Frozen Counter Change Event Without Time
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> FC_CHANGE  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            break;

        case OBJ_ANALOG_INPUT:          // 32-Bit Analog Input
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> Analog Input VAR=%2d, Index_Size=%d, Qualifier=%2d...Start=%d, Stop=%d, DataPos=%d\n", hostid+1,
                host->rcvVar, host->rcvIndex, host->rcvQcode, host->rcvStart, host->rcvStop, host->rcvDataPos);

            count = get_analog_input(hostid, chid, &sndAppDnp->userData[sndPos], host->rcvVar);
            sndPos += count;
            
            /* ---------------------------------------- */
            /*  UN-SOLICIT 메세지 전송 포트 지정...     */
            /*  - 이중화 포트중 Active Port 설정        */
            /* ---------------------------------------- */
            host->activePort = chid;

            break;

        case OBJ_FROZEN_ANALOG:         // 32-Bit Frozen Analog Input
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> FROZEN_ANALOG VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            break;
            
        case OBJ_ANALOG_CHANGE:         // 32-Bit Analog Change Event Without Time
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> ANALOG CHANGE  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
#if 1            
            count = get_analog_change(hostid, chid, &sndAppDnp->userData[sndPos], host->rcvVar);
            sndPos += count;
            
            /* 이벤트 정보 전송에 따른 HOST-ACK 요구 */
            if(host->coaReport > 0)     sndAppDnp->userData[0] = 0xc0 + host->rcvACseq + CONFIRM_BIT;     /* AC */
#endif
            break;
                        
            break;
        case OBJ_FA_CHANGE:             // 32-Bit Frozen Analog Change Event Without Time
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> FA_CHANGE  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            break;
        case OBJ_ANALOG_OUTPUT:         // 32-Bit Analog Output
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> ANALOG_OUTPUT  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            break;
        case OBJ_AOUT_BLOCK:            // 32-Bit Analog Output Block
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> AOUT_BLOCK VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            break;
        case OBJ_TIME_AND_DATE:         // Time and Date
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> TIME_AND_DATE  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            break;
        case OBJ_TIME_AND_DATE_CTO:     // Time and Date with CTO
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> TIME_AND_DATE_CTO  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            break;
        case OBJ_TIME_DELAY_COA:        // Time Delay COARSE
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> TIME_DELAY_COA  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            break;
            
        case OBJ_DEVICE_IIN:            // Device Object : Internal Indication
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> IIN Read  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            
            /* ------------------------ */
            /* IIN 수신시 ... 예비모드  */
            /* ------------------------ */
            if((++host->iinRcvTick > 5) || (host->rcvVar == 0x02))
            {   
            	if(opr->hexDebug == hostid)
            	{	
	                if(host->hostActive != HOST_READY_STS)
    	            Debug(console,"hrcv%2d> >>>>>> Change HOST-MODE : READY_STATUS...!\n", hostid+1);  
        		}
        		        
                /* TCPIP 통신인 경우... */
                if(host->hostComType == COM_TCPIP)
                {     
                    host->hostActive = HOST_READY_STS;      // 예비채널 정의
                    host->iinRcvTick = 10;
                    
                    host->sndTHseqno = 0;
                    host->cosReport  = 0;
                    host->cosRear    = 0;
                    host->soeReport  = 0;
                    host->soeRear    = 0;
            
                    /* ------------------------------------------ */
                    /* 예비 HOST 인 경우 ... 이벤트 내용 Clear... */
                    /* 이벤트 Queue Clear... */
                    /* ------------------------------------------ */
                    soeQ    = (DNP_SOE_QUEUE *) &host->dnpSOEQ;
                    soeQ->front = 0;
                    soeQ->rear  = 0;
                
                    cosQ    = (DNP_COS_QUEUE *) &host->dnpCOSQ;
                    cosQ->front = 0;
                    cosQ->rear  = 0;
                }
                
            }
            break;
            
        case OBJ_CLASS:                 // Class 
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> CLASS  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            if( host->rcvVar == DNP_CLASS0)      // Class 0 Read : All Pre Define Class 0
            {
                // Single-BIT Binary Response 
                count = get_binary_input(hostid, chid, &sndAppDnp->userData[sndPos], 1);
                sndPos += count;
                
                // 32Bit Analog Response 
                count = get_analog_input(hostid, chid, &sndAppDnp->userData[sndPos], 5);
                sndPos += count;                
            }

            /* -------------------------------------------------------- */
            /*  HOST CLASS 정보 : CLASS1-SOE, CLASS2-COS, CLASS3-COA    */ 
            /* -------------------------------------------------------- */                    
            if((host->rcvVar == DNP_CLASS1) || (host->rcvVar == DNP_CLASS2) ||(host->rcvVar == DNP_CLASS3)) // Class 1 Read : SOE
            {
                count = get_class_read(hostid, chid, &sndAppDnp->userData[sndPos], host->rcvVar);
                sndPos += count;
                
                /* 이벤트 정보 전송에 따른 HOST-ACK 요구 */
                if((host->soeReport + host->cosReport) > 0) 
                {
                    sndAppDnp->userData[0] = 0xc0 + host->rcvACseq + CONFIRM_BIT;     /* AC */  
                    
                    if(opr->soeDebug)
                    Debug(console,"host%2d> CLASS-Event response...[soe=%d, cos=%d]\n", hostid+1, host->soeReport, host->cosReport);
                }
                
                /* ---------------------------------------- */
                /*  UN-SOLICIT 메세지 전송 포트 지정...     */
                /*  - 이중화 포트중 Active Port 설정        */
                /* ---------------------------------------- */
                host->activePort = chid;
            }                            
            break;
            
        default:
            break;
        }

    } /* while */
    
    return (sndPos);
            
}


/*
*
*/
int funcAPP_write(int hostid, int chid)
{
    int     rcvCount;
    int     dataPos, sndPos ;
   // byte    acHeader, requestCON;
    //int     object,variation,qualifier;
    //int     indexSize, qcode;
    
    DNP_APP_FRAME   *rcvAppDnp;
    DNP_APP_FRAME   *sndAppDnp;
    HOST_DCB *host;

    host = (HOST_DCB *) hostDCB[hostid];        
    
    //rcvAppDnp = (DNP_APP_FRAME *) &rcvAppFrame[hostid];
    //sndAppDnp = (DNP_APP_FRAME *) &sndAppFrame[hostid];
    rcvAppDnp = (DNP_APP_FRAME *) &host->rcvAppFrame[chid];
    sndAppDnp = (DNP_APP_FRAME *) &host->sndAppFrame[chid];
    rcvCount  = rcvAppDnp->rcvSize;

    dataPos = 2;  /* execpt AC & FC area */             

    /* -------------------------------------------------------- */
    /* Make APPLICATION Message FORMAT :                        */
    /* -------------------------------------------------------- */
    check_DNP_IIN(hostid, chid);
         
    sndPos = 0;
    sndAppDnp->userData[0] = 0xc0 + host->rcvACseq;     /* AC */
    sndAppDnp->userData[1] = APP_RCV_RESPONSE;          /* Function */
    sndAppDnp->userData[2] = host->rtuIIN[0];           // IIN
    sndAppDnp->userData[3] = host->rtuIIN[1];           // IIN
    sndPos = 4;
            
    /* check Multi-Obj */
    while(dataPos < rcvCount)
    {
        host->rcvObj    = rcvAppDnp->userData[dataPos++];
        host->rcvVar    = rcvAppDnp->userData[dataPos++];
        host->rcvQcode  = (rcvAppDnp->userData[dataPos]) & 0x0f;
        host->rcvIndex  = (rcvAppDnp->userData[dataPos] >> 4) & 0x07;
        dataPos++;

        switch(host->rcvQcode)         /* data size check ... */
        {
            case 0:     /* Range: 8bit Start & Stop */
            case 3:     /* Range: 8bit Abs Start & Stop */    
                host->rcvStart   = rcvAppDnp->userData[dataPos++];
                host->rcvStop    = rcvAppDnp->userData[dataPos++];
                host->rcvDataPos = dataPos;
                break;
        
            case 1:     /* Range: 16bit Start & Stop */
            case 4:     /* Range: 16bit Abs Start & Stop */    
                host->rcvStart  = rcvAppDnp->userData[dataPos+0] 
                                  + ((rcvAppDnp->userData[dataPos+1] << 8) & 0xff00);
                host->rcvStop   = rcvAppDnp->userData[dataPos+2] 
                                  + ((rcvAppDnp->userData[dataPos+3] << 8) & 0xff00);
                dataPos += 4;                                   
                host->rcvDataPos = dataPos;    
                break;
        
            case 2:     /* Range: 32bit Start & Stop */
            case 5:     /* Range: 32bit Abs Start & Stop */    
                host->rcvStart   = rcvAppDnp->userData[dataPos+0] 
                                  + ((rcvAppDnp->userData[dataPos+1] << 8) & 0x0000ff00) 
                                  + ((rcvAppDnp->userData[dataPos+2] << 16) & 0x00ff0000)
                                  + ((rcvAppDnp->userData[dataPos+3] << 24) & 0xff000000);

                host->rcvStop    = rcvAppDnp->userData[dataPos+4] 
                                  + ((rcvAppDnp->userData[dataPos+5] << 8) & 0x0000ff00) 
                                  + ((rcvAppDnp->userData[dataPos+6] << 16) & 0x00ff0000)
                                  + ((rcvAppDnp->userData[dataPos+7] << 24) & 0xff000000);
                dataPos += 8;                                   
                host->rcvDataPos = dataPos;                                                       
                break;
        
            case 6:     /* All Object : No Range */
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                host->rcvDataPos = dataPos;
                break;

            case 7:     /* Range : data quantity */             
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                host->rcvQnum    = rcvAppDnp->userData[dataPos++];   /* quentity number */
                host->rcvDataPos = dataPos;
                
                //if(opr->hexDebug == hostid)
                //Debug(console,"hrcv%2d> Single Field Quqntity...Qualifier=%2d, Qnum=%2d\n", hostid+1, host->rcvQcode, host->rcvQnum);
                break;
    
            case 8:
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                host->rcvQnum    = rcvAppDnp->userData[dataPos+0] 
                                  + ((rcvAppDnp->userData[dataPos+1] << 8) & 0xff00);
                dataPos += 2;                                   
                host->rcvDataPos = dataPos;
                
                //if(opr->hexDebug == hostid)
                //    Debug(console,"hrcv%2d> Single Field Quqntity...Qualifier=%2d, Qnum=%2d\n", hostid+1, host->rcvQcode, host->rcvQnum);            
                break;
    
            case 9:
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                host->rcvQnum    = rcvAppDnp->userData[dataPos+0] 
                                  + ((rcvAppDnp->userData[dataPos+1] << 8) & 0x0000ff00) 
                                  + ((rcvAppDnp->userData[dataPos+2] << 16) & 0x00ff0000)
                                  + ((rcvAppDnp->userData[dataPos+3] << 24) & 0xff000000);
                dataPos += 4;                                   
                host->rcvDataPos = dataPos;
                
                //if(opr->hexDebug == hostid)
                //    Debug(console,"hrcv%2d> Single Field Quqntity...Qualifier=%2d, Qnum=%2d\n", hostid+1, host->rcvQcode, host->rcvQnum);             
                break;                        
    
            case 11:    // Free Format Qualifier 
                if(opr->hexDebug == hostid)
                    Debug(console,"hrcv%2d> WRITE FREE Format Quqntity...Qualifier=%2d\n", hostid+1, host->rcvQcode); 
                break;        
    
            default:
                break;
        }/*switch */

        if(opr->hexDebug == hostid)
        Debug(console,"hrcv%2d> -----> WRITE  [%02x] Func=%2d, OBJ=%2d, VAR=%2d, Index=%2d, QCode=%2d...Start=%2d, Stop=%2d, DataPos=%2d\n", 
            hostid+1, host->rcvACseq, host->rcvACfunc, host->rcvObj, host->rcvVar, host->rcvIndex, host->rcvQcode, 
            host->rcvStart, host->rcvStop, host->rcvDataPos);

        switch(host->rcvObj)
        {        
        case OBJ_TIME_AND_DATE:          // Time and Date
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> TIME and DATE write  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);        
            set_TimeDate(hostid, chid, &dataPos);
            break;

        case OBJ_DEVICE_IIN:
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> Internal INDICATION write  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            
            host->sdpRestart = RESET;
            
            host->rtuIIN[0] = 0;
            host->rtuIIN[1] = 0;
            
            sndAppDnp->userData[2] = host->rtuIIN[0];           // IIN
            sndAppDnp->userData[3] = host->rtuIIN[1];           // IIN
            
            // Next Object 처리 ...
            dataPos += 4; 
            break;
                        
        default:
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> default Write Function...VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode); 
            break;
        }

    } /* while */

    
    return (sndPos);
            
}


/*
* DNP : SELECT() 
*/
int funcAPP_select(int hostid, int chid)
{
    int     i;
    int     rcvCount;
    int     dataPos, sndPos;
    //byte    acHeader, requestCON;
    //int     object,variation,qualifier;
    //int     indexSize, qcode;
    int     point=0, code, count, ontime, offtime, status;
    
    
    DNP_APP_FRAME   *rcvAppDnp;
    DNP_APP_FRAME   *sndAppDnp;
    HOST_DCB        *host;

    /* -------------------------------------------- */
    /*  HOST 제어정보 추출...                       */
    /* -------------------------------------------- */
    host = (HOST_DCB *) hostDCB[hostid];        
    rcvAppDnp = (DNP_APP_FRAME *) &host->rcvAppFrame[chid];
    sndAppDnp = (DNP_APP_FRAME *) &host->sndAppFrame[chid];
    rcvCount  = rcvAppDnp->rcvSize;

    dataPos = 2;  /* execpt AC & FC area */             

    /* -------------------------------------------------------- */
    /* Make APPLICATION Message FORMAT :                        */
    /* -------------------------------------------------------- */
    check_DNP_IIN(hostid, chid);
         
    sndPos = 0;
    sndAppDnp->userData[0] = 0xc0 + host->rcvACseq;     /* AC */
    sndAppDnp->userData[1] = APP_RCV_RESPONSE;          /* Function */
    sndAppDnp->userData[2] = host->rtuIIN[0];           // IIN
    sndAppDnp->userData[3] = host->rtuIIN[1];           // IIN
    sndPos = 4;
            
    /* check Multi-Obj */
    while(dataPos < rcvCount)
    {
        host->rcvObj    = rcvAppDnp->userData[dataPos++];
        host->rcvVar    = rcvAppDnp->userData[dataPos++];
        host->rcvQcode  = (rcvAppDnp->userData[dataPos]) & 0x0f;
        host->rcvIndex  = (rcvAppDnp->userData[dataPos] >> 4) & 0x07;
        dataPos++;

        //if(opr->hexDebug == hostid)
        //Debug(console,"hrcv%2d> SELECT...Func = %2d, OBJ=%2d, VAR=%2d, Index_Size=%d, Qualifier=%2d\n", hostid+1,
        //    host->rcvACfunc, host->rcvObj, host->rcvVar, host->rcvIndex, host->rcvQcode);
        
        switch(host->rcvQcode)         /* data size check ... */
        {
            case 7:     /* Range : 8bit single-field quantity */            
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                if(host->rcvIndex == 1)
                {
                    host->rcvQnum = rcvAppDnp->userData[dataPos++];
                }
                else if(host->rcvIndex == 2)
                {
                    host->rcvQnum = rcvAppDnp->userData[dataPos+0] + ((rcvAppDnp->userData[dataPos+1] << 8) & 0xff00);
                    dataPos += 2;                                     
                }                
                host->rcvDataPos = dataPos;
                break;
                
           case 8:     /* Range : 16bit single-field quantity */             
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                
                if(host->rcvIndex == 1)
                {
                    host->rcvQnum = rcvAppDnp->userData[dataPos++];
                }
                else if(host->rcvIndex == 2)
                {
                    host->rcvQnum = rcvAppDnp->userData[dataPos + 0] + ((rcvAppDnp->userData[dataPos + 1] << 8) & 0xff00);
                    dataPos += 2;                                    
                }
                host->rcvDataPos = dataPos;
                break;
                
            default:
                Debug(console,"hrcv%2d> SELECT...Func : *Invalid Qcode =%2d\n", hostid+1, host->rcvQcode);
                return (0);
                break;
        }/*switch */

        if(opr->hexDebug == hostid)
        Debug(console,"hrcv%2d> -----> SELECT [%02x] Func=%2d, OBJ=%2d, VAR=%2d, Index=%2d, QCode=%2d...Start=%2d, Stop=%2d, DataPos=%2d\n", 
            hostid+1, host->rcvACseq, host->rcvACfunc, host->rcvObj, host->rcvVar, host->rcvIndex, host->rcvQcode, 
            host->rcvStart, host->rcvStop, host->rcvDataPos);


        switch(host->rcvObj)
        {        
        case OBJ_CONTROL_RELAY:         // Control Relay Output
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> Select... CONTROL_RELAY  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            
            if(host->rcvVar == 1)
            {
                for(i=0; i< host->rcvQnum; i++)
                {
                    if(host->rcvQcode == 7)
                    {
                        point = rcvAppDnp->userData[dataPos++];     // select point
                    }
                    else if(host->rcvQcode == 8)
                    {
                        point = rcvAppDnp->userData[dataPos+0] + ((rcvAppDnp->userData[dataPos+1] << 8) & 0xff00);     
                        dataPos += 2;                                 
                    }
                    
                    code    = rcvAppDnp->userData[dataPos++];     // control code
                    count   = rcvAppDnp->userData[dataPos++];     // control count
                    
                    ontime  = rcvAppDnp->userData[dataPos+0] 
                              + ((rcvAppDnp->userData[dataPos+1] << 8) & 0x0000ff00) 
                              + ((rcvAppDnp->userData[dataPos+2] << 16) & 0x00ff0000)
                              + ((rcvAppDnp->userData[dataPos+3] << 24) & 0xff000000);
                    dataPos += 4;                              
                    
                    offtime = rcvAppDnp->userData[dataPos+0] 
                              + ((rcvAppDnp->userData[dataPos+1] << 8) & 0x0000ff00) 
                              + ((rcvAppDnp->userData[dataPos+2] << 16) & 0x00ff0000)
                              + ((rcvAppDnp->userData[dataPos+3] << 24) & 0xff000000);
                    dataPos += 4;  
                              
                    status  = rcvAppDnp->userData[dataPos++];
                    
                    if(opr->soeDebug)
                    Debug(console,"hrcv%2d> SELECT  Info : Point=%d, code=%2x, count=%d, ontime=%d, offtime=%d, status=%2x\n",
                        hostid+1, point+1, code, count, ontime, offtime, status);

                }

                /* ------------------------------------ */
                /* Select 응답...                       */
                /* ------------------------------------ */
                memcpy( &sndAppDnp->userData[sndPos], &rcvAppDnp->userData[2], rcvCount - 3);
                sndPos += rcvCount - 3;
                sndAppDnp->userData[sndPos++] = 0;          // status
                                
            }
            break;
            
        default:
            break;
        }

    } /* while */

    return (sndPos);
            
}


/*
* DNP : OPERATE() 
*/
int funcAPP_operate(int hostid, int chid)
{
    int     i;
    int     mode = 0;
    int     rcvCount;
    int     dataPos, sndPos;
    int     point=0, code, count, ontime, offtime, status;
    int     cntrDevice, cntrPoint, cntrTCF, cntrType, cntrInfo;
    char    buffer[256];
    struct timeval  ctime;
    
    DNP_APP_FRAME   *rcvAppDnp;
    DNP_APP_FRAME   *sndAppDnp;
    HOST_DCB        *host;
    CONTROL_INFO    *hostCntr;
    SDP_DEVICE      *dev;
    
    /* -------------------------------------------- */
    /*  HOST 제어정보 추출...                       */
    /* -------------------------------------------- */
    host = (HOST_DCB *) hostDCB[hostid];        
    
    rcvAppDnp = (DNP_APP_FRAME *) &host->rcvAppFrame[chid];
    sndAppDnp = (DNP_APP_FRAME *) &host->sndAppFrame[chid];
    rcvCount  = rcvAppDnp->rcvSize;

    dataPos = 2;  /* execpt AC & FC area */             

    /* -------------------------------------------------------- */
    /* Make APPLICATION Message FORMAT :                        */
    /* -------------------------------------------------------- */
    check_DNP_IIN(hostid, chid);
         
    sndPos = 0;
    sndAppDnp->userData[0] = 0xc0 + host->rcvACseq;     /* AC */
    sndAppDnp->userData[1] = APP_RCV_RESPONSE;          /* Function */
    sndAppDnp->userData[2] = host->rtuIIN[0];           // IIN
    sndAppDnp->userData[3] = host->rtuIIN[1];           // IIN
    sndPos = 4;
            
    /* check Multi-Obj */
    while(dataPos < rcvCount)
    {
        host->rcvObj    = rcvAppDnp->userData[dataPos++];
        host->rcvVar    = rcvAppDnp->userData[dataPos++];
        host->rcvQcode  = (rcvAppDnp->userData[dataPos]) & 0x0f;
        host->rcvIndex  = (rcvAppDnp->userData[dataPos] >> 4) & 0x07;
        dataPos++;

        switch(host->rcvQcode)         /* data size check ... */
        {
            case 7:     /* Range : 8bit single-field quantity */            
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                if(host->rcvIndex == 1)
                {
                    host->rcvQnum = rcvAppDnp->userData[dataPos++];
                }
                else if(host->rcvIndex == 2)
                {
                    host->rcvQnum = rcvAppDnp->userData[dataPos+0]
                                    + ((rcvAppDnp->userData[dataPos+1] << 8) & 0xff00);
                    dataPos += 2;                                    
                }                
                host->rcvDataPos = dataPos;
                break;
                
           case 8:     /* Range : 16bit single-field quantity */             
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                
                if(host->rcvIndex == 1)
                {
                    host->rcvQnum = rcvAppDnp->userData[dataPos++];
                }
                else if(host->rcvIndex == 2)
                {
                    host->rcvQnum = rcvAppDnp->userData[dataPos+0]
                                    + ((rcvAppDnp->userData[dataPos+1] << 8) & 0xff00);
                    dataPos += 2;                
                }
                host->rcvDataPos = dataPos;
                break;
            default:
                break;
        }/*switch */

        if(opr->hexDebug == hostid)
        Debug(console,"hrcv%2d> -----> OPERATE[%02x] Func=%2d, OBJ=%2d, VAR=%2d, Index=%2d, QCode=%2d...Start=%2d, Stop=%2d, DataPos=%2d\n", 
            hostid+1, host->rcvACseq, host->rcvACfunc, host->rcvObj, host->rcvVar, host->rcvIndex, host->rcvQcode, 
            host->rcvStart, host->rcvStop, host->rcvDataPos);

        switch(host->rcvObj)
        {        
        case OBJ_CONTROL_RELAY:         // Control Relay Output
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> Operate... CONTROL_RELAY  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            
            if(host->rcvVar == 1)
            {
                for(i=0; i< host->rcvQnum; i++)
                {
                    /* ---------------------------------------- */
                    /*  DNP 패킷상에서 제어정보 추출...         */
                    /* ---------------------------------------- */    
                    if(host->rcvQcode == 7)
                    {
                        point = rcvAppDnp->userData[dataPos++];     // select point
                    }
                    else if(host->rcvQcode == 8)
                    {
                        point = rcvAppDnp->userData[dataPos+0] + ((rcvAppDnp->userData[dataPos+1] << 8) & 0xff00);     
                        dataPos += 2;                                 
                    }
                    
                    code    = rcvAppDnp->userData[dataPos++];     // control code
                    count   = rcvAppDnp->userData[dataPos++];     // control count
                    
                    ontime  = rcvAppDnp->userData[dataPos+0] 
                              + ((rcvAppDnp->userData[dataPos+1] << 8) & 0x0000ff00) 
                              + ((rcvAppDnp->userData[dataPos+2] << 16) & 0x00ff0000)
                              + ((rcvAppDnp->userData[dataPos+3] << 24) & 0xff000000);
                    dataPos += 4;
                                                  
                    offtime = rcvAppDnp->userData[dataPos+0] 
                              + ((rcvAppDnp->userData[dataPos+1] << 8) & 0x0000ff00) 
                              + ((rcvAppDnp->userData[dataPos+2] << 16) & 0x00ff0000)
                              + ((rcvAppDnp->userData[dataPos+3] << 24) & 0xff000000);
                    dataPos += 4;
                              
                    status  = rcvAppDnp->userData[dataPos++];
                    
                    /* ---------------------------------------- */
                    /*  상위 HOST 제어명령(ARM) Assign ...      */  
                    /* ---------------------------------------- */                      
                    hostCntr = (CONTROL_INFO *) &host->controlInfo[point];
                    cntrDevice = hostCntr->devNo;           // 계전기 번호, 1,2,..64
                    cntrPoint  = hostCntr->devPt;           // 계전기-포인트 번호, 1,2..64
                    cntrType   = hostCntr->type;            // 제어타입 : 포인트 제어 / 시스템 제어
                    
                    cntrTCF    = code & 0xc0;               // TRIP[0x80]/CLOSE[0x40] 제어정보

					//printf(">>> DNP-HOST Control... index=%d, type=%d ... dev=%d, point=%d TCF=%02x \n", point, cntrType, cntrDevice, cntrPoint,cntrTCF );
					
                    /* -------------------------------- */
                    /* 이중화 시스템 절체명령시...      */
                    /* -------------------------------- */
                    if(cntrType == CONTROL_SYSTEM)
                    {
                        if(linkCfg->online == SET)
                        {     
                            if(cntrTCF == 0x80)                 // TRIP 제어 , CPU-A 동작
                            {
                                if((opr->cpuMode == MPU_B) && (opr->runMode == LOCAL_MASTER))
                                {
                                    opr->cpuChange = SET;        
                                    
                                    logEvent_MPU(shmPtr, ENT_CHANGE_CPU, CPU_CHG_DNP, hostid+1, 0, hostid+1, NULL);      
		                            Debug(console,"hrcv%2d> **** Change CPU  : CPU-B => [CPU-A] !\n", hostid+1);   
		                            
		                            /* -------------------------------- */
							        /* LOG File 저장                    */
							        /* -------------------------------- */
							        sprintf(buffer, "hrcv%2d> **** Control: Change CPU  : CPU-B => [CPU-A] !", hostid+1);   
							      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   	
                                }       
                            }    
                            else if(cntrTCF == 0x40)            // CLOSE제어 , CPU-B 동작    
                            {
                                if((opr->cpuMode == MPU_A) && (opr->runMode == LOCAL_MASTER))
                                {
                                    opr->cpuChange = SET; 
                                    
                                    logEvent_MPU(shmPtr, ENT_CHANGE_CPU, CPU_CHG_DNP, hostid+1, 0, hostid+1, NULL);      
	    	                        Debug(console,"hrcv%2d> **** Change CPU  : CPU-A => [CPU-B] !\n", hostid+1);   
	    	                        
	    	                        /* -------------------------------- */
							        /* LOG File 저장                    */
							        /* -------------------------------- */
							        sprintf(buffer, "hrcv%2d> **** Control: Change CPU  : CPU-A => [CPU-B] !", hostid+1);   
							      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   	
                                }  
                            }
                               
                            if(opr->soeDebug)
                            Debug(console,"hrcv%2d> SYSTEM-OPERATE Info : Point=%d, code=%2x, count=%d, ontime=%d, offtime=%d, status=%2x\n",
                                hostid+1, point+1, code, count, ontime, offtime, status);
                        }
                        else
                        {
                            if(opr->soeDebug)
                            Debug(console,"hrcv%2d> *** Control: CPU Change... Failed : LINK Fail \n", hostid+1);
                            
                            /* -------------------------------- */
					        /* LOG File 저장                    */
					        /* -------------------------------- */
					        sprintf(buffer, "hrcv%2d> *** Control: CPU Change... Failed : LINK Fail ", hostid+1);
					      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));   	
                        }
                             
                        /* ------------------------------------ */
                        /* Operate 응답...                      */
                        /* ------------------------------------ */
                        memcpy( &sndAppDnp->userData[sndPos], &rcvAppDnp->userData[2], rcvCount - 3);
                        sndPos += rcvCount - 3;
                
                        sndAppDnp->userData[sndPos++] = 0;          // status    
                        mode = 1;   // 제어성공시                                 
                        break;
                    }
                    else
                    {        
                        opr->rcvONtime  = ontime;       // 수신한 제어시간 정보
                        opr->rcvOFFtime = offtime;
                        
                        /* 제어대상 계전기 정보... */
                        dev = (SDP_DEVICE *) deviceCFG[cntrDevice - 1];
                        
                        if(cntrTCF == 0x80)         cntrInfo = 1;      // 제어상태정보 : [1] TRIP, [2]CLOSE
                        else if(cntrTCF == 0x40)    cntrInfo = 2;
                        else                        cntrInfo = 0;     
                                
                        /* ------------------------------------ */
                        /* ESIO 제어정보 연계...                */
                        /* ------------------------------------ */
                        gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
                        logEvent_MPU(shmPtr, ENT_HOST_CNTR, cntrDevice, cntrPoint, cntrInfo, hostid+1, &ctime);    
                        
                        if(opr->soeDebug)
                        Debug(console, "hrcv%d> rcv DNP-CNTR(point=%d) : Dev=%d, Point=%d, TCF[TRIP=0x80, CLOSE=0x40] =%2x...SCAN-ID=%d \n", 
                            hostid+1, point+1, cntrDevice, cntrPoint, cntrTCF, dev->scanPort);
        
                        /* -------------------------------- */
                        /* LOG File 저장                    */
                        /* -------------------------------- */
                        sprintf(buffer, "<-- rcv Control [Host=%d,PT=%d]...Dev=%d, PT=%d, TCF[TR=1, CL=2]= %2x...SCAN-ID=%d, Target=%s ", 
                            hostid+1, point+1, cntrDevice, cntrPoint, cntrInfo, dev->scanPort, scanModType[dev->targetID]);
                        LogFile_MPU (shmPtr, ENT_HOST_CNTR, buffer, strlen(buffer));           
                        
                        controlInfo_MPU( shmPtr, cntrDevice, cntrPoint, cntrInfo, PASS_HOST_CNTR, &ctime);
                                              
                        /* ------------------------------------ */
                        /* Operate 응답...                      */
                        /* ------------------------------------ */
                        memcpy( &sndAppDnp->userData[sndPos], &rcvAppDnp->userData[2], rcvCount - 3);
                        sndPos += rcvCount - 3;
                
                        sndAppDnp->userData[sndPos++] = 0;          // status    
                        mode = 1;   // 제어성공시                                 
                        break;
                    }
                } /* for */
            }
            break;
            
        default:
            break;
        }

        if(mode == 0)
        {
            /* ------------------------------------ */
            /* Operate 응답...제어포인트 mismatch   */
            /* ------------------------------------ */
            memcpy( &sndAppDnp->userData[sndPos], &rcvAppDnp->userData[2], rcvCount - 3);
            sndPos += rcvCount - 3;
            sndAppDnp->userData[sndPos++] = 2;          // status       
        }
        
    } /* while */

    return (sndPos);
            
}


/*
*
*/
int funcAPP_delayMeasure(int hostid, int chid)
{
    //int     i,j;
    //int     rcvCount;
    int     sndPos;
    //int     objectDelay, headerDelay;
    
    //DNP_APP_FRAME   *rcvAppDnp;
    DNP_APP_FRAME   *sndAppDnp;
    HOST_DCB *host;

    host = (HOST_DCB *) hostDCB[hostid];        
    
    //rcvAppDnp = (DNP_APP_FRAME *) &host->rcvAppFrame[chid];
    sndAppDnp = (DNP_APP_FRAME *) &host->sndAppFrame[chid];
    //rcvCount  = rcvAppDnp->rcvSize;

    if(opr->hexDebug == hostid)
    Debug(console,"hrcv%2d> -----> DELAY-M[%02x] Func=%2d, OBJ=%2d, VAR=%2d, Index=%2d, QCode=%2d...Start=%2d, Stop=%2d, DataPos=%2d\n", 
        hostid+1, host->rcvACseq, host->rcvACfunc, host->rcvObj, host->rcvVar, host->rcvIndex, host->rcvQcode, 
        host->rcvStart, host->rcvStop, host->rcvDataPos);

    /* -------------------------------------------------------- */
    /* Make APPLICATION Message FORMAT :                        */
    /* -------------------------------------------------------- */        
    check_DNP_IIN(hostid, chid);
    
    sndPos = 0;
    sndAppDnp->userData[0] = 0xc0 + host->rcvACseq;     /* AC */
    sndAppDnp->userData[1] = APP_RCV_RESPONSE;          /* Function */
    sndAppDnp->userData[2] = host->rtuIIN[0];           // IIN
    sndAppDnp->userData[3] = host->rtuIIN[1];           // IIN
    sndPos = 4;
                                                            
    //headerDelay = 500;
    //objectDelay = 250;
    
    sndAppDnp->userData[sndPos++] = 0x34;
    sndAppDnp->userData[sndPos++] = 0x02;
    sndAppDnp->userData[sndPos++] = 0x07;
    sndAppDnp->userData[sndPos++] = 0x01;    
    sndAppDnp->userData[sndPos++] = 0x02;
    sndAppDnp->userData[sndPos++] = 0x00;        
            
    return (sndPos);
            
}


/*
*
*/
int funcAPP_UNSOLICIT_Check(int hostid, int chid)
{
    //int     i,j;
    int     rcvCount;
    int     dataPos, sndPos;
    //int     objectDelay, headerDelay;
    
    DNP_APP_FRAME   *rcvAppDnp;
    DNP_APP_FRAME   *sndAppDnp;
    HOST_DCB *host;

    host = (HOST_DCB *) hostDCB[hostid];        
    
    //rcvAppDnp = (DNP_APP_FRAME *) &rcvAppFrame[hostid];
    //sndAppDnp = (DNP_APP_FRAME *) &sndAppFrame[hostid];
    rcvAppDnp = (DNP_APP_FRAME *) &host->rcvAppFrame[chid];
    sndAppDnp = (DNP_APP_FRAME *) &host->sndAppFrame[chid];
    rcvCount  = rcvAppDnp->rcvSize;

    dataPos = 2;  /* execpt AC & FC area */             
    
    /* -------------------------------------------------------- */
    /* Make APPLICATION Message FORMAT :                        */
    /* -------------------------------------------------------- */
    check_DNP_IIN(hostid, chid);
        
    sndPos = 0;
    sndAppDnp->userData[0] = 0xc0 + host->rcvACseq;             /* AC */
    sndAppDnp->userData[1] = APP_RCV_RESPONSE;                  /* Function */
    sndAppDnp->userData[2] = host->rtuIIN[0];                   // IIN
    //sndAppDnp->userData[3] = host->rtuIIN[1] + BIT_FUNC_FAIL;   // IIN : Function 을 지원하지 않음
    sndAppDnp->userData[3] = host->rtuIIN[1];                   // IIN : Function 을 지원하지 않음
    sndPos = 4;
        
    /* check Multi-Obj */
    while(dataPos < rcvCount)
    {
        host->rcvObj    = rcvAppDnp->userData[dataPos++];
        host->rcvVar    = rcvAppDnp->userData[dataPos++];
        host->rcvQcode  = (rcvAppDnp->userData[dataPos]) & 0x0f;
        host->rcvIndex  = (rcvAppDnp->userData[dataPos] >> 4) & 0x07;
        dataPos++;

        switch(host->rcvQcode)         /* data size check ... */
        {
            case 0:     /* Range: 8bit Start & Stop */
            case 3:     /* Range: 8bit Abs Start & Stop */    
                host->rcvStart   = rcvAppDnp->userData[dataPos++];
                host->rcvStop    = rcvAppDnp->userData[dataPos++];
                host->rcvDataPos = dataPos;
                break;
        
            case 1:     /* Range: 16bit Start & Stop */
            case 4:     /* Range: 16bit Abs Start & Stop */    
                host->rcvStart  = rcvAppDnp->userData[dataPos+0] 
                                  + ((rcvAppDnp->userData[dataPos+1] << 8) & 0xff00);
                host->rcvStop   = rcvAppDnp->userData[dataPos+2] 
                                  + ((rcvAppDnp->userData[dataPos+3] << 8) & 0xff00);
                dataPos += 4;                                  
                host->rcvDataPos = dataPos;    
                break;
        
            case 2:     /* Range: 32bit Start & Stop */
            case 5:     /* Range: 32bit Abs Start & Stop */    
                host->rcvStart   = rcvAppDnp->userData[dataPos+0] 
                                  + ((rcvAppDnp->userData[dataPos+1] << 8) & 0x0000ff00) 
                                  + ((rcvAppDnp->userData[dataPos+2] << 16) & 0x00ff0000)
                                  + ((rcvAppDnp->userData[dataPos+3] << 24) & 0xff000000);

                host->rcvStop    = rcvAppDnp->userData[dataPos+4] 
                                  + ((rcvAppDnp->userData[dataPos+5] << 8) & 0x0000ff00) 
                                  + ((rcvAppDnp->userData[dataPos+6] << 16) & 0x00ff0000)
                                  + ((rcvAppDnp->userData[dataPos+7] << 24) & 0xff000000);
                dataPos += 8; 
                host->rcvDataPos = dataPos;                                                       
                break;
        
            case 6:     /* All Object : No Range */
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                host->rcvDataPos = dataPos;
                break;

            case 7:     /* Range : data quantity */             
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                host->rcvQnum    = rcvAppDnp->userData[dataPos++];   /* quentity number */
                host->rcvDataPos = dataPos;
                
                //if(opr->hexDebug == hostid)
                //Debug(console,"hrcv%2d> Single Field Quqntity...Qualifier=%2d, Qnum=%2d\n", hostid+1, host->rcvQcode, host->rcvQnum);
                break;
    
            case 8:
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                host->rcvQnum    = rcvAppDnp->userData[dataPos+0] 
                                  + ((rcvAppDnp->userData[dataPos+1] << 8) & 0xff00);
                dataPos += 2;                                   
                host->rcvDataPos = dataPos;
                
                //if(opr->hexDebug == hostid)
                //    Debug(console,"hrcv%2d> Single Field Quqntity...Qualifier=%2d, Qnum=%2d\n", hostid+1, host->rcvQcode, host->rcvQnum);            
                break;
    
            case 9:
                host->rcvStart   = 0;
                host->rcvStop    = 0;
                host->rcvQnum    = rcvAppDnp->userData[dataPos+0] 
                                  + ((rcvAppDnp->userData[dataPos+1] << 8) & 0x0000ff00) 
                                  + ((rcvAppDnp->userData[dataPos+2] << 16) & 0x00ff0000)
                                  + ((rcvAppDnp->userData[dataPos+3] << 24) & 0xff000000);
                dataPos += 4;                                   
                host->rcvDataPos = dataPos;
                
                //if(opr->hexDebug == hostid)
                //    Debug(console,"hrcv%2d> Single Field Quqntity...Qualifier=%2d, Qnum=%2d\n", hostid+1, host->rcvQcode, host->rcvQnum);             
                break;                        
    
            case 11:    // Free Format Qualifier 
                if(opr->hexDebug == hostid)
                    Debug(console,"hrcv%2d> READ FREE Format Quqntity...Qualifier=%2d\n", hostid+1, host->rcvQcode); 
                break;        
    
            default:
                break;
        }/*switch */

        if(opr->hexDebug == hostid)
        Debug(console,"hrcv%2d> -----> UNSOLICIT Check   [%02x] Func=%2d, OBJ=%2d, VAR=%2d, Index=%2d, QCode=%2d...Start=%2d, Stop=%2d, DataPos=%2d\n", 
            hostid+1, host->rcvACseq, host->rcvACfunc, host->rcvObj, host->rcvVar, host->rcvIndex, host->rcvQcode, 
            host->rcvStart, host->rcvStop, host->rcvDataPos);

        switch(host->rcvObj)
        {        
        case OBJ_CLASS:                 // Class 
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> CLASS  VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvVar, host->rcvQcode);
            break;
            
        default:
            if(opr->hexDebug == hostid)
            Debug(console,"hrcv%2d> >>>>>> *Invalid OBJ=%02d VAR=%2d, Qualifier=%2d \n", hostid+1, host->rcvObj, host->rcvVar, host->rcvQcode);
            break;
        }

    } /* while */
    
    return (sndPos);
                
}



/***************************************************
    FUNCTION : checkAPPFrame_HOST()
*   include TH,AC
***************************************************/
int     checkAPPFrame_HOST(int hostid, int chid, int socketID )
{
    //int     rcvCount;
    int     sndCount;
    //int     dataPos;
    byte    acHeader, requestCON;
    //int     object,variation,qualifier;
    //int     indexSize, qcode;
        
    //DL_FRAME        *rcvData;
    DNP_APP_FRAME   *rcvAppDnp;
    DNP_APP_FRAME   *sndAppDnp;
    //DNP_DEVICE      *dnp;
    
    DNP_SOE_QUEUE   *soeQ;
    DNP_COS_QUEUE   *cosQ;
    //DNP_COA_QUEUE   *coaQ;
    HOST_DCB        *host;

    host = (HOST_DCB *) hostDCB[hostid];
    
    sndCount = 0;
    
    rcvAppDnp = (DNP_APP_FRAME *) &host->rcvAppFrame[chid];
    sndAppDnp = (DNP_APP_FRAME *) &host->sndAppFrame[chid];
    
    //rcvCount  = rcvAppDnp->rcvSize;

    if(opr->hexDebug == hostid)
    {
        Debug(console,"hrcv%2d> -------------------------------------\n", hostid+1);
        DumpBuff(console,"RCVAPP: ", &rcvAppDnp->userData[0], rcvAppDnp->rcvSize);
    }
    
    /* ------------------------------------ */
    /* check AC Header                      */
    /* ------------------------------------ */
    acHeader    = rcvAppDnp->userData[0];
    requestCON  = acHeader & CONFIRM_BIT;
    host->rcvAC = acHeader;
    host->rcvACseq = acHeader & 0x1f;
    
    /* ------------------------------------ */
    /* APP Confirm 요구시...Confirm Send    */
    /* ------------------------------------ */
    if(requestCON)    sndACConfirm( hostid, chid, socketID);
    
    host->rcvACfunc = rcvAppDnp->userData[1];
    //object      = rcvAppDnp->userData[2];
    //variation   = rcvAppDnp->userData[3];
    //qualifier   = rcvAppDnp->userData[4];
    //qcode = qualifier & 0x0f;
    //indexSize = (qualifier >> 4) & 0x07;
    
    switch(host->rcvACfunc)
    {
    case APP_RCV_CONFIRM:       /* Response Function : */ 
        if(opr->soeDebug)  
        Debug(console,"hrcv%2d> -----> Rcv AC-CONFIRM ..[seq rcv=%02x, snd=%02x] Event Report =[soe=%d/cos=%d]\n", hostid+1, host->rcvACseq, host->sndACseq,
            host->soeReport, host->cosReport);
        
        /* ------------------------ */
        /*  COS/SOE Report Clear...   */
        /* ------------------------ */        
        if(host->soeReport)
        {
            soeQ = (DNP_SOE_QUEUE *) &host->dnpSOEQ;
            soeQ->rear = host->soeRear;
            host->soeReport = 0;
            
            host->sndACseq = (host->sndACseq + 1) & 0x1f;
        }            
        
        if(host->cosReport)
        {
            cosQ = (DNP_COS_QUEUE *) &host->dnpCOSQ;
            cosQ->rear = host->cosRear;
            host->cosReport = 0;
        }                    

#if 0
        if(host->coaReport)
        {
            coaQ = (DNP_COA_QUEUE *) &host->dnpCOAQ;
            coaQ->rear = host->coaRear;
            host->coaReport = 0;
        }                    
#endif

        break;          
        
    case APP_RCV_RESPONSE:      
        break;   
        
    case APP_RCV_UNSOLICIT:     
        break;       
        
    case APP_READ:              
        sndCount = funcAPP_read(hostid, chid);
        break;   
        
    case APP_WRITE:             
        sndCount = funcAPP_write(hostid, chid);
        break;   
        
    case APP_SELECT:            
        sndCount = funcAPP_select(hostid, chid);
        break;  
        
    case APP_OPR:               
        sndCount = funcAPP_operate(hostid, chid);
        break;  
        
    case APP_DIRECT:            /* Control  Function : */
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> DIRECT ..[seq=%2x] !\n", hostid+1, host->rcvACseq);        
        break;  
    case APP_DIRECT_NOACK:      /* Control  Function : */
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> DIRECT-NOACK ..[seq=%2x] !\n", hostid+1, host->rcvACseq);        
        break;  
    case APP_FREEZE:            /* Freeze   Function : */
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> FREEZE ..[seq=%2x] !\n", hostid+1, host->rcvACseq);        
        break;  
    case APP_FREEZE_NOACK:      /* Freeze   Function : */
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> FREEZE-NOACK ..[seq=%2x] !\n", hostid+1, host->rcvACseq);        
        break;  
    case APP_FREEZE_CLEAR:      /* Freeze   Function : */ 
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> FREEZE-CLEAR ..[seq=%2x] !\n", hostid+1, host->rcvACseq);        
        break;  
    case APP_FCLEAR_NOACK:      /* Freeze   Function : */
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> FC-NOACK ..[seq=%2x] !\n", hostid+1, host->rcvACseq);        
        break;  
    case APP_FREEZE_TIME:       /* Freeze   Function : */ 
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> FREEZE-TIME ..[seq=%2x] !\n", hostid+1, host->rcvACseq);        
        break;  
    case APP_FTIME_NOACK:       /* Freeze   Function : */
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> FT-NOACK ..[seq=%2x] !\n", hostid+1, host->rcvACseq);        
        break;  
    case APP_COLD_RESTART:      /* APP Control Function : */
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> COLD-Restart ..[seq=%2x] !\n", hostid+1, host->rcvACseq);         
        break;  
    case APP_WARM_RESTART:      /* APP Control Function : */ 
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> WARM Restart ..[seq=%2x] !\n", hostid+1, host->rcvACseq);        
        break;  
    case APP_INIT_DATA:         /* APP Control Function : */ 
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> INIT-DATA ..[seq=%2x] !\n", hostid+1, host->rcvACseq);        
        break;  
    case APP_INIT_APP:          /* APP Control Function : */ 
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> INIT-APP ..[seq=%2x] !\n", hostid+1, host->rcvACseq);           
        break;  
    case APP_START_APP:         /* APP Control Function : */ 
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> START-APP ..[seq=%2x] !\n", hostid+1, host->rcvACseq);           
        break;  
    case APP_STOP_APP:          /* APP Control Function : */ 
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> STOP-APP ..[seq=%2x] !\n", hostid+1, host->rcvACseq);           
        break;  
    case APP_SAVE_CONFIG:       /* Configuration Function : */ 
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> SAVE-CONFIG ..[seq=%2x] !\n", hostid+1, host->rcvACseq);           
        break;  
        
    case APP_ENB_UNSOLICIT:     /* Configuration Function : */ 
        //if(opr->hexDebug == hostid)
        Debug(console, "-----------------------------------------------\n");  
        Debug(console,"hrcv%2d> -----> ENB-UNSOLICIT ..[seq=%2x] !\n", hostid+1, host->rcvACseq); 
        Debug(console, "-----------------------------------------------\n");            
        
        /* DB 에서 먼저 허용여부 Check... */
        if(host->unsolMode == SET)     host->unsolEvent    = SET;
            
        sndCount = funcAPP_UNSOLICIT_Check(hostid, chid);
        break;  
        
    case APP_DISB_UNSOLICIT:    /* Configuration Function : */ 
        //if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> *DISB-UNSOLICIT ..[seq=%2x] !\n", hostid+1, host->rcvACseq);       
        
        host->unsolEvent    = 0;
        sndCount = funcAPP_UNSOLICIT_Check(hostid, chid);
        break;  
        
    case APP_ASSIGN_CLASS:      /* Configuration Function : */ 
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> ASSIGN-CLASS ..[seq=%2x] !\n", hostid+1, host->rcvACseq);       
        break;  
        
    case APP_DELAY_MEASURE: 
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> DELAY-MEASURE ..[seq=%2x] !\n", hostid+1, host->rcvACseq);       
        sndCount = funcAPP_delayMeasure(hostid, chid);
        break;  
    
    default:
        if(opr->hexDebug == hostid)  
        Debug(console,"hrcv%2d> -----> **** Invalid APP-Function [seq=%2x] \n", hostid+1, host->rcvACfunc);
        break;
    }

    if(sndCount)
    {
        if(opr->hexDebug == hostid)
        DumpBuff(console,"SND: ", &sndAppDnp->userData[0], sndCount);
        
        sndAppDnp->sndCount = sndCount;
        sendAppFrame_HOST( hostid, chid, socketID, sndAppDnp);
    }
    
    return (0);        
}



/***************************************************
    FUNCTION : checkDataLinkHOST()
*   include TH,AC
***************************************************/
int     checkDataLinkHOST( int hostid, int chid, int socketID, DNP_DATA_LINK *dnpData)
{
    //int     appControl;
    int     rcvCount;
    int     rcvPos;
    byte    thHeader, seqno;
    byte    *srcPtr;
    //byte    *buf;
    DL_FRAME        *rcvData;
    DNP_APP_FRAME   *rcvAppDnp;
    HOST_DCB        *host;

    
    host = (HOST_DCB *) hostDCB[hostid];
    
    rcvData = (DL_FRAME *) &dnpData->rcvFrame;
    rcvAppDnp = (DNP_APP_FRAME *) &host->rcvAppFrame[chid];
    //buf = (byte *) &dnpData->rcvFrame;
    
    /* ---------------------------- */
    /* check Transport Layer  ...   */
    /* ---------------------------- */
    thHeader = rcvData->userData[0];
    seqno    = thHeader & 0x3f;
    host->rcvTHseq = seqno;
    
    //DumpBuff(console,"RCV: ", dnpData->rcvFrame, 20);
    //DumpBuff(console,"RCV: ", rcvData, 20);
    //Debug(console,"thHeader = %2x\n", thHeader);
    
    /* check Transport Header - FIR set ... */
    if(thHeader & TH_FIR_BIT)
    {
        //if(opr->hexDebug == hostid)   
        //Debug(console,"hrcv%2d> [FIR set] ... seqno=%2x\n", hostid+1, seqno);
        rcvAppDnp->rcvPos   = 0;
        rcvAppDnp->rcvSize  = 0;
        
        host->rcvEndOk = RESET;
    }

    rcvPos  = rcvAppDnp->rcvPos;
    rcvCount= dnpData->rcvCount - 10 - 1;       /* except Data-Header(10), TH */
    srcPtr  = (byte *) &rcvData->userData[1];   /* exclude th-header */
    
    //if(opr->hexDebug == hostid)
    //Debug(console,"hrcv%2d> checkDataLink...TH=%2x, Seq=%2x... rcvCount=%d \n", hostid+1, thHeader,seqno, rcvCount );
    
    /* check App Buffer size: max 2048 */
    if((rcvPos + rcvCount) >= MAX_APP_BUFFER)
    {
        //if(opr->hexDebug == hostid)
        Debug(console,"hrcv%2d> *** APPF overflow .... %d  !\n", hostid+1,rcvPos+rcvCount);
        rcvAppDnp->rcvPos = rcvAppDnp->rcvPos - rcvCount;
        rcvPos  = rcvAppDnp->rcvPos - rcvCount;
    }

    /* store APPF - Data ... */
    memcpy(&rcvAppDnp->userData[rcvPos], srcPtr, rcvCount);
    rcvAppDnp->rcvPos = rcvPos + rcvCount;
    
    if(thHeader & TH_FIN_BIT)
    {
        rcvAppDnp->rcvSize = rcvAppDnp->rcvPos;

        //if(opr->hexDebug == hostid)
        //Debug(console,"hrcv%2d> [FIN set] ... seqno=%2x, rcvSize=%d\n", hostid+1, seqno, rcvAppDnp->rcvSize);

        return (1);

    }
    
    return (0);
}


/*
*   DNP-MESSAGE 
*/
int     chkRcvFrameHOST( int hostid, int chid, int socketID, DNP_DATA_LINK *dnpData)
{
    int     rcvFcb;
    //int     rcvFcv, rcvPrm, rcvDir;
    //int     chkFcv;
    byte    control, function;

    DL_FRAME        *rcvData;
    //DNP_APP_FRAME   *rcvAppDnp;
    DNP_APP_FRAME   *sndAppDnp;
    //DNP_DEVICE      *dnp;
    
    HOST_DCB        *host;
    DNP_SOE_QUEUE   *soeQ;  
    DNP_COS_QUEUE   *cosQ;  

    host = (HOST_DCB *) hostDCB[hostid];
    
    rcvData = (DL_FRAME *) &dnpData->rcvFrame;

    //if(opr->hostDebug == hostid)  
    //{
    //    DumpBuff(console,"check-frame>RCV: ", rcvData, dnpData->rcvCount);
    //}
    
    /* check Data-Link Control & Function code */
    control  = rcvData->control;
    function = control & 0x0f;
    //rcvDir   = control & 0x80;
    //rcvPrm   = control & 0x40;
    rcvFcb   = control & 0x20;
    //rcvFcv   = control & 0x10;

    //if(opr->hostDebug == hostid)
    //Debug(console,"hrcv%2d> control=%2x, DIR=%2x, PRM=%2x, FCB=%2x, FCV=%2x, funct = %d\n", 
    //    hostid+1, control, rcvDir, rcvPrm, rcvFcb, rcvFcv, function);

#if 0
    /* ---------------------------- */        
    /* check DATA-LINK : FCV/FCB    */
    /* ---------------------------- */  
    if(host->rcvLinkSts == SET)
    {
        if(rcvFcv)
        {
            if(rcvFcb != host->rcvNextFCB)      
            {
                if(opr->hexDebug == hostid)  
                Debug(console,"hrcv%2d> *check-frame *** Invalid FCV Bit ... !\n", hostid+1);
                //sendConfirmHOST( tp, host, chid, RCV_NACK);
                sendConfirmHOST( hostid, chid, socketID, RCV_NACK);
                return (100);
            }
        }
    }
#endif

    
    if(control & PRM_BIT)
    {
        switch(function)
        {
        case FUNC_RESET_LINK:       // Reset of Remote LINK

            Debug(console,"============================================\n");
            Debug(console,"hrcv%2d> >>>>>>> RESET LINK rcv ... !\n", hostid+1);
            Debug(console,"============================================\n");
            
            host->resetCount++;

            /* HOST-Confirm 전송 */            
            sendConfirmHOST( hostid, chid, socketID, RCV_ACK);
            
            host->rcvLinkSts = SET;             // LINK RESET 상태 
            host->rcvEndOk   = SET;
            host->rcvNextFCV = FCV_BIT;         // 다음명령의 FCB 예상
            host->rcvNextFCB = FCB_BIT;         // 다음명령의 FCB 예상
            
            host->sdpRestart  = RESET;
            
            /* -------------------------------- */
            /* HOST 별 SOE/COS Queue Clear...   */
            /* -------------------------------- */   
            host->sndTHseqno = 0;
            host->cosReport  = 0;
            host->cosRear    = 0;
            host->soeReport  = 0;
            host->soeRear    = 0;
                     
            soeQ = (DNP_SOE_QUEUE *) &host->dnpSOEQ;
            soeQ->front = 0;
            soeQ->rear  = 0;

            cosQ = (DNP_COS_QUEUE *) &host->dnpCOSQ;
            cosQ->front = 0;
            cosQ->rear  = 0;
            
            break;

        case FUNC_RESET_USER:       // Reset of User- Process
            sendConfirmHOST( hostid, chid, socketID, RCV_ACK);
            host->rcvNextFCB = (~rcvFcb) & FCB_BIT;         // 다음명령의 FCB 예상
            if(opr->hexDebug == hostid)  Debug(console,"hrcv%2d> RESET USER rcv ... !\n", hostid+1);
            break;

        case FUNC_TEST:             // TEST Function for link
            if(host->rcvLinkSts == RESET)  return (100);    // RESET 상태시 무응답 
            sendConfirmHOST( hostid, chid, socketID, RCV_ACK);
            host->rcvNextFCB = (~rcvFcb) & FCB_BIT;         // 다음명령의 FCB 예상
            host->rcvEndOk   = SET;
            if(opr->hexDebug == hostid)  Debug(console,"hrcv%2d> TEST rcv ... !\n", hostid +1);
            break;

        case FUNC_USER_DATA:        // USER data
            if(host->rcvLinkSts == RESET)  return (100);    // RESET 상태시 무응답 
            sendConfirmHOST( hostid, chid, socketID, RCV_ACK);
            host->rcvNextFCB = (~rcvFcb) & FCB_BIT;         // 다음명령의 FCB 예상
            
            //if(opr->hexDebug == hostid)  Debug(console,"hrcv%2d> USER DATA rcv ... !\n", hostid + 1);
            break;

        case FUNC_UNCONFIRM:        // Unconfirmed USER Data
            if(opr->hexDebug == hostid)  Debug(console,"hrcv%2d> UNCONFIRM rcv ... !\n", hostid);
            host->rcvNextFCB = (~rcvFcb) & FCB_BIT;         // 다음명령의 FCB 예상
            break;

        case FUNC_REQUEST:          // Request LINK Status
            if(opr->hexDebug == hostid)  Debug(console,"hrcv%2d> REQUEST rcv ... !\n", hostid+1);
            sendConfirmHOST( hostid, chid, socketID, RCV_RESPONSE);
            break;
        default:
            if(opr->hexDebug == hostid)  Debug(console,"hrcv%2d> default func ... %d!\n", hostid+1, function);
            function =100;
            break;
        }
    }
    else
    {
        switch(function)
        {
        case RCV_ACK:
            if(host->sndLinkSts == RESET)
            {
                //sndAppDnp = (DNP_APP_FRAME *) &sndAppFrame[hostid];
                sndAppDnp = (DNP_APP_FRAME *) &host->sndAppFrame[chid];
                sndAppDnp->sndSeqno = 1;

                host->sndLinkSts = SET;
                host->rcvEndOk   = SET;
                host->sndFcvBit = FCV_BIT;
                host->sndFcbBit = FCB_BIT;
                if(opr->hexDebug == hostid)  Debug(console,"rcv%2d> HOST-RESET-ACK rcv ... !\n", hostid+1);
            }
            else
            {
                if(opr->hexDebug == hostid)  Debug(console,"rcv%2d> ACK rcv ... !\n", hostid+1);
                host->rcvEndOk   = SET;
            }
            break;

        case RCV_NACK:
            if(opr->hexDebug == hostid)  Debug(console,"rcv%2d> NACK rcv ... !\n", hostid+1);
            host->rcvEndOk   = SET;
            break;

        case RCV_RESPONSE:
            if(opr->hexDebug == hostid)  Debug(console,"rcv%2d> RESPONSE rcv ... !\n", hostid+1);
            break;
        default:
            function =100;
            break;
        }
    }

    return (function);
}

/*.................. end of "hostDnpSUB.c ................... */


