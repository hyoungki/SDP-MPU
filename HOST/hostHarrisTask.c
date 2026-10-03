#include	"localLib.h"
#include    "external.h"

extern  int     hostChanRead(int hostid, int chid, byte *rxbuf, int reqCount);

extern  int     dat_dump(byte *sizBuf, HOST_DCB  *host, byte *rxbuf, byte *txbuf);
extern  int     cos_check(HOST_DCB *host, byte *rxbuf, byte *txbuf);
extern  int     cos_dump(HOST_DCB *host, byte *rxbuf, byte *txbuf);
extern  int     soe_dump(HOST_DCB *host, byte *rxbuf, byte *txbuf);
extern  int     sts_dump(HOST_DCB *host, byte *rxbuf, byte *txbuf);

extern  int     pt_arm(HOST_DCB *host, byte *rxbuf, byte *txbuf);
extern  int     pt_opr( HOST_DCB *host, byte *rxbuf, byte *txbuf);
extern  int     pfr(HOST_DCB *host, byte *rxbuf, byte *txbuf);
extern  int     port_scan(HOST_DCB *host, byte *rxbuf, byte *txbuf);
extern  int     time_sync(HOST_DCB *host, byte *rxbuf, byte *txbuf);
extern  int     time_sync_adj(HOST_DCB *host, byte *rxbuf, byte *txbuf);

extern  int     vmeHostInitial(int hostid, int chid);
extern  int     vmeHostChanReset(int hostid, int chid);
extern  int     hostChanWrite(int hostid, int chid, int socketID, byte *sndbuf, int sndCount);

extern  TASK_INFO	    *taskPtr;
extern  int             termExec;

extern  int             wdtHostFlag[MAX_HOST];

static	byte	sndBuffer[256];  
static	byte	rcvBuffer[256];  
    

/**
**  receive data with LRC
**/
int rcvWithHARRIS(int hostid, int chid, byte *rxbuf)
{
    short   i;
    int     idpassed = !OK;
    short   reqSize,rcvCount;
    //short   index , count;
    short   rcvTick;
    RTU     *rtu;

    HOST_DCB    *host;

    host = (HOST_DCB *) hostDCB[hostid];

    rcvCount = 0;
    rcvTick = 0;

    /* -------------------------------- */
    /*  check RTU Address ... HEADER    */
    /* -------------------------------- */
    do {
        if(hostChanRead(hostid, chid, &rxbuf[0], 1) == 0) return (0);

        /* address check */
        for(i = 0; i < MAX_HARRIS_RTU; i++)
        {
            rtu = (RTU *) rtubuf[i];
            if(rxbuf[0] == (rtu->id | 0x40))
            {
                idpassed = OK;
                host->rtuIndex = i+1;
                host->rtuid    = rtu->id;
                break;
            }
        } /* for */
        
        if(++rcvTick > 30)
        {
            if(opr->hostDebug == hostid)    Debug(console,"HOST%d> *** Channel noise check !\n", hostid+1);
            return (0);
        }                
    } while(idpassed != OK);

    pause(100);
    
    /* -------------------------------- */
    /*  read OPCODE & check Size        */
    /* -------------------------------- */
    if(hostChanRead(hostid, chid, &rxbuf[1], 1) == 0)
    {
        if(opr->hostDebug == hostid)    Debug(console,"HOST%d> Opcode Not receive\n", hostid+1);
        return(0);
    }
    else
    {
        if(rxbuf[1] >= 35)
        {
            if(opr->hostDebug == hostid)    Debug(console,"HOST%d> Opcode error -> %x\n", hostid+1, rxbuf[1]);
            return(0);
        }

        reqSize = rtu->cmdSize[rxbuf[1]] - 2;
        if(reqSize == 0)
        {
            if(opr->hostDebug == hostid)    Debug(console,"HOST%d> rxcnt error -> %d\n", hostid+1, reqSize);
            return(0);
        }
    }

    /* recevie BODY & TAIL */
    if(hostChanRead(hostid, chid, &rxbuf[2], reqSize) < reqSize)
    {
        if(opr->hostDebug == hostid)    Debug(console,"HOST%d> Tail not received\n", hostid+1);
        return(0);
    }

    rcvCount = reqSize + 2;
    
    /* check LRC */
    if(genlrc(rxbuf, rcvCount) != 0x40)
    {
        if(opr->hostDebug == hostid)
        { 
            Debug(console,"HOST%d> *LRC error\n", hostid+1);
            DumpBuff(console,"RXD[*] :", rxbuf, rcvCount);
        }   
        return(0);
    }

    else return(rcvCount);
            
}


/*----------------------------------------------------------------------------
* Function Name : cmdrv()
* 수행내용: HARRIS HOST 수신 Packet에 대한 OPCODE 해석및 처리 함수
* ArgList :
*   1. opcode - 수신 Packet내의 Opcode
* Return  :  수신Packet 처리상태
---------------------------------------------------------------------------- */  
int cmdrvHARRIS(int hostid, int opcode, byte *rxbuf, byte *txbuf)
{
    //int     rtuid;
    int     index;
    //byte    txcnt, rxcnt;
    int     cmd_sts;
    //byte    *rxbuf, *txbuf;
    byte    lrc;
    RTU         *rtu;
    HOST_DCB    *host;

    host = (HOST_DCB *) hostDCB[hostid];                 /* 주장치 #1 속성정의 */

    index = host->rtuIndex - 1;            
    rtu = (RTU *) rtubuf[index]; 
                
    txbuf[0] = rtu->id;  /* remote address */
    host->txcnt = 1;

    cmd_sts = OK;

    switch(opcode)
    {
        case 0x00:  /* Data Dump    */
            cmd_sts = dat_dump(&rxbuf[2], host, rxbuf, txbuf);
            break;

        case 0x03:  /* Change_Check */
            cmd_sts = cos_check(host, rxbuf, txbuf);
            break;

        case 0x04:  /* Change_Dump  */
            cmd_sts = cos_dump(host, rxbuf, txbuf);
            break;

        case 0x05:  /* Status_Dump  */
            cmd_sts = sts_dump(host, rxbuf, txbuf);
            break;

        case 0x06:  /* Point_Arm    */
            cmd_sts = pt_arm( host, rxbuf, txbuf);
            break;

        case 0x07:  /* Point_Opr    */
            cmd_sts = pt_opr( host, rxbuf, txbuf);
            break;

        case 0x0b:  /* Power Fail Reset */
            cmd_sts = pfr(host, rxbuf, txbuf);
            break;

        case 0x0c:  /* Port Status Scan */
            cmd_sts = port_scan(host, rxbuf, txbuf);
            break;

        case 0x11:  /* Time Syncronization */
            cmd_sts = time_sync(host, rxbuf, txbuf);
            break;

        case 0x12:  /* SOE Change dump */
            cmd_sts = soe_dump(host, rxbuf, txbuf);
            break;

        case 0x13:  /* Time Sync. Adjustment */
            cmd_sts = time_sync_adj(host, rxbuf, txbuf);
            break;

        default :
            cmd_sts++;
            break;
    }   /* switch end */


    rtu->preOpcode = rxbuf[1];   /* store opcode */
    
    lrc = genlrc(&txbuf[0], host->txcnt);
    txbuf[host->txcnt++] = lrc;
    
#if 0
    /* store command */
    for(i=0; i< 16; i++)    rtu->rxbuf[i] = 0;
    for(i=0; i< rxcnt; i++)
    {
        rtu->rxbuf[i] = rxbuf[i];
        if(i >= 15) break;
    }
#endif

	return (host->txcnt);
    //return(cmd_sts);

}

/* ------------------------------------------------------- */
/*  HOST MAIN 프로그램 Start Routine ....             */
/* ------------------------------------------------------- */
int    hostHarrisThread(int hostid)
{
	int		oldsec=0;
	int     activeFID=0;
	int    	rxcnt, txcnt;
	
	byte    *rxbuf,*txbuf;	
	
	RTU         *rtu;
	HOST_DCB    *host;
	
	// HOST별 Index ...
    host = (HOST_DCB *) hostDCB[hostid];                 /* 주장치 #1 속성정의 */
    
    host->online[MASTER_PORT] 		= 0;    // default OFFLINE
    host->online[SLAVE_PORT]  		= 0;    // default OFFLINE
    host->comFailTick[MASTER_PORT]  = 0;
    host->comFailTick[SLAVE_PORT]   = 0;

    /* -------------------------------------------- */
    /*  VMESIO Channel Comm Parameter 초기화        */
    /* -------------------------------------------- */
    host->vmeChan[MASTER_PORT]->protocolType = SCAN_HARRIS;
    host->vmeChan[MASTER_PORT]->cfgType      = host->hostComType;
    host->vmeChan[MASTER_PORT]->cfgSpeed     = host->hostComSpeed;
    
    host->vmeChan[SLAVE_PORT]->protocolType  = SCAN_HARRIS;
    host->vmeChan[SLAVE_PORT]->cfgType       = host->hostComType;
    host->vmeChan[SLAVE_PORT]->cfgSpeed      = host->hostComSpeed;
    
    // 순서가..vmeHostInitial --> vmeHostChanReset
    vmeHostInitial(hostid, host->vmeMstChan);  
    vmeHostInitial(hostid, host->vmeSlvChan);  

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
        printf(" %s HOST[%d] HARRIS-THREAD *DUAL [%d/%d]*  Activated ... !\n", TARGET_NAME, hostid+1, host->vmeMstChan, host->vmeSlvChan);
    }
    else if(host->hostDualMode == HOST_SINGLE)      // 개별 구조인 경우 
    {
        printf(" %s HOST[%d] HARRIS-THREAD *SINGLE [%d/%d]*  Activated ... !\n", TARGET_NAME, hostid+1, host->vmeMstChan, host->vmeSlvChan);
    }        
    else         // 사용하지 않는 경우 
    {
        printf(" %s HOST[%d] HARRIS-THREAD *NOT Define [%d/%d]*  Activated ... !\n", TARGET_NAME, hostid+1, host->vmeMstChan, host->vmeSlvChan);
        
        while(termExec)
	    {
	        wdtHostFlag[hostid] = 0;
		    pause(1000);        
		    
		    if(opr->wdtDebug)
		    Debug(console,">> *HOST[%d] HARRIS-THREAD *NOT Define ... !\n", hostid+1);
        }		    
    }

    /* ---------------------------------------- */
    /* MASTER/SLAVE 포트구성에 따른 RTU 지정    */
    /* ---------------------------------------- */
    rtu = (RTU *) rtubuf[0]; 
    
    //txbuf = (byte *) &host->sndBuffer[0];  
    //rxbuf = (byte *) &host->rcvBuffer[0];  
    txbuf = (byte *) &sndBuffer[0];  
    rxbuf = (byte *) &rcvBuffer[0];  
             
    host->noCommand  = 0;
    host->rxcnt= 0;
    host->txcnt= 0;  

    opr->hostHarris = SET;
    opr->hostRestart[hostid] = RESET;
    
    /* HOST 별 RTU 지정 */
    rtu->cosq.front = rtu->soeq.front = 0;
    printf("harris> HOST(%d) rtu id = %d, ciPort=%d, dataPort=%d\n", hostid+1, rtu->id, rtu->ciPortCnt, rtu->dataPortCnt);


	while(termExec)
	{
	    taskPtr->wdtCount = 0;
	    wdtHostFlag[hostid] = 0;
		pause(20);

        /* ---------------------------------------------------- */
		/*	CPU 이중화 : LOCAL-SLAVE ... 대기모드 				*/
		/* ---------------------------------------------------- */
		if(opr->runMode == LOCAL_SLAVE)	
		{
		    /* 2015.08.25 Update...SLAVE모드시 HOST Offline 처리 */
		    host->online[MASTER_PORT] = 0;
		    host->online[SLAVE_PORT]  = 0;
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
            
		    continue;
		}
		
        host->rcvPACKET = RESET;
        
        /* ---------------------------------------------------- */
        /*  MASTER 통신포트 : 상위 데이터 수신 처리 ...         */
        /* ---------------------------------------------------- */
        if((rxcnt = rcvWithHARRIS( hostid, MASTER_PORT, rxbuf)) > 0)
        {
            host->rxcnt = rxcnt;
            host->comFailTick[MASTER_PORT]= 0;
            host->rcvPACKET = SET;
            activeFID = MASTER_PORT;
                        
            if(host->online[MASTER_PORT] == 0)  
            {
                logEvent_MPU(shmPtr, ENT_HOST_ONLINE, hostid+1, MASTER_PORT, 0, hostid+1, NULL);    
            }
            host->online[MASTER_PORT] = 1;
        }
        /* ---------------------------------------------------- */
        /*  SLAVE  통신포트 : 상위 데이터 수신 처리 ...         */
        /* ---------------------------------------------------- */        
        else if(host->hostDualMode == HOST_DUAL)
        {
            if((rxcnt = rcvWithHARRIS( hostid, SLAVE_PORT, rxbuf)) > 0)
            {
                host->rxcnt = rxcnt;
                host->comFailTick[SLAVE_PORT]= 0;
                host->rcvPACKET = SET;
                activeFID = SLAVE_PORT;
                        
                if(host->online[SLAVE_PORT] == 0)  
                {
                    logEvent_MPU(shmPtr, ENT_HOST_ONLINE, hostid+1, SLAVE_PORT, 0, hostid+1, NULL);    
                }
            
                host->online[SLAVE_PORT] = 1;
            }            
        }

        /* ---------------------------- */
        /*  response HOST message       */
        /* ---------------------------- */        
        if(host->rcvPACKET == SET)
        {
            if(opr->hostDebug == hostid)
            {
                if(activeFID == MASTER_PORT)        DumpBuff(console,"RXM:", rxbuf, rxcnt);
                else if(activeFID == SLAVE_PORT)    DumpBuff(console,"RXS:", rxbuf, rxcnt);
            }

            /* ---------------------------------------- */
            /*  이중화 구성시...SLAVE 시 Continue       */
            /* ---------------------------------------- */
            if(opr->runMode == LOCAL_SLAVE) continue;
                            
            if((txcnt = cmdrvHARRIS(hostid, rxbuf[1], rxbuf, txbuf)) > 0)
            {
                if(opr->hostDebug == hostid)    
                {
                    if(activeFID == MASTER_PORT)        DumpBuff(console,"TXM:", &txbuf[0], txcnt);
                    else if(activeFID == SLAVE_PORT)    DumpBuff(console,"TXS:", &txbuf[0], txcnt);
                }

                if(activeFID == MASTER_PORT)        hostChanWrite( hostid, MASTER_PORT, 0, txbuf, txcnt);
                else if(activeFID == SLAVE_PORT)    hostChanWrite( hostid, SLAVE_PORT, 0, txbuf, txcnt);
            }
        }                   

        if(oldsec == rtc->sec)  continue;
        oldsec = rtc->sec;
        
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
            
            /* HOST-Offline 이벤트 생성 */
            if(host->online[MASTER_PORT] == 1)  
            {
                logEvent_MPU(shmPtr, ENT_HOST_OFFLINE, hostid+1, MASTER_PORT, 0, hostid+1, NULL);    
            }
            host->online[MASTER_PORT] = 0;

            /* -------------------------------- */                        
            /* VME Channel Reinitial...         */
            /* VMECHAN 프로토콜 타입 지정       */
            /* -------------------------------- */  
            vmeHostChanReset(hostid, MASTER_PORT);
            host->vmeChan[MASTER_PORT]->protocolType  = SCAN_HARRIS;
            host->vmeChan[MASTER_PORT]->cfgType       = host->hostComType;
            host->vmeChan[MASTER_PORT]->cfgSpeed      = host->hostComSpeed;

            vmeHostInitial(hostid, host->vmeMstChan);  
            if(opr->hostDebug == hostid)
            Debug(console,">> HARRIS-HOST(%2d) : Master Comm Point initial...!\n", hostid+1);            
            
        }
        
        /* DUAL 모드인 경우 : Master Port 가 OFFLINE 인 경우 */
        else if(host->hostDualMode == HOST_DUAL)
        {
            if(++host->comFailTick[SLAVE_PORT] > 30)   
            {
                
                host->comFailTick[SLAVE_PORT] = 0;
                if(host->online[SLAVE_PORT] == 1)  
                {
                    logEvent_MPU(shmPtr, ENT_HOST_OFFLINE, hostid+1, SLAVE_PORT, 0, hostid+1, NULL);  
                }
                host->online[SLAVE_PORT] = 0;

                /* -------------------------------- */                        
                /* VME Channel Reinitial...         */
                /* VMECHAN 프로토콜 타입 지정       */
                /* -------------------------------- */  
                vmeHostChanReset(hostid, SLAVE_PORT);
                
                host->vmeChan[SLAVE_PORT]->protocolType  = SCAN_HARRIS;
                host->vmeChan[SLAVE_PORT]->cfgType       = host->hostComType;
                host->vmeChan[SLAVE_PORT]->cfgSpeed      = host->hostComSpeed;

                vmeHostInitial(hostid, host->vmeSlvChan);  
 
                if(opr->hostDebug == hostid)
                Debug(console,">> HARRIS-HOST(%2d) : Slave Comm Point initial...!\n", hostid+1);                               
            }
        }
                
	}/* while */

    return (0);
}

