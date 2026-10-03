/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*      	1998 - 2011, All Rights Reserved		        */
/*									*/
/* MODULE NAME : mi_main.c    						*/
/* PRODUCT(S)  : ICCP TASE.2 Extensions for MMS-EASE Lite		*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*	NONE								*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 03/21/11  EJV           u_mi_association_active: chk if rem_mace set.*/
/* 01/28/11  EJV           _miuService: mv S_SEC_ENABLED fun from mi.c.	*/
/* 01/14/11  EJV	   Check return from mi_initialize.		*/
/* 01/04/11  EJV	   _miuInitIccp: set saveTagVals to SD_TRUE.	*/
/*			     Added sscLogMaskMapCtrl			*/
/* 08/05/10  EJV	   main, _miuService, _miuTerminate: add appName*/
/*			   Call mi_initialize after sscConfigMvl.	*/
/*			   Added S_SEC_ENABLED and SISCO_STACK_CFG code.*/
/* 08/04/10  RKR           Check assoc pointer before sending Block 8   */
/* 07/21/10  NAV           Change Get Attributes of all DVR from A to G */
/* 07/01/10  RKR           Added -D to _miuSetIccpMode                  */
/* 05/03/10  JRB	   Fix multiple binding.			*/
/*			   Add back osicfgx if !defined(SISCO_STACK_CFG)*/
/* 03/19/10  NAV     64    Add DeleteAllDataSets test			*/
/* 01/12/10  JRB     63    Replace osicfgx call with sscConfigMvl (i.e.	*/
/*			   use "siscostackcfg.xml" to configure).	*/
/*			   _miuInitIccp: add appName arg, return ST_RET	*/
/*			   instead of exiting on error.			*/
/*			   Del MAP30_ACSE code.				*/
/*			   Del refs to secManCfgXmlFile (obsolete).	*/
/* 08/26/09  nav     62    printf format change for Linux		*/
/* 08/05/09  nav     61    Add Block 8					*/
/* 08/04/09  MDE     60    Added micuGetAllDvAttrib			*/
/* 01/27/08  MDE     59    Set SLOG command handler			*/
/* 09/03/08  RKR     58    added call to slog_ipc_std_cmd_service       */
/* 08/25/08  RKR     57    put DEBUG_SISCO around logcfgx_ex            */
/* 06/07/06  GLB     56    Example of:  Redirecting SLOG messages       */
/* 05/22/06  GLB     55    Reset DSTS back to not in use                */
/* 03/29/06  RKR     54    added miuAlwaysChangeDev defaulted to OFF    */
/* 03/17/06  RKR     53    Call slog_start to support remote logging    */
/* 03/10/06  GLB     52    Implemented 'U' and 'M' menu options         */
/* 01/30/06  GLB     51    Integrated porting changes for VMS           */
/* 10/21/05  MDE     50    Always change by default, -Q cmd line option	*/
/* 07/26/05  MDE     49    2000-08 work					*/
/* 07/13/05  MDE     48    Added u_mi_reject_ind			*/
/* 07/05/05  MDE     47    Use wasActive flag (MI_ASSOC) for redundancy	*/
/* 06/14/05  MDE     46    More LINUX warning cleanup			*/
/* 06/07/05  EJV     45    Set gsLogMaskMapCtrl if S_MT_SUPPORT defined	*/
/* 06/06/05  MDE     44    LINUX warning cleanup			*/
/* 05/31/05  PLM     43    Added debug_SISCO around EJV's		*/
/*				 logCfgAddMaskGroup (&xxxLogMaskMapCtrl)*/
/* 05/24/05  EJV     42    Added logCfgAddMaskGroup (&xxxLogMaskMapCtrl)*/
/*			   Use logcfgx_ex to parse logging config file.	*/
/* 05/20/05  MDE     41    Fix keyboard input: Use getch on WIN32, QNX.	*/
/*			   Use term_init, term_rest, getchar elsewhere.	*/
/* 04/29/05  MDE     40    Use m_debug_sel for DEBUG_SISCO		*/
/* 04/25/05  MDE     39    General cleanup 				*/
/* 04/06/05  MDE     38    Added stats print/reset keys			*/
/* 04/06/05  MDE     37    Added test probes				*/
/* 04/06/05  MDE     36    Added quiet mode				*/
/* 03/04/05  MDE     35	   Added event handling for MAP30_ACSE		*/
/* 02/10/05  MDE     34	   Redundancy cleanup				*/
/* 01/10/05  MDE     33    Statistics work				*/
/* 08/09/04  EJV     32	   Added keyboard handling on for AIX.		*/
/*			   secManCfgXmlFile: all lowcase "secmancfg.xml"*/
/*			   Added ifdef around  _getCfgTest.		*/
/* 05/28/04  MDE     31	   Tweaked key handling				*/
/* 01/20/04  MDE     30	   Added redundancy support code		*/
/* 12/10/03  MDE     29    Support MAP30_ACSE security			*/
/* 12/03/03  MDE     28    Linux warning cleanup			*/
/* 08/25/03  GLB     27    Don't buffer output                          */
/* 07/01/03  MDE     26    Removed mi_def_heartbeat_time		*/
/* 06/13/03  MDE     25    Added Security				*/
/* 02/24/02  CRM     24	   Added "defined(linux)" code 		   	*/
/* 09/13/02  ASK     23	   Added send identify req functionality   	*/
/* 06/20/02  MDE     22	   Removed setting matching control variables	*/
/* 06/20/02  RKR     21    Copyright year update                        */
/* 06/20/02  MDE     11	   Now use default matching (selectors too)	*/
/* 04/16/02  MDE     10	   Now use mlog					*/
/* 03/21/02  MDE     09	   Added Association State request		*/
/* 02/25/02  MDE     08    Changes for updated MVL_CFG_INFO		*/
/* 02/20/02  MDE     07    Now work with Marben stack too		*/
/* 01/25/02  MDE     06	   Added redundancy support code		*/
/* 10/19/01  MDE     05	   Minor cleanup				*/
/* 10/13/99  NAV     04    Suport for Block4 and Block 5		*/
/* 04/12/99  MDE     03    Minor cleanup				*/
/* 04/01/99  MDE     02    Use MI_ASSOC_CTRL for connection management	*/
/* 01/28/98  MDE     01    New module					*/
/************************************************************************/
/************************************************************************/

#include "glbtypes.h"
#include "sysincs.h"
#include "time.h"
#include "signal.h"
#if defined(_WIN32)
#include "conio.h"
#else
#include "fkeydefs.h"
#endif

#include "mvl_acse.h"
#include "mvl_log.h"
#include "mloguser.h"
#if defined(_WIN32)		// 2016.04.26 ChoiBC Modify
#include "miusrobj.WIN.h"
#elif defined(_PLATFORM_LINUX)
#include "miusrobj.LINUX.h"
#else
#include "miusrobj.PPC.h"
#endif
#include "mi.h"
#include "mi_b8.h"
#include "mi_usr.h"
#include "mics_cfg.h"
#define SISCO_MI_B8
#include "mitypeid.h"	/* include in one user source file only! */

#include "sx_defs.h"
#include "str_util.h"

#include "fkeydefs.h"
#ifdef kbhit	/* CRITICAL: fkeydefs may redefine kbhit. DO NOT want that.*/
#undef kbhit
#endif

#if defined(SISCO_STACK_CFG)
#include "sstackcfg.h"
#endif
#if defined(S_SEC_ENABLED)
#include "mmslusec.h"
#endif

#if defined(_S_TEST_PROBES)
#include "stestprb.h"
#endif
//#include "dataval2.h"

#include "iccpShm.h"

/************************************************************************/
/* For debug version, use a static pointer to avoid duplication of 	*/
/* __FILE__ strings.							*/
/************************************************************************/

#ifdef DEBUG_SISCO
static ST_CHAR *thisFileName = __FILE__;
#endif

/************************************************************************/

#define MIU_MAX_EVENT_WAIT_TIME 	100
#define MIU_MAX_MVL_SERVICE_LOOPS	10

/************************************************************************/

/* Identify server information						*/
#if 1	// 2016.11 ChoiBC Modify : Vendor, Model, Version
IDENT_RESP_INFO miuIdentRespInfo =
{
	//"Ace Control",	/* Vendor 	*/
	"Vendor-SDP",	/* Vendor 	*/
	"ICCP-SDP",		/* Model  	*/
	"1.0.0.0",		/* Version 	*/
	0
};
#endif

/************************************************************************/
/************************************************************************/
/* General program control variables */

ST_BOOLEAN miuDoIt      	= SD_TRUE;
ST_BOOLEAN miuQuiet        	= SD_FALSE;
ST_BOOLEAN miuRedirectLogging   = SD_FALSE;
#if 0	// CHOIBC DELETE
ST_BOOLEAN miuFullSpeed 	= SD_FALSE;
ST_BOOLEAN miuAcrTest	 	= SD_FALSE;
ST_BOOLEAN miuAlwaysChangeDv	= SD_TRUE;
ST_BOOLEAN miuAlwaysChangeDev	= SD_FALSE;
ST_INT miuCfgSource		= MIU_CFG_SOURCE_XML;
ST_CHAR *miuLccName;
ST_BOOLEAN miuBlock8Client	= SD_FALSE;
ST_BOOLEAN miuBlock8Server	= SD_FALSE; 
ST_BOOLEAN miuDeleteAllDs       = SD_FALSE;
ST_BOOLEAN miuNeedDelAllDs	= SD_FALSE;
#endif

#if 1	// 2016.04.07 ChoiBC Modify : Replace ICCP Config Info from DB
// iccp configuration informaion from iccp.conf file
static CFG_ICCP_FILE_ST iccpFileCfg;

#if 0	// CHOIBC DELETE
// iccp DCB shared memory
ICCP_DCB *pIccpDcb = NULL;
#endif
#endif

/************************************************************************/
/* Static Variables and Functions					*/

#if 0	// CHOIBC DELETE
static ST_BOOLEAN     _miuAuto = SD_FALSE;
static MI_ASSOC_CTRL *_miuClientRemoteAssoc;
#endif	// CHOIBC DELETE

static ST_RET  _miUsrKbService (ST_VOID);
#if 0	// CHOIBC DELETE
static ST_VOID _miuSetIccpMode (int argc, char *argv[]);
#endif
static ST_VOID _miuInitMem (ST_VOID);

#if defined(S_SEC_ENABLED)
static ST_RET _miuCheckCertExpiration (ST_VOID);
#endif /* defined(S_SEC_ENABLED) */

#if 1	// 2017.11 ChoiBC MMS LOG 추가
static ST_INT _mmsDebugFlag = 0;
#endif

#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
static INFO_ICCP_ST iccpInfo;

INFO_ICCP_ST *getIccpInfo (void) //공유메모리의 iccp 영역
{
	return &iccpInfo; 
}
#endif

/************************************************************************/
/************************************************************************/
/*                       main						*/
/************************************************************************/
#ifdef _WIN32
ST_INT main (int argc, char *argv[])
{
	ST_INT maxWaitTime;
	ST_CHAR *memMark;
	ST_RET ret;
	ST_CHAR *appName;

	if (argc < 2)
	{
		printf ("\nMust enter 'Application Name' on command line.");
		exit (1);
	}
	appName = argv[1];

	setbuf (stdout, NULL);    	/* do not buffer the output to stdout   */
	setbuf (stderr, NULL);    	/* do not buffer the output to stderr   */

#if 0	// CHOIBC DELETE
	_miuSetIccpMode (argc, argv);	/* Check arguments			*/
#endif

	ret = miuInitIccp (appName, NULL);	/* Initialize ICCP Lite			*/
	if (ret)
	{
		printf ("\nInitialization error %d. Exiting.", ret);
		exit (1);
	}

#if !defined(NO_KEYBOARD) && !defined(_WIN32) && !defined(__QNX__)
	term_init ();	/* makes getchar work right	*/
#endif

	/* We are all initialized, just service communications			*/
	printf ("\nInitialization complete, hit 'x'<Enter> to exit ... ");

	memMark = (ST_CHAR *) chk_malloc (1);	/* marker */

	maxWaitTime = MIU_MAX_EVENT_WAIT_TIME;
	while (miuDoIt)
	{
		miuService (&maxWaitTime, appName);
	}
	miuTerminate (appName);
	dyn_mem_ptr_status2 (memMark);

	printf ("\n\n");

#if !defined(NO_KEYBOARD) && !defined(_WIN32) && !defined(__QNX__)
	term_rest ();	/* Must be called before exit if term_init used.	*/
#endif
	return (0);
}
#endif // _WIN32

#if 0	// 2016.11 ChoiBC Modify : SOE
// End Report for DSTS, Shared Memory의 Soe Queue를 삭제한다.
void miuEndDsts (MIS_DSTS *mis_dsts)
{
	MIS_DSTS_DV_VAL *dv_val;
	MIS_DV *mis_dv;
	MISU_DV *miuDv;

	int i;
	if (!(mis_dsts->ds_conditions & MI_DSC_CHANGE))
		return;
	dv_val = &mis_dsts->dv_val_tbl[0];
	for (i=0; i< mis_dsts->dv_val_count; i++, dv_val++)
	{
		if (dv_val->mi_type != MI_TYPEID_STATE_SUPP_Q_TIMETAG_EXTENDED ||
			dv_val->ds_var_ref.dv != SD_TRUE)
			continue;
		mis_dv = MIS_DV_REF_TO_DV (dv_val->ds_var_ref.v.dv_ref);
		if (mis_dv)
		{
			miuDv = mis_dv->access_info;
			QPRINTF ("    ===> send SOE '%s'", mis_dv->dv_name);
		}
	}
}
#endif


// Check ICCP SOE/COS Queue
ST_VOID miuCheckEventQueue (ST_VOID)
{
	static int commMasterOld = -1;
	int ret, commMaster;
	ICCP_SOEQ_ENTRY soe;
	ICCP_POINT_INFO *pPnt;

	commMaster = iccpShmIsCommMaster ();

	if (commMasterOld == -1)
	{
		commMasterOld = commMaster;
		return;
	}
	if (commMaster == 0)
		return;
	// StartDsts 첫번째 일때 commMaster가 설정된다.
	// StartDsts의 끝을 모르므로 일정시간 지난 후, SOE 를 검사해야한다. ==> 첫번째 DSTS를 무조건 DI 로 만들면 된다. 

	// Check SOE Queue
	for (;;)
	{
		pPnt = iccpShmGetSoeEntry (&soe);
		if (pPnt == NULL)
			break;
		ret = misuChangeDataValue (pPnt, &soe.data, SD_TRUE);
		if (ret == MY_DV_CHANGE_FULL)
		{
			// 다음에 보낸다.
			iccpShmResetSendingSoeEntry (&soe);
			break;
		}
		else
		{
			iccpShmDeleteSoeEntry (&soe, 1);
		}
#if 0	// 2016.05.24 ChoiBC Test
		else
		{
			//printf ("SOE Event (%s)\n", pPnt->iccpName);
			iccpShmDeleteSoeEntry (&soe, 1);
		}
#endif
	}
}

#ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
ST_CHAR *getMpuSwitchDevName (ST_VOID)
{
	return iccpFileCfg.remote.serObj.MpuSwitchDevName;
}

// Check Analog Value Changes
// 1초마다 포인트 타입별로 COA를 검사한다.
ST_VOID miuCheckAnalogChanges (ST_VOID)
{
	static int commMasterOld = -1;
	int ret, commMaster;
	static ST_DOUBLE oldMsTime;
	ST_DOUBLE newMsTime;
	static time_t oldSec;
	time_t newSec;
	static int index = -1;
	static int ptype_ana_list[4] = {SDP_POINT_TYPE_SAI, SDP_POINT_TYPE_DAI, SDP_POINT_TYPE_QAI, SDP_POINT_TYPE_TAI};

	if (iccpFileCfg.remote.serObj.ChangeOfAnalogPeriod == 0)
		return;

	commMaster = iccpShmIsCommMaster ();

	if (commMasterOld == -1)
	{
		commMasterOld = commMaster;
		return;
	}
	if (commMaster == 0)
		return;
	// every 1 seconds
	newSec = time (NULL);
	if (oldSec == newSec)
		return;
	oldSec = newSec;

	if (index == -1)
	{
		newMsTime = sGetMsTime ();
		if ((newMsTime - oldMsTime) > (iccpFileCfg.remote.serObj.ChangeOfAnalogPeriod * 1000))
		{
			oldMsTime = newMsTime;
			index = 0;
		}
		else
			return;
	}
	
	if (index >= 0 && index < 4)
	{
		ret = misuCheckCOA (ptype_ana_list[index]);
		
		if(opr->iccpDebug)
		QPRINTF ("___ End misuCheckCOA (iccpPointType = %d), count=%d\n", ptype_ana_list[index], ret);
	}
	if (++index >= 4)
		index = -1;
}
#endif

#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
#endif


// 이 함수가 반복적으로 호출되기 때문에  static 변수로  flow를 제어하는 구나..
// 근디...
static void	_miUsrService (void) // mi 즉 iccp level의 UserService 
{
	INFO_ICCP_ST *pI = getIccpInfo ();

#if 1	// 2017.11 ChoiBC MMS LOG 추가
	static ST_INT mmsDebugOld;

    // 변경이 있을 때만 건들자..
	if (opr->mmsDebug != mmsDebugOld) //opr->mmsDebug는 lib밖에서...mmsDebugOld는 
	{
		_mmsDebugFlag = 0; // 이건 뭐지...
		if (opr->mmsDebug) //mi_app_dyn_log_fun를 사용하게 한다.
		{
		 
		//    printf("Turn on Slog on console\r\n");   
			_slog_dyn_log_fun = mi_app_dyn_log_fun;
		// 이 걸봐서는 기본으로 mms 층만 보겠다는 것인댜...
			mms_debug_sel |= MMS_LOG_REQ | MMS_LOG_CONF;		// MMS_LOG_CLIENT
			mms_debug_sel |= MMS_LOG_IND | MMS_LOG_RESP;		// MMS_LOG_SERVER
		}
		else 
		{ 
		 //   printf("Turn off Slog on console\r\n"); 
			_slog_dyn_log_fun = NULL;
			mms_debug_sel = MMS_LOG_ERR;
		}
		mmsDebugOld = opr->mmsDebug; // 이 것 때문에 mmsDebug가 변할때 마다 위 routine에 들어온다.
	}
#endif
#if 1	/* 2026-10-02 ChoiBC Add : CLI/CLI-S2 layer FLOW/CFG bit control mailbox */
	if (opr->logbitCtl.seq != opr->logbitCtl.ack)
	{
		if (opr->logbitCtl.cmd == LOGBIT_CMD_SET)
		{
			ST_UINT *sel  = NULL;
			ST_UINT  mask = 0;
			ST_INT   layerFound = 0;
			ST_INT   bitFound   = 0;

			if (strcmp (opr->logbitCtl.layer, "mi") == 0)
			{
				layerFound = 1;
				sel = &mi_debug_sel;
				if (strcmp (opr->logbitCtl.bit, "FLOW") == 0)      { bitFound = 1; mask = MILOG_FLOW; }
				else if (strcmp (opr->logbitCtl.bit, "CFG") == 0)  { bitFound = 1; mask = MILOG_CFG;  }
			}
			else if (strcmp (opr->logbitCtl.layer, "mvl") == 0)
			{
				layerFound = 1;
				sel = &mvl_debug_sel;
				if (strcmp (opr->logbitCtl.bit, "FLOW") == 0)      { bitFound = 1; mask = MVLULOG_FLOW; }
			}
			else if (strcmp (opr->logbitCtl.layer, "mms") == 0)
			{
				layerFound = 1;		/* MMS_LOG* has no FLOW/CFG bit today */
			}
			else if (strcmp (opr->logbitCtl.layer, "acse") == 0)
			{
				layerFound = 1;		/* ACSE_LOG* has no FLOW/CFG bit today */
			}

			if (!layerFound)  // 발견 못 함.
				opr->logbitCtl.result = -1;
			else if (!bitFound) // 발견 못 함.
				opr->logbitCtl.result = -2;
			else // 발견 됬군...처리하자.
			{
				if (opr->logbitCtl.value)
					*sel |= mask;
				else
					*sel &= ~mask;
				opr->logbitCtl.result = 0; // 처리 완료
			}
		}
		else if (opr->logbitCtl.cmd == LOGBIT_CMD_CLEAR_ALL)
		{
			mi_debug_sel  &= ~(MILOG_FLOW | MILOG_CFG);
			mvl_debug_sel &= ~(MVLULOG_FLOW);
			opr->logbitCtl.result = 0;
		}
		else // 명령 지원 안 안 함
			opr->logbitCtl.result = -1;

		opr->logbitCtl.ack = opr->logbitCtl.seq;
	}

	/* Keep the console callback active while mmsDebug OR any CLI-controlled	*/
	/* layer FLOW/CFG bit is on, so mi/mvl FLOW,CFG lines reach the console.	*/
	if (opr->mmsDebug ||
	    (mi_debug_sel  & (MILOG_FLOW | MILOG_CFG)) ||
	    (mvl_debug_sel & (MVLULOG_FLOW)))
		_slog_dyn_log_fun = mi_app_dyn_log_fun;
	else
		_slog_dyn_log_fun = NULL;
#endif


    // 아래르 보면 알겠지만 pI는 변한다.
	if (pI->eStage == SDP_STAGE_WAIT_3SECONDS)
	{
		//  Disable 시킨 후, 3초간 쉰다.
		ST_DOUBLE dTime = sGetMsTime ();
		if ((dTime - pI->dTimeForceDisableAssoc) > 3000)
		{
			pI->eStage = SDP_STAGE_ENABLE;
			printf ("____ SDP_STAGE_WAIT_3SECONDS TimeOut\n");
		}
	}
	if (pI->eStage == SDP_STAGE_ENABLE)
	{
		ST_DOUBLE dTime;
		/* Enable all associations */
		printf (">>>> Enable Association\n");
		mi_enable_remote_assoc (NULL);
		pI->eStage = SDP_STAGE_WAIT_ASSOCIATE;
		dTime = sGetMsTime ();
		pI->dTimeWait = dTime;
		pI->dTimeLastDataReceived     = dTime;
		pI->dTimeLastIdentifyReceived = dTime;
		pI->dTimeLastCommandReceived  = dTime;
		return;
	}
	if (pI->eStage == SDP_STAGE_RUN)
	{
#if 1	// 2020.08 ChoiBC Modify : NO_ICCP_DUAL_LINK (ICCP 통신링크 이중화 제거)
#else
		if (iccpFileCfg.remote.assoc.identifyTimeout != 0)
		{
			ST_DOUBLE dTimeReceived;
			ST_DOUBLE dTime = sGetMsTime ();
			ST_LONG identifyTimeout = iccpFileCfg.remote.assoc.identifyTimeout;

#if 1
			if (pI->dTimeLastIdentifyReceived > pI->dTimeLastCommandReceived)
				dTimeReceived = pI->dTimeLastIdentifyReceived;
			else
				dTimeReceived = pI->dTimeLastCommandReceived;
#else
			// Identify는 3초 마다가 아니고, 데이터 전송이 끝나고 아무것도 없을때 3초 후 보낸다는 의미
			// 마스터로 데이터가 계속 들어 오면 Identify 20초 초과라고 링크를 끊는다.
			// 최근 받은 시간으로 비교한다.
			// 랜 포트를 빼도, 데이터는 계속 주려고 하기때문에 이건 필요 없다.
			if (pI->dTimeLastIdentifyReceived > pI->dTimeLastDataReceived)
				dTimeReceived = pI->dTimeLastIdentifyReceived;
			else
				dTimeReceived = pI->dTimeLastDataReceived;
#endif
			if ((dTime - dTimeReceived) > (identifyTimeout * 1000))
			{
				pI->eStage = SDP_STAGE_DISABLE;
				printf ("Identify is not received during %ld secs, so Disabling all associations\n", identifyTimeout);
				printf ("**** exit *****\n");
				QPRINTF ("Identify is not received during %ld secs, so Disabling all associations\n", identifyTimeout);
				QPRINTF ("**** exit *****\n");
				exit (1);
			}
#if 0
			else
			{
				static time_t tloc_old;
				time_t tloc = time (NULL);
				if (tloc != tloc_old)
				{
					tloc_old = tloc;
					if ((tloc % 3) == 0)
					{
						ST_DOUBLE diff = dTime - dTimeReceived;
						printf ("___   diff(%d) identifyTimeout(%ld)\n",  (int) diff, identifyTimeout);
						printf ("      dTimeLastDataReceived(%d) dTimeLastIdentifyReceived(%d) dTimeReceived(%d) dTime(%d)\n",
							(int) pI->dTimeLastDataReceived, (int)  pI->dTimeLastIdentifyReceived,
							(int) dTimeReceived, (int) dTime);
					}

				}
			}
#endif
		}
#endif	// 2020.08 ChoiBC Modify : NO_ICCP_DUAL_LINK (ICCP 통신링크 이중화 제거)
	}
	if (pI->eStage == SDP_STAGE_DISABLE)
	{
		printf (">>>> Disable Association\n");
		mi_disable_remote_assoc (NULL);
		pI->eStage = SDP_STAGE_WAIT_3SECONDS;
		pI->dTimeForceDisableAssoc = sGetMsTime ();
	}
#if 0	// SDP_STAGE_WAIT_ASSOCIATE 일때 스택이 자동으로 연결을 시도하기 때문에 강제로 disable 시킬필요가 없다.
	if (pI->eStage == SDP_STAGE_WAIT_ASSOCIATE)
	{
		//  10초 동안 Stage가 변하지 않으면 다음 Stage로 바꾼다.
		ST_DOUBLE dTime = sGetMsTime ();
		if ((dTime - pI->dTimeWait) > 10000)
		{
			printf ("Stage(%d) is not changed during 10 secs\n", pI->eStage);
			pI->eStage = SDP_STAGE_DISABLE;
		}
	}
#endif
}

/************************************************************************/
/*			miuService					*/
/************************************************************************/

ST_VOID miuService (ST_INT *maxWaitTimeIo, ST_CHAR *appName)
{
	ST_INT waitTime;
	ST_INT i;

    // get 작은 값
	waitTime = min (mi_wait_service_ms, *maxWaitTimeIo);

	wait_any_event (waitTime);

	for (i = 0; i < MIU_MAX_MVL_SERVICE_LOOPS; ++i)
	{
		if (mvl_comm_serve () == SD_FALSE)
			break; 	
	}
	if (i < MIU_MAX_MVL_SERVICE_LOOPS)	/* MVL has no more to do ... */
		*maxWaitTimeIo = MIU_MAX_EVENT_WAIT_TIME;
	else
		*maxWaitTimeIo = 0;

#ifdef MUI_REDUNDANCY_SUPPORT
	miuIrService ();
#endif
	mi_service (appName);

#if defined (S_SEC_ENABLED)
	/* check periodically for SISCO Stack Configuration updates 	*/
	if (secManChkNewCfgAvail ())
	{
		/* detected configuration change event, update configuration	*/
		if (sscCheckStackCfg (appName) == SD_SUCCESS)
		{
			/* new configuration loaded, update security configuration */
			if (ulCheckSecurityConfiguration () != SD_SUCCESS)
				MI_LOG_NERR0 ("Security reconfiguration failed");
		}
	}
	/* check periodically for new CRLs,				      	*/
	/* if crlDropExisting option set then this fun will abort existing	*/
	/* connections when local/remote MACE certificate become revoked	*/
	ulUpdateCertRevocationList ();
#endif /* defined (S_SEC_ENABLED) */

#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
	_miUsrService ();
#endif

#if 0	// CHOIBC DELETE
	if (_miuAuto == SD_TRUE)
		miuAutoService ();
#endif	// CHOIBC DELETE

#if 0	// CHOIBC DELETE
#if defined(DEBUG_SISCO)
	if (sLogCtrl->logCtrl & LOG_IPC_EN)
		slogIpcEvent ();	/* required for IPC Logging if gensock2.c is	*/
	/* not compiled with GENSOCK_THREAD_SUPPORT	*/
	/* At runtime, periodically need to service SLOG commands and calling connections.   */
	/* The timing of this service is not critical, but to be responsive a default of     */
	/* 100ms works well.                                                                 */
	slog_ipc_std_cmd_service ("logcfg.xml", NULL, NULL, SD_TRUE,  NULL, NULL);
#endif
#endif	// CHOIBC DELETE

#ifdef	_PLATFORM_LINUX	// 2016.05.03 ChoiBC Test
	_miUsrKbService ();
#endif
}

/************************************************************************/
/*			_miuTerminate 					*/
/************************************************************************/

ST_VOID miuTerminate (ST_CHAR *appName)
{
	MI_REMOTE *mi_remote;
	MI_ASSOC_CTRL *mi_assoc;
	ST_INT maxWaitTime;

#ifdef MUI_REDUNDANCY_SUPPORT
	miuIrRequestStandby ();
#endif

	/* Terminate all active connections */
	mi_disable_remote_assoc (NULL);

	/* Now wait for all to go idle */
	miuDoIt = SD_TRUE;
	maxWaitTime = MIU_MAX_EVENT_WAIT_TIME;
	while (miuDoIt)
	{
		mi_remote = mi_remote_list;
		while (mi_remote)
		{
			mi_assoc = mi_remote->mi_assoc_list;
			while (mi_assoc)
			{
				if (mi_assoc->state != MI_ASSOC_STATE_IDLE)
					break;	/* Found one not idle */

				mi_assoc = (MI_ASSOC_CTRL *) list_get_next (mi_remote->mi_assoc_list, mi_assoc);
			}
			if (mi_assoc != NULL)
				break;

			mi_remote = (MI_REMOTE *) list_get_next (mi_remote_list, mi_remote);
		}
		if (mi_remote == NULL)
			break;

		/* Still have associations that are not idle */
		miuService (&maxWaitTime, appName);
	}

#ifdef MUI_REDUNDANCY_SUPPORT
	/* Terminate the redundancy subsystem */
	miuIrTerminate ();
#endif

	mvl_end_acse ();		/* Stop the lower layer subsystem	*/
	mi_terminate ();

	/* Free memory allocated by MI_ICFG */
	mi_icfg_free_local_cc ();
	mi_icfg_free_remote_cc (NULL);

	/* Free all associations */
	mi_free_remote_assoc (NULL);

	/* Now free the MI objects that were allocated at configuration time	*/
	mi_free_remote (NULL);	/* Free all remotes 	*/
	mis_free_dv_tables ();	/* Free local DV's	*/
	mis_free_device_table ();	/* Free local DEV's	*/
}

/************************************************************************/
/************************************************************************/
/* INITIALIZATION FUNCTIONS 						*/
#if 0	// CHOIBC DELETE
/************************************************************************/
/*			_miuSetIccpMode 					*/
/************************************************************************/

static ST_VOID _miuSetIccpMode (int argc, char *argv[])
{
	ST_INT i;

	for (i = 0; i < argc; ++i)
	{
		if (strcmp (argv[i], "-Q") == 0)
			miuQuiet = SD_TRUE;
		if (strcmp (argv[i], "-D") == 0)
		{
			miuDeleteAllDs = SD_TRUE;
			miuNeedDelAllDs = SD_TRUE;
		}
#ifdef MUI_REDUNDANCY_SUPPORT
		if (strcmp (argv[i], "-R") == 0)
			miuRedundancyEnable = SD_TRUE;
#endif
		if (strcmp (argv[i], "-UC") == 0)
			miuCfgSource = MIU_CFG_SOURCE_USR_CLIENT;
		if (strcmp (argv[i], "-US") == 0)
			miuCfgSource = MIU_CFG_SOURCE_USR_SERVER;
		if (strcmp (argv[i], "-BC") == 0)
			miuBlock8Client = SD_TRUE;
		if (strcmp (argv[i], "-BS") == 0)
			miuBlock8Server = SD_TRUE; 

#ifdef MICS_DB
		if (strncmp (argv[i], "-DB:", 4) == 0)
		{
			miuCfgSource = MIU_CFG_SOURCE_DB;
			miuLccName = &argv[i][4];
		}
#endif
	}
}
#endif	// CHOIBC DELETE

#if 1	// 2016.04.07 ChoiBC Modify : Replace ICCP Config Info from DB

// db 에 P/S/AR 등이 없고 xml에 있다 이것을 iccpConfig에 넣는것.]
// ICCP_CONFIG : iccp association을 위한 정보를 가지고 있는곳
// DIB directory information base
static ST_RET my_iccp_addr_replace (ICCP_CONFIG *iccpConfig)
{
	DIB_ENTRY *de;
	DIB_ENTRY *de_list = NULL;
	SSC_STACK_CFG *sscStackCfg = NULL;
	ICCP_UNIT *pAddr;
	ST_CHAR *name;
	ST_UINT hexStrLen;

	/* get the pointers to SISCO Stack Configuration */
	sscStackCfg = sscAccessStackCfg ();
	if (!sscStackCfg || !sscStackCfg->appNetwork)
		goto ERROR_iccp_addr_replace;

// ssStackCfg가 언제 dibEntrylist를 읽어나 
// stack xml 에 기본적인 것이 들어 있구나....
	de_list = sscStackCfg->appNetwork->dibEntryList;
	for (de = de_list; de != NULL; de = list_get_next (de_list, de))
	{
		if (strcmp (de->name, "SDP_A") == 0)
		{
			pAddr = &iccpConfig->SDP_A;
			name = "SDP_A";
		}
		else if (strcmp (de->name, "SDP_B") == 0)
		{
			pAddr = &iccpConfig->SDP_B;
			name = "SDP_B";
		}
		else if (strcmp (de->name, "FEP_A") == 0)
		{
			pAddr = &iccpConfig->FEP_A;
			name = "FEP_A";
		}
		else if (strcmp (de->name, "FEP_B") == 0)
		{
			pAddr = &iccpConfig->FEP_B;
			name = "FEP_B";
		}
		else
			continue;
		// AP Title
		if (pAddr->AP_Title[0] == 0)
			de->ae_title.AP_title_pres = SD_FALSE;
		else
		{
#if 1	// 2017.11.08 ChoiBC Bug Fixed : 이함수 호출후 구성정보가 깨진다.(AE_Title)
			//주) sscParseObjId () 함수 호출 뒤에 pAddr->AP_Title이 깨진다.
#endif
			if (sscParseObjId (&de->ae_title.AP_title, (char *)pAddr->AP_Title) == SD_SUCCESS)
				de->ae_title.AP_title_pres = SD_TRUE;
			else
			{
				SLOGALWAYS2 ("DB Config : Comm Address(%s) AP Title(%s) parsing error.", name, pAddr->AP_Title);
				goto ERROR_iccp_addr_replace;
			}
		}
		// AE Qualifier
		if (pAddr->AE_Qualifier[0] == 0)
			de->ae_title.AE_qual_pres = SD_FALSE;
		else
		{
			if (asciiToSint32 ((char *)pAddr->AE_Qualifier, &de->ae_title.AE_qual) == SD_SUCCESS)
				de->ae_title.AE_qual_pres = SD_TRUE;
			else
			{
				SLOGALWAYS2 ("DB Config : Comm Address(%s) AE Qualifier(%s) parsing error.", name, pAddr->AE_Qualifier);
				goto ERROR_iccp_addr_replace;
			}
		}
		// Psel
		if (pAddr->P_Sel[0] == 0)
			de->pres_addr.psel_len = 0;
		else
		{
			if (ascii_to_hex_str (de->pres_addr.psel, &hexStrLen, MAX_PSEL_LEN, (char *)pAddr->P_Sel) == SD_SUCCESS)
				de->pres_addr.psel_len = hexStrLen;
			else
			{
				SLOGALWAYS2 ("DB Config : Comm Address(%s) Psel(%s) parsing error.", name, pAddr->P_Sel);
				goto ERROR_iccp_addr_replace;
			}
		}
		// Ssel
		if (pAddr->S_Sel[0] == 0)
			de->pres_addr.ssel_len = 0;
		else
		{
			if (ascii_to_hex_str (de->pres_addr.ssel, &hexStrLen, MAX_SSEL_LEN, (char *)pAddr->S_Sel) == SD_SUCCESS)
				de->pres_addr.ssel_len = hexStrLen;
			else
			{
				SLOGALWAYS2 ("DB Config : Comm Address(%s) Ssel(%s) parsing error.", name, pAddr->S_Sel);
				goto ERROR_iccp_addr_replace;
			}
		}
		// Tsel
		if (pAddr->T_Sel[0] == 0)
			de->pres_addr.tsel_len = 0;
		else
		{
			if (ascii_to_hex_str (de->pres_addr.tsel, &hexStrLen, MAX_TSEL_LEN, (char *)pAddr->T_Sel) == SD_SUCCESS)
				de->pres_addr.tsel_len = hexStrLen;
			else
			{
				SLOGALWAYS2 ("DB Config : Comm Address(%s) Tsel(%s) parsing error.", name, pAddr->T_Sel);
				goto ERROR_iccp_addr_replace;
			}
		}
	}
	sscReleaseStackCfg ();
	return (SD_SUCCESS);
ERROR_iccp_addr_replace:
	sscReleaseStackCfg ();
	return (SD_FAILURE);

}
#endif

/************************************************************************/
/*			miuInitIccp 					*/
/************************************************************************/

ST_RET miuInitIccp (ST_CHAR *appName, ST_INT deviceIndex, ICCP_DCB *dcb)
{
	MVL_CFG_INFO mvlCfgInfo; // local변수인데
	ST_RET ret;

#if 0	// 2017.11 ChoiBC : Debugging
	{
    ICCP_UNIT       *iccp;
    ICCP_CONFIG     *iccpCfg;

    iccpCfg = (ICCP_CONFIG *) &dcb->config;  
        
    printf("\n--------[ ICCP ] Parameter setting.---------\n");

    iccp = (ICCP_UNIT *) &iccpCfg->FEP_A;
    printf("[FEP_A] Configuration----------------------------------\n");
    printf(" IP-ADDR[16] = %s\n", iccp->IPAddress);
    printf(" P_SEL[48]   = %s\n", iccp->P_Sel); 
    printf(" S_SEL[48]   = %s\n", iccp->S_Sel);    
    printf(" T_SEL[96]   = %s\n", iccp->T_Sel); 
    printf(" AP_Title[96]= %s\n", iccp->AP_Title); 
    printf(" AP_Quali[11]= %s\n\n", iccp->AE_Qualifier); 
    
    iccp = (ICCP_UNIT *) &iccpCfg->FEP_B;
    printf("[FEP_B] Configuration----------------------------------\n");
    printf(" IP-ADDR[16] = %s\n", iccp->IPAddress);
    printf(" P_SEL[48]   = %s\n", iccp->P_Sel); 
    printf(" S_SEL[48]   = %s\n", iccp->S_Sel);    
    printf(" T_SEL[96]   = %s\n", iccp->T_Sel); 
    printf(" AP_Title[96]= %s\n", iccp->AP_Title); 
    printf(" AP_Quali[11]= %s\n\n", iccp->AE_Qualifier); 
    
    iccp = (ICCP_UNIT *) &iccpCfg->SDP_A;
    printf("[SDP_A] Configuration----------------------------------\n");
    printf(" IP-ADDR[16] = %s\n", iccp->IPAddress);
    printf(" P_SEL[48]   = %s\n", iccp->P_Sel); 
    printf(" S_SEL[48]   = %s\n", iccp->S_Sel);    
    printf(" T_SEL[96]   = %s\n", iccp->T_Sel); 
    printf(" AP_Title[96]= %s\n", iccp->AP_Title); 
    printf(" AP_Quali[11]= %s\n\n", iccp->AE_Qualifier); 
    
    iccp = (ICCP_UNIT *) &iccpCfg->SDP_B;
    printf("[SDP_B] Configuration----------------------------------\n");
    printf(" IP-ADDR[16] = %s\n", iccp->IPAddress);
    printf(" P_SEL[48]   = %s\n", iccp->P_Sel); 
    printf(" S_SEL[48]   = %s\n", iccp->S_Sel);    
    printf(" T_SEL[96]   = %s\n", iccp->T_Sel); 
    printf(" AP_Title[96]= %s\n", iccp->AP_Title); 
    printf(" AP_Quali[11]= %s\n\n", iccp->AE_Qualifier); 
    printf("-------------------------------------------------------\n\n");
	}
#endif

#if 0	// CHOIBC DELETE
	pIccpDcb = dcb;
#endif

	_miuInitMem ();  	/* Set up memory allocation tools 		*/

#if defined(DEBUG_SISCO)
	logCfgAddMaskGroup (&miLogMaskMapCtrl);
	logCfgAddMaskGroup (&mvlLogMaskMapCtrl);
#if defined(SISCO_STACK_CFG)
	logCfgAddMaskGroup (&sscLogMaskMapCtrl);
#endif
#if defined(S_SEC_ENABLED)
	logCfgAddMaskGroup (&secLogMaskMapCtrl);
	logCfgAddMaskGroup (&ssleLogMaskMapCtrl);
	logCfgAddMaskGroup (&ssleASN1LogMaskMapCtrl);	/* in SSL Engine */
#endif
	logCfgAddMaskGroup (&mmsLogMaskMapCtrl);
	logCfgAddMaskGroup (&acseLogMaskMapCtrl);
	logCfgAddMaskGroup (&tp4LogMaskMapCtrl);
	logCfgAddMaskGroup (&clnpLogMaskMapCtrl);
	logCfgAddMaskGroup (&asn1LogMaskMapCtrl);
	logCfgAddMaskGroup (&sxLogMaskMapCtrl);
#if defined(S_MT_SUPPORT)
	logCfgAddMaskGroup (&gsLogMaskMapCtrl);
#endif
	logCfgAddMaskGroup (&sockLogMaskMapCtrl);
#if defined(SISCO_STACK_CFG)
	logCfgAddMaskGroup (&ipcLogMaskMapCtrl);
#endif
	logCfgAddMaskGroup (&memLogMaskMapCtrl);
	logCfgAddMaskGroup (&memDebugMapCtrl);
#ifdef MUI_REDUNDANCY_SUPPORT
	logCfgAddMaskGroup (&irLogMaskMapCtrl);
#endif
#endif /* defined(DEBUG_SISCO) */

#if defined(DEBUG_SISCO)
	/* At initialization, install a SLOGIPC command handler. The      */
	/* build in SLOGIPC handler just receives the command and put's   */
	/* on a list to be handled by the application at it's leisure ... */
	sLogCtrl->ipc.slog_ipc_cmd_fun = slog_ipc_std_cmd_fun;
#ifdef _WIN32	// 2016.04.27 ChoiBC Modify
	ret = logcfgx_ex (sLogCtrl, "logcfg.xml", NULL, SD_FALSE, SD_TRUE);
#else
	ret = logcfgx_ex (sLogCtrl, "/mnt/config/logcfg.xml", NULL, SD_FALSE, SD_TRUE);
#endif
	if (ret != SD_SUCCESS)
	{
		printf ("\n Parsing of 'logging' configuration file failed.");
		if (sLogCtrl->fc.fileName)
			printf ("\n Check log file '%s'.", sLogCtrl->fc.fileName);
		return (ret);
	}
	slog_start (sLogCtrl, MAX_LOG_SIZE);  /* call after logging parameters are configured	*/

	SLOGALWAYS0 ("Initializing ...");

	ml_mlog_install ();
#endif

#if defined(_S_TEST_PROBES)
	ret = _sTestProbeLoad ("sTestProbes.xml");
	if (ret != SD_SUCCESS)
		printf ("\nCould not load test probes");
#endif

#ifndef _WIN32	// 2016.03.25 ChoiBC Modify
	sscSetCfgFilePath ("/mnt/config");  
#endif
	/* sscConfigMvl must be called before mi_initialize */
	ret = sscConfigMvl (appName, &mvlCfgInfo);  // 그냥 쓰자.. sscConfigMvl
	if (ret)
	{
		SLOGALWAYS1 ("Stack configuration error = 0x%X. Check 'siscostackcfg.xml'.", ret);
		printf ("\n Stack configuration error = 0x%X. Check 'siscostackcfg.xml'.\n", ret);
		return (ret);
	}
	/* CRITICAL: bind information filled in by sscConfigMvl must not be	*/
	/*           changed by mi_find_mvl_cfg_info below when		*/
	/*           SISCO_STACK_CFG is defined.				*/

#if 1	// 2016.04.07 ChoiBC Modify : Replace ICCP Config Info from DB
#if 1	// 2017.11.08 ChoiBC Bug Fixed : 이함수 호출후 구성정보가 깨진다.(AE_Title)
	{
		ICCP_CONFIG iccpConfig;
		memcpy (&iccpConfig, &dcb->config, sizeof(iccpConfig));
		if (my_iccp_addr_replace (&iccpConfig) != SD_SUCCESS)
		{
			SLOGALWAYS0 ("ICCP DB configuration error.");
			printf ("\nICCP DB configuration error.\n");
			exit (1);
		}
	}
#else
	if (my_iccp_addr_replace (&dcb->config) != SD_SUCCESS)
	{
		SLOGALWAYS0 ("ICCP DB configuration error.");
		printf ("\nICCP DB configuration error.\n");
		exit (1);
	}
#endif

	// initial(load) iccp configuration informaion from iccp.conf file
	my_iccp_config_file_load (&iccpFileCfg); // iccp./conf
#endif

	// 2018.09.08 ChoiBC : FEP A network 뺐을때, FEP B의 연결이 많이 지연되서
	//	TCP_KEEPALIVE 적용했으나, 해결않됨.
#if 0	// 2016.07.20 ChoiBC Modify for TCP_KEEPALIVE
	{
#if defined (_PLATFORM_LINUX)	// ???????? 나중에  stack 을 LINUX용으로 컴파일한후에 지워야한다.
	// /etc/sysctl.conf 에 관련 파라미터 설정
#else
		// added at gensock2.c
		extern void setTcpKeepAlive (int nKeepAliveIdle, int nKeepAliveCnt, int nKeepAliveInterval);
		setTcpKeepAlive (3, 3, 3);
		//setTcpKeepAlive (pIcdCfg->tNonVisible.iTcpKeepAliveIdle,
		//				 pIcdCfg->tNonVisible.iTcpKeepAliveCount,
		//				 pIcdCfg->tNonVisible.iTcpKeepAliveInterval);
#endif
/*
	- TCP keep alive tip
	HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\services\Tcpip\Parameters\KeepAliveTime
	netstat  -napo				: LINUX keep alive process 별 정보 보기 명령
	/sbin/sysctl -a | grep keep : LINUX 시스템 설정 보기 명령
		net.ipv4.tcp_keepalive_intvl = 75
		net.ipv4.tcp_keepalive_probes = 9
		net.ipv4.tcp_keepalive_time = 7200
	/etc/sysctl.conf			: LINUX 시스템 설정 파일
	setsockopt parameter
		TCP_KEEPCNT: overrides  tcp_keepalive_probes
		TCP_KEEPIDLE: overrides tcp_keepalive_time
		TCP_KEEPINTVL: overrides  tcp_keepalive_intvl
*/
	}
#endif	// 2016.07.20 ChoiBC Modify for TCP_KEEPALIVE

	ret = mi_initialize ();	/* Start the MI (ICCP) subsystem	*/
	if (ret != SD_SUCCESS)
	{
		printf ("\nCould not initialize  MI (ICCP) subsystem.\n");
		return (ret);
	}
#if 1	// 2016.04.07 ChoiBC Modify : Replace ICCP Config Info from DB
	my_iccp_config (deviceIndex, &dcb->pointDef, &iccpFileCfg);
#else
	miu_icfg_load (miuCfgSource, MI_ICFG_MODE_INITIAL, NULL);
#endif

#if 0	// CHOIBC DELETE
	miuEnableBlock8 ();
#endif

#if 1	// 2017.11 ChoiBC : Debugging
	printf ("ICCP : appName(%s) deviceIndex(%d) localAr(%s)\n",
			appName, deviceIndex, mvlCfgInfo.local_ar_name);
	//{
 //   ICCP_UNIT       *iccp;
 //   ICCP_CONFIG     *iccpCfg;

 //   iccpCfg = (ICCP_CONFIG *) &dcb->config;  
 //       
 //   printf("\n--------[ ICCP ] Parameter setting.---------\n");

 //   iccp = (ICCP_UNIT *) &iccpCfg->FEP_A;
 //   printf("[FEP_A] Configuration----------------------------------\n");
 //   printf(" IP-ADDR[16] = %s\n", iccp->IPAddress);
 //   printf(" P_SEL[48]   = %s\n", iccp->P_Sel); 
 //   printf(" S_SEL[48]   = %s\n", iccp->S_Sel);    
 //   printf(" T_SEL[96]   = %s\n", iccp->T_Sel); 
 //   printf(" AP_Title[96]= %s\n", iccp->AP_Title); 
 //   printf(" AP_Quali[11]= %s\n\n", iccp->AE_Qualifier); 
 //   
 //   iccp = (ICCP_UNIT *) &iccpCfg->FEP_B;
 //   printf("[FEP_B] Configuration----------------------------------\n");
 //   printf(" IP-ADDR[16] = %s\n", iccp->IPAddress);
 //   printf(" P_SEL[48]   = %s\n", iccp->P_Sel); 
 //   printf(" S_SEL[48]   = %s\n", iccp->S_Sel);    
 //   printf(" T_SEL[96]   = %s\n", iccp->T_Sel); 
 //   printf(" AP_Title[96]= %s\n", iccp->AP_Title); 
 //   printf(" AP_Quali[11]= %s\n\n", iccp->AE_Qualifier); 
 //   
 //   iccp = (ICCP_UNIT *) &iccpCfg->SDP_A;
 //   printf("[SDP_A] Configuration----------------------------------\n");
 //   printf(" IP-ADDR[16] = %s\n", iccp->IPAddress);
 //   printf(" P_SEL[48]   = %s\n", iccp->P_Sel); 
 //   printf(" S_SEL[48]   = %s\n", iccp->S_Sel);    
 //   printf(" T_SEL[96]   = %s\n", iccp->T_Sel); 
 //   printf(" AP_Title[96]= %s\n", iccp->AP_Title); 
 //   printf(" AP_Quali[11]= %s\n\n", iccp->AE_Qualifier); 
 //   
 //   iccp = (ICCP_UNIT *) &iccpCfg->SDP_B;
 //   printf("[SDP_B] Configuration----------------------------------\n");
 //   printf(" IP-ADDR[16] = %s\n", iccp->IPAddress);
 //   printf(" P_SEL[48]   = %s\n", iccp->P_Sel); 
 //   printf(" S_SEL[48]   = %s\n", iccp->S_Sel);    
 //   printf(" T_SEL[96]   = %s\n", iccp->T_Sel); 
 //   printf(" AP_Title[96]= %s\n", iccp->AP_Title); 
 //   printf(" AP_Quali[11]= %s\n\n", iccp->AE_Qualifier); 
 //   printf("-------------------------------------------------------\n\n");
	//}
#endif

	/* MI_ICFG has loaded the ICCP configuration, which contains some    	*/
	/* information required to correctly set up MMS-EASE Lite.		*/
	/* Get the ICCP configuration based parameters */
	
	                                    // mvlCfgInfo내부 변수 setting 
	mi_find_mvl_cfg_info (&mvlCfgInfo); // mmslite/iccp/src/mi.c 예 strcpy (mvl_cfg_info->local_ar_name, cfgAr->arName);

	/* OK, finally we are ready to start */
	ret = mvl_start_acse (&mvlCfgInfo);
	if (ret != SD_SUCCESS)
	{
		printf ("\nCould not start MVL");
		return (ret);
	}

#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
	{
		INFO_ICCP_ST *pI = getIccpInfo ();
		pI->eStage = SDP_STAGE_ENABLE;
	}
#else
	/* Enable all associations */
	mi_enable_remote_assoc (NULL);
#endif

	SLOGALWAYS0 ("Initialization complete");
	return (SD_SUCCESS);
}

/************************************************************************/
/************************************************************************/
/* MI CONNECTION ACTIVITY 						*/
/************************************************************************/
/************************************************************************/
/* 		u_mi_association_active 				*/
/************************************************************************/

ST_VOID u_mi_association_active (MI_ASSOC_CTRL *mi_assoc)
{
#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
	{
		ST_DOUBLE dTime = sGetMsTime ();
		INFO_ICCP_ST *pI = getIccpInfo ();
		pI->eStage = SDP_STAGE_RUN;
		pI->dTimeWait = dTime;
		pI->dTimeLastDataReceived     = dTime;
		pI->dTimeLastIdentifyReceived = dTime;
		pI->dTimeLastCommandReceived  = dTime;
	}
#endif	

#if 1	// 2016.01.22 ChoiBC Modify : Association Active
	time_t lastTime = time (NULL);
	iccpShmSetAssociationActive (mi_assoc->active_ar_name_index, lastTime);
	// ICCP SOE Queue에 모든 Entry의 sending flag를 Reset한다.
	iccpShmResetSendingSoeAllEntry ();
#endif
	printf ("Association '%s', AR '%s' Active\n", 
		mi_assoc->name, mi_assoc->remote_ar[mi_assoc->active_ar_name_index]);
	if (mi_assoc->ctrl_flags & MI_ASSOC_CTRL_CALLING)
		printf ("  Calling");
	else
		printf ("  Called");

	if (mi_assoc->ctrl_flags & MI_ASSOC_CTRL_CLIENT)
		printf (", Client\n");
	if (mi_assoc->ctrl_flags & MI_ASSOC_CTRL_SERVER)
		printf (", Server\n");

	QPRINTF (">> Association '%s', AR '%s' Active\n", 
			mi_assoc->name, mi_assoc->remote_ar[mi_assoc->active_ar_name_index]);
#if defined(S_SEC_ENABLED)
	/* See what our security status is */
	if (mi_assoc->s_sec_auth_chk.authPres == SD_TRUE)
	{
		if (mi_assoc->s_sec_auth_chk.mechType == ACSE_AUTH_MECH_MACE_CERT)
		{
			if (mi_assoc->mi_conn->net_info->rem_mace)
				printf ("\n  MACE");
			else
				/* in this case we received rem MACE but did not accepted it	*/
				/* connection was allowed because of fall-back to non-secure	*/
				printf ("\n  No MACE");
		}
		else
			printf ("\n  Non-MACE");
	}
	else
		printf ("\n  No MACE");

	if (mi_assoc->s_sec_auth_chk.encryptInfo.encryptMode == S_SEC_ENCRYPT_SSL)
		printf (", SSL");
	else
		printf (", No SSL");
#endif	/* defined(S_SEC_ENABLED)	*/

#ifdef MUI_REDUNDANCY_SUPPORT
	miuIrAssociationActive (mi_assoc);
#endif

#if 0	// CHOIBC DELETE
	if (mi_assoc->ctrl_flags & MI_ASSOC_CTRL_CLIENT)
	{
		if (_miuClientRemoteAssoc == NULL)
			_miuClientRemoteAssoc = mi_assoc;

		micuStartAssocActivity (mi_assoc->mi_conn);
	}
#endif	// CHOIBC DELETE
}

/************************************************************************/
/* 		u_mi_association_inactive				*/
/************************************************************************/

ST_VOID u_mi_association_inactive(MI_ASSOC_CTRL *mi_assoc, ST_INT reason)
{
	MI_CONN *mi_conn;
	MIU_REMOTE *miuRemote;
	MICU_DSTS  *miuDsTs;
	char buf[256];

#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
	{
		INFO_ICCP_ST *pI = getIccpInfo ();
		if (pI->eStage != SDP_STAGE_WAIT_3SECONDS)
		{
			pI->eStage = SDP_STAGE_WAIT_ASSOCIATE;
			pI->dTimeWait = sGetMsTime ();
		}
		else
			printf ("___ eStage is SDP_STAGE_WAIT_3SECONDS\n");
	}
#endif	

#if 1	// 2016.01.22 ChoiBC Modify : Association Inactive
	time_t lastTime = time (NULL);
	iccpShmSetAssociationInactive (mi_assoc->active_ar_name_index, lastTime);
	// ICCP SOE Queue에 모든 Entry의 sending flag를 Reset한다.
	iccpShmResetSendingSoeAllEntry ();
#endif
	snprintf ( buf, sizeof(buf)-1, "Association '%s', AR '%s' Inactive: ", 
		mi_assoc->name, mi_assoc->remote_ar[mi_assoc->active_ar_name_index]);
	if (reason == MI_ASSOC_INACTIVE_ABORT_IND)
		strcat (buf, "Abort Indication");
	if (reason == MI_ASSOC_INACTIVE_ABORT_REQ)
		strcat (buf, "Abort Confirm");
	if (reason == MI_ASSOC_INACTIVE_TERMINATE_IND)
		strcat (buf, "Terminate Indication");
	if (reason == MI_ASSOC_INACTIVE_TERMINATE_CONF)
		strcat (buf, "Terminate Confirm");
	if (reason == MI_ASSOC_FAILED_ASSOC_REQ)
		strcat (buf, "Associate Request Failed");

	printf ("%s\n", buf);

	QPRINTF (">> %s\n", buf);	// Association '%s', AR '%s' Inactive:
	
	/* reset DSTS back to not in use */

	mi_conn = mi_assoc->mi_conn;
	if (mi_conn != NULL) /* we only want to do this when the connection is terminated */
	{
		miuRemote = miuFindMiRemote (mi_conn->mi_remote);
		miuDsTs = miuRemote->dsts_list;
		while (miuDsTs)
		{
			if (!strcmp (miuDsTs->assocName, mi_conn->mi_assoc->name))
				miuDsTs->miu_ds->mic_ds_in_use = SD_FALSE;

			miuDsTs = list_get_next (miuRemote->dsts_list, miuDsTs);
		}
	}

#if 0	// CHOIBC DELETE
	/* If this was our test client association, can't use it anymore */
	if (mi_assoc == _miuClientRemoteAssoc)
		_miuClientRemoteAssoc = NULL;

	miuNeedDelAllDs = SD_TRUE;
#endif
#ifdef MUI_REDUNDANCY_SUPPORT
	if (mi_assoc->wasActive ==  SD_TRUE)
		miuIrAssociationInactive (mi_assoc);
#endif
}

/************************************************************************/
/* 		u_mi_association_ind_err 				*/
/************************************************************************/

ST_VOID u_mi_association_ind_err (ST_INT reason)
{
	printf ("\nMI Initiate Indication Error detected:");
	if (reason == MI_ASSOC_IND_ERR_NOREMOTE)
		printf ("Could not match remote");
	if (reason == MI_ASSOC_IND_ERR_NOMISVCC)
		printf ("Remote does not have a MIS_VCC");
	if (reason == MI_ASSOC_IND_ERR_NOASSOC)
		printf ("Could not find available MI_ASSOC");
}


/************************************************************************/
/*			u_mi_reject_ind					*/
/************************************************************************/

ST_VOID u_mi_reject_ind (MI_ASSOC_CTRL *mi_assoc, REJECT_RESP_INFO *rej_info)
{

	/* A reject has occured; this is not good! */
	printf ("\nReject Indication:");
	printf ("\n  Sent by %s", rej_info->detected_here ? "Local" : "Remote");
	printf ("\n  Class: %d, Code: %d", rej_info->rej_class, rej_info->rej_code);
	if (rej_info->invoke_known)
		printf ("\n  Invoke: %lu", (unsigned long) rej_info->invoke);
	else
		printf ("\n  Invoke unknown");


	switch (rej_info->rej_class)
	{
	case MMS_REJ_CLASS_CONFIRMED_REQUEST_PDU :
        printf("rej_info : MMS_REJ_CLASS_CONFIRMED_REQUEST_PDU\r\n"); break;
	case MMS_REJ_CLASS_CONFIRMED_RESPONSE_PDU :
        printf("rej_info : MMS_REJ_CLASS_CONFIRMED_RESPONSE_PDU\r\n"); break;	
	case MMS_REJ_CLASS_CONFIRMED_ERROR_PDU :
        printf("rej_info : MMS_REJ_CLASS_CONFIRMED_ERROR_PDU\r\n"); break;
	case MMS_REJ_CLASS_UNCONFIRMED_PDU :
        printf("rej_info : MMS_REJ_CLASS_UNCONFIRMED_PDU\r\n"); break;
	case MMS_REJ_CLASS_PDU_ERROR :
        printf("rej_info : MMS_REJ_CLASS_PDU_ERROR\r\n"); break;	    
	case MMS_REJ_CLASS_CANCEL_REQUEST_PDU :
        printf("rej_info : MMS_REJ_CLASS_CANCEL_REQUEST_PDU\r\n"); break;	    
	case MMS_REJ_CLASS_CANCEL_RESPONSE_PDU :
        printf("rej_info : MMS_REJ_CLASS_CANCEL_RESPONSE_PDU\r\n"); break;	    
	case MMS_REJ_CLASS_CANCEL_ERROR_PDU :
        printf("rej_info : MMS_REJ_CLASS_CANCEL_ERROR_PDU\r\n"); break;	    
	case MMS_REJ_CLASS_CONCLUDE_REQUEST_PDU :
        printf("rej_info : MMS_REJ_CLASS_CONCLUDE_REQUEST_PDU\r\n"); break;	    
	case MMS_REJ_CLASS_CONCLUDE_RESPONSE_PDU :
        printf("rej_info : MMS_REJ_CLASS_CONCLUDE_RESPONSE_PDU\r\n"); break;	    
	case MMS_REJ_CLASS_CONCLUDE_ERROR_PDU :
        printf("rej_info : MMS_REJ_CLASS_CONCLUDE_ERROR_PDU\r\n"); break;
	default:
        printf("rej_info : not classed\r\n");
    }    

#if 0
	switch (rej_info->rej_class)
	{
	case MMS_REJ_CLASS_CONFIRMED_REQUEST_PDU :
	case MMS_REJ_CLASS_CONFIRMED_RESPONSE_PDU :
	case MMS_REJ_CLASS_CONFIRMED_ERROR_PDU :
	case MMS_REJ_CLASS_UNCONFIRMED_PDU :
	case MMS_REJ_CLASS_PDU_ERROR :
	case MMS_REJ_CLASS_CANCEL_REQUEST_PDU :
	case MMS_REJ_CLASS_CANCEL_RESPONSE_PDU :
	case MMS_REJ_CLASS_CANCEL_ERROR_PDU :
	case MMS_REJ_CLASS_CONCLUDE_REQUEST_PDU :
	case MMS_REJ_CLASS_CONCLUDE_RESPONSE_PDU :
	case MMS_REJ_CLASS_CONCLUDE_ERROR_PDU :
	default:
		printf ("Aborting & disabling association"); 
		mi_disable_assoc (mi_assoc);
		_mi_abort_assoc (mi_assoc);
		break;
	}
#endif
}


/************************************************************************/
/*			u_mvl_ident_ind 				*/
/************************************************************************/

ST_VOID u_mvl_ident_ind (MVL_IND_PEND *mvl_ind_ctrl)
{
#if 1	// 2016.11 ChoiBC Modify : Identify received
	iccpShmReceivedIdentify (0);
	QPRINTF (">> Identify received\n");
#endif
#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
	{
		INFO_ICCP_ST *pI = getIccpInfo ();
		pI->dTimeLastIdentifyReceived = sGetMsTime ();
	}
#endif
	mvl_ind_ctrl->u.ident.resp_info = &miuIdentRespInfo;
	mplas_ident_resp (mvl_ind_ctrl);
}

/************************************************************************/
/************************************************************************/
/* MISC. FUNCTIONS							*/
/************************************************************************/
/************************************************************************/
/*			    mi_app_dyn_log_fun				*/
/************************************************************************/
#if 0 //hkkim
ST_VOID mi_app_dyn_log_fun (LOG_CTRL *lc,
	SD_CONST ST_CHAR *timeStr,
	SD_CONST ST_INT logType, // 용도 모름
	SD_CONST ST_CHAR *SD_CONST logTypeStr,
	SD_CONST ST_CHAR *SD_CONST sourceFile,
	SD_CONST ST_INT lineNum,
	SD_CONST ST_INT bufLen,
	SD_CONST ST_CHAR *buf)
{

#if 1	// 2017.11 ChoiBC MMS LOG 추가
	if (opr->mmsDebug == 0)  
		return;
		
	if (logType == 0 && logTypeStr) // logTypeStr이  ACES_DEC  SOCK..등...호출한 놈.
	{                               // logType에 continue 가 있는가...
	    
		if (strncmp (logTypeStr, "MMS_LOG", 7) == 0)
		{
			_mmsDebugFlag = 1; // mmsDebug가 1이고,logType == 0, logTypeStr=MMS_LOG 일때, 
			                    //그 다음 mmsDebug=1이고, logTypeStr가 mms_log가아ㅣ이니도
			
			Debug(console, "\n%s  %s", timeStr, buf);
		}
		else
		{    
			_mmsDebugFlag = 0; // 화면으로는 mms_log만 출력되는데...
	    
	    }
	}
	else
	{  // 연속된 경우 time을 출력하지 않는다.
		if (_mmsDebugFlag) // mi_app_dyn_log_fun (..0,"MMS_LOG)"로 다음부터는 Debug 가능..0,MMS_LOG이되..
		   
			Debug(console, "\n  %s", buf);
	}
#else
	//	QPRINTF ("\nRedirected message: %s",buf);
#endif
}
#else
ST_VOID mi_app_dyn_log_fun (LOG_CTRL *lc,
	SD_CONST ST_CHAR *timeStr,
	SD_CONST ST_INT logType, // 용도 모름
	SD_CONST ST_CHAR *SD_CONST logTypeStr,
	SD_CONST ST_CHAR *SD_CONST sourceFile,
	SD_CONST ST_INT lineNum,
	SD_CONST ST_INT bufLen,
	SD_CONST ST_CHAR *buf)
{

#if 1	// 2017.11 ChoiBC MMS LOG 추가
	if (opr->mmsDebug == 0)  
		return;
		
	if (logType == 0 && logTypeStr) // logTypeStr이  ACES_DEC  SOCK..등...호출한 놈.
	{
		if (strncmp (logTypeStr, "MMS_LOG", 7) == 0)
		{
			_mmsDebugFlag = 1; // mmsDebug가 1이고,logType == 0, logTypeStr=MMS_LOG 일때, 
			                    //그 다음 mmsDebug=1이고, logTypeStr가 mms_log가아ㅣ이니도
			
			Debug(console, "\n%s  %s", timeStr, buf);
		}
		else if (strncmp (logTypeStr, "ACSE", 4) == 0)
		{
			_mmsDebugFlag = 1; // mmsDebug가 1이고,logType == 0, logTypeStr=MMS_LOG 일때, 
			                    //그 다음 mmsDebug=1이고, logTypeStr가 mms_log가아ㅣ이니도
			
			Debug(console, "\n%s  %s", timeStr, buf);
		}	
		else if (strncmp (logTypeStr, "MILOG", 4) == 0)
		{
			_mmsDebugFlag = 1; // mmsDebug가 1이고,logType == 0, logTypeStr=MMS_LOG 일때, 
			                    //그 다음 mmsDebug=1이고, logTypeStr가 mms_log가아ㅣ이니도
			
			Debug(console, "\n%s  %s", timeStr, buf);
		}	
		else if (strncmp (logTypeStr, "TP4", 3) == 0)
		{
			_mmsDebugFlag = 1; // mmsDebug가 1이고,logType == 0, logTypeStr=MMS_LOG 일때, 
			                    //그 다음 mmsDebug=1이고, logTypeStr가 mms_log가아ㅣ이니도
			
			Debug(console, "\n%s  %s", timeStr, buf);
		}	
		else if (strncmp (logTypeStr, "MVLULOG", 7) == 0)
		{
			_mmsDebugFlag = 1; /* 2026-10-02 : mvl FLOW (MVLULOG_FLOW) passthrough for CLI layer-bit control */

			Debug(console, "\n%s  %s", timeStr, buf);
		}	
		else
		{    
			_mmsDebugFlag = 0; // 화면으로는 mms_log만 출력되는데...
	    
	    }
	}
	else
	{  // 연속된 경우 time을 출력하지 않는다.
		if (_mmsDebugFlag) // mi_app_dyn_log_fun (..0,"MMS_LOG)"로 다음부터는 Debug 가능..0,MMS_LOG이되..
		   
			Debug(console, "\n  %s", buf);
	}
#else
	//	QPRINTF ("\nRedirected message: %s",buf);
#endif
}
#endif 

/************************************************************************/
/*			_miUsrKbService 					*/
/************************************************************************/

static ST_RET _miUsrKbService ()
{
#if !defined(NO_KEYBOARD)
	ST_CHAR c;
	ST_DOUBLE elapsedMs;
#if 0	// CHOIBC DELETE
	ST_BOOLEAN quietSave;
#endif
	if (kbhit ())		/* Report test keyboard input */
	{
#if defined(_WIN32) || defined(__QNX__)
		c = getch ();
#else
		c = (ST_CHAR) getchar ();	/* works only if term_init called first	*/
#endif

#ifdef MUI_REDUNDANCY_SUPPORT
		if (miuIrKeyService (c))
			return (SD_TRUE);
#endif
		printf ("_miUsrKbService (%.2X)\n", c);
		if (c == 'x')
		{
			miuDoIt = SD_FALSE;
		}
		if (c == 'q')
		{
			miuQuiet = miuQuiet ? SD_FALSE: SD_TRUE;
			printf ("\nQuiet: %s", miuQuiet ? "On": "Off");
			opr->iccpDebug = miuQuiet;
		}
#if 0	// CHOIBC DELETE
		if (c == 'a')
		{
			_miuAuto = _miuAuto ? SD_FALSE: SD_TRUE;
			printf ("\nAuto mode: %s", _miuAuto ? "On": "Off");
		}
		if (c == 'f')
		{
			miuFullSpeed = miuFullSpeed ? SD_FALSE: SD_TRUE;
			printf ("\nFull Speed: %s", miuFullSpeed ? "On": "Off");
		}
		if (c == 'A')	// All Changes Reported Test
		{
			miuAcrTest = miuAcrTest ? SD_FALSE: SD_TRUE;
			printf ("\nACR Test: %s", miuAcrTest ? "On": "Off");
		}
		if (c == 'i')
		{
			miuAlwaysChangeDv = miuAlwaysChangeDv ? SD_FALSE: SD_TRUE;
			printf ("\nAlways Changing DataValues: %s", miuAlwaysChangeDv ? "On": "Off");
			miuAlwaysChangeDev = miuAlwaysChangeDev ? SD_FALSE: SD_TRUE;
			printf ("\nAlways Changing Devices: %s", miuAlwaysChangeDev ? "On": "Off");
		}
#endif	// CHOIBC DELETE

		if (c == 'e')
		{
			printf ("\nEnabling all associations");
			mi_enable_remote_assoc (NULL);
		}
		if (c == 'd')
		{
			printf ("\nDisabling all associations");
			mi_disable_remote_assoc (NULL);
		}
#if 0	// CHOIBC DELETE
		if (c == 'u')
		{
			quietSave = miuQuiet;
			miuQuiet = SD_FALSE;
			misuChangeNextDv ();
			miuQuiet = quietSave;
		}
		if (c == 'U')
		{
			quietSave = miuQuiet;
			miuQuiet = SD_FALSE;
			misuChangeNextDev ();
			miuQuiet = quietSave;
		}
		if (c == 'm')
			misuSendMessage ();
		if (c == 'r')
		{
			if (_miuClientRemoteAssoc != NULL && _miuClientRemoteAssoc->state == MI_ASSOC_STATE_ACTIVE)
				micuReadAllDv (_miuClientRemoteAssoc->mi_conn);
			else
				printf ("\nNo active client association");
		}
		if (c == 'G')
		{
			if (_miuClientRemoteAssoc != NULL && _miuClientRemoteAssoc->state == MI_ASSOC_STATE_ACTIVE)
				micuGetAllDvAttrib (_miuClientRemoteAssoc->mi_conn);
			else
				printf ("\nNo active client association");
		}
		if (c == 'R')
		{
			if (_miuClientRemoteAssoc != NULL && _miuClientRemoteAssoc->state == MI_ASSOC_STATE_ACTIVE)
				micuReadAllDs (_miuClientRemoteAssoc->mi_conn);
			else
				printf ("\nNo active client association");
		}
		if (c == 'w')
		{
			if (_miuClientRemoteAssoc != NULL && _miuClientRemoteAssoc->state == MI_ASSOC_STATE_ACTIVE)
				micuWriteDv (_miuClientRemoteAssoc->mi_conn);
			else
				printf ("\nNo active client association");
		}
		if (c == 'o')
		{
			if (_miuClientRemoteAssoc != NULL && _miuClientRemoteAssoc->state == MI_ASSOC_STATE_ACTIVE)
				micuTestAllDevices (_miuClientRemoteAssoc->mi_conn);
			else
				printf ("\nNo active client association");
		}
		if (c == 's')
		{
			if (_miuClientRemoteAssoc != NULL && _miuClientRemoteAssoc->state == MI_ASSOC_STATE_ACTIVE)
			{
				printf ("\nStarting client activity");
				micuStartAssocActivity (_miuClientRemoteAssoc->mi_conn);
			}
			else
				printf ("\nNo active client association");
		}
		if (c == 't')
		{
			if (_miuClientRemoteAssoc != NULL && _miuClientRemoteAssoc->state == MI_ASSOC_STATE_ACTIVE)
			{
				printf ("\nStopping client activity");
				micuStopAssocActivity (_miuClientRemoteAssoc->mi_conn);
			}
			else
				printf ("\nNo active client association");
		}
#endif	// CHOIBC DELETE

		if (c == 'l')
		{
#if defined(DEBUG_SISCO)
			printf ("\nReloading log masks");
			logcfgx_ex (sLogCtrl, "logcfg.xml", NULL, SD_TRUE, SD_FALSE);
#endif
		}

		if (c == 'L')
		{
			printf ("\nLogging status & statistics");
			mi_log_status ();
			mi_log_all_stats ();

			elapsedMs = _mi_curr_ms_time -  mi_statistics.start_time;
			if (mi_statistics.num_dsts_dv_txd)
			{
				printf ("\n  %ld DV sent via DSTS     (%.2f per sec)", 
					mi_statistics.num_dsts_dv_txd, 
					1000.0 * ((ST_DOUBLE) mi_statistics.num_dsts_dv_txd / elapsedMs)); 
			}
			if (mi_statistics.num_dsts_dv_rxd)
			{
				printf ("\n  %ld DV received via DSTS (%.2f per sec)", 
					mi_statistics.num_dsts_dv_rxd, 
					1000.0 * ((ST_DOUBLE) mi_statistics.num_dsts_dv_rxd / elapsedMs));
			}
		}

		if (c == 'S')
		{
			printf ("\nResetting statistics ...");
			mi_reset_all_stats ();
			printf (" done.");
		}

		if (c == 'C')
		{
			printf ("\nLogging configuration ...");
			mi_icfg_log_cfg ();
			printf (" done.");
		}

		if (c == 'M')
		{
			printf ("\nLogging memory subsystem status ... ");
			dyn_mem_ptr_status ();  /* Log memory allocation usage */
			printf (" done. ");
		}

#if 0	// CHOIBC DELETE
		if (c == 'H')
		{
			if (mvl_init_ind_hold)
				printf ("\nReleasing initiate indications");
			else
				printf ("\nHolding initiate indications");
			mvl_init_ind_hold = mvl_init_ind_hold ? SD_FALSE: SD_TRUE; 
		}
#endif

		if (c == 'X')
		{
			miuRedirectLogging = miuRedirectLogging ? SD_FALSE: SD_TRUE; 
			if (miuRedirectLogging)
			{
				_slog_dyn_log_fun = mi_app_dyn_log_fun;
			}
			else
				_slog_dyn_log_fun = NULL;

			QPRINTF ("\nRedirect: %s", miuRedirectLogging ? "On": "Off");
		}

#if 0	// CHOIBC DELETE
		if (c == '8')
		{
			if (_miuClientRemoteAssoc != NULL && _miuClientRemoteAssoc->state == MI_ASSOC_STATE_ACTIVE)
			{

				misuSendBlock8 (_miuClientRemoteAssoc->mi_conn);
			}
		}

		if (c == 'D')
		{
			miuDeleteAllDs = miuDeleteAllDs ? SD_FALSE : SD_TRUE;
			QPRINTF ("\nDeleteAllDataSets:  %s", miuDeleteAllDs ? "On" : "Off");
		}
#endif	// CHOIBC DELETE

#if defined(S_SEC_ENABLED)
		if (c == 'E')
		{
			/* check certificates expiration on all secured connections */
			printf ("\nChecking certificates expiration ...");
			_miuCheckCertExpiration ();
		}
#endif

		if (c == '?')
		{
			printf ("\n\n Options:");
			printf ("\n   x = Exit");
			printf ("\n   q = Quiet mode toggle");
			printf ("\n   a = Auto mode toggle");
			printf ("\n   i = Always change DataValue toggle");
			printf ("\n   f = Full speed toggle");
			printf ("\n   e = Enable all associations");
			printf ("\n   d = Disable all associations");
			printf ("\n   A = ACR Test                   (server option)");
			printf ("\n   u = Update a local DataValue   (server option)");
			printf ("\n   U = Update a local Device      (server option)");
			printf ("\n   m = Send a message             (server option)");
			printf ("\n   r = Read all remote DataValues (client option)");
			printf ("\n   R = Read all remote DataSets   (client option)");
			printf ("\n   w = Write a remote DataValue   (client option)");
			printf ("\n   o = Operate all remote Devices (client option)");
			printf ("\n   G = Get Attributes of all DVR  (client option)");
			printf ("\n   s = Start client activity      (client option)");
			printf ("\n   t = Terminate client activity  (client option)");
			printf ("\n   l = Reload log masks");
			printf ("\n   L = Log Status & Statistics");
			printf ("\n   S = Reset Statistics");
			printf ("\n   C = Log Configuration");
			printf ("\n   M = Log Memory");
			printf ("\n   H = Hold initiate indications");
			printf ("\n   X = Redirect SLOG Messages to the screen");
			printf ("\n   8 = Send Transfer Accounts");
			printf ("\n   D = Toggle DeleteAllDataSets option");
#if defined (S_SEC_ENABLED)
			printf ("\n   E = Check certificates expiration");
#endif


#ifdef MUI_REDUNDANCY_SUPPORT
			miuIrKeyHelp ();
#endif
		}

#if defined(linux)
		while (kbhit ())	/* Flush keyboard */
			getchar ();

#elif defined(_AIX)
		/* on AIX there is only one kbhit indication for key+NL sequence	*/
		if (c != '\n')
			getchar ();	/* flush the NL */

#elif defined(__VMS) 
		getchar ();

#else
		while (kbhit ())	/* Flush keyboard */
			getch ();
#endif
	}
#endif /* #if !defined(NO_KEYBOARD) */
	return (SD_TRUE);
}

/************************************************************************/
/************************************************************************/
/*			    mit_app_dyn_log_fun				*/
/************************************************************************/

ST_VOID 	mit_app_dyn_log_fun (LOG_CTRL *lc,
	SD_CONST ST_CHAR *timeStr,
	SD_CONST ST_INT logType,
	SD_CONST ST_CHAR *SD_CONST logTypeStr,
	SD_CONST ST_CHAR *SD_CONST sourceFile,
	SD_CONST ST_INT lineNum,
	SD_CONST ST_INT bufLen,
	SD_CONST ST_CHAR *buf)
{
	QPRINTF ("%s\n", buf);
}
/************************************************************************/
/* Memory Allocation Error Handling Functions.				*/
/* These functions are called from mem_chk when it is unable to 	*/
/* perform the requested operation. These functions must either return 	*/
/* a valid buffer or not return at all.					*/
/************************************************************************/

static ST_CHAR *spareMem;

static ST_VOID _miuMemChkErrorDetected (ST_VOID)
{
	free (spareMem);
	printf ("\nMemory Error Detected! Check log file");
	dyn_mem_ptr_status ();
}

/* These are called when unable to allocate memory, which is fatal	*/
static ST_VOID *_miuMallocError (ST_UINT size)
{
	_miuMemChkErrorDetected ();
	exit (2);
	return (NULL);   
}
static ST_VOID *_miuCallocError (ST_UINT num, ST_UINT size)
{
	_miuMemChkErrorDetected ();
	exit (3);
	return (NULL);   
}
static ST_VOID *_miuReallocError (ST_VOID *old, ST_UINT size)
{
	_miuMemChkErrorDetected ();
	exit (4);
	return (NULL);   
}


/************************************************************************/
/*			_miuInitMem					*/
/************************************************************************/

static ST_VOID _miuInitMem ()
{
#if defined(NO_GLB_VAR_INIT)
	mvl_init_glb_vars ();
#endif

	/* Allocate spare memory to allow logging/printing memory errors	*/
	spareMem = (ST_CHAR *) malloc (500);

	/* trap mem_chk errors							*/
	mem_chk_err  = _miuMemChkErrorDetected;
	m_memerr_fun = _miuMallocError;
	c_memerr_fun = _miuCallocError;
	r_memerr_fun = _miuReallocError;

	/* Turn on debug mode. This makes things run slower but will catch 	*/
	/* many memory related errors during development			*/
#ifdef DEBUG_SISCO
	m_mem_debug  = SD_TRUE; 
#endif
}

#if 0	// CHOIBC DELETE
/************************************************************************/
/*			  miuEnableBlock8				*/
/************************************************************************/

ST_VOID miuEnableBlock8 (ST_VOID)
{
	MI_REMOTE *mi_remote;

	mi_remote = mi_remote_list;
	while (mi_remote)
	{
		if (miuBlock8Server)
		{
			if (mi_remote->mis_vcc != NULL)
				mis_enable_transfer_accounts (mi_remote->mis_vcc);
		}

		if (miuBlock8Client)
			if (mi_remote->mic_vcc != NULL)
				mic_enable_transfer_accounts (mi_remote->mic_vcc);

		mi_remote = (MI_REMOTE *) list_get_next (mi_remote_list, mi_remote);
	}

	mms_debug_sel |= MMS_LOG_MIDD;
}

#endif


/************************************************************************/
/************************************************************************/
/************************************************************************/

ST_VOID u_mvl_getvar_ind (MVL_IND_PEND *mvl_ind_ctrl)
{
	_mplas_err_resp (mvl_ind_ctrl,4,0);	/* class service, code other */
}
ST_VOID u_mvl_namelist_ind (MVL_IND_PEND *mvl_ind_ctrl)
{
	_mplas_err_resp (mvl_ind_ctrl,4,0);	/* class service, code other */
}
ST_VOID u_mvl_write_ind (MVL_IND_PEND *mvl_ind_ctrl)
{
	_mplas_err_resp (mvl_ind_ctrl,4,0);	/* class service, code other */
}
ST_VOID u_mvl_read_ind (MVL_IND_PEND *mvl_ind_ctrl)
{
	_mplas_err_resp (mvl_ind_ctrl,4,0);	/* class service, code other */
}
ST_VOID u_mvl_defvlist_ind (MVL_IND_PEND *mvl_ind_ctrl)
{
	_mplas_err_resp (mvl_ind_ctrl,4,0);	/* class service, code other */
}
ST_VOID u_mvl_delvlist_ind (MVL_IND_PEND *mvl_ind_ctrl)
{
	_mplas_err_resp (mvl_ind_ctrl,4,0);	/* class service, code other */
}
ST_VOID u_mvl_getvlist_ind (MVL_IND_PEND *mvl_ind_ctrl)
{
	_mplas_err_resp (mvl_ind_ctrl,4,0);	/* class service, code other */
}
ST_VOID u_mvl_info_rpt_ind (MVL_COMM_EVENT *event)
{
	_mvl_free_comm_event (event);
}


#if defined(S_SEC_ENABLED) // sisco security 가 enable 된 경우가 보다.
/************************************************************************/
/*		_miuCheckCertExpiration					*/
/************************************************************************/
static ST_RET _miuCheckCertExpiration (ST_VOID)
{
	ST_RET             ret = SD_SUCCESS;
	ST_INT             i;
	MVL_NET_INFO      *cc;
	S_SEC_CERT_STATUS  certStatus;

	/* CALLING... */
	printf ("\n\n CALLING CONNECTIONS: ");
	printf ("\n\n ------------------- ");
	cc = mvl_calling_conn_ctrl;
	for (i = 0; i < mvl_cfg_info->num_calling; ++i, ++cc)
	{
		if (!(cc->in_use && cc->conn_active))
			continue;  /* next cc */

		memset (&certStatus, 0, sizeof(S_SEC_CERT_STATUS));
		ret = ulCheckCertExpiration (cc, SD_TRUE, &certStatus);
		if (ret != SD_SUCCESS)
			break;
		ret = ulPrintSecConnInfo (cc,  &certStatus);
		if (ret != SD_SUCCESS)
			break;
	}

	if (ret == SD_SUCCESS)
	{
		/* CALLED... */
		printf ("\n\n CALLED CONNECTIONS: ");
		printf ("\n\n ------------------- ");
		cc = mvl_called_conn_ctrl;
		for (i = 0; i < mvl_cfg_info->num_called; ++i, ++cc)
		{
			if (!(cc->in_use && cc->conn_active))
				continue;  /* next cc */
			memset (&certStatus, 0, sizeof(S_SEC_CERT_STATUS));
			ret = ulCheckCertExpiration (cc, SD_TRUE, &certStatus);
			if (ret != SD_SUCCESS)
				break;
			ret = ulPrintSecConnInfo (cc,  &certStatus);
			if (ret != SD_SUCCESS)
				break;
		}
	}

	return (ret);
}
#endif /* defined(S_SEC_ENABLED) */
