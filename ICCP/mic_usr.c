/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*      	1998 - 2004, All Rights Reserved		        */
/*									*/
/* MODULE NAME : mic_usr.c    						*/
/* PRODUCT(S)  : ICCP TASE.2 Extensions for MMS-EASE Lite		*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 08/19/10  RKR     54    Moved check for NULL handle_info             */
/* 04/28/10  RKR     53    Moved call to miuIrMicDvUpdate in            */
/*                         u_mic_dv_ind_ex                              */
/* 03/19/10  NAV     52    Add DeleteAllDataSets option			*/
/* 02/10/10  NAV     51    Add StateSupplemental types			*/
/* 08/28/09  NAV     50    compatability mode & CurveType changes	*/
/* 08/26/09  NAV     49    Add (long) casts for Linux			*/
/* 08/05/09  NAV     48    Add Block 8 Support				*/
/* 08/04/09  MDE     47    Added micuGetAllDvAttrib			*/
/* 04/14/09  TJS     46    Fixed a QPRINTF in u_mic_dv_ind_ex           */
/* 01/14/09  MDE     45    Print more complete error for read		*/
/* 11/04/08  RKR     44    Added SQA_REGRESSION hook for bugzilla issues*/
/* 12/12/06  RKR     43    Added some braces around an if statement     */
/* 12/05/06  GLB     42    Added "won't work" for Read and Write        */
/* 10/18/06  MDE     41    Fix for redundancy (disabled) & discovery	*/
/* 07/28/06  MDE     40    VS2005 porting work				*/
/* 04/03/06  RKR     39    Modified a "printf" format                   */
/* 03/30/06  MDE     38    Update redundancy                            */
/* 03/29/06  RKR     37    Worked on BLOCK4, Read Write DV, & Devices   */
/* 03/28/06  RKR     36    Worked on _micuReadDs                        */
/* 12/14/05  MDE     35    LINUX warning cleanup			*/
/* 08/25/05  MDE     34    Added mic_ds to u_mic_dv_ind_start_ex	*/
/* 07/26/05  MDE     33    2000-08 work					*/
/* 07/22/05  RKR     32    Fixed printf in _micuStopIMTransfers         */
/* 06/28/05  MDE     31    Fix for discover new DV			*/
/* 06/06/05  MDE     30    LINUX warning cleanup			*/
/* 04/29/05  MDE     29    MIU_IR naming update				*/
/* 04/25/05  MDE     28    General cleanup 				*/
/* 04/06/05  MDE     27    Added quiet mode				*/
/* 02/10/05  MDE     26    Now use _ex DV functions			*/
/* 01/21/05  MDE     25    Tolerate missing ICCP Lite Plus		*/
/* 12/13/04  MDE     24    Changed StartTime from time_t to ST_INT32	*/
/* 11/05/04  MDE     23    Fixed DSTS stop (was startind ...)		*/
/* 08/09/04  EJV     22    Typecasted param to ctime().			*/
/* 05/10/04  MDE     21    Added 'corrected' handling			*/
/* 03/31/04  MDE     20    Updated Discovery support code		*/
/* 03/31/04  MDE     19	   Added AUTO DSTS support			*/
/* 01/20/04  MDE     18	   Added redundancy support code		*/
/* 01/20/04  MDE     17    Startup sequence correct for multiple assoc	*/
/* 12/03/03  MDE     16    Linux warning cleanup			*/
/* 05/07/03  MDE     15    Fixed up startup sequence (DSTS assocName)	*/
/* 01/08/03  MDE     14    Check for timestamp invalid before using	*/
/* 09/13/02  ASK     13    Added micuSendIdentify			*/
/* 06/20/02  RKR     12    Copyright year update                        */
/* 06/20/02  MDE     11	   Use cleaned up mics_icfg names		*/
/* 04/16/02  MDE     10	   Corrected handling of StartTime		*/
/* 04/16/02  MDE     09	   Changed startup to use mic_get_rem_dv, etc..	*/
/* 03/25/02  MDE     08	   Added tolerance of mis-configured DS		*/
/* 03/21/02  MDE     07	   Remote AR info moved to MI_ASSOC (printed) 	*/
/* 01/25/02  MDE     06	   Added redundancy support code		*/
/* 01/22/02  MDE     05	   minor data value display changes		*/
/* 10/29/01  MDE     04	   Added USE_ICFG code				*/
/* 10/11/99  NAV     03    Add block 4 and 5 support			*/
/* 04/01/99  MDE     02    Use MI_ASSOC_CTRL for connection management	*/
/* 12/31/98  MDE     01    Created					*/
/************************************************************************/

#include "glbtypes.h"
#include "sysincs.h"
#include "mi_usr.h"
#include "mi_icfg.h"
#include "mics_cfg.h"

/************************************************************************/
/* For debug version, use a static pointer to avoid duplication of 	*/
/* __FILE__ strings.							*/
/************************************************************************/

#ifdef DEBUG_SISCO
#if 0	// CHOIBC DELETE
static ST_CHAR *thisFileName = __FILE__;
#endif
#endif

/************************************************************************/
/************************************************************************/
/* SAMPLE MI CLIENT CODE 						*/
/************************************************************************/
/************************************************************************/
/* Static Local functions */

#if 0	// CHOIBC DELETE
static ST_VOID _micuIdentifyDone (MI_REQ_CTRL *miReq);
static ST_VOID _micuDiscover (MI_CONN *mi_conn);
static ST_VOID _micuDiscoverDone (MI_REQ_CTRL *miReq);

static ST_VOID _micuStartAutoDsts (MI_CONN *mi_conn);
static ST_VOID _micuStartAutoDstsDone (MI_REQ_CTRL *miReq);
static ST_VOID _micuCreateCfgDs (MI_CONN *mi_conn);
      
static ST_VOID _micuCreateDs (MI_CONN *mi_conn, MICU_DSTS *miuDsTs);
static ST_VOID _micuCreateDsDone (MI_REQ_CTRL *req);

static ST_VOID _micuStartAllDsTs (MI_CONN *mi_conn);
static ST_VOID _micuStartDsTs (MI_CONN *mi_conn, MICU_DSTS  *miuDsTs);
static ST_VOID _micuStartDsTsDone (MI_REQ_CTRL *req);

static ST_VOID _micuStartIMTransfers (MI_CONN *mi_conn);
static ST_VOID _micuStartIMTransferDone (MI_REQ_CTRL *req);

static ST_VOID _micuStopAutoDstsDone (MI_REQ_CTRL *miReq);
static ST_VOID _micuStopAllDsTs (MI_CONN *mi_conn);
static ST_VOID _micuStopDsTs (MI_CONN *mi_conn, MICU_DSTS  *miuDsTs);
static ST_VOID _micuStopDsTsDone (MI_REQ_CTRL *req);

static ST_VOID _micuStopIMTransfers (MI_CONN *mi_conn);
static ST_VOID _micuStopIMTransfersDone (MI_REQ_CTRL *req);

static ST_VOID _micuReadDv (MI_CONN *mi_conn, MICU_DV *micIcfgDv);
static ST_VOID _micuReadDvDone (MI_REQ_CTRL *req);

static ST_VOID _micuReadDs (MI_CONN *mi_conn, MICU_DS *micIcfgDs);
static ST_VOID _micuReadDsDone (MI_REQ_CTRL *req);
static ST_VOID _micuCreateDsForRdDone (MI_REQ_CTRL *miReq);

static ST_VOID _micuDvWriteDone (MI_REQ_CTRL *req);

/* Device Operations */
static ST_VOID _micuDeviceTest (MI_CONN *mi_conn, MICU_DEV *micIcfgDev);
static ST_VOID _micuTestNextDevice (MI_CONN *mi_conn, 
			    MIU_REMOTE *miuRemote, 
			    MICU_DEV *micIcfgDev);

static ST_VOID _micuSelect (MI_CONN *mi_conn, MICU_DEV *micIcfgDev);
static ST_VOID _micuSelectDone  (MI_REQ_CTRL *miReq);
static ST_VOID _micuOperate (MI_CONN *mi_conn, MICU_DEV *micIcfgDev);
static ST_VOID _micuOperateDone (MI_REQ_CTRL *miReq);
static ST_VOID _micuSetTag (MI_CONN *mi_conn, MICU_DEV *micIcfgDev);
static ST_VOID _micuSetTagDone  (MI_REQ_CTRL *miReq);
static ST_VOID _micuGetTag (MI_CONN *mi_conn, MICU_DEV *micIcfgDev);
static ST_VOID _micuGetTagDone  (MI_REQ_CTRL *miReq);

static ST_INT numDevTested;
static ST_INT numDevTestErr;

static ST_VOID _micuShowDeviceResult (ST_RET mic_result);

/* Get DV Attributes */
static ST_VOID _micuGetDvAttribDone (MI_REQ_CTRL *miReq);
static ST_RET micuSendGetDvAttrib (MI_CONN *mi_conn, MICU_DV *micIcfgDv, MI_REQ_CTRL **miReqOut);

/* Block 8 Operations */
static ST_VOID _micuStartTATransfers (MI_CONN *mi_conn);
static ST_VOID _micuStartTATransferDone (MI_REQ_CTRL *req);

static ST_VOID _micuStopTATransfers (MI_CONN *mi_conn);
static ST_VOID _micuStopTATransfersDone (MI_REQ_CTRL *req);

static ST_RET  _micuDeleteAllDataSets (MI_CONN *mi_conn);
static ST_VOID _micuDeleteAllDataSetsDone (MI_REQ_CTRL *req);

static ST_VOID slogTAReport (MI_TA_REPORT *taReport);
static ST_VOID slogDevOut (MI_DEVICE_OUTAGE *devOut);
static ST_VOID slogAvail (MI_AVAILABILITY *avail);
static ST_VOID slogStatus (MI_REAL_TIME_STATUS *status);
static ST_VOID slogForecast (MI_FORCAST_SCHEDULE *forecast);
static ST_VOID slogCurve (MI_CURVE *curve);
static ST_VOID slogGenData (MI_GEN_DATA_RPT *genData);
static ST_VOID slogLocalRef (MI_LOCAL_REF *localRef);
static ST_VOID slogPeriodicSegs (MI_TA_REPORT *taReport);
static ST_VOID slogProfileSegs (MI_TA_REPORT *taReport);
static ST_VOID slogSegUnion (MI_TA_SEGMENT *seg);
static ST_VOID slogPeriodicData (MI_TA_PERIODIC *periodic);
static ST_VOID slogProfileData (MI_TA_PROFILE *profile);
static ST_VOID slogFloatMatrix (MI_FLOAT_MATRIX *floatMatrix);
static ST_VOID slogIntMatrix (MI_INTEGER_MATRIX *intMatrix);
static ST_VOID slogTextMatrix (MI_TEXT_MATRIX *textMatrix);
#endif


#if 0	// CHOIBC DELETE
/************************************************************************/
/************************************************************************/
/*			micuStartAssocActivity 				*/
/************************************************************************/

/* Kick off a state machine for client activity on this association	*/
/*   1. Send Identify 							*/
/*   2. Execute Discovery						*/
/*   3  Create all configured Data Sets					*/
/*   4. Start all configured DS Transfer Sets				*/
/*   5. Enable IM transfers						*/

ST_VOID micuStartAssocActivity (MI_CONN *mi_conn)
  {
MI_REQ_CTRL *miReq;

  printf ("\nIssuing Identify request ...");
  miReq = mic_identify (mi_conn);
  if (miReq != NULL)
    miReq->u_req_done = _micuIdentifyDone;
  else
    {
    printf ("mic_identify error %04x", mi_op_err);
    return;
    }
  }

/************************************************************************/
/*			_micuIdentifyDone 				*/
/************************************************************************/

static ST_VOID _micuIdentifyDone (MI_REQ_CTRL *miReq)
  {
MI_REMOTE *mi_remote;
MI_CONN *mi_conn;
IDENT_RESP_INFO *ident_resp;

  mi_conn = miReq->mi_conn;
  mi_remote = mi_conn->mi_remote;

  if (miReq->result == MIC_RESULT_SUCCESS)
    {
    ident_resp = miReq->o.identify;
    printf ("\nIdentify OK");
    printf ("\n  Vendor = %s", ident_resp->vend);
    printf ("\n  Model = %s", ident_resp->model);
    printf ("\n  Rev = %s", ident_resp->rev);
    }
  else
    {
    printf ("\nIdentify failed %04x", miReq->result);
    }
  mi_free_req_ctrl (miReq);
  _micuDiscover (mi_conn);
  }

/************************************************************************/
/************************************************************************/
/*			_micuDiscover					*/
/************************************************************************/

static ST_VOID _micuDiscover (MI_CONN *mi_conn)
  {
MIC_GROBJ_CTRL *grobjCtrl;
MI_REQ_CTRL *miReq;
ST_RET rc;

/* See if we need to use the discovery features */
  grobjCtrl = mi_conn->mi_assoc->mic_grobj_ctrl;
  if (grobjCtrl && grobjCtrl->enable &&
      ((grobjCtrl->control & MIC_GROBJ_CTRL_EXECUTE_ALWAYS) ||
       ((grobjCtrl->control & MIC_GROBJ_CTRL_EXECUTE_ONCE) && grobjCtrl->state == 0)))
    {
    grobjCtrl->state = 1;
    printf ("\nExecuting discovery process ...");
    rc = mic_discover (mi_conn, grobjCtrl, &miReq);
    if (rc == SD_SUCCESS)
      {
      if (miReq != NULL)
        miReq->u_req_done = _micuDiscoverDone;
      else  /* No discover request needed */
        _micuStartAutoDsts (mi_conn);
      }
    else  
      {
      printf ("\nmic_discover error");
      _micuStartAutoDsts (mi_conn);
      }
    }
  else	/* Not discoverng, start creating datasets */
    _micuStartAutoDsts (mi_conn);
  }
      
/************************************************************************/
/*			_micuDiscoverDone  				*/
/************************************************************************/

static ST_VOID _micuDiscoverDone (MI_REQ_CTRL *miReq)
  {
MI_REMOTE *mi_remote;
MI_CONN *mi_conn;
MIC_GROBJ_CTRL *grobjCtrl;
MIU_REMOTE *miuRemote;

#ifdef MUI_REDUNDANCY_SUPPORT
MICU_DV    *micuDv;
#endif

MI_ICFG_USR_CTRL lc;
ST_INT rc;
ST_CHAR xmlOutFile[200];
ST_INT16 attrib_mask;

  mi_conn = miReq->mi_conn;
  mi_remote = mi_conn->mi_remote;
  miuRemote = miuFindMiRemote (mi_remote);

  if (miReq->result == MIC_RESULT_SUCCESS)
    {
    printf ("\nDiscover complete OK");

  /* At this point, configured DV validation and new DV discovery are	*/
  /* complete.								*/

    grobjCtrl = miReq->mi_grobj_ctrl;
    if (grobjCtrl->total_mismatches)
      {
      printf ("\n   Remote Data Value configuration mismatches (%d) detected", 
		    grobjCtrl->total_mismatches);
      printf ("\n   See log file for details"); 
      }
    else
      printf ("\n   Remote Data Value matches configuration");

    if (grobjCtrl->num_corrections)
      {
      printf ("\n   Remote Data Value configuration corrections (%d) detected", 
		    grobjCtrl->num_corrections);
      }

  /* Write the DV changes to the config database as appropriate		*/
    if (grobjCtrl->control & MIC_GROBJ_CTRL_UPDATE_DB)
      {
      }

  /* Write an XML encoded configuration file as appropriate */
    if (grobjCtrl->control & MIC_GROBJ_CTRL_WRITE_XML)
      {
      strcpy (xmlOutFile, "rem_");
      strcat (xmlOutFile, mi_conn->mi_remote->name);
      strcat (xmlOutFile, ".xml");
      rc = mic_encode_remote_xml (mi_conn->mi_assoc,  grobjCtrl, 
			        "RemoteList", xmlOutFile);
      if (rc == SD_SUCCESS)
        printf ("\nCreated IccpCfg XML file '%s'", xmlOutFile);
      else
        printf ("\nCould not create IccpCfg XML file '%s'", xmlOutFile);
      }

  /* Create new client Data Values as though configured */
    if (grobjCtrl->num_new_vcc_dv || grobjCtrl->num_new_icc_dv)
      {
      printf ("\nAdding %d newly discovered Data Values", 
	      grobjCtrl->num_new_vcc_dv + grobjCtrl->num_new_icc_dv);

    /* The mi_icfg_process_grobj function gives us the ICFG callbacks	*/
    /* for client DV only. 								*/
      lc.miuCurrRemote = miuRemote;
      rc = mi_icfg_process_grobj (mi_conn->mi_remote, grobjCtrl, &lc);
      if (rc != SD_SUCCESS)
        printf ("\nmi_icfg_process_grobj error");

#ifdef MUI_REDUNDANCY_SUPPORT
      if (miuRedundancyEnable)
        {
        micuDv = miuRemote->dv_list;
        while (micuDv != NULL)
          {
          micuDv->ir_idx = irFindMicDvIdx (mi_remote->mic_vcc, micuDv->mic_dv);
          micuDv = list_get_next (miuRemote->dv_list, micuDv);
          }
        irCfgChange ();
        }
#endif
      }

  /* Now see if we need to un-exclude some of the mismatched DV	*/
    attrib_mask = 0;
    if (!(grobjCtrl->control & MIC_GROBJ_CTRL_REMOVE_MISSING_FROM_DS))
      attrib_mask |= MIC_DV_MISSING;
    if (!(grobjCtrl->control & MIC_GROBJ_CTRL_REMOVE_MISTYPED_FROM_DS))
      attrib_mask |= MIC_DV_MISTYPE;
    if (!(grobjCtrl->control & MIC_GROBJ_CTRL_REMOVE_READ_ERROR_FROM_DS))
      attrib_mask |= MIC_DV_READ_ERROR;
  
    if (attrib_mask)
      mic_clear_dv_attrib (mi_conn->mi_remote, attrib_mask);

  /* OK, we are all done with the auto-discovery information */
    mic_free_grobj_ctrl (grobjCtrl);
    }
  else
    printf ("\nDiscover failed");

  mi_free_req_ctrl (miReq);
  _micuStartAutoDsts (mi_conn);
  }

/************************************************************************/
/************************************************************************/
/*			_micuStartAutoDsts			   	*/
/************************************************************************/

static ST_VOID _micuStartAutoDsts (MI_CONN *mi_conn)
  {
MIC_AUTO_DSTS_PARAM *autoDstsParam;
MIC_AUTO_DSTS_CTRL *autoDstsCtrl;
MI_REQ_CTRL *miReq;
ST_RET rc;

  /* See if we need to DeleteAllDataSets  */
  if (miuDeleteAllDs && miuNeedDelAllDs)
    {
    rc = _micuDeleteAllDataSets (mi_conn);
    if (rc == SD_SUCCESS)
      return;
    }

  /* See if we need to autocreate DSTS */
  autoDstsParam = mi_conn->mi_assoc->mic_auto_dsts_param;
  if (autoDstsParam && autoDstsParam->enable)
    {
    /* We will recreate the AUTO DS every time, so free the previous 	*/
    if (mi_conn->mi_assoc->mic_auto_dsts_ctrl != NULL)
      {
      mic_free_auto_dsts_ctrl (mi_conn->mi_assoc->mic_auto_dsts_ctrl);
      mi_conn->mi_assoc->mic_auto_dsts_ctrl = NULL;
      }

   /* Set the overall PDU size limit */
    autoDstsParam->maxDstsPduSize = mi_conn->net_info->max_pdu_size;
    rc = mic_create_auto_dsts_ctrl (mi_conn->mi_remote->mic_vcc, autoDstsParam, 
			       &autoDstsCtrl);
    if (rc != SD_SUCCESS)
      {
      printf ("\nmic_create_auto_dsts_ctrl error");
      _micuCreateCfgDs (mi_conn);
      return;
      }

  /* We could log the AUTO DSTS here for diagnostics   	*/
  /* mic_log_auto_dsts_ctrl (autoDstsCtrl); 		*/
  
  /* OK, we have set up the AUTO DSTS */
    mi_conn->mi_assoc->mic_auto_dsts_ctrl = autoDstsCtrl;
    if (autoDstsCtrl->numDsts)
      {
    /* Create the AUTO DS and start them */
      miReq = mic_start_auto_dsts (mi_conn, autoDstsCtrl);
      if (miReq != NULL)
        miReq->u_req_done = _micuStartAutoDstsDone;				   
      else
        {
        printf ("\nCould not start AUTO DSTS");
        _micuCreateCfgDs (mi_conn);
        }
      }
    else /* No AUTO DSTS */
      _micuCreateCfgDs (mi_conn);
    }
  else /* AUTO DSTS not enabled */
    _micuCreateCfgDs (mi_conn);
  }

/************************************************************************/
/*			_micuStartAutoDstsDone				*/
/************************************************************************/

static ST_VOID _micuStartAutoDstsDone (MI_REQ_CTRL *miReq)
  {
MI_CONN *mi_conn;

  mi_conn = miReq->mi_conn;
  if (miReq->result == SD_SUCCESS)
    printf ("\nAuto DSTS started OK");
  else
    printf ("\nAuto DSTS start error");

  mi_free_req_ctrl (miReq);
  _micuCreateCfgDs (mi_conn);
  }

/************************************************************************/
/************************************************************************/
/*			_micuCreateCfgDs				*/
/************************************************************************/

static ST_VOID _micuCreateCfgDs (MI_CONN *mi_conn)
  {
MIU_REMOTE *miuRemote;
MICU_DSTS  *miuDsTs;

/* Start with the dataset for the first DSTS */
  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  miuDsTs = miuRemote->dsts_list;
  while (miuDsTs)
    {
    if (!strcmp (miuDsTs->assocName, mi_conn->mi_assoc->name) &&
        (miuDsTs->miu_ds->mic_ds_in_use == SD_FALSE))
      break;

    miuDsTs = list_get_next (miuRemote->dsts_list, miuDsTs);
    }

/* Did we find a DSTS for this association? */
  if (miuDsTs != NULL)
    _micuCreateDs (mi_conn, miuDsTs);
  else
    {
    printf ("\nNo remote DSTS found for association");
    if (miuRemote->mi_remote->mic_vcc->misv.Supported_Features[0] & MI_BLOCK4)
      _micuStartIMTransfers (mi_conn);
    else
      {
      printf ("\nBLOCK4 not supported for Remote, not enabling");
      if (miuRemote->mi_remote->mic_vcc->misv.Supported_Features[0] & MI_BLOCK8)
	_micuStartTATransfers (mi_conn);
      }
    }
  }

/************************************************************************/
/*			_micuCreateDs					*/
/************************************************************************/

static ST_VOID _micuCreateDs (MI_CONN *mi_conn, MICU_DSTS *miuDsTs)
  {
MICU_DS *miIcfgDs;
MI_REQ_CTRL *miReq;

/* Create a Data Set in the server */
  miIcfgDs = miuDsTs->miu_ds;

  /* The assertion is that the DataSet is not already in use by a DSTS*/

  miIcfgDs->mic_ds_ok = SD_FALSE;
  miReq = mic_create_ds_ex (mi_conn, miIcfgDs->ds_name, 
				miIcfgDs->ds_scope,
				miIcfgDs->num_var, 
				miIcfgDs->var_tbl, &miIcfgDs->mic_ds);

  if (miReq != NULL)
    {
    miReq->user_info[0] = miuDsTs;
    miReq->u_req_done = _micuCreateDsDone;
    }
  else
    {
    printf ("\n _micuCreateDs Error! Could not create dataset '%s'",miIcfgDs->ds_name);
    _micuStartAllDsTs (mi_conn);
    }
  }

/************************************************************************/
/*			_micuCreateDsDone				*/
/************************************************************************/

static ST_VOID _micuCreateDsDone (MI_REQ_CTRL *miReq)
  {
MIU_REMOTE *miuRemote;
MICU_DSTS  *miuDsTs;
MICU_DS    *miIcfgDs;
MI_CONN *mi_conn;

  mi_conn = miReq->mi_conn;
  miuDsTs = miReq->user_info[0];
  miIcfgDs = miuDsTs->miu_ds;
  if (miReq->result == SD_SUCCESS)
    {
    printf ("\nICCP Create DS '%s' OK", miIcfgDs->ds_name);
    miIcfgDs->mic_ds_ok = SD_TRUE;
    miIcfgDs->mic_ds_in_use = SD_FALSE;
    }
  else
    {
    printf ("\nICCP Create DS '%s' Error", miIcfgDs->ds_name);
    }

  mi_free_req_ctrl (miReq);


#if 0
/* If the DataSet failed we can Reorganize the DataSet by reading the */
/* LOV. Or we could simply ditch the DataSet and unlink it from the   */
/* configuration. The "R" Read a DataSet feature functions with the   */
/* list of DataSets that passed the CreateDataSet operation           */
  if (miReq->result != SD_SUCCESS) 
    {                                 
    
    return;
    }
#endif

/* Ok, now define the next dataset, if there is one */
  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  miuDsTs = list_get_next (miuRemote->dsts_list, miuDsTs);
  while (miuDsTs)
    {
    if (!strcmp (miuDsTs->assocName, mi_conn->mi_assoc->name) &&
         (miuDsTs->miu_ds->mic_ds_in_use == SD_FALSE))
      break;

    miuDsTs = list_get_next (miuRemote->dsts_list, miuDsTs);
    }

/* If there are more DS to create then create them, else start DSTS	*/
  if (miuDsTs != NULL)
    _micuCreateDs (mi_conn, miuDsTs);
  else
    _micuStartAllDsTs (mi_conn);
  }

/************************************************************************/
/************************************************************************/
/*			_micuStartAllDsTs 				*/
/************************************************************************/

static ST_VOID _micuStartAllDsTs (MI_CONN *mi_conn)
  {
MIU_REMOTE *miuRemote;
MICU_DSTS  *miuDsTs;

/* Start with the first DSTS */
  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  miuDsTs = miuRemote->dsts_list;
  while (miuDsTs)
    {
    if (miuDsTs->miu_ds->mic_ds_ok)
      {
      if (!strcmp (miuDsTs->assocName, mi_conn->mi_assoc->name))
        break;
      }
    miuDsTs = list_get_next (miuRemote->dsts_list, miuDsTs);
    }

  if (miuDsTs)
    _micuStartDsTs (mi_conn, miuDsTs);
  else
    {
    printf ("\nNo remote DSTS started");
    if (miuRemote->mi_remote->mic_vcc->misv.Supported_Features[0] & MI_BLOCK4)
      _micuStartIMTransfers (mi_conn);
    else
      {
      printf ("\nBLOCK4 not supported for Remote, not enabling");
      if (miuRemote->mi_remote->mic_vcc->misv.Supported_Features[0] & MI_BLOCK8)
	_micuStartTATransfers (mi_conn);
      }
    }
  }

/************************************************************************/
/*			_micuPrintDstsData				*/
/************************************************************************/

static ST_VOID _micuPrintDstsData (const ST_CHAR *title, MI_DSTS_DATA *d)
  {
ST_UINT8 cond = d->DSConditionsRequested;

  printf ("\n%s DSTS Parameters:", title);
  printf ("\n  DataSet      : %s / %s",
    d->DataSetName.DomainName, d->DataSetName.name);
  printf ("\n  StartTime    : %ld%s",
    (long) d->StartTime, (d->StartTime == 0) ? " (즉시)" : "");
  printf ("\n  Interval     : %d 초", (int) d->Interval);
  printf ("\n  TLE          : %d 초", (int) d->TLE);
  printf ("\n  BufferTime   : %d 초", (int) d->BufferTime);
  printf ("\n  IntegrityChk : %d", (int) d->IntegrityCheck);
  printf ("\n  DSConditions : 0x%02x  [%s%s%s%s%s]",
    (unsigned) cond,
    (cond & MI_DSC_INTERVAL)  ? "Interval "  : "",
    (cond & MI_DSC_INTEGRITY) ? "Integrity " : "",
    (cond & MI_DSC_CHANGE)    ? "Change "    : "",
    (cond & MI_DSC_OPERATOR)  ? "Operator "  : "",
    (cond & MI_DSC_EXTERNAL)  ? "External "  : "");
  printf ("\n  RBE          : %c", d->RBE              ? 'Y' : 'N');
  printf ("\n  BlockData    : %c", d->BlockData         ? 'Y' : 'N');
  printf ("\n  Critical     : %c", d->Critical          ? 'Y' : 'N');
  printf ("\n  AllChanges   : %c", d->AllChangesReported ? 'Y' : 'N');
  printf ("\n  Status       : %c", d->Status            ? 'Y' : 'N');
  printf ("\n  EventCode    : %d\n", (int) d->EventCodeRequested);
  }

/************************************************************************/
/*			_micuStartDsTs					*/
/************************************************************************/

static ST_VOID _micuStartDsTs (MI_CONN *mi_conn, MICU_DSTS  *miuDsTs)
  {
MI_REQ_CTRL *miReq;
MI_DSTS_DATA mi_dsts_data;
MIU_REMOTE *miuRemote;

  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  mi_dsts_data = miuDsTs->dsts_data;
  if (miuDsTs->dsts_data.StartTime != 0)
    mi_dsts_data.StartTime = miuDsTs->dsts_data.StartTime + (ST_INT32) time (NULL);

  _micuPrintDstsData ("[CDSTS SEND]", &mi_dsts_data);
  miReq = mic_start_dsts (mi_conn, miuDsTs->miu_ds->mic_ds,  
			&mi_dsts_data, &miuDsTs->mic_dsts);

  if (miReq != NULL)
    {
    miReq->u_req_done =  _micuStartDsTsDone;
    miReq->user_info[0] = miuDsTs;
    miuDsTs->mic_dsts->user_info[0] = miuDsTs;
    }
  else
    {
    printf ("\n Error! Could not start the DSTS");
    if (miuRemote->mi_remote->mic_vcc->misv.Supported_Features[0] & MI_BLOCK4)
      _micuStartIMTransfers (mi_conn);
    else
      {
      printf ("\nBLOCK4 not supported for Remote, not enabling");
      if (miuRemote->mi_remote->mic_vcc->misv.Supported_Features[0] & MI_BLOCK8)
	_micuStartTATransfers (mi_conn);
      }
    
    }
  }

/************************************************************************/
/*	       	_micuStartDsTsDone 					*/
/************************************************************************/

static ST_VOID _micuStartDsTsDone (MI_REQ_CTRL *miReq)
  {
MIU_REMOTE *miuRemote;
MICU_DSTS  *miuDsTs;
MI_CONN *mi_conn;

  mi_conn = miReq->mi_conn;
  miuDsTs = miReq->user_info[0];
  if (miReq->result == SD_SUCCESS)
    {
    printf ("\nICCP Start DSTS '%s' OK", miuDsTs->dsTsName);
    miuDsTs->miu_ds->mic_ds_in_use = SD_TRUE;
    }

  else
    printf ("\nICCP Start DSTS '%s' Error", miuDsTs->dsTsName);

  mi_free_req_ctrl (miReq);

/* Ok, now define the next dataset */
  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  miuDsTs = list_get_next (miuRemote->dsts_list, miuDsTs);
  while (miuDsTs)
    {
    if (miuDsTs->miu_ds->mic_ds_ok)
      {
      if (!strcmp (miuDsTs->assocName, mi_conn->mi_assoc->name))
        break;
      }
    miuDsTs = list_get_next (miuRemote->dsts_list, miuDsTs);
    }

  if (miuDsTs)
    _micuStartDsTs (mi_conn, miuDsTs);
  else
    {
    if (miuRemote->mi_remote->mic_vcc->misv.Supported_Features[0] & MI_BLOCK4)
      _micuStartIMTransfers (mi_conn);
    else
      {
      printf ("\nBLOCK4 not supported for Remote, not enabling");
      if (miuRemote->mi_remote->mic_vcc->misv.Supported_Features[0] & MI_BLOCK8)
	_micuStartTATransfers (mi_conn);
      }
    }
  }

/************************************************************************/
/************************************************************************/
/*		      _micuStartIMTransfers				*/
/************************************************************************/

static ST_VOID _micuStartIMTransfers (MI_CONN *mi_conn)
  {
MI_REQ_CTRL *miReq;

  printf ("\n Starting IMTransfers for remote '%s'", mi_conn->remote_ar);
  miReq = mic_start_IMTransfer (mi_conn);
  if (miReq != NULL)
    miReq->u_req_done = _micuStartIMTransferDone;
  else
    printf (" : Failed!");
  }

/************************************************************************/
/* 		_micuStartIMTransferDone 				*/
/************************************************************************/
/*#define SQA_REGRESSION*/

#ifdef SQA_REGRESSION
ST_VOID br5_10(MI_CONN *mi_conn);
#endif

static ST_VOID _micuStartIMTransferDone (MI_REQ_CTRL *miReq)
  {
MI_CONN *mi_conn;
MVL_REQ_PEND *mvlReqPend;

  mvlReqPend = miReq->mvl_req;
  mi_conn = mvlReqPend->net_info->mi_conn; 
  printf ("\n START IMTransfer result = 0x%x", miReq->result);
#ifdef SQA_REGRESSION
  br5_10 (miReq->mi_conn);
#endif
  mi_free_req_ctrl (miReq);

  if (mi_conn->mi_remote->mic_vcc->misv.Supported_Features[0] & MI_BLOCK8)
    _micuStartTATransfers (mi_conn);

  }

/************************************************************************/
/************************************************************************/
/************************************************************************/
/*			micuStopAssocActivity 				*/
/************************************************************************/

/* Kick off a state machine to stop client activity on this association	*/
/*   1. Stop all auto DSTS						*/
/*   2. Stop all configured DS Transfer Sets				*/
/*   3. Disable IM transfers						*/

ST_VOID micuStopAssocActivity (MI_CONN *mi_conn)
  {
MIC_AUTO_DSTS_PARAM *autoDstsParam;
MIC_AUTO_DSTS_CTRL *autoDstsCtrl;
MI_REQ_CTRL *miReq;

  /* See if we need to stop auto DSTS */
  autoDstsParam = mi_conn->mi_assoc->mic_auto_dsts_param;
  if (autoDstsParam && autoDstsParam->enable)
    {
    autoDstsCtrl = mi_conn->mi_assoc->mic_auto_dsts_ctrl;
    miReq = mic_stop_auto_dsts (mi_conn, autoDstsCtrl);
    if (miReq != NULL)
      miReq->u_req_done = _micuStopAutoDstsDone;				   
    else
      {
      printf ("\nCould not stop AUTO DSTS");
      _micuStopAllDsTs (mi_conn);
      }
    }
  else
    _micuStopAllDsTs (mi_conn);
  }

/************************************************************************/
/*			_micuStopAutoDstsDone				*/
/************************************************************************/

static ST_VOID _micuStopAutoDstsDone (MI_REQ_CTRL *miReq)
  {
MI_CONN *mi_conn;

  mi_conn = miReq->mi_conn;
  if (miReq->result == SD_SUCCESS)
    printf ("\nAuto DSTS stopped OK");
  else
    printf ("\nAuto DSTS stop error");

  mi_free_req_ctrl (miReq);
  _micuStopAllDsTs (mi_conn);
  }

/************************************************************************/
/*			_micuStopAllDsTs 		 		*/
/************************************************************************/

static ST_VOID _micuStopAllDsTs (MI_CONN *mi_conn)
  {
MIU_REMOTE *miuRemote;
MICU_DSTS  *miuDsTs;

/* Start with the first DSTS */
  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  miuDsTs = miuRemote->dsts_list;
  while (miuDsTs)
    {
    if (miuDsTs->miu_ds->mic_ds_ok)
      {
      _micuStopDsTs (mi_conn, miuDsTs);
      break;
      }
    miuDsTs = list_get_next (miuRemote->dsts_list, miuDsTs);
    }

  if (miuDsTs == NULL)
    {
    _micuStopIMTransfers (mi_conn);
    _micuStopTATransfers (mi_conn);
    }
  }

/************************************************************************/
/*			_micuStopDsTs 					*/
/************************************************************************/

static ST_VOID _micuStopDsTs (MI_CONN *mi_conn, MICU_DSTS *miuDsTs)
  {
MI_REQ_CTRL *miReq;

  miReq = mic_stop_dsts (mi_conn, miuDsTs->mic_dsts);
  if (miReq != NULL)
    {
    miReq->u_req_done =  _micuStopDsTsDone;
    miReq->user_info[0] = miuDsTs;
    }
  else
    {
    printf ("\n Error! Could not stop the DSTS");
    _micuStopIMTransfers (mi_conn);
    _micuStopTATransfers (mi_conn);
    }
  }

/************************************************************************/
/*	       	_micuStopDsTsDone 					*/
/************************************************************************/

static ST_VOID _micuStopDsTsDone (MI_REQ_CTRL *miReq)
  {
MIU_REMOTE *miuRemote;
MICU_DSTS  *miuDsTs;
MI_CONN *mi_conn;

  mi_conn = miReq->mi_conn;
  miuDsTs = miReq->user_info[0];
  if (miReq->result == SD_SUCCESS)
    printf ("\nICCP Stop TS OK");
  else
    printf ("\nICCP Stop TS Error");
  mi_free_req_ctrl (miReq);


/* Ok, now stop the next one ... */
  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  miuDsTs = list_get_next (miuRemote->dsts_list, miuDsTs);
  while (miuDsTs)
    {
    if (miuDsTs->miu_ds->mic_ds_ok)
      {
      _micuStopDsTs (mi_conn, miuDsTs);
      break;
      }
    miuDsTs = list_get_next (miuRemote->dsts_list, miuDsTs);
    }
  if (miuDsTs == NULL)
    {
    _micuStopIMTransfers (mi_conn);
    _micuStopTATransfers (mi_conn);
    }
  }

/************************************************************************/
/************************************************************************/
/*		      _micuStopIMTransfers				*/
/************************************************************************/

static ST_VOID _micuStopIMTransfers (MI_CONN *mi_conn)
  {
MI_REQ_CTRL *miReq;

  printf ("\n Stopping IMTransfers for remote '%s'", mi_conn->remote_ar);
  miReq = mic_stop_IMTransfer (mi_conn);
  if (miReq)
    miReq->u_req_done = _micuStopIMTransfersDone;
  else
    printf ("\n Failed!!!");
  }

/************************************************************************/
/*			_micuStopIMTransfersDone 			*/
/************************************************************************/

static ST_VOID _micuStopIMTransfersDone (MI_REQ_CTRL *miReq)
  {
MI_CONN *mi_conn;
MVL_REQ_PEND *mvlReqPend;

  mvlReqPend = miReq->mvl_req;
  mi_conn = mvlReqPend->net_info->mi_conn;
  printf ("\n STOP IMTransfer for '%s' Result = 0x%x", 
	  mi_conn->remote_ar, miReq->result);
  mi_free_req_ctrl (miReq);
  }


/************************************************************************/
/************************************************************************/
/*			micuReadAllDv 					*/
/************************************************************************/

ST_VOID micuReadAllDv (MI_CONN *mi_conn)
  {
MIU_REMOTE *miuRemote;

/* Start with the first data value */
  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  _micuReadDv (mi_conn, miuRemote->dv_list);
  }

/************************************************************************/
/*			_micuReadDv					*/
/************************************************************************/

#define MAX_DV_PER_READ	100
ST_INT numReadOut = 0;

static ST_VOID _micuReadDv (MI_CONN *mi_conn, MICU_DV *micIcfgDv)
  {
MIU_REMOTE *miuRemote;
MI_REQ_CTRL *miReq;
MIC_DV *dv[MAX_DV_PER_READ];
ST_INT i;

  if (micIcfgDv == NULL)
    {
    QPRINTF ("\nNo Remote DV(s) to Read 'r' .. won't work!");
    return;
    }

/* Read a number of DV ... */
  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  for (i = 0; i < MAX_DV_PER_READ && micIcfgDv; ++i)
    {
    dv[i] = micIcfgDv->mic_dv;
    micIcfgDv = list_get_next (miuRemote->dv_list, micIcfgDv);
    }

  miReq = mic_read_dv (mi_conn, i, dv);
  if (!miReq)
    {
    printf ("\n Error! Could not read the data values");
    return;
    }

  ++numReadOut;
  miReq->u_req_done =  _micuReadDvDone;
  miReq->user_info[0] = micIcfgDv;
  }

/************************************************************************/
/*			_micuReadDvDone 				*/
/************************************************************************/

static ST_VOID _micuReadDvDone (MI_REQ_CTRL *miReq)
  {
MI_CONN *mi_conn;
MIU_REMOTE *miuRemote;
MICU_DV *micIcfgDv;

  --numReadOut;

  mi_conn = miReq->mi_conn;
  micIcfgDv = miReq->user_info[0];
  if (miReq->result == SD_SUCCESS)
    {
    QPRINTF ("\nDV Read OK");
    }
  else
    printf ("\nDV Read Error");
  mi_free_req_ctrl (miReq);

/* Ok, now read the next bunch */
  miuRemote = miuFindMiRemote (mi_conn->mi_remote);

  micIcfgDv = list_get_next (miuRemote->dv_list, micIcfgDv);
  if (micIcfgDv != NULL)  /* If this was not the last ... */
    _micuReadDv (mi_conn, micIcfgDv);
  else
    {
    QPRINTF ("Read all DV complete");
    }
  }

/************************************************************************/
/************************************************************************/
/*			micuReadAllDs 					*/
/************************************************************************/

ST_VOID micuReadAllDs (MI_CONN *mi_conn)
  {
MIU_REMOTE *miuRemote;

/* Start with the first data value */
  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  _micuReadDs (mi_conn, miuRemote->ds_list);
  }

/************************************************************************/
/*			_micuReadDs					*/
/************************************************************************/

static ST_VOID _micuReadDs (MI_CONN *mi_conn, MICU_DS *micIcfgDs)
  {
MIU_REMOTE *miuRemote;
MI_REQ_CTRL *miReq;

  if (micIcfgDs == NULL)
    { 
    QPRINTF ("\nNo Remote DataSet(s) to Read 'R' .. won't work!");
    return;
    }

  miuRemote = miuFindMiRemote (mi_conn->mi_remote);

/* If this DS was not attached to a DSTS it may not have been created */
  if (micIcfgDs->mic_ds == NULL)
    {
    micIcfgDs->mic_ds_ok = SD_FALSE;
    miReq = mic_create_ds_ex (mi_conn, micIcfgDs->ds_name, 
				micIcfgDs->ds_scope,
				micIcfgDs->num_var, 
				micIcfgDs->var_tbl, &micIcfgDs->mic_ds);

    if (miReq != NULL)
      {
      miReq->user_info[0] = micIcfgDs;
      miReq->u_req_done = _micuCreateDsForRdDone;
      }
    else
      printf ("\n _micuReadDs Error! Could not create the dataset");

    return;
    }

  /* This function is passed in a pointer to some place in the ds_list */
  /* for some Remote.  If the micIcfgDs we have our hands on is not ok */
  /* we either need to find one that is ok or not read any DataSets    */

  while (micIcfgDs && (micIcfgDs->mic_ds_ok == SD_FALSE) )
    {
    micIcfgDs = (MICU_DS *)list_get_next(miuRemote->ds_list,micIcfgDs);
    }

  if (micIcfgDs == NULL ) /* we went though the entire list and came up empty */
    {
    printf ("\n Error! Could not find a valid DS to read");
    return;
    }

  miReq = mic_read_ds (mi_conn, micIcfgDs->mic_ds);
  if (!miReq)
    {
    printf ("\n Error! Could not read the DS");
    return;
    }

  miReq->u_req_done =  _micuReadDsDone;
  miReq->user_info[0] = micIcfgDs;
  }

/************************************************************************/
/*			_micuCreateDsForRdDone				*/
/************************************************************************/

static ST_VOID _micuCreateDsForRdDone (MI_REQ_CTRL *miReq)
  {
MICU_DS    *micIcfgDs;
MI_CONN *mi_conn;

  mi_conn = miReq->mi_conn;
  micIcfgDs = miReq->user_info[0];
  if (miReq->result == SD_SUCCESS)
    {
    printf ("\nICCP Create DS '%s' OK", micIcfgDs->ds_name);
    micIcfgDs->mic_ds_ok = SD_TRUE;
    _micuReadDs (mi_conn, micIcfgDs);
    }
  else
    printf ("\nICCP Create DS '%s' Error", micIcfgDs->ds_name);

  mi_free_req_ctrl (miReq);
  }


/************************************************************************/
/*			_micuReadDsDone 				*/
/************************************************************************/

static ST_VOID _micuReadDsDone (MI_REQ_CTRL *miReq)
  {
MI_CONN *mi_conn;
MIU_REMOTE *miuRemote;
MICU_DS *micIcfgDs;

  mi_conn = miReq->mi_conn;
  micIcfgDs = miReq->user_info[0];
  if (miReq->result == SD_SUCCESS)
    {
    QPRINTF ("\nDS Read OK");
    }
  else
    printf ("\nDS Read Error");
  mi_free_req_ctrl (miReq);

/* Ok, now read the next bunch */
  miuRemote = miuFindMiRemote (mi_conn->mi_remote);

  micIcfgDs = list_get_next (miuRemote->ds_list, micIcfgDs);
  if (micIcfgDs != NULL)  /* If this was not the last ... */
    _micuReadDs (mi_conn, micIcfgDs);
  else
    {
    QPRINTF ("Read all DS complete");
    }
  }
#endif	// CHOIBC DELETE

/************************************************************************/
/************************************************************************/
/* DATA VALUE ACTIVITY							*/
/************************************************************************/
/*			u_mic_dv_ind_start 				*/
/************************************************************************/

/* This function is called when a set of new Data Value values is 	*/
/* received, and before MI starts calling 'u_mic_dv_ind'. After all 	*/
/* calls to 'u_mic_dv_ind' have been made for the MMS PDU, 		*/
/* 'u_mic_dv_ind_end' will be called.					*/

ST_VOID u_mic_dv_ind_start_ex (MI_CONN *mi_conn, ST_INT reason,
			       MIC_DSTS *mic_dsts, MIC_DS *mic_ds, 
			       ST_INT num_dv, ST_VOID **usrOut)
  {
#if 0	// CHOIBC DELETE
  if (reason == MIC_DV_IND_REASON_REPORT)
    {
    QPRINTF ("\n\nReport DV indications starting ...");
    *usrOut = mic_dsts;
    }
  else /* reason == MIC_DV_IND_REASON_READ */
    {
    QPRINTF ("\n\nRead response DV indications starting ...");
    *usrOut = NULL;
    }
#endif	// CHOIBC DELETE
  }

/************************************************************************/
/*			u_mic_dv_ind 					*/
/************************************************************************/

/* This function is called once for each Data Value received via Read	*/
/* confirm or Information Report indication.				*/

/* The application can do what it pleases with the data; in the case of	*/
/* our sample we will simply print it. The 'handle_info' is a 'tag'	*/
/* string to use in the print.						*/

ST_VOID u_mic_dv_ind_ex (MI_CONN *mi_conn, MIC_DV *dv, ST_RET result,
		        ST_VOID *data_buf, ST_INT mi_type, ST_VOID *usr)
  {
#if 0	// CHOIBC DELETE
MIC_DSTS 		*mic_dsts;
MICU_DV 		*micIcfgDv;
MI_ACCESS_RESULT	*ar;

MI_REAL 		*dr;
MI_STATE 	   	*ds;
MI_DISCRETE 	   	*dd;
MI_STATE_SUPP		*dss;
MI_REAL_Q 	   	*drq;
MI_STATE_Q 	   	*dsq;
MI_DISCRETE_Q 	   	*ddq;
MI_STATE_SUPP_Q		*dssq;
MI_REAL_Q_TIMETAG 	*drqt;
MI_STATE_Q_TIMETAG    	*dsqt;
MI_DISCRETE_Q_TIMETAG 	*ddqt;
MI_STATE_SUPP_Q_TIMETAG *dssqt;
MI_REAL_EXTENDED 	*dre;
MI_STATE_EXTENDED    	*dse;
MI_DISCRETE_EXTENDED 	*dde;
MI_STATE_SUPP_EXTENDED  *dsse;
MI_REAL_Q_TIMETAG_EXTENDED 	 *drqte;
MI_STATE_Q_TIMETAG_EXTENDED    	 *dsqte;
MI_DISCRETE_Q_TIMETAG_EXTENDED 	 *ddqte;
MI_STATE_SUPP_Q_TIMETAG_EXTENDED *dssqte;

MI_MMS_OBJECT_NAME 	*ts_name;
MI_TS_TIMESTAMP    	*ts_timestamp;
MI_DS_CONDITIONS   	*ds_cond;
MI_EC_DETECTED     	*ec_detected;
ST_CHAR timeBuf[30];
time_t  tt;

  if (result != SD_SUCCESS)
    {
    ar = data_buf;
    printf ("\n Error receiving DV '%s', (%d, %d)", 
    	dv->dv_name, (int) ar->acc_rslt_tag, (int) ar->failure);

    return;
    }

  QPRINTF ("\nDV:%s\t", dv->dv_name);
  mic_dsts = (MIC_DSTS *) usr;

  micIcfgDv = dv->handle_info;
  
  switch (mi_type)
    {
    case MI_TYPEID_REAL:
      dr = data_buf;
      QPRINTF ("Val: %f  ", *dr);

    /* Store the received remote data for later use */
      micIcfgDv->data.r = *dr;
    break;
  
    case MI_TYPEID_STATE:
      ds = data_buf;
      QPRINTF ("Flags: %02x  ", (int) *ds);

    /* Store the received remote data for later use */
      micIcfgDv->data.s = *ds;
    break;
  
    case MI_TYPEID_DISCRETE:
      dd = data_buf;
      QPRINTF ("Val: %ld  ", (long) *dd);

    /* Store the received remote data for later use */
      micIcfgDv->data.d = *dd;
    break;
  
    case MI_TYPEID_STATE_SUPP:
      dss = data_buf;
      QPRINTF ("Val: %02x  ", (int) *dss);

    /* Store the received remote data for later use */
      micIcfgDv->data.ss = *dss;
    break;

    case MI_TYPEID_REAL_Q:
      drq = data_buf;
      QPRINTF ("Val: %f  ", drq->Value);
      QPRINTF ("Flags: %02x  ", (int) drq->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.rq = *drq;
    break;
  
    case MI_TYPEID_STATE_Q:
      dsq = data_buf;
      QPRINTF ("Flags: %02x  ", (int) *dsq);

    /* Store the received remote data for later use */
      micIcfgDv->data.sq = *dsq;
    break;
  
    case MI_TYPEID_DISCRETE_Q:
      ddq = data_buf;
      QPRINTF ("Val: %ld  ", (long) ddq->Value);
      QPRINTF ("Flags: %02x  ", (int) ddq->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.dq = *ddq;
    break;
  
    case MI_TYPEID_STATE_SUPP_Q:
      dssq = data_buf;
      QPRINTF ("Val: %02x  ", (int) dssq->Value);
      QPRINTF ("Flags: %02x  ", (int) dssq->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.ssq = *dssq;
    break;

    case MI_TYPEID_REAL_Q_TIMETAG:
      drqt = data_buf;
      QPRINTF ("Val: %f  ", drqt->Value);
      if (drqt->Flags & MI_QFLAG_TIMESTAMP_INVALID)
        {
        QPRINTF ("TS: Invalid");
	}
      else
        {
        tt = (time_t) drqt->TimeStamp;
        strcpy (timeBuf, ctime(&tt));
        timeBuf[24] = 0;
        QPRINTF ("TS: %s  ", timeBuf);
	}

      QPRINTF ("Flags: %02x  ", (int) drqt->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.rqt = *drqt;
    break;
  
    case MI_TYPEID_STATE_Q_TIMETAG:
      dsqt = data_buf;
      if (dsqt->Flags & MI_QFLAG_TIMESTAMP_INVALID)
        {
        QPRINTF ("TS: Invalid");
	}
      else
        {
        tt = (time_t) dsqt->TimeStamp;
        strcpy (timeBuf, ctime(&tt));
        timeBuf[24] = 0;
        QPRINTF ("TS: %s  ", timeBuf);
	}
      QPRINTF ("Flags: %02x  ", (int) dsqt->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.sqt = *dsqt;
    break;
  
    case MI_TYPEID_DISCRETE_Q_TIMETAG:
      ddqt = data_buf;
      QPRINTF ("Val: %ld  ", (long) ddqt->Value);
      if (ddqt->Flags & MI_QFLAG_TIMESTAMP_INVALID)
        {
        QPRINTF ("TS: Invalid");
	}
      else
        {
        tt = (time_t) ddqt->TimeStamp;
        strcpy (timeBuf, ctime(&tt));
        timeBuf[24] = 0;
        QPRINTF ("TS: %s  ", timeBuf);
	}
      QPRINTF ("Flags: %02x  ", (int) ddqt->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.dqt = *ddqt;
    break;
  
    case MI_TYPEID_STATE_SUPP_Q_TIMETAG:
      dssqt = data_buf;
      QPRINTF ("Val: %02x  ", (int) dssqt->Value);
      if (dssqt->Flags & MI_QFLAG_TIMESTAMP_INVALID)
        {
        QPRINTF ("TS: Invalid");
	}
      else
        {
        tt = (time_t) dssqt->TimeStamp;
        strcpy (timeBuf, ctime(&tt));
        timeBuf[24] = 0;
        QPRINTF ("TS: %s  ", timeBuf);
	}
      QPRINTF ("Flags: %02x  ", (int) dssqt->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.ssqt = *dssqt;
    break;

    case MI_TYPEID_REAL_EXTENDED:
      dre = data_buf;
      QPRINTF ("Val: %f  ", dre->Value);
      if (dre->Flags & MI_QFLAG_TIMESTAMP_INVALID)
        {
        QPRINTF ("TS: Invalid");
	}
      else
        {
        tt = (time_t) dre->TimeStamp;
        strcpy (timeBuf, ctime(&tt));
        timeBuf[24] = 0;
        QPRINTF ("TS: %s  ", timeBuf);
	}
      QPRINTF ("COV: %d  ", (int)dre->COV);
      QPRINTF ("Flags: %02x  ", (int) dre->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.re = *dre;
    break;
  
    case MI_TYPEID_STATE_EXTENDED:
      dse = data_buf;
      if (dse->Flags & MI_QFLAG_TIMESTAMP_INVALID)
        {
        QPRINTF ("TS: Invalid");
	}
      else
        {
        tt = (time_t) dse->TimeStamp;
        strcpy (timeBuf, ctime(&tt));
        timeBuf[24] = 0;
        QPRINTF ("TS: %s  ", timeBuf);
	}
      QPRINTF ("COV: %d  ", (int)dse->COV);
      QPRINTF ("Flags: %02x  ", (int) dse->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.se = *dse;
    break;
  
    case MI_TYPEID_DISCRETE_EXTENDED:
      dde = data_buf;
      QPRINTF ("Val: %ld  ", (long) dde->Value);
      if (dde->Flags & MI_QFLAG_TIMESTAMP_INVALID)
        {
        QPRINTF ("TS: Invalid");
	}
      else
        {
        tt = (time_t) dde->TimeStamp;
        strcpy (timeBuf, ctime(&tt));
        timeBuf[24] = 0;
        QPRINTF ("TS: %s  ", timeBuf);
	}
      QPRINTF ("COV: %d  ", (int)dde->COV);
      QPRINTF ("Flags: %02x  ", (int) dde->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.de = *dde;
    break;

    case MI_TYPEID_STATE_SUPP_EXTENDED:
      dsse = data_buf;
      QPRINTF ("Val: %02x  ", (int) dsse->Value);
      if (dsse->Flags & MI_QFLAG_TIMESTAMP_INVALID)
        {
        QPRINTF ("TS: Invalid");
	}
      else
        {
        tt = (time_t) dsse->TimeStamp;
        strcpy (timeBuf, ctime(&tt));
        timeBuf[24] = 0;
        QPRINTF ("TS: %s  ", timeBuf);
	}
      QPRINTF ("COV: %d  ", (int)dsse->COV);
      QPRINTF ("Flags: %02x  ", (int) dsse->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.sse = *dsse;
    break;

    case MI_TYPEID_REAL_Q_TIMETAG_EXTENDED:
      drqte = data_buf;
      QPRINTF ("Val: %f  ", drqte->Value);
      if (drqte->Flags & MI_QFLAG_TIMESTAMP_INVALID)
        {
        QPRINTF ("TS: Invalid");
	}
      else
        {
        tt = (time_t) drqte->TimeStamp.GMTBasedS;
        strcpy (timeBuf, ctime(&tt));
        timeBuf[24] = 0;
        QPRINTF ("TS: %s  ", timeBuf);
        QPRINTF ("MS: %d  ", (int) drqte->TimeStamp.Milliseconds);
	}

      QPRINTF ("Flags: %02x  ", (int) drqte->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.rqte = *drqte;
    break;
  
    case MI_TYPEID_STATE_Q_TIMETAG_EXTENDED:
      dsqte = data_buf;
      if (dsqte->Flags & MI_QFLAG_TIMESTAMP_INVALID)
        {
        QPRINTF ("TS: Invalid");
	}
      else
        {
        tt = (time_t) dsqte->TimeStamp.GMTBasedS;
        strcpy (timeBuf, ctime(&tt));
        timeBuf[24] = 0;
        QPRINTF ("TS: %s  ", timeBuf);
        QPRINTF ("MS: %d  ", (int) dsqte->TimeStamp.Milliseconds);
	}
      QPRINTF ("Flags: %02x  ", (int) dsqte->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.sqte = *dsqte;
    break;
  
    case MI_TYPEID_DISCRETE_Q_TIMETAG_EXTENDED:
      ddqte = data_buf;
      QPRINTF ("Val: %ld  ", (long) ddqte->Value);
      if (ddqte->Flags & MI_QFLAG_TIMESTAMP_INVALID)
        {
        QPRINTF ("TS: Invalid");
	}
      else
        {
        tt = (time_t) ddqte->TimeStamp.GMTBasedS;
        strcpy (timeBuf, ctime(&tt));
        timeBuf[24] = 0;
        QPRINTF ("TS: %s  ", timeBuf);
        QPRINTF ("MS: %d  ", (int) ddqte->TimeStamp.Milliseconds);
	}
      QPRINTF ("Flags: %02x  ", (int) ddqte->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.dqte = *ddqte;
    break;

    case MI_TYPEID_STATE_SUPP_Q_TIMETAG_EXTENDED:
      dssqte = data_buf;
      QPRINTF ("Val: %02x  ", (int) dssqte->Value);
      if (dssqte->Flags & MI_QFLAG_TIMESTAMP_INVALID)
        {
        QPRINTF ("TS: Invalid");
	}
      else
        {
        tt = (time_t) dssqte->TimeStamp.GMTBasedS;
        strcpy (timeBuf, ctime(&tt));
        timeBuf[24] = 0;
        QPRINTF ("TS: %s  ", timeBuf);
        QPRINTF ("MS: %d  ", (int) dssqte->TimeStamp.Milliseconds);
	}
      QPRINTF ("Flags: %02x  ", (int) dssqte->Flags);

    /* Store the received remote data for later use */
      micIcfgDv->data.ssqte = *dssqte;
    break;

  /* the rest are not data values */
    case MI_TYPEID_MMS_OBJECT_NAME:
      ts_name = data_buf;
      QPRINTF ("TS Name: %s  ", ts_name->Name);
      if (ts_name->Scope == 1)
        {
        QPRINTF (  ", Domain: %s  ", ts_name->DomainName);
	}
    break;

    case MI_TYPEID_GMT_BASED_S:
      ts_timestamp = data_buf;
      QPRINTF ("TS TimeStamp: %ld  ", (long) *ts_timestamp);
    break;

    case MI_TYPEID_INT16:
      ec_detected = data_buf;
      QPRINTF ("Event Code Detected: 0x%02x  ", (int) *ec_detected);
    break;

    case MI_TYPEID_DS_COND:
      ds_cond = data_buf;
      QPRINTF ("DSConditions Detected:");
      if (*ds_cond & MI_DSC_INTERVAL)
        {
        QPRINTF (" Interval");
        }
      if (*ds_cond & MI_DSC_INTEGRITY)
        {
        QPRINTF (" Integrity");
        }
      if (*ds_cond & MI_DSC_CHANGE)
        {
        QPRINTF (" Change");
        }
      if (*ds_cond & MI_DSC_OPERATOR)
        {
        QPRINTF (" Operator");
        }
      if (*ds_cond & MI_DSC_EXTERNAL)
        {
        QPRINTF (" External");
        }
    break;
  
    default:
      printf ("\nError - Check Me Out!");
    break;
    }

#ifdef MUI_REDUNDANCY_SUPPORT
  if (micIcfgDv == NULL)
    return;	/* Special Variables have no handle_info */
  miuIrMicDvUpdate (mi_conn->mi_remote, micIcfgDv->ir_idx, 
  		    micIcfgDv->mi_type, &micIcfgDv->data);
#endif
#endif	// CHOIBC DELETE
  }

/************************************************************************/
/*			u_mic_tag_value_ind_ex				*/
/************************************************************************/

ST_VOID u_mic_tag_value_ind_ex (MI_CONN *mi_conn, MIC_DEVICE *dev, ST_RET result,
			        MI_TAG_VALUE *tag_value, ST_VOID *usr)
  {
#if 0	// CHOIBC DELETE
  QPRINTF ("\nDevice:%s Tag:\t", dev->device_name);
  switch (tag_value->TagFlags & MI_TAG_FLAG_TAG_MASK)
    {
    case MI_TAG_FLAG_NO_TAG:
      QPRINTF ("NO_TAG");
    break;

    case MI_TAG_FLAG_OPEN_CLOSE_INHIBIT:
      QPRINTF ("OPEN_CLOSE_INHIBIT");
    break;

    case MI_TAG_FLAG_CLOSE_INHIBIT:
      QPRINTF ("CLOSE_INHIBIT");
    break;

    default:
      QPRINTF ("Invalid");
    break;
    }

  if (tag_value->TagFlags & MI_TAG_FLAG_ARMED)
    {
    QPRINTF ("ARMED");
    }
  else
    {
    QPRINTF ("NOT ARMED");
    }

  QPRINTF (" Reason:'%s'", tag_value->Reason);
#endif	// CHOIBC DELETE
  }

/************************************************************************/
/*			u_mic_dv_ind_end   				*/
/************************************************************************/
/* This function is called when all calls to 'u_mic_dv_ind' have been 	*/
/* made for the MMS PDU.						*/

ST_VOID u_mic_dv_ind_end_ex (MI_CONN *mi_conn, ST_INT reason,
		             MIC_DSTS *mic_dsts, MIC_DSTS_STO *sto, 
		             ST_VOID *usr)
  {
#if 0	// CHOIBC DELETE
ST_RET rc;
static ST_INT ackCount = 0;

  if (reason == MIC_DV_IND_REASON_READ)
    {
    QPRINTF ("\nRead response DV indications complete");
    }
  else if (reason == MIC_DV_IND_REASON_REPORT)
    {
    QPRINTF ("\nReport DV indications complete");
    if (mic_dsts->mi_dsts_data.Critical)
      {
      ++ackCount;
      if (ackCount % 3)
        rc = mic_ack_dsts (mi_conn, mic_dsts, sto, SD_FALSE);
      else
        rc = mic_ack_dsts (mi_conn, mic_dsts, sto, SD_TRUE);

      if (rc != SD_SUCCESS)
        printf ("\nReport ACK error %d", rc);
      }
    }
#endif	// CHOIBC DELETE
  }


#if 0	// CHOIBC DELETE
/************************************************************************/
/************************************************************************/
/*			micuWriteDv 					*/
/************************************************************************/

#define MAX_DV_PER_WRITE	100

ST_VOID micuWriteDv (MI_CONN *mi_conn)
  {
MIU_REMOTE *miuRemote;
MICU_DV 	*micIcfgDv;
MI_REQ_CTRL *miReq;
MIC_WR_DV_CTRL *wrDvCtrl;
MIC_DV_WR_INFO *dvWrInfo;
ST_INT i;

  wrDvCtrl = chk_calloc (1, sizeof (MIC_WR_DV_CTRL) +
                            MAX_DV_PER_WRITE * sizeof (MIC_DV_WR_INFO));
  wrDvCtrl->mic_dv_wr_info = (MIC_DV_WR_INFO *) (wrDvCtrl+1);
  dvWrInfo = wrDvCtrl->mic_dv_wr_info;

/* Write a number of DV; assume that the data has been set 		*/
  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  micIcfgDv = miuRemote->dv_list;

  if (micIcfgDv==NULL)
    {
    QPRINTF ("\nNo Remote DV(s) to Write 'w' .. won't work!");
    chk_free (wrDvCtrl);
    return;
    }

  for (i = 0; i < MAX_DV_PER_WRITE && micIcfgDv; ++i)
    {
    ++wrDvCtrl->num_dv;
    dvWrInfo[i].mic_dv = micIcfgDv->mic_dv;
    dvWrInfo[i].wr_data = &micIcfgDv->data;
    micIcfgDv = list_get_next (miuRemote->dv_list, micIcfgDv);
    }

  miReq = mic_write_dv (mi_conn, wrDvCtrl);
  if (!miReq)
    {
    printf ("\n Error! Could not issue the write");
    return;
    }
  miReq->u_req_done =  _micuDvWriteDone;
  }

/************************************************************************/
/*			_micuDvWriteDone 	      			*/
/************************************************************************/

static ST_VOID _micuDvWriteDone (MI_REQ_CTRL *miReq)
  {
MIC_WR_DV_CTRL *wrDvCtrl;
MIC_DV_WR_INFO *dvWrInfo;
ST_INT i;

  wrDvCtrl = miReq->mic_wr_dv_ctrl;
  dvWrInfo = wrDvCtrl->mic_dv_wr_info;

  if (miReq->result == SD_SUCCESS)
    {
    QPRINTF ("\nICCP DV Write Confirm OK");
    for (i = 0; i < wrDvCtrl->num_dv; ++i)
      {
      if (dvWrInfo[i].result != SD_SUCCESS)
        printf ("\n DV '%s' Write Error 0x%02x", dvWrInfo[i].mic_dv->dv_name, dvWrInfo[i].result);
      }
    }
  else
    printf ("\nICCP DV Write Error");

  chk_free (wrDvCtrl);
  mi_free_req_ctrl (miReq);
  }
#endif	// CHOIBC DELETE

/************************************************************************/
/* Block 4								*/
/************************************************************************/
/************************************************************************/
/* u_mic_receive_IMTransfer: called when an Information Message is recd*/
/************************************************************************/

ST_VOID u_mic_receive_IMTransfer (MI_CONN *mi_conn, 
                                  MI_INFO_BUF_HEADER *buf_header, 
			          ST_UCHAR *info_buf, ST_INT msg_scope)
  {
#if 0	// CHOIBC DELETE
  QPRINTF ("\n Incoming IMTransfer Report:  ");
  QPRINTF ("\n\t InfoReference = %ld", (long) buf_header->InfoReference);
  QPRINTF ("\n\t LocalReference = %ld", (long) buf_header->LocalReference);
  QPRINTF ("\n\t MessageId = %ld", (long) buf_header->MessageId);
  QPRINTF ("\n\t Size = %ld", (long) buf_header->Size);
#endif	// CHOIBC DELETE
  }

#if 0	// CHOIBC DELETE
/************************************************************************/
/* Block 5								*/
/************************************************************************/
/************************************************************************/
/*			micuTestAllDevices 				*/
/************************************************************************/

ST_VOID micuTestAllDevices (MI_CONN *mi_conn)
  {
MIU_REMOTE *miuRemote;
MICU_DEV *micIcfgDev;

  numDevTested = 0;
  numDevTestErr = 0;

/* Start with the first configured device */
  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  if (miuRemote->dev_list != NULL)
    {
    micIcfgDev = miuRemote->dev_list;
    _micuDeviceTest (mi_conn, micIcfgDev);
    }
  else
    printf ("\nNo Devices To Test");
  }

/************************************************************************/
/*			_micuTestNextDevice 				*/
/************************************************************************/

static ST_VOID _micuTestNextDevice (MI_CONN *mi_conn, 
			    MIU_REMOTE *miuRemote, 
			    MICU_DEV *micIcfgDev)
  {

/* Get the next configured device */
  micIcfgDev = list_get_next (miuRemote->dev_list, micIcfgDev);
  if (micIcfgDev != NULL)
    _micuDeviceTest (mi_conn, micIcfgDev);
  else
    printf ("\n%d Devices Tested, %d errors", numDevTested, numDevTestErr);
  }

/************************************************************************/
/*			_micuDeviceTest 					*/
/************************************************************************/

static ST_VOID _micuDeviceTest (MI_CONN *mi_conn, MICU_DEV *micIcfgDev)
  {
MIU_REMOTE *miuRemote;
MIC_DEVICE *mic_dev;

  ++numDevTested;

  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  mic_dev = micIcfgDev->mic_dev;

/* If this is a SBO device, need to select first 			*/
  if (mic_dev->device_attrib & MIC_DEVICE_SBO)
    _micuSelect (mi_conn, micIcfgDev);
  else	/* Not a SBO device, just operate it ... */
    _micuOperate (mi_conn, micIcfgDev);
  }

/************************************************************************/
/*			_micuSelect 					*/
/************************************************************************/

static ST_VOID _micuSelect (MI_CONN *mi_conn, MICU_DEV *micIcfgDev)
  {
MIU_REMOTE *miuRemote;
MIC_DEVICE *mic_dev;
MI_REQ_CTRL *miReq;

  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  mic_dev = micIcfgDev->mic_dev;

/* If this is a SBO device, need to select first 			*/
  miReq = mic_device_select (mi_conn, micIcfgDev->mic_dev);
  if (!miReq)
    {
    SLOGALWAYS0 ("mic_device_select Failed.");
    ++numDevTestErr;
    }
  else
    {
    miReq->u_req_done = _micuSelectDone;
    miReq->user_info[0] = micIcfgDev;
    }
  }

/************************************************************************/
/*			_micuSelectDone  				*/
/************************************************************************/

static ST_VOID _micuSelectDone  (MI_REQ_CTRL *miReq)
  {
MICU_DEV *micIcfgDev;
MI_CONN *mi_conn;
MIC_DEVICE *mic_dev;

  mi_conn = miReq->mi_conn;
  micIcfgDev = miReq->user_info[0];
  mic_dev = miReq->mic_device;

  SLOGALWAYS1 ("DeviceSelect for device %s completed", mic_dev->device_name);
  QPRINTF ("\nDeviceSelect for device %s completed", mic_dev->device_name);
  _micuShowDeviceResult (miReq->result);

  if (miReq->result == MIC_RESULT_SUCCESS)
    {
    SLOGCALWAYS1 ("CheckBack ID = %d", miReq->o.check_back_id);
    QPRINTF ("\nCheckBack ID = %d", miReq->o.check_back_id);
    }

  mi_free_req_ctrl (miReq);
  _micuOperate (mi_conn, micIcfgDev);
  }

/************************************************************************/
/************************************************************************/
/*			_micuOperate 					*/
/************************************************************************/

static ST_VOID _micuOperate (MI_CONN *mi_conn, MICU_DEV *micIcfgDev)
  {
MIC_DEVICE *mic_dev;
MI_DEV_VALUE_UNION devValue;
MI_REQ_CTRL *miReq;
static MI_CONTROL_COMMAND devCmdValue = 0;
static MI_CONTROL_SETPOINT_REAL devRealValue = 0.0;
static MI_CONTROL_SETPOINT_DISCRETE devDiscreteValue = 0;

  mic_dev = micIcfgDev->mic_dev;
  switch (micIcfgDev->dev_type)
    {
    case MI_TYPEID_DEV_COMMAND:
      devValue.cmd = ++devCmdValue;
    break;

    case MI_TYPEID_DEV_REAL:
      devRealValue += (MI_CONTROL_SETPOINT_REAL) 1.1;
      devValue.set_pt_r = devRealValue;
    break;

    case MI_TYPEID_DEV_DISCRETE:
      devValue.set_pt_d = ++devDiscreteValue;
    break;
    }

  SLOGALWAYS1 ("Operating device %s", mic_dev->device_name);
  QPRINTF ("\nOperating device %s", mic_dev->device_name);
  miReq = mic_device_operate (mi_conn, mic_dev, &devValue);
  if (!miReq)
    {
    SLOGCALWAYS0 ("mic_device_operate failed.");
    ++numDevTestErr;
    }
  else
    {
    miReq->u_req_done = _micuOperateDone;
    miReq->user_info[0] = micIcfgDev;
    }
  }

/************************************************************************/
/*			_micuOperateDone 				*/
/************************************************************************/

static ST_VOID _micuOperateDone (MI_REQ_CTRL *miReq)
  {
MIU_REMOTE *miuRemote;
MICU_DEV *micIcfgDev;
MI_CONN *mi_conn;
MIC_DEVICE *mic_dev;

  mi_conn = miReq->mi_conn;
  micIcfgDev = miReq->user_info[0];
  mic_dev = miReq->mic_device;

  SLOGALWAYS1 ("DeviceOperate for device %s completed", mic_dev->device_name);
  QPRINTF ("\nDeviceOperate for device %s completed", mic_dev->device_name);
  _micuShowDeviceResult (miReq->result);

  mi_free_req_ctrl (miReq);

/* If this device is taggable, let's tag it */
  if (mic_dev->device_attrib & MIC_DEVICE_TAGGABLE)
    _micuSetTag (mi_conn, micIcfgDev);
  else /* not taggable, we are done with this one ... */
    {
    miuRemote = miuFindMiRemote (mi_conn->mi_remote);
    _micuTestNextDevice (mi_conn, miuRemote, micIcfgDev);
    }
  }

/************************************************************************/
/************************************************************************/
/*			_micuSetTag 					*/
/************************************************************************/

static ST_VOID _micuSetTag (MI_CONN *mi_conn, MICU_DEV *micIcfgDev)
  {
MIC_DEVICE *mic_dev;
MI_REQ_CTRL *miReq;
MI_TAG_VALUE tagValue;

  mic_dev = micIcfgDev->mic_dev;
  SLOGALWAYS1 ("Tagging device %s", mic_dev->device_name);
  QPRINTF ("\nTagging device %s", mic_dev->device_name);
  tagValue.TagFlags = MI_TAG_FLAG_NO_TAG;
  strcpy (tagValue.Reason, "No Tag!");
  miReq = mic_device_settag (mi_conn, mic_dev, &tagValue);
  if (!miReq)
    {
    SLOGCALWAYS1 ("mic_device_settag failed (0x%04x)", mi_op_err);
    ++numDevTestErr;
    }
  else
    {
    miReq->u_req_done = _micuSetTagDone;
    miReq->user_info[0] = micIcfgDev;
    }
  }

/************************************************************************/
/*			_micuSetTagDone  				*/
/************************************************************************/

static ST_VOID _micuSetTagDone  (MI_REQ_CTRL *miReq)
  {
MICU_DEV *micIcfgDev;
MI_CONN *mi_conn;
MIC_DEVICE *mic_dev;

  mi_conn = miReq->mi_conn;
  micIcfgDev = miReq->user_info[0];
  mic_dev = miReq->mic_device;

  SLOGALWAYS1 ("DeviceSetTag for device %s completed", mic_dev->device_name);
  QPRINTF ("\nDeviceSetTag for device %s completed", mic_dev->device_name);

  _micuShowDeviceResult (miReq->result);

  mi_free_req_ctrl (miReq);

  /* now try to get the tag		*/
  _micuGetTag (mi_conn, micIcfgDev);
  }

/************************************************************************/
/************************************************************************/
/*			_micuGetTag 					*/
/************************************************************************/

static ST_VOID _micuGetTag (MI_CONN *mi_conn, MICU_DEV *micIcfgDev)
  {
MIC_DEVICE *mic_dev;
MI_REQ_CTRL *miReq;

  mic_dev = micIcfgDev->mic_dev;
  SLOGALWAYS1 ("Getting device %s tag", mic_dev->device_name);
  QPRINTF ("\nGetting device %s tag", mic_dev->device_name);
  miReq = mic_device_gettag (mi_conn, mic_dev);
  if (!miReq)
    {
    SLOGCALWAYS0 ("mic_device_gettag failed");
    ++numDevTestErr;
    }
  else
    {
    miReq->u_req_done = _micuGetTagDone;
    miReq->user_info[0] = micIcfgDev;
    }
  }

/************************************************************************/
/*			_micuGetTagDone  				*/
/************************************************************************/

static ST_VOID _micuGetTagDone  (MI_REQ_CTRL *miReq)
  {
MIU_REMOTE *miuRemote;
MICU_DEV *micIcfgDev;
MI_CONN *mi_conn;
MIC_DEVICE *mic_dev;

  mi_conn = miReq->mi_conn;
  micIcfgDev = miReq->user_info[0];
  mic_dev = miReq->mic_device;

  SLOGALWAYS1 ("DeviceGetTag complete for device '%s'", mic_dev->device_name);
  QPRINTF ("\nDeviceGetTag complete for device '%s'", mic_dev->device_name);
  _micuShowDeviceResult (miReq->result);
  if (miReq->result == MIC_RESULT_SUCCESS)
    {
    SLOGCALWAYS1 ("  TagFlags = 0x%x", miReq->o.tag_value->TagFlags);
    SLOGCALWAYS1 ("  Reason = '%s'", miReq->o.tag_value->Reason);
    QPRINTF ("\n  TagFlags = 0x%x", miReq->o.tag_value->TagFlags);
    QPRINTF ("\n  Reason = '%s'", miReq->o.tag_value->Reason);
    }
  mi_free_req_ctrl (miReq);

  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  _micuTestNextDevice (mi_conn, miuRemote, micIcfgDev);
  }

/************************************************************************/
/*			_micuShowDeviceResult 				*/
/************************************************************************/

static ST_VOID _micuShowDeviceResult (ST_RET mic_result)
  {
   switch (mic_result)
    {
    case MIC_RESULT_SUCCESS:
      SLOGCALWAYS0 ("  SUCCESS");
      QPRINTF ("  SUCCESS");
    break;

    case MIC_RESULT_OBJ_NON_EXISTENT:
      SLOGCALWAYS0 ("  Device does not exist.");
      QPRINTF ("  Device does not exist.");
      ++numDevTestErr;
    break;

    case MIC_RESULT_ACCESS_DENIED:
      SLOGCALWAYS0 ("  Access Denied.");
      QPRINTF ("  Access Denied.");
      ++numDevTestErr;
    break;

    case MIC_RESULT_HARDWARE_FAULT:
      SLOGCALWAYS0 ("  Hardware Fault");
      QPRINTF ("  Hardware Fault");
      ++numDevTestErr;
    break;

    case MIC_RESULT_TEMP_UNAVAIL:
      SLOGCALWAYS0 ("  Temporarily Unavailable.");
      QPRINTF ("  Temporarily Unavailable.");
      ++numDevTestErr;
    break;

    default:
      SLOGCALWAYS1 ("  Undefined error 0x%04x", mic_result);
      QPRINTF ("  Undefined error 0x%04x", mic_result);
      ++numDevTestErr;
    break;
    }
  }

/************************************************************************/
/************************************************************************/
/*			micuGetOneDvAttrib				*/
/************************************************************************/

ST_VOID micuGetOneDvAttrib (MI_CONN *mi_conn)
  {
MIU_REMOTE *miuRemote;
MI_REQ_CTRL *miReq;
MICU_DV *micIcfgDv;
ST_RET rc;

  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  micIcfgDv = miuRemote->dv_list;
  if (micIcfgDv == NULL)
    return;

  rc = micuSendGetDvAttrib (mi_conn, micIcfgDv, &miReq);
  if (rc == SD_SUCCESS)
    miReq->user_info[2] = (ST_VOID *) 0;
  }

/************************************************************************/
/*			micuGetAllDvAttrib	 			*/
/************************************************************************/

ST_VOID micuGetAllDvAttrib (MI_CONN *mi_conn)
  {
MIU_REMOTE *miuRemote;
MI_REQ_CTRL *miReq;
MICU_DV *micIcfgDv;

  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  micIcfgDv = miuRemote->dv_list;
  if (micIcfgDv == NULL)
    return;

  micuSendGetDvAttrib (mi_conn, micIcfgDv, &miReq);
  }

/************************************************************************/
/*			micuSendGetDvAttrib				*/
/************************************************************************/

static ST_RET micuSendGetDvAttrib (MI_CONN *mi_conn, MICU_DV *micIcfgDv, MI_REQ_CTRL **miReqOut)
  {
MIU_REMOTE *miuRemote;
MI_REQ_CTRL *miReq;
ST_INT scope;

  miuRemote = miuFindMiRemote (mi_conn->mi_remote);
  if (micIcfgDv->mic_dv->dv_attrib & MIC_DV_SCOPE_ICC)
    scope = ICC_SPEC;
  else
    scope = VCC_SPEC;

  miReq = mic_get_dv_attrib (mi_conn, micIcfgDv->mic_dv->dv_name, scope);
  if (!miReq)
    {
    printf ("\n Error! Could not get DV attrib");
    return (SD_FAILURE);
    }
  miReq->u_req_done =  _micuGetDvAttribDone;
  miReq->user_info[0] = (ST_VOID *) miuRemote;
  miReq->user_info[1] = (ST_VOID *) micIcfgDv;
  miReq->user_info[2] = (ST_VOID *) 1;
  return (SD_SUCCESS);
  }

/************************************************************************/
/*			_micuGetAllDvAttribDone 			*/
/************************************************************************/

static ST_VOID _micuGetDvAttribDone (MI_REQ_CTRL *miReq)
  {
MI_CONN *mi_conn;
MIU_REMOTE *miuRemote;
MICU_DV *micIcfgDv;
ST_BOOLEAN doNext;

  mi_conn = miReq->mi_conn;
  miuRemote = (MIU_REMOTE *) miReq->user_info[0];
  micIcfgDv = (MICU_DV *)    miReq->user_info[1];

  if (miReq->result == SD_SUCCESS)
    printf ("\nGet DV Attrib for '%s' OK : %s", micIcfgDv->mic_dv->dv_name, mi_type_to_string (miReq->o.mi_type));
  else
    printf ("\nGet DV Attrib for '%s' Error", micIcfgDv->mic_dv->dv_name);

  if (miReq->user_info[2] != NULL)
    doNext = SD_TRUE;
  else
    doNext = SD_FALSE;

  mi_free_req_ctrl (miReq);

  if (!doNext)
    return;

  micIcfgDv = list_get_next (miuRemote->dv_list, micIcfgDv);
  if (micIcfgDv == NULL)
    {
    printf ("\nAll done");
    return;
    }
  micuSendGetDvAttrib (mi_conn, micIcfgDv, &miReq);
  }

/************************************************************************/
/*		      Block 8 Operations				*/
/************************************************************************/
/*		      _micuStartTATransfers				*/
/************************************************************************/

static ST_VOID _micuStartTATransfers (MI_CONN *mi_conn)
  {
MI_REQ_CTRL *miReq;

  printf ("\n Starting TATransfers for remote '%s'", mi_conn->remote_ar);
  miReq = mic_start_transfer_accounts (mi_conn, TA_COND_BeforeTheHour);
  if (miReq != NULL)
    miReq->u_req_done = _micuStartTATransferDone;
  else
    printf (" : Failed!");

  mi_set_compatability_mode (mi_conn->mi_remote, 0);
/*  mi_set_compatability_mode (mi_conn->mi_remote, MI_B8_MODE_SISCO_ICCP_TOOLKIT);*/
  }

/************************************************************************/
/*		      _micuStartTATransferDone				*/
/************************************************************************/

static ST_VOID _micuStartTATransferDone (MI_REQ_CTRL *req)
  {
MI_CONN *mi_conn;
MVL_REQ_PEND *mvlReqPend;

  mvlReqPend = req->mvl_req;
  mi_conn = mvlReqPend->net_info->mi_conn; 
  printf ("\n START TATransfer result = 0x%x", req->result);
  mi_free_req_ctrl (req);
  }

/************************************************************************/
/*		      _micuStopTATransfers				*/
/************************************************************************/

static ST_VOID _micuStopTATransfers (MI_CONN *mi_conn)
  {
MI_REQ_CTRL *miReq;

  printf ("\n Stopping TATransfers for remote '%s'", mi_conn->remote_ar);
  miReq = mic_stop_transfer_accounts (mi_conn);
  if (miReq)
    miReq->u_req_done = _micuStopTATransfersDone;
  else
    printf ("\n Failed!!!");
  }


/************************************************************************/
/*		      _micuStopTATransfersDone				*/
/************************************************************************/

static ST_VOID _micuStopTATransfersDone (MI_REQ_CTRL *req)
  {
MI_CONN *mi_conn;
MVL_REQ_PEND *mvlReqPend;

  mvlReqPend = req->mvl_req;
  mi_conn = mvlReqPend->net_info->mi_conn;
  printf ("\n STOP TATransfer for '%s' Result = 0x%x", 
	  mi_conn->remote_ar, req->result);
  mi_free_req_ctrl (req);
  }

/************************************************************************/
/*                    _micuDeleteAllDataSets				*/
/************************************************************************/

static ST_RET  _micuDeleteAllDataSets (MI_CONN *mi_conn)
  {
MI_REQ_CTRL *miReq;

  printf ("\n Deleting All DataSets for Remote '%s'", mi_conn->remote_ar);

  miReq = mic_delete_all_ds (mi_conn);
  if (!miReq)
    {
    printf ("\n Failed!!!");
    return SD_FAILURE;
    }

  miReq->u_req_done = _micuDeleteAllDataSetsDone;
  return SD_SUCCESS;
  }

/************************************************************************/
/*		      _micuDeelteAllDataSetsDone			*/
/************************************************************************/

static ST_VOID _micuDeleteAllDataSetsDone (MI_REQ_CTRL *req)
  {
MI_CONN *mi_conn;
MVL_REQ_PEND *mvlReqPend;

  mvlReqPend = req->mvl_req;
  mi_conn = mvlReqPend->net_info->mi_conn;
  printf ("\n DeleteAllDataSets for '%s' Result = 0x%x", 
	  mi_conn->remote_ar, req->result);

  mi_free_req_ctrl (req);

  miuNeedDelAllDs = SD_FALSE;
  _micuStartAutoDsts (mi_conn);
  }
#endif	// CHOIBC DELETE



/************************************************************************/
/*			u_mic_receive_transfer_account			*/
/************************************************************************/

ST_VOID u_mic_receive_transfer_account (MI_CONN *mi_conn, MI_TRANSFER_ACCOUNT *tran_acct)
  {
#if 0	// CHOIBC DELETE
MI_GEN_DATA_RESP  genDataResp;
MI_GEN_DATA_RPT  *genDataRept;

  SLOGALWAYS1 ("Incoming Transfer Account Report from remote %s", mi_conn->mi_remote->name);
  switch (tran_acct->type)
    {
    case MI_TA_TYPE_REPORT:	      slogTAReport (&tran_acct->acct.taReport);	    break;
    case MI_TA_TYPE_DEVICE_OUTAGE:    slogDevOut (&tran_acct->acct.deviceOutage);   break;
    case MI_TA_TYPE_AVAILABILITY:     slogAvail (&tran_acct->acct.availability);    break;
    case MI_TA_TYPE_REAL_TIME_STATUS: slogStatus (&tran_acct->acct.realTimeStatus); break;
    case MI_TA_TYPE_FORECAST:	      slogForecast (&tran_acct->acct.forecast);	    break;
    case MI_TA_TYPE_CURVE:	      slogCurve (&tran_acct->acct.curve);	    break;
    case MI_TA_TYPE_GEN_DATA:	      
      slogGenData (&tran_acct->acct.generalData);
      genDataRept = &tran_acct->acct.generalData;
      strcpy (genDataResp.ReportName, genDataRept->ReportName);
      genDataResp.ReportReferenceNumber = genDataRept->GeneralDataReportReferenceNumber;
      genDataResp.ReportTimeStamp = genDataRept->ReportDateAndTime;
      genDataResp.ResponseCode = MI_RESP_CODE_APPROVED;
      genDataResp.ResponseData = 123;
      strcpy (genDataResp.ResponseText, "General Data Response Text");
      genDataResp.NumberOfLocalReference = genDataRept->LocalRef.NumberOfLocalReference;  /* not required by spec but easy for testing */
      genDataResp.ListOfLocalReference = genDataRept->LocalRef.ListOfLocalReference;
      mic_send_general_data_resp (mi_conn, &genDataResp);
    break;
    }
#endif	// CHOIBC DELETE
  }

#if 0	// CHOIBC DELETE
/************************************************************************/
/*			    slogTAReport				*/
/************************************************************************/

static ST_VOID slogTAReport (MI_TA_REPORT *taReport)
  {
time_t tt;

  if (taReport->DataType == TA_ACCT_DATA_TYPE_PERIODIC)
    {
    if (taReport->TransmissionSegmentIncluded)
      SLOGCALWAYS1 ("%s Received", MI_TA_SEG_PERIODIC_VAR_NAME);
    else
      SLOGCALWAYS1 ("%s Received", MI_TA_NO_SEG_PERIODIC_VAR_NAME);
    }
  else
    {
    if (taReport->TransmissionSegmentIncluded)
      SLOGCALWAYS1 ("%s Received", MI_TA_SEG_PROFILE_VAR_NAME);
    else
      SLOGCALWAYS1 ("%s Received", MI_TA_NO_SEG_PROFILE_VAR_NAME);
    }

  SLOGCALWAYS1 ("   TransferAccountReference:  %d", taReport->TransferAccountReference);
  SLOGCALWAYS1 ("                SendUtility:  %d", taReport->SendUtility);
  SLOGCALWAYS1 ("             ReceiveUtility:  %d", taReport->ReceiveUtility);
  SLOGCALWAYS1 ("              SelleingAgent:  %d", taReport->SellingAgent);
  SLOGCALWAYS1 ("                BuyingAgent:  %d", taReport->BuyingAgent);
  tt = (time_t) taReport->TimeStamp;
  SLOGCALWAYS1 ("                  TimeStamp:  %s", ctime (&tt));
  SLOGCALWAYS1 ("            TransactionCode:  %d", taReport->TransactionCode);
  slogLocalRef (&taReport->LocalRef);
  SLOGCALWAYS1 ("                       Name:  %s", taReport->Name);
  SLOGCALWAYS1 ("TransmissionSegmentIncluded:  %d", taReport->TransmissionSegmentIncluded);
  if (taReport->DataType == TA_ACCT_DATA_TYPE_PERIODIC)
    {
    if (taReport->NumberofTransSegments > 0)
      slogPeriodicSegs (taReport);
    
    slogPeriodicData (&taReport->Data.Periodic);
    }
  else
    {
    if (taReport->NumberofTransSegments > 0)
      slogProfileSegs (taReport);

    slogProfileData (&taReport->Data.Profile);
    }
  }

/************************************************************************/
/*			    slogDevOut					*/
/************************************************************************/

static ST_VOID slogDevOut (MI_DEVICE_OUTAGE *devOut)
  {
time_t tt;
ST_CHAR rptType[128];

  switch (devOut->Activity)
    {
    case MI_DO_ACTIVITY_NEWPLAN:  strcpy (rptType, "NewPlan");	  break;
    case MI_DO_ACTIVITY_REVISE:	  strcpy (rptType, "Revised");	  break;
    case MI_DO_ACTIVITY_CANCEL:	  strcpy (rptType, "Cancel");	  break;
    case MI_DO_ACTIVITY_ACTUAL:	  strcpy (rptType, "Actual");	  break;
    default:			  strcpy (rptType, "NONE!");	  break;
    }

  SLOGCALWAYS1 ("Device Outage %s Report Received", rptType);
  SLOGCALWAYS1 ("          OutageReferenceId:  %d", devOut->OutageReferenceId);
  SLOGCALWAYS1 ("            OwningUtilityID:  %d", devOut->OwningUtilityID);
  tt = (time_t) devOut->TimeStamp;
  SLOGCALWAYS1 ("                  TimeStamp:  %s", ctime (&tt));
  SLOGCALWAYS1 ("                StationName:  %s", devOut->StationName);
  SLOGCALWAYS1 ("                 DeviceType:  %d", devOut->DeviceType);
  SLOGCALWAYS1 ("                 DeviceName:  %s", devOut->DeviceName);
  SLOGCALWAYS1 ("               DeviceNumber:  %d", devOut->DeviceNumber);
  SLOGCALWAYS1 ("               DeviceRating:  %f", devOut->DeviceRating);
  tt = (time_t) devOut->ActivityDateAndTime;
  SLOGCALWAYS1 ("        ActivityDataAndTime:  %s", ctime(&tt));
  SLOGCALWAYS1 ("                   Comments:  %s", devOut->Comments);
  SLOGCALWAYS1 ("               OutageEffect:  %s", devOut->OutageEffect);
  SLOGCALWAYS1 ("                   Activity:  %d", devOut->Activity);

  switch (devOut->Activity)
    {
    case MI_DO_ACTIVITY_NEWPLAN:
    case MI_DO_ACTIVITY_REVISE:
      SLOGCALWAYS0 ("        New or Revised Data:");
      SLOGCALWAYS1 ("                   PlanType:  %d", devOut->ActivityData.NewRevise.PlanType);
      tt = (time_t) devOut->ActivityData.NewRevise.PlannedOpenOrOutOfServiceDateAndTime;
      SLOGCALWAYS1 (" PlanOpenOrOutOfServiceDate:  %s", ctime (&tt));
      tt = (time_t) devOut->ActivityData.NewRevise.PlannedCloseOrInServiceDateAndTime;
      SLOGCALWAYS1 ("PlannedCloseOrInServiceDate:  %s", ctime (&tt));
      SLOGCALWAYS1 ("               OutagePeriod:  %d", devOut->ActivityData.NewRevise.OutagePeriod);
      SLOGCALWAYS1 ("                 OutageType:  %d", devOut->ActivityData.NewRevise.OutageType);
      SLOGCALWAYS1 ("           OutageAmountType:  %d", devOut->ActivityData.NewRevise.OutageAmountType);
      if (devOut->ActivityData.NewRevise.OutageAmountType == MI_DO_OUTAGE_AMT_TYPE_PARTIAL)
	{
	SLOGCALWAYS1 ("             Partial Amount:  %f", devOut->ActivityData.NewRevise.Partial.Amount);
	SLOGCALWAYS1 ("PartialUpperOpreratingLimit:  %f", devOut->ActivityData.NewRevise.Partial.UpperOpreratingLimit);
	SLOGCALWAYS1 ("PartialLowerOpreratingLimit:  %f", devOut->ActivityData.NewRevise.Partial.LowerOperatingLimit);
	}
      SLOGCALWAYS1 ("                      Class:  %d", devOut->ActivityData.NewRevise.Class);
    break;

    case MI_DO_ACTIVITY_ACTUAL:
      SLOGCALWAYS0 ("                Actual Data:");
      SLOGCALWAYS1 ("                     Action:  %d", devOut->ActivityData.Actual.Action);
      SLOGCALWAYS1 ("             AffectedAmount:  %f", devOut->ActivityData.Actual.AffectedAmount);
    break;
    }
  }

/************************************************************************/
/*			    slogAvail					*/
/************************************************************************/

static ST_VOID slogAvail (MI_AVAILABILITY *avail)
  {
time_t tt;

  SLOGCALWAYS0 ("Availability Report Received");
  SLOGCALWAYS1 ("    AvailabilityReferenceID:  %d", avail->AvailabilityReferenceID);
  tt = (time_t) avail->TimeStamp;
  SLOGCALWAYS1 ("                  TimeStamp:  %s", ctime (&tt));
  SLOGCALWAYS1 ("           PlantReferenceID:  %d", avail->PlantReferenceID);
  SLOGCALWAYS1 ("                     UnitID:  %d", avail->UnitID);
  SLOGCALWAYS1 ("               ReportStatus:  %d", avail->ReportStatus);
  tt = (time_t) avail->StartDateAndTime;
  SLOGCALWAYS1 ("           StartDateAndTime:  %s", ctime (&tt));
  tt = (time_t) avail->StopDateAndTime;
  SLOGCALWAYS1 ("            StopDateAndTime:  %s", ctime (&tt));
  SLOGCALWAYS1 ("                   Duration:  %d", avail->Duration);
  SLOGCALWAYS1 ("           ProvidingReserve:  %d", avail->ProvidingReserve);
  SLOGCALWAYS1 ("         AvailabilityStatus:  %d", avail->AvailabilityStatus);

  if (avail->AvailabilityStatus == MI_AVAIL_STATUS_AVAILABLE)
    {
    SLOGCALWAYS0 ("             Available Data:");
    SLOGCALWAYS1 ("             EconomicImpact:  %d", avail->AvailabilityData.Available.EconomicImpact);
    SLOGCALWAYS1 ("                PriceImpact:  %f", avail->AvailabilityData.Available.PriceImpact);
    SLOGCALWAYS1 ("             RampRateImpact:  %d", avail->AvailabilityData.Available.RampRateImpact);
    if (avail->AvailabilityData.Available.RampRateImpact)
      {
      SLOGCALWAYS1 ("              MaxRampRateUp:  %f", avail->AvailabilityData.Available.RampRates.MaxRampRateUp);
      SLOGCALWAYS1 ("            MaxRampRateDown:  %f", avail->AvailabilityData.Available.RampRates.MaxRampRateDown);
      }

    SLOGCALWAYS1 ("             CapacityImpact:  %d", avail->AvailabilityData.Available.CapacityImpact);
    if (avail->AvailabilityData.Available.CapacityImpact)
      {
      SLOGCALWAYS1 ("               UnitCapacity:  %d", avail->AvailabilityData.Available.UnitCapacity);
      switch (avail->AvailabilityData.Available.UnitCapacity)
	{
	case MI_UNIT_CAPACITY_GROSS:
	  SLOGCALWAYS1 ("           GrossMaxCapacity:  %f", avail->AvailabilityData.Available.Capacity.Gross.GrossMaxCapacity);
	  SLOGCALWAYS1 ("           GrossMinCapacity:  %f", avail->AvailabilityData.Available.Capacity.Gross.GrossMinCapacity);
	break;

	case MI_UNIT_CAPACITY_NET:
	  SLOGCALWAYS1 ("             NetMaxCapacity:  %f", avail->AvailabilityData.Available.Capacity.Net.NetMaxCapacity);
	  SLOGCALWAYS1 ("             NetMinCapacity:  %f", avail->AvailabilityData.Available.Capacity.Net.NetMinCapacity);
	break;

	case MI_UNIT_CAPACITY_BOTH:
	  SLOGCALWAYS1 ("           GrossMaxCapacity:  %f", avail->AvailabilityData.Available.Capacity.Both.GrossMaxCapacity);
	  SLOGCALWAYS1 ("           GrossMinCapacity:  %f", avail->AvailabilityData.Available.Capacity.Both.GrossMinCapacity);
	  SLOGCALWAYS1 ("             NetMaxCapacity:  %f", avail->AvailabilityData.Available.Capacity.Both.NetMaxCapacity);
	  SLOGCALWAYS1 ("             NetMinCapacity:  %f", avail->AvailabilityData.Available.Capacity.Both.NetMinCapacity);
	break;
	}
      }

    SLOGCALWAYS1 ("                        LFC:  %d", avail->AvailabilityData.Available.LFC);
    SLOGCALWAYS1 ("         TypeOfAvailability:  %d", avail->AvailabilityData.Available.TypeOfAvailability);
    if (avail->AvailabilityData.Available.TypeOfAvailability == MI_AVAIL_TYPE_STANDBY)
      {
      tt = (time_t) avail->AvailabilityData.Available.AvailData.TimeToOnline;
      SLOGCALWAYS1 ("               TimeToOnline:  %s", ctime (&tt));
      }
    else if (avail->AvailabilityData.Available.TypeOfAvailability == MI_AVAIL_TYPE_ONLINE)
      {
      if (avail->AvailabilityData.Available.LFC)
	{
	SLOGCALWAYS1 ("               Dispatchable:  %d", avail->AvailabilityData.Available.AvailData.LFC_yes.Dispatchable);
	SLOGCALWAYS1 ("                 Regulating:  %d", avail->AvailabilityData.Available.AvailData.LFC_yes.Regulating);
	SLOGCALWAYS1 ("            Manually_Loaded:  %d", avail->AvailabilityData.Available.AvailData.LFC_yes.Manually_Loaded);
	}
      else
	{
	SLOGCALWAYS1 ("             ReasonForNoLFC:  %d", avail->AvailabilityData.Available.AvailData.ReasonForNoLFC);
	}
      }
    SLOGCALWAYS1 ("                    Comment:  %s", avail->AvailabilityData.Available.Comment);
    }
  else if (avail->AvailabilityStatus == MI_AVAIL_STATUS_UNAVAILABLE)
    {
    SLOGCALWAYS1 ("       ReasonForUnavailable:  %d", avail->AvailabilityData.Unavailable.ReasonForUnavailable);
    SLOGCALWAYS1 ("                    Comment:  %s", avail->AvailabilityData.Unavailable.Comment);
    }
  }

/************************************************************************/
/*			    slogStatus					*/
/************************************************************************/

static ST_VOID slogStatus (MI_REAL_TIME_STATUS *status)
  {
time_t tt;

  SLOGCALWAYS0 ("Real Time Status Report Received");
  SLOGCALWAYS1 ("  RealTimeStatusReferenceID:  %d", status->RealTimeStatusReferenceID);
  tt = (time_t) status->TimeStamp;
  SLOGCALWAYS1 ("                  TimeStamp:  %s", ctime(&tt));
  SLOGCALWAYS1 ("           PlantReferenceID:  %d", status->PlantReferenceID);
  SLOGCALWAYS1 ("                     UnitID:  %d", status->UnitID);
  SLOGCALWAYS1 ("         AvailabilityStatus:  %d", status->AvailabilityStatus);

  if (status->AvailabilityStatus == MI_AVAIL_STATUS_AVAILABLE)
    {
    SLOGCALWAYS1 ("              MaxRampRateUp:  %f", status->AvailData.Available.MaxRampRateUp);
    SLOGCALWAYS1 ("            MaxRampRateDown:  %f", status->AvailData.Available.MaxRampRateDown);
    SLOGCALWAYS1 ("               UnitCapacity:  %d", status->AvailData.Available.UnitCapacity);
    switch (status->AvailData.Available.UnitCapacity)
      {
      case MI_UNIT_CAPACITY_GROSS:
        SLOGCALWAYS1 ("           GrossMaxCapacity:  %f", status->AvailData.Available.Capacity.Gross.GrossMaxCapacity);
        SLOGCALWAYS1 ("           GrossMinCapacity:  %f", status->AvailData.Available.Capacity.Gross.GrossMinCapacity);
      break;

      case MI_UNIT_CAPACITY_NET:
        SLOGCALWAYS1 ("             NetMaxCapacity:  %f", status->AvailData.Available.Capacity.Net.NetMaxCapacity);
        SLOGCALWAYS1 ("             NetMinCapacity:  %f", status->AvailData.Available.Capacity.Net.NetMinCapacity);
      break;

      case MI_UNIT_CAPACITY_BOTH:
        SLOGCALWAYS1 ("           GrossMaxCapacity:  %f", status->AvailData.Available.Capacity.Both.GrossMaxCapacity);
        SLOGCALWAYS1 ("           GrossMinCapacity:  %f", status->AvailData.Available.Capacity.Both.GrossMinCapacity);
        SLOGCALWAYS1 ("             NetMaxCapacity:  %f", status->AvailData.Available.Capacity.Both.NetMaxCapacity);
        SLOGCALWAYS1 ("             NetMinCapacity:  %f", status->AvailData.Available.Capacity.Both.NetMinCapacity);
      break;
      }

    SLOGCALWAYS1 ("         TypeOfAvailability:  %d", status->AvailData.Available.TypeOfAvailability);
    SLOGCALWAYS1 ("                        LFC:  %d", status->AvailData.Available.LFC);
    if (status->AvailData.Available.TypeOfAvailability == MI_AVAIL_TYPE_STANDBY)
      {
      tt = (time_t) status->AvailData.Available.AvailData.TimeToOnline;
      SLOGCALWAYS1 ("               TimeToOnline:  %s", ctime (&tt));
      }
    else if (status->AvailData.Available.TypeOfAvailability == MI_AVAIL_TYPE_ONLINE)
      {
      if (status->AvailData.Available.LFC)
	{
	SLOGCALWAYS1 ("               Dispatchable:  %d", status->AvailData.Available.AvailData.LFC_yes.Dispatchable);
	SLOGCALWAYS1 ("                 Regulating:  %d", status->AvailData.Available.AvailData.LFC_yes.Regulating);
	SLOGCALWAYS1 ("            Manually_Loaded:  %d", status->AvailData.Available.AvailData.LFC_yes.Manually_Loaded);
	}
      else
	{
	SLOGCALWAYS1 ("             ReasonForNoLFC:  %d", status->AvailData.Available.AvailData.ReasonForNoLFC);
	}
      }

    SLOGCALWAYS1 ("      ExternallyBlockedHigh:  %d", status->AvailData.Available.ExternallyBlockedHigh);
    SLOGCALWAYS1 ("       ExternallyBlockedLow:  %d", status->AvailData.Available.ExternallyBlockedLow);
    }
  else if (status->AvailabilityStatus == MI_AVAIL_STATUS_UNAVAILABLE)
    {
    SLOGCALWAYS1 ("       ReasonForUnavailable:  %d", status->AvailData.ReasonForUnavailable);
    }

  SLOGCALWAYS1 ("           ProvidingReserve:  %d", status->ProvidingReserve);
  }

/************************************************************************/
/*			    slogForecast				*/
/************************************************************************/

static ST_VOID slogForecast (MI_FORCAST_SCHEDULE *forecast)
  {
time_t tt;

  SLOGCALWAYS0 ("Forecast Report Received");
  SLOGCALWAYS1 ("ForecastScheduleReferenceID:  %d", forecast->ForecastScheduleReferenceID);
  SLOGCALWAYS1 ("           PlantReferenceID:  %d", forecast->PlantReferenceID);
  SLOGCALWAYS1 ("                     UnitID:  %d", forecast->UnitID);
  SLOGCALWAYS1 ("               ForecastType:  %d", forecast->ForecastType);
  tt = (time_t) forecast->StartTime;
  SLOGCALWAYS1 ("                  StartTime:  %s", ctime(&tt));
  SLOGCALWAYS1 ("           PeriodResolution:  %d", forecast->PeriodResolution);
  SLOGCALWAYS1 ("            NumberOfPeriods:  %d", forecast->NumberOfPeriods);
  SLOGCALWAYS0 ("            ListOfForecasts: ");

  slogFloatMatrix (&forecast->ListOfForecasts.MW);
  slogIntMatrix (&forecast->ListOfForecasts.LFC_Code);
  }

/************************************************************************/
/*			    slogCurve					*/
/************************************************************************/

static ST_VOID slogCurve (MI_CURVE *curve)
  {
int i, j;
MI_CURVE_SEGMENT_DESCR *seg;

  SLOGCALWAYS0 ("Curve Report Received");
  SLOGCALWAYS1 ("                  CurveName:  %s", curve->CurveName);
  SLOGCALWAYS1 ("           PlantReferenceID:  %d", curve->PlantReferenceID);
  SLOGCALWAYS1 ("                     UnitID:  %d", curve->UnitID);
  SLOGALWAYS0  ("                  CurveType: ");
  SLOGALWAYSH  (4, curve->CurveType);
  SLOGCALWAYS1 ("           NumberOfSegments:  %d", curve->NumberOfSegments);
  SLOGCALWAYS0 ("             Curve Segments:");
  for (i = 0; i < curve->NumberOfSegments; i++)
    {
    SLOGCALWAYS1 ("           Curve Segment %d:", i);
    seg = &curve->SequenceOfCurveSegmentDescription[i];
    SLOGCALWAYS1 ("                      Order:  %d", seg->Order);
    SLOGCALWAYS1 ("                   LowRange:  %f", seg->LowRange);
    SLOGCALWAYS1 ("                  HighRange:  %f", seg->HighRange);
    SLOGCALWAYS0 ("     SequenceOfCoefficients:");
    for (j = 0; j < seg->Order; j++)
      {
      SLOGCALWAYS2 ("                        %d.:  %f", i, seg->SequenceOfCoefficients[j]);
      }
    }
  }

/************************************************************************/
/*			    slogGenData					*/
/************************************************************************/

static ST_VOID slogGenData (MI_GEN_DATA_RPT *genData)
  {
time_t tt;

  SLOGCALWAYS0 ("General Data Report Received");
  SLOGCALWAYS1 ("GeneralDataReportReferenceN:  %d", genData->GeneralDataReportReferenceNumber);
  SLOGCALWAYS1 ("                 ReportName:  %s", genData->ReportName);
  tt = (time_t) genData->ReportDateAndTime;
  SLOGCALWAYS1 ("          ReportDateAndTime:  %s", ctime (&tt));
  SLOGCALWAYS1 ("            TransactionCode:  %d", genData->TransactionCode);

  slogLocalRef (&genData->LocalRef);
  slogFloatMatrix (&genData->FloatingPoint1);
  slogFloatMatrix (&genData->FloatingPoint2);
  slogIntMatrix (&genData->IntegerValues1);
  slogIntMatrix (&genData->IntegerValues2);
  slogTextMatrix (&genData->Text1);
  slogTextMatrix (&genData->Text2);
  }

/************************************************************************/
/*			    slogLocalRef				*/
/************************************************************************/

static ST_VOID slogLocalRef (MI_LOCAL_REF *localRef)
  {
int i;

  SLOGCALWAYS1 ("     NumberOfLocalReference:  %d", localRef->NumberOfLocalReference);
  for (i = 0; i < localRef->NumberOfLocalReference; i++)
    {
    SLOGCALWAYS2 ("                     Ref %d:  %d", i, localRef->ListOfLocalReference[i]);
    }
  }

/************************************************************************/
/*			  slogPeriodicSegs				*/
/************************************************************************/

static ST_VOID slogPeriodicSegs (MI_TA_REPORT *taReport)
  {
int i;
MI_TA_SEGMENT *seg;

  SLOGCALWAYS1 ("      NumberofTransSegments:  %d", taReport->NumberofTransSegments);
  for (i = 0; i < taReport->NumberofTransSegments; i++)
    {
    seg = &taReport->ListOfTransmissionSegment[i];

    SLOGCALWAYS1 ("      TransmissionReference:  %d", seg->TransmissionReference);
    SLOGCALWAYS1 ("               UtilWheeling:  %d", seg->UtilWheeling);
    SLOGCALWAYS1 ("                 UtilPaying:  %d", seg->UtilPaying);
    slogSegUnion (seg);
    slogPeriodicData (&seg->Data.Periodic);
    }
  }

/************************************************************************/
/*			  slogProfileSegs				*/
/************************************************************************/

static ST_VOID slogProfileSegs (MI_TA_REPORT *taReport)
  {
int i;
MI_TA_SEGMENT *seg;

  SLOGCALWAYS1 ("      NumberofTransSegments:  %d", taReport->NumberofTransSegments);
  for (i = 0; i < taReport->NumberofTransSegments; i++)
    {
    seg = &taReport->ListOfTransmissionSegment[i];

    SLOGCALWAYS1 ("      TransmissionReference:  %d", seg->TransmissionReference);
    SLOGCALWAYS1 ("               UtilWheeling:  %d", seg->UtilWheeling);
    SLOGCALWAYS1 ("                 UtilPaying:  %d", seg->UtilPaying);
    slogSegUnion (seg);
    slogProfileData (&seg->Data.Profile);
    }
  }

/************************************************************************/
/*			  slogSegUnion					*/
/************************************************************************/

static ST_VOID slogSegUnion (MI_TA_SEGMENT *seg)
  {
  switch (seg->TransmissionSegType)
    {
    case TA_SEG_TYPE_INONLY:
      SLOGCALWAYS0 ("        TransmissionSegType:  InOnly");
      SLOGCALWAYS1 ("                     UtilIn:  %d", seg->TransmissionSegUnion.inOnly.UtilIn);
      SLOGCALWAYS1 ("            InterchangePtIn:  %d", seg->TransmissionSegUnion.inOnly.InterchangePtIn);
    break;

    case TA_SEG_TYPE_OUTONLY:
      SLOGCALWAYS0 ("        TransmissionSegType:  OutOnly");
      SLOGCALWAYS1 ("                    UtilOut:  %d", seg->TransmissionSegUnion.outOnly.UtilOut);
      SLOGCALWAYS1 ("           InterchangePtOut:  %d", seg->TransmissionSegUnion.outOnly.InterchangePtOut);
    break;

    case TA_SEG_TYPE_INOUT:
      SLOGCALWAYS0 ("        TransmissionSegType:  InOut");
      SLOGCALWAYS1 ("                     UtilIn:  %d", seg->TransmissionSegUnion.inOut.UtilIn);
      SLOGCALWAYS1 ("            InterchangePtIn:  %d", seg->TransmissionSegUnion.inOut.InterchangePtIn);
      SLOGCALWAYS1 ("                    UtilOut:  %d", seg->TransmissionSegUnion.inOut.UtilOut);
      SLOGCALWAYS1 ("           InterchangePtOut:  %d", seg->TransmissionSegUnion.inOut.InterchangePtOut);
    break;

    case TA_SEG_TYPE_DIRECT:
      SLOGCALWAYS0 ("        TransmissionSegType:  Direct");
      SLOGCALWAYS1 ("              InterchangePt:  %d", seg->TransmissionSegUnion.direct.InterchangePt);
    break;
    }
  }

/************************************************************************/
/*			    slogPeriodicData				*/
/************************************************************************/

static ST_VOID slogPeriodicData (MI_TA_PERIODIC *periodic)
  {
time_t tt;

  SLOGCALWAYS0 ("               PeriodicData: ");
  tt = (time_t) periodic->StartTime;
  SLOGCALWAYS1 ("                  StartTime:  %s", ctime (&tt));
  SLOGCALWAYS1 ("           PeriodResolution:  %d", periodic->PeriodResolution);
  SLOGCALWAYS1 ("            NumberOfPeriods:  %d", periodic->NumberOfPeriods);
  slogFloatMatrix (&periodic->ListOfPeriodicValues.floatMatrix);
  slogIntMatrix (&periodic->ListOfPeriodicValues.integerMatrix);
  }

/************************************************************************/
/*			    slogProfileData				*/
/************************************************************************/

static ST_VOID slogProfileData (MI_TA_PROFILE *profile)
  {
int i;
MI_TA_PROFILE_VALUE *value;
time_t tt;

  SLOGCALWAYS0 ("                ProfileData: ");
  SLOGCALWAYS1 ("              NumOfProfiles:  %d", profile->NumberOfProfiles);
  for (i = 0; i < profile->NumberOfProfiles; i++)
    {
    value = &profile->ListOfProfileValues[i];
    SLOGCALWAYS0 ("                ProfileData: ");
    tt = (time_t) value->RampStartTime;
    SLOGCALWAYS1 ("		 RampStartTime:  %s", ctime (&tt));
    SLOGCALWAYS1 ("               RampDuration:  %d", value->RampDuration);
    SLOGCALWAYS1 ("                      Price:  %f", value->Price);
    SLOGCALWAYS1 ("                TargetClass:  %d", value->TargetClass);
    SLOGCALWAYS1 ("                    Profile:  %f", value->Profile);
    }
  }

/************************************************************************/
/*			    slogFloatMatrix				*/
/************************************************************************/

static ST_VOID slogFloatMatrix (MI_FLOAT_MATRIX *floatMatrix)
  {
int i;

  if (floatMatrix->NumCols == 0)
    return;

  SLOGCALWAYS0 ("                FloatMatrix: ");
  SLOGCALWAYS1 ("                    NumRows:  %d", floatMatrix->NumRows);
  SLOGCALWAYS1 ("                    NumCols:  %d", floatMatrix->NumCols);
  for (i = 0; i < floatMatrix->NumCols; i++)
    {
    SLOGCALWAYS2 ("                MatrixId %d:  %d", i, floatMatrix->MatrixIds[i]);
    }

  for (i = 0; i < (floatMatrix->NumCols * floatMatrix->NumRows); i++)
    {
    SLOGCALWAYS2 ("              FloatValue %d:  %f", i, floatMatrix->FloatArray[i]);
    }
  }

/************************************************************************/
/*			    slogIntMatrix				*/
/************************************************************************/

static ST_VOID slogIntMatrix (MI_INTEGER_MATRIX *intMatrix)
  {
int i;

  if (intMatrix->NumCols == 0)
    return;

  SLOGCALWAYS0 ("              IntegerMatrix: ");
  SLOGCALWAYS1 ("                    NumRows:  %d", intMatrix->NumRows);
  SLOGCALWAYS1 ("                    NumCols:  %d", intMatrix->NumCols);
  for (i = 0; i < intMatrix->NumCols; i++)
    {
    SLOGCALWAYS2 ("                MatrixId %d:  %d", i, intMatrix->MatrixIds[i]);
    }

  for (i = 0; i < (intMatrix->NumCols * intMatrix->NumRows); i++)
    {
    SLOGCALWAYS2 ("            IntegerValue %d:  %d", i, intMatrix->IntegerArray[i]);
    }
  }

/************************************************************************/
/*			    slogTextMatrix				*/
/************************************************************************/

static ST_VOID slogTextMatrix (MI_TEXT_MATRIX *textMatrix)
  {
int i;

  if (textMatrix->NumCols == 0)
    return;

  SLOGCALWAYS0 ("                 TextMatrix: ");
  SLOGCALWAYS1 ("                    NumRows:  %d", textMatrix->NumRows);
  SLOGCALWAYS1 ("                    NumCols:  %d", textMatrix->NumCols);
  for (i = 0; i < textMatrix->NumCols; i++)
    {
    SLOGCALWAYS2 ("                MatrixId %d:  %d", i, textMatrix->MatrixIds[i]);
    }

  for (i = 0; i < (textMatrix->NumCols * textMatrix->NumRows); i++)
    {
    SLOGCALWAYS2 ("               TextValue %d:  %s", i, textMatrix->Text32Array[i]);
    }
  }

#endif	// CHOIBC DELETE
