/*
 * ==============================================================
 * System TARGET : ACE Control VME-CU Ver 2.0   
 * Target CPU    : MPC8248, VME6U
 * Main Factors  : 호남고속철용 VMECU
 *          - CPU 이중화 구성
 *          - VMEBUS Function : DI/DO/AI (프로세서 타입)    
 *          - 통합 시뮬레이터 : 2013/07/08 V8.2 적용 
 * --------------------------------------------------------------
 * System DESIGN : SANE-SYSTEM   .... by  Lee Ho-Sang
 * Initial-DATA  : 2012,12,10
 * Last Updated  : 2013,07,10
 * ==============================================================
 */

#ifndef	USRTYPE_HEADER_INCLUDED
#define	USRTYPE_HEADER_INCLUDED

/* ---------------------------------------- */
/* HISTORY & EVENT 저장용 메모리 영역       */
/* ---------------------------------------- */
// hkkim
//#define MRAM_START_ADDRESS     	0xeb000000         /* this definition is related with dd and cpld */
#define MRAM_START_ADDRESS     	0x54000000         /* this definition is related with dd and cpld */

#define MRAM_SIZE              	0x80000            /* 512K */

/* -------------------------------------------- */
/*  이중화 절체기 상태 정보                     */ 
/* -------------------------------------------- */
#define MANUAL_MODE         0
#define AUTO_MODE           1  

#define SYSTEM_A            0                
#define SYSTEM_B            1

#define CHANGE_S_TO_M       1       // Slave  에서 Master 로 절체
#define CHANGE_M_TO_S       2       // Master 에서 Slage  로 절체

#define CHANGE_ALL_DEVICE   1       // 전체 계전기 통신상태 
#define CHANGE_EACH_DEVICE  2       // 개별 계전기 통신상태


/* -------------------------------------------- */
/*  CPU 상태 정보                               */ 
/* -------------------------------------------- */

#define SET_CPU_MASTER      0x80                // LINK : CPU 가 MASTER-ACTIVE 임을 표시
#define SET_CPU_LIVE        0x01                // LINK : CPU 가 동작중임을 표시, LINK1

//#define DATA_CPU_LIVE       0x01                // LINK : CPU 가 동작중임을 표시
#define SYS_CPU_ACTIVE      0x04                // LINK : CPU 가 ACTIVE 임을 표시

#define SYS_PEER_BIT        0x20                // LINK : PEER IN/OUT 판정 Bit... 전원투입여부
#define SYS_MASTER_BIT      0x10                // LINK : Master/Slave 판정 Bit
#define SYS_DUAL_CPU_BIT    0x01                // LINK : 이중화 CPU 장착여부, LINK1

/* MPU 상태정보.... ESIO, LINK, SIM 연계 */
#define MPU_PFR_BIT         0x80                // MPU Status : PFR 상태 [0:정상, 1:재기동]
#define MPU_DUAL_MODE       0x40                // MPU Status : 이중화 운영모드 [0:SINGLE, 1:DUAL]
#define MPU_CHG_MODE        0x20                // MPU Status : 이중화 절체모드 [0:수동, 1:자동]
#define MPU_LINK_FAIL       0x10                // MPU Status : 링크통신상태 [0:이상, 1:정상]

#define MPU_RUN_FAIL        0x08                // MPU Status : 동작이상 [0 : 정상, 1:이상]
#define MPU_SDP_A_BIT       0x04                // MPU Status : SDP-A, SDP-B 동작
#define MPU_INS_BIT         0x02                // MPU Status : 장착여부 [0 : 이상, 1: 장착]
#define MPU_ACT_BIT         0x01                // MPU Status : Master 동장모드 [0:Slave, 1: Master]

#define ESIO_PFR_BIT        0x80                // ESIO Status : PFR 상태 [0:정상, 1:재기동]
#define ESIO_TIME_SYNC      0x40                // ESIO Status : 시각동기 요청 [0:정상, 1: 요청]
#define ESIO_DB_CHK         0x20                // ESIO Status : 데이터베이스 Chksum 요청 [0:정상, 1:요청]
#define ESIO_EVENT          0x10                // ESIO Status : 이벤트 상태 [0:정상, 1:이벤트]
#define ESIO_RUN_FAIL       0x08                // ESIO Status : 동작이상 [0 : 정상, 1:이상]
#define ESIO_SDP_A_BIT      0x04                // ESIO Status : SDP-A, SDP-B 동작
#define ESIO_INS_BIT        0x02                // ESIO Status : 장착여부 [0 : 이상, 1: 장착]
#define ESIO_ACT_BIT        0x01                // ESIO Status : Master 동장모드 [0:Slave, 1: Master]


#define LINK_PFR_BIT        0x80
#define LINK_DBCHK_BIT      0x02
#define LINK_SOE_BIT        0x01


#define LED_RUN_BIT         0x01                // MPU FRONT-LED : RUN
#define LED_MASTER_BIT      0x04                // MASTER/SLAVE 보드 상태
#define LED_ACTIVE_BIT      0x08                // MASTER/SLAVE 모드 상태
#define LED_D1_BIT          0x10                // MPU FRONT-LED : D1  ICCP commMaster 일때 toggle
#define LED_D2_BIT          0x20                // MPU FRONT-LED : D2
#define LED_D3_BIT          0x40                // MPU FRONT-LED : D3
#define LED_D4_BIT          0x80                // MPU FRONT-LED : D4

#define LED_M1_BIT          0x01                // MPU FRONT-LED : M1
#define LED_S1_BIT          0x02                // MPU FRONT-LED : S1
#define LED_M2_BIT          0x04                // MPU FRONT-LED : M2
#define LED_S2_BIT          0x08                // MPU FRONT-LED : S2
#define LED_M3_BIT          0x10                // MPU FRONT-LED : M3
#define LED_S3_BIT          0x20                // MPU FRONT-LED : S3
#define LED_M4_BIT          0x40                // MPU FRONT-LED : M4
#define LED_S4_BIT          0x80                // MPU FRONT-LED : S4


#define LED_LINK_ONLINE     LED_M4_BIT
#define LED_SCU_ONLINE      LED_S4_BIT 

#define LED_ESIO_SCADA      LED_M1_BIT
#define LED_ESIO_REMOTE     LED_S1_BIT
#define LED_ESIO_ELECQ      LED_M2_BIT 
#define LED_ESIO_61850      LED_S2_BIT
#define LED_ESIO_RTU        LED_M3_BIT

#define CHANNEL_TIMEOUT_TICK    30          // 채널 Time out Tick
#define CHANGE_CPU_TICK         10          // CPU 절체 Tick

/* 제어주체별 구분 */
#define PASS_ICCP_CNTR  	0       // ICCP-관제에 의한 제어
#define PASS_HOST_CNTR      1      // HOST-관제에 의한 제어
#define PASS_SIM_CNTR       2      // 시뮬레이터에 의한 제어
#define PASS_USER_CNTR      3      // 사용자 자체 제어
#define PASS_LINK_CNTR      4      // SLAVE-CPU에 의한 제어

typedef struct
    {
        short   sec;
        short   min;
        short   hour;
        short   week;
        short   day;
        short   month;
        short   year;
        long    rtctick;
#if 0  // hkkim 2026-02-10 오후 7:45:52
    } __attribute__ ((packed)) RTC ;
#else 
    }  RTC ;
#endif 
/* ---------------------------------------------------------------------- */
/* 2026-10-02 : CLI/CLI-S2 -> ICCP layer FLOW/CFG bit control mailbox      */
/*   mi_debug_sel / mvl_debug_sel / mms_debug_sel / acse_debug_sel bits    */
/*   are set/cleared by the ICCP process in response to this mailbox.     */
/*   See design/20261002_iccp_log_control_S2.md for the full design.      */
/* ---------------------------------------------------------------------- */
#define LOGBIT_CMD_SET          1
#define LOGBIT_CMD_CLEAR_ALL    2

typedef struct
    {
        short   seq;        /* CLI/CLI-S2 increments this after filling the request */
        short   ack;        /* ICCP copies seq here after processing               */
        short   cmd;        /* LOGBIT_CMD_SET | LOGBIT_CMD_CLEAR_ALL               */
        short   value;      /* cmd==SET only: 1=on, 0=off                          */
        short   result;     /* 0=OK, -1=unknown layer, -2=bit not supported here   */
        char    layer[8];   /* cmd==SET only: "mi" | "mvl" | "mms" | "acse"        */
        char    bit[8];     /* cmd==SET only: "FLOW" | "CFG"                       */
    } ICCP_LOGBIT_CTL;


typedef struct
    {
        short   scanInitial;            // 하위 계전기부 SCAN 초기화 완료
        short   allDevOnline;           // 전체계전기 통신상태, 1 : Online, 0: Offline
        
        /* -------------------------------- */
        /*	CPU 이중화 설정 파라메터		*/
        /* -------------------------------- */
        short   dualCpuSts;             // MPU 이중화 상태
        short   cpuMode;                // MPU-A, B 상태
        short   runMode;                // MASTER, SLAVE 동작상태
        short   cpuChange;              // 이중화 상태 - 절체요청 
        short	iccpResetFlag;			// SDP 재기동 명령
        
        short	calCalcFlag;			// 연산포인트 계산주기
        short	iccpResetENB;			// ICCP 관제 : SDP 재기동 허용/금지
        short   cpuChgMode;				// SDP 이중화 절체모드
        short   rtuinit;                // RTU 초기화 상태
        
        short   wdtEnable;              // SDP : H/W WDT Clear 운영모드
        short   romDBFail;
        
        byte    stscode;
        byte    stscode1;
        
        short   max_sdpPoint;

		//short	slaveMode;				// SLAVE MODE Flag : 이중화 절체시 SDP 상태정보 SOE 발생
		
        //short   systemSOE;              // CPU 이중화 : Master 절체시 SOE 생성...
        short   chkLinkOK;              // CPU 이중화 - LINK 초기화 상태
        //short   initLinkSts;            // CPU 이중화 - LINK 초기화 상태
        //short	initLinkCount;
        
        short	cntrLinkPass;
        
        //short   esioDBChange;       
        short   iccpEnbFlag;            // ICCP-HOST 통신연계 허용(1)/금지(0)
        
        /* -------------------------------- */
        /*  HOST 운영 관련                  */
        /* -------------------------------- */
        short   useSIOBoard;                // HOST : SIO 보드 사용유무, 절체조건
        short   dnpHostEnb;                 // HOST : DNP HOST 정의시
        short   hostHarris;                 // HOST : HARRIS HOST 정의시
        //short   hostICCP;                   // HOST : ICCP HOST 정의시
        
        short   rcvONtime;                  // 상위 수신한 계전기 제어시간
        short   rcvOFFtime;                 // 상위 수신한 계전기 제어시간
        
        /* -------------------------------- */
        /* HARRIS-HOST 관련 변수....        */
        /* -------------------------------- */
        short   armPoint;
        short   armTCF;
        short   tcf;            /* trip/close flag */
        short   point;
        short   port;           /* port  number */
        short   pointState;     /* point number */
        short   cntStatus;
        
        //word    cntTime;
        //word    timeBias;
        
	    /* -------------------------------- */
	    /* DO 모듈 제어변수                 */
	    /* -------------------------------- */
	    //short   cntrState;
        //short   cntrData;
        //struct  timespec setclock;               // DO제어시...Relay 동작시간 Check
        //struct  timespec curclock;               // DO제어시...Relay 동작시간 Check
        
	    //short   cntrDevId[MAX_SCAN_PORT];
	    //short   cntrFlag[MAX_SCAN_PORT];
	    //short   cntrComTick[MAX_SCAN_PORT];     // 계전기 : 제어명령 송수신 카운트
	    
        short   stsDumpPeriod;              // 계전기 : Status Dump 주기
        short   anaDumpPeriod;              // 계전기 : Analog Dump 주기
        
        /* -------------------------------- */
        /*  EVENT 생성 및 저장 관련         */
        /* -------------------------------- */
        short   nramAccess; 
        short   eventLogging;           // EVENT 생성중 
        
        short   soeYear;                // 계전기/장비 SOE EVENT - 년
        short   soeMonth;               // 계전기/장비 SOE EVENT - 월
        short   soeDay;                 // 계전기/장비 SOE EVENT - 일
        short   soeHour;                // 계전기/장비 SOE EVENT - 시    
        short   soeMin;                 // 계전기/장비 SOE EVENT - 분
        short   soeSec;                 // 계전기/장비 SOE EVENT - 초
        short   soeMilisec;             // 계전기/장비 SOE EVENT - milisec    
        
        short	testWDTFlag;
        
        short   testDeviceFLAG;
        short   testAnalogFLAG;
        short   testStatusFLAG;

        short   testDevid;
        short   testState;
        short   testPoint;
        float   testAIData;
        
        /* SIMULATOR 용 이벤트 Queue */
        short   simStatus;                  // SIMULATOR : 상태정보
        short   rcvSimSeq;                  // SIMULATOR : 수신 Sequence No
        short   simEventNum;                // SIMULATOR : 현재 이벤트 갯수 
        
        word    simFront;                   // SIMULATOR : 이벤트 Front
        word    simRear;                    // SIMULATOR : 이벤트 Rear
        short   simOverlab;                 // SIMULATOR : 이벤트 Overlab
        short   simReport;                  // SIMULATOR : 이벤트 Report
        
        short   mpuCfgDown_OK;              // SIMULATOR : MPU-Config Down OK
        short   esioCfgDown_OK;             // SIMULATOR : ESIO-Config Down OK
        short   hostCfgDown_OK;             // SIMULATOR : HOST-Config Down OK
        short	iccpCfgDown_OK;				// SIMULATOR : ICCP-Config Down OK
        short   devCfgDown_OK;              // SIMULATOR : Device-Config Down OK
        short   modbusCfgDown_OK;           // SIMULATOR : MODBUS-Config Down OK
        short   scanCfgDown_OK;             // SIMULATOR : SCAN-Config Down OK
        short   pointCfgDown_OK;            // SIMULATOR : Point-Config Down OK
        
        short   calPt_CfgDown_OK;           // SIMULATOR : CAL Point-Config Down OK
        short   esioRestart;                // ESIO RESTART Flag
        short   hostRestart[MAX_HOST];      // HOST RESTART Flag

       	short	iccpChkTime;
       	
        //short   dbUpdate;                   // DB Update 상태 */
        
        word    scanWDT[MAX_ESIO];     // SCAN : 통신 Thread
        
        /* -------------------------------- */
        /*  RTU-Update & Time Sync 관련     */
        /* -------------------------------- */
        short   year;
        short   month;
        short   day;
        short   week;
        short   hour;
        short   min;
        short   sec;
        short   milisec;
        
        short   rtcUpdateFlag;              // TIME-Sync 요청 Flag
        short   rtcUpdateICCP;              // ICCP-TIME-Sync 요청 Flag
        short	devSoeENBTick;				// DEVICE-SOE 생성관련
        
        //struct  timeval dnpTimeVal;         // DNP-HOST 수신 timeval 
        //short   hostDnpTimeSET;             // DNP-HOST Time SET
        
        short   getClockNTP;                // NTP Server - Time Sync 
        short   cpuChgTick;
        
        short   userReset;
		
		short	logFileWrite;			// LOG 화일 저장중 Flag.... 
		                    
        /* -------------------------------- */
        /* 내부 운영 DEBUG 관련 변수        */ 
        /* -------------------------------- */       
        short   wdtDebug;                   // WDT 내부 메세지 Debug
        short   simDebug;                   // Simulator Comm Debug
        short   scuDebug;                   // 이중화절체장치 Comm Debug
        short   lineDebug;                  // ASYNC Line Comm Debug
        short   linkDebug;                  // LINK Comm Debug
        short   esioStsDebug;               // ESIO 동작상태 모니터링
        short   hostDebug;                  // HOST Comm Debug
        short   hexDebug;                   // HOST Message Debug
        short   esioDebug;                  // ESIO# Comm Debug
        short	dnpDebug;	
        short   msgDebug;
        short   calDebug;                   // CAL Point 연산 Debug
        short   mpuDebug;
        
        //short   rtuComDebug;                // RTU 관련 통신 Debug
        //short   rtuMsgDebug;                
        short   ntpDebug;
        short   checkDebug;
        
        short	devTestFlag;
        
        short   soeDebug;                   // SOE Event Debug
        short   iccpDebug;                  // ICCP-HOST Comm Debug
        short   mmsDebug;                   // ICCP-MMS Comm Debug        
        ICCP_LOGBIT_CTL logbitCtl;          // 2026-10-02 : CLI<->ICCP layer FLOW/CFG bit control mailbox
                  
// hkkim
#if 0                  
                         
                                    
    } __attribute__ ((packed)) OPR_MSG;
#else
        short   endian ;                    
                                    
    } OPR_MSG;


#endif 



#endif	// USRTYPE_HEADER_INCLUDED
