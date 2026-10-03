/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,       */
/*          2001-2002, All Rights Reserved                              */
/*                                                                      */
/* MODULE NAME : miu_icfg.c                                             */
/* PRODUCT(S)  : MI                                                     */
/*                                                                      */
/*                                                                      */
/* MODULE DESCRIPTION :                                                 */
/*                                                                      */
/*                                                                      */
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :                            */
/*                                                                      */
/* MODIFICATION LOG :                                                   */
/*  Date     Who   Rev          Comments                                */
/* --------  ---  ------  --------------------------------------------  */
/* 01/30/06  GLB     03    Integrated porting changes for VMS           */
/* 06/06/05  MDE     02    LINUX warning cleanup			*/
/* 05/24/05  MDE     01    Created                                      */
/************************************************************************/

#include "glbtypes.h"
#include "sysincs.h"
#include "mem_chk.h"
#include "mi.h"
#include "mi_icfg.h"
#include "mi_log.h"

#if 1	// 2016.04.11 ChoiBC Modify
#include "mi_usr.h"
#include "rtubuf.h"
#endif

/************************************************************************/
/* For debug version, use a static pointer to avoid duplication of 	*/
/* __FILE__ strings.							*/
/************************************************************************/

#ifdef DEBUG_SISCO
static ST_CHAR *thisFileName = __FILE__;
#endif

/************************************************************************/
#define ICFGU_MAX_PDU_SIZE      32000
#define ICFGU_MAX_DS            10
#define ICFGU_MAX_DSTS          10
#if 1	// 2016.04.11 ChoiBC Modify
#define ICFGU_DEFAULT_SHORTEST_INTERVAL		1
#define ICFGU_DEFAULT_INITIATE_TIMEOUT		5
#define ICFGU_DEFAULT_CONCLUDE_TIMEOUT		5
#define ICFGU_DEFAULT_HEARTBIT_TIME			3
#define ICFGU_DEFAULT_MAX_REQ_PEND			5
#define ICFGU_DEFAULT_MAX_IND_PEND			5
#define ICFGU_DEFAULT_MAX_NEST				10
#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
#define ICFGU_DEFAULT_MAX_IDENTIFY_TIMEOUT	0
#endif
#define ICFGU_DEFAULT_POINT_NUMBER_WIDTH	3

#endif

#if 0	// CHOIBC DELETE
#define ICFGU_NUM_LDV           10
#define ICFGU_NUM_LDEV          5
#define ICFGU_NUM_LIM           1
#define ICFGU_NUM_RCC           1
#define ICFGU_NUM_ASSOC         1
#define ICFGU_NUM_RDV           10
#define ICFGU_NUM_RDEV          5
#define ICFGU_NUM_RIM           1

static ST_RET icfgUsr (ST_INT serviceRole, MI_ICFG_CTRL *miIcfgCtrl);

/************************************************************************/
/************************************************************************/
/*                      miuIcfg                                         */
/************************************************************************/

ST_RET miuIcfg (ST_INT serviceRole, ST_INT mode, ST_CHAR *remoteName, ST_VOID *usr)
{
	MI_ICFG_CTRL lc;
	ST_RET rc;

// mi_debug_sel 는 lib에서 사용하는 flag이고
// icfg_debug_sels는 iccp 에서 사용하는 flag인 듯

	/* We mirror the MI log masks into the ICFG log mask */
	if (mi_debug_sel & MILOG_ERR)
		icfg_debug_sel |= ICFG_LOG_ERR;

	if (mi_debug_sel & MILOG_NERR)
		icfg_debug_sel |= ICFG_LOG_NERR;

	if (mi_debug_sel & MILOG_CFG)
		icfg_debug_sel |= ICFG_LOG_FLOW;

	lc.usr = usr;
	lc.mode = mode;
	lc.remote_name = remoteName;

	rc = icfgUsr (serviceRole, &lc);
	if (rc == SD_SUCCESS && lc.mode == MI_ICFG_MODE_RELOAD)
	{
		if (lc.target_remote_found != SD_TRUE)
		{
			MI_LOG_NERR1 ("Reload error, remote name '%s' not found", remoteName);
			rc = MI_ICFG_ERR_REMOTE_NOT_FOUND;      
		}
	}
	return (rc);
}

/************************************************************************/
/*                      icfgUsr                                         */
/************************************************************************/

static ST_RET icfgUsr (ST_INT serviceRole, MI_ICFG_CTRL *miIcfgCtrl)
{
	ICFG_LOCAL_CC *icfgLocalCc;
	ICFG_LOCAL_DV *icfgLdv;
	ICFG_LOCAL_DEV *icfgLdev; 
	ICFG_LOCAL_INFOMSG *icfgLim;
	ICFG_REMOTE_CC *icfgRemoteCc; 
	ICFG_ASSOCIATION *icfgAssoc; 
	ICFG_SERVER_OBJ_COUNTS *serverObjCounts;
	ICFG_SERVER_DV *icfgSdv;
	ICFG_SERVER_DEV *icfgSdev;
	ICFG_SERVER_INFOMSG *icfgSim;
	ICFG_SERVER_DS *icfgSds;
	ICFG_CLIENT_OBJ_COUNTS *clientObjCounts;
	ICFG_CLIENT_DV *icfgCdv; 
	ICFG_CLIENT_DEV *icfgCdev; 
	ICFG_CLIENT_INFOMSG *icfgCim; 
	ICFG_CLIENT_DS *icfgCds; 
	ICFG_CLIENT_DSTS *icfgDsTs; 
	MIC_GROBJ_CTRL *icfgGrobjCtrl;
	MIC_AUTO_DSTS_PARAM *autoDstsParam;
	ST_INT numDv;
	ST_INT numDev;
	ST_INT numIm;
	ST_INT numCC;
	ST_INT numAssoc;
	ST_INT numDs;
	ST_INT numDsTs;
	ST_INT gRef;
	ST_INT i;
	ST_INT j;
	ST_INT k;
	ST_RET ret;

	/* We can pass information all the way through the ICFG subsystem and */
	/* to the application code. Demonstrate this by passing an artificial */
	/* reference.                                                         */
	gRef = 42;

	/* Local Control Center */
	icfgLocalCc = (ICFG_LOCAL_CC *) chk_calloc (1, sizeof (ICFG_LOCAL_CC));
	strcpy (icfgLocalCc->name, "SampleLCC");
	icfgLocalCc->maxDs = ICFGU_MAX_DSTS;
	icfgLocalCc->maxDsTs = ICFGU_MAX_DSTS;
	icfgLocalCc->maxMmsPduSize = ICFGU_MAX_PDU_SIZE;
	icfgLocalCc->icfgRef.ref = ++gRef; 
	ret = u_icfg_StartLocalControlCenter (miIcfgCtrl, icfgLocalCc);

	/* Local Objects */
	if (serviceRole == ICFG_ROLE_SERVER)
	{
		u_icfg_StartLocalObjects (miIcfgCtrl);

		/* Local Data Values */
		numDv = ICFGU_NUM_LDV;
		ret = u_icfg_StartLocalDataValues (miIcfgCtrl, numDv);
		for (i = 0; i < numDv; ++i)
		{
			icfgLdv = (ICFG_LOCAL_DV *) chk_calloc (1, sizeof (ICFG_LOCAL_DV));
			sprintf (icfgLdv->name, "Dv%03d", i);
			icfgLdv->dvType       = RealExtended;
			icfgLdv->normalSource = Telemetered;
			icfgLdv->minPresent   = SD_FALSE;
			icfgLdv->maxPresent   = SD_FALSE;
			icfgLdv->icfgRef.ref = ++gRef;
			ret = u_icfg_LocalDataValue (miIcfgCtrl, icfgLdv);
		}
		u_icfg_EndLocalDataValues (miIcfgCtrl);

		/* Local Devices */
		numDev = ICFGU_NUM_LDEV;
		ret = u_icfg_StartLocalDevices (miIcfgCtrl, numDev);
		for (i = 0; i < numDev; ++i)
		{
			icfgLdev = (ICFG_LOCAL_DEV *) chk_calloc (1, sizeof (ICFG_LOCAL_DEV));
			sprintf (icfgLdev->name, "Dev%03d", i);
			icfgLdev->devType = DevReal;
			icfgLdev->sbo = SD_TRUE; 
			icfgLdev->checkBackId = i+1;
			icfgLdev->selTime = 30;  
			icfgLdev->tagEnable = SD_TRUE; 
			icfgLdev->icfgRef.ref = ++gRef; 
			ret = u_icfg_LocalDevice (miIcfgCtrl, icfgLdev);
		}
		ret = u_icfg_EndLocalDevices (miIcfgCtrl);

		/* Local Information Messages */
		numIm = ICFGU_NUM_LIM;
		ret = u_icfg_StartLocalInfoMsgs (miIcfgCtrl, numIm);
		for (i = 0; i < numIm; ++i)
		{
			icfgLim = chk_calloc (1, sizeof (ICFG_LOCAL_INFOMSG));
			icfgLim->infoRef = i + 100;
			icfgLim->maxSize = (i + 1) * 100; 
			icfgLim->icfgRef.ref = ++gRef;
			ret = u_icfg_LocalInfoMsg (miIcfgCtrl, icfgLim);
		}
		ret = u_icfg_EndLocalInfoMsgs (miIcfgCtrl);
		u_icfg_EndLocalObjects (miIcfgCtrl);
	} /* serviceRole == ICFG_ROLE_SERVER */

	/* Remote Control Centers */
	numCC = ICFGU_NUM_RCC;

	ret = u_icfg_StartRemoteCtrlCenters (miIcfgCtrl, numCC);

	for (i = 0; i < numCC; ++i)
	{
		icfgRemoteCc = (ICFG_REMOTE_CC *) chk_calloc (1, sizeof (ICFG_REMOTE_CC));
		sprintf (icfgRemoteCc->name, "Rcc%d", i);
		sprintf (icfgRemoteCc->blt.name, "BLT_NAME");
		sprintf (icfgRemoteCc->blt.id, "BLT_ID");
		sprintf (icfgRemoteCc->blt.localDom, "DomX");
		sprintf (icfgRemoteCc->blt.remoteDom, "DomX");
		icfgRemoteCc->blt.shortestInterval = 1; 
		icfgRemoteCc->blt.blocks = ICFG_BLOCK1 | ICFG_BLOCK2 | ICFG_BLOCK4 | ICFG_BLOCK5; 
		icfgRemoteCc->icfgRef.ref = ++gRef;
		ret =  u_icfg_StartRemoteControlCenter (miIcfgCtrl, icfgRemoteCc);

		/* Associations */
		numAssoc = ICFGU_NUM_ASSOC;
		ret = u_icfg_StartAssociations (miIcfgCtrl, numAssoc);
		for (j = 0; j < numAssoc; ++j)
		{
			icfgAssoc = (ICFG_ASSOCIATION *) chk_calloc (1, sizeof (ICFG_ASSOCIATION));
			sprintf (icfgAssoc->name, "Assoc%d", j);
			sprintf (icfgAssoc->localAr, "LocalAr");

			icfgAssoc->totalRemoteArNames = 1;
			sprintf (icfgAssoc->remoteAr[0], "RemoteAr");
			icfgAssoc->serviceRole = serviceRole;
			if (serviceRole == ICFG_ROLE_CLIENT)
				icfgAssoc->connectRole = ICFG_ROLE_CALLING;
			else
				icfgAssoc->connectRole = ICFG_ROLE_CALLED;

			icfgAssoc->retryTime = 10;
			icfgAssoc->initiateTimeout = 5;
			icfgAssoc->concludeTimeout = 5;
			icfgAssoc->heartbeatTime = 30;
			icfgAssoc->maxMmsPduSize = ICFGU_MAX_PDU_SIZE;
			icfgAssoc->maxReqPend = 5;
			icfgAssoc->maxIndPend = 5;
			icfgAssoc->maxNest = 10;
			icfgAssoc->icfgRef.ref = ++gRef;
			ret = u_icfg_Association (miIcfgCtrl, icfgAssoc);
		}
		ret = u_icfg_EndAssociations (miIcfgCtrl);

		/* Server Objects */
		serverObjCounts = (ICFG_SERVER_OBJ_COUNTS *) chk_calloc (1, sizeof (ICFG_SERVER_OBJ_COUNTS));
		if (serviceRole == ICFG_ROLE_SERVER)
		{
			/* Server Data Values */
			serverObjCounts->numVccDv         = numDv  = ICFGU_NUM_LDV;
			serverObjCounts->numVccDev        = numDev = ICFGU_NUM_LDEV;
			serverObjCounts->numVccInfoMsg    = numIm  = ICFGU_NUM_LIM;
			serverObjCounts->numVccDs         = 0;
			serverObjCounts->numIccDv         = 0;
			serverObjCounts->numIccDev        = 0;
			serverObjCounts->numIccInfoMsg    = 0;
			serverObjCounts->numIccDs         = 0;
			u_icfg_StartServerObjects (miIcfgCtrl, serverObjCounts);

			ret = u_icfg_StartServerDataValues (miIcfgCtrl, numDv);
			for (j = 0; j < numDv; ++j)
			{
				icfgSdv = (ICFG_SERVER_DV *) chk_calloc (1, sizeof (ICFG_SERVER_DV));
				sprintf (icfgSdv->name, "Dv%03d", j);
				icfgSdv->scope = VCC;
				icfgSdv->dvType = RealExtended;
				icfgSdv->readOnly = SD_TRUE;
				icfgSdv->icfgRef.ref = ++gRef;
				ret = u_icfg_ServerDataValue (miIcfgCtrl, icfgSdv);
			}
			ret = u_icfg_EndServerDataValues (miIcfgCtrl);

			/* Server Devices */
			ret = u_icfg_StartServerDevices (miIcfgCtrl, numDev);
			for (j = 0; j < numDev; ++j)
			{
				icfgSdev = (ICFG_SERVER_DEV *) chk_calloc (1, sizeof (ICFG_SERVER_DEV));
				sprintf (icfgSdev->name, "Dev%03d", j);
				icfgSdev->scope = VCC;
				icfgSdev->devType = DevReal;
				icfgSdev->tagEnable = SD_TRUE; 
				icfgSdev->icfgRef.ref = ++gRef;
				ret = u_icfg_ServerDevice (miIcfgCtrl, icfgSdev);
			}
			ret = u_icfg_EndServerDevices (miIcfgCtrl);

			/* Server Information Messages */
			ret = u_icfg_StartServerInfoMsgs (miIcfgCtrl, numIm);
			for (j = 0; j < numIm; ++j)
			{
				icfgSim = (ICFG_SERVER_INFOMSG *) chk_calloc (1, sizeof (ICFG_SERVER_INFOMSG));
				icfgSim->infoRef = i + 100;   
				icfgSim->scope = VCC;
				icfgSim->icfgRef.ref = ++gRef;
				ret = u_icfg_ServerInfoMsg (miIcfgCtrl, icfgSim);
			}
			ret = u_icfg_EndServerInfoMsgs (miIcfgCtrl);

			/* Server DataSets */
			numDs = 0;
			numDv = 0;
			ret = u_icfg_StartServerDataSets (miIcfgCtrl, numDs);
			for (j = 0; j < numDs; ++j)
			{
				icfgSds = (ICFG_SERVER_DS *) chk_calloc (1, sizeof (ICFG_SERVER_DS) + (numDv * sizeof (ICFG_SERVER_DS_DV)));
				sprintf (icfgSds->name, "Sds%d", j);
				icfgSds->scope = VCC;
				icfgSds->transferSetName = SD_TRUE;
				icfgSds->transferSetTimeStamp = SD_TRUE;
				icfgSds->dsConditionsDetected = SD_TRUE;
				icfgSds->eventCodeDetected = SD_TRUE;
				icfgSds->numDv = numDv;
				icfgSds->dvArray = (ICFG_SERVER_DS_DV *) (icfgSds+1);
				for (k = 0; k < numDv; ++k)
				{
					sprintf (icfgSds->dvArray[j].name, "Dv%03d", k);
					icfgSds->dvArray[j].scope = VCC;
				}
				icfgSds->icfgRef.ref = ++gRef;
				ret = u_icfg_ServerDataSet (miIcfgCtrl, icfgSds);
			}
			ret = u_icfg_EndServerDataSets (miIcfgCtrl);
			ret = u_icfg_EndServerObjects (miIcfgCtrl);
		}

		/* Client Objects */
		clientObjCounts = (ICFG_CLIENT_OBJ_COUNTS *) chk_calloc (1, sizeof (ICFG_CLIENT_OBJ_COUNTS));
		if (serviceRole == ICFG_ROLE_CLIENT)
		{
			clientObjCounts->numVccDv         = numDv  = ICFGU_NUM_RDV;
			clientObjCounts->numVccDev        = numDev = ICFGU_NUM_RDEV;
			clientObjCounts->numVccInfoMsg    = numIm  = ICFGU_NUM_RIM;
			clientObjCounts->numIccDv         = 0;
			clientObjCounts->numIccDev        = 0;
			clientObjCounts->numIccInfoMsg    = 0;
			clientObjCounts->numDs            = numDs   = 1;
			clientObjCounts->numDsTs          = numDsTs = 1;
			ret = u_icfg_StartClientObjects (miIcfgCtrl, clientObjCounts);

			/* Client Data Values */
			ret = u_icfg_StartClientDataValues (miIcfgCtrl, numDv);
			for (j = 0; j < numDv; ++j)
			{
				icfgCdv = (ICFG_CLIENT_DV *) chk_calloc (1, sizeof (ICFG_CLIENT_DV));
				sprintf (icfgCdv->name, "Dv%03d", j);
				icfgCdv->scope = VCC;
				icfgCdv->dvType = RealExtended;
				icfgCdv->readOnly = SD_TRUE;
				icfgCdv->icfgRef.ref = ++gRef;
				ret = u_icfg_ClientDataValue (miIcfgCtrl, icfgCdv);
			}
			ret = u_icfg_EndClientDataValues (miIcfgCtrl);

			/* Client Devices */
			ret = u_icfg_StartClientDevices (miIcfgCtrl, numDev);
			for (j = 0; j < numDev; ++j)
			{
				icfgCdev = (ICFG_CLIENT_DEV *) chk_calloc (1, sizeof (ICFG_CLIENT_DEV));
				sprintf (icfgCdev->name, "Dev%03d", j);
				icfgCdev->scope = VCC;
				icfgCdev->devType = DevReal;
				icfgCdev->tagEnable = SD_TRUE; 
				icfgCdev->icfgRef.ref = ++gRef;
				ret = u_icfg_ClientDevice (miIcfgCtrl, icfgCdev);
			}
			ret = u_icfg_EndClientDevices (miIcfgCtrl);

			/* Client Information Messages */
			ret = u_icfg_StartClientInfoMsgs (miIcfgCtrl, numDv);
			for (j = 0; j < numIm; ++j)
			{
				icfgCim = (ICFG_CLIENT_INFOMSG *) chk_calloc (1, sizeof (ICFG_CLIENT_INFOMSG));
				icfgCim->infoRef = i + 100;         icfgCim->scope = VCC;
				icfgCim->scope = VCC;
				icfgCim->icfgRef.ref = ++gRef;
				ret = u_icfg_ClientInfoMsg (miIcfgCtrl, icfgCim);
			}
			ret = u_icfg_EndClientInfoMsgs (miIcfgCtrl);

			/* Client Data Sets */  
			ret = u_icfg_StartClientDataSets (miIcfgCtrl, numDs);
			for (j = 0; j < numDs; ++j)
			{
				icfgCds = (ICFG_CLIENT_DS *) chk_calloc (1, sizeof (ICFG_CLIENT_DS) + (numDv * sizeof (ICFG_CLIENT_DS_DV)));
				sprintf (icfgCds->name, "Cds%d", j);
				icfgCds->transferSetName = SD_TRUE;
				icfgCds->transferSetTimeStamp = SD_TRUE;
				icfgCds->dsConditionsDetected = SD_TRUE;
				icfgCds->eventCodeDetected = SD_TRUE;
				icfgCds->numDv = numDv;
				icfgCds->dvArray = (ICFG_CLIENT_DS_DV *) (icfgCds+1);
				for (k = 0; k < numDv; ++k)
				{
					sprintf (icfgCds->dvArray[k].name, "Dv%03d", k);
					icfgCds->dvArray[k].scope = VCC;
				}
				icfgCds->icfgRef.ref = ++gRef;
				ret = u_icfg_ClientDataSet (miIcfgCtrl, icfgCds);
			}
			ret = u_icfg_EndClientDataSets (miIcfgCtrl);

			/* Client Data Set Transfer Sets */  
			ret = u_icfg_StartClientDSTSs (miIcfgCtrl, numDsTs);
			for (j = 0; j < numDsTs; ++j)
			{
				icfgDsTs = (ICFG_CLIENT_DSTS *) chk_calloc (1, sizeof (ICFG_CLIENT_DSTS));

				sprintf (icfgDsTs->dsName, "Cds%d", j);
				sprintf (icfgDsTs->assocName, "Assoc0");
				icfgDsTs->interval = 0;
				icfgDsTs->rbe = SD_TRUE;
				icfgDsTs->bufferTime = 2;
				icfgDsTs->integrity = 30;
				icfgDsTs->startTime = 0;
				icfgDsTs->tle = 0;
				icfgDsTs->dsConditionsRequested = ICFG_DSC_INTEGRITY | ICFG_DSC_CHANGE;
				icfgDsTs->critical = SD_FALSE;
				icfgDsTs->icfgRef.ref = ++gRef;
				ret = u_icfg_ClientDSTS (miIcfgCtrl, icfgDsTs);
			}
			ret = u_icfg_EndClientDSTSs (miIcfgCtrl);

			/* Discovery */  
			icfgGrobjCtrl = (MIC_GROBJ_CTRL *) chk_calloc (1, sizeof (MIC_GROBJ_CTRL));
			sprintf (icfgGrobjCtrl->assocName, "Assoc0");
			icfgGrobjCtrl->enable = SD_TRUE;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_EXECUTE_ONCE;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_EXECUTE_ALWAYS;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_GETNAMELIST;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_CFG_GETVAA;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_CFG_READ;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_REMOVE_MISSING_FROM_DS;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_REMOVE_MISTYPED_FROM_DS;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_REMOVE_READ_ERROR_FROM_DS;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_DB_DELETE_MISSING;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_DB_CORRECT_MISTYPED ;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_NEW_GETVAA;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_NEW_READ;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_NEW_DV_RDONLY;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_NEW_ADD_DV;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_DB_ADD_NEW;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_DB_AUTO_ACCEPT;
			icfgGrobjCtrl->control |= MIC_GROBJ_CTRL_WRITE_XML;
			ret = u_icfg_Discovery (miIcfgCtrl, icfgGrobjCtrl);

			/* Auto-DSTS */
			autoDstsParam = (MIC_AUTO_DSTS_PARAM *) chk_calloc (1, sizeof (MIC_AUTO_DSTS_PARAM));
			autoDstsParam->enable       =  SD_TRUE;
			autoDstsParam->assignByType =  SD_TRUE;
			sprintf (autoDstsParam->assocName, "Assoc0");
			if (autoDstsParam->assignByType == SD_FALSE)
			{
				/* All DSTS use the same parameters */
				autoDstsParam->a.enable = SD_TRUE;
				autoDstsParam->a.dsParam.maxDstsPduSize         = ICFGU_MAX_PDU_SIZE;
				autoDstsParam->a.dsParam.maxDvPerDs             = 0;
				autoDstsParam->a.dsParam.conservative           = SD_TRUE;
				autoDstsParam->a.dstsData.RBE                   = SD_TRUE;
				autoDstsParam->a.dsParam.transferSetName        = SD_TRUE;
				autoDstsParam->a.dsParam.transferSetTimeStamp   = SD_TRUE;
				autoDstsParam->a.dsParam.dsConditionsDetected   = SD_TRUE;
				autoDstsParam->a.dsParam.eventCodeDetected      = SD_TRUE;
				autoDstsParam->a.dstsData.Critical              = SD_FALSE;
				autoDstsParam->a.dstsData.StartTime             = 0;
				autoDstsParam->a.dstsData.Interval              = 0;
				autoDstsParam->a.dstsData.TLE                   = 0;
				autoDstsParam->a.dstsData.BufferTime            = 2;
				autoDstsParam->a.dstsData.IntegrityCheck        = 30;
				autoDstsParam->a.dstsData.DSConditionsRequested = MI_DSC_INTEGRITY | MI_DSC_CHANGE;
			}
			else
			{
				/* Real DSTS parameters */
				autoDstsParam->r.enable = SD_TRUE;
				autoDstsParam->r.dsParam.maxDstsPduSize         = ICFGU_MAX_PDU_SIZE;
				autoDstsParam->r.dsParam.maxDvPerDs             = 0;
				autoDstsParam->r.dsParam.conservative           = SD_TRUE;
				autoDstsParam->r.dstsData.RBE                   = SD_TRUE;
				autoDstsParam->r.dsParam.transferSetName        = SD_TRUE;
				autoDstsParam->r.dsParam.transferSetTimeStamp   = SD_TRUE;
				autoDstsParam->r.dsParam.dsConditionsDetected   = SD_TRUE;
				autoDstsParam->r.dsParam.eventCodeDetected      = SD_TRUE;
				autoDstsParam->r.dstsData.Critical              = SD_FALSE;
				autoDstsParam->r.dstsData.StartTime             = 0;
				autoDstsParam->r.dstsData.Interval              = 0;
				autoDstsParam->r.dstsData.TLE                   = 0;
				autoDstsParam->r.dstsData.BufferTime            = 2;
				autoDstsParam->r.dstsData.IntegrityCheck        = 30;
				autoDstsParam->r.dstsData.DSConditionsRequested = MI_DSC_INTEGRITY | MI_DSC_CHANGE;

				/* Discrete DSTS parameters */
				autoDstsParam->d.enable = SD_TRUE;
				autoDstsParam->d.dsParam.maxDstsPduSize         = ICFGU_MAX_PDU_SIZE;
				autoDstsParam->d.dsParam.maxDvPerDs             = 0;
				autoDstsParam->d.dsParam.conservative           = SD_TRUE;
				autoDstsParam->d.dstsData.RBE                   = SD_TRUE;
				autoDstsParam->d.dsParam.transferSetName        = SD_TRUE;
				autoDstsParam->d.dsParam.transferSetTimeStamp   = SD_TRUE;
				autoDstsParam->d.dsParam.dsConditionsDetected   = SD_TRUE;
				autoDstsParam->d.dsParam.eventCodeDetected      = SD_TRUE;
				autoDstsParam->d.dstsData.Critical              = SD_FALSE;
				autoDstsParam->d.dstsData.StartTime             = 0;
				autoDstsParam->d.dstsData.Interval              = 0;
				autoDstsParam->d.dstsData.TLE                   = 0;
				autoDstsParam->d.dstsData.BufferTime            = 2;
				autoDstsParam->d.dstsData.IntegrityCheck        = 30;
				autoDstsParam->d.dstsData.DSConditionsRequested = MI_DSC_INTEGRITY | MI_DSC_CHANGE;

				/* State DSTS parameters */
				autoDstsParam->s.enable = SD_TRUE;
				autoDstsParam->s.dsParam.maxDstsPduSize         = ICFGU_MAX_PDU_SIZE;
				autoDstsParam->s.dsParam.maxDvPerDs             = 0;
				autoDstsParam->s.dsParam.conservative           = SD_TRUE;
				autoDstsParam->s.dstsData.RBE                   = SD_TRUE;
				autoDstsParam->s.dsParam.transferSetName        = SD_TRUE;
				autoDstsParam->s.dsParam.transferSetTimeStamp   = SD_TRUE;
				autoDstsParam->s.dsParam.dsConditionsDetected   = SD_TRUE;
				autoDstsParam->s.dsParam.eventCodeDetected      = SD_TRUE;
				autoDstsParam->s.dstsData.Critical              = SD_FALSE;
				autoDstsParam->s.dstsData.StartTime             = 0;
				autoDstsParam->s.dstsData.Interval              = 0;
				autoDstsParam->s.dstsData.TLE                   = 0;
				autoDstsParam->s.dstsData.BufferTime            = 2;
				autoDstsParam->s.dstsData.IntegrityCheck        = 30;
				autoDstsParam->s.dstsData.DSConditionsRequested = MI_DSC_INTEGRITY | MI_DSC_CHANGE;
			}
			ret = u_icfg_ClientAutoDSTS (miIcfgCtrl, autoDstsParam);
		}
		else /* serviceRole == ICFG_ROLE_SERVER */
		{
			memset (clientObjCounts, 0, sizeof (ICFG_CLIENT_OBJ_COUNTS));
			ret = u_icfg_StartClientObjects (miIcfgCtrl, clientObjCounts);
		}

		ret = u_icfg_EndClientObjects (miIcfgCtrl);
		ret = u_icfg_EndRemoteControlCenter (miIcfgCtrl);
	}
	ret = u_icfg_EndRemoteControlCenters (miIcfgCtrl);
	ret = u_icfg_EndLocalControlCenter (miIcfgCtrl);

	return (ret);
}
#endif	// CHOIBC DELETE

#if 1	// 2016.04.11 ChoiBC Modify
/************************************************************************/
/*                      my_iccp_config                                  */
/************************************************************************/
/* pCfg는 iccp.cfg 등에서 읽은 정보가 update되어 있다. */
ST_RET my_iccp_config (ST_INT deviceIndex, ICCP_POINT_DEF *pPointDef, CFG_ICCP_FILE_ST *pCfg)
{
	ICFG_LOCAL_CC *icfgLocalCc;
	ICFG_REMOTE_CC *icfgRemoteCc; 
	ICFG_ASSOCIATION *icfgAssoc; 
	ICFG_SERVER_OBJ_COUNTS *serverObjCounts;
	ICFG_CLIENT_OBJ_COUNTS *clientObjCounts;
	MIC_GROBJ_CTRL *icfgGrobjCtrl;
	ST_INT numDv, numDev;
	ST_INT i;
	ST_RET ret;
	MI_ICFG_CTRL lc, *miIcfgCtrl = &lc;
	ICCP_POINT_INFO *pI;
	CFG_ICCP_SERVER_OBJ_ST *pSerObj = &pCfg->remote.serObj;
#ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
	ST_INT found_MpuSwitchDevName = 0;
#endif

	/* We mirror the MI log masks into the ICFG log mask */
	if (mi_debug_sel & MILOG_ERR)
		icfg_debug_sel |= ICFG_LOG_ERR;

	if (mi_debug_sel & MILOG_NERR)
		icfg_debug_sel |= ICFG_LOG_NERR;

	if (mi_debug_sel & MILOG_CFG)
		icfg_debug_sel |= ICFG_LOG_FLOW;

	lc.usr = &lc;
	lc.mode = MI_ICFG_MODE_INITIAL;
	lc.remote_name = NULL;

	/* Local Control Center */
	icfgLocalCc = (ICFG_LOCAL_CC *) chk_calloc (1, sizeof (ICFG_LOCAL_CC));
	//strncpy (icfgLocalCc->name, pCfg->local.name, sizeof(icfgLocalCc->name)-1);
	
	
	// 이 말이  db등록시  SDP_A는 FEP_A와만 통신해야하는 건 아니지..
	if (deviceIndex == DEVICE_INDEX_A)
		strncpy (icfgLocalCc->name, "SDP_A", sizeof(icfgLocalCc->name)-1);
	else
		strncpy (icfgLocalCc->name, "SDP_B", sizeof(icfgLocalCc->name)-1);
	
	
	
	
	icfgLocalCc->maxDs         = pCfg->local.maxDs;
	icfgLocalCc->maxDsTs       = pCfg->local.maxDsTs;
	icfgLocalCc->maxMmsPduSize = pCfg->local.maxMmsPduSize;
	icfgLocalCc->icfgRef.ref   = deviceIndex;

	ret = u_icfg_StartLocalControlCenter (miIcfgCtrl, icfgLocalCc);

	/* Local Objects */
	u_icfg_StartLocalObjects (miIcfgCtrl);

	numDv = numDev = 0;
#if 1	// 2016.11 ChoiBC MPU 절체 명령 추가..   MPU 절체 명령은  DB에 없나 ?
	numDev += SYSTEM_DEVICE_COUNT;  //1, DB상의 SDO외 추가로 하나더..
#endif

//pI ICCP_POINT_INFO  
	pI = &pPointDef->point[0];
	for (i=0; i<pPointDef->numPoint; i++, pI++) // pPointDef->numPoint 는 ICCP에 등록된 전체 point 수
	{
		if (!VALID_SDP_POINT_TYPE(pI->iccpPointType))
			continue;
		if (pI->iccpPointType == SDP_POINT_TYPE_SDO)
			numDev++;  // 제어는 Dev로..
		else
			numDv++;  // 일단 point는 Dv (data value로)
	}
	/* Local Data Values */
	ret = u_icfg_StartLocalDataValues (miIcfgCtrl, numDv);

	pI = &pPointDef->point[0];
	for (i=0; i<pPointDef->numPoint; i++, pI++)
	{
		ICFG_LOCAL_DV *icfgLdv;  // 포인트다..
		if (pI->iccpPointType == SDP_POINT_TYPE_SDO)
			continue; // 제어는 Dev로...
			
		// 메모리 할당헤서	
		icfgLdv = (ICFG_LOCAL_DV *) chk_calloc (1, sizeof (ICFG_LOCAL_DV));
		
		snprintf (icfgLdv->name, sizeof(icfgLdv->name)-1, "%s%0*d",
				pSerObj->point[pI->iccpPointType-1].name,
				pSerObj->point[pI->iccpPointType-1].numberWidth, // 이 것이 * 표
				pI->iccpPointIndex);
		if (DIGITAL_SDP_POINT_TYPE(pI->iccpPointType))
			icfgLdv->dvType   = StateSupplementalQTimeTagExtended; // 예/아니오, 또는 정해진 단계를 가진 데이터 
		else
			icfgLdv->dvType   = RealQTimetagExtended;  // 전압, 전류 등 수치 Data를 처리할 때
		icfgLdv->normalSource = Telemetered;
		icfgLdv->minPresent   = SD_FALSE;
		icfgLdv->maxPresent   = SD_FALSE;
		icfgLdv->icfgRef.ref  = i;  // 이 i는 전체 ICCP_POINT_INFO의 t순서. 아 것으로 rd_dv 할때 ref가 나오는 구나.
		strncpy (pI->iccpName, icfgLdv->name, sizeof(pI->iccpName)-1);
		// 이 함수에서 등록하나 보다.
		ret = u_icfg_LocalDataValue (miIcfgCtrl, icfgLdv);
	}
	u_icfg_EndLocalDataValues (miIcfgCtrl);

	/* Local Devices */
	/* 제어 포인터 등록 */
	ret = u_icfg_StartLocalDevices (miIcfgCtrl, numDev);

	pI = &pPointDef->point[0];
	for (i=0; i<pPointDef->numPoint; i++, pI++)
	{
		ICFG_LOCAL_DEV *icfgLdev; 
		if (pI->iccpPointType != SDP_POINT_TYPE_SDO)
			continue;
		icfgLdev = (ICFG_LOCAL_DEV *) chk_calloc (1, sizeof (ICFG_LOCAL_DEV));
		snprintf (icfgLdev->name, sizeof(icfgLdev->name)-1, "%s%0*d",
				pSerObj->point[pI->iccpPointType-1].name,
				pSerObj->point[pI->iccpPointType-1].numberWidth,
				pI->iccpPointIndex);
		icfgLdev->devType     = DevCommand;
		icfgLdev->sbo         = SD_TRUE; 
		icfgLdev->checkBackId = 0;
		icfgLdev->selTime     = ICCP_DEV_SELECT_TIMEOUT;  
		icfgLdev->tagEnable   = SD_FALSE;
		icfgLdev->icfgRef.ref = i;
		strncpy (pI->iccpName, icfgLdev->name, sizeof(pI->iccpName)-1);
		ret = u_icfg_LocalDevice (miIcfgCtrl, icfgLdev);
#ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
		//  getMpuSwitchDevName 는 config file에 이름이 정의되 있다
		if (strcmp (icfgLdev->name, getMpuSwitchDevName ()) == 0)
			found_MpuSwitchDevName = 1;
#endif

	}
#ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
	if (found_MpuSwitchDevName == 0) // 즉 위에서 DB에 없으면...
	{
		ICFG_LOCAL_DEV *icfgLdev; 
		icfgLdev = (ICFG_LOCAL_DEV *) chk_calloc (1, sizeof (ICFG_LOCAL_DEV));
		
		// ICCP/mi_main.c L372
		snprintf (icfgLdev->name, sizeof(icfgLdev->name)-1, "%s", getMpuSwitchDevName ());
		icfgLdev->devType     = DevCommand;
		icfgLdev->sbo         = SD_FALSE;
		icfgLdev->checkBackId = 0;
		icfgLdev->selTime     = 0;
		icfgLdev->tagEnable   = SD_FALSE;
		icfgLdev->icfgRef.ref = i;		// 조심...
		ret = u_icfg_LocalDevice (miIcfgCtrl, icfgLdev);
	}
	found_MpuSwitchDevName = 0; // 왜 다시 없다 하지.???
#else
#if 1	// 2016.11 ChoiBC MPU 절체 명령 추가
	{
		ICFG_LOCAL_DEV *icfgLdev; 
		icfgLdev = (ICFG_LOCAL_DEV *) chk_calloc (1, sizeof (ICFG_LOCAL_DEV));
		// ICCP/mi_usr.h   "MPU_SWTICH"
		snprintf (icfgLdev->name, sizeof(icfgLdev->name)-1, "%s", MPU_SWITCH_DEV_NAME); //"MPU_SWITCH"
		icfgLdev->devType     = DevCommand;
		icfgLdev->sbo         = SD_FALSE;
		icfgLdev->checkBackId = 0;
		icfgLdev->selTime     = 0;
		icfgLdev->tagEnable   = SD_FALSE;
		icfgLdev->icfgRef.ref = i;		// 조심...
		ret = u_icfg_LocalDevice (miIcfgCtrl, icfgLdev);
		
		// DB 상에서 포인트가 없으므로... strncpy (pI->iccpName, 가 없다.
        // mis_usr.c u_mis_device_operate() 에서  제어 명령이 내려 왔을 때 처리한다.

	}
#endif
#endif	// _VITZROSYS_FEP
	
	// 제어 point 등록 끝.
	ret = u_icfg_EndLocalDevices (miIcfgCtrl);

	/* Local Information Messages */
	ret = u_icfg_StartLocalInfoMsgs (miIcfgCtrl, 0);
	ret = u_icfg_EndLocalInfoMsgs (miIcfgCtrl);
	u_icfg_EndLocalObjects (miIcfgCtrl);

	/* Remote Control Centers */
	/* 상대방 ???  */
//	numCC = 1;
	ret = u_icfg_StartRemoteCtrlCenters (miIcfgCtrl, 1);
	icfgRemoteCc = (ICFG_REMOTE_CC *) chk_calloc (1, sizeof (ICFG_REMOTE_CC));
	strncpy (icfgRemoteCc->name, pCfg->remote.name, sizeof(icfgRemoteCc->name)-1);
	icfgRemoteCc->version = pCfg->remote.version;
	strncpy (icfgRemoteCc->blt.name, pCfg->remote.blt.name, sizeof(icfgRemoteCc->blt.name)-1);
	strncpy (icfgRemoteCc->blt.id, pCfg->remote.blt.id, sizeof(icfgRemoteCc->blt.id)-1);
	strncpy (icfgRemoteCc->blt.localDom, pCfg->remote.blt.localDom, sizeof(icfgRemoteCc->blt.localDom)-1);
	strncpy (icfgRemoteCc->blt.remoteDom, pCfg->remote.blt.remoteDom, sizeof(icfgRemoteCc->blt.remoteDom)-1);
	icfgRemoteCc->blt.shortestInterval = pCfg->remote.blt.shortestInterval;


// 밑에는 이름만 다른 거군..
#ifdef	VITZRO_FEP_ENABLE	
	icfgRemoteCc->blt.blocks = ICFG_BLOCK1 | ICFG_BLOCK2 | ICFG_BLOCK4 | ICFG_BLOCK5; 		// 비츠로시스 FEP 연동시
	printf("\n------------------------------------------\n");
	printf(">>> ICCP : VITZRO-FEP Enable ...\n");
	printf("------------------------------------------\n");
#else		
	icfgRemoteCc->blt.blocks = ICFG_BLOCK1 | ICFG_BLOCK2 | ICFG_BLOCK4 | ICFG_BLOCK5; 		// 에이스 FEP 연동시
	printf("\n------------------------------------------\n");
	printf(">>> ICCP : SDP-FEP Enable ...\n"); // ACE->SDP  2026-09-12 오후 2:41:50 
	printf("------------------------------------------\n");
#endif	
	
	icfgRemoteCc->icfgRef.ref = 0;
	ret =  u_icfg_StartRemoteControlCenter (miIcfgCtrl, icfgRemoteCc);

	/* Associations */
//	numAssoc = 1;
	ret = u_icfg_StartAssociations (miIcfgCtrl, 1);
	icfgAssoc = (ICFG_ASSOCIATION *) chk_calloc (1, sizeof (ICFG_ASSOCIATION));
	sprintf (icfgAssoc->name, "Association");
	if (deviceIndex == DEVICE_INDEX_A)
		strncpy (icfgAssoc->localAr, "SDP_A", sizeof(icfgAssoc->localAr)-1);
	else
		strncpy (icfgAssoc->localAr, "SDP_B", sizeof(icfgAssoc->localAr)-1);

	icfgAssoc->totalRemoteArNames = 2;
	sprintf (icfgAssoc->remoteAr[0], "FEP_A");
	sprintf (icfgAssoc->remoteAr[1], "FEP_B");
	icfgAssoc->serviceRole = ICFG_ROLE_SERVER;
	icfgAssoc->connectRole = ICFG_ROLE_CALLED;

	icfgAssoc->retryTime = 10;
	icfgAssoc->initiateTimeout = pCfg->remote.assoc.initiateTimeout;
	icfgAssoc->concludeTimeout = pCfg->remote.assoc.concludeTimeout;
	icfgAssoc->heartbeatTime   = pCfg->remote.assoc.heartbeatTime;
	icfgAssoc->maxMmsPduSize   = pCfg->remote.assoc.maxMmsPduSize;
	icfgAssoc->maxReqPend      = pCfg->remote.assoc.maxReqPend;
	icfgAssoc->maxIndPend      = pCfg->remote.assoc.maxIndPend;
	icfgAssoc->maxNest         = pCfg->remote.assoc.maxNest;
	icfgAssoc->icfgRef.ref = 0;
	ret = u_icfg_Association (miIcfgCtrl, icfgAssoc);
	ret = u_icfg_EndAssociations (miIcfgCtrl);

	/* Server Objects */
	serverObjCounts = (ICFG_SERVER_OBJ_COUNTS *) chk_calloc (1, sizeof (ICFG_SERVER_OBJ_COUNTS));
	/* Server Data Values */
	if (pSerObj->scope == VCC_SPEC)
	{
		serverObjCounts->numVccDv  = numDv;
		serverObjCounts->numVccDev = numDev;
	}
	else
	{
		serverObjCounts->numIccDv  = numDv;
		serverObjCounts->numIccDev = numDev;
	}
	u_icfg_StartServerObjects (miIcfgCtrl, serverObjCounts);

	/* Server Data Values */
	ret = u_icfg_StartServerDataValues (miIcfgCtrl, numDv);

	pI = &pPointDef->point[0];
	for (i=0; i<pPointDef->numPoint; i++, pI++)
	{
		ICFG_SERVER_DV *icfgSdv;
		if (pI->iccpPointType == SDP_POINT_TYPE_SDO)
			continue;
		icfgSdv = (ICFG_SERVER_DV *) chk_calloc (1, sizeof (ICFG_SERVER_DV));
		snprintf (icfgSdv->name, sizeof(icfgSdv->name)-1, "%s%0*d",
				pSerObj->point[pI->iccpPointType-1].name,
				pSerObj->point[pI->iccpPointType-1].numberWidth,
				pI->iccpPointIndex);
		if (DIGITAL_SDP_POINT_TYPE(pI->iccpPointType))
			icfgSdv->dvType   = StateSupplementalQTimeTagExtended;
		else
			icfgSdv->dvType   = RealQTimetagExtended;
		icfgSdv->scope        = (pSerObj->scope == VCC_SPEC) ? VCC : ICC;
		icfgSdv->readOnly     = SD_TRUE;
		icfgSdv->icfgRef.ref  = i;
		
	//	printf("icfgSdv->name[%s]\r\n", icfgSdv->name) ;
		
		ret = u_icfg_ServerDataValue (miIcfgCtrl, icfgSdv);
	}
	ret = u_icfg_EndServerDataValues (miIcfgCtrl);

	/* Server Devices */
	ret = u_icfg_StartServerDevices (miIcfgCtrl, numDev);

	pI = &pPointDef->point[0];
	for (i=0; i<pPointDef->numPoint; i++, pI++)
	{
		ICFG_SERVER_DEV *icfgSdev;
		if (pI->iccpPointType != SDP_POINT_TYPE_SDO)
			continue;
		icfgSdev = (ICFG_SERVER_DEV *) chk_calloc (1, sizeof (ICFG_SERVER_DEV));
		snprintf (icfgSdev->name, sizeof(icfgSdev->name)-1, "%s%0*d",
				pSerObj->point[pI->iccpPointType-1].name,
				pSerObj->point[pI->iccpPointType-1].numberWidth,
				pI->iccpPointIndex);
		icfgSdev->devType     = DevCommand;
		
		// VCC로 정해져 있다
		icfgSdev->scope       = (pSerObj->scope == VCC_SPEC) ? VCC : ICC;
		icfgSdev->tagEnable   = SD_FALSE; 
		icfgSdev->icfgRef.ref = i;
		ret = u_icfg_ServerDevice (miIcfgCtrl, icfgSdev);
#ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
		if (strcmp (icfgSdev->name, getMpuSwitchDevName ()) == 0)
			found_MpuSwitchDevName = 1;
#endif
	}
#ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
	if (found_MpuSwitchDevName == 0)
	{
		ICFG_SERVER_DEV *icfgSdev;
		icfgSdev = (ICFG_SERVER_DEV *) chk_calloc (1, sizeof (ICFG_SERVER_DEV));
		snprintf (icfgSdev->name, sizeof(icfgSdev->name)-1, "%s", MPU_SWITCH_DEV_NAME);
		icfgSdev->devType     = DevCommand;
		icfgSdev->scope       = (pSerObj->scope == VCC_SPEC) ? VCC : ICC;
		icfgSdev->tagEnable   = SD_FALSE; 
		icfgSdev->icfgRef.ref = i;		// 조심...
		ret = u_icfg_ServerDevice (miIcfgCtrl, icfgSdev);
	}
#else
#if 1	// 2016.11 ChoiBC MPU 절체 명령 추가
	{
		ICFG_SERVER_DEV *icfgSdev;
		icfgSdev = (ICFG_SERVER_DEV *) chk_calloc (1, sizeof (ICFG_SERVER_DEV));
		snprintf (icfgSdev->name, sizeof(icfgSdev->name)-1, "%s", MPU_SWITCH_DEV_NAME);
		icfgSdev->devType     = DevCommand;
		icfgSdev->scope       = (pSerObj->scope == VCC_SPEC) ? VCC : ICC;
		icfgSdev->tagEnable   = SD_FALSE; 
		icfgSdev->icfgRef.ref = i;		// 조심...
		ret = u_icfg_ServerDevice (miIcfgCtrl, icfgSdev);
	}
#endif
#endif	// _VITZROSYS_FEP
	ret = u_icfg_EndServerDevices (miIcfgCtrl);

	/* Server Information Messages */
	ret = u_icfg_StartServerInfoMsgs (miIcfgCtrl, 0);
	ret = u_icfg_EndServerInfoMsgs (miIcfgCtrl);

	/* Server DataSets */
	ret = u_icfg_StartServerDataSets (miIcfgCtrl, 0);
	ret = u_icfg_EndServerDataSets (miIcfgCtrl);
	ret = u_icfg_EndServerObjects (miIcfgCtrl);

	/* Client Objects */
	clientObjCounts = (ICFG_CLIENT_OBJ_COUNTS *) chk_calloc (1, sizeof (ICFG_CLIENT_OBJ_COUNTS));
	ret = u_icfg_StartClientObjects (miIcfgCtrl, clientObjCounts);

	/* Client Data Values */
	u_icfg_StartClientDataValues (miIcfgCtrl, 0);
	ret = u_icfg_EndClientDataValues (miIcfgCtrl);
	/* Client Devices */
	ret = u_icfg_StartClientDevices (miIcfgCtrl, 0);
	ret = u_icfg_EndClientDevices (miIcfgCtrl);

	/* Client Information Messages */
	ret = u_icfg_StartClientInfoMsgs (miIcfgCtrl, 0);
	ret = u_icfg_EndClientInfoMsgs (miIcfgCtrl);

	/* Client Data Sets */
	ret = u_icfg_StartClientDataSets (miIcfgCtrl, 0);
	ret = u_icfg_EndClientDataSets (miIcfgCtrl);

	/* Client Data Set Transfer Sets */
	ret = u_icfg_StartClientDSTSs (miIcfgCtrl, 0);
	ret = u_icfg_EndClientDSTSs (miIcfgCtrl);

	/* Discovery */
	icfgGrobjCtrl = (MIC_GROBJ_CTRL *) chk_calloc (1, sizeof (MIC_GROBJ_CTRL));
	sprintf (icfgGrobjCtrl->assocName, "Association");
	ret = u_icfg_Discovery (miIcfgCtrl, icfgGrobjCtrl);

	ret = u_icfg_EndClientObjects (miIcfgCtrl);

	ret = u_icfg_EndRemoteControlCenter (miIcfgCtrl);

	ret = u_icfg_EndRemoteControlCenters (miIcfgCtrl);

	ret = u_icfg_EndLocalControlCenter (miIcfgCtrl);

	return (ret);

#if 0	// CHOIBC DELETE
{
	ICFG_LOCAL_CC *icfgLocalCc;
	ICFG_LOCAL_DV *icfgLdv;
	ICFG_LOCAL_DEV *icfgLdev; 
	ICFG_REMOTE_CC *icfgRemoteCc; 
	ICFG_ASSOCIATION *icfgAssoc; 
	ICFG_SERVER_OBJ_COUNTS *serverObjCounts;
	ICFG_SERVER_DV *icfgSdv;
	ICFG_SERVER_DEV *icfgSdev;
	ICFG_CLIENT_OBJ_COUNTS *clientObjCounts;
	ST_INT numDv, numVccDv, numIccDv;
	ST_INT numDev, numVccDev, numIccDev;
	ST_INT i;
	ST_RET ret;
	MI_ICFG_CTRL lc, *miIcfgCtrl = &lc;
	ICCP_POINT_INFO *pPoint;

	/* We mirror the MI log masks into the ICFG log mask */
	if (mi_debug_sel & MILOG_ERR)
		icfg_debug_sel |= ICFG_LOG_ERR;

	if (mi_debug_sel & MILOG_NERR)
		icfg_debug_sel |= ICFG_LOG_NERR;

	if (mi_debug_sel & MILOG_CFG)
		icfg_debug_sel |= ICFG_LOG_FLOW;

	lc.usr = &lc;
	lc.mode = MI_ICFG_MODE_INITIAL;
	lc.remote_name = NULL;

	/* Local Control Center */
	icfgLocalCc = (ICFG_LOCAL_CC *) chk_calloc (1, sizeof (ICFG_LOCAL_CC));
	strncpy (icfgLocalCc->name, pCfg->local.name, sizeof(icfgLocalCc->name)-1);
	icfgLocalCc->maxDs = pCfg->local.maxDs;
	icfgLocalCc->maxDsTs = pCfg->local.maxDsTs;
	icfgLocalCc->maxMmsPduSize = pCfg->local.maxMmsPduSize;
	icfgLocalCc->icfgRef.ref = 0;

	ret = u_icfg_StartLocalControlCenter (miIcfgCtrl, icfgLocalCc);

	/* Local Objects */
	u_icfg_StartLocalObjects (miIcfgCtrl);

	/* Local Data Values */
	numDv = 0;
	pPoint = pointCFG;
	for (i=0; i<MAX_ICCP_POINT; i++, pPoint++)
	{
		if (pPoint->config == 0 || pPoint->iccpName[0] == 0)
			continue;
		if (pPoint->type != DI_POINT && pPoint->type != AI_POINT)
			continue;
		numDv++;
	}
	ret = u_icfg_StartLocalDataValues (miIcfgCtrl, numDv);
	pPoint = pointCFG;
	numVccDv = numIccDv = 0;
	for (i=0; i<MAX_ICCP_POINT; i++, pPoint++)
	{
		if (pPoint->config == 0 || pPoint->iccpName[0] == 0)
			continue;
		if (pPoint->type != DI_POINT && pPoint->type != AI_POINT)
			continue;
		icfgLdv = (ICFG_LOCAL_DV *) chk_calloc (1, sizeof (ICFG_LOCAL_DV));
		sprintf (icfgLdv->name, "%s", pPoint->iccpName);
		if (pPoint->iccpScope)
			numIccDv++;
		else
			numVccDv++;
		if (pPoint->iccpType == ICCP_DV_TYPE_STATE)
			icfgLdv->dvType   = StateSupplementalQTimeTagExtended;
		else if (pPoint->iccpType == ICCP_DV_TYPE_DISCRETE)
			icfgLdv->dvType   = DiscreteQ;
		else
			icfgLdv->dvType   = RealQTimetagExtended;
		icfgLdv->normalSource = Telemetered;
		icfgLdv->minPresent   = SD_FALSE;
		icfgLdv->maxPresent   = SD_FALSE;
		icfgLdv->icfgRef.ref  = i;
		ret = u_icfg_LocalDataValue (miIcfgCtrl, icfgLdv);
	}
	u_icfg_EndLocalDataValues (miIcfgCtrl);

	/* Local Devices */
	numDev = 0;
	pPoint = pointCFG;
	for (i=0; i<MAX_ICCP_POINT; i++, pPoint++)
	{
		if (pPoint->config == 0 || pPoint->iccpName[0] == 0)
			continue;
		if (pPoint->type != DO_POINT && pPoint->type != AO_POINT)
			continue;
		numDev++;
	}
	ret = u_icfg_StartLocalDevices (miIcfgCtrl, numDev);
	pPoint = pointCFG;
	numVccDev = numIccDev = 0;
	for (i=0; i<MAX_ICCP_POINT; i++, pPoint++)
	{
		if (pPoint->config == 0 || pPoint->iccpName[0] == 0)
			continue;
		if (pPoint->type != DO_POINT && pPoint->type != AO_POINT)
			continue;
		icfgLdev = (ICFG_LOCAL_DEV *) chk_calloc (1, sizeof (ICFG_LOCAL_DEV));
		sprintf (icfgLdev->name, "%s", pPoint->iccpName);
		if (pPoint->iccpScope)
			numIccDev++;
		else
			numVccDev++;
		if (pPoint->iccpType == ICCP_DEV_TYPE_COMMAND)
			icfgLdev->devType = DevCommand;
		else if (pPoint->iccpType == ICCP_DEV_TYPE_DISCRETE)
			icfgLdev->devType = DevDiscrete;
		else
			icfgLdev->devType = DevReal;
		icfgLdev->sbo = SD_TRUE; 
		icfgLdev->checkBackId = 0;
		icfgLdev->selTime = ICCP_DEV_SELECT_TIMEOUT;  
		icfgLdev->tagEnable = SD_FALSE;
		icfgLdev->icfgRef.ref = i;
		ret = u_icfg_LocalDevice (miIcfgCtrl, icfgLdev);
	}
	ret = u_icfg_EndLocalDevices (miIcfgCtrl);

	/* Local Information Messages */
	ret = u_icfg_StartLocalInfoMsgs (miIcfgCtrl, 0);
	ret = u_icfg_EndLocalInfoMsgs (miIcfgCtrl);
	u_icfg_EndLocalObjects (miIcfgCtrl);

	/* Remote Control Centers */
//	numCC = 1;
	ret = u_icfg_StartRemoteCtrlCenters (miIcfgCtrl, 1);
	icfgRemoteCc = (ICFG_REMOTE_CC *) chk_calloc (1, sizeof (ICFG_REMOTE_CC));
	strncpy (icfgRemoteCc->name, pCfg->remote.name, sizeof(icfgRemoteCc->name)-1);
	icfgRemoteCc->version = pCfg->remote.version;
	strncpy (icfgRemoteCc->blt.name, pCfg->remote.blt.name, sizeof(icfgRemoteCc->blt.name)-1);
	strncpy (icfgRemoteCc->blt.id, pCfg->remote.blt.id, sizeof(icfgRemoteCc->blt.id)-1);
	strncpy (icfgRemoteCc->blt.localDom, pCfg->remote.blt.localDom, sizeof(icfgRemoteCc->blt.localDom)-1);
	strncpy (icfgRemoteCc->blt.remoteDom, pCfg->remote.blt.remoteDom, sizeof(icfgRemoteCc->blt.remoteDom)-1);
	icfgRemoteCc->blt.shortestInterval = pCfg->remote.blt.shortestInterval;
	icfgRemoteCc->blt.blocks = ICFG_BLOCK1 | ICFG_BLOCK2 | ICFG_BLOCK5; 
	icfgRemoteCc->icfgRef.ref = 0;
	ret =  u_icfg_StartRemoteControlCenter (miIcfgCtrl, icfgRemoteCc);

	/* Associations */
//	numAssoc = 1;
	ret = u_icfg_StartAssociations (miIcfgCtrl, 1);
	icfgAssoc = (ICFG_ASSOCIATION *) chk_calloc (1, sizeof (ICFG_ASSOCIATION));
	sprintf (icfgAssoc->name, "Association");
	sprintf (icfgAssoc->localAr, "SDP_A");

	icfgAssoc->totalRemoteArNames = 2;
	sprintf (icfgAssoc->remoteAr[0], "FEP_A");
	sprintf (icfgAssoc->remoteAr[1], "FEP_B");
	icfgAssoc->serviceRole = ICFG_ROLE_SERVER;
	icfgAssoc->connectRole = ICFG_ROLE_CALLED;

	icfgAssoc->retryTime = 10;
	icfgAssoc->initiateTimeout = pCfg->remote.assoc.initiateTimeout;
	icfgAssoc->concludeTimeout = pCfg->remote.assoc.concludeTimeout;
	icfgAssoc->heartbeatTime   = pCfg->remote.assoc.heartbeatTime;
	icfgAssoc->maxMmsPduSize   = pCfg->remote.assoc.maxMmsPduSize;
	icfgAssoc->maxReqPend      = pCfg->remote.assoc.maxReqPend;
	icfgAssoc->maxIndPend      = pCfg->remote.assoc.maxIndPend;
	icfgAssoc->maxNest         = pCfg->remote.assoc.maxNest;
	icfgAssoc->icfgRef.ref = 0;
	ret = u_icfg_Association (miIcfgCtrl, icfgAssoc);
	ret = u_icfg_EndAssociations (miIcfgCtrl);

	/* Server Objects */
	serverObjCounts = (ICFG_SERVER_OBJ_COUNTS *) chk_calloc (1, sizeof (ICFG_SERVER_OBJ_COUNTS));

	/* Server Data Values */
	serverObjCounts->numVccDv         = numVccDv;
	serverObjCounts->numVccDev        = numVccDev;
	serverObjCounts->numVccInfoMsg    = 0;
	serverObjCounts->numVccDs         = 0;
	serverObjCounts->numIccDv         = numIccDv;
	serverObjCounts->numIccDev        = numIccDev;
	serverObjCounts->numIccInfoMsg    = 0;
	serverObjCounts->numIccDs         = 0;
	u_icfg_StartServerObjects (miIcfgCtrl, serverObjCounts);

	ret = u_icfg_StartServerDataValues (miIcfgCtrl, numDv);
	pPoint = pointCFG;
	for (i=0; i<MAX_ICCP_POINT; i++, pPoint++)
	{
		if (pPoint->config == 0 || pPoint->iccpName[0] == 0)
			continue;
		if (pPoint->type != DI_POINT && pPoint->type != AI_POINT)
			continue;
		icfgSdv = (ICFG_SERVER_DV *) chk_calloc (1, sizeof (ICFG_SERVER_DV));
		sprintf (icfgSdv->name, "%s", pPoint->iccpName);
		icfgSdv->scope = pPoint->iccpScope ? ICC : VCC;
		if (pPoint->iccpType == ICCP_DV_TYPE_STATE)
			icfgSdv->dvType   = StateSupplementalQTimeTagExtended;
		else if (pPoint->iccpType == ICCP_DV_TYPE_DISCRETE)
			icfgSdv->dvType   = DiscreteQ;
		else
			icfgSdv->dvType   = RealQTimetagExtended;
		icfgSdv->readOnly     = SD_TRUE;
		icfgSdv->icfgRef.ref  = i;
		ret = u_icfg_ServerDataValue (miIcfgCtrl, icfgSdv);
	}
	ret = u_icfg_EndServerDataValues (miIcfgCtrl);

	/* Server Devices */
	/* 제어등록하는것 같은데... */
	ret = u_icfg_StartServerDevices (miIcfgCtrl, numDev);
	pPoint = pointCFG;
	for (i=0; i<MAX_ICCP_POINT; i++, pPoint++)
	{
		if (pPoint->config == 0 || pPoint->iccpName[0] == 0)
			continue;
		if (pPoint->type != DO_POINT && pPoint->type != AO_POINT) // 제어가 아니면
			continue;
		icfgSdev = (ICFG_SERVER_DEV *) chk_calloc (1, sizeof (ICFG_SERVER_DEV));
		sprintf (icfgSdev->name, "%s", pPoint->iccpName);
		icfgSdev->scope = pPoint->iccpScope ? ICC : VCC;
		if (pPoint->iccpType == ICCP_DEV_TYPE_COMMAND)
			icfgSdev->devType = DevCommand;
		else if (pPoint->iccpType == ICCP_DEV_TYPE_DISCRETE)
			icfgSdev->devType = DevDiscrete;
		else
			icfgSdev->devType = DevReal;
		icfgSdev->tagEnable = SD_FALSE; 
		icfgSdev->icfgRef.ref = i;
		ret = u_icfg_ServerDevice (miIcfgCtrl, icfgSdev);
	}
	ret = u_icfg_EndServerDevices (miIcfgCtrl);

	/* Server Information Messages */
	ret = u_icfg_StartServerInfoMsgs (miIcfgCtrl, 0);
	ret = u_icfg_EndServerInfoMsgs (miIcfgCtrl);

	/* Server DataSets */
	ret = u_icfg_StartServerDataSets (miIcfgCtrl, 0);
	ret = u_icfg_EndServerDataSets (miIcfgCtrl);
	ret = u_icfg_EndServerObjects (miIcfgCtrl);

	/* Client Objects */
	clientObjCounts = (ICFG_CLIENT_OBJ_COUNTS *) chk_calloc (1, sizeof (ICFG_CLIENT_OBJ_COUNTS));
	memset (clientObjCounts, 0, sizeof (ICFG_CLIENT_OBJ_COUNTS));
	ret = u_icfg_StartClientObjects (miIcfgCtrl, clientObjCounts);

	ret = u_icfg_EndClientObjects (miIcfgCtrl);
	ret = u_icfg_EndRemoteControlCenter (miIcfgCtrl);

	ret = u_icfg_EndRemoteControlCenters (miIcfgCtrl);
	ret = u_icfg_EndLocalControlCenter (miIcfgCtrl);

	return (ret);
}
#endif
}

/*
 *	my_iccp_config_file_load
 *		initial(load) iccp configuration informaion from iccp.conf file
 *
 *	RETURN : result status
 *		SD_FAILURE : failure or error
 *		SD_SUCCESS : success
 */
ST_VOID my_iccp_config_file_load (CFG_ICCP_FILE_ST *pCfg)
{
	int no, val;
	char line[200], path[256], str[200];
	char *fname = "iccp.conf";
	FILE *fp;
	
	memset( pCfg, 0, sizeof(*pCfg) );
	/* set default */
	strcpy (pCfg->local.name, "Local");
	pCfg->local.maxDsTs = ICFGU_MAX_DSTS;
	pCfg->local.maxDs = ICFGU_MAX_DS;
	pCfg->local.maxMmsPduSize = ICFGU_MAX_PDU_SIZE;

	strcpy (pCfg->remote.name, "Remote");
	pCfg->remote.version = MI_VERSION_1996;
	strcpy (pCfg->remote.blt.name, "BLT_NAME");
	strcpy (pCfg->remote.blt.id, "BLT_ID");
	strcpy (pCfg->remote.blt.localDom, "BLT_LDOM");
	strcpy (pCfg->remote.blt.remoteDom, "BLT_RDOM");
	pCfg->remote.blt.shortestInterval = ICFGU_DEFAULT_SHORTEST_INTERVAL;

	pCfg->remote.assoc.initiateTimeout = ICFGU_DEFAULT_INITIATE_TIMEOUT;
	pCfg->remote.assoc.concludeTimeout = ICFGU_DEFAULT_CONCLUDE_TIMEOUT;
	pCfg->remote.assoc.heartbeatTime   = ICFGU_DEFAULT_HEARTBIT_TIME;
	pCfg->remote.assoc.maxMmsPduSize   = ICFGU_MAX_PDU_SIZE;
	pCfg->remote.assoc.maxReqPend = ICFGU_DEFAULT_MAX_REQ_PEND;
	pCfg->remote.assoc.maxIndPend = ICFGU_DEFAULT_MAX_IND_PEND;
	pCfg->remote.assoc.maxNest    = ICFGU_DEFAULT_MAX_NEST;
#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
	pCfg->remote.assoc.identifyTimeout = ICFGU_DEFAULT_MAX_IDENTIFY_TIMEOUT;
#endif

	pCfg->remote.serObj.scope = VCC_SPEC;
	strcpy (pCfg->remote.serObj.point[INDEX_SDI].name, "SDI");
	pCfg->remote.serObj.point[INDEX_SDI].numberWidth = ICFGU_DEFAULT_POINT_NUMBER_WIDTH;
	strcpy (pCfg->remote.serObj.point[INDEX_SAI].name, "SAI");
	pCfg->remote.serObj.point[INDEX_SAI].numberWidth = ICFGU_DEFAULT_POINT_NUMBER_WIDTH;
	strcpy (pCfg->remote.serObj.point[INDEX_SDO].name, "SDO");
	pCfg->remote.serObj.point[INDEX_SDO].numberWidth = ICFGU_DEFAULT_POINT_NUMBER_WIDTH;
	strcpy (pCfg->remote.serObj.point[INDEX_DDI].name, "DDI");
	pCfg->remote.serObj.point[INDEX_DDI].numberWidth = ICFGU_DEFAULT_POINT_NUMBER_WIDTH;
	strcpy (pCfg->remote.serObj.point[INDEX_DAI].name, "DAI");
	pCfg->remote.serObj.point[INDEX_DAI].numberWidth = ICFGU_DEFAULT_POINT_NUMBER_WIDTH;
	strcpy (pCfg->remote.serObj.point[INDEX_QDI].name, "QDI");
	pCfg->remote.serObj.point[INDEX_QDI].numberWidth = ICFGU_DEFAULT_POINT_NUMBER_WIDTH;
	strcpy (pCfg->remote.serObj.point[INDEX_QAI].name, "QAI");
	pCfg->remote.serObj.point[INDEX_QAI].numberWidth = ICFGU_DEFAULT_POINT_NUMBER_WIDTH;
	strcpy (pCfg->remote.serObj.point[INDEX_TDI].name, "TDI");
	pCfg->remote.serObj.point[INDEX_TDI].numberWidth = ICFGU_DEFAULT_POINT_NUMBER_WIDTH;
	strcpy (pCfg->remote.serObj.point[INDEX_TAI].name, "TAI");
	pCfg->remote.serObj.point[INDEX_TAI].numberWidth = ICFGU_DEFAULT_POINT_NUMBER_WIDTH;
	strcpy (pCfg->remote.serObj.point[INDEX_DEV].name, "DEV");
	pCfg->remote.serObj.point[INDEX_DEV].numberWidth = ICFGU_DEFAULT_POINT_NUMBER_WIDTH;

	// read datas from file
	snprintf (path, sizeof(path), "%s/%s", CONFIG_DIR, fname);
	if ((fp = fopen (path, "r")) == NULL)
	{
		printf ("%s : open (%s) error <%d:%s>\n",
				fname, path, errno, strerror(errno) );
		return;
	}
	/* read data from file */
	for (no=1; ; no++ )
	{
		if (fgets (line, sizeof(line), fp ) == NULL )
			break;
		if( line[0] == '#' || line[0] == 0x0A)
			continue;
		if (strncmp (line, "LocalCC_", strlen("LocalCC_")) == 0)
		{
			if (sscanf (line, "LocalCC_Name = %s\n", str) == 1)
			{
				strncpy (pCfg->local.name, str, sizeof(pCfg->local.name)-1);
				continue;
			}
			if (sscanf (line, "LocalCC_MaxDsTs = %d\n", &val) == 1)
			{
				if (val < 0)
				{
					printf ("%s : LocalCC_MaxDsTs = %d error\n", fname, val);
					val = ICFGU_MAX_DSTS;
				}
				pCfg->local.maxDsTs = val;
				continue;
			}
			if (sscanf (line, "LocalCC_MaxDataSets = %d\n", &val) == 1)
			{
				if (val < 0)
				{
					printf ("%s : LocalCC_MaxDataSets = %d error\n", fname, val);
					val = ICFGU_MAX_DS;
				}
				pCfg->local.maxDs = val;
				continue;
			}
			if (sscanf (line, "LocalCC_MaxMmsMsgSize = %d\n", &val) == 1)
			{
				if (val < 0)
				{
					printf ("%s : LocalCC_MaxMmsMsgSize = %d error\n", fname, val);
					val = ICFGU_MAX_PDU_SIZE;
				}
				pCfg->local.maxMmsPduSize = val;
				continue;
			}
		}
		else if (sscanf (line, "RemoteCC_Name = %s\n", str) == 1)
		{
			strncpy (pCfg->remote.name, str, sizeof(pCfg->remote.name)-1);
			continue;
		}
		else if (sscanf (line, "RemoteCC_Version = %s\n", str) == 1)
		{
			if (!strcmp (str, "1996-08"))
				pCfg->remote.version = MI_VERSION_1996;
			else if (!strcmp (str, "2000-08"))
				pCfg->remote.version = MI_VERSION_2000;
			else
			{
				printf ("%s : RemoteCC_Version = %s error\n", fname, str);
				pCfg->remote.version = MI_VERSION_1996;
			}
			continue;
		}
		else if (strncmp (line, "RemoteCC_BilateralTable_", strlen("RemoteCC_BilateralTable_")) == 0)
		{
			if (sscanf (line, "RemoteCC_BilateralTable_Name = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.blt.name, str, sizeof(pCfg->remote.blt.name)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_BilateralTable_Id = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.blt.id, str, sizeof(pCfg->remote.blt.id)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_BilateralTable_LocalDomain = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.blt.localDom, str, sizeof(pCfg->remote.blt.localDom)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_BilateralTable_RemoteDomain = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.blt.remoteDom, str, sizeof(pCfg->remote.blt.remoteDom)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_BilateralTable_ShortestInterval = %d\n", &val) == 1)
			{
				if (val < 0)
				{
					printf ("%s : RemoteCC_BilateralTable_ShortestInterval = %d error\n", fname, val);
					val = ICFGU_DEFAULT_SHORTEST_INTERVAL;
				}
				pCfg->remote.blt.shortestInterval = val;
				continue;
			}
		}
		else if (strncmp (line, "RemoteCC_Association_", strlen("RemoteCC_Association_")) == 0)
		{
			if (sscanf (line, "RemoteCC_Association_InitiateTimeout = %d\n", &val) == 1)
			{
				if (val < 0)
				{
					printf ("%s : RemoteCC_Association_InitiateTimeout = %d error\n", fname, val);
					val = ICFGU_DEFAULT_INITIATE_TIMEOUT;
				}
				pCfg->remote.assoc.initiateTimeout = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_Association_ConcludeTimeout = %d\n", &val) == 1)
			{
				if (val < 0)
				{
					printf ("%s : RemoteCC_Association_ConcludeTimeout = %d error\n", fname, val);
					val = ICFGU_DEFAULT_CONCLUDE_TIMEOUT;
				}
				pCfg->remote.assoc.concludeTimeout = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_Association_AssocHeartbeatTime = %d\n", &val) == 1)
			{
				if (val < 0)
				{
					printf ("%s : RemoteCC_Association_AssocHeartbeatTime = %d error\n", fname, val);
					val = ICFGU_DEFAULT_HEARTBIT_TIME;
				}
				pCfg->remote.assoc.heartbeatTime = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_Association_MaxMmsMsgSize = %d\n", &val) == 1)
			{
				if (val < 0)
				{
					printf ("%s : RemoteCC_Association_MaxMmsMsgSize = %d error\n", fname, val);
					val = ICFGU_MAX_PDU_SIZE;
				}
				pCfg->remote.assoc.maxMmsPduSize = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_Association_MaxReqPend = %d\n", &val) == 1)
			{
				if (val < 0)
				{
					printf ("%s : RemoteCC_Association_MaxReqPend = %d error\n", fname, val);
					val = ICFGU_DEFAULT_MAX_REQ_PEND;
				}
				pCfg->remote.assoc.maxReqPend = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_Association_MaxIndPend = %d\n", &val) == 1)
			{
				if (val < 0)
				{
					printf ("%s : RemoteCC_Association_MaxIndPend = %d error\n", fname, val);
					val = ICFGU_DEFAULT_MAX_IND_PEND;
				}
				pCfg->remote.assoc.maxIndPend = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_Association_MaxNest = %d\n", &val) == 1)
			{
				if (val <= 0)
				{
					printf ("%s : RemoteCC_Association_MaxNest = %d error\n", fname, val);
					val = ICFGU_DEFAULT_MAX_NEST;
				}
				pCfg->remote.assoc.maxNest = val;
				continue;
			}
#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
			if (sscanf (line, "RemoteCC_Association_IdentifyTimeout = %d\n", &val) == 1)
			{
				if (val < 0)
					printf ("%s : RemoteCC_Association_IdentifyTimeout = %d error\n", fname, val);
				else
					pCfg->remote.assoc.identifyTimeout = val;
				continue;
			}
#endif
		}
		else if (strncmp (line, "RemoteCC_ServerObjects_", strlen("RemoteCC_ServerObjects_")) == 0)
		{
			if (sscanf (line, "RemoteCC_ServerObjects_Scope = %s\n", str) == 1)
			{
				if (!strcmp (str, "VCC"))
					pCfg->remote.serObj.scope = VCC_SPEC;
				else if (!strcmp (str, "ICC"))
					pCfg->remote.serObj.scope = ICC_SPEC;
				else
					printf ("%s : RemoteCC_ServerObjects_Scope = %s error\n", fname, str);
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Sdi.Name = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.serObj.point[INDEX_SDI].name, str, sizeof(pCfg->remote.serObj.point[INDEX_SDI].name)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Sdi.NumberWidth = %d\n", &val) == 1)
			{
				if (val <= 1)
					printf ("%s : RemoteCC_ServerObjects_Sdi.NumberWidth = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_SDI].numberWidth = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Sdi.Count = %d\n", &val) == 1)
			{
				if (val < 0)
					printf ("%s : RemoteCC_ServerObjects_Sdi.Count = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_SDI].count = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Sdo.Name = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.serObj.point[INDEX_SDO].name, str, sizeof(pCfg->remote.serObj.point[INDEX_SDO].name)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Sdo.NumberWidth = %d\n", &val) == 1)
			{
				if (val <= 1)
					printf ("%s : RemoteCC_ServerObjects_Sdo.NumberWidth = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_SDO].numberWidth = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Sdo.Count = %d\n", &val) == 1)
			{
				if (val < 0)
					printf ("%s : RemoteCC_ServerObjects_Sdo.Count = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_SDO].count = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Sai.Name = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.serObj.point[INDEX_SAI].name, str, sizeof(pCfg->remote.serObj.point[INDEX_SAI].name)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Sai.NumberWidth = %d\n", &val) == 1)
			{
				if (val <= 1)
					printf ("%s : RemoteCC_ServerObjects_Sai.NumberWidth = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_SAI].numberWidth = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Sai.Count = %d\n", &val) == 1)
			{
				if (val < 0)
					printf ("%s : RemoteCC_ServerObjects_Sai.Count = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_SAI].count = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Ddi.Name = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.serObj.point[INDEX_DDI].name, str, sizeof(pCfg->remote.serObj.point[INDEX_DDI].name)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Ddi.NumberWidth = %d\n", &val) == 1)
			{
				if (val <= 1)
					printf ("%s : RemoteCC_ServerObjects_Ddi.NumberWidth = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_DDI].numberWidth = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Ddi.Count = %d\n", &val) == 1)
			{
				if (val < 0)
					printf ("%s : RemoteCC_ServerObjects_Ddi.Count = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_DDI].count = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Dai.Name = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.serObj.point[INDEX_DAI].name, str, sizeof(pCfg->remote.serObj.point[INDEX_DAI].name)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Dai.NumberWidth = %d\n", &val) == 1)
			{
				if (val <= 1)
					printf ("%s : RemoteCC_ServerObjects_Dai.NumberWidth = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_DAI].numberWidth = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Dai.Count = %d\n", &val) == 1)
			{
				if (val < 0)
					printf ("%s : RemoteCC_ServerObjects_Dai.Count = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_DAI].count = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Qdi.Name = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.serObj.point[INDEX_QDI].name, str, sizeof(pCfg->remote.serObj.point[INDEX_QDI].name)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Qdi.NumberWidth = %d\n", &val) == 1)
			{
				if (val <= 1)
					printf ("%s : RemoteCC_ServerObjects_Qdi.NumberWidth = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_QDI].numberWidth = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Qdi.Count = %d\n", &val) == 1)
			{
				if (val < 0)
					printf ("%s : RemoteCC_ServerObjects_Qdi.Count = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_QDI].count = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Qai.Name = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.serObj.point[INDEX_QAI].name, str, sizeof(pCfg->remote.serObj.point[INDEX_QAI].name)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Qai.NumberWidth = %d\n", &val) == 1)
			{
				if (val <= 1)
					printf ("%s : RemoteCC_ServerObjects_Qai.NumberWidth = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_QAI].numberWidth = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Qai.Count = %d\n", &val) == 1)
			{
				if (val < 0)
					printf ("%s : RemoteCC_ServerObjects_Qai.Count = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_QAI].count = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Tdi.Name = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.serObj.point[INDEX_TDI].name, str, sizeof(pCfg->remote.serObj.point[INDEX_TDI].name)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Tdi.NumberWidth = %d\n", &val) == 1)
			{
				if (val <= 1)
					printf ("%s : RemoteCC_ServerObjects_Tdi.NumberWidth = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_TDI].numberWidth = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Tdi.Count = %d\n", &val) == 1)
			{
				if (val < 0)
					printf ("%s : RemoteCC_ServerObjects_Tdi.Count = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_TDI].count = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Tai.Name = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.serObj.point[INDEX_TAI].name, str, sizeof(pCfg->remote.serObj.point[INDEX_TAI].name)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Tai.NumberWidth = %d\n", &val) == 1)
			{
				if (val <= 1)
					printf ("%s : RemoteCC_ServerObjects_Tai.NumberWidth = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_TAI].numberWidth = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Tai.Count = %d\n", &val) == 1)
			{
				if (val < 0)
					printf ("%s : RemoteCC_ServerObjects_Tai.Count = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_TAI].count = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Dev.Name = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.serObj.point[INDEX_DEV].name, str, sizeof(pCfg->remote.serObj.point[INDEX_DEV].name)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Dev.NumberWidth = %d\n", &val) == 1)
			{
				if (val <= 1)
					printf ("%s : RemoteCC_ServerObjects_Dev.NumberWidth = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_DEV].numberWidth = val;
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_Dev.Count = %d\n", &val) == 1)
			{
				if (val < 0)
					printf ("%s : RemoteCC_ServerObjects_Dev.Count = %d error\n", fname, val);
				else
					pCfg->remote.serObj.point[INDEX_DEV].count = val;
				continue;
			}
#ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
			if (sscanf (line, "RemoteCC_ServerObjects_MpuSwitchDevName = %s\n", str) == 1)
			{
				strncpy (pCfg->remote.serObj.MpuSwitchDevName, str, sizeof(pCfg->remote.serObj.MpuSwitchDevName)-1);
				continue;
			}
			if (sscanf (line, "RemoteCC_ServerObjects_ChangeOfAnalogPeriod = %d\n", &val) == 1)
			{
				if (val < 0 || val > 60)
					printf ("%s : RemoteCC_ServerObjects_ChangeOfAnalogPeriod = %d error(0~60)\n", fname, val);
				else
					pCfg->remote.serObj.ChangeOfAnalogPeriod = val;
				continue;
			}
#endif
		}
		printf ("%s : invalid format of %s\n  line #%d %s", fname, path, no, line );
	}
	fclose( fp );
	return;
}

#endif


