/*
 * ==============================================================
 * System TARGET : ACE Control SDP-2000-V1.0   
 * Target CPU    : P1011, SDP
 * Main Factors  : 스마트급전제어시스템 - 자장치
 *          - CPU 이중화 구성
 * --------------------------------------------------------------
 * System DESIGN : SANE-SYSTEM   .... by  Lee Ho-Sang
 * Initial-DATA  : 2017,11,01
 * Last Updated  : 
 * ==============================================================
 */

#ifndef	SHMMEMORY_HEADER_INCLUDED

#define	SHMMEMORY_HEADER_INCLUDED

//
// Include Files
//
#include "common.h"


typedef struct
{
	pid_t	pid;
	int		define;
	int		check;
	int		active;
	int     hostid;
	
	int     debug;
	int     restart;
	int     initial;
	int		wdtEnable;
	int     wdtCount;
	int		lastWdt;
	int		prevEtime;
	int		maxEtime;
	char	name[16];
#if 0 // hkkim 2026-02-10 오전 9:34:44	
	
} __attribute__ ((packed)) TASK_INFO;
#else
} TASK_INFO;
#endif 


#define	PORTNAME_LEN		32

typedef struct
{
	char	name[PORTNAME_LEN];
	int		baudRate;
	BYTE	byteSize;
	BYTE	stopBit;
	BYTE	parity;
	BYTE	resetCom;
} __attribute__ ((packed)) PORT_INFO;


/* ---------------------------------------------------- */
/* STURECTURE 정의 : SHM_MEMORY                         */
/* - SDP 자장치 : 공유메모리                            */
/* ---------------------------------------------------- */  

typedef struct
{
    
	TASK_INFO	    taskInfo[MAX_PROCESS + 2];  

// 2026-06-01 오후 4:23:50 맨아프으로  unpacked
	SDP_DEVICE      deviceCFG[MAX_DEVICE];              // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)

	
	RTC             rtc; 
	OPR_MSG         opr_msg;    /* unpacked */                        
	
	CONSOLE_INFO	console;

 //   HOST_DCB        hostDCB[MAX_HOST];                  // HOST 관련 구조체 : DNP, HARRIS, LANDIS...


    /* -------------------------------- */
	/* ICCP 운영 버퍼 Queue             */
	/* -------------------------------- */
    ICCP_DCB        iccpDCB;                            // ICCP-HOST 운영 버퍼
    ICCP_60870_DCB  iccpInfo;                           // ICCP-HOST 참조용 : 모니터링 구조체
    
    MPU_CONFIG      mpuConfig;                          // MPU Config
    ESIO_CONFIG     esioConfig[MAX_ESIO];               // ESIO 장치 Config
    
    LINK_MSG        link_msg;                           // CPU 이중화 구조체
    SCU_MSG         scu_msg;                            // 이중화 절체장치 구조체 

#if 1 // hkkim 위치이동.
    /* -------------------------------- */
	/* HOST 운영 버퍼 Queue             */
	/* -------------------------------- */
    HOST_DCB        hostDCB[MAX_HOST];                  // HOST 관련 구조체 : DNP, HARRIS, LANDIS...
    
#endif     
    /* HARRIS-HOST 데이터 구조 */
    RTU             rtubuf[MAX_HARRIS_RTU];         /* HARRIS RTU Structure */
	PORT_DB         portdb[MAX_HARRIS_PORT];        /* HARRIS #1 PORT Structure */
		
    /* -------------------------------- */
	/* 계전기 SCAN 버퍼 Queue           */
	/* -------------------------------- */
	SCAN_CONFIG     scanCFG[MAX_SCAN_PORT];             // 하위계전기 SCAN Config
// 2026-06-01 오후 4:23:34 맨 앞으로..
//	SDP_DEVICE      deviceCFG[MAX_DEVICE];              // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)

	POINT_BUF       devPtBuf[MAX_DEV_POINT];            // 장치 포인트용 구조체
	CAL_POINT_BUF   calPtBuf[MAX_CAL_POINT];            // 연산 포인트용 구조체

    RTU_DATABASE    rtuDatabase;                        // SDP 데이터베이스
    
    /* -------------------------------- */
	/* 시스템 운영 이벤트 Queue         */
	/* -------------------------------- */
	MPU_SOE_QUEUE   mpuSoeQueue;
	//MPU_COS_QUEUE   mpuCosQueue;
	
	
    HISTORY_QUE     localHistoryQ;                      // CONSOLE 용 LOCAL Event (SOE...)
    
#ifdef __PPC_ARCH__   //hkkim
} __attribute__ ((packed)) SHM_MEMORY;
#else 
} SHM_MEMORY;
#endif 

#endif		// SHMMEMORY_HEADER_INCLUDED

//
// End of shmMemory.h
//
