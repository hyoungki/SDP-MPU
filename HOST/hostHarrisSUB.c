
#include	"localLib.h"
#include    "external.h"

/*
*/
void controlResetHARRIS(int hostid)
{
    opr->armPoint       = 0;
    opr->armTCF         = 0;
    opr->cntStatus      = RESET;
    Debug(console,"harris%2d> * Control Reset ...!\n", hostid+1);
}

/*
*
*/
int controlArmHARRIS(HOST_DCB *host, int cntPoint)
{
    int     cntrDevice;
    //int     cntrPoint;
    int     port, point;
    
    CONTROL_INFO    *hostCntr;

    if(cntPoint >= MAX_DNP_DO_POINT)
    {
        Debug(console,"harris%02d> *ARM Fail : Invalid HOST Control Point ... %d \n", host->id, cntPoint);
        return (NOK);        
    }
    
    /* ---------------------------------------- */
    /*  상위 HOST 제어명령(ARM) Assign ...      */  
    /* ---------------------------------------- */                      
    hostCntr = (CONTROL_INFO *) &host->harrisCntr[cntPoint];
    cntrDevice = hostCntr->devNo;
    //cntrPoint  = hostCntr->devPt;
    
    port    = hostCntr->port;
    point   = hostCntr->point;

    if((cntrDevice < 0) || (cntrDevice >= MAX_DEVICE))
    {
        Debug(console,"harris%02d> * ARM Fail : devid = %d, point=%d\n", host->id, cntrDevice);
        return (NOK);
    }

    if((port < 1) || (port > MAX_HARRIS_PORT) || (point < 1) || (point > MAX_HARRIS_POINT))
    {
        Debug(console,"harris%02d> * ARM Fail : port = %d, point=%d\n", host->id, port, point);
        return (NOK);
    }

    if(opr->soeDebug)
    Debug(console,"harris%02d> SELECT Info : Port=%d, Point=%d \n",  host->id, port, point );
                        
    opr->cntStatus    = ARM;
    
    return(OK);
                                
}


/*
*
*/
int controlOprHARRIS( HOST_DCB *host, int point, int state)
{
    int     i,j;
    int     cntrDevice, cntrPoint, cntrTCF;
    
    CONTROL_INFO    *hostCntr;
    SDP_DEVICE      *dev;
    SCAN_CONFIG     *scan;
    POINT_BUF       *ptBuf;
    
    /* ---------------------------------------- */
    /*  상위 HOST 제어명령(ARM) Assign ...      */  
    /* ---------------------------------------- */                      
    hostCntr = (CONTROL_INFO *) &host->harrisCntr[point];
    cntrDevice = hostCntr->devNo;
    cntrPoint  = hostCntr->devPt;
    if(state == 1)          cntrTCF = 0x80;
    else if(state == 2)     cntrTCF = 0x40;
    else
    {
        Debug(console,"harris%02d> *** Control Fail...!\n", host->id);
        return (0);
    }

    opr->cntStatus  = RESET;
    opr->armPoint   = 0;
    opr->armTCF     = 0;
    
    /* ---------------------------------------- */
    /* SCAN Task 제어정보 Setting ...           */
    /* ---------------------------------------- */
    for(i=0 ; i< MAX_SCAN_PORT; i++)
    {
        scan = (SCAN_CONFIG *) scanCFG[i];
        if(scan->scanIndex <= 0)   continue;
    
        for(j=0; j< scan->scanIndex; j++)
        {
            /* 제어대상 모듈을 search... */
            if(scan->scanDevice[j] == cntrDevice)
            {
                dev = (SDP_DEVICE *) deviceCFG[cntrDevice];
                ptBuf = (POINT_BUF *) &dev->doPtBuf[cntrPoint];
                
                dev->cntPoint = cntrPoint;
                dev->cntTCF   = cntrTCF;   /* TRIP */
                dev->cntrType = ptBuf->ptConfig;         // 제어속성 : Pulse, Latch 제어
                dev->selectReq= SET;
    
                //opr->cntrDevId[i]   = cntrDevice;
                //opr->cntrComTick[i] = 0;
                //opr->cntrFlag[i]    = SET;

                if(cntrTCF == 0x80)         
                {
                }
                else if(cntrTCF == 0x40)    
                {
                }
                
                if(opr->soeDebug)
                {	
                	Debug(console,"harris%02d> OPERATE Info : Port=%d, Point=%d, code=%2x \n",  host->id, hostCntr->port, hostCntr->point, cntrTCF );
					Debug(console,"Control... dev=%d, point=%d, tcf=%2x\n",  cntrDevice+1,  dev->cntPoint+1, dev->cntTCF);
				}
				
				return (OK);                  
                break;
            }                            
        }
    }

    return (NOK);
}


/*----------------------------------------------------------------------------
* Function Name : getCOSCount()
* 수행내용: 원격소장치내 COS 데이터 수 취득 및 처리 함수
* ArgList :
* Return  :  COS 데이터 수
---------------------------------------------------------------------------- */  
int getCOSCount(HOST_DCB *host)
{
    short   cosCount;
    int     rtuid;
    RTU     *rtu;    
    HARRIS_COS_Q   *cosq;

    rtuid = host->rtuIndex - 1;   
    rtu = (RTU *) rtubuf[rtuid]; 
    cosq = (HARRIS_COS_Q *) &rtu->cosq;

    cosCount = (cosq->front - cosq->rear + 256) % 256;

    if(cosCount >= 250)     cosq->report = 0x3f;  /* queue_overflow */
    else if(cosCount >= 32) cosq->report = 31;
    else                    cosq->report = cosCount;

    return(cosq->report);
}

/*----------------------------------------------------------------------------
* Function Name : getRTUStatus()
* 수행내용: 원격소장치내 SOE 유무 및 포트상태정보 취득 및 처리 함수
* ArgList :
* Return  :  원격소장치 상태정보
---------------------------------------------------------------------------- */  
int getRTUStatus(HOST_DCB *host)
{
    int i;
    int rtuid;
    int status = 0;
    RTU     *rtu;
    HARRIS_SOE_Q   *soeq;
    PORT_DB *port;

    rtuid = host->rtuIndex - 1;   
    rtu = (RTU *) rtubuf[rtuid]; 
    soeq = (HARRIS_SOE_Q *) &rtu->soeq;
    
    for(i = 0; i < MAX_HARRIS_PORT; i++)
    {
        port = (PORT_DB *) portdb[i];
        if(port->rtuid != host->rtuIndex) continue;
        status |= port->portStatus;
    }

    if(soeq->front) status |= SOE_S;
    rtu->status = status;
    return(status);
}


/*----------------------------------------------------------------------------
* Function Name : dat_dump()
* 수행내용: 상위로부터의 데이터 덤프에 대한 처리함수
* ArgList :
*   1. sizBuf - 데이터 덤프에 대한 포인트 수를 가리키는 Pointer 
* Return  :  처리결과 상태
---------------------------------------------------------------------------- */  
int dat_dump(byte *sizBuf, HOST_DCB  *host, byte *rxbuf, byte *txbuf)
{
    int     i;
    int     portid;
    //int     rtuid;
    short  *buf;
    PORT_DB *port;

    //rtuid = host->rtuid;    
    
    txbuf[1] = getCOSCount(host);       /* check cos queue_count */
    txbuf[2] = getRTUStatus(host);      /* rtu port_status */
    host->txcnt = 3;
    
    /* data read */
    for(portid = 0; portid < MAX_HARRIS_PORT; portid++)
    {
        port = (PORT_DB *) portdb[portid];
        
        if(port->rtuid != host->rtuIndex) continue;
        if((port->type != ANA) && (port->type != ACC)) continue;

        if(*sizBuf == 0)
        {
            sizBuf++;
            continue;
        }

        /*  Subway34:Host Data - dump mth   */
         buf = port->pointData;
         
        /* response data assign */
        for(i = 0; i < *sizBuf; i++)
        {
            txbuf[host->txcnt++] = (buf[i] >> 6) & 0x3f;  /* high_byte:D11-D6 */
            txbuf[host->txcnt++] = buf[i] & 0x3f;         /* low_byte: D5-D0 */
        }

        sizBuf++;

        /* port status */
        txbuf[host->txcnt++] = port->portStatus;

    } /* for */

#if 0
    memcpy(rtu->reportbuf, txbuf, txcnt);
    rtu->reportcnt = txcnt;
#endif

    return(OK);
}


/*----------------------------------------------------------------------------
* Function Name : cos_check()
* 수행내용: 상위로부터의 COS Check 대한 처리함수
* ArgList :
* Return  :  처리결과 상태
---------------------------------------------------------------------------- */  
int cos_check(HOST_DCB *host, byte *rxbuf, byte *txbuf)
{
    int     i;
    //int     rtuid;
    int     cicount = 0;
    PORT_DB *port;

    //rtuid = host->rtuid;    

    txbuf[1] = getCOSCount(host);   /* check cosqueue_count */
    txbuf[2] = getRTUStatus(host);  /* rtu port_status */

    for(i = 0; i < MAX_HARRIS_PORT; i++)
    {
        port = (PORT_DB *) portdb[i];
        
        if(port->rtuid != host->rtuIndex) continue;
        if(port->type == CAI) cicount++;
    }

    if(cicount == 0)	txbuf[2] |= 0x04;  /* if no C&I, MESSAGE FAIL */
    host->txcnt = 3;   

    return(OK);
}


/*----------------------------------------------------------------------------
* Function Name : cos_dump()
* 수행내용: 상위로부터의 COS Dump 대한 처리함수
* ArgList :
* Return  :  처리결과 상태
---------------------------------------------------------------------------- */  
int cos_dump(HOST_DCB *host, byte *rxbuf, byte *txbuf)
{
    int i;
    int rear;
    //int     rtuid;
    int     index;
    RTU     *rtu;     
    HARRIS_COS_Q  *cosq;

    index = host->rtuIndex - 1;
    //rtuid = host->rtuid;           
    rtu = (RTU *) rtubuf[index]; 

    cosq = (HARRIS_COS_Q *) &rtu->cosq;

    if(rtu->preOpcode != 0x04)
    {
        txbuf[0] = rtu->id;  /* remote address */
        host->txcnt = 1;
        
        if(cosq->report <= 0 )  return(NOK);

        if(rxbuf[2] > cosq->report)
        {
            rear = (cosq->front + 255) & 0xff;  /* fill LAST-COS */
            for(i = 0; i < rxbuf[2]; i++)
            {
                txbuf[host->txcnt++] = cosq->que[rear][0];
                txbuf[host->txcnt++] = cosq->que[rear][1];
            }
            txbuf[host->txcnt++] = getRTUStatus(host) | 0x04;    /* MESSAGE FAIL */
        }
        else if(rxbuf[2] < cosq->report)
        {
            for(i = 0; i < rxbuf[2]; i++)
            {
                txbuf[host->txcnt++] = cosq->que[cosq->rear][0];
                txbuf[host->txcnt++] = cosq->que[cosq->rear][1];
                cosq->rear = (cosq->rear + 1) & 0xff;
            }
            txbuf[host->txcnt++] = getRTUStatus(host) | 0x04;    /* MESSAGE FAIL */
        }
        else if(cosq->report > 0)
        {
            for(i = 0; i < cosq->report; i++)
            {
                txbuf[host->txcnt++] = cosq->que[cosq->rear][0];
                txbuf[host->txcnt++] = cosq->que[cosq->rear][1];
                cosq->rear = (cosq->rear + 1) & 0xff;
            }
            txbuf[host->txcnt++] = getRTUStatus(host);    /* rtu port_status */
        }
        memcpy(rtu->reportbuf, txbuf, host->txcnt);
        rtu->reportcnt = host->txcnt;
    }
    else
    {
        memcpy(txbuf, rtu->reportbuf, rtu->reportcnt);
        host->txcnt = rtu->reportcnt;
    }

    return(OK);
}


/*----------------------------------------------------------------------------
* Function Name : sts_dump()
* 수행내용: 상위로부터의 Status Dump에 대한 처리함수
* ArgList :
* Return  :  처리결과 상태
---------------------------------------------------------------------------- */ 
int sts_dump(HOST_DCB *host, byte *rxbuf, byte *txbuf)
{
    int  i;
    int  portid;
    //int  rtuid;
    int  index;
    
    short *buf;
    byte *sizBuf;
    HARRIS_COS_Q  *cosq;
    RTU     *rtu; 
    PORT_DB *port;    

    index = host->rtuIndex - 1;
    //rtuid = host->rtuid;  
    rtu = (RTU *) rtubuf[index]; 

    cosq = (HARRIS_COS_Q *) &rtu->cosq;
    bzero(&txbuf[0], 64);
    
    txbuf[0] = rtu->id;  /* remote address */
    host->txcnt = 1;
    
    sizBuf = &rxbuf[2];

    for(portid = 0; portid < MAX_HARRIS_PORT; portid++)
    {
        port = (PORT_DB *) portdb[portid];
        
        if(port->rtuid != host->rtuIndex) continue;
        if(port->type != CAI) continue;

        if(*sizBuf == 0)
        {
            sizBuf++;
            continue;
        }

        for(i = 0, buf = port->pointData; i <= *sizBuf; i += 6)
        {
            txbuf[host->txcnt] = 0;
            if(buf[i + 0]) txbuf[host->txcnt] |= 0x01;
            if(buf[i + 1]) txbuf[host->txcnt] |= 0x02;
            if(buf[i + 2]) txbuf[host->txcnt] |= 0x04;
            if(buf[i + 3]) txbuf[host->txcnt] |= 0x08;
            if(buf[i + 4]) txbuf[host->txcnt] |= 0x10;
            if(buf[i + 5]) txbuf[host->txcnt] |= 0x20;
            host->txcnt++;
        }

        if(*sizBuf == 0x20)          txbuf[host->txcnt-1] &= 0x03;
        else if(*sizBuf == 0x3f)     txbuf[host->txcnt-1] &= 0x07;

        sizBuf++;

        /* port status */
        txbuf[host->txcnt++] = port->portStatus;

    } /* for(port) */

    /* clear cos after S/D */
    cosq->rear = cosq->front = 0;
    cosq->report= 0;
    return(OK);
}

/*----------------------------------------------------------------------------
* Function Name : pt_arm()
* 수행내용: 상위로부터의 Control-ARM 에 대한 처리함수
* ArgList :
* Return  :  처리결과 상태
---------------------------------------------------------------------------- */  
int pt_arm(HOST_DCB *host, byte *rxbuf, byte *txbuf)
{
    int     i, mode;
    int     portid, point;
    int     portNum, ptNum;
    //int     rtuid; 
    
    PORT_DB     *port;  
    CONTROL_INFO    *hostCntr;

    //rtuid = host->rtuid;           
    
    txbuf[1] = rxbuf[2];
    txbuf[2] = rxbuf[3];
    host->txcnt = 3;

    opr->port       = rxbuf[2];    /* port */
    opr->pointState = rxbuf[3];    /* point & state */

    /* ----------------------------------------- */
    /*  Multi RTU 의 경우 : 해당 Port Offset ... */
    /* ----------------------------------------- */
    for(portid = 0,mode = OFF; portid < MAX_HARRIS_PORT; portid++)
    {
        port = (PORT_DB *) portdb[portid];
        
        if(port->rtuid == host->rtuIndex)
        {
            mode = ON;
            break;
        }
    }
    
    if(mode == OFF) return (NOK);
    
    /* ---------------------------------------- */
    /*  MULTI-RTU 의 경우 PORT Index 변경...    */
    /* ---------------------------------------- */
    portid  += (opr->port + 1);                 // 해당 RTU 의 PORT Offset 지정 */
    point = (opr->pointState >> 1) + 1;
    opr->armTCF = (opr->pointState & 1) + 1;
    
    opr->armPoint       = 0;
    opr->armTCF         = 0;
    
    for(i=0; i< MAX_DNP_DO_POINT; i++)
    {
        hostCntr = (CONTROL_INFO *) &host->harrisCntr[i];
        portNum = hostCntr->port;
        ptNum   = hostCntr->point;
            
        if((portid == portNum) && (point == ptNum))
        {
            if(controlArmHARRIS( host, i) == OK)  
            {
                opr->armPoint   = i;
                return (OK);
            }
            else    return(NOK);    /* ARM fail */  
        }                 
    }

    /* ARM fail */
    return(NOK);

}

/*----------------------------------------------------------------------------
* Function Name : pt_opr()
* 수행내용: 상위로부터의 Control-OPR 에 대한 처리함수
* ArgList :
* Return  :  처리결과 상태
---------------------------------------------------------------------------- */   
int pt_opr( HOST_DCB *host, byte *rxbuf, byte *txbuf)
{
    int     portid, point;
    //int     rtuid;
    int     retVal; 

    //rtuid = host->rtuid;           

    if(opr->cntStatus == ARM)
    {
        if((opr->port == rxbuf[2]) && (opr->pointState == (~rxbuf[3] & 0x3f)))
        {
            portid  = opr->port + 1;
            point   = (opr->pointState >> 1) + 1;
            opr->armTCF = (opr->pointState & 1) + 1;

            Debug(console,"harris%02d> OPR port=%d, point=%d, tcf=%d \n", host->id, portid, point, opr->armTCF);
                
            retVal = controlOprHARRIS( host, opr->armPoint, opr->armTCF);

            if(retVal == OK)	return(OK);
        } /* if */
    }
    
    /* operate FAIL */
    return(NOK);
}

/*----------------------------------------------------------------------------
* Function Name : pfr()
* 수행내용: 상위로부터의 Power Fail Reset 에 대한 처리함수
* ArgList :
* Return  :  처리결과 상태
---------------------------------------------------------------------------- */   
int pfr(HOST_DCB *host, byte *rxbuf, byte *txbuf)
{
    int    i;
    //int     rtuid;
    int     index;
    RTU     *rtu;         
    PORT_DB *port;  

	index = host->rtuIndex - 1;
    //rtuid = host->rtuid;           
    rtu = (RTU *) rtubuf[index]; 
      
    for(i = 0 ; i < MAX_HARRIS_PORT; i++)
    {
        port = (PORT_DB *) portdb[i];
        
        if(port->rtuid != host->rtuIndex) continue;
        port->portStatus &= (~PF);
    }

    rtu->status = 0;
    
    host->txcnt = 1;
    
    Debug(console,"harris%2d> SCADA:  Pfr reset ....!\n", host->id);
    
    return(OK);
}

/*----------------------------------------------------------------------------
* Function Name : port_scan()
* 수행내용: 상위로부터의 Port Scan 에 대한 처리함수
* ArgList :
* Return  :  처리결과 상태
---------------------------------------------------------------------------- */   
int port_scan(HOST_DCB *host, byte *rxbuf, byte *txbuf)
{
    int    i;
    int     index;
    //int     rtuid; 
    RTU     *rtu;  
    PORT_DB *port;        
        
    index = host->rtuIndex - 1;        
    //rtuid = host->rtuid;           
    rtu = (RTU *) rtubuf[index];   
    
    /* 97.4.16 updated */
    for(i=0; i< 10; i++)    txbuf[i] = 0;
    txbuf[0] = rtu->id;  /* remote address */

    for(i = 0 ; i < MAX_HARRIS_PORT; i++)
    {
        port = (PORT_DB *) portdb[i];
        
        if(port->rtuid != host->rtuIndex) continue;
        txbuf[host->txcnt++] = (port->portStatus) & (OL|PF|MF);
    }

    /* 97.4.16 updated */
    host->txcnt = 8;  

    return(OK);
}

/*----------------------------------------------------------------------------
* Function Name : time_sync()
* 수행내용: 상위로부터의 Time Sync 에 대한 처리함수
* ArgList :
* Return  :  처리결과 상태
---------------------------------------------------------------------------- */  
int time_sync(HOST_DCB *host, byte *rxbuf, byte *txbuf)
{
    int hour, min, sec;
    long tick, rtctick;
    //int     rtuid;

    hour=0;
    min = 0;
    sec = 0;
#if 1
    //rtuid = host->rtuid;           
        
    tick = rxbuf[2] & 0x7;
    tick = (tick << 6) + rxbuf[3];
    tick = (tick << 6) + rxbuf[4];
    tick = (tick << 6) + rxbuf[5];
    tick = (tick << 6) + rxbuf[6];

    if(tick >= 24*3600*1000)
    {
        txbuf[host->txcnt++] = 1;     /* error */
    }
    else
    {
        rtc->rtctick = tick;     /* set RTC */
        txbuf[host->txcnt++] = 0;

        /*
         * RTU timetick update ...
         */
        rtctick = tick;
        
        tick = rtctick / 1000;
        sec  = tick % 60;
        min  = (tick / 60) % 60;
        hour = tick / 3600;        

        //writeClock( rtc.year, rtc.month, rtc.day, hour, min, sec, 0);
        
        opr->year   = (rtc->year % 100) + 2000;
    	opr->month  = rtc->month;
    	opr->day    = rtc->day;
    	opr->week   = rtc->week;
    	opr->hour   = hour;
    	opr->min    = min;
    	opr->sec    = sec;
        
        opr->rtcUpdateICCP = SET;		// ICCP-HOST Time-Sync 정보 
    	opr->rtcUpdateFlag = SET;
    
        Debug(console,"HARRIS> TIME SYNC : %4d/%2d/%2d - %2d:%2d:%2d ... !\n",
        	opr->year, opr->month, opr->day, opr->hour, opr->min, opr->sec);
        	
    } /* else */

    pause(10);
    Debug(console,"harris%2d> SCADA:  Time sync %2d:%2d:%2d ....!\n", host->id, hour, min, sec);

#endif

    return(OK);

} /* time_sync */

/*----------------------------------------------------------------------------
* Function Name : soe_dump()
* 수행내용: 상위로부터의 SOE Dump 에 대한 처리함수
* ArgList :
* Return  :  처리결과 상태
---------------------------------------------------------------------------- */  
int soe_dump(HOST_DCB *host, byte *rxbuf, byte *txbuf)
{
    //int     rtuid;
    int     index;
    RTU     *rtu;       
    HARRIS_SOE_Q  *soeq;
    
    index = host->rtuIndex - 1;
    //rtuid = host->rtuid;           
    rtu = (RTU *) rtubuf[index]; 
        
    soeq = (HARRIS_SOE_Q *)&rtu->soeq;

    if(rxbuf[2] == 0)   /* resend Last SOE */
    {
        memcpy( &txbuf[1], soeq->lastSOE, 7);
        txbuf[8] = getRTUStatus(host); /* rtu port_status */
    }
    else if(rxbuf[2] == 1)  /* send Next SOE change */
    {
        if(soeq->front != 0)
        {
            soeq->front -= 1;
            memcpy(&txbuf[1], &soeq->que[soeq->front][0], 7);
            txbuf[8] = getRTUStatus(host); /* rtu port_status */
        }
        else
        {
            txbuf[1] = 0;
            txbuf[2] = 0;
            txbuf[3] = 0;
            txbuf[4] = 0;
            txbuf[5] = 0;
            txbuf[6] = 0;   /* port & status */
            txbuf[7] = 0;   /* point address */
            txbuf[8] = getRTUStatus(host) | MF;
        }

        memcpy(soeq->lastSOE, txbuf + 1, 8);    /* store last SOE */

    }
    else if(rxbuf[2] == 0x3f)   /* reset SOE */
    {
        soeq->front = 0;
        rtu->status &= (~(SOE_S | SOEOF));
        txbuf[1] = 0;
        txbuf[2] = 0;
        txbuf[3] = 0;
        txbuf[4] = 0;
        txbuf[5] = 0;
        txbuf[6] = 0;   /* port & status */
        txbuf[7] = 0;   /* point address */
        txbuf[8] = getRTUStatus(host);
    }

    host->txcnt = 9;

    return(OK);

} /* soe_dump */

/*----------------------------------------------------------------------------
* Function Name : time_sync_adj()
* 수행내용: 상위로부터의 Time Sync Adj 에 대한 처리함수
* ArgList :
* Return  :  처리결과 상태
---------------------------------------------------------------------------- */  
int time_sync_adj(HOST_DCB *host, byte *rxbuf, byte *txbuf)
{
    long tick;
    //int     rtuid;
       
    //rtuid = host->rtuid;           
    
    tick = (rxbuf[2] << 6) + rxbuf[3];
    rtc->rtctick += tick;

    return(OK);

} /* time_sync_adj() */


/* ............ end of "hostsk.c " ....................................... */
