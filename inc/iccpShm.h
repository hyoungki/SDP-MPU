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

#ifndef	ICCPSHM_HEADER_INCLUDED
#define	ICCPSHM_HEADER_INCLUDED

//
// Include Files
//
#include "localLib.h"
#include "filelock.h"

/*
 * ICCP shared memory LOCK/UNLOCK
 */
#define	ICCP_SHM_LOCK_FNAME		"/tmp/iccpshm.lock"

extern int iccp_shm_lock_fd;     /* shm lock file : global variable */

#define ICCP_SHM_LOCK_READ() \
	file_lock_read	(iccp_shm_lock_fd, 0, 1)
#define ICCP_SHM_LOCK_WRITE() \
	file_lock_write	(iccp_shm_lock_fd, 0, 1)
#define ICCP_SHM_UNLOCK() \
	file_unlock		(iccp_shm_lock_fd, 0, 1)


/*
 *	proto-type definition for lib_iccpShm.c
*/
char *sprt_time_t (char *out, int outSize, time_t *pTime);
char *sprt_timeval (char *out, int outSize, struct timeval *pTime);

int	iccpShmInit (ICCP_DCB *iccpShm);
void iccpShmEnd (void);

int	iccpShmDcbInit (SHM_MEMORY *shmPtr);

void iccpShmSetAssociationInit (void);
void iccpShmSetAssociationEnd (void);
void iccpShmSetAssociationActive (int ArIndex, time_t lastTime);
//int iccpShmIsAssociationActive (void);
void iccpShmSetAssociationInactive (int ArIndex, time_t lastTime);
void iccpShmSetCommMode (int commMaster, time_t lastTime);
int iccpShmIsCommMaster (void);
void iccpShmReceivedIdentify (time_t lastTime);
void iccpShmSetLastDataSendTime (time_t lastTime);

#if 0	// CHOIBC DELETE
ICCP_POINT_INFO	*iccpShmGetCosEntry (ICCP_COSQ_ENTRY *pEntry);
int	iccpShmResetSendingCosEntry (ICCP_COSQ_ENTRY *pEntry);
void iccpShmResetSendingCosAllEntry (void);
int	iccpShmAddCosEntry (ICCP_COSQ_ENTRY *pEntry);
int	iccpShmDeleteCosEntry (ICCP_COSQ_ENTRY *pEntry);
#endif	// CHOIBC DELETE

ICCP_POINT_INFO	*iccpShmGetSoeEntry (ICCP_SOEQ_ENTRY *pEntry);
int	iccpShmResetSendingSoeEntry (ICCP_SOEQ_ENTRY *pEntry);
void iccpShmResetSendingSoeAllEntry (void);

//int	iccpShmAddSoeEntry (ICCP_SOEQ_ENTRY *pEntry);
int	iccpShmAddSoeEntry (SHM_MEMORY *shmPtr, ICCP_SOEQ_ENTRY *pEntry);

int	iccpShmDeleteSoeEntry (ICCP_SOEQ_ENTRY *pEntry, int sendToOther);

int	iccpShmAddSoeEntryDelete (ICCP_SOEQ_ENTRY *pEntry);

int	iccpShmGetPointInfo (int ref, ICCP_POINT_INFO *pInfo);
int	iccpShmGetPointData (int ref, ICCP_POINT_DATA *pData);

int iccpShmGetControlData (ICCP_CONTROL_DATA *pControl);
int iccpShmSetControlData (ICCP_CONTROL_DATA *pControl);
int iccpShmResetControlData (void);

#endif	// ICCPSHM_HEADER_INCLUDED
