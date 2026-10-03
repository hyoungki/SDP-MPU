/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*      	      1998-2002, All Rights Reserved                    */
/*									*/
/* MODULE NAME : mi_usr.h    						*/
/* PRODUCT(S)  : MMSEASE-LITE						*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*    This file contains declarations needed by rt_types.c to resolve	*/
/*    variable association initiailzation statements.			*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*	NONE								*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 03/19/10  nav     23    Add miuDeleteAllDs				*/
/* 08/05/09  nav     22    Add block 8 support				*/
/* 08/04/09  MDE     21    Added micuGetAllDvAttrib			*/
/* 05/21/07  RKR     20    deleted extern decl miuRplCfg                */
/* 06/07/06  GLB     19    added mi_app_dyn_log_fun                     */
/* 03/29/06  RKR     18    added miuAlwaysChangeDev                     */
/* 07/26/05  MDE     17    2000-08 work					*/
/* 04/29/05  MDE     16    MIU_IR naming update				*/
/* 04/25/05  MDE     15    General cleanup 				*/
/* 04/06/05  MDE     14    Added quiet mode				*/
/* 02/10/04  MDE     13	   Redundancy cleanup				*/
/* 08/10/04  EJV     12	   Comment out MUI_REDUNDANCY_SUPPORT (optional)*/
/* 01/20/04  MDE     11	   Added redundancy support code		*/
/* 12/03/03  MDE     10    Linux warning cleanup			*/
/* 09/13/02  ASK     09    Added micuSendIdentify			*/
/* 06/20/02  RKR     08    Copyright year update                        */
/* 04/16/02  MDE     07	   Moved mics_icfg related to (new) mics_icfg.h	*/
/* 03/25/02  MDE     06	   Added tolerance of mis-configured DS		*/
/* 01/25/02  MDE     05	   Added redundancy support code		*/
/* 10/29/01  MDE     04	   Added USE_ICFG code				*/
/* 10/19/01  MDE     03	   Minor cleanup				*/
/* 10/12/99  NAV     02    Add Block 4 and 5 support			*/
/* 12/31/97  MDE     01    New file					*/
/************************************************************************/

#ifndef MI_USR_INCLUDED
#define MI_USR_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/

#include "glbtypes.h"
#include "sysincs.h"
#include "mi.h"
#include "mi_icfg.h"
#include "mi_b8.h"

/* #define MUI_REDUNDANCY_SUPPORT */

#ifdef MUI_REDUNDANCY_SUPPORT
#include "rpl.h"
#endif

#include "rtubuf.h"
#include "usrtype.h"
#include "logdb.h"

/************************************************************************/

#if 0	// CHOIBC DELETE
extern ICCP_DCB *pIccpDcb;
#endif

extern ST_BOOLEAN miuDoIt;
#if 0	// CHOIBC DELETE
extern ST_BOOLEAN miuFullSpeed;
extern ST_BOOLEAN miuAcrTest;
extern ST_BOOLEAN miuAlwaysChangeDv;
extern ST_BOOLEAN miuAlwaysChangeDev;
#endif

extern ST_CHAR   *miuLccName;
extern ST_BOOLEAN miuQuiet;

#if 0	// CHOIBC DELETE
extern ST_BOOLEAN miuDeleteAllDs;
extern ST_BOOLEAN miuNeedDelAllDs;
#endif

#if 1	// 2016.05.24 ChoiBC Modify : For QPRINTF:
extern  OPR_MSG         *opr;
extern  CONSOLE_INFO	*console;
#define QPRINTF(fmt, args...) if(opr->iccpDebug)Debug(console, fmt, ## args)
#else
#define QPRINTF if(!miuQuiet)printf
#endif

#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
//
// definition of ICCP Information
//
typedef enum
{
	SDP_STAGE_ENABLE,				// Association을 Enable 시킨다.
//	SDP_STAGE_WAIT_ACTIVE_SYSTEM,	// FEP이 Active가 되기를 기다림
	SDP_STAGE_WAIT_ASSOCIATE,		// Associate Enable 요청 후 기다림
	SDP_STAGE_DISABLE,				// 에러가 발생하여 Association을  Disable 시킨다.
									// 일정 시간동안 링크로 데이터 또는 Identify가 오지 않으면
	SDP_STAGE_WAIT_3SECONDS,		// Disable 시킨 후, 3초간 쉰다. (쉬지 않으면 끊자 마다 다시 붙이다가 다시 에러가 무한 반복할 수 있다)
	SDP_STAGE_RUN					// Associated
}	SDP_STAGE_ENUM;

typedef struct
{
	SDP_STAGE_ENUM		eStage;
	ST_DOUBLE			dTimeWait;		// not used
										// 10초 동안 Stage가 변하지 않으면 다음 Stage로 바꾼다.
	ST_DOUBLE			dTimeForceDisableAssoc;		// 최근 강제로 접속을 끊은 시간
	ST_DOUBLE			dTimeLastDataReceived;		// 최근 데이터 읽기 요구 시간
	ST_DOUBLE			dTimeLastIdentifyReceived;	// 최근 Identify 받은 시간
	ST_DOUBLE			dTimeLastCommandReceived;	// 최근 Command(Select, Operate) 받은 시간
} INFO_ICCP_ST;

INFO_ICCP_ST *getIccpInfo (void);

#endif


/*
 *	proto-type definition for mis_usr.c
*/
ST_INT misuChangeDataValue (ICCP_POINT_INFO *pPnt, ICCP_POINT_DATA *pData, ST_BOOLEAN bSoe);
#ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
ST_INT misuCheckCOA (ST_INT iccpPointType);
#endif

/* Server activity simulation */
#if 0	// CHOIBC DELETE
ST_VOID miuAutoService (ST_VOID);
ST_VOID misuChangeNextDv (ST_VOID);
ST_VOID misuChangeNextDev (ST_VOID);
ST_VOID misuSendMessage (ST_VOID);
ST_VOID misuSendBlock8 (MI_CONN *mi_conn);
#endif	// CHOIBC DELETE

/* For Client tests */
ST_VOID micuStartAssocActivity (MI_CONN *mi_conn);
ST_VOID micuStopAssocActivity (MI_CONN *mi_conn);

ST_VOID micuReadAllDv (MI_CONN *mi_conn);
ST_VOID micuReadAllDs (MI_CONN *mi_conn);
ST_VOID micuWriteDv (MI_CONN *mi_conn);
ST_VOID micuTestAllDevices (MI_CONN *mi_conn);
ST_VOID micuGetAllDvAttrib (MI_CONN *mi_conn);

/* For Redundancy */
#ifdef MUI_REDUNDANCY_SUPPORT
extern ST_BOOLEAN miuRedundancyEnable;

ST_VOID miuAddIrCfgGroup (ST_VOID);
ST_RET  miuIrStart (ST_VOID);
ST_VOID miuIrTerminate (ST_VOID);
ST_VOID miuIrService (ST_VOID);
ST_VOID miuIrAssociationActive   (MI_ASSOC_CTRL *mi_assoc);
ST_VOID miuIrAssociationInactive (MI_ASSOC_CTRL *mi_assoc);
ST_VOID miuIrRequestActive  (ST_VOID);
ST_VOID miuIrRequestStandby (ST_VOID);
ST_VOID miuIrMisDvUpdate (MIS_DV_REF dvRef, ST_INT mi_type, ST_VOID *data);
ST_VOID miuIrMicDvUpdate (MI_REMOTE *mi_remote, ST_INT idx, ST_INT type_id, ST_VOID *data);

ST_BOOLEAN miuIrKeyService (ST_CHAR c);
ST_VOID    miuIrKeyHelp (ST_VOID);
#endif

/* For redirecting SLOG messages */
ST_VOID 	mi_app_dyn_log_fun (LOG_CTRL *lc,
				     SD_CONST ST_CHAR *timeStr,
			     	     SD_CONST ST_INT logType,
			     	     SD_CONST ST_CHAR *SD_CONST logTypeStr,
                             	     SD_CONST ST_CHAR *SD_CONST sourceFile,
                             	     SD_CONST ST_INT lineNum,
                             	     SD_CONST ST_INT bufLen,
                             	     SD_CONST ST_CHAR *buf);

/*
 *	proto-type definition for mi_main.c
*/
ST_VOID miuCheckEventQueue (ST_VOID);
#ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
ST_CHAR *getMpuSwitchDevName (ST_VOID);
ST_VOID miuCheckAnalogChanges (ST_VOID);
#endif

ST_VOID miuService (ST_INT *maxWaitTimeIo, ST_CHAR *appName);
ST_VOID miuTerminate (ST_CHAR *appName);
ST_RET miuInitIccp (ST_CHAR *appName, ST_INT deviceIndex, ICCP_DCB *dcb);

/*
 *	proto-type definition for iccpMain.c
*/
int getDeviceData (ICCP_POINT_INFO *pI, ICCP_POINT_DATA *pData);

#if 0	// CHOIBC DELETE
/************************************************************************/
/* block 8 support							*/
/************************************************************************/

extern ST_BOOLEAN miuBlock8Client;
extern ST_BOOLEAN miuBlock8Server; 

ST_VOID miuEnableBlock8 (ST_VOID);
#endif

/************************************************************************/
/* SDP DB							*/
/************************************************************************/

#ifdef _WIN32
#define	CONFIG_DIR		"."
#define	LOG_DIR			"."
#elif _PLATFORM_LINUX
#define	CONFIG_DIR		"config"
#define	LOG_DIR			"log"
#else
#define	CONFIG_DIR		"/mnt/config"
#define	LOG_DIR			"/mnt/log"
#endif

#define	APP_NAME_A	"APP_SDP_A"
#define	APP_NAME_B	"APP_SDP_B"

#define	DEVICE_INDEX_A		0
#define	DEVICE_INDEX_B		1



#ifdef _WIN32
#define	snprintf(_sdt, _cnt, _format, ...)	_snprintf(_sdt, _cnt, _format, __VA_ARGS__)
#endif

#if 1	// 2016.11 ChoiBC Modify : SOE
//
// SOE가 발생하면 STATE_SUPP_Q_TIMETAG_EXTENDED 의 ss에 아래 Flag를 Set 한다. 
//
#define	MI_DATA_STATE_SUPP_SOE	0x08
#endif

typedef struct
{
	ST_CHAR	name[MAX_IDENT_LEN+1];		
	ST_LONG	maxDsTs;
	ST_LONG	maxDs;
	ST_LONG	maxMmsPduSize;
} CFG_ICCP_LOCAL_CC_ST;

typedef struct
{
	ST_CHAR name[MI_MAX_BLT_ID_LEN+1];
	ST_CHAR id[MI_MAX_BLT_ID_LEN+1];
	ST_CHAR localDom[MAX_IDENT_LEN+1];
	ST_CHAR remoteDom[MAX_IDENT_LEN+1];
	ST_LONG shortestInterval; 
} CFG_ICCP_BLT_ST;

typedef struct
{
	ST_LONG   initiateTimeout;
	ST_LONG   concludeTimeout;
	ST_LONG   heartbeatTime;
	ST_LONG   maxMmsPduSize;
	ST_INT16  maxReqPend;
	ST_INT16  maxIndPend;
	ST_INT8   maxNest;			// >= 0
#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
	ST_LONG   identifyTimeout;		// if 0, not check
#endif
} CFG_ICCP_ASSOCIATION_ST;

#define	INDEX_SDI	0
#define	INDEX_SDO	1
#define	INDEX_SAI	2
#define	INDEX_DDI	3
#define	INDEX_DAI	4
#define	INDEX_QDI	5
#define	INDEX_QAI	6
#define	INDEX_TDI	7
#define	INDEX_TAI	8
#define	INDEX_DEV	9

#if 0	// CHOIBC DELETE
//
// SDP Point Reference
//		SDP_POINT_TYPE | SDP_POINT_NO(1~)
#define	SET_POINT_REF(t, n)		((t) << 24 | (n))
#define	GET_POINT_TYPE(r)		((r) >> 24)
#define	GET_POINT_NO(r)			((r) & 0xFFFFFF)
#endif


typedef struct
{
	ST_CHAR		name[8];		
	ST_INT		numberWidth;
	ST_INT		count;		// not used
} CFG_SDP_POINT_ST;

typedef struct
{
	ST_CHAR				scope;		// 0:VCC_SPEC, 1:ICC_SPEC
	CFG_SDP_POINT_ST	point[MAX_SDP_POINT_TYPE];
#ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
	ST_CHAR				MpuSwitchDevName[MAX_IDENT_LEN+1];
	ST_INT				ChangeOfAnalogPeriod;
#endif
} CFG_ICCP_SERVER_OBJ_ST;


typedef struct
{
	ST_CHAR	name[MAX_IDENT_LEN+1];		
	ST_INT	version;		/* MI_VERSION_1996, MI_VERSION_2000 */
	CFG_ICCP_BLT_ST			blt;
	CFG_ICCP_ASSOCIATION_ST	assoc;
	CFG_ICCP_SERVER_OBJ_ST	serObj;
} CFG_ICCP_REMOTE_CC_ST;

//
// definition of iccp.conf
//
typedef struct
{
	CFG_ICCP_LOCAL_CC_ST	local;
	CFG_ICCP_REMOTE_CC_ST	remote;
} CFG_ICCP_FILE_ST;


#if 0	// CHOIBC DELETE
/* ICCP Data Value Type */
#define	ICCP_DV_TYPE_STATE		0
#define	ICCP_DV_TYPE_DISCRETE	1
#define	ICCP_DV_TYPE_REAL		2

/* ICCP Device Type */
#define	ICCP_DEV_TYPE_COMMAND	0
#define	ICCP_DEV_TYPE_DISCRETE	1
#define	ICCP_DEV_TYPE_REAL		2
#endif

#if 1	// 2016.11 ChoiBC MPU 절체 명령 추가
// MPU 절체 명령 추가
#define	MPU_SWITCH_DEV_NAME		"MPU_SWITCH"
#define	SYSTEM_DEVICE_COUNT		1
#endif

/* SBO 제어시 Select Timeout */
#define	ICCP_DEV_SELECT_TIMEOUT	3	// seconds

// misuChangeDataValue () 리턴 값
#define MY_DV_CHANGE_OK			0	// MIS_DV_CHANGE_OK
#define MY_DV_CHANGE_LOST		1	// MIS_DV_CHANGE_LOST
#define MY_DV_CHANGE_FULL		2	// MIS_DV_CHANGE_FULL
#define	MY_DV_CHANGE_NOT_FOUND	3
#define	MY_DV_CHANGE_NOT_DSTS	4

/*
	proto-type definition for miu_icfg.c
*/
ST_RET my_iccp_config (ST_INT deviceIndex, ICCP_POINT_DEF *pPointDef, CFG_ICCP_FILE_ST *pCfg);
ST_VOID my_iccp_config_file_load (CFG_ICCP_FILE_ST *pCfg);

/************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* MI_USR_INCLUDED */
/************************************************************************/

