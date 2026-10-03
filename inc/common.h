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

#ifndef	COMMON_HEADER_INCLUDED

#define	COMMON_HEADER_INCLUDED

/* ------------------------------------------------ */
/* 비츠로시스 FEP 사용시... 활성화                     		*/
/* ------------------------------------------------ */
#if 0
#define	VITZRO_FEP_ENABLE		100
#define	_VITZROSYS_FEP			100		// By CHoi...define
#endif

/* ------------------------------------------------ */
/* 원격소장치/CU 관련 상수정의                      */
/* ------------------------------------------------ */
//#define DATE_STRING         "2022.07.19"
//#define VERSION_STRING      "Ver22.07.19"
#define DATE_STRING         "2024.11.15"
#define VERSION_STRING      "Ver24.11.15"

#define SIMULATOR_NAME      "SDP-Simulator V1.1.5, 2019-11-02"

#ifdef	VITZRO_FEP_ENABLE
#define TARGET_NAME         ">>ACE-VTZ-SDP-MPU-V3 : "
#else

//#define TARGET_NAME         ">>ACE-SDP-MPU-V3 : "
#define TARGET_NAME         ">> NB-SDP-MPU-V3 : "
#endif


/* ------------------------------------------------ */
/* ESIO Network 초기화 : VMEBUS 선택                */
/* ------------------------------------------------ */
#define VME_NETWORK_INITIAL       100

/* ------------------------------------------------ */
/* ROM Version 또는 NFS Version 선택                */
/* ------------------------------------------------ */
#if 1
#define ROM_VERSION             100        
#endif

#define NETWORK_CONSOLE         200     // Network - CONSOLE   
//#define LOGFILE_DISABLE         200     // Network - CONSOLE   


#define ROMDB_STRING      "/mnt/bin/DB-SDP-MPU-A"   
#define ROMDB_STRING2     "/mnt/bin/DB-SDP-MPU-B"                

#define _PACK_ __attribute__((__packed__))      // Structure Data Size Packed...

//
// Macro
//
//#define	max(a,b)	((a) > (b) ? (a) : (b))
//#define	min(a,b)	((a) < (b) ? (a) : (b))
#define pause(x)    usleep(x*1000) // x is in mS *

//
// General Data Type
//
typedef unsigned char   BYTE;
typedef unsigned char   UCHAR;
typedef unsigned short  WORD;

typedef unsigned char   byte;
typedef unsigned char   uchar;
typedef unsigned short  word;


//
// General Constant Data Type
//
#define	NO 		        0
#define	YES		        1

#define	RESET 	        0
#define	SET		        1

#define	TRUE 	        1
#define	FAIL	        -1
#define FALSE           0

#define	OK 		        1
#define	NOK		        0

#define	ON 		        1
#define	OFF		        0

#define ERROR           -1
#define CONNECT         1

//
// Return Codes
//
#define	E_OK		    0
#define	E_ERR			-1
#define	E_SKIP			-1
#define	E_WAIT			-2

#define RTS_ON          0x02
#define RTS_OFF         0x03


/* ------------------------------------------------ */
/*  Process Index                                   */
/*  PROCESS 별 ID 부여 (실제 운영되는 프로세서 정의 */
/* ------------------------------------------------ */
#define	MAX_PROCESS		9

#define	SIM_PROCESS	    0       /* SIMULATOR 통신용 Process */
#define	SCU_PROCESS	    1       /* 이중화 절체장치 통신용 Process */
#define	LINK_PROCESS    2       /* MPU 이중화 연계 Process */
#define	SCAN_PROCESS	3       /* RTU SCAN Process */

#define	ICCP_PROCESS    4      /* ICCP-60870 HOST LINK Process */

#define	HOST1_PROCESS   5       /* DNP, HARRIS, LANDIS HOST LINK Process */
#define	HOST2_PROCESS   6       /* DNP, HARRIS, LANDIS HOST LINK Process */
#define	HOST3_PROCESS   7       /* DNP, HARRIS, LANDIS HOST LINK Process */
#define	HOST4_PROCESS   8       /* DNP, HARRIS, LANDIS HOST LINK Process */


#define	CLI_PROCESS		MAX_PROCESS
#define	WDT_PROCESS	    (MAX_PROCESS + 1)       /* WDT     통신용 Process */

/* ------------------------------------------------ */
/*  Processor Priority 생성                         */
/*  - Processor 호출시 개별 NICE 값지정 ( -19 ~ 20) */
/* ------------------------------------------------ */
#define NICE_WDT        1     // WDT 관리 프로세스
#define NICE_SCAN       2      // ESIO 연계 프로세스
#define NICE_SCU        3      // SCU(이중화절체) 연계 프로세스
#define NICE_ICCP       5      // ICCP HOST 연계 프로세스
#define NICE_HOST       5      // DNP/HARRIS HOST 연계 프로세스
#define NICE_LINK       6      // MPU 연계 프로세스
#define NICE_SIM        8      // SDP Simulator 연계 프로세스
#define NICE_CLI        9      // 사용자 Console 프로세스


//
// IPC : Shared Memory 관련
//
#define	SHM_KEY			(key_t)100

#define	PRCNAME_LEN			32
#define	FILENAME_LEN		64
#define	MESSAGE_LEN			128

/* ----------------------------- */
/* SHARERAM 관련 Structure 외부선언 */
/* ----------------------------- */ 
typedef struct 
{
	int		id;
	key_t	key;
	int		size;
	char	*address;
} SHM_DESC;

/* ----------------------------- */
/* THREAD 관련 Structure 외부선언 */
/* ----------------------------- */
#define	TASK_NAME_LEN	16

typedef struct 
{
	char	name[TASK_NAME_LEN];
	pthread_t		tid;
	int		priority;
	int		options;
	void	*entryPt;
	void	*arg;

} THREAD_ENTRY;
                
/* ----------------------------- */
/* MESSAGE 관련 Structure 외부선언 */
/* ----------------------------- */ 
#define	MSG_LENGTH		1024

typedef struct {
	long	mType;
	char	mText[MSG_LENGTH];
} MSG_BUFFER;

typedef struct {
	key_t	key;
	int		id;
	MSG_BUFFER	mBuffer;
} MSG_DESC;

/* ----------------------------- */
/* FILE 관련 Structure 외부선언 */
/* ----------------------------- */  
#define	FIO_NORMAL			0
#define	FIO_APPEND			1
#define	FIO_CREATE			2

#define	FIO_BUFLEN			(256 * 1024)
#define	FIO_DELAY_TIME		(1000)

typedef struct {
	FILE	*id;
	int		mode;
	char	name[FILENAME_LEN];
} FILE_DESC;


//
// 구조체 선언
//
#define	WDT_WAIT_COUNT			10

typedef struct
{
	char	name[PRCNAME_LEN];
	char	argument[MESSAGE_LEN];
	short	check;
	char	waitCount;
} PROCESS_DESC;

                
/* ----------------------------- */
/* TTYDESC 관련 Structure 외부선언 */
/* ----------------------------- */ 
#define	MAX_TTY_DEVICE		64
#define	MAX_TTY_DEVICE		64
#define	FILENAME_LEN		64
#define	MESSAGE_LEN			128


#define ASYNC_8530          1           // ASYNC PORT : 8530 기반
#define ASYNC_860T          2           // ASYNC PORT : 860 기반

typedef struct {
    int     chid;
	int		id;
	FILE	*fId;
	int		config;             /* device 속성 정의 (외부8530, 내부 ASYNC) */
	int     commMode;
	int		baudRate;
	int		dataBits;
	int		stopBits;
	int     lineDebug;
	char	parity;
	char	name[FILENAME_LEN];
	struct timeval  timeOut;
} TTY_DESC;


/* \033 은 escape  [1 과  31은 색상 m은 종료   */

#define COLOR_RED       "\033[1;31m"  // 색바꾸기..
#define COLOR_GREEN     "\033[1;32m"  // 색바꾸기..
#define COLOR_YELLOW    "\033[1;33m"  // 색바꾸기..
#define COLOR_BLUE      "\033[1;34m"  // 색바꾸기..
#define COLOR_PURPLE    "\033[1;35m"  // 색바꾸기..
#define COLOR_CYAN      "\033[1;36m"  // 색바꾸기..
#define COLOR_WHITE     "\033[1;37m"  // 색바꾸기..
#define COLOR_NORMAL    "\033[0m"

#define    PR_COLOR_YELLOW         printf("%s",COLOR_YELLOW);
#define    PR_COLOR_RED         printf("%s",COLOR_RED);
#define    PR_RESET             printf("%s",COLOR_NORMAL);


#endif		// COMMON_HEADER_INCLUDED



// End of common.h
//
