/* ======================================================================================== */
/*  철도청 통합 CU - 상위 DNP 공통 Library                                                  */
/* ======================================================================================== */
/*  Designed : Sanesys.Co.Ltd, LEE HO-SANG                                                  */
/*  Updated  : 2009. 04. 10                                                                 */
/* ======================================================================================== */
/*      - int checkCUstatus(int hostid, int chid)                                           */
/*      - int sendConfirmHOST( int hostid, int chid, int code)                              */
/*      - int get_analog_input(int hostid,  int chid, byte *sndbuf, int variation)          */
/*      - int get_binary_input(int hostid, int chid, byte *sndbuf, int variation)           */
/*      - int get_binary_change(int hostid, int chid, byte *sndbuf, int variation)          */
/*      - int get_class_read(int hostid, int chid, byte *sndbuf, int variation)             */
/*      - int set_TimeDate(int hostid, int chid, int *dataPos)                              */
/* ======================================================================================== */

#include	"localLib.h"
#include    "external.h"

extern  int     hostChanWrite(int hostid, int chid, int socketID, byte *sndbuf, int sndCount);
        

extern  PDPDTIME_DATE_TIME      dnpRTC;
extern  PDPDTIME_MS_SINCE_70    dnpRTCInfo;

extern  byte    bitcode[8];

/*
* DNP 형식의 시각정보를 TimeVal 형식으로 변경
*/
void DnpTimeToTimeVal (PDPDTIME_DATE_TIME *pDnp, struct timeval *pTv)
{
    struct tm ts;
    time_t tt;

    if (pDnp->year)
    {
        ts.tm_year = pDnp->year - 1900;
        ts.tm_mon  = pDnp->month - 1;
        ts.tm_mday = pDnp->day;
        ts.tm_hour = pDnp->hour;
        ts.tm_min  = pDnp->minute;
        ts.tm_sec  = pDnp->second;
        ts.tm_isdst = 0;

        if((tt = mktime (&ts)) > 0)
        {
            pTv->tv_sec  = tt;
            pTv->tv_usec = pDnp->millisecond * 1000;
            return;
        }
    }

    pTv->tv_sec  = 0;
    pTv->tv_usec = 0;

}


/*
*/
void check_DNP_IIN(int hostid, int chid)
//void checkCUstatus(int hostid, int chid)
{
    byte    status[2];
    byte    front1, rear1;
    byte    front2, rear2;
    //byte    front3, rear3;
    
    //DNP_DEVICE      *dnp;
    HOST_DCB        *host;
    DNP_SOE_QUEUE   *soeQ;  
    DNP_COS_QUEUE   *cosQ;  
    //DNP_COA_QUEUE   *coaQ; 
    
    
    status[0] = 0;
    status[1] = 0;
    
    host = (HOST_DCB *) hostDCB[hostid];

    /* ------------------------------------ */
    /*  장치 상태정보 Update...             */
    /* ------------------------------------ */
    if(host->unsolEvent == 0)
    {      
        /* ------------------------------------ */
        /*  SOE Event : Class Assign...         */
        /* ------------------------------------ */    
        soeQ    = (DNP_SOE_QUEUE *) &host->dnpSOEQ;
        front1  = soeQ->front& 0xff;
        rear1   = soeQ->rear & 0xff;
        if(front1!= rear1)          
        {
            if(host->soeClass == 1)         status[0] |= BIT_CLASS1_REQ;
            else if(host->soeClass == 2)    status[0] |= BIT_CLASS2_REQ;
            else if(host->soeClass == 3)    status[0] |= BIT_CLASS3_REQ;
        }

        /* ------------------------------------ */
        /*  COS Event : Class Assign...         */
        /* ------------------------------------ */            
        cosQ    = (DNP_COS_QUEUE *) &host->dnpCOSQ;
        front2  = cosQ->front& 0xff;
        rear2   = cosQ->rear & 0xff;
        if(front2!= rear2)          
        {
            if(host->cosClass == 1)         status[0] |= BIT_CLASS1_REQ;
            else if(host->cosClass == 2)    status[0] |= BIT_CLASS2_REQ;
            else if(host->cosClass == 3)    status[0] |= BIT_CLASS3_REQ;
        }
    }
        
    /* ------------------------------------ */
    /*  2015.08.25 Update...                */
    /* 해당 HOST 가 예비HOST 인경우...      */
    /* 해당 채널에서 IIN Dump시...          */
    /* ------------------------------------ */
    if(host->hostActive == HOST_READY_STS)                          status[0] = 0;
    if((host->rcvObj == OBJ_DEVICE_IIN) && (host->rcvVar == 0x02))  status[0] = 0;
          
#if 0
    /* ------------------------------------ */
    /*  COA Event : Class Assign...         */
    /* ------------------------------------ */   
    coaQ    = (DNP_COA_QUEUE *) &host->dnpCOAQ;
    front3  = coaQ->front& 0xff;
    rear3   = coaQ->rear & 0xff;    
    if(front3!= rear3)          
    {
        if(host->coaClass == 1)         status[0] |= BIT_CLASS1_REQ;
        else if(host->coaClass == 2)    status[0] |= BIT_CLASS2_REQ;
        else if(host->coaClass == 3)    status[0] |= BIT_CLASS3_REQ;
    }
#endif
        
    if(host->sdpRestart == SET)  status[0] |= BIT_DEV_RESTART;
    
    host->rtuIIN[0] = status[0];
    host->rtuIIN[1] = status[1];
    
    //Debug(console,"===> check IIN : %2x %2x\n", host->rtuIIN[0], host->rtuIIN[1]);
    
}

/***************************************************
*   FUNCTION : sendConfirmHOST()
*   send DataLink Layer : make send frame & send
***************************************************/
void sendConfirmHOST( int hostid, int chid, int socketID, int code)
{
    //int     i,j,k;
    int     txcnt;
    byte    *txbuf;
    word    crc;

    DL_FRAME    *sndFrame;
    //DNP_DEVICE  *dnp;
    HOST_DCB    *host;
    //TTY_DESC    *tp;

    host = (HOST_DCB *) hostDCB[hostid];
    //tp   = (TTY_DESC *) host->ttyPort[chid];
    
    //sndFrame = (DL_FRAME *) &dataLinkFrame[hostid].sndFrame;
    sndFrame = (DL_FRAME *) &host->dataLinkFrame[chid].sndFrame;
    txbuf = (byte *) &sndFrame->start1;

    /* ------------------------ */
    /*  Make HEADER frame ...   */
    /* ------------------------ */
    txcnt = 0;

    txbuf[txcnt++] = 0x05;
    txbuf[txcnt++] = 0x64;
    txbuf[txcnt++] = 5;                             /* length */
    txbuf[txcnt++] = 0x00 + code;                   /* control */
    txbuf[txcnt++] = host->hostid & 0xff;           /* des Addr */
    txbuf[txcnt++] = (host->hostid >> 8) & 0xff;
    txbuf[txcnt++] = host->rtuAddr & 0xff;          /* src Addr */
    txbuf[txcnt++] = (host->rtuAddr >> 8) & 0xff;
    crc = B013_mkdnpcrc(txbuf, txcnt);

    txbuf[txcnt++] = crc & 0xff;
    txbuf[txcnt++] = (crc >> 8) & 0xff;

    if(opr->hostDebug == hostid)   DumpDNP_snd(console,"ACK:", txbuf, txcnt);
    
    //PortWrite( (TTY_DESC *) tp, (char *) txbuf, txcnt);
    hostChanWrite( hostid, chid, socketID, txbuf, txcnt);
    pause(100);
}


/*
*   FUNCTION : get_analog_change()
*   ANALOG INPUT : Analog Change => VAR 1, 2, 3, 4
*/
int get_analog_change(int hostid, int chid, byte *sndbuf, int variation)
{
#if 0
    int     i,j,point, index;
    int     startPt, stopPt;
    int     count = 0;
    byte    front, rear;
    //byte    coaCount;

    DNP_COA_QUEUE   *coaQ;
    DNP_COAQ_ENTRY  *entryCOA;
    HOST_DCB *host;
    
    host = (HOST_DCB *) hostDCB[hostid];

    coaQ = (DNP_COA_QUEUE *) &host->dnpCOAQ;
    front = coaQ->front & 0xff;
    rear  = coaQ->rear & 0xff;
    
    coaCount = (front - rear + 256) % 256;
    if(coaCount > 16)   coaCount = 16;

    //if(opr->soeDebug)
    //Debug(console,"hsnd%2d> <--- coaQ front=%d, rear = %d\n", hostid+1, front, rear);        
        
    if(coaCount > 0)
    {
        sndbuf[count++] = OBJ_ANALOG_CHANGE;        // OBJ code             
        sndbuf[count++] = variation;                // Variation #2
        sndbuf[count++] = 0x17;                     // QCode 7 : Single field Quantity, 포인트가 256 이하인 경우
        sndbuf[count++] = coaCount;                 // SOE Count Number
        
        for(i=0; i< coaCount; i++)
        {
            entryCOA = ( DNP_COAQ_ENTRY *) &coaQ->coaQueue[rear];

            if(variation == 1)          // 32-Bit Analog Change Event without TIME :
            {
            }
            else if(variation == 2)     // 16-Bit Analog Change Event without TIME :
            {
            }
            else if(variation == 3)     // 32-Bit Analog Change Event with TIME :
            {
                sndbuf[count++] = entryCOA->pointIndex;  
                sndbuf[count++] = entryCOA->pointFlag;
                
                sndbuf[count++] = entryCOA->pointData[0]; 
                sndbuf[count++] = entryCOA->pointData[1];
                sndbuf[count++] = entryCOA->pointData[2];
                sndbuf[count++] = entryCOA->pointData[3];
                sndbuf[count++] = entryCOA->dnpTime[0]; 
                sndbuf[count++] = entryCOA->dnpTime[1];
                sndbuf[count++] = entryCOA->dnpTime[2];
                sndbuf[count++] = entryCOA->dnpTime[3];
                sndbuf[count++] = entryCOA->dnpTime[4];
                sndbuf[count++] = entryCOA->dnpTime[5];
            }
            else if(variation == 4)     // 16-Bit Analog Change Event with TIME :
            {
                sndbuf[count++] = entryCOA->pointIndex;  
                sndbuf[count++] = entryCOA->pointFlag;
                
                sndbuf[count++] = entryCOA->pointData[0]; 
                sndbuf[count++] = entryCOA->pointData[1];
                sndbuf[count++] = entryCOA->dnpTime[0]; 
                sndbuf[count++] = entryCOA->dnpTime[1];
                sndbuf[count++] = entryCOA->dnpTime[2];
                sndbuf[count++] = entryCOA->dnpTime[3];
                sndbuf[count++] = entryCOA->dnpTime[4];
                sndbuf[count++] = entryCOA->dnpTime[5];                
            }
                
            rear = (rear + 1) & 0xff;
        }
            //cosQ->rear = rear & 0xff;
        host->coaReport = coaCount;
        host->coaRear   = rear;
    }

    return (count);

#endif    
    return (0);
}


/*
*   FUNCTION : get_analog_input()
*   ANALOG INPUT : 32Bit Analog Input => VAR 0, 1, 5
*/
int get_analog_input(int hostid,  int chid, byte *sndbuf, int variation)
{
    int     i;
    int     count = 0;
    int     startPt, stopPt;
    
    DNP_ANA_INPUT   *anaPoint;
    HOST_DCB *host;
    
    host = (HOST_DCB *) hostDCB[hostid];
    
    /* CU 내 상위 포인트가 정의되어 있지 않은 경우 ... return 0 */
    if(host->aiPtNum <= 0)  return (0);
    
    /* check RCV-Qcode ... */
    if(host->rcvQcode == 6)
    {
        startPt = 0;
        stopPt  = host->aiPtNum;
    }
    else 
    {
        startPt = host->rcvStart;
        stopPt  = host->rcvStop + 1;
    }        

    /* -------------------------------- */
    /*  데이터 Dump 요구시...Active     */
    /* -------------------------------- */
    host->hostActive = HOST_ACTIVE_STS;
    host->iinRcvTick = 0;
    
    /* check Variation ...: VAR 0,1,5 */            
    if((variation == 0) || (variation == 5))    // Default Analog Input
    {
        sndbuf[count++] = OBJ_ANALOG_INPUT;     // OBJ code        
        sndbuf[count++] = 5;                    // Variation #5
        
        if((host->rcvQcode == 0) || (host->rcvQcode == 6))
        {
            sndbuf[count++] = 0;                                // qualifier Code
            sndbuf[count++] = startPt;                          // Start point
            sndbuf[count++] = stopPt-1;                         // Stop  point
        }
        else if(host->rcvQcode == 1)
        {
            sndbuf[count++] = 1;                                // qualifier Code
            sndbuf[count++] = host->rcvStart & 0xff;            // Start point
            sndbuf[count++] = (host->rcvStart >> 8) & 0xff; 
            sndbuf[count++] = host->rcvStop & 0xff;             // Stop point
            sndbuf[count++] = (host->rcvStop >> 8) & 0xff;                  
        }
        else if(host->rcvQcode == 2)
        {
            sndbuf[count++] = 2;                                // qualifier Code
            sndbuf[count++] = host->rcvStart & 0xff;            // Start point
            sndbuf[count++] = (host->rcvStart >> 8) & 0xff;     
            sndbuf[count++] = (host->rcvStart >> 16) & 0xff;    
            sndbuf[count++] = (host->rcvStart >> 24) & 0xff;    
            sndbuf[count++] = host->rcvStop & 0xff;             // Stop point
            sndbuf[count++] = (host->rcvStop >> 8) & 0xff;         
            sndbuf[count++] = (host->rcvStop >> 16) & 0xff;        
            sndbuf[count++] = (host->rcvStop >> 24) & 0xff;                 
        }        
        
        /* BIT 상태에 따른 데이터 Assign */
        for(i=startPt; i< stopPt; i++)
        {
            anaPoint = (DNP_ANA_INPUT *) &host->ana_pointData[i];

            /* Float 형태 Report */
            sndbuf[count++] = anaPoint->flag;
            sndbuf[count++] = anaPoint->lowdata[0];
            sndbuf[count++] = anaPoint->lowdata[1];
            sndbuf[count++] = anaPoint->lowdata[2];
            sndbuf[count++] = anaPoint->lowdata[3];
        }
    }
    else if(variation == 1)  // 32-BIT ANALOG INPUT with FLAG
    {
        sndbuf[count++] = OBJ_ANALOG_INPUT;     // OBJ code        
        sndbuf[count++] = 1;                    // Variation #1
        
        if((host->rcvQcode == 0) || (host->rcvQcode == 6))
        {
            sndbuf[count++] = 0;                                // qualifier Code
            sndbuf[count++] = startPt;                          // Start point
            sndbuf[count++] = stopPt-1;                           // Stop  point
        }
        else if(host->rcvQcode == 1)
        {
            sndbuf[count++] = 1;                                // qualifier Code
            sndbuf[count++] = host->rcvStart & 0xff;            // Start point
            sndbuf[count++] = (host->rcvStart >> 8) & 0xff; 
            sndbuf[count++] = host->rcvStop & 0xff;             // Stop point
            sndbuf[count++] = (host->rcvStop >> 8) & 0xff;                  
        }
        else if(host->rcvQcode == 2)
        {
            sndbuf[count++] = 2;                                // qualifier Code
            sndbuf[count++] = host->rcvStart & 0xff;            // Start point
            sndbuf[count++] = (host->rcvStart >> 8) & 0xff;     
            sndbuf[count++] = (host->rcvStart >> 16) & 0xff;    
            sndbuf[count++] = (host->rcvStart >> 24) & 0xff;    
            sndbuf[count++] = host->rcvStop & 0xff;             // Stop point
            sndbuf[count++] = (host->rcvStop >> 8) & 0xff;         
            sndbuf[count++] = (host->rcvStop >> 16) & 0xff;        
            sndbuf[count++] = (host->rcvStop >> 24) & 0xff;                 
        }        
        
        /* BIT 상태에 따른 ANALOG 데이터 Assign */
        for(i=startPt; i< stopPt; i++)
        {
            anaPoint = (DNP_ANA_INPUT *) &host->ana_pointData[i];

            /* Float 형태 Report */
            sndbuf[count++] = anaPoint->flag;
            sndbuf[count++] = anaPoint->pointData & 0xff;
            sndbuf[count++] = (anaPoint->pointData >> 8) & 0xff;
            sndbuf[count++] = (anaPoint->pointData >> 16) & 0xff;
            sndbuf[count++] = (anaPoint->pointData >> 24) & 0xff;
        }        
    }
    else if(variation == 2)  // 16-BIT ANALOG INPUT with FLAG
    {
        sndbuf[count++] = OBJ_ANALOG_INPUT;     // OBJ code        
        sndbuf[count++] = 2;                    // Variation #2
        
        if((host->rcvQcode == 0) || (host->rcvQcode == 6))
        {
            sndbuf[count++] = 0;                                // qualifier Code
            sndbuf[count++] = startPt;                          // Start point
            sndbuf[count++] = stopPt-1;                           // Stop  point
        }
        else if(host->rcvQcode == 1)
        {
            sndbuf[count++] = 1;                                // qualifier Code
            sndbuf[count++] = host->rcvStart & 0xff;            // Start point
            sndbuf[count++] = (host->rcvStart >> 8) & 0xff; 
            sndbuf[count++] = host->rcvStop & 0xff;             // Stop point
            sndbuf[count++] = (host->rcvStop >> 8) & 0xff;                  
        }
        else if(host->rcvQcode == 2)
        {
            sndbuf[count++] = 2;                                // qualifier Code
            sndbuf[count++] = host->rcvStart & 0xff;            // Start point
            sndbuf[count++] = (host->rcvStart >> 8) & 0xff;     
            sndbuf[count++] = (host->rcvStart >> 16) & 0xff;    
            sndbuf[count++] = (host->rcvStart >> 24) & 0xff;    
            sndbuf[count++] = host->rcvStop & 0xff;             // Stop point
            sndbuf[count++] = (host->rcvStop >> 8) & 0xff;         
            sndbuf[count++] = (host->rcvStop >> 16) & 0xff;        
            sndbuf[count++] = (host->rcvStop >> 24) & 0xff;                 
        }        
        
        /* BIT 상태에 따른 ANALOG 데이터 Assign */
        for(i=startPt; i< stopPt; i++)
        {
            anaPoint = (DNP_ANA_INPUT *) &host->ana_pointData[i];

            /* Float 형태 Report */
            sndbuf[count++] = anaPoint->flag;
            sndbuf[count++] = anaPoint->pointData & 0xff;
            sndbuf[count++] = (anaPoint->pointData >> 8) & 0xff;
        }        
    }    
        
    return (count);
}

/*
*   FUNCTION : get_binary_input()
*   ANALOG INPUT : Binary Input => VAR 0, 1, 2
*/
int get_binary_input(int hostid, int chid, byte *sndbuf, int variation)
{
    int     i,point, index;
    int     startPt, stopPt, reportPt;
    int     count = 0;
    byte    buff;
    byte    tmpbuf[128];

    HOST_DCB *host;
    
    host = (HOST_DCB *) hostDCB[hostid];
        
    bzero8248( tmpbuf, 128);

    /* CU 내 상위 포인트가 정의되어 있지 않은 경우 ... return 0 */
    if(host->diPtNum <= 0)  return (0);

    /* check RCV-Qcode ... */
    if(host->rcvQcode == 6)
    {
        startPt = 0;
        stopPt  = host->diPtNum;
    }
    else 
    {
        startPt = host->rcvStart;
        stopPt  = host->rcvStop + 1;
    }        
    
    /* -------------------------------- */
    /*  데이터 Dump 요구시...Active     */
    /* -------------------------------- */
    host->hostActive = HOST_ACTIVE_STS;
    host->iinRcvTick = 0;
    
    /* -------------------------------- */        
    /* check Variation ...: VAR 0,1,5   */           
    /*  Single-Bit Binary Input         */ 
    /* -------------------------------- */      
    if((variation == 0) || (variation == 1))    // Default Binary Input
    {
        sndbuf[count++] = OBJ_BINARY_INPUT;     // OBJ code             
        sndbuf[count++] = 1;                    // Variation #1

        if((host->diIndexWord == SET) && (host->rcvQcode == 6))
        {
            reportPt = stopPt - 1;
            
            sndbuf[count++] = 1;                                // qualifier Code
            sndbuf[count++] = startPt & 0xff;            // Start point
            sndbuf[count++] = (startPt >> 8) & 0xff; 
            sndbuf[count++] = reportPt & 0xff;             // Stop point
            sndbuf[count++] = (reportPt >> 8) & 0xff;                      
        }
        else if((host->rcvQcode == 0) || (host->rcvQcode == 6))
        {
            sndbuf[count++] = 0;                                // qualifier Code
            sndbuf[count++] = startPt;                          // Start point
            sndbuf[count++] = stopPt-1;                         // Stop  point
        }
        else if(host->rcvQcode == 1)
        {
            sndbuf[count++] = 1;                                // qualifier Code
            sndbuf[count++] = host->rcvStart & 0xff;            // Start point
            sndbuf[count++] = (host->rcvStart >> 8) & 0xff; 
            sndbuf[count++] = host->rcvStop & 0xff;             // Stop point
            sndbuf[count++] = (host->rcvStop >> 8) & 0xff;                  
        }
        else if(host->rcvQcode == 2)
        {
            sndbuf[count++] = 2;                                // qualifier Code
            sndbuf[count++] = host->rcvStart & 0xff;            // Start point
            sndbuf[count++] = (host->rcvStart >> 8) & 0xff;     
            sndbuf[count++] = (host->rcvStart >> 16) & 0xff;    
            sndbuf[count++] = (host->rcvStart >> 24) & 0xff;    
            sndbuf[count++] = host->rcvStop & 0xff;             // Stop point
            sndbuf[count++] = (host->rcvStop >> 8) & 0xff;         
            sndbuf[count++] = (host->rcvStop >> 16) & 0xff;        
            sndbuf[count++] = (host->rcvStop >> 24) & 0xff;                 
        }        

        /* BIT 상태에 따른 데이터 Assign */
        for(i=startPt, index=0, point=0; i<= stopPt; i++)
        {
            point = i/8;
            index = i%8;
            
            buff = host->sts_pointData[i];
            if(buff & PT_FLAG_STS_ON)   tmpbuf[point] = tmpbuf[point] | bitcode[index];      
        }
        
        /* BIT 상태에 따른 데이터 Assign */
        if(point == 0) 
        {
            if(index != 0)  sndbuf[count++] = tmpbuf[point];
        }
        else 
        {
            for(i=0; i< point; i++) sndbuf[count++] = tmpbuf[i];
            if(index != 0)  sndbuf[count++] = tmpbuf[point];
            //if(index != 7)  sndbuf[count++] = tmpbuf[point];
        }
    }
    else if(variation == 2)  // Binary Input with Status
    {
        sndbuf[count++] = OBJ_BINARY_INPUT;     // OBJ code             
        sndbuf[count++] = 2;                    // Variation #1

        if((host->diIndexWord == SET) && (host->rcvQcode == 6))
        {
            reportPt = stopPt - 1;
            
            sndbuf[count++] = 1;                            // qualifier Code
            sndbuf[count++] = startPt & 0xff;               // Start point
            sndbuf[count++] = (startPt >> 8) & 0xff; 
            sndbuf[count++] = reportPt & 0xff;              // Stop point
            sndbuf[count++] = (reportPt >> 8) & 0xff;                      
        }
        else if((host->rcvQcode == 0) || (host->rcvQcode == 6))
        {
            sndbuf[count++] = 0;                                // qualifier Code
            sndbuf[count++] = startPt;                          // Start point
            sndbuf[count++] = stopPt-1;                         // Stop  point
        }
        else if(host->rcvQcode == 1)
        {
            sndbuf[count++] = 1;                                // qualifier Code
            sndbuf[count++] = host->rcvStart & 0xff;            // Start point
            sndbuf[count++] = (host->rcvStart >> 8) & 0xff; 
            sndbuf[count++] = host->rcvStop & 0xff;             // Stop point
            sndbuf[count++] = (host->rcvStop >> 8) & 0xff;                  
        }
        else if(host->rcvQcode == 2)
        {
            sndbuf[count++] = 2;                                // qualifier Code
            sndbuf[count++] = host->rcvStart & 0xff;            // Start point
            sndbuf[count++] = (host->rcvStart >> 8) & 0xff;     
            sndbuf[count++] = (host->rcvStart >> 16) & 0xff;    
            sndbuf[count++] = (host->rcvStart >> 24) & 0xff;    
            sndbuf[count++] = host->rcvStop & 0xff;             // Stop point
            sndbuf[count++] = (host->rcvStop >> 8) & 0xff;         
            sndbuf[count++] = (host->rcvStop >> 16) & 0xff;        
            sndbuf[count++] = (host->rcvStop >> 24) & 0xff;                 
        }      

        /* BIT 상태에 따른 데이터 Assign */
        for(i=startPt; i< stopPt; i++)
        {
            sndbuf[count++] = host->sts_pointData[i];
        }
    }
    
    return (count);
}


/*
*   FUNCTION : get_binary_change()
*   ANALOG INPUT : Binary Change => VAR 0, 1, 2
*/
int get_binary_change(int hostid, int chid, byte *sndbuf, int variation)
{
    int     i;
    //int     startPt, stopPt;
    int     count = 0;
    byte    front, rear;
    byte    soeCount;
    byte    cosCount;
    DNP_SOE_QUEUE   *soeQ;
    DNP_SOEQ_ENTRY  *entrySOE;
    DNP_COS_QUEUE   *cosQ;
    DNP_COSQ_ENTRY  *entryCOS;
    HOST_DCB *host;
    
    host = (HOST_DCB *) hostDCB[hostid];

    /* -------------------------------- */
    /*  데이터 Dump 요구시...Active     */
    /* -------------------------------- */
    host->hostActive = HOST_ACTIVE_STS;
    host->iinRcvTick = 0;
    
    if(variation == 1)          // Binary Input Change without TIME : COS 
    {
        cosQ = (DNP_COS_QUEUE *) &host->dnpCOSQ;
        front = cosQ->front & 0xff;
        rear  = cosQ->rear & 0xff;
    
        cosCount = (front - rear + 256) % 256;
        if(cosCount > 64)   cosCount = 64;
    
        //if(opr->soeDebug)
        //Debug(console,"hsnd%2d> <--- cosQ front=%d, rear = %d\n", hostid+1, front, rear);        
        
        if(cosCount > 0)
        {
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
                    entryCOS = ( DNP_COSQ_ENTRY *) &cosQ->cosQueue[rear];
                    sndbuf[count++] = entryCOS->cosBuf[0];
                    sndbuf[count++] = entryCOS->cosBuf[1];
                    rear = (rear + 1) & 0xff;
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
                    entryCOS = ( DNP_COSQ_ENTRY *) &cosQ->cosQueue[rear];
                    sndbuf[count++] = entryCOS->cosBuf[0];          // point 번호 : 하위 
                    sndbuf[count++] = entryCOS->cosBuf[1];          // point 번호 : 상위 
                    sndbuf[count++] = entryCOS->cosBuf[2];          // point state 
                                        
                    rear = (rear + 1) & 0xff;
                }                                
            }
            
            //cosQ->rear = rear & 0xff;
            host->cosReport = cosCount;
            host->cosRear   = rear;
        }
    }
    else if(variation == 2)     // Binary Input Change with TIME : SOE 
    {
        soeQ = (DNP_SOE_QUEUE *) &host->dnpSOEQ;
        front = soeQ->front & 0xff;
        rear  = soeQ->rear & 0xff;
    
        soeCount = (front - rear + 256) % 256;
        if(soeCount > 24)   soeCount = 24;
    
        //if(opr->soeDebug)
        //Debug(console,"hsnd%2d> <<<< soeQ front=%d, rear = %d\n", hostid, front, rear);        
        
        if(soeCount > 0)
        {
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
                    entrySOE = ( DNP_SOEQ_ENTRY *) &soeQ->soeQueue[rear];
                    sndbuf[count++] = entrySOE->soeBuf[0];
                    sndbuf[count++] = entrySOE->soeBuf[1];
                    sndbuf[count++] = entrySOE->soeBuf[2];
                    sndbuf[count++] = entrySOE->soeBuf[3];
                    sndbuf[count++] = entrySOE->soeBuf[4];
                    sndbuf[count++] = entrySOE->soeBuf[5];
                    sndbuf[count++] = entrySOE->soeBuf[6];
                    sndbuf[count++] = entrySOE->soeBuf[7];
            
                    rear = (rear + 1) & 0xff;
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
                    entrySOE = ( DNP_SOEQ_ENTRY *) &soeQ->soeQueue[rear];
                    sndbuf[count++] = entrySOE->soeBuf[0];          // point 번호 : 하위 
                    sndbuf[count++] = entrySOE->soeBuf[1];          // point 번호 : 상위 
                    sndbuf[count++] = entrySOE->soeBuf[2];          // point state 
                
                    sndbuf[count++] = entrySOE->soeBuf[3];          // SOE Time 
                    sndbuf[count++] = entrySOE->soeBuf[4];
                    sndbuf[count++] = entrySOE->soeBuf[5];
                    sndbuf[count++] = entrySOE->soeBuf[6];
                    sndbuf[count++] = entrySOE->soeBuf[7];
                    sndbuf[count++] = entrySOE->soeBuf[8];
            
                    rear = (rear + 1) & 0xff;
                }
            }
                        
            //soeQ->rear = rear & 0xff;
            host->soeReport = soeCount;
            host->soeRear = rear;
        }
        
    }    
    
    return (count);
}

/*
*   FUNCTION : get_class_read()
*   ANALOG INPUT : Binary Change => VAR 0, 1, 2
*/
int get_class_read(int hostid, int chid, byte *sndbuf, int variation)
{
    int     i;
    //int     startPt, stopPt;
    int     count = 0;
    byte    front1, rear1;
    byte    front2, rear2;
    //byte    front3, rear3;
    byte    soeCount;
    byte    cosCount;
    //byte    coaCount;
    
    DNP_SOE_QUEUE   *soeQ;
    DNP_SOEQ_ENTRY  *entrySOE;
    
    DNP_COS_QUEUE   *cosQ;
    DNP_COSQ_ENTRY  *entryCOS;
    
    //DNP_COA_QUEUE   *coaQ;
    //DNP_COAQ_ENTRY  *entryCOA;
    
    HOST_DCB *host;
    
    host = (HOST_DCB *) hostDCB[hostid];

    /* -------------------------------- */
    /*  데이터 Dump 요구시...Active     */
    /* -------------------------------- */
    host->hostActive = HOST_ACTIVE_STS;
    host->iinRcvTick = 0;
    
    
    /* Variation 2,3,4 => Class 1,2,3 */
    variation = variation - 1;
    
    /* -------------------------------- */
    /*  SOE Event Report :              */
    /* -------------------------------- */
    if(host->soeClass == variation)
    {
        soeQ = (DNP_SOE_QUEUE *) &host->dnpSOEQ;
        front1 = soeQ->front & 0xff;
        rear1  = soeQ->rear & 0xff;
    
        soeCount = (front1 - rear1 + 256) % 256;
        if(soeCount > 24)   soeCount = 24;
    
        //if(opr->soeDebug)
        //Debug(console,"hsnd%2d> *** CLASS: soeQ front=%d, rear = %d.... var=%d\n", hostid+1, front1, rear1, variation);        
        
        if(soeCount > 0)
        {
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
                    entrySOE = ( DNP_SOEQ_ENTRY *) &soeQ->soeQueue[rear1];
                    sndbuf[count++] = entrySOE->soeBuf[0];
                    sndbuf[count++] = entrySOE->soeBuf[1];
                    sndbuf[count++] = entrySOE->soeBuf[2];
                    sndbuf[count++] = entrySOE->soeBuf[3];
                    sndbuf[count++] = entrySOE->soeBuf[4];
                    sndbuf[count++] = entrySOE->soeBuf[5];
                    sndbuf[count++] = entrySOE->soeBuf[6];
                    sndbuf[count++] = entrySOE->soeBuf[7];
            
                    rear1 = (rear1 + 1) & 0xff;
                }
            }
            else
            {
                /* ---------------------------------------------------------------- */
                /* QCode 8 : 16bit Single field Quantity, 포인트가 256 이상인 경우  */
                /* ---------------------------------------------------------------- */    
                sndbuf[count++] = OBJ_BINARY_CHANGE;        // OBJ code             
                sndbuf[count++] = 2;                        // Variation #2
                sndbuf[count++] = 0x28;                     // QCode 8 : Single field Quantity, 포인트가 256 이하인 경우
                sndbuf[count++] = soeCount;                 // SOE Count Number
                sndbuf[count++] = 0;                        // SOE Count Number
        
                for(i=0; i< soeCount; i++)
                {
                    entrySOE = ( DNP_SOEQ_ENTRY *) &soeQ->soeQueue[rear1];
                    sndbuf[count++] = entrySOE->soeBuf[0];          // point 번호 : 하위 
                    sndbuf[count++] = entrySOE->soeBuf[1];          // point 번호 : 상위 
                    sndbuf[count++] = entrySOE->soeBuf[2];          // point state 
                
                    sndbuf[count++] = entrySOE->soeBuf[3];          // SOE Time 
                    sndbuf[count++] = entrySOE->soeBuf[4];
                    sndbuf[count++] = entrySOE->soeBuf[5];
                    sndbuf[count++] = entrySOE->soeBuf[6];
                    sndbuf[count++] = entrySOE->soeBuf[7];
                    sndbuf[count++] = entrySOE->soeBuf[8];
            
                    rear1 = (rear1 + 1) & 0xff;
                }
            }
                        
            //soeQ->rear = rear & 0xff;
            host->soeReport = soeCount;
            host->soeRear = rear1;

        }                
    }
    else if(host->cosClass == variation)
    {
        /* -------------------------------- */
        /*  COS Event Report :              */
        /* -------------------------------- */  
        cosQ = (DNP_COS_QUEUE *) &host->dnpCOSQ;
        front2 = cosQ->front & 0xff;
        rear2  = cosQ->rear & 0xff;
    
        cosCount = (front2 - rear2 + 256) % 256;
        if(cosCount > 64)   cosCount = 64;
    
        //if(opr->soeDebug)
        //Debug(console,"hsnd%2d> *** CLASS: cosQ front=%d, rear = %d ... var=%d\n", hostid+1, front2, rear2, variation);        
        
        if(cosCount > 0)
        {
            /* ---------------------------------------------------------------- */
            /* QCode 7 : 8bit Single field Quantity, 포인트가 256 이하인 경우   */
            /* ---------------------------------------------------------------- */    
            if(host->diIndexWord == RESET)
            {            
                sndbuf[count++] = OBJ_BINARY_CHANGE;        // OBJ code             
                sndbuf[count++] = 1;                        // Variation #2
                sndbuf[count++] = 0x17;                     // QCode 7 : Single field Quantity
                sndbuf[count++] = cosCount;                 // SOE Count Number
        
                for(i=0; i< cosCount; i++)
                {
                    entryCOS = ( DNP_COSQ_ENTRY *) &cosQ->cosQueue[rear2];
                    sndbuf[count++] = entryCOS->cosBuf[0];
                    sndbuf[count++] = entryCOS->cosBuf[1];
                    rear2 = (rear2 + 1) & 0xff;
                }
            }
            else
            {
                /* ---------------------------------------------------------------- */
                /* QCode 8 : 16bit Single field Quantity, 포인트가 256 이상인 경우  */
                /* ---------------------------------------------------------------- */    
                sndbuf[count++] = OBJ_BINARY_CHANGE;        // OBJ code             
                sndbuf[count++] = 1;                        // Variation #2
                sndbuf[count++] = 0x28;                     // QCode 8 : 16Bit Single field Quantity, 포인트가 256 이하인 경우
                sndbuf[count++] = cosCount;                 // SOE Count Number
                sndbuf[count++] = 0;                        // SOE Count Number
        
                for(i=0; i< cosCount; i++)
                {
                    entryCOS = ( DNP_COSQ_ENTRY *) &cosQ->cosQueue[rear2];
                    sndbuf[count++] = entryCOS->cosBuf[0];          // point 번호 : 하위 
                    sndbuf[count++] = entryCOS->cosBuf[1];          // point 번호 : 상위 
                    sndbuf[count++] = entryCOS->cosBuf[2];          // point state 
                                        
                    rear2 = (rear2 + 1) & 0xff;
                }                                                
            }
            
            //cosQ->rear = rear & 0xff;
            host->cosReport = cosCount;
            host->cosRear   = rear2;
        }        
    }

    return (count);
    
}


/*
*   FUNCTION : set_TimeDate()
*   ANALOG INPUT : Binary Input => VAR 0, 1, 2
*/
int set_TimeDate(int hostid, int chid, int *dataPos)
{
    //int     i,j,point, index;
    //int     startPt, stopPt;
    //int     rcvCount;
    int     count = 0;
    //byte    buff;

    DNP_APP_FRAME   *rcvAppDnp;
    //DNP_APP_FRAME   *sndAppDnp;
    HOST_DCB *host;
    char    buffer[256];

    host = (HOST_DCB *) hostDCB[hostid];        

    //rcvAppDnp = (DNP_APP_FRAME *) &rcvAppFrame[hostid];
    //sndAppDnp = (DNP_APP_FRAME *) &sndAppFrame[hostid];
    rcvAppDnp = (DNP_APP_FRAME *) &host->rcvAppFrame[chid];
    //sndAppDnp = (DNP_APP_FRAME *) &host->sndAppFrame[chid];
    //rcvCount  = rcvAppDnp->rcvSize;

	/* 2020.06.02 HOST TIME-SYNC 허용/금지 */
	if(host->timeSyncDISB == SET)
	{
		//if(opr->hexDebug == hostid)
        Debug(console,"\n------------------------------\n");
        Debug(console,"==> HOST(%d) set TIME : DISABLE ... RETURN \n", hostid + 1);
        Debug(console,"------------------------------\n");
		return (count);
	}
		
    /* ---------------------------------------- */
    /* 상위 호스트 ...TIME and DATE Setting ... */  
    /* ---------------------------------------- */      
    if((host->rcvObj == OBJ_TIME_AND_DATE) && (host->rcvVar == 1))   
    {
        //if(opr->hexDebug == hostid)
        //Debug(console,"\n------------------------------\n");
        //Debug(console,"==> HOST(%d) set TIME : \n", hostid + 1);
        //Debug(console,"------------------------------\n");
        
        *dataPos += 6;
        
        //memcpy( &dnpRTCInfo.mostSignificant, &rcvAppDnp->userData[host->rcvDataPos], 6);
        dnpRTCInfo.leastSignificant = rcvAppDnp->userData[host->rcvDataPos + 0]
                                    + ((rcvAppDnp->userData[host->rcvDataPos + 1] << 8) & 0xff00);
                                    
        dnpRTCInfo.mostSignificant = rcvAppDnp->userData[host->rcvDataPos + 2]
                                    + ((rcvAppDnp->userData[host->rcvDataPos + 3] << 8) & 0xff00)
                                    + ((rcvAppDnp->userData[host->rcvDataPos + 4] << 16) & 0xff0000)
                                    + ((rcvAppDnp->userData[host->rcvDataPos + 5] << 24) & 0xff000000);
                                    
        /* Convert String into RTC */
        makeDnpTimeRTC( &dnpRTC, &dnpRTCInfo);
        
        //if(opr->hexDebug == hostid)
        Debug(console,"hrcv%2d> =====> TIMESET: %4d/%2d/%2d %02d:%02d:%02d-%d\n", hostid+1,
            dnpRTC.year, dnpRTC.month, dnpRTC.day, dnpRTC.hour, dnpRTC.minute, dnpRTC.second, dnpRTC.millisecond);

        /* -------------------------------------------------------- */
        /* TIME-SYNC 관련 이벤트 생성                               */    	
        /* -------------------------------------------------------- */
        logEvent_MPU(shmPtr, ENT_HOST_TIMESET, hostid+1, 0, 0, hostid+1, NULL);  
        
#if 0
        sprintf(buffer, "hrcv%2d> =====> TIMESET: %4d/%2d/%2d %02d:%02d:%02d-%d", hostid+1,
            dnpRTC.year, dnpRTC.month, dnpRTC.day, dnpRTC.hour, dnpRTC.minute, dnpRTC.second, dnpRTC.millisecond);
        LogFile_MPU (shmPtr, ENT_HOST_CNTR, buffer, strlen(buffer));  
#endif

#if 0

        /* -------------------------------------------------------- */
        /* DNP-HOST TIME SYNC 시.....즉시 시스템 시각 Setting...    */
        /*  mili-SEC 까지 Time Sync...                              */
        /*  WDT() 에서 RTC 정보 Update...                           */
        /* -------------------------------------------------------- */
        DnpTimeToTimeVal( &dnpRTC, &opr->dnpTimeVal);      // DNP-HOST Timeval 정보
#endif
        
            
        /* -------------------------------- */
        /*  SEC 까지 Time Sync...           */
        /* -------------------------------- */
        opr->year   = dnpRTC.year;
        opr->month  = dnpRTC.month;
        opr->day    = dnpRTC.day;
        opr->week   = 0;
        opr->hour   = dnpRTC.hour;
        opr->min    = dnpRTC.minute;
        opr->sec    = dnpRTC.second;
        
        opr->rtcUpdateICCP = SET;		// ICCP-HOST Time-Sync 정보 
        opr->rtcUpdateFlag = SET;
       
    }
    
    return (count);
}

/*.................. end of "hostDnpLib.c ................... */

    