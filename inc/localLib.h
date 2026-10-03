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

#ifndef	LOCALLIB_HEADER_INCLUDED
#define	LOCALLIB_HEADER_INCLUDED

//
// Include Files
//
#include	<stdio.h>
#include	<unistd.h>
#include	<stdlib.h>
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	<stdarg.h>
#include 	<signal.h>
#include    <time.h>
#include 	<termios.h>
#include 	<sys/types.h>
#include 	<sys/time.h>
//#include 	<sys/io.h>
#include 	<sys/ioctl.h>
#include 	<sys/ipc.h>
#include 	<sys/shm.h>
#include 	<sys/signal.h>
#include 	<sys/stat.h>
#include 	<sys/socket.h>
#include    <sys/mman.h>
#include 	<arpa/inet.h>
#include    <linux/rtc.h>  // hello world 

#include 	<errno.h>
#include	<pthread.h>
#include 	<fcntl.h>
#include 	<unistd.h>
#include 	<ctype.h>
#include 	<math.h>


#include    "includes.h"

#include    "common.h"

#include    "m860io.h"
#include    "vmesio.h"

#include    "op_dnp.h"                  // DNP 구조체 및 관련 Header
#include    "op_harris.h"
#include    "op_rtusim.h"               // SIMULATOR 관련 Header

#include    "rtudb.h"
#include    "rtubuf.h"

#include    "op_landis.h"

#include    "usrtype.h"
#include    "logdb.h"
#include    "shmMemory.h"
#include    "extfunc.h"
#include    "iccpShm.h"

// hkkim
#include    "mylib.h"

/* ------------------------------------------------ */
/*  영상검지기용 860 Board : NET860-500             */
/*  860 Internal ASYNC -PORT Define ...             */
/* ------------------------------------------------ */
#define DEVICE_HERO_IO      "/dev/hera-io"
#define DEVICE_EX_IO        "/dev/ex-io"

#define FID_CONSOLE_PORT  	"/dev/ttyS0"

#if 0
#define FID_LINK_PORT  	    "/dev/ttyS1"	    /* CPU 링크연계 */
#define FID_SCU_PORT  	    "/dev/ttyS8"	    /* 절체장치 연계 */
#define FID_MMI_PORT  	    "/dev/ttyS6"	    /* MMI 장치 연계 */
#endif

// #define FID_LINK_PORT  	    "/dev/ttyS8"	    /* CPU 링크연계 */
// #define FID_SCU_PORT  	    "/dev/ttyS1"	    /* 절체장치 연계 */
// IMX6SX 2026-02-04 오후 1:16:17
#define FID_LINK_PORT  	    "/dev/ttymxc1"	    /* CPU 링크연계 DUSB RSV*/
#define FID_SCU_PORT  	    "/dev/ttymxc5"	    /* 절체장치 연계  RJ45 */

#define FID_MMI_PORT  	    "/dev/ttyS6"	    /* MMI 장치 연계 */


/*  External ASYNC -PORT Define ... */
#define FID_SCAN1_PORT	    "/dev/ttyS5"		/* 하위 계전기#1  */
#define FID_SCAN2_PORT	    "/dev/ttyS4"		/* 하위 계전기#2  */
#define FID_SCAN3_PORT	    "/dev/ttyS3"		/* 하위 계전기#3  */
#define FID_SCAN4_PORT	    "/dev/ttyS2" 		/* 하위 계전기#4  */

#define FID_SCAN5_PORT	    "/dev/ttyS7"		/* 하위 계전기#5  */



/* 전체 통신포트에 대한 ID */
#define FID_CONSOLE     0
#define FID_LINK        1
#define FID_SCU         2
#define FID_RFU         3
#define FID_MMI         4

/* ------------------------------------------------ */
/*  하위 계전기용 통신포트 ...(총 10개)             */
/* ------------------------------------------------ */
#define FID_SCAN1       0       // CPU 내장 SCAN #1
#define FID_SCAN2       1       // CPU 내장 SCAN #2
#define FID_SCAN3       2       // CPU 내장 SCAN #3
#define FID_SCAN4       3       // CPU 내장 SCAN #4

#define FID_SCAN5       4
#define FID_SCAN6       5
#define FID_SCAN7       6
#define FID_SCAN8       7
#define FID_SCAN9       8
#define FID_SCAN10      9


#define MAX_MPU_PORT    5

#define MPU_COM1        0       // MPU Port #1
#define MPU_COM2        1       // MPU Port #2
#define MPU_COM3        2       // MPU Port #3
#define MPU_COM4        3       // MPU Port #4
#define MPU_COM5        4       // MPU Port #5
#define MPU_COM6        5       // MPU Port #6
#define MPU_COM7        6       // MPU Port #7
#define MPU_COM8        7       // MPU Port #8

#define VME_COM1        8       // VME SIO Port #1
#define VME_COM2        9       // VME SIO Port #2
#define VME_COM3        10      // VME SIO Port #3
#define VME_COM4        11      // VME SIO Port #4
#define VME_COM5        12      // VME SIO Port #5
#define VME_COM6        13      // VME SIO Port #6
#define VME_COM7        14      // VME SIO Port #7
#define VME_COM8        15      // VME SIO Port #8


#include "tk_net_lib.h"

#endif	// LOCALLIB_HEADER_INCLUDED

//
// End of aesLib.h
//
