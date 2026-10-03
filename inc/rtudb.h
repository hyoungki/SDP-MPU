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

#ifndef	RTUDB_HEADER_INCLUDED
#define	RTUDB_HEADER_INCLUDED

//
// Include Files
//
#include "common.h"


/* ------------------------------------------------ */
/* 	이중화 관련 Parameter 		                    */
/* 	MUU-A        : MPU-A-System MPU 모듈	        */
/* 	CPU_SLAVE    :          CPU SLAVE  설정(Switch)	*/
/* 	LOCAL_MASTER : Active CPU 설정상태				*/
/* 	LOCAL_SLAVE  : NON-Active CPU 설정상태			*/
/* ------------------------------------------------ */
#define MPU_A    	        0           // MPU 이중화 : 보드 실장 상태 A-System
#define MPU_B     	        1           // MPU 이중화 : 보드 실장 상태 B-System

#define LOCAL_MASTER	    0           // MPU 이중화 : 동작상태 Local-Master 
#define LOCAL_SLAVE         1           // MPU 이중화 : 동작상태 Local-Slave 

#define MASTER_PORT         0           // HOST 이중화 내 MASTER/SLAVE 포트 구분
#define SLAVE_PORT          1           // HOST 이중화 내 MASTER/SLAVE 포트 구분

#define STATUS_DUMP_TICK    2           // 초기화 상태덤프 수

/* ------------------------------------------------ */
/* SDPT 관련 상수정의                               */
/* ------------------------------------------------ */
#define MAX_HOST            8           // 최대 SDP 수용 HOST 수 
#define MAX_ESIO            6           // 최대 장착 ESIO 구조체 : ESIO#0 - SIO, ESIO#1-전력감시, ESIO#2-원격진단, ESIO#3-전력품질, ESIO#4-61850, ESIO#5-RTU
#define MAX_ESIO_PORT       8           // ESIO 수용 Channel 수

/* ============================================ */
/*	2020.06.04 공유메모리 최적화 : Down Sizing			*/
/* ============================================ */
//#define MAX_SCAN_PORT       32          // SDP 수용 SCAN-Channel 수
//#define MAX_MODBUS_PROFILE  16          // SDP 수용 MODBUS-PROFILE 수
//#define MAX_DEVICE          112         // SDP 수용 최대 계전기 수
//#define MAX_DBASE_POINT     4096        // 데이터베이스 상의 수용 포인트 DB

#define MAX_SCAN_PORT       16          	// SDP 수용 SCAN-Channel 수
#define MAX_MODBUS_PROFILE  16         		// SDP 수용 MODBUS-PROFILE 수

#define MAX_DEVICE          64         		// SDP 수용 최대 계전기 수

#define MAX_DBASE_POINT     (2048+1024)     // 데이터베이스 상의 수용 포인트 DB


#define SDP_NULL            0           // ESIO 장치 TYPE : 미정의
#define SDP_SIO             1           // ESIO 장치 TYPE : 미정의
#define SDP_ESIO_1          2           // ESIO 장치 TYPE : 전철제어
#define SDP_ESIO_2          3           // ESIO 장치 TYPE : 원격진단
#define SDP_ESIO_3          4           // ESIO 장치 TYPE : 전력품질
#define SDP_ESIO_4          5           // ESIO 장치 TYPE : 61850
#define SDP_RTU             6           // ESIO 장치 TYPE : 전용선 SIO
#define SDP_MPU_1           7
#define SDP_MPU_2           8

/* ESIO # 구조체별 기능할당...  */
#define ESIO_SCADA          1
#define ESIO_REMOTE         2
#define ESIO_ELECQ          3
#define ESIO_61850          4
#define ESIO_RTU            5

  
#define SDP_SCAN_NULL       0
#define SDP_SCAN_1          1           // SCAN # Index 1
#define SDP_SCAN_2          2           // SCAN # Index 1
#define SDP_SCAN_3          3           // SCAN # Index 1
#define SDP_SCAN_4          4           // SCAN # Index 1
#define SDP_SCAN_5          5           // SCAN # Index 1
#define SDP_SCAN_6          6           // SCAN # Index 1
#define SDP_SCAN_7          7           // SCAN # Index 1
#define SDP_SCAN_8          8           // SCAN # Index 1
#define SDP_SCAN_9          9           // SCAN # Index 1
#define SDP_SCAN_10         10          // SCAN # Index 1
#define SDP_SCAN_11         11          // SCAN # Index 1
#define SDP_SCAN_12         12          // SCAN # Index 1
#define SDP_SCAN_13         13          // SCAN # Index 1
#define SDP_SCAN_14         14          // SCAN # Index 1
#define SDP_SCAN_15         15          // SCAN # Index 1
#define SDP_SCAN_16         16          // SCAN # Index 1


/* MPU-ESIO/RTU 간 통신연계방식 */
#define MPU_LINK_MODULE     0
#define MPU_LINK_DNP        1

  
/* HOST 운영모드 지정 */
#define HOST_NOT_USE        0           // HOST 운영모드 : 사용않함.
#define HOST_SINGLE         1           // HOST 운영모드 : 개별 HOST
#define HOST_DUAL           2           // HOST 운영모드 : 이중화 HOST

/* HOST 통신프로토콜 */
#define HOST_NULL           0           // HOST Protocol : 미지정, 사용않함
#define HOST_HARRIS         1           // HOST Protocol : HARRIS 
#define HOST_LANDIS         2           // HOST Protocol : LANDIS 
#define HOST_DNP            3           // HOST Protocol : DNP 
#define HOST_MODBUS         4           // HOST Protocol : MODBUS 
#define HOST_ICCP	        5           // HOST Protocol : ICCP-60870 
#define HOST_IEC_101	    6           // HOST Protocol : IEC60870-101 
#define HOST_RESERVE1       7           // HOST Protocol : Reserved
#define HOST_RESERVE2       8           // HOST Protocol : Reserved

/* HOST 연계방식 */
#define COM_RS232      	    0           // HOST 통신모드 : RS232
#define COM_MODEM      	    1           // HOST 통신모드 : MODEM
#define	COM_RS485		    2           // HOST 통신모드 : RS485
#define COM_TCPIP      	    3           // HOST 통신모드 : TCPIP (Network)

/* HOST 연계속도 */
#define SPEED_1200  	    0
#define SPEED_2400          1
#define SPEED_4800          2
#define SPEED_9600          3
#define SPEED_19200         4
#define SPEED_38400         5
#define SPEED_57600         6
#define SPEED_115200        7

/* ESIO PORT Index  */
#define ESIO_PORT_1  	    0           // ESIO 내장 PORT Type : Port 1
#define ESIO_PORT_2 	    1           // ESIO 내장 PORT Type : Port 2
#define ESIO_PORT_3  	    2           // ESIO 내장 PORT Type : Port 3
#define ESIO_PORT_4  	    3           // ESIO 내장 PORT Type : Port 4
#define ESIO_PORT_5  	    4           // ESIO 내장 PORT Type : Port 5
#define ESIO_PORT_6  	    5           // ESIO 내장 PORT Type : Port 6
#define ESIO_PORT_7  	    6           // ESIO 내장 PORT Type : Port 7
#define ESIO_PORT_8  	    7           // ESIO 내장 PORT Type : Port 8


/* ------------------------------------------------ */
/* DNP HOST 관련 MAX 포인트 정의                    */
/* ------------------------------------------------ */
#define MAX_DNP_DI_POINT    2048        // DNP HOST 운영 포인트 : 상태감시 
#define MAX_DNP_AI_POINT    1024        // DNP HOST 운영 포인트 : 아날로그 

#define MAX_DNP_DO_POINT    256         // DNP HOST 운영 포인트 : 상태제어 
#define MAX_DNP_AO_POINT    64          // DNP HOST 운영 포인트 : 아날로그 설정 
#define MAX_DNP_COUNT_POINT 64          // DNP HOST 운영 포인트 : COUNT 

/* ------------------------------------------------ */
/* CU 최대 포인트 - 계전기 32 장착시 */
/* ------------------------------------------------ */
#if 0

#define MAX_DEV_DI_POINT    1024        // 장치당 최대 DI 포인트 
#define MAX_DEV_AI_POINT    384         // 장치당 최대 AI 포인트 

#define MAX_DEV_DO_POINT    64          // 장치당 최대 DO 포인트 
#define MAX_DEV_AO_POINT    64          // 장치당 최대 AI 포인트 
#define MAX_DEV_COUNT_POINT 64          // 장치당 최대 COUNT 포인트 

#endif

/* 2022.06.29 용산SSP 적용시 ... 변경...(김수현) */
#define MAX_DEV_DI_POINT    1024        // 장치당 최대 DI 포인트 
#define MAX_DEV_AI_POINT    200         // 장치당 최대 AI 포인트 
#define MAX_DEV_DO_POINT    200         // 장치당 최대 DO 포인트 
#define MAX_DEV_AO_POINT    64          // 장치당 최대 AI 포인트 
#define MAX_DEV_COUNT_POINT 64          // 장치당 최대 COUNT 포인트 


#define MAX_DEV_POINT       100         // 전체 Device 포인트 : 계전기 통신상태, 이중화, 절체 등 Device 종속 포인트 
//#define MAX_CAL_POINT       32          // 장치당 최대 연산포인트  
#define MAX_CAL_POINT       16          // 장치당 최대 연산포인트  

#define	MAX_DI_DATA_NUM	    (MAX_DEV_DI_POINT / 8)		// 계전기로부터의 최대 DI Byte Count


/* ------------------------------------------------ */
/* 계전기 Module Type defination                    */
/* ------------------------------------------------ */
#define	TYPE_NULL			0	
#define TYPE_DIM            1           // Digital Input Board
#define TYPE_DOM            2           // Digital Output Board[1] 
#define TYPE_AIM            3           // Analog Input Board      
#define TYPE_AOM            4           // Analog Output Board

#define TYPE_DNP_GIPAM      5           // LG-GIPAM 계전기
#define TYPE_DNP_SIEMENS    6           // SIMENSE 계전기
#define TYPE_DNP_HIMAP      7           // 현대-HIMAP 계전기
#define TYPE_DNP_FRTU       8           // FRTU 계전기
#define TYPE_DNP_PROPACK    9           // 효성계전기 -PROPACK  
#define TYPE_DNP_VIPAM      10           // 비츠로계전기   
#define TYPE_DNP_LBS        11          // 부하개폐기

#define TYPE_MOD_LG         12          // MODBUS - LG 계전기   
#define TYPE_MOD_SIEMENS    13          // MODBUS - 지멘스 계전기   
#define TYPE_MOD_SUB        14          // MODBUS - SUB RTU

#define TYPE_LOC_TYPE1      15          // 고장점 인식기 - TYPE1
#define TYPE_LOC_TYPE2      16          // 고장점 인식기 - TYPE2

#define TYPE_REM_UHF        17          // 원격진단 - 부분방전 진단장치
#define TYPE_REM_DGA        18          // 원격진단 - 유증가스 분석장치
#define TYPE_REM_LA         19          // 원격진단 - 피뢰기 감시장치
#define TYPE_REM_TYPE1      20          // 원격진단 - 예비장치1
#define TYPE_REM_TYPE2      21          // 원격진단 - 예비장치2
#define TYPE_REM_TYPE3      22          // 원격진단 - 예비장치3
#define TYPE_REM_TYPE4      23          // 원격진단 - 예비장치4

#define TYPE_EQM_TYPE1      24          // 전력품질 - 예비장치1
#define TYPE_EQM_TYPE2      25          // 전력품질 - 예비장치2
#define TYPE_EQM_TYPE3      26          // 전력품질 - 예비장치3
#define TYPE_EQM_TYPE4      27          // 전력품질 - 예비장치4

#define TYPE_61850_1        28          // 전력품질 - 예비장치4
#define TYPE_61850_2        29          // 전력품질 - 예비장치4
#define TYPE_61850_3        30          // 전력품질 - 예비장치4


/* ------------------------------------------------ */
/* 포인트 관련 상수정의                             */
/* Point Type 및 상수 정의                          */
/* ------------------------------------------------ */
/* 포인트 DB - DEVICE Type Define */
#define NULL_DEV            0
#define DI_POINT            1           // Module & IED : DI 포인트
#define DO_POINT            2           // Module & IED : DO 포인트
#define AI_POINT            3           // Module & IED  : AI 포인트
#define AO_POINT            4           // Module & IED  : AO 포인트
#define VDI_POINT           5           // 연산포인트 : DI 포인트
#define VAI_POINT           6           // 연산포인트 : AI 포인트
#define VDO_POINT           7           // 연산포인트 : DO 포인트
#define DEV_POINT           8           // 가상 포인트 : 통신이상/정상 

#define SDP_STS_POINT       9           // 가상 포인트 : SDP-STATUS (상태) 
#define CPU_RUN_POINT       10          // 가상 포인트 : CPU 동작정보 ,[0] 1계, [1] 2계 동작정보 
#define CPU_STS_POINT       11          // 가상 포인트 : CPU 상태정보, [0] 정상,[1] 이상
#define RTU_RUN_POINT       12          // 가상 포인트 : RTU 동작정보 ,[0] 1계, [1] 2계 동작정보 
#define RTU_STS_POINT       13          // 가상 포인트 : RTU 상태정보, [0] 정상,[1] 이상
#define CU_RUN_POINT        14          // 가상 포인트 : 전철제어반 동작정보 ,[0] 1계, [1] 2계 동작정보 
#define CU_STS_POINT        15          // 가상 포인트 : 전철제어반 상태정보, [0] 정상,[1] 이상
#define DIG_RUN_POINT       16          // 가상 포인트 : 원격진단부 동작정보 ,[0] 1계, [1] 2계 동작정보 
#define DIG_STS_POINT       17          // 가상 포인트 : 원격진단부 상태정보, [0] 정상,[1] 이상
#define EQM_RUN_POINT       18          // 가상 포인트 : 전력품질부 동작정보 ,[0] 1계, [1] 2계 동작정보 
#define EQM_STS_POINT       19          // 가상 포인트 : 전력품질부 상태정보, [0] 정상,[1] 이상
#define IEC_RUN_POINT       20          // 가상 포인트 : 61850 동작정보 ,[0] 1계, [1] 2계 동작정보 
#define IEC_STS_POINT       21          // 가상 포인트 : 61850 상태정보, [0] 정상,[1] 이상


/* 포인트 TYPE 정의  */
#define NULPT               0
#define COSPT               1           // 상태 포인트 : COS 
#define SOEPT               2           // 상태 포인트 : SOE 
#define ALLPT               3           // 상태 포인트 : COS & SOE 
#define ACCPT               4           // 계측 포인트 : 누적포인트
#define SBOPT               5           // 제어 포인트 : SBO Control - Select Before Operate
#define DCOPT               6           // 제어 포인트 : DCO Control - Direct Control
#define AI11PT              7           // 계측 포인트 : +1mA ~ - 1mA
#define AI420PT             8           // 계측 포인트 : 4mA ~ 20mA
#define AI10VPT             9           // 계측 포인트 : +10V ~ -10V
#define RALLPT              10          // 가상 포인트 : 전체 계전기 통신상태
#define RDEVPT              11          // 가상 포인트 : 개별 계전기 통신상태
#define MCPUPT              12          // 가상 포인트 : CPU-A 동작상태
#define SCPUPT              13          // 가상 포인트 : CPU-B 동작상태
#define CNTRPT              14          // 가상 포인트 : CPU 절체


/* ------------------------------------ */
/* DO 포인트 타입 설정                  */
/* ------------------------------------ */
#define CONTROL_POINT       1           // 개별 포인트 제어
#define CONTROL_SYSTEM      2           // 시스템 절체 제어

/* ------------------------------------ */
/* DO 포인트 Config 설정                */
/* ------------------------------------ */
#define DNP_POINT_TC        0           // DNP 계전기 : TRIP-CLOSE 방식제어
#define DNP_PULSE_CLOSE     1           // DNP 계전기 : PULSE 방식제어 (CLOSE 우선)
#define DNP_PULSE_TRIP      2           // DNP 계전기 : PULSE 방식제어 (TRIP 우선)
#define DNP_POINT_LATCH     3           // DNP 계전기 : LATCH 방식제어

#define SIEMENS_TRIP_FIRST  0           // MODBUS 계전기 : 지멘스-TRIP 선행
#define SIEMENS_CLOSE_FIRST 1           // MODBUS 계전기 : 지멘스-CLOSE 선행
#define SIEMENS_RESET_CNTR  2           // MODBUS 계전기 : 지멘스-RESET
#define SIEMENS_BIT_CNTR    3           // MODBUS 계전기 : 지멘스-BIT
#define MODBUS_LG_CNTR      8           // MODBUS 계전기 : LG Select-Operate 제어

#define TRIP_CONTROL        0x80        // DNP-TRIP  Control
#define CLOSE_CONTROL       0x40        // DNP-CLOSE Control

#define TRIP_INFO           1
#define CLOSE_INFO          2

/* IEC-HOST : 상위 제어명령에 대한 구분... */
#define IEC_CNTR_CLEAR      0
#define IEC_CNTR_SELECT     1
#define IEC_CNTR_OPERATE    2

/* --------------------------------------- */
/* ESIO : SCAN Task : 프로토콜 타입 정의 */
/* --------------------------------------- */
#define SCAN_PROT_NULL      0
#define SCAN_PROT_DNP       1		    // SCAN# 프로토콜 타입 : DNP
#define SCAN_PROT_MODBUS    2           // SCAN# 프로토콜 타입 : MODBUS
#define SCAN_PROT_BUS       3           // SCAN# 프로토콜 타입 : BUS
#define SCAN_PROT_REMOTE1   4           // SCAN# 프로토콜 타입 : 원격진단#1
#define SCAN_PROT_REMOTE2   5           // SCAN# 프로토콜 타입 : 원격진단#2
#define SCAN_PROT_REMOTE3   6           // SCAN# 프로토콜 타입 : 원격진단#3
#define SCAN_PROT_EQM1      7           // SCAN# 프로토콜 타입 : 전력품질#1
#define SCAN_PROT_EQM2      8           // SCAN# 프로토콜 타입 : 전력품질#2
#define SCAN_PROT_EQM3      9           // SCAN# 프로토콜 타입 : 전력품질#3
#define SCAN_PROT_61850     10          // SCAN# 프로토콜 타입 : 61850
#define SCAN_PROT_LOCATOR1  11          // SCAN# 프로토콜 타입 : 고장점#1
#define SCAN_PROT_LOCATOR2  12          // SCAN# 프로토콜 타입 : 고장점#2
#define SCAN_PROT_TYPE1     13          // SCAN# 프로토콜 타입 : 예비
#define SCAN_PROT_TYPE2     14          // SCAN# 프로토콜 타입 : 예비
#define SCAN_PROT_TYPE3     15          // SCAN# 프로토콜 타입 : 예비



/* --------------------------------------- */
/* 시뮬레이터 관련                          */
/* --------------------------------------- */
#define SIMULATOR_NET_PORT  	9600        // SIMULATOR S/W - TCPIP PORT 번호       
#define ESIO_SCAN_NET_PORT  	9700        // ESIO SCAN S/W - TCPIP PORT 번호       
#define MAX_SIMULATOR_BUF   	4096
#define LINK_NET_PORT       	9900        // CPU 이중화 LINK 연결 PORT

/* ---------------------------------------------------- */
/* STURECTURE 정의 : DB_HOST_CONFIG                     */
/* - HOST 구성정보 및 운영 Parameter DB                 */
/* ---------------------------------------------------- */   
typedef struct
    {
        byte    netPort;            // Network# 사용 Port, [0] NET1 ~[7] NET8
        byte    ipAddr[16];         // Network# IP-Address

    } __attribute__ ((packed)) DB_HNET_ENTRY;
    
typedef struct
    {
        byte    runMode;                // HOST Config : HOST 운영모드 : [0]사용안함, [1]개별, [2]이중화 
        byte    protocol;               // HOST Config : HOST 통신 프로토콜 : HARRIS/LANDIS/DNP/MODBUS/IEC...
        byte    comMode;                // HOST Config : HOST 통신모드 : RS232/MODEM/RS485/TCPIP
        byte    comSpeed;               // HOST Config : HOST 통신속도 
        
        byte    masterPort;             // HOST Config : HOST MASTER 통신포트
        byte    slavePort;              // HOST Config : HOST SLAVE  통신포트
        
        byte	hostAddr[2];		    // HOST Config : HOST DNP - 센터 Address
        byte	rtuAddr[2];			    // HOST Config : HOST DNP - RTU Address
        byte	hostNameStr[20];		// HOST Config : HOST Name String
        
        byte    timeSyncDISB;            // HOST TIME-SYNC 허용/금지
        byte    tcpipPort[2];
        
        DB_HNET_ENTRY  masterNetCfg[2];    // HOST Config : HOST 주장치 Network Config
        DB_HNET_ENTRY  slaveNetCfg[2];     // HOST Config : HOST 부장치 Network Config
        
        byte    soeClass;               // HOST Config : SOE Class 지정 
        byte    cosClass;               // HOST Config : COS Class 지정 
        byte    coaClass;               // HOST Config : COA Class 지정 
        byte    unsolite;               // HOST Config : Unsolite 지정  
        
        byte    comDelay;               // HOST Config : HOST 통신 지연 (10ms)
        byte    offCount;               // HOST Config : HOST 통신 Offline Count
        
        byte    chgMode;                // HOST Config : HOST Change Mode;
        byte	reserved1;			// HOST TIME-SYNC 허용/금지
        byte	reserved2;
                
    } __attribute__ ((packed)) DB_HOST_CONFIG;


/* ---------------------------------------------------- */
/* STURECTURE 정의 : DB_MPU_CONFIG                      */
/* - MPU Network 구성정보 및 운영 Parameter DB          */
/* ---------------------------------------------------- */   
typedef struct
    {
        byte    useFlag;            // Network# 사용유무
        byte    reserved;           // 예약
        
        byte    ipAddr[16];         // Network# IP-Address
        byte    gwAddr[16];         // Network# GW-Address
        byte    subMask[16];        // Network# SUB-Mask

    } __attribute__ ((packed)) DB_MNET_ENTRY;


typedef struct
    {
        byte    sdpNameStr[20];         // SDP 장치이름

        /* ---------------------------- */        
        /* Master MPU Network 구성정보  */
        /* ---------------------------- */ 
        DB_MNET_ENTRY   master_netCfg[4];
        
        /* ---------------------------- */ 
        /* Slave  MPU Network 구성정보  */
        /* ---------------------------- */ 
        DB_MNET_ENTRY   slave_netCfg[4];
        
        byte    dualMpu;                // MPU Parameter : CPU 이중화 운영모드
        byte	dualModule;             // MPU Parameter : Module 이중화 운영모드 
        
        byte    scuUseFlag;        	    // MPU Parameter : SCU 모듈 사용유무
        byte	mmiUseFlag;             // MPU Parameter : MMI 모듈 사용유무
        
        byte    statusDump;        	    // MPU Parameter : STATUS Dump 주기
       	byte    analogDump;        	    // MPU Parameter : ANALOG Dump 주기
        byte    debounce;               // MPU Parameter : Function 코드#1
        byte    dbCheck;                // MPU Parameter : Function 코드#2
        byte    hostWDT;                // MPU Parameter : Function 코드#3
        byte    iccpFlag;               // MPU Parameter : Function 코드#4
        
        byte    func1;
        byte    func2;
        byte    func3;
        byte    func4;
        byte    func5;
        byte    func6;
        

    } __attribute__ ((packed)) DB_MPU_CONFIG;
    
#if 0
typedef struct
    {
        byte    sdpNameStr[20];         // SDP 장치이름

        /* ---------------------------- */        
        /* Master MPU Network 구성정보  */
        /* ---------------------------- */ 
        DB_MNET_ENTRY   master_netCfg[3];        
        /* ---------------------------- */ 
        /* Slave  MPU Network 구성정보  */
        /* ---------------------------- */ 
        DB_MNET_ENTRY   slave_netCfg[3];        

        byte    dualMpu;                // MPU Parameter : CPU 이중화 운영모드
        byte	dualModule;             // MPU Parameter : Module 이중화 운영모드 
        
        byte    scuUseFlag;        	    // MPU Parameter : SCU 모듈 사용유무
        byte	mmiUseFlag;             // MPU Parameter : MMI 모듈 사용유무
        
        byte    statusDump;        	    // MPU Parameter : STATUS Dump 주기
       	byte    analogDump;        	    // MPU Parameter : ANALOG Dump 주기
        byte    debounce;               // MPU Parameter : Function 코드#1
        byte    dbCheck;                // MPU Parameter : Function 코드#2
        byte    hostWDT;                // MPU Parameter : Function 코드#3
        byte    iccpFlag;               // MPU Parameter : Function 코드#4
        
        byte    func1;
        byte    func2;
        byte    func3;
        byte    func4;
        byte    func5;
        byte    func6;
        

    } __attribute__ ((packed)) DB_MPU_CONFIG2;
#endif 
typedef struct
    {
        byte    sdpNameStr[20];         // SDP 장치이름

        /* ---------------------------- */        
        /* Master MPU Network 구성정보  */
        /* ---------------------------- */ 
        DB_MNET_ENTRY   master_netCfg[3];
        
        /* ---------------------------- */ 
        /* Slave  MPU Network 구성정보  */
        /* ---------------------------- */ 
        DB_MNET_ENTRY   slave_netCfg[3];
        
        byte    dualMpu;                // MPU Parameter : CPU 이중화 운영모드
        byte	dualModule;             // MPU Parameter : Module 이중화 운영모드 
        
        byte    scuUseFlag;        	    // MPU Parameter : SCU 모듈 사용유무
        byte	mmiUseFlag;             // MPU Parameter : MMI 모듈 사용유무
        
        byte    statusDump;        	    // MPU Parameter : STATUS Dump 주기
       	byte    analogDump;        	    // MPU Parameter : ANALOG Dump 주기
        byte    debounce;               // MPU Parameter : Function 코드#1
        byte    dbCheck;                // MPU Parameter : Function 코드#2
        byte    hostWDT;                // MPU Parameter : Function 코드#3
        byte    iccpFlag;               // MPU Parameter : Function 코드#4
        
        byte    func1;
        byte    func2;
        byte    func3;
        byte    func4;
        byte    func5;
        byte    func6;
        

    } __attribute__ ((packed)) DB_ESIO_MPU_CONFIG;

/* ---------------------------------------------------- */
/* STURECTURE 정의 : DB_ESIO_CONFIG                     */
/* - ESIO Network 구성정보 및 운영 Parameter DB         */
/* ---------------------------------------------------- */       
typedef struct
    {
        byte    useFlag;            // Network# 사용유무
        byte    ipAddr[16];         // Network# IP-Address
        byte    gwAddr[16];         // Network# GW-Address
        byte    subMask[16];        // Network# SUB-Mask

    } __attribute__ ((packed)) DB_ENET_ENTRY;

typedef struct
    {
        byte    useFlag;            // PORT# 사용유무
        byte    function;           // PORT# Function
        byte    comMode;            // PORT# 통신모드, 0:RS232, 1:MODEM, 2:RS485, 3:TCPIP
        byte    comSpeed;           // PORT# 통신속도, [0] 1200 ~ [7] 115200
        
        byte    portNameStr[20];    // PORT# 포트이름

    } __attribute__ ((packed)) DB_PORT_ENTRY;

// 2026-05-14 오후 4:15:07   ESIO ARM 인 경우 network 이 4개가 기본.
#ifdef __ARM_ARCH__
    #define  ESIO_NETWORK_NUMBER  4
#else
    #define  ESIO_NETWORK_NUMBER  3
#endif 


typedef struct
    {
        byte    useFlag;                // ESIO Parameter : ESIO 사용유무
        byte	targetID;               // ESIO Parameter : ESIO Target Module-ID, 0:사용않함, 1: ESIO1, 2:ESIO2, 3:ESIO3, 4:ESIO4, 5:SIO
        byte    autoChgFlag;            // ESIO Parameter : 자동절체 Flag
        byte    comDelay;               // ESIO Parameter : MPU 통신 지연 (10ms)
        
        byte    esioNameStr[20];        // ESIO 장치이름

        /* ---------------------------- */        
        /* Master MPU Network 구성정보  */
        /* ---------------------------- */ 
        DB_ENET_ENTRY   mstNetConfig[ESIO_NETWORK_NUMBER ];
        DB_ENET_ENTRY   slvNetConfig[ESIO_NETWORK_NUMBER ];
        
        DB_PORT_ENTRY   portConfig[8];			// ESIO 당 8 Channel
        
    } __attribute__ ((packed)) DB_ESIO_CONFIG;
        


/* ---------------------------------------------------- */
/* STURECTURE 정의 : DB_SCAN_CONFIG                     */
/* - SDP 전체 SCAN-Channel 구성정보                     */
/* ---------------------------------------------------- */     

typedef struct
    {
        byte    useFlag;                // SDP SCAN : Channel 사용유무
        byte	targetID;               // SDP SCAN : ESIO Target Module-ID, 0:사용않함, 1: ESIO1, 2:ESIO2, 3:ESIO3, 4:ESIO4, 5:SIO
        
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

    } __attribute__ ((packed)) DB_SCAN_CONFIG;


/* ---------------------------------------------------- */
/* STURECTURE 정의 : DB_DEVICE_CONFIG                   */
/* - SDP 전체 계전기 구성정보                           */
/* ---------------------------------------------------- */     

typedef struct
    {
        byte	devNameStr[40];		    // SDP DEVICE : 계전기 Name String

        byte    useFlag;                // SDP DEVICE : 계전기 사용유무
        byte    comDevID;               // SDP DEVICE : 기능모듈내 계전기 ID 
        byte    comDevIndex;            // SDP DEVICE : 기능모듈내 계전기 Index 
        byte    scanPort;               // SDP DEVICE : 계전기 통신 포트, 0: 사용않함, 1 ~ 32
        byte    type;           	    // SDP DEVICE : 계전기 TYPE
		byte	dualENB;			    // SDP DEVICE : 계전기 - 계전기 이중화 여부
		byte	modbusFileNo;		    // SDP DEVICE : 계전기 - MODBUS 프로파일, 0: 사용않함, 1 ~ 16
		
		byte	di_ptnum[2];		    // SDP DEVICE : DI 포인트 수
		byte	do_ptnum[2];		    // SDP DEVICE : DO 포인트 수
		byte	ai_ptnum[2];		    // SDP DEVICE : AI 포인트 수
		byte	ao_ptnum[2];		    // SDP DEVICE : AO 포인트 수
		byte	cnt_ptnum[2];		    // SDP DEVICE : COUNT 포인트 수
		
		/* -------------------- */
		/*  계전기 Network 정보 */
		/* -------------------- */
		byte	ipString1[16];		    // SDP DEVICE : HOST IP Address
		byte	ipString2[16];		    // SDP DEVICE : HOST IP Address
		
        byte	netPort[2];			    // SDP DEVICE : HOST TCPIP Port번호 */
        byte    function1;              // SDP DEVICE : SCAN Function#1
   		
    } __attribute__ ((packed)) DB_SDP_DEVICE;

/* ------------------------------------------------ */
/* ICCP HOST 관련 MAX 포인트 정의                   */
/* ------------------------------------------------ */
#define MAX_ICCP_POINT      MAX_DBASE_POINT        // ICCP-HOST 용 포인트 DB

#define MAX_SDP_POINT_SDI   2048        // ICCP HOST 운영 포인트 : SCADA Digital Input
#define MAX_SDP_POINT_SDO   256         // ICCP HOST 운영 포인트 : SCADA Digital Output
#define MAX_SDP_POINT_SAI   1024        // ICCP HOST 운영 포인트 : SCADA Analog Input
#define MAX_SDP_POINT_DDI   1024        // ICCP HOST 운영 포인트 : 원격진단 Digital Input
#define MAX_SDP_POINT_DAI   1024        // ICCP HOST 운영 포인트 : 원격진단 Analog Input
#define MAX_SDP_POINT_QDI   256         // ICCP HOST 운영 포인트 : 전력품질 Digital Input
#define MAX_SDP_POINT_QAI   2048        // ICCP HOST 운영 포인트 : 전력품질 Analog Input
#define MAX_SDP_POINT_TDI   256         // ICCP HOST 운영 포인트 : 고장점 표정반 Digital Input

#define MAX_SDP_POINT_TAI   256         // ICCP HOST 운영 포인트 : 고장점 표정반 Analog Input
#define MAX_SDP_POINT_DEV   128         // ICCP HOST 운영 포인트 : 시스템장치 포인트(가상)

/* ---------------------------------------------------- */
/* ICCP - 포인트 TYPE 정의 : DB_POINT_CONFIG            */
/* - SDP 전체 포인트 구성정보                           */
/* ---------------------------------------------------- */  
#define	SDP_POINT_NULL      0	// 미지정
#define	SDP_POINT_TYPE_SDI	1	// SCADA Digital Input
#define	SDP_POINT_TYPE_SDO	2	// SCADA Digital Output
#define	SDP_POINT_TYPE_SAI	3	// SCADA Analog  Input

#define	SDP_POINT_TYPE_DDI	4	// 원격진단 : Diagnosis Digital Input
#define	SDP_POINT_TYPE_DAI	5	// 원격진단 : Diagnosis Analog  Input
#define	SDP_POINT_TYPE_QDI	6	// 전력품질 : Quality Digital Input
#define	SDP_POINT_TYPE_QAI	7	// 전력품질 : Quality Analog  Input
#define	SDP_POINT_TYPE_TDI	8	// 고장점   : Train Digital Input
#define	SDP_POINT_TYPE_TAI	9	// 고장점   : Train Analog  Input
#define	SDP_POINT_TYPE_DEV	10	// 장치     : Device (가상포인트)

#define	MAX_SDP_POINT_TYPE	10

#define	VALID_SDP_POINT_TYPE(x)	((x) >= SDP_POINT_TYPE_SDI && (x) <= SDP_POINT_TYPE_DEV)
#define	DIGITAL_SDP_POINT_TYPE(x)	((x) == SDP_POINT_TYPE_SDI || (x) == SDP_POINT_TYPE_DDI ||\
									 (x) == SDP_POINT_TYPE_QDI || (x) == SDP_POINT_TYPE_TDI ||\
									 (x) == SDP_POINT_TYPE_DEV)

/* ---------------------------------------------------- */
/* STURECTURE 정의 : DB_POINT_CONFIG                    */
/* - SDP 전체 포인트 구성정보                           */
/* ---------------------------------------------------- */     
typedef struct
    {
        byte    ptNameStr[40];      // SDP POINT : 포인트 이름

        byte    devNo;              // SDP POINT : Device 번호 [1..32]
        byte    devPt[2];           // SDP POINT : Device 포인트 번호 [1..1024]
        byte    devType;            // SDP POINT : Devic TYPE      

        /* ------------------------ */
        /*  ICCP 참조 영역          */
        /* ------------------------ */
		byte	iccpPointType;		// SDP POINT : ICCP 포인트 TYPE, SDI/SDO/SAI/DDI/DAI/QDI/QAI/TDI/TAI/DEV
		byte	iccpPointIndex[2];	// SDP POINT : ICCP 포인트 인덱스, [0: 미지정, 1 ~ 1024]
        byte	reserved1;	        // SDP POINT : ICCP 예비
        byte	reserved2;	        // SDP POINT : ICCP 예비
                    
        byte    ptType;             // SDP POINT : device 포인트 TYPE 
        byte    ptConfig;           // SDP POINT : Point Config
        
        byte    pointMax[2];        // SDP POINT : Point MAX Scale 값
        byte    pointOffset[2];     // SDP POINT : Point Offset Scale 값
        byte    pointDelta;         // SDP POINT : Point Delts Scale 값
        byte    localIndex[2];
        
     	byte	modBase[2];			// SDP POINT : MODBUS base Address
     	byte	modIndex[2];		// SDP POINT : MODBUS index
             
        byte    port;               // SDP POINT : HARRIS-주장치 PORT 번호  [1..16]
        byte    point;              // SDP POINT : HARRIS-주장치 POINT 번호 [1..64] 

        byte    hostIndex[8][2];    // SDP POINT : 상위 호스트#1~8 Index 번호 [1..1024]

		byte    onStr[10];      	// SDP POINT : ON  상태 이름
		byte    offStr[10];      	// SDP POINT : OFF 상태 이름
        
    } __attribute__ ((packed)) DB_POINT_BUF;


/* ---------------------------------------------------- */
/* STURECTURE 정의 : DB_MODBUS_PROFILE                  */
/* - MODBUS PROFILE 세부 구성DB                         */
/* ---------------------------------------------------- */  
typedef struct
    {
        byte    useFlag;           	/* 사용여부 */
        byte    opcode;           	/* OPCODE */

        byte    baseAddr[2];   		/* BASE Address */
        byte    baseOffset[2];   	/* BASE Address */
		byte    dataNum[2];			/* point Number */

		byte	dataType;			/* Data Type */
		byte	dataFormat;			/* Data Format */
		byte	swepEnb;    
		byte    function;

    } __attribute__ ((packed)) DB_MODBUS_READ;
    
typedef struct
    {
        byte    useFlag;           	/* 사용여부 */
        byte    opcode;           	/* OPCODE */

        byte    baseAddr[2];   		/* BASE Address */
        byte    baseOffset[2];   	/* BASE Address */
        byte    dataNum[2];			/* point Number */

		byte	dataType;			/* Data Type */
		byte	dataFormat;			/* Data Format */
		
		byte	tripData[2];
		byte	closeData[2];

    } __attribute__ ((packed)) DB_MODBUS_WRITE;    

typedef struct
    {
        byte    useFlag;           	/* 사용여부 */
        byte    opcode;           	/* OPCODE */

        byte    baseAddr[2];   		/* BASE Address */
        byte    baseOffset[2];   	/* BASE Address */
		byte    dataNum[2];			/* point Number */

		byte	dataType;			/* Data Type */
		byte	dataFormat;			/* Data Format */
		byte    reserved1;
		byte    reserved2;

    } __attribute__ ((packed)) DB_MODBUS_TIME;
    
typedef struct
    {
    	DB_MODBUS_READ	readBlock[8];
    	DB_MODBUS_WRITE	writeBlock[4];
    	DB_MODBUS_TIME	timeBlock;
    	
    } __attribute__ ((packed)) DB_MODBUS_PROFILE;

/* ---------------------------------------------------- */
/* STURECTURE 정의 : DB_ICCP_CONFIG                     */
/* - ICCP HOST 세부 구성DB                              */
/* ---------------------------------------------------- */  
typedef struct
    {
        char    IPAddress[16];      // SDP : ICCP IP Address
        char    P_Sel[48];          // SDP : ICCP Presentation Selector, null 이면 미사용 "00 00 00 01"
        char    S_Sel[48];          // SDP : ICCP Session Selector, null 이면 미사용 "00 01"
        char    T_Sel[96];          // SDP : ICCP Transport Selector, "00 01"
        char    AP_Title[96];       // SDP : ICCP Application Process Title, null 이면 미사용 "1 3 9999 33"
        char    AE_Qualifier[11];   // SDP : ICCP Application Entity Qualifier, null 이면 미사용 "33"
    } __attribute__ ((packed)) DB_ICCP_UNIT;
    
typedef struct
    {
    	DB_ICCP_UNIT    FEP_A;      // FEP-A ICCP Parameter
    	DB_ICCP_UNIT    FEP_B;      // FEP-B ICCP Parameter
    	DB_ICCP_UNIT    SDP_A;      // SDP-A ICCP Parameter
    	DB_ICCP_UNIT    SDP_B;      // SDP-B ICCP Parameter
    	          
    } __attribute__ ((packed)) DB_ICCP_CONFIG;


typedef struct
    {
        char    pointType;          // SDP : 연산포인트 TYPE,     [0]NULL, [1] STATUS, [2]ANALOG, [3]CONTROL
        char    useFlag;            // SDP : 연산포인트 사용유무, [0]사용않함, [1] 사용
        char    calcTime;           // SDP : 연산포인트 연산주기, sec
        char    function;           // SDP : 연산포인트 Function#
        
        char    calString[128];     // SDP : 연산식 String

    } __attribute__ ((packed)) DB_CAL_POINT;
    
/* ---------------------------------------------------- */
/* STURECTURE 정의 : RTU_DATABASE                       */
/* - SDP 자장치 구성 데이터베이스                       */
/* ---------------------------------------------------- */  

#define MAX_DB_HOST 			8           // 최대 SDP 수용 HOST 수 
#define MAX_DB_ESIO        		6           // 최대 장착 ESIO 구조체 : ESIO#0 - SIO, ESIO#1-전력감시, ESIO#2-원격진단, ESIO#3-전력품질, ESIO#4-61850, ESIO#5-RTU

#define MAX_DB_SCAN_PORT       	32          // SDP 수용 SCAN-Channel 수
#define MAX_DB_MODBUS_PROFILE  	16          // SDP 수용 MODBUS-PROFILE 수

#define MAX_DB_DEVICE          	112         // SDP 수용 최대 계전기 수
#define MAX_DB_POINT     		4096        // 데이터베이스 상의 수용 포인트 DB
#define MAX_DB_CAL_POINT       	32          // 장치당 최대 연산포인트  

typedef struct
        {
        DB_MPU_CONFIG		mpuConfig;					        // SDP : MPU Network 구성정보 & Parameter
        DB_ESIO_CONFIG      esioConfig[MAX_DB_ESIO];            // SDP : ESIO Network 구성정보 & Parameter, MAX 6
		DB_HOST_CONFIG 		hostCfg[MAX_DB_HOST];		        // SDP : HOST 구성정보 & 운영파라메터, MAX 8

        DB_ICCP_CONFIG      iccpConfig;                         // SDP : ICCP HOST 구성정보 & Parameter
        
        byte    portdb[32];                			            // SDP : HARRIS-HOST 구성정보
        byte    chassisNum;                 			        // SDP : LANDIS Chassis Number
        byte    chassis[16][16];            			        // SDP : LANDIS RTU 구성정보
        
        DB_MODBUS_PROFILE	modbusProfile[MAX_DB_MODBUS_PROFILE];  // SDP : MODBUS 프로파일 구성정보, MAX 16
        
        DB_SCAN_CONFIG	scanConfig[MAX_DB_SCAN_PORT];	            // SDP : 계전기 통신포트 구성, MAX 32
        DB_SDP_DEVICE   deviceConfig[MAX_DB_DEVICE];	            // SDP : 계전기별 운영 구성, MAX 112
        DB_POINT_BUF    pointBuf[MAX_DB_POINT]; 	            	// SDP : 운영 포인트 구성, MAX 4096
        
        DB_CAL_POINT        calPointBuf[MAX_DB_CAL_POINT];         	// SDP : 연산포인트 구성, MAX 32
		
		byte    chksum[2];
        
	    } __attribute__ ((packed)) RTU_DATABASE; // LOCAL DB 이고..
	    

typedef struct
        {
        //      DB_MPU_CONFIG2  기존 ESIO 와 호환을 위한 것..       
        DB_ESIO_MPU_CONFIG		esio_mpuConfig;					        // SDP : MPU Network 구성정보 & Parameter
        DB_ESIO_CONFIG      esioConfig[MAX_DB_ESIO];            // SDP : ESIO Network 구성정보 & Parameter, MAX 6
		DB_HOST_CONFIG 		hostCfg[MAX_DB_HOST];		        // SDP : HOST 구성정보 & 운영파라메터, MAX 8

        DB_ICCP_CONFIG      iccpConfig;                         // SDP : ICCP HOST 구성정보 & Parameter
        
        byte    portdb[32];                			            // SDP : HARRIS-HOST 구성정보
        byte    chassisNum;                 			        // SDP : LANDIS Chassis Number
        byte    chassis[16][16];            			        // SDP : LANDIS RTU 구성정보
        
        DB_MODBUS_PROFILE	modbusProfile[MAX_DB_MODBUS_PROFILE];  // SDP : MODBUS 프로파일 구성정보, MAX 16
        
        DB_SCAN_CONFIG	scanConfig[MAX_DB_SCAN_PORT];	            // SDP : 계전기 통신포트 구성, MAX 32
        DB_SDP_DEVICE   deviceConfig[MAX_DB_DEVICE];	            // SDP : 계전기별 운영 구성, MAX 112
        DB_POINT_BUF    pointBuf[MAX_DB_POINT]; 	            	// SDP : 운영 포인트 구성, MAX 4096
        
        DB_CAL_POINT        calPointBuf[MAX_DB_CAL_POINT];         	// SDP : 연산포인트 구성, MAX 32
		
		byte    chksum[2];
        
	    } __attribute__ ((packed)) ESIO_RTU_DATABASE;  /* ESIO DB */


#endif	// RTUDB_HEADER_INCLUDED
