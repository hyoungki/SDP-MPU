/*
 * ==============================================================
 * System TARGET : ACE Control SDP-2000 Ver 1.0   
 * Target CPU    : MPC8248, VME6U
 * Main Factors  : 스마트급전용 SDP 자장치 
 *     - MPU 이중화 구성
 *     - ESOP/SIO 연계 : 전력감시, 원격진단, 전력품질, 고장점 등
 *     - 통합 시뮬레이터 : 2013/07/08 V8.2 적용 
 * --------------------------------------------------------------
 * System DESIGN : SANE-SYSTEM   .... by  Lee Ho-Sang
 * Initial-DATA  : 2016,03,25
 * Last Updated  : 2016,03,25
 * ==============================================================
 */


#include	"localLib.h"
#include    "smbus.h"
#include    "i2c-dev.h"

TTY_DESC linkPort 	 = {FID_LINK, 0, NULL, ASYNC_860T, COM_RS232, 115200,   8,  1,  0,'N',    FID_LINK_PORT,   {0, 50000}};

SHM_DESC	    shmDesc = {-1, SHM_KEY, sizeof(SHM_MEMORY), NULL};
SHM_MEMORY	    *shmPtr = NULL;
int             termExec;

TASK_INFO	    *taskPtr= NULL;

RTC             *rtc    = NULL;
OPR_MSG         *opr    = NULL;
CONSOLE_INFO	*console= NULL;

ICCP_DCB                *iccpDCB = NULL;        // ICCP-HOST 구조체
ICCP_SOE_DELETE_QUEUE   *iccpDelSOEQ =NULL;     // ICCP-HOST Delete SOEQ


LINK_MSG        *linkCfg= NULL;                 // CPU 이중화 구조체
SCU_MSG         *scuCfg = NULL;                 // 이중화 절체장치(SCU) 구조체

ICCP_CONFIG     *iccpCFG = NULL;               // ICCP-HOST 구조체
MPU_CONFIG      *mpuCFG  = NULL;                 // MPU Config
ESIO_CONFIG     *esioCFG[MAX_ESIO];             // ESIO 장치 Config

HOST_DCB        *hostDCB[MAX_HOST];             // HOST 관련 구조체 : ICCP, DNP, HARRIS, LANDIS...
POINT_BUF       *devPtBuf[MAX_DEV_POINT];       // SDP 포인트 Config 정보
SCAN_CONFIG     *scanCFG[MAX_SCAN_PORT];        // 하위계전기 SCAN Config
SDP_DEVICE      *deviceCFG[MAX_DEVICE];         // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)
	 
RTU_DATABASE    *rtudb  = NULL;                 // SDP 데이터베이스
HISTORY_QUE     *hque   = NULL;                 // CONSOLE 용 이벤트


PDPDTIME_DATE_TIME      dnpRTC;
PDPDTIME_MS_SINCE_70    dnpRTCInfo;

byte    bit[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

int     devHero;
int     devExio;

MPC860IO_DESC    devSwitch = {0, NULL, 0, "/dev/i2c-1", O_RDWR|O_NDELAY};    // MPU DIP-Switch
MPC860IO_DESC    devInput  = {0, NULL, 0, "/dev/i2c-1", O_RDWR|O_NDELAY};    // LINK 부 Input .....
MPC860IO_DESC    devOutput = {0, NULL, 0, "/dev/i2c-1", O_RDWR|O_NDELAY};    // LINK 부 Output .....

#define P2_IO_INPUT_I2C_ADDR        0x1E
#define P2_IO_OUTPUT_I2C_ADDR       0x18
#define SWITCH_I2C_ADDR             0x1A


byte    cosSts = 0;
int     initCheck  = 0;
int     linkInitial;

extern  int linkMaster();
extern  int linkSlave();





#if 0 // 2026-09-02 오후 2:39:12 move to 2026-09-02 오후 2:39:15
void print_RTU_DATABASE_size(void)
{
    printf("sizeof( ESIO_RTU_DATABASE ) =%d\r\n", sizeof ( ESIO_RTU_DATABASE   ));
    printf("sizeof( DB_ESIO_MPU_CONFIG ) =%d\r\n", sizeof ( DB_ESIO_MPU_CONFIG   ));
    
    printf("sizeof( DB_ESIO_CONFIG ) =%d    MAX_DB_ESIO=%d \r\n", sizeof ( DB_ESIO_CONFIG  ) , MAX_DB_ESIO);

    printf("sizeof( DB_ENET_ENTRY ) =%d\r\n", sizeof ( DB_ENET_ENTRY   ));    
    printf("sizeof( DB_PORT_ENTRY ) =%d MAX_ESIO_PORT=%d\r\n", sizeof ( DB_PORT_ENTRY   ) ,MAX_ESIO_PORT  );    
    printf("sizeof( DB_ESIO_CONFIG ) =%d    MAX_DB_ESIO=%d \r\n", sizeof ( DB_ESIO_CONFIG  ) , MAX_DB_ESIO);
    


    printf("sizeof( DB_HOST_CONFIG ) =%d    MAX_DB_HOST=%d \r\n", sizeof ( DB_HOST_CONFIG  ) , MAX_DB_HOST );
    printf("sizeof( DB_ICCP_CONFIG ) =%d\r\n", sizeof ( DB_ICCP_CONFIG   ));
    
    
    printf("sizeof( DB_MODBUS_PROFILE ) =%d MAX_DB_MODBUS_PROFILE=%d\r\n", sizeof ( DB_MODBUS_PROFILE ), MAX_DB_SCAN_PORT);
    
    
    printf("sizeof( DB_SCAN_CONFIG ) =%d    MAX_DB_DEVICE=%d\r\n", sizeof ( DB_SCAN_CONFIG ),MAX_DB_SCAN_PORT );    
    printf("sizeof( DB_SDP_DEVICE ) =%d    MAX_DB_DEVICE=%d\r\n", sizeof ( DB_SDP_DEVICE ),MAX_DB_DEVICE );        
    printf("sizeof( DB_POINT_BUF ) =%d  MAX_DB_POINT=%d\r\n", sizeof ( DB_POINT_BUF  ),MAX_DB_POINT );    
    printf("sizeof( DB_CAL_POINT ) =%d  MAX_DB_CAL_POINT=%d\r\n", sizeof ( DB_CAL_POINT ) ,MAX_DB_CAL_POINT  );
                 
                 
}   	    
#endif 


/*
*   I2C : INPUT-PORT 극성반전 
*/
int  set_polarity(int fd,  unsigned char data)
{
    
    char buf[3];
    int rc;
    
    /* chage to output  */
    buf[0]= 0x02 ;// register address 
    buf[1]= data ; // all to out 
    
    if ( (rc=write( fd, buf, 2) ) != 2)
    {
        perror("change ploartiy step1\r\n");
        return -1;
          
    }
    
    /* verify */
    buf[0]=0x02 ;
    
    /* set register */
    if ( (rc=write( fd, buf, 1) ) != 1)
    {
        perror("change ploartiy step2\r\n");
        return -1;
          
    }     
    /* read */
     if ( (rc=read( fd, buf, 1) ) != 1)
    {
        perror("change ploartiy step3\r\n");
        return -1;
          
    }     
       
    if ( ( unsigned char)buf[0] != data)   
     {
        printf("change ploartiy step4 R[%x]-W[%x]\r\n", buf[0], data);
        return -1;
          
    }       
    return 0 ;  
}

int  chage_mode( int fd)
{
    char buf[3];
    int rc;

    /* chage to output  */
    buf[0]= 0x03 ;// register address 
    buf[1]= 0x00 ; // all to out 
    
    if ( (rc=write( fd, buf, 2) ) != 2)
    {
        perror("change mode\r\n");
        return 0;
          
    }

    /* chage polarity */
    buf[0]= 0x02 ;// register address 
    buf[1]= 0x00 ; // all to out 
    
    if ( (rc=write( fd, buf, 2) ) != 2)
    {
        perror("change ploartiy\r\n");
        return 0;
          
    }

    return 0;
}


unsigned char   read_i2c ( int fd )
{
    char buf[3];
    buf[0]=0x0;
    int rc; 
    
    /* set register */
    if ( (rc=write( fd, buf, 1) ) != 1)
    {
        perror("set addr\r\n");
        return 0;
    }

    /*read */
    if ( (rc=read ( fd, buf, 1) ) != 1)
    {
        perror("set addr\r\n");
        return 0;
    }

   return  ( unsigned char) buf[0];
      
}

/*
*   CPU : LINK 관련한 CPU 상태정보 Write
*/
int  write_cpuStatus( int fd, unsigned char data)
{
    char buf[3];
    buf[0]=0x1;
    buf[1]=data ;
    int rc; 
    if ( (rc=write( fd, buf, 2) ) != 2)
    {
        perror("change mode\r\n");
        return 0;
    }
   return rc;
}


/* *************************************************************************************
*	FUNCTION : sigHandler()
* **************************************************************************************/
void	SigHandler( int sig )
{
	char    buffer[256];
    if(opr->wdtDebug)   Debug(console,"link> ... signal generated (%2d)...!\n", sig);
    pause(100);
    
	switch(sig)
	{
    case SIGTERM:
        if(opr->wdtDebug)   Debug(console,"link> ... signal [SIGTERM] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "link> ... signal [SIGTERM] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));    	
        pause(1000);     
        termExec = 0;
        linkCfg->connectStatus = 0;
        break;
	        
	case SIGBUS : 
        if(opr->wdtDebug)   Debug(console,"link> ... signal [SIGBUS] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "link> ... signal [SIGBUS] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));    		
        pause(1000);  
        termExec = 0;   	
        linkCfg->connectStatus = 0;    
	    break;
		    
	case SIGSEGV: 
        if(opr->wdtDebug)   Debug(console,"link> ... signal [SIGSEGV] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "link> ... signal [SIGSEGV] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));    		
        pause(1000);  
        termExec = 0;   	
        linkCfg->connectStatus = 0;    
	    break;
		    
	case SIGPIPE: 
        if(opr->wdtDebug)   Debug(console,"link> ... signal [SIGPIPE] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "link> ... signal [SIGPIPE] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));    	
        pause(1000);     	  
        //termExec = 0;   	
        linkCfg->connectStatus = 0;      
		break;

	default : 
	    if(opr->wdtDebug)   Debug(console,"link> ... signal[%2d] generated ...!\n", sig);
	    /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "link> ... signal[%2d] generated ...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));    		
		break;
	}
    
}


//
// 모듈:	ClearEnvironment()
//
void ClearEnv(void)
{
    printf("link exit...!\n");
    PortClose(&linkPort);

    // ICCP Shared memory lock file close
	iccpShmEnd ();
	
	// 공유 메모리 설정 해제
	if (shmPtr != NULL)
		ShmDetach(&shmDesc);
    
    pause(1000);
    close(devSwitch.id);
    close(devInput.id);
    //close(devOutput.id);
    
	// 프로세스 정보를 정리한다
	UpdateProcessInfo(taskPtr, 0, getpid(), 0);
			
}
					    

//
// 모듈:	InitEnvironment()
//
int InitEnv( void)
{
    int     i;
    
	// 공유 메모리 상태 확인
	if (ShmCheck(&shmDesc) < 0)
	{
		printf("link> *ERR_공유 메모리 상태 이상\n");
		return(0);
	}
	
	// 포인터 변수 초기화
	shmPtr = (SHM_MEMORY *) shmDesc.address;
	taskPtr = (TASK_INFO *) &shmPtr->taskInfo[LINK_PROCESS];

	rtc     = (RTC *)       &shmPtr->rtc;
	opr     = (OPR_MSG *)   &shmPtr->opr_msg;
    
    linkCfg = (LINK_MSG *)  &shmPtr->link_msg;                  // MPU 이중화 관련 구조체
    scuCfg 	= (SCU_MSG *)   &shmPtr->scu_msg;                   // 이중화절체장치(SCU) 구조체
    
    iccpDCB = (ICCP_DCB *) &shmPtr->iccpDCB;                   // ICCP_DCB 
    iccpDelSOEQ = (ICCP_SOE_DELETE_QUEUE *) &iccpDCB->soeDeleteQueue;
    
	rtudb   = (RTU_DATABASE *) &shmPtr->rtuDatabase;
    console = (CONSOLE_INFO *) &shmPtr->console; 
    
	hque    = (HISTORY_QUE *)  &shmPtr->localHistoryQ; 

    mpuCFG  = (MPU_CONFIG *) &shmPtr->mpuConfig;                // MPU Network 구성정보
    iccpCFG  = (ICCP_CONFIG *) &shmPtr->iccpDCB.config;                // MPU Network 구성정보
    
    /* ------------------------------------ */
    /* ESIO CFG : ESIO 구조체 (MMAX 5)      */
    /* ------------------------------------ */ 
    for(i=0; i< MAX_ESIO; i++)      esioCFG[i] = (ESIO_CONFIG *) &shmPtr->esioConfig[i];
    for(i=0; i< MAX_HOST; i++)      hostDCB[i] = (HOST_DCB *) &shmPtr->hostDCB[i];
    for(i=0; i< MAX_SCAN_PORT; i++) scanCFG[i] = (SCAN_CONFIG *) &shmPtr->scanCFG[i];
    for(i=0; i< MAX_DEVICE; i++)    deviceCFG[i] = (SDP_DEVICE *) &shmPtr->deviceCFG[i];
	for(i=0; i< MAX_DEV_POINT; i++) devPtBuf[i] = (POINT_BUF *) &shmPtr->devPtBuf[i];
	
	
	/* ------------------------------------ */
    /*  ICCP 공유메모리 초기화              */
    /* ------------------------------------ */
    iccpShmInit (iccpDCB);
    
    /* ------------------------------------------------------------ */
	/* 프로세스 정보를 초기화한다                                   */
	/* HOST Task 는 TCPIP 전송대기를 고려하여 WDT 기능을 삭제함...  */
	/* ------------------------------------------------------------ */
	taskPtr->initial    = 1;
	taskPtr->wdtEnable  = 1;            
	
	/* ------------------------------------------------ */
	/*  PROCESS 호출시... Priority 지정 (-19 ~ 20)      */
	/* ------------------------------------------------ */
	nice(NICE_LINK);
	
	UpdateProcessInfo(taskPtr, 1, getpid(), 1);
	
	return(1);
}



/*
*/
int rcvWithLINK(TTY_DESC *tp, byte *rxbuf)
{
    int     rcvCount;
    int     rcvTimeout;
    int     reqSize, rcvSize;
    byte    lrc;

    rcvTimeout = 0;

    /* receive 1'st STX */
    do {
        if(asyncRead( opr, tp, &rxbuf[0], 1) == 0) return(0);
        if(rcvTimeout++ > 50) return (0);
    } while(rxbuf[0] != 0x7e);
    
    /* receive size */
    if(asyncRead( opr, tp, &rxbuf[1], 3) == 0)
    {
        //if(opr->linkDebug)    
        //Debug(console,"link> Size not received ...!\n");
        return(0);
    }
    
	if(rxbuf[1] != 0x7E)	return (-1);
		
    reqSize = (rxbuf[2]*256 + rxbuf[3]) - 4;   /* except stx, size */ 

    /* receive body */
    if((rcvSize = asyncRead( opr, tp, &rxbuf[4], reqSize)) == reqSize)
    {
        rcvCount = rcvSize + 4;
        lrc = genlrc(rxbuf, rcvCount);
        
        if(lrc == 0)   return (rcvCount);
        else    
        {
            if(opr->linkDebug)    
            {
                Debug(console,"link> *LRC Error ...cal=%2x, rcv=%2x!\n", lrc, rxbuf[rcvCount-1]);
                DumpBuff(console,"*LRC: ", rxbuf, rcvCount);
            }                
            return(0);
        }
        
    }
    else    
    {
        if(opr->linkDebug)    
            Debug(console,"link> *Tail not received ...req=%2d, rcv=%2d!\n", reqSize, rcvSize);
        return(0);
    }
    
}


/*
========================================================
*   CPU : Active/활성화
*    - 상위/하위 통신 Master 기능, Shutdown Disable
========================================================
*/
void setMaster()
{
    int     i;
    char    buffer[256];
    SDP_DEVICE      *dev;
    
    /* -------------------------------------------- */
    /* CPU SHUTDOWN 상태표출 ... ENABLE             */
    /* -------------------------------------------- */
    //linkCfg->cpuControl |= (SET_CPU_MASTER + SET_CPU_LIVE);
    linkCfg->cpuControl = 0x85;   
    
    write_cpuStatus(devOutput.id, linkCfg->cpuControl);  
    
	/* -------------------------------------------- */
	/*	ACTIVE-CPU 관련 이벤트 생성 ...				*/
	/* -------------------------------------------- */
	if(opr->runMode != LOCAL_MASTER)
	{
	    logEvent_MPU(shmPtr, ENT_MASTER_ACT, 0, 0, 0, opr->cpuMode, NULL);  
		    
	    /* LOG File 저장 */
        sprintf(buffer, "%s", ">> SET-MASTER ");
        LogFile_MPU(shmPtr, ENT_MASTER_ACT, buffer, strlen(buffer));  
	}
		
	opr->runMode = LOCAL_MASTER;
    opr->stscode |= LED_ACTIVE_BIT;     // ACT-LED ON
    
    opr->devSoeENBTick = 0;
    
    /* ------------------------------------------------ */
    /*	CPU Active 절체후 .. 60초 뒤... ESIO 통신 Check 		*/
    /* ------------------------------------------------ */
    opr->cpuChgTick = 60;
    //opr->cpuChgTick = 120;
    
    /* MPU Master/Slave 동작상태 표시 */
    mpuCFG->mpuStatus |= MPU_ACT_BIT;   // MPU - Local MASTER 동작
    
    Debug(console,"\n=======================================\n");
    Debug(console," [%02d/%02d-%02d:%02d:%02d]... *** SET MASTER ...%02x !\n", rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec, linkCfg->cpuControl);
    Debug(console,"=======================================\n");

    /* ---------------------------------------------------- */
	/*  MASTER/SLAVE 동작모드에 따른 계전기 Update          */
    /* ---------------------------------------------------- */
    for(i = 0; i < MAX_DEVICE; i++)
    {
        dev = (SDP_DEVICE *) deviceCFG[i];
        dev->offlineTick= 0;
        //dnp->online		= 2;            // 초기상태 ... 미지정.
        dev->init_status = RESET;        // 상태덤프 초기화
        dev->init_analog = RESET;        // 상태덤프 초기화
        
        dev->timeSyncReq = SET;
        dev->statusDump = SET;
        dev->analogDump = SET;
    }
    
    /* -------------------------------- */
    /* LINK-SOE Queue Clear... 			*/
    /* -------------------------------- */
    linkCfg->front = 0;
    linkCfg->rear  = 0;
    
}

/*
========================================================
*   CPU : DeActive/비활성화
*    - 상위/하위 통신 Slave 기능, Shutdown Enable
========================================================
*/
void resetMaster()
{
    char    buffer[256];
    
    /* -------------------------------------------- */
    /* CPU SHUTDOWN 상태표출 ... DISABLE            */
    /* -------------------------------------------- */
    //linkCfg->cpuControl &= ~(SET_CPU_MASTER;
    //linkCfg->cpuControl |= SET_CPU_LIVE;

	/* -------------------------------------------- */
	/*	ACTIVE-CPU 관련 이벤트 생성 ...				*/
	/* -------------------------------------------- */
	if(opr->runMode != LOCAL_SLAVE)
	{
	    logEvent_MPU(shmPtr, ENT_SLAVE_ACT, 0, 0, 0, opr->cpuMode, NULL);  
		    
	    /* LOG File 저장 */
        sprintf(buffer, "%s", ">> *RESET-MASTER ");
        LogFile_MPU(shmPtr, ENT_SLAVE_ACT, buffer, strlen(buffer));  
	}
	    
    linkCfg->cpuControl = 0x01;
    
    write_cpuStatus(devOutput.id, linkCfg->cpuControl);  
    
    opr->runMode = LOCAL_SLAVE;
	opr->rtuinit = 32;
    opr->stscode &= ~LED_ACTIVE_BIT;     // ACT-LED ON
    
	opr->devSoeENBTick = 0;
	
	/* MPU Master/Slave 동작상태 표시 */
	mpuCFG->mpuStatus &= ~MPU_ACT_BIT;      // MPU - Local SLAVE 동작
	
    Debug(console,"\n=======================================\n");
    Debug(console," [%02d/%02d-%02d:%02d:%02d]... *** RESET MASTER ... %02x !\n", rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec, linkCfg->cpuControl);
    Debug(console,"=======================================\n");

    /* -------------------------------- */
    /* LINK-SOE Queue Clear... 			*/
    /* -------------------------------- */
    linkCfg->front = 0;
    linkCfg->rear  = 0;
    
}

static  byte    linkStatus =0, cpuStatus =0;
static  byte    linkChange=0;
static  byte    linkOld, linkTick=0;

/*
*   CPU 이중화 상태 Check...
*/
int	checkDualCPU(int mode)
{
    int check_dualCPU;
    char    buffer[256];
    
    /* -------------------------------------------- */
	/*  이중화 CPU 상태 Check...I2C Read            		*/
	/* -------------------------------------------- */
    linkStatus =  read_i2c (devInput.id);
    
    //if(opr->wdtDebug)	Debug(console,"link> INPUT ... MPU status = %02x ...\n", linkStatus);
    
    /* ---------------------------------------------------- */
    /*  이중화 CPU Check ... Default Status                   	*/
    /*  - 상대방 전원 OFF, 프로세스 미실행 상태, Reboot 상태...          	*/
    /* ---------------------------------------------------- */
    if(opr->cpuMode == MPU_A)
    {
        if((linkStatus == 0x74) ||(linkStatus ==0x73) || (linkStatus == 0x53) || (linkStatus == 0x13))  	// [0x74] Process Kill, [0x73] Rebooting, [0x53] Power Off, [0x13] SIO Reboot
        {
        	if(opr->wdtDebug)	Debug(console,"link> INPUT ... MPU status = %02x ...\n", linkStatus);
        	linkStatus = 0x50;	// SDP-A Single Mode 
        	//linkStatus = (linkStatus & 0x51);
            //if(opr->dualCpuSts == SET)    mode = 1;
        }
    }
    else
    {
        if((linkStatus == 0x64) ||(linkStatus ==0x63) || (linkStatus == 0x43) || (linkStatus == 0x03))		// [0x64] Process Kill, [0x63] Rebooting, [0x43] Power Off, [0x03] SIO Reboot
        {
        	if(opr->wdtDebug)	Debug(console,"link> INPUT ... MPU status = %02x ...\n", linkStatus);
        	linkStatus = 0x40;	// SDP-B Single Mode 	
        	//linkStatus = (linkStatus & 0x41);
            //if(opr->dualCpuSts == SET)    mode = 1;
        }    
    }

    linkChange	= (linkStatus ^ linkOld ) & 0x17;
    linkOld     = linkStatus;

    /* ---------------------------------------------------- */
    /*  이중화 CPU Check...신호COS 처리                          	*/
    /*  - Chattering Check ... 3회                          	*/
    /* ---------------------------------------------------- */    
    if((linkCfg->cmdFlag == 0) && (mode == 0))
    {       
        if(linkChange)
        {
        	if(opr->wdtDebug)	Debug(console,"link> CHANGE CPU status (Return) = %02x ...\n", linkStatus);
            linkTick = 0;
            return (1);
        }   
        else
        {    
            if(++linkTick < 3)  return (1);
            linkTick = 0;
        }
    }
        
    linkCfg->cpuStatus = linkStatus;
    cosSts    = (cpuStatus ^ linkCfg->cpuStatus) & 0x17;      // LINK Check MASK ... [0x01] Dual/Single, [0x02] Master/Slave, [0x04] Active status, [0x10] MPU-A/B
    cpuStatus = linkCfg->cpuStatus;

    if(opr->wdtDebug)	Debug(console,"link> MPU status = %02x ...\n",linkCfg->cpuStatus);
    
    if(cosSts)
  	{  	
    	if(opr->wdtDebug)	Debug(console,"link> CHANGE CPU(*) status = %02x ...\n", linkCfg->cpuStatus);
	}
	
    /* -------------------------------------------- */
    /*  CPU 상태 및 절체스위치 상태에 따른 처리     				*/
    /* -------------------------------------------- */
    if(mode + linkCfg->cmdFlag)	cosSts = 1;
    if(cosSts == 0)     return 0;				/*  CPU 상태변화가 없는경우... Return					*/
    
    linkCfg->cmdFlag = RESET;
                
    /* -------------------------------------------- */
	/* 상대방 CPU 장착유무 확인                     */
	/* SYS_SIO_BIT (0x40)      : SIO 보드 상태 ... 이중화 Check 	*/
	/* SYS_MASTER_BIT (0x10)   : CPU-A, CPU-B 모드 결정				*/
	/* SYS_CPU_ACTIVE (0x04)   : 상대방 CPU 의 ACTIVE 상태			*/ 	
	/* SYS_DUAL_CPU_BIT (0x01) : 상대방 CPU 동작상태... 이중화 Check 	*/
	/* -------------------------------------------- */
	check_dualCPU = linkCfg->cpuStatus & SYS_DUAL_CPU_BIT;
	
	if(check_dualCPU)       // 상대방 CPU의 Live 상태 Check... ACTIVE => 0, 비활성 => 1
    {
        /* ---------------------------------------- */
    	/*  MASTER-CPU 의 경우...                   */
        /* ---------------------------------------- */
    	if(linkCfg->cpuStatus & SYS_MASTER_BIT)  
        {
            if(opr->dualCpuSts != SET)
            {
                printf("\n=======================================\n");
                printf("   **** DUAL CPU MODE : MPU-A ***\n");
                printf("=======================================\n");
                
                /* LOG File 저장 */
                sprintf(buffer, "%s", "**** DUAL CPU MODE : MPU-A ***");
                LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
            }
            
            opr->dualCpuSts = SET;                              // CPU 이중화 여부 결정...
            
	        opr->cpuMode = MPU_A;
	        opr->stscode |= LED_MASTER_BIT;     // MST-LED ON
    	    
    	    /* -------------------------------------------- */
    	    /*  SCU 통신상태에 따른 이중화 절체...          */
    	    /* -------------------------------------------- */
    	    if(scuCfg->remoteMode == MANUAL_MODE)       /* 수동전환 모드의 경우...      */
    	    {
    	        if(scuCfg->chanMode == SYSTEM_A)   
    	        {
    	            printf("MPU-A> SCU-Manual : System-A MODE.... SET-Master...!\n");
    	            
    	            /* LOG File 저장 */
                    sprintf(buffer, "%s", "MPU-A> SCU-Manual : System-A MODE.... SET-Master ");
                    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    
    	            setMaster();
    	        }
    	        else if(scuCfg->chanMode == SYSTEM_B)   
    	        {
    	            printf("MPU-A> SCU-Manual : System-B MODE.... *RESET-Master...!\n");
    	            
    	            /* LOG File 저장 */
                    sprintf(buffer, "%s", "MPU-A> SCU-Manual : System-B MODE.... *RESET-Master ");
                    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
                    
    	            resetMaster();
    	        }
    	    }
    	    else if(scuCfg->remoteMode == AUTO_MODE)    /* 자동전환 모드의 경우...      */
    	    {
    	        /* ------------------------------------------- */
	            /*  이중화 CPU 모두 Master 모드인 경우...               */
    	        /*    => MASTER-CPU : Active 모드 유지...          */
	            /*    => SLAVE-CPU  : Non-Active 모드 설정         */
	            /* -------------------------------------------  */
        	    if(opr->runMode == LOCAL_MASTER)
                {
                    if((linkCfg->cpuStatus & SYS_CPU_ACTIVE))  
                    {
                        printf("MPU-A> MPU-B/MASTER-MODE.... *RESET-Master...!\n");
                        
                        /* LOG File 저장 */
                        sprintf(buffer, "%s", "MPU-A> MPU-B/MASTER-MODE.... *RESET-Master...");
                        LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
                    
                        resetMaster();
                    }
                }
                else 
                {
                    if((linkCfg->cpuStatus & SYS_CPU_ACTIVE) == 0)
                    {
    	                printf("MPU-A> MPU-B/Slave-MODE.... SET-Master...");
    	                
    	                /* LOG File 저장 */
                        sprintf(buffer, "%s", "MPU-A> MPU-B/Slave-MODE.... SET-Master");
                        LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
                        
    	                setMaster();    
    	            }
    	        }    
    	    }    

    	    /* ------------------------------------ */
    	    /*  SDP 운영상태 정보..                 */
    	    /* ------------------------------------ */
    	    mpuCFG->mpuStatus |= (MPU_SDP_A_BIT + MPU_INS_BIT);
        }
        else
        {
            if(opr->dualCpuSts != SET)
            {
                printf("\n=======================================\n");
                printf("   **** DUAL CPU MODE : MPU-B ***\n");
                printf("=======================================\n");
                
                /* LOG File 저장 */
                sprintf(buffer, "%s", "**** DUAL CPU MODE : MPU-B ***");
                LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
            }
            
            opr->dualCpuSts = SET;                              // CPU 이중화 여부 결정...
                
   	        opr->cpuMode = MPU_B;
	        opr->stscode &= ~LED_MASTER_BIT;     // MST-LED ON
	        
	        if(scuCfg->remoteMode == MANUAL_MODE)
    	    {
    	        if(scuCfg->chanMode == SYSTEM_A)   
    	        {
    	            printf("MPU-B> SCU-Manual : System-A MODE.... RESET-Master...\n");

    	            /* LOG File 저장 */
                    sprintf(buffer, "%s", "MPU-B> SCU-Manual : System-A MODE.... RESET-Master");
                    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    	            
    	            resetMaster();
    	        }
    	        else if(scuCfg->chanMode == SYSTEM_B)   
    	        {
    	            printf("MPU-B> SCU-Manual : System-B MODE.... SET-Master...\n");
    	            
    	            /* LOG File 저장 */
                    sprintf(buffer, "%s", "MPU-B> SCU-Manual : System-B MODE.... SET-Master");
                    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
                    
    	            setMaster();
    	        }
    	    }
    	    else if(scuCfg->remoteMode == AUTO_MODE)
    	    {
    	        if(opr->runMode == LOCAL_MASTER)
                {
                    if((linkCfg->cpuStatus & SYS_CPU_ACTIVE))   
                    {
                        printf("MPU-B> MPU-A/MASTER-MODE.... *RESET-Master...\n");
                        
                        /* LOG File 저장 */
                        sprintf(buffer, "%s", "MPU-B> MPU-A/MASTER-MODE.... *RESET-Master");
                        LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
                    
                        resetMaster();
                    }
                }
                else
                {
                    if((linkCfg->cpuStatus & SYS_CPU_ACTIVE) == 0)
                    {
        	            printf("MPU-B> MPU-A/SLAVE-MODE.... SET-Master...!\n");
        	            
        	            /* LOG File 저장 */
                        sprintf(buffer, "%s", "MPU-B> MPU-A/SLAVE-MODE.... SET-Master");
                        LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
                        
    	                setMaster();    
    	            }
                }     
    	    }
    	    
	        /* ------------------------------------ */
    	    /*  SLAVE  측면 : 절체상태 표기...      */
    	    /* ------------------------------------ */
    	    mpuCFG->mpuStatus &= (~MPU_SDP_A_BIT);      // SDP-A Clear... SDT-B 표시
    	    mpuCFG->mpuStatus |= MPU_INS_BIT;           // SDP-B 장착상태
    	}
        
    }	
    else                                     
    {
        /* ---------------------------------------------------- */
	    /*  MASTER/SLAVE 동작모드에 따른 OUTPUT- Enable         */
    	/* ---------------------------------------------------- */
	    if(linkCfg->cpuStatus & SYS_MASTER_BIT)  
    	{
	        opr->cpuMode = MPU_A;
	        opr->stscode |= LED_MASTER_BIT;     // MST-LED ON
	        
			if(opr->dualCpuSts != RESET)
            {
                printf("\n=======================================\n");
                printf("   **** SINGLE CPU MODE : SDP-A MASTER ***\n");
                printf("=======================================\n");
                
  	            /* LOG File 저장 */
                sprintf(buffer, "%s", "**** SINGLE CPU MODE : SDP-A MASTER ***");
                LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
            }
            
			opr->dualCpuSts = RESET;
			
			printf("   **** SINGLE CPU MODE : SDP-A MASTER *** mode = %d, cos=%02x, cur=%02x\n", mode, cosSts, linkCfg->cpuStatus );
			
			setMaster();
    		pause(100);
    		
			/* ------------------------------------ */
    	    /*  SDP 운영상태 정보..                 */
    	    /* ------------------------------------ */
    	    mpuCFG->mpuStatus |= (MPU_SDP_A_BIT + MPU_INS_BIT);
			
	    }
    	else                
	    {
        	opr->cpuMode = MPU_B;
        	opr->stscode &= ~LED_MASTER_BIT;     // MST-LED ON
        	
			if(opr->dualCpuSts != RESET)
            {
                printf("\n=======================================\n");
                printf("   **** SINGLE CPU MODE : SDP-B MASTER ***\n");
                printf("=======================================\n");
                
  	            /* LOG File 저장 */
                sprintf(buffer, "%s", "**** SINGLE CPU MODE : SDP-B MASTER ***");
                LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
            }
            
			opr->dualCpuSts = RESET;

            printf("   **** SINGLE CPU MODE : SDP-B MASTER *** mode = %d, cos=%02x, cur=%02x\n", mode, cosSts, linkCfg->cpuStatus );
			setMaster();
    		pause(100);
    		
	        /* ------------------------------------ */
    	    /*  SLAVE  측면 : 절체상태 표기...      */
    	    /* ------------------------------------ */
    	    mpuCFG->mpuStatus &= (~MPU_SDP_A_BIT);      // SDP-A Clear... SDT-B 표시
    	    mpuCFG->mpuStatus |= MPU_INS_BIT;           // SDP-B 장착상태
            
    	}
    }

    return (1);        
}

/*
*
*/
int check_IPString(char *str, byte *ipNum)
{
    int i;
    int     inx1, inx2;
    char    temp[8][8];
    
   	inx1 =0;
   	inx2 =0;
   	
   	ipNum[0] = 0;
    ipNum[1] = 0;
    ipNum[2] = 0;
    ipNum[3] = 0;
   	
   	for(i=0; i < 16; i++)
    {
        temp[inx1][inx2] = str[i];
        if(str[i] == '.')
        {
            temp[inx1][inx2] = '\0';
            inx1++;
            inx2 = 0;
        }    
        else
        {
            inx2++;
        }    
    }
        
    temp[inx1][inx2++] = '\0';
       
    ipNum[0] = atoi(temp[0]);
    ipNum[1] = atoi(temp[1]);
    ipNum[2] = atoi(temp[2]);
    ipNum[3] = atoi(temp[3]);
    
   
   return (0); 
    
}


/*
*   CPU 부팅후... HOST Route-Table 추가 
*/
int readRouteTable()
{
	char    buf[200];

    FILE_DESC	routeFile = {NULL, FIO_NORMAL, "/mnt/bin/routetable.txt"};
    
    if(opr->cpuMode == MPU_A)  sprintf( routeFile.name, "%s", "/mnt/bin/routetable-A.txt");
    else                       sprintf( routeFile.name, "%s", "/mnt/bin/routetable-B.txt");        

    printf("\n====================================================================\n");
    printf("  ROUTE-Table Read Start .... %s \n", routeFile.name);
    
    /* Database File #1 Read ... */	
	if (FileOpen(&routeFile) < 0)
	{
		printf("====================================================================\n\n");
		return (-1);
	}
	
    while( fgets(buf, 200, routeFile.id) != NULL)
    {
        if(buf[0] =='#')    
        {
            printf(">> %s ", buf);   
            continue;
        }
        
        printf(">> %s ", buf);     
        
        /* ---------------------------- */
        /* Linux-System 명령 출력...    */
        /* ---------------------------- */
        system(buf);
        pause(100);
    }
    
	FileClose(&routeFile); 
	
	printf("  ROUTE-Table Read End .... %s \n", routeFile.name);
	printf("====================================================================\n");
	return (1);
	
}

//
enum {
    SYSRUN_ERR_SYSTEM = -1000,
    SYSRUN_ERR_UNKNOWN = -1001
};

/**
 * system() 반환값을 "단순한 정수"로 변환한다.
 * 반환:
 *  0            : 명령 성공 (exit code 0)
 *  1..255       : 명령 정상 종료했지만 실패 (exit code)
 *  -N           : 시그널 N으로 종료됨 (예: -9는 SIGKILL)
 *  SYSRUN_ERR_* : system() 자체 실패 또는 알 수 없는 상태
 */
void  system_simple(const char *cmd)
{
    int ret = system(cmd);
    if (ret == -1) {
        printf("running system(%s) failed\r\n",cmd);
        //return SYSRUN_ERR_SYSTEM;   // system() 자체 실패
        return  ; 
    }

    if (WIFEXITED(ret)) {
        if (  WEXITSTATUS(ret) ) 
                printf( "running system(%s) exited with %d\r\n",cmd,WEXITSTATUS(ret)  );    
        return  ; 
   }

    if (WIFSIGNALED(ret)) {
        printf("running system(%s) failed with signal\r\n",cmd);
        //return -WTERMSIG(ret);      // -signal
        return  ; 
    }
        printf("running system(%s) failed with unkown reason\r\n",cmd);
    return SYSRUN_ERR_UNKNOWN;      // 드문 케이스 (정지/재개 등)
}


#if  0 //mv to wdt
int check_interface_exists(const char *ifname) {
    char path[256];
    snprintf(path, sizeof(path), "/sys/class/net/%s", ifname);
    
    // 해당 디렉토리가 존재하면 인터페이스가 있는 것
    if (access(path, F_OK) == 0) {
        return 0;  // 존재함
    }
    return -1;  // 존재하지 않음
}


/*
*   NETWORK-Config 
*/
void mpuNetwork_Initial()
{
    char    buffer[256];
    
    MPU_NET_ENTRY   *mpuNet1, *mpuNet2;
    MPU_NET_ENTRY   *mpuNet3, *mpuNet4;
    //MPU_NET_ENTRY   *mpuNet3;
    
    /* -------------------------------------------- */
    /*  MPU 실장모드에 따라서 각 Network IP 설정    */
    /* -------------------------------------------- */
    if(opr->cpuMode == MPU_A)
    {
        mpuNet1 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[0];
        mpuNet2 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[1];
        //mpuNet3 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[2];
        // hkkim
        mpuNet3 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[2];
        mpuNet4 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[3];
    
    }        
    else
    {
        mpuNet1 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[0];
        mpuNet2 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[1];
        //mpuNet3 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[2];
        // hkkim
        mpuNet3 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[2];
        mpuNet4 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[3];
    }
        

    printf("=> Network#1 : IP=%s, Mask=%s \n", mpuNet1->ipAddr, mpuNet1->subMask);
    printf("=> Network#2 : IP=%s, Mask=%s \n", mpuNet2->ipAddr, mpuNet2->subMask);
    printf("=> Network#3 : IP=%s, Mask=%s \n", mpuNet3->ipAddr, mpuNet3->subMask);    
    printf("=> Network#4 : IP=%s, Mask=%s \n", mpuNet4->ipAddr, mpuNet4->subMask);

#ifdef  ROM_VERSION
    if(mpuNet1->useFlag == SET)      
    {       
        /* MASTER-CPU : eth0 Network...   */      
        
        if ( ! check_interface_exists("eth0") )       
        {      
        sprintf(buffer, "ifconfig eth0 %s netmask %s up\n", mpuNet1->ipAddr, mpuNet1->subMask);
        printf("=> Network#0 : %s", buffer);
//      	system(buffer); 
   	    system_simple(buffer);
   	    pause(100);  
    	}
    	else printf("=> Network#0 not ready\r\n");
   	
   	}
   	else
    {
        /* MASTER-CPU : eth0 Network...   */  
        if ( ! check_interface_exists("eth0") )   
        {          
            sprintf(buffer, "ifconfig eth0 down\n");
            printf("=> Network#0 : %s", buffer);
   	        system_simple(buffer);
   	        pause(100);  
        }
    	else printf("=> Network#0 not ready\r\n");        
    }  	    
#endif
        
    /* MASTER-CPU : eth1 Network...   */ 
    if(mpuNet2->useFlag == SET)
    {            
        if ( ! check_interface_exists("eth1") ) 
        {
            sprintf(buffer, "ifconfig eth1 %s netmask %s up\n", mpuNet2->ipAddr, mpuNet2->subMask);
            printf("=> Network#1 : %s", buffer);
//      	system(buffer);  
   	        system_simple(buffer);
   	        pause(100);  
   	    }
    	else printf("=> Network#1 not ready\r\n");   	    
   	    
    }
    else  
    {
        /* MASTER-CPU : eth0 Network...   */            
         if ( ! check_interface_exists("eth1") ) 
        {
        sprintf(buffer, "ifconfig eth1 down\n");
        printf("=> Network#1 : %s", buffer);

   	    system_simple(buffer); 
   	    pause(100);  

        }
    	else printf("=> Network#1 not ready\r\n");  
                
    }
    
    /* MASTER-CPU : eth1 Network...   */ 
    if(mpuNet3->useFlag == SET)
    {            
        if ( ! check_interface_exists("eth2") ) 
        {

        sprintf(buffer, "ifconfig eth2 %s netmask %s up\n", mpuNet3->ipAddr, mpuNet3->subMask);
        printf("=> Network#2 : %s", buffer);
   	    system_simple(buffer);
   	    pause(100);  
   	    }
    	else printf("=> Network#2 not ready\r\n");      	    
    }
    else  
    {
        /* MASTER-CPU : eth0 Network...   */            
         if ( ! check_interface_exists("eth2") ) 
        {
           
        sprintf(buffer, "ifconfig eth2 down\n");
        printf("=> Network#2 : %s", buffer);

   	    system_simple(buffer);
   	    pause(100);  

        }
    	else printf("=> Network#2 not ready\r\n");  
                
    }
    
    /* MASTER-CPU : eth1 Network...   */ 
    if(mpuNet4->useFlag == SET)
    {            
         if ( ! check_interface_exists("eth3") ) 
        {
        
        sprintf(buffer, "ifconfig eth3 %s netmask %s up\n", mpuNet4->ipAddr, mpuNet4->subMask);
        printf("=> Network#4 : %s", buffer);
   	    system_simple(buffer); 
   	    pause(100);  
        }
    	else printf("=> Network#3 not ready\r\n");      	    
    }
    else  
    {
        /* MASTER-CPU : eth3 Network...   */            
         if ( ! check_interface_exists("eth3") ) 
        {
        
        sprintf(buffer, "ifconfig eth3 down\n");
        printf("=> Network#4 : %s", buffer);

   	    system_simple(buffer); 
   	    pause(100);  
             
        }
    	else printf("=> Network#3 not ready\r\n");               
             
                
    }    

    if(opr->cpuMode == MPU_A)
    {
        printf("\n------------------------------------------\n");
        printf("link> MPU-A ...Network Initial OK ...! \n");
        printf("------------------------------------------\n");
    }
    else
    {
        printf("\n------------------------------------------\n");
        printf("link> MPU-B ...Network Initial OK ...! \n");
        printf("------------------------------------------\n");
    }
    


}

#endif 

/* ------------------------------------------------------- */
/*  LINK MAIN 프로그램 Start Routine ....             */
/* ------------------------------------------------------- */
int	main(int argc, char **argv)
{
    //int index, status;
    char	buffer[128];
    
	// 작업 환경을 초기화한다
	termExec = InitEnv();
	
	taskPtr->wdtCount = 0;
		
    
 		
	/* Signal Handler initial */
	signal( SIGBUS,  SigHandler);
	signal( SIGSEGV, SigHandler);
	signal( SIGTERM, SigHandler);	
	signal( SIGPIPE, SigHandler);

    PortInitial((TTY_DESC *) &linkPort);

    /* ---------------------------------------------------- */
    /*  MPU DIP-Switch  IO :                                */
    /* ---------------------------------------------------- */
    if((devSwitch.id = open( devSwitch.name, devSwitch.attr)) <0)  
	{
        printf("link> *Device Open Error ... SWITCH (%s) \n", devSwitch.name);
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    if (ioctl(devSwitch.id,  I2C_SLAVE , SWITCH_I2C_ADDR) < 0) 
    {
        printf("link> *Failed to acquire bus access and/or talk to slave 0... SWITCH \n");
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    if( set_polarity(devSwitch.id, 0x00) <0 )
    {
        printf("link> *set_polarity fail ... SWITCH  \n");
        //exit(1);
        termExec = 0;
        return (0);
    }

    /* ---------------------------------------------------- */
    /*  MPU P2-IO-INPUT  :                                  */
    /* ---------------------------------------------------- */
    if((devInput.id = open( devInput.name, devInput.attr)) <0)  
	{
        printf("link> *Device Open Error ... P2-INPUT (%s) \n", devInput.name);
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    if (ioctl(devInput.id,  I2C_SLAVE , P2_IO_INPUT_I2C_ADDR) < 0) 
    {
        printf("link> *Failed to acquire bus access and/or talk to slave 0... P2-INPUT \n");
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    if( set_polarity(devInput.id, 0x00) <0 )
    {
        printf("link> *set_polarity fail ... P2-INPUT  \n");
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    /* ---------------------------------------------------- */
    /*  MPU P2-IO-OUTPUT  :                                 */
    /* ---------------------------------------------------- */
    if((devOutput.id = open( devOutput.name, devOutput.attr)) <0)  
	{
        printf("link> *Device Open Error ... P2-OUTPUT (%s) \n", devOutput.name);
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    if (ioctl(devOutput.id,  I2C_SLAVE , P2_IO_OUTPUT_I2C_ADDR) < 0) 
    {
        printf("link> *Failed to acquire bus access and/or talk to slave 0... P2-OUTPUT \n");
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    if( set_polarity(devOutput.id, 0x00) <0 )
    {
        printf("link> *set_polarity fail ... P2-OUTPUT  \n");
        //exit(1);
        termExec = 0;
        return (0);
    }
    
    /* OUTPUT 속성 변경 */
    chage_mode( devOutput.id);
    
    /* -------------------------------------------- */
    /* SCU - 디폴트 절체 상체....                   */
    /* -------------------------------------------- */
    scuCfg->remoteMode  = AUTO_MODE;
    scuCfg->chanMode    = SYSTEM_A;

    resetMaster();
    pause(1000);
    
//    print_RTU_DATABASE_size();    move to cli2
    
    
    /* LOG File 저장 */
	sprintf(buffer, " %s LINK-MAIN PROCESS Activated ... !", TARGET_NAME);
    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    
    printf(" %s LINK-MAIN PROCESS Activated ... !\n", TARGET_NAME);
        
    /* -------------------------------------------- */
    /*  CPU 이중화 상태 Check...                    */
    /* -------------------------------------------- */
    linkInitial = SET;
    if(checkDualCPU(1) == 0)
    {
        printf("link> *** check Dual CPU status... Error !\n");
        exit(0);
    }

    linkInitial = RESET;

    /* ---------------------------------------------------- */
	/*  MASTER/SLAVE LOCAL 환경정보 초기화 ...              */
	/*  - MASTER : NET#1/NET#2/NET#3 초기화                 */
	/*  - SLAVE  : NET#1/NET#2/NET#3 초기화                 */
    /* ---------------------------------------------------- */
 // hkkim
 //   mpuNetwork_Initial();
    
    /* ---------------------------------------------------- */
    /*  HOST Network 초기화 : Route Tabel...                */
    /*  - file : /mnt/routetable.txt                        */
    /* ---------------------------------------------------- */
    readRouteTable();

    /* ---------------------------------------------------- */
	/*  MASTER/SLAVE 운영모드별 TASK 호출                   */
    /* ---------------------------------------------------- */
   	if(opr->cpuMode == MPU_A)   linkMaster();        
    else                        linkSlave();
    
    resetMaster();    		
		            
    /* -------------------------------------------- */
    /* CPU SHUTDOWN 상태표출 ... ENABLE             */
    /* -------------------------------------------- */
    //linkCfg->cpuControl = 0x03;
    //write_cpuStatus(devOutput.id, linkCfg->cpuControl);  
    		          
    printf("LINK: ..... CLEAR....%02x !\n", linkCfg->cpuControl);
        		              
	// 작업 환경을 정리한다
	ClearEnv();
	
	return (0);
}
