/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*      	1998 - 2002, All Rights Reserved		        */
/*									*/
/* MODULE NAME : mics_cfg.c  						*/
/* PRODUCT(S)  : ICCP TASE.2 Extensions for MMS-EASE Lite		*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 07/27/10  NAV           Klocwork Changes				*/
/* 02/11/10  NAV     23    Add StateSupplemental types			*/
/* 05/17/07  LWP     22	   Added IM functionality			*/
/* 11/02/06  NAV     21    Add Block Data DSTS Parm                     */
/* 06/23/06  MDE     20    VS2005 porting changes			*/
/* 12/14/05  MDE     19    LINUX warning cleanup			*/
/* 08/30/05  MDE     18    Added MICS_DB option				*/
/* 07/26/05  MDE     17    2000-08 work					*/
/* 06/28/05  MDE     16    Moved MI_ICFG_USR_CTRL to mics_cfg.h		*/
/* 06/06/05  MDE     15    LINUX warning cleanup			*/
/* 05/26/05  MDE     14    Added support for user developed ICFG	*/
/* 03/31/04  MDE     13    Removed Remote DV validation			*/
/* 03/31/04  MDE     12    Added AUTO DSTS support			*/
/* 03/31/04  MDE     11    Added Discovery support			*/
/* 03/31/04  MDE     10    Load speed enhancentns			*/
/* 05/07/03  MDE     09    Set assocName in MICU_DSTS			*/
/* 06/20/02  RKR     08    Copyright year update                        */
/* 06/20/02  MDE     07    Removed ICFG naming				*/
/* 06/20/02  DGE     06    Added micuFindDv.				*/
/* 04/26/02  MDE     05    Moved DSCondReq helper code to icfgxml.c	*/
/* 04/24/02  MDE     04    Use new DSConditions for DSTS		*/
/* 04/17/02  MDE     03	   Added remote DV config options  		*/
/* 01/21/02  MDE     02    Now add special variables to client DS	*/
/* 11/01/01  MDE     01    Created					*/
/************************************************************************/

#include "glbtypes.h"
#include "sysincs.h"
#include "mvl_acse.h"
#include "mi.h"
#include "mics_cfg.h"
#include "mi_icfg.h"
#include "mi_usr.h"

/************************************************************************/
#ifdef DEBUG_SISCO
static ST_CHAR *thisFileName = __FILE__;
#endif

/************************************************************************/

MIU_REMOTE *miu_remote_list;
MISU_DV    *misu_dv_list;
MISU_DEV   *misu_dev_list;
MISU_IM	   *misu_im_list;

/************************************************************************/
/************************************************************************/

#if 0	// CHOIBC DELETE
ST_RET miuIcfg (ST_INT serviceRole, ST_INT mode, ST_CHAR *remoteName, ST_VOID *usr);

/************************************************************************/
/************************************************************************/
/*			miu_icfg_load					*/
/************************************************************************/

ST_RET miu_icfg_load (ST_INT source, ST_INT mode, ST_CHAR *remoteName)
{
	MI_ICFG_USR_CTRL lc;
	ST_RET rc = SD_FAILURE;			/* Klocwork Recommended */

	if (source == MIU_CFG_SOURCE_XML)
		rc = mi_icfg_load ("mics_usr.xml", mode, remoteName, &lc);
#ifdef MICS_DB
	else if (source == MIU_CFG_SOURCE_DB)
		rc = mi_icfg_db_load (miuLccName, MI_ICFG_MODE_INITIAL, NULL, &lc);
#endif
	else
	{
		if (source == MIU_CFG_SOURCE_USR_CLIENT)
			rc = miuIcfg (ICFG_ROLE_CLIENT, mode, remoteName, &lc);
		if (source == MIU_CFG_SOURCE_USR_SERVER)
			rc = miuIcfg (ICFG_ROLE_SERVER, mode, remoteName, &lc);
	}

	if (rc != SD_SUCCESS)
	{
		printf ("\nICCP configuration using ICFG failed");
		exit (2);
	}
	return (rc);
}
#endif	// CHOIBC DELETE

/************************************************************************/
/************************************************************************/
/*			miuFindRemote 					*/
/************************************************************************/

MIU_REMOTE *miuFindRemote (ST_CHAR *remoteName)
{
	MIU_REMOTE *miuRemote;

	miuRemote = miu_remote_list;
	while (miuRemote != NULL)
	{
		if (!strcmp (miuRemote->name, remoteName))
			break;

		miuRemote = list_get_next (miu_remote_list, miuRemote);
	}
	return (miuRemote);
}

/************************************************************************/
/*			miuFindMiRemote 				*/
/************************************************************************/

MIU_REMOTE *miuFindMiRemote (MI_REMOTE *mi_remote)
{
	MIU_REMOTE *miuRemote;

	miuRemote = miu_remote_list;
	while (miuRemote != NULL)
	{
		if (miuRemote->mi_remote == mi_remote)
			break;

		miuRemote = list_get_next (miu_remote_list, miuRemote);
	}
	return (miuRemote);
}

/************************************************************************/
/*			micuFindDv					*/
/************************************************************************/

MICU_DV *micuFindDv (MIU_REMOTE *miuRemote, 
	ST_CHAR *dvName, ST_INT scope)
{
	MICU_DV *dv_list;

	dv_list = miuRemote->dv_list;
	while (dv_list != NULL)
	{
		if (!strcmp (dv_list->mic_dv->dv_name, dvName))
			break;

		dv_list = list_get_next (miuRemote->dv_list, dv_list);
	}
	return (dv_list);
}

/************************************************************************/
/*			micuFindDs 					*/
/************************************************************************/

MICU_DS *micuFindDs (MIU_REMOTE *miuRemote, ST_CHAR *dsName)
{
	MICU_DS *ds_list;

	ds_list = miuRemote->ds_list;
	while (ds_list != NULL)
	{
		if (!strcmp (ds_list->ds_name, dsName))
			break;

		ds_list = list_get_next (miuRemote->ds_list, ds_list);
	}
	return (ds_list);
}

/************************************************************************/
/*			micuFindDsTs 					*/
/************************************************************************/

MICU_DSTS *micuFindDsTs (MIU_REMOTE *miuRemote, ST_CHAR *dsTsName)
{
	MICU_DSTS *dsts;

	dsts = miuRemote->dsts_list;
	while (dsts != NULL)
	{
		if (!strcmp (dsts->dsTsName, dsTsName))
			break;

		dsts = list_get_next (miuRemote->dsts_list, dsts);
	}
	return (dsts);
}

/************************************************************************/
/************************************************************************/
/* ICFG Specific Code: Setting up mics_cfg data structures		*/
/************************************************************************/
/************************************************************************/
/************************************************************************/
/* These are callback functions from the MiIcfg subsystem, and are 	*/
/* called during configuration load time. We can perform any desired	*/
/* processing here, and in our case we will asociate the network	*/
/* visible data values with our local storage.				*/
/************************************************************************/
/* 		u_miIcfgProcLocalControlCenter 				*/
/************************************************************************/

ST_RET u_miIcfgProcLocalControlCenter (ST_VOID *usr, ICFG_LOCAL_CC *icfgLocalCc)
{
	/* Note: icfgLocalCc must not be free'd in this function, and will be	*/
	/* free'd (by mi_icfg) when configuration is complete 			*/

	return (SD_SUCCESS);
}

/************************************************************************/
/* 		u_miIcfgProcRemoteControlCenter 			*/
/************************************************************************/

ST_RET u_miIcfgProcRemoteControlCenter (ST_VOID *usr, MI_REMOTE *mi_remote, 
	ICFG_REMOTE_CC *icfgRemoteCc)
{
	MIU_REMOTE *miuRemote;
	MI_ICFG_USR_CTRL *lc;

	lc = (MI_ICFG_USR_CTRL *) usr;

	miuRemote = chk_calloc (1, sizeof (MIU_REMOTE));
	list_add_last ((ST_VOID **)&miu_remote_list, miuRemote);
	lc->miuCurrRemote = miuRemote;

	strcpy (miuRemote->name, icfgRemoteCc->name);
	miuRemote->mi_remote = mi_remote;

	/* Note: icfgRemoteCc must not be free'd in this function, and will be	*/
	/* free'd (by mi_icfg) when configuration is complete 			*/
	return (SD_SUCCESS);
}

/************************************************************************/
/* 		u_miIcfgProcAssociation 				*/
/************************************************************************/

ST_RET u_miIcfgProcAssociation (ST_VOID *usr, MI_ASSOC_CTRL *mi_assoc, 
	ICFG_ASSOCIATION *icfgAssoc)
{
	mi_icfg_free (icfgAssoc);		/* We have no need for this information */
	return (SD_SUCCESS);
}  

/************************************************************************/
/*			u_miIcfgProcLocalDv 				*/
/************************************************************************/

ST_RET u_miIcfgProcLocalDv (ST_VOID *usr, MIS_DV_REF misDvRef, 
	ICFG_LOCAL_DV *icfgLocalDv, ST_BOOLEAN newDv)
{
	MIS_DV  *mis_dv;
	MISU_DV *miuDv;
	static ST_INT dvCount = 0;

	miuDv = chk_calloc (1, sizeof (MISU_DV));
	list_add_last((ST_VOID **) &misu_dv_list, (ST_VOID *) miuDv);
	miuDv->dvRef = misDvRef;
	miuDv->mi_type = icfgLocalDv->dvType;
	mis_dv = MIS_DV_REF_TO_DV (misDvRef);
	mis_dv->access_info = miuDv;
	miuDv->icfgRef = icfgLocalDv->icfgRef;

	switch (miuDv->mi_type)
	{
	case MI_TYPEID_REAL:
	case MI_TYPEID_REAL_Q:
	case MI_TYPEID_REAL_Q_TIMETAG:
	case MI_TYPEID_REAL_EXTENDED:
	case MI_TYPEID_REAL_Q_TIMETAG_EXTENDED:
		miuDv->data.Value.r = ((ST_FLOAT) 1.01) * (ST_FLOAT) dvCount;
		break;

	case MI_TYPEID_DISCRETE:
	case MI_TYPEID_DISCRETE_Q:
	case MI_TYPEID_DISCRETE_Q_TIMETAG:
	case MI_TYPEID_DISCRETE_Q_TIMETAG_EXTENDED:
	case MI_TYPEID_DISCRETE_EXTENDED:
		miuDv->data.Value.d = dvCount;
		break;

	case MI_TYPEID_STATE:
	case MI_TYPEID_STATE_Q:
	case MI_TYPEID_STATE_Q_TIMETAG:
	case MI_TYPEID_STATE_EXTENDED:
	case MI_TYPEID_STATE_Q_TIMETAG_EXTENDED:
	case MI_TYPEID_STATE_SUPP:
	case MI_TYPEID_STATE_SUPP_Q:
	case MI_TYPEID_STATE_SUPP_Q_TIMETAG:
	case MI_TYPEID_STATE_SUPP_EXTENDED:
	case MI_TYPEID_STATE_SUPP_Q_TIMETAG_EXTENDED:
		if (dvCount % 2)
		{
			MI_SET_DATA_STATE (miuDv->data.Flags,MI_DATA_STATE_ON);
		}
		else
		{
			MI_SET_DATA_STATE (miuDv->data.Flags,MI_DATA_STATE_OFF);
		}
		break;
	}

	/* Set up the various quality flags */      
	MI_QSET_VALIDITY    (miuDv->data.Flags, MI_QFLAG_VALIDITY_VALID);
	MI_QSET_CURR_SOURCE (miuDv->data.Flags, MI_QFLAG_CURR_SOURCE_TELEMETERED);
	MI_QSET_NORMAL_VAL  (miuDv->data.Flags, MI_QFLAG_NORMAL_VAL_NORMAL);

	/* Set the timestamp values */
	miuDv->data.TimeStamp.GMTBasedS    = (ST_INT32) time (NULL);
	miuDv->data.TimeStamp.Milliseconds = (ST_INT16) (dvCount + 1);

	++dvCount;

	mi_icfg_free (icfgLocalDv);
	return (SD_SUCCESS);
}  

/************************************************************************/
/*			u_miIcfgProcLocalDev				*/
/************************************************************************/

ST_RET u_miIcfgProcLocalDev (ST_VOID *usr, MIS_DEVICE_REF misDevRef, 
	ICFG_LOCAL_DEV *icfgLocalDev,  ST_BOOLEAN newDev)
{
	MISU_DEV *miuDev;
	MIS_DEVICE *mis_dev;

	miuDev = chk_calloc (1, sizeof (MISU_DEV));
	list_add_last((ST_VOID **) &misu_dev_list, (ST_VOID *) miuDev);
	miuDev->devRef = misDevRef;
	miuDev->dev_type = icfgLocalDev->devType;
	miuDev->selTime = icfgLocalDev->selTime;  
	miuDev->icfgRef = icfgLocalDev->icfgRef;

	mis_dev = MIS_DEV_REF_TO_DEV (misDevRef);
	mis_dev->access_info = miuDev;

	if (mis_dev->taggable)
	{
		mis_dev->tag_value.TagFlags = MI_TAG_FLAG_NO_TAG;
		sprintf (mis_dev->tag_value.Reason, "%s: All Clear", mis_dev->device_name);
	}

	mi_icfg_free (icfgLocalDev);	/* We have no need for this information */
	return (SD_SUCCESS);
}

/************************************************************************/
/*			u_miIcfgProcLocalIm 				*/
/************************************************************************/

ST_RET u_miIcfgProcLocalIm (ST_VOID *usr, ICFG_LOCAL_INFOMSG *icfgLocalIm)  
{
	MISU_IM *miuIM;

	miuIM = chk_calloc (1, sizeof (MISU_IM));

	list_add_last((ST_VOID **) &misu_im_list, (ST_VOID *) miuIM);
	miuIM->InfoReference = icfgLocalIm->infoRef;
	strcpy (miuIM->mapInfo,icfgLocalIm->mapInfo);
	miuIM->maxSize = icfgLocalIm->maxSize;

	mi_icfg_free (icfgLocalIm);	/* We have no need for this information */
	return (SD_SUCCESS);
}

/************************************************************************/
/*			u_miIcfgProcServerDv  				*/
/************************************************************************/

ST_RET u_miIcfgProcServerDv  (ST_VOID *usr, MIS_VCC *mis_vcc, 
	MIS_DV_REF dv_ref, ICFG_SERVER_DV *icfgServerDv)
{
	mi_icfg_free (icfgServerDv);	/* We have no need for this information */
	return (SD_SUCCESS);
}

/************************************************************************/
/*			u_miIcfgProcServerDev 				*/
/************************************************************************/

ST_RET u_miIcfgProcServerDev (ST_VOID *usr, MIS_VCC *mis_vcc, 
	MIS_DEVICE_REF dev_ref, ICFG_SERVER_DEV *icfgServerDev)
{
	mi_icfg_free (icfgServerDev);	/* We have no need for this information */
	return (SD_SUCCESS);
}

/************************************************************************/
/*			u_miIcfgProcServerIM  				*/
/************************************************************************/

ST_RET u_miIcfgProcServerIM  (ST_VOID *usr, MIS_VCC *mis_vcc, 
	ICFG_SERVER_INFOMSG *icfgServerIm)
{
	MISU_IM_ASSOC *im_assoc;
	MI_ASSOC_CTRL *miassoc;
	MISU_IM *pim;


	pim = misu_im_list;
	while(pim != NULL)
	{
		if (icfgServerIm->infoRef == pim->InfoReference)
		{
			break;
		}
		pim =list_get_next(misu_im_list,pim);
	}
	miassoc = mis_vcc->mi_remote->mi_assoc_list;
	while((miassoc != NULL)&&(pim != NULL))
	{
		pim->numAssoc++;
		im_assoc = chk_calloc (1, sizeof (MISU_IM_ASSOC));
		list_add_last((ST_VOID **) &pim->hol_mi_assoc, (ST_VOID *) im_assoc);
		im_assoc->mi_assoc = miassoc;
		im_assoc->scope = icfgServerIm->scope;
		miassoc = list_get_next(mis_vcc->mi_remote->mi_assoc_list,miassoc);

	}
	mi_icfg_free (icfgServerIm);	/* We have no need for this information */
	return (SD_SUCCESS);
}

/************************************************************************/
/*			u_miIcfgProcServerDs  				*/
/************************************************************************/

ST_RET u_miIcfgProcServerDs (ST_VOID *usr, MIS_VCC *mis_vcc, 
	MIS_DS *mis_ds, ICFG_SERVER_DS *icfgServerDs)
{
	mi_icfg_free (icfgServerDs);	/* We have no need for this information */
	return (SD_SUCCESS);
}

/************************************************************************/
/*			u_miIcfgProcClientDv 				*/
/************************************************************************/

ST_RET u_miIcfgProcClientDv (ST_VOID *usr, MIC_VCC *mic_vcc, MIC_DV *micDv, 
	ICFG_CLIENT_DV *icfgClientDv)
{
	MI_ICFG_USR_CTRL *lc;
	MICU_DV *micuDv;

	lc = (MI_ICFG_USR_CTRL *) usr;
	micuDv = chk_calloc (1, sizeof (MICU_DV));
	list_add_last((ST_VOID **) &lc->miuCurrRemote->dv_list, (ST_VOID *) micuDv);
	micuDv->mic_dv = micDv;
	micuDv->mi_type = icfgClientDv->dvType;
	micDv->handle_info = micuDv;
	micuDv->icfgRef = icfgClientDv->icfgRef;

	mi_icfg_free (icfgClientDv); /* We have no need for this information */
	return (SD_SUCCESS);
}

/************************************************************************/
/*			u_miIcfgProcClientDev 				*/
/************************************************************************/

ST_RET u_miIcfgProcClientDev (ST_VOID *usr, MIC_VCC *mic_vcc, 
	MIC_DEVICE *micDev, ICFG_CLIENT_DEV *icfgClientDev)
{
	MI_ICFG_USR_CTRL *lc;
	MICU_DEV *micuDev;

	lc = (MI_ICFG_USR_CTRL *) usr;

	micuDev = chk_calloc (1, sizeof (MICU_DEV));
	list_add_last((ST_VOID **) &lc->miuCurrRemote->dev_list, (ST_VOID *) micuDev);
	micuDev->mic_dev = micDev;
	micuDev->dev_type = icfgClientDev->devType;
	micuDev->sbo = icfgClientDev->sbo;
	micuDev->icfgRef = icfgClientDev->icfgRef;
	micDev->handle_info = micuDev;

	mi_icfg_free (icfgClientDev); /* We have no need for this information */
	return (SD_SUCCESS);
}  

/************************************************************************/
/*			u_miIcfgProcClientIM  				*/
/************************************************************************/

ST_RET u_miIcfgProcClientIM  (ST_VOID *usr, MIC_VCC *mic_vcc, 
	ICFG_CLIENT_INFOMSG *icfgClientIm)
{
	mi_icfg_free (icfgClientIm);	/* We have no need for this information */
	return (SD_SUCCESS);
}

/************************************************************************/
/*			u_miIcfgProcClientDs  				*/
/************************************************************************/

ST_RET u_miIcfgProcClientDs  (ST_VOID *usr, MIC_VCC *mic_vcc, 
	ICFG_CLIENT_DS *icfgDs)
{
	MI_ICFG_USR_CTRL *lc;
	ICFG_CLIENT_DS_DV *icfg_ds_dv;
	MICU_DS     *miu_ds;
	MIC_DV      *mic_dv;
	MIC_DEVICE  *mic_dev;
	ST_INT	     i;
	ST_INT	     dvIndex;

	lc = (MI_ICFG_USR_CTRL *) usr;

	/* Allow room for the special variables */
	miu_ds = chk_calloc (1, sizeof (MICU_DS) + 
		((icfgDs->numDv + 4) * sizeof (MIC_DSTS_VAR)));

	list_add_last ((ST_VOID **) &lc->miuCurrRemote->ds_list, miu_ds);
	strcpy (miu_ds->ds_name, icfgDs->name);
	miu_ds->ds_scope = ICC_SPEC;
	miu_ds->num_var = icfgDs->numDv;
	miu_ds->var_tbl = (MIC_DSTS_VAR *)(miu_ds + 1);
	miu_ds->icfgRef = icfgDs->icfgRef;

	/* Add the special variables, if selected */
	dvIndex = 0;
	if (icfgDs->transferSetName == SD_TRUE)
	{
		mic_dv = mic_find_dv (mic_vcc, "Transfer_Set_Name", ICC_SPEC); 
		if (mic_dv != NULL)
		{
			miu_ds->var_tbl[dvIndex].dv = SD_TRUE;
			miu_ds->var_tbl[dvIndex].v.mic_dv = mic_dv;
			++dvIndex;
			++miu_ds->num_var;
		}
		else
			return (SD_FAILURE);
	}

	if (icfgDs->transferSetTimeStamp == SD_TRUE)
	{
		mic_dv = mic_find_dv (mic_vcc, "Transfer_Set_Time_Stamp", ICC_SPEC); 
		if (mic_dv != NULL)
		{
			miu_ds->var_tbl[dvIndex].dv = SD_TRUE;
			miu_ds->var_tbl[dvIndex].v.mic_dv = mic_dv;
			++dvIndex;
			++miu_ds->num_var;
		}
		else
			return (SD_FAILURE);
	}

	if (icfgDs->dsConditionsDetected == SD_TRUE)
	{
		mic_dv = mic_find_dv (mic_vcc, "DSConditions_Detected", ICC_SPEC); 
		if (mic_dv != NULL)
		{
			miu_ds->var_tbl[dvIndex].dv = SD_TRUE;
			miu_ds->var_tbl[dvIndex].v.mic_dv = mic_dv;
			++dvIndex;
			++miu_ds->num_var;
		}
		else
			return (SD_FAILURE);
	}

	if (icfgDs->eventCodeDetected == SD_TRUE)
	{
		mic_dv = mic_find_dv (mic_vcc, "Event_Code_Detected", ICC_SPEC); 
		if (mic_dv != NULL)
		{
			miu_ds->var_tbl[dvIndex].dv = SD_TRUE;
			miu_ds->var_tbl[dvIndex].v.mic_dv = mic_dv;
			++dvIndex;
			++miu_ds->num_var;
		}
		else
			return (SD_FAILURE);
	}

	for (i = 0; i < icfgDs->numDv; ++i, ++dvIndex)
	{
		icfg_ds_dv = &icfgDs->dvArray[i];
		if (icfg_ds_dv->tag == SD_FALSE)
		{
			mic_dv = mic_find_dv (mic_vcc, icfg_ds_dv->name, icfg_ds_dv->scope); 
			if (mic_dv != NULL)
			{
				miu_ds->var_tbl[dvIndex].dv = SD_TRUE;
				miu_ds->var_tbl[dvIndex].v.mic_dv = mic_dv;
			} 
			else
				return (SD_FAILURE);
		}
		else
		{
			mic_dev = mic_find_device (mic_vcc, icfg_ds_dv->name, icfg_ds_dv->scope); 
			if (mic_dev != NULL)
			{
				miu_ds->var_tbl[dvIndex].dv = SD_FALSE;
				miu_ds->var_tbl[dvIndex].v.mic_device = mic_dev;
			} 
			else
				return (SD_FAILURE);
		}
	}

	mi_icfg_free (icfgDs);	/* We have no need for this information */
	return (SD_SUCCESS);
}

/************************************************************************/
/*			u_miIcfgProcClientDsTs				*/
/************************************************************************/

ST_RET u_miIcfgProcClientDsTs (ST_VOID *usr, MIC_VCC *mic_vcc, 
	ICFG_CLIENT_DSTS *icfgDsTs)
{
	MI_ICFG_USR_CTRL *lc;
	MICU_DSTS  *miu_dsts;
	MICU_DS    *miu_ds;

	lc = (MI_ICFG_USR_CTRL *) usr;

	miu_dsts = (MICU_DSTS *) chk_calloc (1, sizeof (MICU_DSTS));
	list_add_last ((ST_VOID **)&lc->miuCurrRemote->dsts_list, miu_dsts);

	strcpy (miu_dsts->dsTsName, icfgDsTs->dsName);
	strcpy (miu_dsts->assocName, icfgDsTs->assocName);
	miu_ds = micuFindDs (lc->miuCurrRemote, icfgDsTs->dsName);
	if (miu_ds == NULL)
	{
		return (SD_FAILURE);
	}
	miu_dsts->miu_ds = miu_ds;
	miu_dsts->icfgRef = icfgDsTs->icfgRef;

	miu_dsts->dsts_data.StartTime = (ST_INT32) icfgDsTs->startTime;
	miu_dsts->dsts_data.Interval = (ST_INT16)icfgDsTs->interval;
	miu_dsts->dsts_data.TLE = (ST_INT16)icfgDsTs->tle;
	miu_dsts->dsts_data.BufferTime = (ST_INT16)icfgDsTs->bufferTime;
	miu_dsts->dsts_data.IntegrityCheck = (ST_INT16)icfgDsTs->integrity;
	miu_dsts->dsts_data.Critical = icfgDsTs->critical;
	miu_dsts->dsts_data.RBE = icfgDsTs->rbe;
	miu_dsts->dsts_data.AllChangesReported = icfgDsTs->AllChangesReported;
	miu_dsts->dsts_data.BlockData = icfgDsTs->BlockData;

	miu_dsts->dsts_data.DSConditionsRequested = 
		icfgDsTs->dsConditionsRequested;

	miu_dsts->dsts_data.Status = SD_TRUE;
	miu_dsts->dsts_data.EventCodeRequested = 0;

	mi_icfg_free (icfgDsTs);	/* We have no need for this information */
	return (SD_SUCCESS);
}


/************************************************************************/
/*			u_miIcfgProcDiscovery				*/
/************************************************************************/

ST_RET u_miIcfgProcDiscovery (ST_VOID *usr, MI_ASSOC_CTRL *mi_assoc, 
	MIC_GROBJ_CTRL *grobj_ctrl)
{
	/* mi_assoc->mic_grobj_ctrl has already been updated ... */
	return (SD_SUCCESS);
}				  


/************************************************************************/
/*			u_miIcfgProcClientAutoDsTs			*/
/************************************************************************/

ST_RET u_miIcfgProcClientAutoDsTs (ST_VOID *usr, MI_ASSOC_CTRL *mi_assoc, 
	MIC_AUTO_DSTS_PARAM *autoDstsParam)
{  
	/* mi_assoc->autoDstsParam has already been updated ... */
	return (SD_SUCCESS);
}

