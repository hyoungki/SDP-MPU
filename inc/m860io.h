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
 
#define ARMAGEDDON_LED_MAGIC 'O' 

#define	LED_RUN		1
#define	LED_STP		2
#define	LED_FLS		3
#define	LED_CM		4
#define	LED_ALL		5

#define	LED_WDT     6

#define LED_RUN_OFF _IO(ARMAGEDDON_LED_MAGIC,1)
#define LED_RUN_ON  _IO(ARMAGEDDON_LED_MAGIC,2)
#define LED_STP_OFF _IO(ARMAGEDDON_LED_MAGIC,3)
#define LED_STP_ON  _IO(ARMAGEDDON_LED_MAGIC,4)
#define LED_CM_OFF  _IO(ARMAGEDDON_LED_MAGIC,5)
#define LED_CM_ON   _IO(ARMAGEDDON_LED_MAGIC,6)
#define LED_FLS_OFF _IO(ARMAGEDDON_LED_MAGIC,7)
#define LED_FLS_ON  _IO(ARMAGEDDON_LED_MAGIC,8)


#define LED_ONL_OFF _IO(ARMAGEDDON_LED_MAGIC,9)
#define LED_ONL_ON  _IO(ARMAGEDDON_LED_MAGIC,10)
#define ALLLED_OFF  _IO(ARMAGEDDON_LED_MAGIC,11)
#define ALLLED_ON   _IO(ARMAGEDDON_LED_MAGIC,12)

#define LED_WDT_CLR     _IO(ARMAGEDDON_LED_MAGIC,13)
#define LED_WDT_CTR_ON  _IO(ARMAGEDDON_LED_MAGIC,14)
#define LED_WDT_CTR_OFF _IO(ARMAGEDDON_LED_MAGIC,15)

#define LED_MAXNR  	15

#if 0
#define CRUTCHES_IOCTL_MAGIC 'O'
#define CRUTCHES_WDT_CTR_ON   _IO(CRUTCHES_IOCTL_MAGIC,1)
#define CRUTCHES_WDT_CTR_OFF  _IO(CRUTCHES_IOCTL_MAGIC,2)
#define CRUTCHES_WDT_CTR_CLR  _IO(CRUTCHES_IOCTL_MAGIC,3)
#define CRUTCHES_IOCTL_MAXNR  3
#endif


#define PMU_IOCTL_MAGIC 'O'
#define PMU_WDT_CTR_ON   _IO(PMU_IOCTL_MAGIC,1)
#define PMU_WDT_CTR_OFF  _IO(PMU_IOCTL_MAGIC,2)
#define PMU_WDT_CTR_CLR  _IO(PMU_IOCTL_MAGIC,3)
#define PMU_IOCTL_MAXNR  3

typedef struct {
    int     id;
    FILE    *fId;
    int    	parity;
    char    name[32];
   	int		attr;
} MPC860IO_DESC;





/*
 *	Description of Z8530 Z85C30 and Z85230 communications chips
 *
 * Copyright (C) 1995 David S. Miller (davem@caip.rutgers.edu)
 * Copyright (C) 1998 Alan Cox <alan@redhat.com>
 */

#ifndef _Z8530_H
#define _Z8530_H

/************************************
  SCC definition for Odyssey  1. 0 
*************************************/

#define  CHAN_A_DATA    0x03 
#define  CHAN_A_CMD     0x02
#define	 CHAN_B_DATA    0x01
#define  CHAN_B_CMD     0x00

#define  SCCBASE        0x10000000
#define  SCCSIZE        0x1000

/********************************

  A30    A31
  0      0   
  /B     /C   B-channel command  0
  0      1    B-channel data     1
         D 
  1      0    A-channel command  2
  1      1    A-channle data     3
**********************************/


/* ------------------------------------ */
/* SCLK: 7.3728Mhz, 1/16 BG input       */
/* ------------------------------------ */
/* 82530 ASYNC I/O  BAUD RATE CONSTANT  */

#define  SCC57600     2
#define  SCC38400     4
#define  SCC9600      22
#define  SCC2400      94
#define  SCC19200     10


#define     OdysseySET    1
#define     OdysseyRESET  0 


#define    MIN(a,b)   ( (a<b) ? a:b )

#define RTS_SET    0x02
#define RTS_CLEAR  0x00	

#define  SCC_IOC_MAGIC 'O'

#define SCC_IOC_SET_BAUDRATE    _IO(SCC_IOC_MAGIC,1)
#define SCC_IOCAUTO             _IO(SCC_IOC_MAGIC,2)
#define SCC_IOC_AUTO            _IO(SCC_IOC_MAGIC,2)
#define SCC_IOCLOOPBACK         _IO(SCC_IOC_MAGIC,3)
#define SCC_IOCRXQUEUE          _IO(SCC_IOC_MAGIC,4)
#define SCC_IOC_CHANNEL_RESET   _IO(SCC_IOC_MAGIC,5)
#define SCC_IOC_RTSCTRL         _IO(SCC_IOC_MAGIC,6)

#define SCC_IOC_SET_PARITY      _IO(SCC_IOC_MAGIC,7)
#define SCC_IOC_SET_DATASIZE    _IO(SCC_IOC_MAGIC,8)
#define SCC_IOC_SET_STOPBIT     _IO(SCC_IOC_MAGIC,9)
#define SCC_IOC_GET_CTS         _IO(SCC_IOC_MAGIC,10)

#define SCC_IOC_MAXNR  10


#ifndef __SVME_H__
#define __SVME_H__

#include <linux/ioctl.h>

#define SVME_IOC_MAGIC	'S'

#define SVME_IOC_HARDWARE_RESET		_IO  (SVME_IOC_MAGIC, 0)
#define SVME_IOCS_WATCHDOG_ENABLE	_IOW (SVME_IOC_MAGIC, 1, int)
#define SVME_IOC_WATCHDOG_STROBE	_IO  (SVME_IOC_MAGIC, 2)
#define SVME_IOCS_LED_STATUS		_IOW (SVME_IOC_MAGIC, 3, int)
#define SVME_IOCG_SWITCH_STATUS		_IOR (SVME_IOC_MAGIC, 4, int)

#define SVME_MMAP_ID_NVRAM		(0x1000 * 0)
#define SVME_MMAP_ID_SRAM		(0x1000 * 1)
#define SVME_MMAP_ID_VMEBUS		(0x1000 * 2)

#endif /* __SVME_H__ */

#endif /* !(_Z8530_H) */
