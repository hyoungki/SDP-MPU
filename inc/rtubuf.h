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

#ifndef	RTUBUF_HEADER_INCLUDED
#define	RTUBUF_HEADER_INCLUDED

//
// Include Files
//
#include "common.h"
#include "rtudb.h"
#include "op_dnp.h"
#include "op_harris.h"
#include "vmesio.h"


/* ---------------------------------------------------- */
/* STURECTURE 정의 : SCU_MSG                            */
/* - 이중화 절체장치 연계 데이터                        */
/* ---------------------------------------------------- */   
#define SCU_GPOLL           0x00
#define SCU_STATUS_DUMP     0x10
#define SCU_CONTROL         0x20
#define SCU_PFR_CLEAR       0x40

/* ------------------------------------ */
/*  HOST 통신상태 정보                  */
/* ------------------------------------ */
#define HOST_NOT_READY      0   
#define HOST_READY_STS      1               // 예비 채널 통신 (IIN-Check)
#define HOST_ACTIVE_STS     2               // 주채널 통신 (DUMP)

/* ------------------------------------ */
/*  DEVICE 포인트 Index                 */
/* ------------------------------------ */
#define INDEX_ALL_DEVICE    0               // Device Point : 전체 계전기 통신상태
#define INDEX_AUTO_MANUAL   65             // Device Point-65 : 수동/자동 상태, 0:자동, 1:수동
#define INDEX_CNTR_CHANGE   66             // Device Point-66 : 시스템 절체(제어포인트)
#define INDEX_SDP_TIMESYNC  69             // Device Point-69 : SDP TIME SYNC-REQ,  0:--, 1:요청
#define INDEX_SDP_STS       70             // Device Point-70 : SDP 동작상태, 0:정상, 1:이상  
#define INDEX_SDP_RUN_MODE  71             // Device Point-71 : SDP 동작모드, 0:SDP-A 동작, 1:SDP-B 동작  
#define INDEX_SDP_RUN_A     72             // Device Point-72 : SDP-A 동작상태, 0:정상, 1:이상  
#define INDEX_SDP_RUN_B     73             // Device Point-73 : SDP-B 동작상태, 0:정상, 1:이상  

#define INDEX_RTU_MODE      74             // Device Point-74 : RTU 동작모드, 0:SDP-A 동작, 1: SDP-B 동작
#define INDEX_RTU_RUN_A     75             // Device Point-75 : RTU#1 동작모드, 0:정상, 1:이상
#define INDEX_RTU_RUN_B     76             // Device Point-76 : RTU#2 동작모드, 0:정상, 1:이상

#define INDEX_SCADA_MODE    77             // Device Point-77 : SCADA 동작모드, 0:SDP-A 동작, 1: SDP-B 동작
#define INDEX_SCADA_RUN_A   78             // Device Point-78 : SCADA#1 동작모드, 0:정상, 1:이상
#define INDEX_SCADA_RUN_B   79             // Device Point-79 : SCADA#2 동작모드, 0:정상, 1:이상

#define INDEX_REMOTE_MODE   80             // Device Point-80 : 원격진단 동작모드, 0:SDP-A 동작, 1: SDP-B 동작
#define INDEX_REMOTE_RUN_A  81             // Device Point-81 : 원격진단 동작모드, 0:정상, 1:이상
#define INDEX_REMOTE_RUN_B  82             // Device Point-82 : 원격진단 동작모드, 0:정상, 1:이상

#define INDEX_ELECQ_MODE    83             // Device Point-83 : 전력품질 동작모드, 0:SDP-A 동작, 1: SDP-B 동작
#define INDEX_ELECQ_RUN_A   84             // Device Point-84 : 전력품질 동작모드, 0:정상, 1:이상
#define INDEX_ELECQ_RUN_B   85             // Device Point-85 : 전력품질 동작모드, 0:정상, 1:이상

#define INDEX_61850_MODE    86             // Device Point-86 : 61850 동작모드, 0:SDP-A 동작, 1: SDP-B 동작
#define INDEX_61850_RUN_A   87             // Device Point-87 : 61850 동작모드, 0:정상, 1:이상
#define INDEX_61850_RUN_B   88             // Device Point-88 : 61850 동작모드, 0:정상, 1:이상

/* ------------------------------------ */
/*  ICCP-DEVICE 포인트 Index            */
/* ------------------------------------ */
#define ICCP_SDP_TIMESYNC   69              // Device Point-69 : SDP TIME SYNC-REQ,  0:--, 1:요청


/* SDP 이중화 Flag */
#define	CHECK_SDP_NULL		0
#define	CHECK_SDP_CPU		1
#define	CHECK_SDP_FULL		2

/* ------------------------------------ */
/*  HOST 통신상태 정보                  */
/* ------------------------------------ */
#define CAL_STATUS          1               // 연산포인트 - 상태
#define CAL_ANALOG          2               // 연산포인트 - 계측
#define CAL_CONTROL         3               // 연산포인트 - 제어

#define	SINGLE_CPU_MODE		0
#define	DUAL_CPU_MODE		1

/*
* ----------------------------------------------------------
*   SCU_MSG : 이중화 절체장치(SCU) 운영 Buffer 
* ----------------------------------------------------------
*/  
typedef struct
    {
        short   status;             // SCU 상태정보 :
        short   oldsts;             // SCU 상태정보 :
        short   changeFlag;         // SCU 상태정보 :
        
        short   remoteMode;         // SCU 상태정보 : 수동/자동 모드, [0] 수동, [1] 자동
        short   chanMode;           // SCU 상태정보 : 시스템 A/B 모드, [0] A, [1] B
        
        short   sendFlag;           // SCU 송신 Flag : 전송요청
        short   address;            // SCU 송신 Address, [1] MPU-A, [2] MPU-B
        
        short   online;             // SCU 통신상태 정보
        short   sndSeqno;           // SCU 통신상태 정보
        short   rcvSeqno;           // SCU 통신상태 정보
        short   comFailTick;        // SCU 통신상태 정보
        
        short   cpuRestart;         // MPU 재기동 상태
        short   pfrReset;           // SCU 재기동 상태
        short   activeFlag;         // SCU 제어활성화 정보    
        short   cntrFlag;           // SCU 절체제어 요청
        
        short   chanSts[8];         // SCU 동작상태 : channel 절체상태
        short   chanCmd[8];         // SCU 절체명령 : channel 절체명령

    } __attribute__ ((packed)) SCU_MSG;
    

typedef struct
    {
        byte    eventCode;                              // SOE 정보구분자 , 0x13 : 계전기 Online, 0x14: 계전기 Offline, 0x20 : 상태포인트 SOE
        byte    devNo;                                  // SOE 발생 계전기 번호, [1...64]
        word    pointNo;                                // SOE 발생 포인트 번호, [1..1024]
        byte    state;                                  // SOE 발생 상태정보
        byte    esioNo;                                 // SOE 발생 ESIO 번호, [1..6]
        //byte    reserve1;
        //byte    reserve2;
        
        struct timeval	updateTime;		                // SOE 발생 시각정보, timeval
#if 0  // hkkim 2026-02-10 오전 9:46:14       
        // 2026-03-05 오후 4:24:20  ESIO 와 통신 문제로 다시 실린다.
        // 2026-03-06 오후 2:39:31  다시 
    } __attribute__ ((packed)) MPU_SOEQ_ENTRY;  
#else     
    } MPU_SOEQ_ENTRY;      
#endif     


typedef struct
    {
        byte    eventCode;                              // SOE 정보구분자 , 0x13 : 계전기 Online, 0x14: 계전기 Offline, 0x20 : 상태포인트 SOE
        byte    devNo;                                  // SOE 발생 계전기 번호, [1...64]
        word    pointNo;                                // SOE 발생 포인트 번호, [1..1024]
        byte    state;                                  // SOE 발생 상태정보
        byte    esioNo;                                 // SOE 발생 ESIO 번호, [1..6]
        //byte    reserve1;
        //byte    reserve2;
        
        struct timeval	updateTime;		                // SOE 발생 시각정보, timeval
#if 1  // hkkim 2026-02-10 오전 9:46:14       
        // 2026-03-05 오후 4:24:20  ESIO 와 통신 문제로 다시 실린다.
        // 2026-03-06 오후 2:39:31  다시 
    } __attribute__ ((packed)) MPU_SOEQ_ENTRY_PACKED;  
#else     
    } MPU_SOEQ_ENTRY_PACKED;      
#endif   


    
typedef struct
    {
        byte    front;                                  // [초기화/WRITE-61850] 61850 계전기 : SOE-Queue Front ... [0...255]
        byte    rear;                                   // [초기화/WRITE-LHS]   61850 계전기 : SOE-Queue Rear  ... [0...255]
        
        byte    overflow;                               // [초기화/WRITE-61850] 61850 계전기 : SOE Queue Overflow 상태 ... 0 : 없음, 1 : Overflow
        byte    report;                                 // [초기화/WRITE-LHS]                : SOE Queue Report 값

        MPU_SOEQ_ENTRY  Queue[256 + 2];
        
    } __attribute__ ((packed)) MPU_SOE_QUEUE, MPU_COS_QUEUE;               // [초기화/WRITE-61850] 61850 계전기 : SOE-Queue 정보
    

/*
* ----------------------------------------------------------
*   LINK_MSG : MPU 이중화 정보연계 운영 Buffer 
* ----------------------------------------------------------
*/

typedef struct
        {
        byte    cpuControl;         // 이중화 CPU 상태정보 => WRITE
        byte    cpuStatus;          // 이중화 CPU 상태정보 => READ
        byte    rcvStatus;          // 이중화 CPU 상태정보 => READ
        
        byte    sdpStatus;
        
        short   online;             // 이중화 CPU 통신상태
        
        byte    sndSeqNo;
        byte    rcvSeqNo;
        
        word    comFailTick;        // 통신상태 Check
       	
       	short   chgOpcode;          // 이중화 CPU 정보연계
        short   cmdFlag;            // 이중화 절체상태 Check
        //short   linkStatus;
        short   changeCPU;          // 이중화 절체 명령
        
        short   iccpDelSOE;         // ICCP Delete SOE Count;
        short   delReport;          
        
		/* ------------------------------------ */
        /*  이중화 CPU : 데이터베이스 연계      */
        /* ------------------------------------ */
		//short	sendFlag;           // Master : 전송요구
		
		short	timeSyncReq;                // TIME Sync 전송요구              
        short   chksumReq;                  // DB CheckSUm 전송요구
        short	dbDownFlag;                 // DB 전송상태
        
        short   dbChanged;                  // DB 변경유무
            
        short   mpuCfgDown;                 // MPU Config 전송요구
        short   esioCfgDown;                // ESIO Config 전송요구
        short   dbDownEsio;
        
        short   calptCfgDown;               // CAL-POINT Config 전송요구
        short   dbDownCalpt;    
        
        short   hostCfgDown;                // HOST Config 전송요구
        short   iccpCfgDown;                // ICCP Config 전송요구
        short   harrisCfgDown;              // HARRIS Config 전송요구
        short   landisCfgDown;              // LANDIS Config 전송요구
        short   modbusCfgDown;              // MODBUS Config 전송요구
        short   scanCfgDown;                // SCAN Config 전송요구
        short   deviceCfgDown;              // DEVICE Config 전송요구
        short	dbDownDevice;
        
        short   pointCfgDown;               // POINT Config 전송요구
          
        short	endPointFlag;               // POINT-DB 전송종료
        short	endEsioFlag;                // ESIO-DB 전송종료
        short	endDeviceFlag;              // DEVICE-DB 전송종료
        
        short	dbDownPoint;
        short	dbDownIndex;
        
        //short   dbWriteDelay;               // DB 전송후 Write Delay
        
        /* ------------------------------------ */
        /*  이중화 CPU : 상위 제어정보          */
        /* ------------------------------------ */
        short   cntDev;
        short   cntPoint;
        short   cntTCF;                  // 제어상태정보 : [1] TRIP, [2]CLOSE
        short   cntHost;
        short   rcvONtime;
        short   rcvOFFtime;
        short   cntFlag;
                                 
        /* ------------------------------------ */
        /*  TCP/IP 통신 : DEVICE 별 구조체      */
        /* ------------------------------------ */
        struct sockaddr_in	serv_addr;
        struct sockaddr_in	clientPtr;
        
        short   SocketFd;           /* for Client */
        short   SocketID;        /* for Client */
        short   connectStatus;
        short   connectFail;
                                                
        /* ------------------------------------ */
        /*  이중화 CPU : SOE Queue              */
        /* ------------------------------------ */
        byte    front;
        byte    rear;
        byte    reportSOE;
        byte    temp;
                              
        MPU_SOEQ_ENTRY    linkSOE[256+2];
                                            
        } __attribute__ ((packed)) LINK_MSG;


/*
* ----------------------------------------------------------
*   ICCP_CONFIG : ICCP HOST 세부 구성DB  
* ----------------------------------------------------------
*/

typedef struct
    {
        char    IPAddress[16];      // SDP : ICCP IP Address
        char    P_Sel[48];          // SDP : ICCP Presentation Selector, null 이면 미사용 "00 00 00 01"
        char    S_Sel[48];          // SDP : ICCP Session Selector, null 이면 미사용 "00 01"
        char    T_Sel[96];          // SDP : ICCP Transport Selector, "00 01"
        char    AP_Title[96];       // SDP : ICCP Application Process Title, null 이면 미사용 "1 3 9999 33"
        char    AE_Qualifier[11];   // SDP : ICCP Application Entity Qualifier, null 이면 미사용 "33"
    } __attribute__ ((packed)) ICCP_UNIT;
    
typedef struct
    {
    	ICCP_UNIT    FEP_A;      // FEP-A ICCP Parameter
    	ICCP_UNIT    FEP_B;      // FEP-A ICCP Parameter
    	ICCP_UNIT    SDP_A;      // SDP-A ICCP Parameter
    	ICCP_UNIT    SDP_B;      // SDP-B ICCP Parameter
    	          
    } __attribute__ ((packed)) ICCP_CONFIG;


/*
* ----------------------------------------------------------
*   MPU_CONFIG : MPU 운영 Buffer
* ----------------------------------------------------------
*/
typedef struct
    {
        byte    useFlag;            // Network# 사용유무
        byte    reserved;           // 예약
        
        char    ipAddr[16];         // Network# IP-Address
        char    gwAddr[16];         // Network# GW-Address
        char    subMask[16];        // Network# SUB-Mask

    } __attribute__ ((packed)) MPU_NET_ENTRY;


typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */  
        byte    sdpNameStr[20];         // SDP 장치이름
        MPU_NET_ENTRY   master_netCfg[4];
        MPU_NET_ENTRY   slave_netCfg[4];
        
        byte    dualMpu;                // MPU Parameter : CPU 이중화 운영모드
        byte	dualModule;             // MPU Parameter : Module 이중화 운영모드 
        byte    scuUseFlag;        	    // MPU Parameter : SCU 모듈 사용유무
        byte	mmiUseFlag;             // MPU Parameter : MMI 모듈 사용유무
        
        byte    statusDump;        	    // MPU Parameter : STATUS Dump 주기
       	byte    analogDump;        	    // MPU Parameter : ANALOG Dump 주기
        byte    debounce;               // MPU Parameter : Function 코드#1
        byte    dbCheck;                // MPU Parameter : Function 코드#2
        byte    hostWDT;                // MPU Parameter : Function 코드#3
        
        byte    iccpEnbFlag;            // ICCP-HOST 통신연계 허용(1)/금지(0)
        
        byte    func1;
        byte    func2;
        byte    func3;
        byte    func4;
        byte    func5;
        byte    func6;
        
        /* ------------------------ */
        /*  MPU 운영 Parameter      */
        /* ------------------------ */  
        byte    linkStatus;             // MPU LINK 상태정보
        
        byte    mpuStatus;              // MPU 상태정보 status, MASTER/SLAVE, SDP-A/SDP-B     
        byte    mpuRackSts;             // MPU RACK 실장정보
        byte    mpuRunSts;              // MPU 모듈 동작상태 정보
        
        byte    rcvMpuSts;              // 수신 MPU 동작상태
        byte    rcvRackSts;             // 수신 MPU RACK 실장상태
        byte    rcvRunSts;              // 수신 MPU 모듈 동작상태 정보
        
    } __attribute__ ((packed)) MPU_CONFIG;
    

/*
* ----------------------------------------------------------
*   POINT_BUF : DI/DO/AI등 CU내 포인트 운영을 위한 스트럭쳐    
* ----------------------------------------------------------
*/            
typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */     
        byte    ptNameStr[40];      // SDP POINT : 포인트 이름

        byte    devNo;              // SDP POINT : device 번호 [1..32] 
        word    devPt;              // SDP POINT : device 포인트 번호 [1..1024] 
        byte    devType;            // SDP POINT : devic TYPE      

        byte	iccpType;		    // SDP POINT : ICCP 포인트 TYPE, SDI/SDO/SAI/DDI/DAI/QDI/QAI/TDI/TAI/DEV
		word	iccpIndex;	        // SDP POINT : ICCP 포인트 인덱스, [0: 미지정, 1 ~ 1024]
        byte	iccpRes1;	        // SDP POINT : ICCP 예비
        byte	iccpRes2;	        // SDP POINT : ICCP 예비
        
        byte    ptType;             // SDP POINT : device 포인트 TYPE  */     
        byte    ptConfig;           // SDP POINT : Point Config
        
        word    pointMax;           // SDP POINT :Point MAX Scale 값
        word    pointOffset;        // SDP POINT :Point Offset Scale 값
        byte    pointDelta;         // SDP POINT :Point Delts Scale 값
        
        word    localIndex;         // SDP POINT : SLAVE-RTU 인덱스 ... 1... ~ 1024
        
     	word	modBase;			// SDP POINT :MODBUS base Address
     	word	modIndex;		    // SDP POINT :MODBUS index
             
        byte    dbport;             // SDP POINT :주장치 PORT 번호  [1..16]
        byte    dbpoint;            // SDP POINT :주장치 POINT 번호 [1..64] 

        word    hostIndex[MAX_HOST];       // SDP POINT : 상위 호스트#1 Index 번호 [1..1024]

		byte    onStr[10];      	    // SDP POINT : ON  상태 이름
		byte    offStr[10];      	// SDP POINT : OFF 상태 이름
        
        
        /* ------------------------ */
        /*  APP-프로그램 참조       */
        /* ------------------------ */
        //byte    runStatus;          // SIM : 계전기 동작상태 
        byte    config;             // device 포인트 : 포인트 사용여부 ... 
        byte    cos;
        byte    debounce;           // Point Debounce Time [ms]     
        word    cntrTime;           // Point Control Delay Time [ms]  
        
        word    dbPtIndex;          // 데이터베이스 상의 포인트 DB Index ...[0...4095]     
                   
       	/* ------------------------ */
        /* POINT 운영 정보 ...      */
        /* ------------------------ */
        byte    status;                 /* Point status */   
        byte    oldsts;                 /* 이전 상태값 */
        
        int     pointData;              /* Analog Point Data */
        float	floatData;			    /* FLOAT 데이터 */
		struct timeval	updateTime;		// 포인트 정보 Update Time
        
// hkkim
#if 0        
    } __attribute__ ((packed)) POINT_BUF;
#else 
    } POINT_BUF;
#endif 

typedef struct
    {
        byte    pointType;          // SDP : 연산포인트 TYPE,     [0]NULL, [1] STATUS, [2]ANALOG, [3]CONTROL
        byte    useFlag;            // SDP : 연산포인트 사용유무, [0]사용않함, [1] 사용
        byte    calcTime;           // SDP : 연산포인트 연산주기, sec
        byte    function;           // SDP : 연산포인트 Function#
        
        char    calString[128];     // SDP : 연산식 String

        byte    config;
        byte    devNo;              // SDP POINT : device 번호 [1..32] 
        word    devPt;              // SDP POINT : device 포인트 번호 [1..1024] 
        //byte    devType;            // SDP POINT : devic TYPE      

        //word    localIndex;         // SDP POINT : 연산포인트 인덱스 ... 1... ~ 1024                
        
        /* ------------------------ */
        /* POINT 운영 정보 ...      */
        /* ------------------------ */
        byte    status;                 /* Point status */   
        float	floatData;			    /* FLOAT 데이터 */

		struct timeval	updateTime;		// 포인트 정보 Update Time
#if 0		
    } __attribute__ ((packed)) CAL_POINT_BUF;
#else
    }  CAL_POINT_BUF;
#endif     
/*
* ----------------------------------------------------------
*   SCAN_CONFIG : SCAN 구조체    
* ----------------------------------------------------------
*/     
typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */
        byte    useFlag;                // SDP SCAN : Channel 사용유무
        byte	targetID;               // SDP SCAN : ESIO Target Module-ID, 0:사용않함, 1: MPU, 2: ESIO1, 3:ESIO2, 4:ESIO3, 5:ESIO4, 6:SIO
        
        byte    protocol;               // SDP SCAN : SCAN 통신 프로토콜 
        byte    comMode;                // SDP SCAN : SCAN 통신모드
        byte	comPort;			    // SDP SCAN : SCAN 통신포트
        byte    comSpeed;               // SDP SCAN : SCAN 통신속도

        byte	comDelay;			    // SDP SCAN : SCAN 통신간격
        byte	offCount;			    // SDP SCAN : SCAN Offline Count
        byte    chgMode;                // SDP SCAN : SCAN Change Mode
        
        byte    function1;              // SDP SCAN : SCAN Function#1
        byte    function2;              // SDP SCAN : SCAN Function#2
        byte    function3;              // SDP SCAN : SCAN Function#3
        
        byte	scanNameStr[20];		// SDP SCAN : SCAN Name String
        
        /* ------------------------ */
        /*  APP-프로그램 참조       */
        /* ------------------------ */
        short   scanid;                 // SDP SCAN : SCAN 구조체 id
        short   scanIndex;              // SDP SCAN : SCAN 할당 계전기 수
        short   scanDevice[MAX_DEVICE]; // SDP SCAN : SCAN 할당 계전기 LIST
        
        //short   devNum;                 // SDP SCAN : SCAN 할당 계전기 수
        //short   device[MAX_DEVICE];     // SDP SCAN : SCAN 할당 계전기 LIST
        
        short   cntrDevID;              // SDP SCAN : SCAN 제어대상 계전기 번호
	    short   cntrFlag;               // SDP SCAN : SCAN 제어 요청
	    short   cntrComTick;            // SDP SCAN : SCAN 제어 Timeout
	    
        /* ------------------------ */
        /* SCAN 통신 Buffer         */
        /* ------------------------ */
        //byte    sndBuffer[256];
        //byte    rcvBuffer[512];
                            
    } __attribute__ ((packed)) SCAN_CONFIG;
        

/*
* ----------------------------------------------------------
*   MODBUS_READ/WRITE : MODBUS Protocol 구성정보     
* ----------------------------------------------------------
*/     
typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */
        byte    useFlag;           	// READ Block : 사용여부 
        byte    opcode;           	// READ Block : OPCODE 
        word    baseAddr;   		// READ Block : BASE Address 
        word    baseOffset;   		// READ Block : Offset Address
		word	dataNum;			// READ Block : Read Point Number
		byte	dataType;			// READ Block : Read Data Type
		byte	dataFormat;			// READ Block : Data Format 
		byte	swepEnb;            // READ Block : SWEP 지정
		byte    function;           // READ Block : Function#

		word	commIndex;          /* READ-Block 별 통신회수 제어 */
		
    } __attribute__ ((packed)) MODBUS_READ;
    
typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */
        byte    useFlag;           	// WRITE Block : 사용여부 
        byte    opcode;           	// WRITE Block : OPCODE 
        word    baseAddr;   		// WRITE Block : BASE Address 
        word    baseOffset;   		// WRITE Block : Offset Address
		word	dataNum;			// WRITE Block : Read Point Number
		byte	dataType;			// WRITE Block : Read Data Type
		byte	dataFormat;			// WRITE Block : Data Format
		word	tripData;           // WRITE Block : TRIP Control Data
		word	closeData;          // WRITE Block : CLOSE Control Data

    } __attribute__ ((packed)) MODBUS_WRITE;    

typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */
        byte    useFlag;           	// TIME Block : 사용여부 
        byte    opcode;           	// TIME Block : OPCODE 
        word    baseAddr;   		// TIME Block : BASE Address 
        word    baseOffset;   		// TIME Block : Offset Address
		word	dataNum;			// TIME Block : Read Point Number
		byte	dataType;			// TIME Block : Read Data Type
		byte	dataFormat;			// TIME Block : Data Format
		word	reserved;           // TIME Block : TRIP Control Data

    } __attribute__ ((packed)) MODBUS_TIME;    


/*
* ----------------------------------------------------------
*   SDP_DEVICE : 계전기/장치별 운영 Buffer     
* ----------------------------------------------------------
*/   
typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */
        byte	devNameStr[40];		    // SDP DEVICE : 계전기 Name String
        byte    scan;                   // SDP DEVICE : 계전기 사용유무
        byte    comDevID;               // SDP DEVICE : 기능모듈내 계전기 ID 
        byte    comDevIndex;            // SDP DEVICE : 기능모듈내 계전기 Index 
        byte    scanPort;               // SDP DEVICE : 계전기 통신 포트
        byte    type;           	    // SDP DEVICE : 계전기 TYPE
		byte	dualENB;			    // SDP DEVICE : 계전기 - 계전기 이중화 여부
		byte	modbusFileNo;		    // SDP DEVICE : 계전기 - MODBUS 프로파일
		
		word    devDiPoint;             // SDP DEVICE : 장치별 DI 포인트 수 
        word    devDoPoint;             // SDP DEVICE : 장치별 DO 포인트 수 
        word    devAiPoint;             // SDP DEVICE : 장치별 AI 포인트 수 
        word    devAoPoint;             // SDP DEVICE : 장치별 AO 포인트 수 
        word    devCntPoint;            // SDP DEVICE : 장치별 Binary Counter 포인트 수
        
		byte	ipString1[16];		    // SDP DEVICE : HOST IP Address
		byte	ipString2[16];		    // SDP DEVICE : HOST IP Address
        word    netPort;			    // SDP DEVICE : HOST TCPIP Port번호 */
        byte    function1;              // SDP DEVICE : SCAN Function#1
        
        /* ------------------------ */
        /*  APP-프로그램 참조       */
        /* ------------------------ */
        short   ioid;                   // SDP DEVICE : 계전기 ID 
        short   hostid;                 // SDP DEVICE : CU ID
        
        short   targetID;               // ESIO Target ID, 0:사용않함, 1: SIO, 2:ESIO1, 3:ESIO2, 4:ESIO3, 5:ESIO4, 6:RTU, 7:MPU1, 8:MPU2
        //short   slaveRTU;               // SDP DEVICE : SLAVE-RTU 해당 계전기 여부
        
        short   runStatus;              // SIM : 계전기 동작상태 
        byte    rcvIIN[2];              // SDP DEVICE : 계전기 DNP 상태정보    
        
        short   init_status;            // SDP DEVICE : DEVICE 상태정보 초기화
        short   init_analog;            // SDP DEVICE : DEVICE 계측정보 초기화 
        short   online;                 // SDP DEVICE : DEVICE Online 상태
        
        short	offlineTick;            // SDP DEVICE : DEVICE Offline Count Tick
        short	offCount;			    // SDP DEVICE : 통신 Offline 기준
        short   timeSyncReq;            // SDP DEVICE : TIME SYNC Flag
        
        //short	simDumpInit;			// SIM : 계전기 포인트 상태 Dump Flag
        //short	reportDIpt;
        //short	reportAIpt;
        short	regDiPointNum;			// 계전기 등록 DI 포인트 수
        short	regAiPointNum;			// 계전기 등록 AI 포인트 수
        
        word    comSndCount;            // SDP DEVICE : 계전기 통신 - Send Count
        word    comRcvCount;            // SDP DEVICE : 계전기 통신 - Recv Count
        word    oldRcvCount;            // SDP DEVICE : 계전기 통신 - 이전 Recv Count
        
        short   statusDump;		        // SDP DEVICE : Status Input Dump Flag
        short   analogDump;		        // SDP DEVICE : Analog Input Dump Flag
		
        short   selectReq;              // SDP DEVICE : SELECT 제어요청
        short   operateReq;             // SDP DEVICE : OPERATE 제어요청
        short   cntPoint;               // SDP DEVICE : 제어 포인트 [0...]
        short   cntTCF;                 // SDP DEVICE : 제어 상태, [1] TRIP, [2]CLOSE
        short   cntrType;               // SDP DEVICE : 계전기 제어방식 : Pulse/Latch ... 
        
        //byte    vmeSoeRear;             /* VMEBUS : SOE 정보 Rear 포인터 */
        short   stsByteCnt;             /* CPU 이중화연계 : 상태정보 수 */
        
        byte    cursts[MAX_DI_DATA_NUM];      /* DEVICE 별 포인트 정보 : 상태포인트 Current */
        //byte    df[MAX_DI_DATA_NUM];          /* DEVICE 별 포인트 정보 : 상태포인트 Old */
        //byte    linkSts[MAX_DI_DATA_NUM];     /* Slave 전송용 상태 데이터용 버퍼 */
        
        POINT_BUF       diPtBuf[MAX_DEV_DI_POINT];      // 전자식배전반 DI 상태포인트  1024
	    POINT_BUF       doPtBuf[MAX_DEV_DO_POINT];      // 전자식배전반 DO 상태포인트
	    POINT_BUF       aiPtBuf[MAX_DEV_AI_POINT];      // 전자식배전반 AI 상태포인트
	    //POINT_BUF       aoPtBuf[MAX_DEV_AO_POINT];      // 전자식배전반 AO 포인트
	    //POINT_BUF       countBuf[MAX_DEV_COUNT_POINT];  // 전자식배전반 AO 포인트

#if 0	    
	    /* ------------------------ */
        /* 지멘스(TCPIP) 이중화 관련*/
        /*  - MODBUS 프로토콜 관련  */
        /* ------------------------ */
        short   activeMode;             // 계전기 통신 Active Flag... 
        short   activeFlag;             // 계전기 통신 Active Flag... 
        short   activeTick;
        short	activeChan;
#endif

#if 0 // hkkim 2026-02-07 오후 4:59:23
        
    } __attribute__ ((packed)) SDP_DEVICE;
#else
    } SDP_DEVICE;
#endif 


/*
* ----------------------------------------------------------
*   HOST_DCB : HOST_DCB, HOST 운영  Buffer
* ----------------------------------------------------------
*/   
typedef struct
        {
        /* ------------------------ */
        /* POINT Config 정보 ...    */
        /* ------------------------ */
        byte    devNo;              // 계전기 번호, 1..64
        word    devPt;              // 계전기내 포인트 번호, 1...
        byte    type;               // 포인트 Type
        byte    config;             // 포인트 Config
        
        byte    port;               // HARRIS Port
        byte    point;              // HARRIS Point
        
        byte    cntrConfig;         // 제어방식 : Pulse/Latch...
        word    dbmax;              // 제어 인덱스 
        } __attribute__ ((packed)) DOPOINT_INFO;

typedef struct
        {
        /* ------------------------ */
        /* POINT Config 정보 ...    */
        /* ------------------------ */
        byte    devNo;              // 계전기 번호, 1..64
        word    devPt;              // 계전기내 포인트 번호, 1...
        byte    type;               // 포인트 Type
        byte    config;             // 포인트 Config
        
        //byte    multiPoint;
                        
        } __attribute__ ((packed)) DIPOINT_INFO;

typedef struct
        {
        /* ------------------------ */
        /* POINT Config 정보 ...    */
        /* ------------------------ */
        byte    devNo;              // 계전기 번호, 1..64
        word    devPt;              // 계전기내 포인트 번호, 1...
        byte    type;               // 포인트 Type
        byte    config;             // 포인트 Config

        } __attribute__ ((packed)) AIPOINT_INFO;

typedef struct
        {
        /* ------------------------ */
        /* POINT Config 정보 ...    */
        /* ------------------------ */
        byte    devNo;
        word    devPt;
        byte    type;
        byte    config;

		//word	iecIndex;			// IEC HOST의 경우...                        
        } __attribute__ ((packed)) AOPOINT_INFO;        

typedef struct
        {
        /* ------------------------ */
        /* POINT Config 정보 ...    */
        /* ------------------------ */
        byte    devNo;
        word    devPt;
        word    dnpPoint;
        byte    config;

        byte    dbport1;            /* device 포인트 : HARRIS-HOST #1 Port  Index */    
        byte    dbpoint1;           /* device 포인트 : HARRIS-HOST #1 Point Index */ 

        byte    dbport2;            /* device 포인트 : HARRIS-HOST #1 Port  Index */    
        byte    dbpoint2;           /* device 포인트 : HARRIS-HOST #1 Point Index */         
        
        //word    landisPoint;
        //word	iecIndex;			// IEC HOST의 경우...               
        
        byte    nameStr[40];
        
        } __attribute__ ((packed)) DEVPOINT_INFO;        
                        
typedef struct
        {
        /* ------------------------ */
        /* POINT Config 정보 ...    */
        /* ------------------------ */
        byte    devNo;
        word    devPt;
        byte    port;           // HARRIS Port
        byte    point;          // HARRIS Point
        byte    type;           // 포인트 타입 : 계전기 또는 시스템   
        byte    config;         // 제어포인트 설정여부
        
        byte    cntrConfig;     // 제어방식 : Pulse/Latch...
        
        word    dbmax;          // 제어 인덱스 
        
        //word	iecIndex;			// IEC HOST의 경우...                        
        } __attribute__ ((packed)) CONTROL_INFO;
                                

typedef struct
    {
        byte    netPort;            // Network# 사용 Port, [0] NET1 ~[7] NET8
        char    ipAddr[16];         // Network# IP-Address

    } __attribute__ ((packed)) HOST_NET_ENTRY;

    
typedef struct
    {                 
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */
        short   hostDualMode;           // HOST Config : HOST 운영모드 : [0]사용안함, [1]개별, [2]이중화 
        short   hostProtocol;           // HOST Config : HOST 통신 프로토콜 : HARRIS/LANDIS/DNP/MODBUS/IEC...
        short   hostComType;            // HOST Config : HOST 통신모드 : RS232/MODEM/RS485/TCPIP
        short   hostComSpeed;           // HOST Config : HOST 통신속도 
        short   masterChan;             // HOST Config : HOST MASTER 통신포트
        short   slaveChan;              // HOST Config : HOST SLAVE  통신포트
        
        short   vmeMstChan;             // HOST Config : VME-SIO Channel Index
        short   vmeSlvChan;             // HOST Config : VME-SIO Channel Index

        word	hostid;		            // HOST Config : HOST DNP - 센터 Address
        word	rtuAddr;			    // HOST Config : HOST DNP - RTU Address

        byte	hostNameStr[20];		// HOST Config : HOST Name String
        short   timeSyncDISB;            // HOST SOE 연계
        word    tcpPort;
        
        HOST_NET_ENTRY  masterNetCfg[2];    // HOST Config : HOST 주장치 Network Config
        HOST_NET_ENTRY  slaveNetCfg[2];     // HOST Config : HOST 부장치 Network Config
        
        short    soeClass;               // HOST Config : SOE Class 지정 
        short    cosClass;               // HOST Config : COS Class 지정 
        short    coaClass;               // HOST Config : COA Class 지정 
        short    unsolEvent;             // HOST Config : Unsolite 지정  
        short   unsolMode;
        short    comDelay;               // HOST Config : HOST 통신 지연 (10ms)
        short    offCount;               // HOST Config : HOST 통신 Offline Count
        short    chgMode;                // HOST Config : HOST Change Mode;
        short	reserved1;			// HOST TIME-SYNC 허용/금지	
        short	reserved2;
        
        /* ------------------------ */
        /*  APP-프로그램 참조       */
        /* ------------------------ */
        short   sdpRestart; 
        short   resetCount;
        
        short   initial;
        short   sndFlag;
        
        short   id;                                     // HOST# 구분자, 1,2,3..         
        short   runStatus;                              // SIM : HOST 동작상태 
        short   online[2];                              // HOST별 통신상태, Master/Slave
        short   comFailTick[2];                         // HOST별 통신상태, Master/Slave
        short   taskActivate;                           // HOST-THREAD 실행상태
        short   activePort;                             // HOST Active PORT
        short   hostActive;                             // HOST 동작상태, Active, Ready ..
        
        byte    cosReport;                              // HOST : COS Report Num
        byte    coaReport;                              // HOST : COA Report Num
        byte    soeReport;                              // HOST : SOE Report Num
        byte    soeRear;                                // HOST : SOE Current Rear-Index
        byte    coaRear;                                // HOST : COA Current Rear-Index
        byte    cosRear;                                // HOST : COS Current Rear-Index
        
        /* ------------------------ */
        /*  DNP-HOST 참조           */
        /* ------------------------ */
        short   rcvAC;  
        short   rcvTHseq;  
        short   rcvACseq;  
        short   rcvACfunc;      
        short   rcvObj;
        short   rcvVar;
        short   rcvIndex;
        short   rcvQcode;

        word    rcvStart;
        word    rcvStop;
        word    rcvQnum;
        short   rcvDataPos;
        
        /*  Control Inform */
        short   selectReq;
        short   armPoint;
        short   armTCF;
        short   armONTime;
        short   armOFFTime;
        
        short   sndLinkSts;                             // DNP : LINK Reset 상태
        short   rcvLinkSts;                             // DNP : LINK Reset 상태     
        short   sndFcbBit;                              // DNP : snd FCB Bit
        short   sndFcvBit;                              // DNP : snd FCV Bit
        short   rcvNextFCV;                             // DNP : rcv Next FCV Bit
        short   rcvNextFCB;                             // DNP : rcv Next FCB Bit
        byte    rtuIIN[2];                              // DNP : Report IIN 상태
        
        short   iinRcvTick;                             // DNP : 예비장치 IIN Read
                
        short   sndTHseqno;                             // DNP : TH-Header Seq no
        short   rcvEndOk;                               // DNP : Packet 수신상태
        short   sndACseq;                               // DNP : UN-Solict 응답 Sequence No
        
        DNP_APP_FRAME   rcvAppFrame[2];
        DNP_APP_FRAME   sndAppFrame[2];
        DNP_DATA_LINK   dataLinkFrame[2];
        
        //DNP_APP_FRAME   sndUnsolFrame[2];
        
        /* ------------------------ */
        /*  HARRIS/LANDIS-HOST 참조 */
        /* ------------------------ */
        short   rtuIndex;
        short   rtuid;
        short   rcvPACKET;
        short   noCommand;
        byte    txcnt;
        byte    rxcnt;

        //byte    sndBuffer[256];
        //byte    rcvBuffer[256];
        
        /* ---------------------------- */
        /*  HOST 포인트-TYPE 참조정보   */
        /* ---------------------------- */
        short   diPtNum;                                // HOST 상태(DI)포인트 수
        short   doPtNum;                                // HOST 제어(DO)포인트 수
        short   aiPtNum;                                // HOST 감시(AI)포인트 수
        short   aoPtNum;                                // HOST 설정(AO)포인트 수
        short   devPtNum;                               // HOST 장치(DEV)포인트 수
        
        short   diIndexWord;                            // DI Point 최대 수 : 256보다 큰경우 WORD 처리 
        short   aiIndexWord;                            // AI Point 최대 수 : 256보다 큰경우 WORD 처리 
        
        
        // 이 INFO 들은 dev를 찾기 위한 것이고
        CONTROL_INFO    controlInfo[MAX_DNP_DO_POINT];  // HOST 상태제어(DO) 포인트 참조
        DIPOINT_INFO    stateInfo[MAX_DNP_DI_POINT];    // HOST 상태감시(DI) 포인트 참조
        AIPOINT_INFO    analogInfo[MAX_DNP_AI_POINT];   // HOST 계측감시(AI) 포인트 참조
        //AOPOINT_INFO    aoutInfo[MAX_DNP_AO_POINT];     // HOST 계측설정(AO) 포인트 참조
        //DEVPOINT_INFO   deviceInfo[MAX_DEV_POINT];      // HOST 장치상태(DEV) 포인트 참조
        
       	DNP_SOE_QUEUE   dnpSOEQ;                        // HOST 별 SOE Queue...
        DNP_COS_QUEUE   dnpCOSQ;                        // HOST 별 COS Queue...
        
        /* HARRIS-HOST 데이터 구조 */
       	//RTU             rtubuf[MAX_HARRIS_RTU];         /* HARRIS RTU Structure */
		//PORT_DB         portdb[MAX_HARRIS_PORT];        /* HARRIS #1 PORT Structure */

		CONTROL_INFO    harrisCntr[MAX_DNP_DO_POINT];
		//DEVPOINT_INFO   harrisInfo[MAX_DEV_POINT];
		        
        /* DNP, LANDIS-HOST 데이터 구조 */
        byte            sts_pointData[MAX_DNP_DI_POINT];        // HOST 별 상태포인트 영역 
        //byte            multi_point[MAX_DNP_DI_POINT];          // HOST 별 MULTI-POINT 영역 
        
        DNP_ANA_INPUT   ana_pointData[MAX_DNP_AI_POINT];        // HOST 별 계측포인트 영역 
        
        /* ------------------------------------ */
		/* HOST 통신포트 정의(Master/Slave)     */
		/* ------------------------------------ */
        VME_CHAN_DCB    *vmeChan[2];                    // VME Channel 포인터 지정
        TTY_DESC        *ttyPort[2];                    // ASYNC 통신포트 지정
        
        /* ------------------------------------ */
		/* HOST TCPIP-SERVER 정의(Master/Slave) */
		/* ------------------------------------ */
        short   acceptFailCnt;                          // HOST 접속이상 Count...
        
        int     socketServerFd;                         // TCPIP- for Server Socket
        struct  sockaddr_in	srvAddr;                    // TCPIP- for Server Socket
        //struct  sockaddr_in	clientPtr;
        
        int     threadActive[2];                        // TCPIP-Thread 동작상태
                
        /* 접속대상 PORT 관리 */
        int     portResetCount;
        int     acceptPORT[2];
#if 0 // hkkim 2026-02-11 오후 2:00:16        
    } __attribute__ ((packed)) HOST_DCB;
#else
    }  HOST_DCB;
#endif 

/*
* ----------------------------------------------------------
*   ESIO_CONFIG : ESIO#1~#5 운영 Buffer
* ----------------------------------------------------------
*/
        
typedef struct
    {
        byte    useFlag;            // Network# 사용유무
        char    ipAddr[16];         // Network# IP-Address
        char    gwAddr[16];         // Network# GW-Address
        char    subMask[16];        // Network# SUB-Mask

    } __attribute__ ((packed)) ESIO_NET_ENTRY;

typedef struct
    {
        byte    useFlag;            // PORT# 사용유무
        byte    function;           // PORT# Function
        byte    comMode;            // PORT# 통신모드, 0:RS232, 1:MODEM, 2:RS485, 3:TCPIP
        byte    comSpeed;           // PORT# 통신속도, [0] 1200 ~ [7] 115200
        
        byte    portNameStr[20];    // PORT# 포트이름

    } __attribute__ ((packed)) ESIO_PORT_ENTRY;


// 2026-05-14 오후 4:15:07   ESIO ARM 인 경우 network 이 4개가 기본.
#ifdef __ARM_ARCH__
    #define  ESIO_NETWORK_NUMBER  4
#else
    #define  ESIO_NETWORK_NUMBER  3
#endif 




typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */
        byte    useFlag;                // ESIO Parameter : ESIO 사용유무
        byte	targetID;               // ESIO Parameter : ESIO Target Module-ID, 0:사용않함, 1: ESIO1, 2:ESIO2, 3:ESIO3, 4:ESIO4, 5:SIO
        byte    autoChgFlag;            // ESIO Parameter : 자동절체 Flag
        byte    comDelay;               // ESIO Parameter : MPU 통신 지연 (10ms)
        
        byte    esioNameStr[20];        // ESIO 장치이름
        
        ESIO_NET_ENTRY   mstNetConfig[ ESIO_NETWORK_NUMBER ];
        ESIO_NET_ENTRY   slvNetConfig[ ESIO_NETWORK_NUMBER ];
        
        ESIO_PORT_ENTRY portConfig[MAX_ESIO_PORT];  // ESIO Parameter : PORT Config
        
        /* ------------------------ */
        /*  APP-프로그램 참조       */
        /* ------------------------ */
        short   online;
        short   scanDevNo;                  // SCAN Device 번호
        short   chgOpcode;
        short   sndOpcode;                  // ESIO 전송 Opcode        
        byte    mpuStatus;                  // MPU 전송 상태정보
        byte    rcvStatus;                  // ESIO# 수신 상태정보 => READ
        
        byte    sndSeqNo;                   // ESIO Packet 송신 Sequence No
        byte    rcvSeqNo;                   // ESIO Packet 수신 Sequence No
       
        word    sndCount;
        word    rcvCount;
        word    preRcvCount;
       
        short   rackInstall;                // ESIO 장착여부, 0: 미장착, 1: 장착
        short   sioRunFail;                 // ESIO 동작상태, 0: 정상,   1: 동작이상
                
        short   scanIndex;                  // ESIO 계전기 Index
        short   scanMaxNum;                 // ESIO 등록된 계전기 수
        short   scanDevice[MAX_DEVICE];     // ESIO 계전기 리스트
        
        /* ------------------------------------ */
        /*  ESIO 할당 계전기 : 제어정보 연계    */
        /* ------------------------------------ */
        short   cntFlag;                    // 제어 Flag
        short   cntDev;                     // 계전기 번호 [1..64]        
        short   cntPoint;                   // 계전기 포인트 [1..256]            
        short   cntTCF;                     // 제어상태, [1] TRIP, [2] CLOSE   
        
        /* ------------------------------------ */
        /*  이중화 CPU : 데이터베이스 연계      */
        /* ------------------------------------ */
		//short	sendFlag;           // Master : 전송요구
		
		short	timeSyncReq;                // TIME Sync 전송요구              
        short   chksumReq;                  // DB CheckSUm 전송요구
        short	dbDownFlag;                 // DB 전송상태
        
        short   dbChanged;                  // DB 변경유무
            
        short   mpuCfgDown;                 // MPU Config 전송요구
        short   esioCfgDown;                // ESIO Config 전송요구
        short   dbDownEsio;
        
        short   hostCfgDown;                // HOST Config 전송요구
        short   iccpCfgDown;                // ICCP Config 전송요구
        short   harrisCfgDown;              // HARRIS Config 전송요구
        short   landisCfgDown;              // LANDIS Config 전송요구
        short   modbusCfgDown;              // MODBUS Config 전송요구
        short   scanCfgDown;                // SCAN Config 전송요구
        short   deviceCfgDown;              // DEVICE Config 전송요구
        short	dbDownDevice;
        short   calptCfgDown;               // CAL-POINT Config 전송요구
        short   pointCfgDown;               // POINT Config 전송요구
        
        short   dbDownCalpt;   
          
        short	endPointFlag;               // POINT-DB 전송종료
        short	endEsioFlag;                // ESIO-DB 전송종료
        short	endDeviceFlag;              // DEVICE-DB 전송종료
        
        short	dbDownPoint;
        short	dbDownIndex;

        /* ------------------------------------ */
        /*  TCP/IP 통신 : DEVICE 별 구조체      */
        /* ------------------------------------ */
        short   vmeNetConfig;               // ESIO Config 정보 => VME
        short   netCfgCount;
        
        struct sockaddr_in	serv_addr;
        struct sockaddr_in	clientPtr;
        
        short   SocketFd;           /* for Client */
        short   SocketID;           /* for Server */
        short   connectStatus;
        short   connectFail;
        short   comFailTick;
        
        
        char    targetAddr[32];
        
    } __attribute__ ((packed)) ESIO_CONFIG;
        

/* ============================================================================== */
/* ============================================================================== */

typedef struct
{
	int		cntrFlag;		// ICCP Control Flag 표시, 0: 제어없음, 1 : 제어 있음

	int		cntrType;		// 0: Point 제어, 1: MPU 절체

	int     cntrDev;        // ICCP Control 계전기 번호 ,           1 ~ 64
	int     cntrPoint;      // ICCP Control 계전기-포인트 번호 ,    1 ~ 1024
	int     cntrTime;       // ICCP Control Time ,    1 ~ 2000 ms - not used
	short   cntrState;      // ICCP Control 상태 ,    1 : TRIP, 2: CLOSE

	int		errorFlag;		// ICCP Control 실패표시, 0: 정상, 1 : 제어실패
	
} __attribute__ ((packed)) ICCP_CONTROL_DATA;

typedef struct
{
	int				flag;			// 0:online, 1:offline
	float			value;			// 포인트 정보 : 상태/계측 포인트 정보
	struct timeval	updateTime;		// 포인트 정보 Update Time
} __attribute__ ((packed)) ICCP_POINT_DATA;


typedef struct
{
    /* ------------------------ */
    /* POINT Config 정보 ...    */
    /* ------------------------ */
    byte    config;             // 포인트 사용유무,  [0] : 미지정, [1] : 사용
    byte    devNo;              // 대상 계전기 번호 : 1 ~ 64
    word    devPt;              // 대상 계전기내 포인트 번호 : 1 ~ 1024

	word	index;				// 나의 ICCP_POINT_INFO 인덱스 [0 ~ MAX_ICCP_POINT-1]
	word	dbIndex;			// rtuDatabase.pointBuf 인덱스 [0 ~ MAX_DBASE_POINT(4096)-1]

	byte	soe;				// SOE Point Type (SOEPT | ALLPT)
	byte	iccpPointType;		// SDP POINT : ICCP 포인트 TYPE, SDI/SDO/SAI/DDI/DAI/QDI/QAI/TDI/TAI/DEV
    word    iccpPointIndex;	    // SDP POINT : ICCP 포인트 인덱스, [0: 미지정, 1 ~ 1024]
	
	char	iccpName[16];		// ICCP Data Name : set by ICCP task

	/* ------------------------ */
	/* 포인트 운영 정보 ...     */
	/* ------------------------ */
	//ICCP_POINT_DATA		data;
	int		reserve[2];
} __attribute__ ((packed)) ICCP_POINT_INFO;

            
typedef struct
{
	word				numType[MAX_SDP_POINT_TYPE];	// ICCP 포인트 TYPE별 등록 포인트 수
	word				maxIndex[MAX_SDP_POINT_TYPE];	// ICCP 포인트 TYPE별 포인트 인덱스 최재값
	word				numPoint;						// ICCP 포인트 수
	ICCP_POINT_INFO		point[MAX_ICCP_POINT];
	word				mapTable[MAX_DBASE_POINT];		// soe 받았을때, DB index를 ICCP point index(1~)로 바꾸기 위한 mapping table
} __attribute__ ((packed)) ICCP_POINT_DEF;

            
typedef struct
{
	short				index;			// rtuDatabase.pointBuf 인덱스 [0 ~ MAX_DBASE_POINT(4096)-1]
	byte				sending;		// ICCP 타스크가 이벤트 전송시 설정, 전송 완료 후 삭제된다.
	byte				reserved;
	ICCP_POINT_DATA		data;
} __attribute__ ((packed)) ICCP_SOEQ_ENTRY;

/* ------------------------------------ */
/*  ICCP : SOE Queue                    */
/* ------------------------------------ */    
#define ICCP_SOEQ_MAX	256
typedef struct
{
	short				count;
	ICCP_SOEQ_ENTRY		queue[ICCP_SOEQ_MAX];
} __attribute__ ((packed)) ICCP_SOE_QUEUE;  

/* ------------------------------------ */
/*  ICCP : SOE Delete Queue             */
/* ------------------------------------ */    
typedef struct
{
	byte    front;
	byte    rear;
	ICCP_SOEQ_ENTRY		queue[ICCP_SOEQ_MAX];
} __attribute__ ((packed)) ICCP_SOE_DELETE_QUEUE;  

/* ------------------------------------ */
/*  ICCP : COS Queue                    */
/* ------------------------------------ */    
//#define ICCP_COSQ_MAX	256
//typedef struct
//{
//	short				count;
//	ICCP_COSQ_ENTRY		queue[ICCP_COSQ_MAX];
//} __attribute__ ((packed)) ICCP_COS_QUEUE;  

/* ------------------------------------ */
/*  ICCP : COS Delete Queue             */
/* ------------------------------------ */    
//typedef struct
//{
//	byte    front;
//	byte    rear;
//	ICCP_COSQ_ENTRY		queue[ICCP_COSQ_MAX];
//} __attribute__ ((packed)) ICCP_COS_DELETE_QUEUE;  

/* ================================================================ */
/*  ICCP-HOST 용 운영 버퍼 Structure                                */
/* ================================================================ */
typedef struct
{   
	ICCP_CONFIG     config;                 // ICCP-HOST 구성 DB

	short			commMaster;				/* comm mode - 0:slave, 1:master
												slave 인 경우, Identify 명령만 주고 받는다.
												master인 경우, data및 SOE를 주며,
												StartDsTs 명령이 온경우 master가 되며,
												StopDsTs  명령이 온경우 slave 가 된다. */
	short			assocStatus;			/* association status - 0:inactive, 1:active */
	short			activeArIndex;			/* active remote ar index - 0:FEP_A, 1:FEP_B */
    unsigned short  inactiveCount;          /* association inactive counter */
    time_t          lastActiveTime;			/* last association active time */
    time_t          lastInactiveTime;       /* last association inactive time */
    time_t          lastDataSendTime;		/* last server variable data send time */
	unsigned short	identifyRecvCount;      // 접속 된 후, Identify 명령 받은 수

	ICCP_POINT_DEF			pointDef;		// ICCP-HOST 운영 포인트 참조

    ICCP_CONTROL_DATA       cntrInfo;       // ICCP-HOST 제어정보 참조 
	ICCP_SOE_QUEUE			soeQueue;
	ICCP_SOE_DELETE_QUEUE	soeDeleteQueue;

	//ICCP_COS_QUEUE			cosQueue;
	//ICCP_COS_DELETE_QUEUE	cosDeleteQueue;

	byte			reserve[20];

} __attribute__ ((packed)) ICCP_DCB;    

/*
*   ICCP POINT-DATA 참조용
*/
typedef struct
{
    short       devNo;          // SDP POINT : device 번호 [1..32] 
    short       devPt;          // SDP POINT : device 포인트 번호 [1..1024] 
    short       devType;        // SDP POINT : devic TYPE      
        
    short       config;         // Device Config Type, [0]미정의, [1] 정의
    byte        ptNameStr[40];  // SDP POINT : 포인트 이름
    
    word        accessTick;     // ICCP Report Tick     
            
	short		offline;		// 0:online, 1:offline
    short       flag;
	float		value;			// 포인트 정보 : 상태/계측 포인트 정보
	float       report;         // 포인트 정보 : 상태/계측 포인트 정보
	
	struct timeval	updateTime;		// 포인트 정보 Update Time
	
} __attribute__ ((packed)) I60870_DATA;


typedef struct
{   
    int     maxIndex_sdi;
    int     maxIndex_sdo;
    int     maxIndex_sai;
    int     maxIndex_ddi;
    int     maxIndex_dai;
    int     maxIndex_qdi;
    int     maxIndex_qai;
    int     maxIndex_tdi;
    int     maxIndex_tai;
    int     maxIndex_dev;
    
    I60870_DATA     iccp_sdi[MAX_SDP_POINT_SDI];    // ICCP HOST 운영 포인트 : SCADA Digital Input
    I60870_DATA     iccp_sdo[MAX_SDP_POINT_SDO];    // ICCP HOST 운영 포인트 : SCADA Digital Control
    I60870_DATA     iccp_sai[MAX_SDP_POINT_SAI];    // ICCP HOST 운영 포인트 : SCADA Digital Output
    
    I60870_DATA     iccp_ddi[MAX_SDP_POINT_DDI];    // ICCP HOST 운영 포인트 : 원격진단 Digital Input
    I60870_DATA     iccp_dai[MAX_SDP_POINT_DAI];    // ICCP HOST 운영 포인트 : 원격진단 Analog Input
    
    I60870_DATA     iccp_qdi[MAX_SDP_POINT_QDI];    // ICCP HOST 운영 포인트 : 전력품질 Digital Input
    I60870_DATA     iccp_qai[MAX_SDP_POINT_QAI];    // ICCP HOST 운영 포인트 : 전력품질 Analog Input
    
    I60870_DATA     iccp_tdi[MAX_SDP_POINT_TDI];    // ICCP HOST 운영 포인트 : 고장점 표정반 Digital Input
    I60870_DATA     iccp_tai[MAX_SDP_POINT_TAI];    // ICCP HOST 운영 포인트 : 고장점 표정반 Analog Input
    
    I60870_DATA     iccp_dev[MAX_SDP_POINT_DEV];    // ICCP HOST 운영 포인트 : 시스템장치 포인트(가상)
    
} __attribute__ ((packed)) ICCP_60870_DCB;    

#endif	// RTUBUF_HEADER_INCLUDED
