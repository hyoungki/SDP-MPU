
/************************************************************************/
/*  This file created from input file 'miusrobj.odf'
    Leaf Access Parameter (LAP) File: 'Not Used'
	Created Wed Apr  1 07:32:38 2026
*/

#include "glbtypes.h"
#include "sysincs.h"
#include "mmsdefs.h"
#include "mms_pvar.h"
#include "mms_vvar.h"
#include "mvl_acse.h"
#include "miusrobj.h"

#include "mi_usr.h"	/* User Specified */

#ifdef DEBUG_SISCO
SD_CONST static ST_CHAR *SD_CONST thisFileName = __FILE__; 
#endif


/************************************************************************/
MVL_TYPE_CTRL *mvl_type_ctrl;
ST_INT mvl_num_types;
MVL_VMD_CTRL mvl_vmd;

/************************************************************************/

/* Init alignment table according to 'align.cfg'	*/
ST_INT m_lite_data_algn_tbl [NUM_ALGN_TYPES] = /* Data Alignment Table*/
  {
  0x0000,  /* ARRSTRT_ALGN   00  */
  0x0000,  /* ARREND_ALGN    01  */
  0x0000,  /* STRSTRT_ALGN   02  */
  0x0000,  /* STREND_ALGN    03  */
  0x0000,  /* INT8_ALGN      04  */
  0x0001,  /* INT16_ALGN     05  */
  0x0003,  /* INT32_ALGN     06  */
  0x0003,  /* INT64_ALGN     07  */
  0x0003,  /* FLOAT_ALGN     08  */
  0x0003,  /* DOUBLE_ALGN    09  */
  0x0000,  /* OCT_ALGN       10  */
  0x0000,  /* BOOL_ALGN      11  */
  0x0000,  /* BCD1_ALGN      12  */
  0x0001,  /* BCD2_ALGN      13  */
  0x0003,  /* BCD4_ALGN      14  */
  0x0000,  /* BIT_ALGN       15  */
  0x0000   /* VIS_ALGN       16  */
  };

/* Init structure alignment mode variables according to 'align.cfg'	*/
ST_INT m_struct_start_algn_mode = M_STRSTART_MODE_LARGEST;	/* value of M_STRSTART_MODE from align.cfg*/
ST_INT m_struct_end_algn_mode = M_STREND_MODE_LARGEST;	/* value of M_STREND_MODE from align.cfg*/
/************************************************************************/


/*	Common Strings Table	*/
ST_INT numMvlRtNames;
#ifdef USE_RT_TYPE_2
ST_CHAR mvlCompName_ [] = "";
ST_CHAR mvlCompName_Scope [] = "Scope";
ST_CHAR mvlCompName_DomainName [] = "DomainName";
ST_CHAR mvlCompName_Name [] = "Name";
ST_CHAR mvlCompName_DataSetName [] = "DataSetName";
ST_CHAR mvlCompName_StartTime [] = "StartTime";
ST_CHAR mvlCompName_Interval [] = "Interval";
ST_CHAR mvlCompName_TLE [] = "TLE";
ST_CHAR mvlCompName_BufferTime [] = "BufferTime";
ST_CHAR mvlCompName_IntegrityCheck [] = "IntegrityCheck";
ST_CHAR mvlCompName_DSConditionsRequested [] = "DSConditionsRequested";
ST_CHAR mvlCompName_BlockData [] = "BlockData";
ST_CHAR mvlCompName_Critical [] = "Critical";
ST_CHAR mvlCompName_RBE [] = "RBE";
ST_CHAR mvlCompName_Status [] = "Status";
ST_CHAR mvlCompName_EventCodeRequested [] = "EventCodeRequested";
ST_CHAR mvlCompName_AllChangesReported [] = "AllChangesReported";
ST_CHAR mvlCompName_TAConditionsRequested [] = "TAConditionsRequested";
ST_CHAR mvlCompName_MajorVersionNumber [] = "MajorVersionNumber";
ST_CHAR mvlCompName_MinorVersionNumber [] = "MinorVersionNumber";
ST_CHAR mvlCompName_Value [] = "Value";
ST_CHAR mvlCompName_Flags [] = "Flags";
ST_CHAR mvlCompName_TimeStamp [] = "TimeStamp";
ST_CHAR mvlCompName_COV [] = "COV";
ST_CHAR mvlCompName_GMTBasedS [] = "GMTBasedS";
ST_CHAR mvlCompName_Milliseconds [] = "Milliseconds";
ST_CHAR mvlCompName_Reason [] = "Reason";
ST_CHAR mvlCompName_InfoReference [] = "InfoReference";
ST_CHAR mvlCompName_LocalReference [] = "LocalReference";
ST_CHAR mvlCompName_MessageId [] = "MessageId";
ST_CHAR mvlCompName_Size [] = "Size";
ST_CHAR mvlCompName_TransferAccountReference [] = "TransferAccountReference";
ST_CHAR mvlCompName_Duration [] = "Duration";
ST_CHAR mvlCompName_RequestId [] = "RequestId";
ST_CHAR mvlCompName_TaConditionsRequested [] = "TaConditionsRequested";
#else	/* !USE_RT_TYPE_2	*/
SD_CONST ST_CHAR *SD_CONST foMvlRtNames[] =
  {
  "",
  "Scope",
  "DomainName",
  "Name",
  "DataSetName",
  "StartTime",
  "Interval",
  "TLE",
  "BufferTime",
  "IntegrityCheck",
  "DSConditionsRequested",
  "BlockData",
  "Critical",
  "RBE",
  "Status",
  "EventCodeRequested",
  "AllChangesReported",
  "TAConditionsRequested",
  "MajorVersionNumber",
  "MinorVersionNumber",
  "Value",
  "Flags",
  "TimeStamp",
  "COV",
  "GMTBasedS",
  "Milliseconds",
  "Reason",
  "InfoReference",
  "LocalReference",
  "MessageId",
  "Size",
  "TransferAccountReference",
  "Duration",
  "RequestId",
  "TaConditionsRequested"
  };

ST_CHAR **mvlRtNames;
ST_INT maxMvlRtNames;
#endif	/* !USE_RT_TYPE_2	*/

/************************************************************************/

/* MMS OBJECT INITIALIZATION */

ST_VOID mvl_init_type_ctrl (ST_VOID);
static ST_VOID mvl_init_vmd_vars (ST_VOID);
static ST_VOID mvl_init_dom_vars (ST_VOID);
static ST_VOID mvl_init_aa_vars (ST_VOID);
static ST_VOID mvl_init_vmd_varLists (ST_VOID);
static ST_VOID mvl_init_dom_varLists (ST_VOID);
static ST_VOID mvl_init_aa_varLists (ST_VOID);
static ST_VOID mvl_init_journals (ST_VOID);

/* mvl_init_mms_objs may be called more than once, but only first call	*/
/* has any effect.							*/
ST_VOID mvl_init_mms_objs ()
  {
#if defined(OBSOLETE_AA_OBJ_INIT)
ST_INT i;
#endif	/*#if defined(OBSOLETE_AA_OBJ_INIT)*/
static ST_BOOLEAN _mvlInitMmsObjsCalled = SD_FALSE;

/* If already called once, do NOTHING.	*/
  if (_mvlInitMmsObjsCalled)
    return;
  _mvlInitMmsObjsCalled = SD_TRUE;

  if (mvl_max_dyn.aa_nvls == 0)
    mvl_max_dyn.aa_nvls = MVL_NUM_DYN_AA_NVLS;

  if (mvl_max_dyn.aa_vars == 0)
    mvl_max_dyn.aa_vars = MVL_NUM_DYN_AA_VARS;

  if (mvl_max_dyn.doms == 0)
    mvl_max_dyn.doms = MVL_NUM_DYN_DOMS;

  if (mvl_max_dyn.dom_nvls == 0)
    mvl_max_dyn.dom_nvls = MVL_NUM_DYN_DOM_NVLS;

  if (mvl_max_dyn.dom_vars == 0)
    mvl_max_dyn.dom_vars = MVL_NUM_DYN_DOM_VARS;

  if (mvl_max_dyn.journals == 0)
    mvl_max_dyn.journals = MVL_NUM_DYN_JOUS;

  if (mvl_max_dyn.types == 0)
    mvl_max_dyn.types = MVLU_NUM_DYN_TYPES;

  if (mvl_max_dyn.vmd_nvls == 0)
    mvl_max_dyn.vmd_nvls = MVL_NUM_DYN_VMD_NVLS;

  if (mvl_max_dyn.vmd_vars == 0)
    mvl_max_dyn.vmd_vars = MVL_NUM_DYN_VMD_VARS;


#if defined(OBSOLETE_AA_OBJ_INIT)
/* Make sure conn_ctrl allocated (by mvl_start_acse)	*/
  assert (mvl_calling_conn_ctrl || mvl_called_conn_ctrl);

/* Set up the AA Control structures */
  for (i = 0; i < mvl_cfg_info->num_called; ++i)
    mvl_called_conn_ctrl[i].aa_objs = (MVL_AA_OBJ_CTRL *) M_CALLOC (MSMEM_STARTUP, 1, sizeof(MVL_AA_OBJ_CTRL));
  for (i = 0; i < mvl_cfg_info->num_calling; ++i)
    mvl_calling_conn_ctrl[i].aa_objs = (MVL_AA_OBJ_CTRL *) M_CALLOC (MSMEM_STARTUP, 1, sizeof(MVL_AA_OBJ_CTRL));
#endif	/*#if defined(OBSOLETE_AA_OBJ_INIT)*/

  mvl_init_type_ctrl ();
  mvl_init_vmd_vars ();
  mvl_init_dom_vars ();
  mvl_init_aa_vars ();
  mvl_init_vmd_varLists ();
  mvl_init_dom_varLists ();
  mvl_init_aa_varLists ();
  mvl_init_journals ();
  }
/************************************************************************/

/* VMD WIDE NAMED VARIABLE ASSOCIATION INITIALIZATION */

static ST_VOID mvl_init_vmd_vars ()
  {
MVL_VAR_ASSOC **ppva;

  mvl_vmd.max_num_var_assoc = 0 + mvl_max_dyn.vmd_vars;
  mvl_vmd.num_var_assoc = 0;
  if (mvl_vmd.max_num_var_assoc)
    mvl_vmd.var_assoc_tbl = ppva = (MVL_VAR_ASSOC **) M_CALLOC (MSMEM_STARTUP, mvl_vmd.max_num_var_assoc, sizeof (MVL_VAR_ASSOC *));
  }
/************************************************************************/

/* DOMAIN VARIABLE INITIALIZATION */
static ST_VOID mvl_init_dom_vars ()
  {
MVL_DOM_CTRL **ppdom;

  mvl_vmd.num_dom = 0;
  mvl_vmd.max_num_dom = 0 + mvl_max_dyn.doms;
  if (mvl_vmd.max_num_dom)
    mvl_vmd.dom_tbl = ppdom = (MVL_DOM_CTRL **) M_CALLOC (MSMEM_STARTUP, mvl_vmd.max_num_dom, sizeof (MVL_DOM_CTRL *));
  }

/************************************************************************/

/* AA VARIABLE INITIALIZATION */
static ST_VOID mvl_init_aa_vars ()
  {
#if defined(OBSOLETE_AA_OBJ_INIT)
MVL_AA_OBJ_CTRL *aa;
MVL_VAR_ASSOC **ppva;
ST_INT i;
ST_INT j;

/* Do AA specific variables */
  i = 0;
  for (j = 0; j < mvl_cfg_info->num_called; ++j, ++i)
    {
    aa = (MVL_AA_OBJ_CTRL *) mvl_called_conn_ctrl[j].aa_objs;
    aa->foundry_objects = SD_TRUE;
    aa->max_num_var_assoc = 0 + mvl_max_dyn.aa_vars;
    aa->num_var_assoc = 0;
    if (aa->max_num_var_assoc)
      aa->var_assoc_tbl = ppva = (MVL_VAR_ASSOC **) M_CALLOC (MSMEM_STARTUP, aa->max_num_var_assoc, sizeof (MVL_VAR_ASSOC *));
    }
  for (j = 0; j < mvl_cfg_info->num_calling; ++j, ++i)
    {
    aa = (MVL_AA_OBJ_CTRL *) mvl_calling_conn_ctrl[j].aa_objs;
    aa->foundry_objects = SD_TRUE;
    aa->max_num_var_assoc = 0 + mvl_max_dyn.aa_vars;
    aa->num_var_assoc = 0;
    if (aa->max_num_var_assoc)
      aa->var_assoc_tbl = ppva = (MVL_VAR_ASSOC **) M_CALLOC (MSMEM_STARTUP, aa->max_num_var_assoc, sizeof (MVL_VAR_ASSOC *));
    }
#endif	/*#if defined(OBSOLETE_AA_OBJ_INIT)*/
  }

/************************************************************************/

/* NAMED VARIABLE LIST INITIALIZATION */

static ST_VOID mvl_init_vmd_varLists ()
  {
MVL_NVLIST_CTRL **ppvl;

/* Do VMD wide variable lists */
  mvl_vmd.max_num_nvlist = 0 + mvl_max_dyn.vmd_nvls;
  mvl_vmd.num_nvlist = 0;
  if (mvl_vmd.max_num_nvlist)
    mvl_vmd.nvlist_tbl = ppvl = (MVL_NVLIST_CTRL **) M_CALLOC (MSMEM_STARTUP, mvl_vmd.max_num_nvlist, sizeof (MVL_NVLIST_CTRL *));
  }
/************************************************************************/

/* DOMAIN VARIABLE LIST INITIALIZATION */
static ST_VOID mvl_init_dom_varLists ()
  {
  }

/************************************************************************/

/* AA VARIABLE LIST INITIALIZATION */
static ST_VOID mvl_init_aa_varLists ()
  {
#if defined(OBSOLETE_AA_OBJ_INIT)
MVL_AA_OBJ_CTRL *aa;
MVL_NVLIST_CTRL **ppvl;
ST_INT i;


/* Now do AA specific Variable Lists */
  for (i = 0; i < mvl_cfg_info->num_called; ++i)
    {
    aa = (MVL_AA_OBJ_CTRL *) mvl_called_conn_ctrl[i].aa_objs;
    aa->foundry_objects = SD_TRUE;
    aa->max_num_nvlist = 0 + mvl_max_dyn.aa_nvls;
    aa->num_nvlist = 0;
    if (aa->max_num_nvlist)
      aa->nvlist_tbl = ppvl = (MVL_NVLIST_CTRL **) M_CALLOC (MSMEM_STARTUP, aa->max_num_nvlist, sizeof (MVL_NVLIST_CTRL *));
    }
  for (i = 0; i < mvl_cfg_info->num_calling; ++i)
    {
    aa = (MVL_AA_OBJ_CTRL *) mvl_calling_conn_ctrl[i].aa_objs;
    aa->foundry_objects = SD_TRUE;
    aa->max_num_nvlist = 0 + mvl_max_dyn.aa_nvls;
    aa->num_nvlist = 0;
    if (aa->max_num_nvlist)
      aa->nvlist_tbl = ppvl = (MVL_NVLIST_CTRL **) M_CALLOC (MSMEM_STARTUP, aa->max_num_nvlist, sizeof (MVL_NVLIST_CTRL *));
    }
#endif	/*#if defined(OBSOLETE_AA_OBJ_INIT)*/
  }

/************************************************************************/

/* JOURNAL INITIALIZATION */
static ST_VOID mvl_init_journals ()
  {
MVL_JOURNAL_CTRL **ppjou;

  /* initialize VMD_SPEC journals */

  mvl_vmd.max_num_jou = 0 + mvl_max_dyn.journals;
  mvl_vmd.num_jou = 0;
  if (mvl_vmd.max_num_jou)
    mvl_vmd.jou_tbl = ppjou = (MVL_JOURNAL_CTRL **) M_CALLOC (MSMEM_STARTUP, mvl_vmd.max_num_jou, sizeof (MVL_JOURNAL_CTRL *));
  }

/************************************************************************/

ST_VOID mvl_init_type_ctrl ()
  {
static ST_BOOLEAN _mvlInitTypeCtrlCalled = SD_FALSE;

  if (_mvlInitTypeCtrlCalled == SD_TRUE)
    return;
  _mvlInitTypeCtrlCalled = SD_TRUE;

#ifndef USE_RT_TYPE_2
  maxMvlRtNames = 35;
  numMvlRtNames = 35;
  mvlRtNames = foMvlRtNames;
#endif
  mvl_num_types = 40 + mvl_max_dyn.types;

  if (mvl_num_types)
    mvl_type_ctrl = (MVL_TYPE_CTRL *) M_CALLOC (MSMEM_STARTUP, mvl_num_types, sizeof(MVL_TYPE_CTRL));
#if defined USR_SUPPLIED_RT
  u_mvl_start_init_rt_tbl (40, 170);
#endif

/* Integer16 :  */
/*
  mvl_type_ctrl[Integer16_TYPEID].tdl = 
  "Short";
*/
  mvl_type_ctrl[Integer16_TYPEID].num_rt = 1;
  mvl_type_ctrl[Integer16_TYPEID].data_size = 2;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Integer16_TYPEID].rt = u_mvl_get_rt_tbl (Integer16_TYPEID, 1);
#else
  mvl_type_ctrl[Integer16_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Integer16_TYPEID].type_name, "Integer16");

/* Visible_String_32 :  */
/*
  mvl_type_ctrl[Visible_String_32_TYPEID].tdl = 
  "Vstring32";
*/
  mvl_type_ctrl[Visible_String_32_TYPEID].num_rt = 1;
  mvl_type_ctrl[Visible_String_32_TYPEID].data_size = 33;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Visible_String_32_TYPEID].rt = u_mvl_get_rt_tbl (Visible_String_32_TYPEID, 1);
#else
  mvl_type_ctrl[Visible_String_32_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Visible_String_32_TYPEID].type_name, "Visible_String_32");

/* MMSObjectName :  */
/*
  mvl_type_ctrl[MMSObjectName_TYPEID].tdl = 
  "{(Scope)<Unsigned8>,(DomainName)<MMSIdentifier>,(Name)<MMSIdentifier>\
}";
*/
  mvl_type_ctrl[MMSObjectName_TYPEID].num_rt = 5;
  mvl_type_ctrl[MMSObjectName_TYPEID].data_size = 131;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[MMSObjectName_TYPEID].rt = u_mvl_get_rt_tbl (MMSObjectName_TYPEID, 5);
#else
  mvl_type_ctrl[MMSObjectName_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[MMSObjectName_TYPEID].type_name, "MMSObjectName");

/* GMTBasedS :  */
/*
  mvl_type_ctrl[GMTBasedS_TYPEID].tdl = 
  "<Integer32>";
*/
  mvl_type_ctrl[GMTBasedS_TYPEID].num_rt = 1;
  mvl_type_ctrl[GMTBasedS_TYPEID].data_size = 4;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[GMTBasedS_TYPEID].rt = u_mvl_get_rt_tbl (GMTBasedS_TYPEID, 1);
#else
  mvl_type_ctrl[GMTBasedS_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[GMTBasedS_TYPEID].type_name, "GMTBasedS");

/* DSConditions :  */
/*
  mvl_type_ctrl[DSConditions_TYPEID].tdl = 
  "Bstring5";
*/
  mvl_type_ctrl[DSConditions_TYPEID].num_rt = 1;
  mvl_type_ctrl[DSConditions_TYPEID].data_size = 1;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[DSConditions_TYPEID].rt = u_mvl_get_rt_tbl (DSConditions_TYPEID, 1);
#else
  mvl_type_ctrl[DSConditions_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[DSConditions_TYPEID].type_name, "DSConditions");

/* TAConditions :  */
/*
  mvl_type_ctrl[TAConditions_TYPEID].tdl = 
  "Bstring8";
*/
  mvl_type_ctrl[TAConditions_TYPEID].num_rt = 1;
  mvl_type_ctrl[TAConditions_TYPEID].data_size = 1;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[TAConditions_TYPEID].rt = u_mvl_get_rt_tbl (TAConditions_TYPEID, 1);
#else
  mvl_type_ctrl[TAConditions_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[TAConditions_TYPEID].type_name, "TAConditions");

/* DSTransferSet1996 :  */
/*
  mvl_type_ctrl[DSTransferSet1996_TYPEID].tdl = 
  "{(DataSetName)<MMSObjectName>,(StartTime)<GMTBasedS>,(Interval)<TimeI\
ntervalS>,(TLE)<TimeIntervalS>,(BufferTime)<TimeIntervalS>,(Integrity\
Check)<TimeIntervalS>,(DSConditionsRequested)<DSConditions>,(BlockData\
)<Boolean>,(Critical)<Boolean>,(RBE)<Boolean>,(Status)<Boolean>,(Eve\
ntCodeRequested)<Integer16>}";
*/
  mvl_type_ctrl[DSTransferSet1996_TYPEID].num_rt = 18;
  mvl_type_ctrl[DSTransferSet1996_TYPEID].data_size = 152;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[DSTransferSet1996_TYPEID].rt = u_mvl_get_rt_tbl (DSTransferSet1996_TYPEID, 18);
#else
  mvl_type_ctrl[DSTransferSet1996_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[DSTransferSet1996_TYPEID].type_name, "DSTransferSet1996");

/* DSTransferSet2000 :  */
/*
  mvl_type_ctrl[DSTransferSet2000_TYPEID].tdl = 
  "{(DataSetName)<MMSObjectName>,(StartTime)<GMTBasedS>,(Interval)<TimeI\
ntervalS>,(TLE)<TimeIntervalS>,(BufferTime)<TimeIntervalS>,(Integrity\
Check)<TimeIntervalS>,(DSConditionsRequested)<DSConditions>,(BlockData\
)<Boolean>,(Critical)<Boolean>,(RBE)<Boolean>,(AllChangesReported)<Bo\
olean>,(Status)<Boolean>,(EventCodeRequested)<Integer16>}";
*/
  mvl_type_ctrl[DSTransferSet2000_TYPEID].num_rt = 19;
  mvl_type_ctrl[DSTransferSet2000_TYPEID].data_size = 152;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[DSTransferSet2000_TYPEID].rt = u_mvl_get_rt_tbl (DSTransferSet2000_TYPEID, 19);
#else
  mvl_type_ctrl[DSTransferSet2000_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[DSTransferSet2000_TYPEID].type_name, "DSTransferSet2000");

/* IMTransferSet :  */
/*
  mvl_type_ctrl[IMTransferSet_TYPEID].tdl = 
  "<Boolean>";
*/
  mvl_type_ctrl[IMTransferSet_TYPEID].num_rt = 1;
  mvl_type_ctrl[IMTransferSet_TYPEID].data_size = 1;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[IMTransferSet_TYPEID].rt = u_mvl_get_rt_tbl (IMTransferSet_TYPEID, 1);
#else
  mvl_type_ctrl[IMTransferSet_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[IMTransferSet_TYPEID].type_name, "IMTransferSet");

/* TATransferSet :  */
/*
  mvl_type_ctrl[TATransferSet_TYPEID].tdl = 
  "{(TAConditionsRequested)<TAConditions>,(BlockData)<Boolean>,(Status)<\
Boolean>}";
*/
  mvl_type_ctrl[TATransferSet_TYPEID].num_rt = 5;
  mvl_type_ctrl[TATransferSet_TYPEID].data_size = 3;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[TATransferSet_TYPEID].rt = u_mvl_get_rt_tbl (TATransferSet_TYPEID, 5);
#else
  mvl_type_ctrl[TATransferSet_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[TATransferSet_TYPEID].type_name, "TATransferSet");

/* SupportedFeatures :  */
/*
  mvl_type_ctrl[SupportedFeatures_TYPEID].tdl = 
  "Bstring12";
*/
  mvl_type_ctrl[SupportedFeatures_TYPEID].num_rt = 1;
  mvl_type_ctrl[SupportedFeatures_TYPEID].data_size = 2;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[SupportedFeatures_TYPEID].rt = u_mvl_get_rt_tbl (SupportedFeatures_TYPEID, 1);
#else
  mvl_type_ctrl[SupportedFeatures_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[SupportedFeatures_TYPEID].type_name, "SupportedFeatures");

/* TASE2Version :  */
/*
  mvl_type_ctrl[TASE2Version_TYPEID].tdl = 
  "{(MajorVersionNumber)<Integer16>,(MinorVersionNumber)<Integer16>}";
*/
  mvl_type_ctrl[TASE2Version_TYPEID].num_rt = 4;
  mvl_type_ctrl[TASE2Version_TYPEID].data_size = 4;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[TASE2Version_TYPEID].rt = u_mvl_get_rt_tbl (TASE2Version_TYPEID, 4);
#else
  mvl_type_ctrl[TASE2Version_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[TASE2Version_TYPEID].type_name, "TASE2Version");

/* Data_Real :  */
/*
  mvl_type_ctrl[Data_Real_TYPEID].tdl = 
  "Float";
*/
  mvl_type_ctrl[Data_Real_TYPEID].num_rt = 1;
  mvl_type_ctrl[Data_Real_TYPEID].data_size = 4;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_Real_TYPEID].rt = u_mvl_get_rt_tbl (Data_Real_TYPEID, 1);
#else
  mvl_type_ctrl[Data_Real_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_Real_TYPEID].type_name, "Data_Real");

/* Data_State :  */
/*
  mvl_type_ctrl[Data_State_TYPEID].tdl = 
  "Bstring8";
*/
  mvl_type_ctrl[Data_State_TYPEID].num_rt = 1;
  mvl_type_ctrl[Data_State_TYPEID].data_size = 1;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_State_TYPEID].rt = u_mvl_get_rt_tbl (Data_State_TYPEID, 1);
#else
  mvl_type_ctrl[Data_State_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_State_TYPEID].type_name, "Data_State");

/* Data_Discrete :  */
/*
  mvl_type_ctrl[Data_Discrete_TYPEID].tdl = 
  "<Integer32>";
*/
  mvl_type_ctrl[Data_Discrete_TYPEID].num_rt = 1;
  mvl_type_ctrl[Data_Discrete_TYPEID].data_size = 4;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_Discrete_TYPEID].rt = u_mvl_get_rt_tbl (Data_Discrete_TYPEID, 1);
#else
  mvl_type_ctrl[Data_Discrete_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_Discrete_TYPEID].type_name, "Data_Discrete");

/* Data_RealQ :  */
/*
  mvl_type_ctrl[Data_RealQ_TYPEID].tdl = 
  "{(Value)<Data_Real>,(Flags)<Data_Flags>}";
*/
  mvl_type_ctrl[Data_RealQ_TYPEID].num_rt = 4;
  mvl_type_ctrl[Data_RealQ_TYPEID].data_size = 8;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_RealQ_TYPEID].rt = u_mvl_get_rt_tbl (Data_RealQ_TYPEID, 4);
#else
  mvl_type_ctrl[Data_RealQ_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_RealQ_TYPEID].type_name, "Data_RealQ");

/* Data_StateQ :  */
/*
  mvl_type_ctrl[Data_StateQ_TYPEID].tdl = 
  "<Data_State>";
*/
  mvl_type_ctrl[Data_StateQ_TYPEID].num_rt = 1;
  mvl_type_ctrl[Data_StateQ_TYPEID].data_size = 1;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_StateQ_TYPEID].rt = u_mvl_get_rt_tbl (Data_StateQ_TYPEID, 1);
#else
  mvl_type_ctrl[Data_StateQ_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_StateQ_TYPEID].type_name, "Data_StateQ");

/* Data_DiscreteQ :  */
/*
  mvl_type_ctrl[Data_DiscreteQ_TYPEID].tdl = 
  "{(Value)<Data_Discrete>,(Flags)<Data_Flags>}";
*/
  mvl_type_ctrl[Data_DiscreteQ_TYPEID].num_rt = 4;
  mvl_type_ctrl[Data_DiscreteQ_TYPEID].data_size = 8;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_DiscreteQ_TYPEID].rt = u_mvl_get_rt_tbl (Data_DiscreteQ_TYPEID, 4);
#else
  mvl_type_ctrl[Data_DiscreteQ_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_DiscreteQ_TYPEID].type_name, "Data_DiscreteQ");

/* Data_RealQTimeTag :  */
/*
  mvl_type_ctrl[Data_RealQTimeTag_TYPEID].tdl = 
  "{(Value)<Data_Real>,(TimeStamp)<Data_TimeStamp>,(Flags)<Data_Flags>}\
";
*/
  mvl_type_ctrl[Data_RealQTimeTag_TYPEID].num_rt = 5;
  mvl_type_ctrl[Data_RealQTimeTag_TYPEID].data_size = 12;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_RealQTimeTag_TYPEID].rt = u_mvl_get_rt_tbl (Data_RealQTimeTag_TYPEID, 5);
#else
  mvl_type_ctrl[Data_RealQTimeTag_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_RealQTimeTag_TYPEID].type_name, "Data_RealQTimeTag");

/* Data_StateQTimeTag :  */
/*
  mvl_type_ctrl[Data_StateQTimeTag_TYPEID].tdl = 
  "{(TimeStamp)<Data_TimeStamp>,(Flags)<Data_Flags>}";
*/
  mvl_type_ctrl[Data_StateQTimeTag_TYPEID].num_rt = 4;
  mvl_type_ctrl[Data_StateQTimeTag_TYPEID].data_size = 8;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_StateQTimeTag_TYPEID].rt = u_mvl_get_rt_tbl (Data_StateQTimeTag_TYPEID, 4);
#else
  mvl_type_ctrl[Data_StateQTimeTag_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_StateQTimeTag_TYPEID].type_name, "Data_StateQTimeTag");

/* Data_DiscreteQTimeTag :  */
/*
  mvl_type_ctrl[Data_DiscreteQTimeTag_TYPEID].tdl = 
  "{(Value)<Data_Discrete>,(TimeStamp)<Data_TimeStamp>,(Flags)<Data_Flag\
s>}";
*/
  mvl_type_ctrl[Data_DiscreteQTimeTag_TYPEID].num_rt = 5;
  mvl_type_ctrl[Data_DiscreteQTimeTag_TYPEID].data_size = 12;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_DiscreteQTimeTag_TYPEID].rt = u_mvl_get_rt_tbl (Data_DiscreteQTimeTag_TYPEID, 5);
#else
  mvl_type_ctrl[Data_DiscreteQTimeTag_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_DiscreteQTimeTag_TYPEID].type_name, "Data_DiscreteQTimeTag");

/* Data_RealExtended :  */
/*
  mvl_type_ctrl[Data_RealExtended_TYPEID].tdl = 
  "{(Value)<Data_Real>,(TimeStamp)<Data_TimeStamp>,(Flags)<Data_Flags>,\
(COV)<COV_Counter>}";
*/
  mvl_type_ctrl[Data_RealExtended_TYPEID].num_rt = 6;
  mvl_type_ctrl[Data_RealExtended_TYPEID].data_size = 12;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_RealExtended_TYPEID].rt = u_mvl_get_rt_tbl (Data_RealExtended_TYPEID, 6);
#else
  mvl_type_ctrl[Data_RealExtended_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_RealExtended_TYPEID].type_name, "Data_RealExtended");

/* Data_StateExtended :  */
/*
  mvl_type_ctrl[Data_StateExtended_TYPEID].tdl = 
  "{(TimeStamp)<Data_TimeStamp>,(Flags)<Data_State>,(COV)<COV_Counter>}\
";
*/
  mvl_type_ctrl[Data_StateExtended_TYPEID].num_rt = 5;
  mvl_type_ctrl[Data_StateExtended_TYPEID].data_size = 8;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_StateExtended_TYPEID].rt = u_mvl_get_rt_tbl (Data_StateExtended_TYPEID, 5);
#else
  mvl_type_ctrl[Data_StateExtended_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_StateExtended_TYPEID].type_name, "Data_StateExtended");

/* Data_DiscreteExtended :  */
/*
  mvl_type_ctrl[Data_DiscreteExtended_TYPEID].tdl = 
  "{(Value)<Data_Discrete>,(TimeStamp)<Data_TimeStamp>,(Flags)<Data_Flag\
s>,(COV)<COV_Counter>}";
*/
  mvl_type_ctrl[Data_DiscreteExtended_TYPEID].num_rt = 6;
  mvl_type_ctrl[Data_DiscreteExtended_TYPEID].data_size = 12;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_DiscreteExtended_TYPEID].rt = u_mvl_get_rt_tbl (Data_DiscreteExtended_TYPEID, 6);
#else
  mvl_type_ctrl[Data_DiscreteExtended_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_DiscreteExtended_TYPEID].type_name, "Data_DiscreteExtended");

/* Data_RealQTimeTagExtended :  */
/*
  mvl_type_ctrl[Data_RealQTimeTagExtended_TYPEID].tdl = 
  "{(Value)<Data_Real>,(TimeStamp)<TimeStampExtended>,(Flags)<Data_Flags\
>}";
*/
  mvl_type_ctrl[Data_RealQTimeTagExtended_TYPEID].num_rt = 8;
  mvl_type_ctrl[Data_RealQTimeTagExtended_TYPEID].data_size = 16;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_RealQTimeTagExtended_TYPEID].rt = u_mvl_get_rt_tbl (Data_RealQTimeTagExtended_TYPEID, 8);
#else
  mvl_type_ctrl[Data_RealQTimeTagExtended_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_RealQTimeTagExtended_TYPEID].type_name, "Data_RealQTimeTagExtended");

/* Data_StateQTimeTagExtended :  */
/*
  mvl_type_ctrl[Data_StateQTimeTagExtended_TYPEID].tdl = 
  "{(TimeStamp)<TimeStampExtended>,(Flags)<Data_Flags>}";
*/
  mvl_type_ctrl[Data_StateQTimeTagExtended_TYPEID].num_rt = 7;
  mvl_type_ctrl[Data_StateQTimeTagExtended_TYPEID].data_size = 12;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_StateQTimeTagExtended_TYPEID].rt = u_mvl_get_rt_tbl (Data_StateQTimeTagExtended_TYPEID, 7);
#else
  mvl_type_ctrl[Data_StateQTimeTagExtended_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_StateQTimeTagExtended_TYPEID].type_name, "Data_StateQTimeTagExtended");

/* Data_DiscreteQTimeTagExtended :  */
/*
  mvl_type_ctrl[Data_DiscreteQTimeTagExtended_TYPEID].tdl = 
  "{(Value)<Data_Discrete>,(TimeStamp)<TimeStampExtended>,(Flags)<Data_F\
lags>}";
*/
  mvl_type_ctrl[Data_DiscreteQTimeTagExtended_TYPEID].num_rt = 8;
  mvl_type_ctrl[Data_DiscreteQTimeTagExtended_TYPEID].data_size = 16;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_DiscreteQTimeTagExtended_TYPEID].rt = u_mvl_get_rt_tbl (Data_DiscreteQTimeTagExtended_TYPEID, 8);
#else
  mvl_type_ctrl[Data_DiscreteQTimeTagExtended_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_DiscreteQTimeTagExtended_TYPEID].type_name, "Data_DiscreteQTimeTagExtended");

/* Data_StateSupplemental :  */
/*
  mvl_type_ctrl[Data_StateSupplemental_TYPEID].tdl = 
  "Bstring8";
*/
  mvl_type_ctrl[Data_StateSupplemental_TYPEID].num_rt = 1;
  mvl_type_ctrl[Data_StateSupplemental_TYPEID].data_size = 1;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_StateSupplemental_TYPEID].rt = u_mvl_get_rt_tbl (Data_StateSupplemental_TYPEID, 1);
#else
  mvl_type_ctrl[Data_StateSupplemental_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_StateSupplemental_TYPEID].type_name, "Data_StateSupplemental");

/* Data_StateSupplementalQ :  */
/*
  mvl_type_ctrl[Data_StateSupplementalQ_TYPEID].tdl = 
  "{(Value)<Data_StateSupplemental>,(Flags)<Data_Flags>}";
*/
  mvl_type_ctrl[Data_StateSupplementalQ_TYPEID].num_rt = 4;
  mvl_type_ctrl[Data_StateSupplementalQ_TYPEID].data_size = 2;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_StateSupplementalQ_TYPEID].rt = u_mvl_get_rt_tbl (Data_StateSupplementalQ_TYPEID, 4);
#else
  mvl_type_ctrl[Data_StateSupplementalQ_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_StateSupplementalQ_TYPEID].type_name, "Data_StateSupplementalQ");

/* Data_StateSupplementalQTimeTag :  */
/*
  mvl_type_ctrl[Data_StateSupplementalQTimeTag_TYPEID].tdl = 
  "{(Value)<Data_StateSupplemental>,(TimeStamp)<Data_TimeStamp>,(Flags)<\
Data_Flags>}";
*/
  mvl_type_ctrl[Data_StateSupplementalQTimeTag_TYPEID].num_rt = 5;
  mvl_type_ctrl[Data_StateSupplementalQTimeTag_TYPEID].data_size = 12;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_StateSupplementalQTimeTag_TYPEID].rt = u_mvl_get_rt_tbl (Data_StateSupplementalQTimeTag_TYPEID, 5);
#else
  mvl_type_ctrl[Data_StateSupplementalQTimeTag_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_StateSupplementalQTimeTag_TYPEID].type_name, "Data_StateSupplementalQTimeTag");

/* Data_StateSupplementalExtended :  */
/*
  mvl_type_ctrl[Data_StateSupplementalExtended_TYPEID].tdl = 
  "{(Value)<Data_StateSupplemental>,(TimeStamp)<Data_TimeStamp>,(Flags)<\
Data_Flags>,(COV)<COV_Counter>}";
*/
  mvl_type_ctrl[Data_StateSupplementalExtended_TYPEID].num_rt = 6;
  mvl_type_ctrl[Data_StateSupplementalExtended_TYPEID].data_size = 12;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_StateSupplementalExtended_TYPEID].rt = u_mvl_get_rt_tbl (Data_StateSupplementalExtended_TYPEID, 6);
#else
  mvl_type_ctrl[Data_StateSupplementalExtended_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_StateSupplementalExtended_TYPEID].type_name, "Data_StateSupplementalExtended");

/* Data_StateSupplementalQTimeTagExtended :  */
/*
  mvl_type_ctrl[Data_StateSupplementalQTimeTagExtended_TYPEID].tdl = 
  "{(Value)<Data_StateSupplemental>,(TimeStamp)<TimeStampExtended>,(Flag\
s)<Data_Flags>}";
*/
  mvl_type_ctrl[Data_StateSupplementalQTimeTagExtended_TYPEID].num_rt = 8;
  mvl_type_ctrl[Data_StateSupplementalQTimeTagExtended_TYPEID].data_size = 16;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Data_StateSupplementalQTimeTagExtended_TYPEID].rt = u_mvl_get_rt_tbl (Data_StateSupplementalQTimeTagExtended_TYPEID, 8);
#else
  mvl_type_ctrl[Data_StateSupplementalQTimeTagExtended_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Data_StateSupplementalQTimeTagExtended_TYPEID].type_name, "Data_StateSupplementalQTimeTagExtended");

/* Control_Command :  */
/*
  mvl_type_ctrl[Control_Command_TYPEID].tdl = 
  "Short";
*/
  mvl_type_ctrl[Control_Command_TYPEID].num_rt = 1;
  mvl_type_ctrl[Control_Command_TYPEID].data_size = 2;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Control_Command_TYPEID].rt = u_mvl_get_rt_tbl (Control_Command_TYPEID, 1);
#else
  mvl_type_ctrl[Control_Command_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Control_Command_TYPEID].type_name, "Control_Command");

/* Control_Setpoint_Real :  */
/*
  mvl_type_ctrl[Control_Setpoint_Real_TYPEID].tdl = 
  "Float";
*/
  mvl_type_ctrl[Control_Setpoint_Real_TYPEID].num_rt = 1;
  mvl_type_ctrl[Control_Setpoint_Real_TYPEID].data_size = 4;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Control_Setpoint_Real_TYPEID].rt = u_mvl_get_rt_tbl (Control_Setpoint_Real_TYPEID, 1);
#else
  mvl_type_ctrl[Control_Setpoint_Real_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Control_Setpoint_Real_TYPEID].type_name, "Control_Setpoint_Real");

/* Control_Setpoint_Discrete :  */
/*
  mvl_type_ctrl[Control_Setpoint_Discrete_TYPEID].tdl = 
  "Short";
*/
  mvl_type_ctrl[Control_Setpoint_Discrete_TYPEID].num_rt = 1;
  mvl_type_ctrl[Control_Setpoint_Discrete_TYPEID].data_size = 2;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[Control_Setpoint_Discrete_TYPEID].rt = u_mvl_get_rt_tbl (Control_Setpoint_Discrete_TYPEID, 1);
#else
  mvl_type_ctrl[Control_Setpoint_Discrete_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[Control_Setpoint_Discrete_TYPEID].type_name, "Control_Setpoint_Discrete");

/* SBO_CheckBackName :  */
/*
  mvl_type_ctrl[SBO_CheckBackName_TYPEID].tdl = 
  "Short";
*/
  mvl_type_ctrl[SBO_CheckBackName_TYPEID].num_rt = 1;
  mvl_type_ctrl[SBO_CheckBackName_TYPEID].data_size = 2;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[SBO_CheckBackName_TYPEID].rt = u_mvl_get_rt_tbl (SBO_CheckBackName_TYPEID, 1);
#else
  mvl_type_ctrl[SBO_CheckBackName_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[SBO_CheckBackName_TYPEID].type_name, "SBO_CheckBackName");

/* TagValue :  */
/*
  mvl_type_ctrl[TagValue_TYPEID].tdl = 
  "{(Flags)<TagFlags>,(Reason)<TextString>}";
*/
  mvl_type_ctrl[TagValue_TYPEID].num_rt = 4;
  mvl_type_ctrl[TagValue_TYPEID].data_size = 257;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[TagValue_TYPEID].rt = u_mvl_get_rt_tbl (TagValue_TYPEID, 4);
#else
  mvl_type_ctrl[TagValue_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[TagValue_TYPEID].type_name, "TagValue");

/* InfoMessHeader :  */
/*
  mvl_type_ctrl[InfoMessHeader_TYPEID].tdl = 
  "{(InfoReference)<ReferenceNum>,(LocalReference)<ReferenceNum>,(Messag\
eId)<ReferenceNum>,(Size)<Number>}";
*/
  mvl_type_ctrl[InfoMessHeader_TYPEID].num_rt = 6;
  mvl_type_ctrl[InfoMessHeader_TYPEID].data_size = 16;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[InfoMessHeader_TYPEID].rt = u_mvl_get_rt_tbl (InfoMessHeader_TYPEID, 6);
#else
  mvl_type_ctrl[InfoMessHeader_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[InfoMessHeader_TYPEID].type_name, "InfoMessHeader");

/* InfoBuffXXX :  */
/*
  mvl_type_ctrl[InfoBuffXXX_TYPEID].tdl = 
  "Ostring128";
*/
  mvl_type_ctrl[InfoBuffXXX_TYPEID].num_rt = 1;
  mvl_type_ctrl[InfoBuffXXX_TYPEID].data_size = 128;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[InfoBuffXXX_TYPEID].rt = u_mvl_get_rt_tbl (InfoBuffXXX_TYPEID, 1);
#else
  mvl_type_ctrl[InfoBuffXXX_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[InfoBuffXXX_TYPEID].type_name, "InfoBuffXXX");

/* TAAccountRequest :  */
/*
  mvl_type_ctrl[TAAccountRequest_TYPEID].tdl = 
  "{(TransferAccountReference)<ReferenceNum>,(StartTime)<GMTBasedS>,(Dur\
ation)<TimeIntervalL32>,(RequestId)<ReferenceNum>,(TaConditionsRequest\
ed)<TAConditions>}";
*/
  mvl_type_ctrl[TAAccountRequest_TYPEID].num_rt = 7;
  mvl_type_ctrl[TAAccountRequest_TYPEID].data_size = 20;
#if defined USR_SUPPLIED_RT
  mvl_type_ctrl[TAAccountRequest_TYPEID].rt = u_mvl_get_rt_tbl (TAAccountRequest_TYPEID, 7);
#else
  mvl_type_ctrl[TAAccountRequest_TYPEID].rt = mvl_rt_tables[rt_table_index++];
#endif /* #if defined USR_SUPPLIED_RT */
  strcpy (mvl_type_ctrl[TAAccountRequest_TYPEID].type_name, "TAAccountRequest");


#if defined USR_SUPPLIED_RT
  u_mvl_end_init_rt_tbl ();
#endif
  }


/************************************************************************/
/* RUNTIME TYPE DATA */


#if !defined USR_SUPPLIED_RT

/************************************************************************/
/* Integer16 :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_0[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Visible_String_32 :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_1[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_VISIBLE_STRING,		/* el_tag				*/
    33,				/* el_size				*/
    33,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      -32,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* MMSObjectName :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_2[5] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    131,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_UNSIGNED,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Scope, 	/* comp_name_ptr 'Scope'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_SCOPE_STRING_INDEX, 	/* name_index 'Scope'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_VISIBLE_STRING,		/* el_tag				*/
    65,				/* el_size				*/
    65,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      -64,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_DomainName, 	/* comp_name_ptr 'DomainName'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_DOMAINNAME_STRING_INDEX, 	/* name_index 'DomainName'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_VISIBLE_STRING,		/* el_tag				*/
    65,				/* el_size				*/
    65,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      -64,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Name, 	/* comp_name_ptr 'Name'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_NAME_STRING_INDEX, 	/* name_index 'Name'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* GMTBasedS :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_3[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* DSConditions :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_4[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      5,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* TAConditions :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_5[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* DSTransferSet1996 :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_6[18] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    152,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      16,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    131,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_DataSetName, 	/* comp_name_ptr 'DataSetName'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_DATASETNAME_STRING_INDEX, 	/* name_index 'DataSetName'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_UNSIGNED,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Scope, 	/* comp_name_ptr 'Scope'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_SCOPE_STRING_INDEX, 	/* name_index 'Scope'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_VISIBLE_STRING,		/* el_tag				*/
    65,				/* el_size				*/
    65,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      -64,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_DomainName, 	/* comp_name_ptr 'DomainName'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_DOMAINNAME_STRING_INDEX, 	/* name_index 'DomainName'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_VISIBLE_STRING,		/* el_tag				*/
    65,				/* el_size				*/
    65,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      -64,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Name, 	/* comp_name_ptr 'Name'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_NAME_STRING_INDEX, 	/* name_index 'Name'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[5] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    1,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[6] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_StartTime, 	/* comp_name_ptr 'StartTime'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_STARTTIME_STRING_INDEX, 	/* name_index 'StartTime'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[7] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Interval, 	/* comp_name_ptr 'Interval'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_INTERVAL_STRING_INDEX, 	/* name_index 'Interval'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[8] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TLE, 	/* comp_name_ptr 'TLE'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TLE_STRING_INDEX, 	/* name_index 'TLE'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[9] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_BufferTime, 	/* comp_name_ptr 'BufferTime'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_BUFFERTIME_STRING_INDEX, 	/* name_index 'BufferTime'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[10] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_IntegrityCheck, 	/* comp_name_ptr 'IntegrityCheck'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_INTEGRITYCHECK_STRING_INDEX, 	/* name_index 'IntegrityCheck'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[11] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      5,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_DSConditionsRequested, 	/* comp_name_ptr 'DSConditionsRequested'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_DSCONDITIONSREQUESTED_STRING_INDEX, 	/* name_index 'DSConditionsRequested'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[12] init data ...			*/
    RT_BOOL,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_BlockData, 	/* comp_name_ptr 'BlockData'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_BLOCKDATA_STRING_INDEX, 	/* name_index 'BlockData'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[13] init data ...			*/
    RT_BOOL,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Critical, 	/* comp_name_ptr 'Critical'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_CRITICAL_STRING_INDEX, 	/* name_index 'Critical'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[14] init data ...			*/
    RT_BOOL,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_RBE, 	/* comp_name_ptr 'RBE'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_RBE_STRING_INDEX, 	/* name_index 'RBE'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[15] init data ...			*/
    RT_BOOL,		/* el_tag				*/
    2,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Status, 	/* comp_name_ptr 'Status'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_STATUS_STRING_INDEX, 	/* name_index 'Status'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[16] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_EventCodeRequested, 	/* comp_name_ptr 'EventCodeRequested'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_EVENTCODEREQUESTED_STRING_INDEX, 	/* name_index 'EventCodeRequested'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[17] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      16,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* DSTransferSet2000 :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_7[19] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    152,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      17,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    131,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_DataSetName, 	/* comp_name_ptr 'DataSetName'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_DATASETNAME_STRING_INDEX, 	/* name_index 'DataSetName'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_UNSIGNED,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Scope, 	/* comp_name_ptr 'Scope'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_SCOPE_STRING_INDEX, 	/* name_index 'Scope'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_VISIBLE_STRING,		/* el_tag				*/
    65,				/* el_size				*/
    65,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      -64,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_DomainName, 	/* comp_name_ptr 'DomainName'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_DOMAINNAME_STRING_INDEX, 	/* name_index 'DomainName'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_VISIBLE_STRING,		/* el_tag				*/
    65,				/* el_size				*/
    65,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      -64,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Name, 	/* comp_name_ptr 'Name'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_NAME_STRING_INDEX, 	/* name_index 'Name'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[5] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    1,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[6] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_StartTime, 	/* comp_name_ptr 'StartTime'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_STARTTIME_STRING_INDEX, 	/* name_index 'StartTime'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[7] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Interval, 	/* comp_name_ptr 'Interval'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_INTERVAL_STRING_INDEX, 	/* name_index 'Interval'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[8] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TLE, 	/* comp_name_ptr 'TLE'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TLE_STRING_INDEX, 	/* name_index 'TLE'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[9] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_BufferTime, 	/* comp_name_ptr 'BufferTime'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_BUFFERTIME_STRING_INDEX, 	/* name_index 'BufferTime'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[10] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_IntegrityCheck, 	/* comp_name_ptr 'IntegrityCheck'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_INTEGRITYCHECK_STRING_INDEX, 	/* name_index 'IntegrityCheck'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[11] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      5,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_DSConditionsRequested, 	/* comp_name_ptr 'DSConditionsRequested'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_DSCONDITIONSREQUESTED_STRING_INDEX, 	/* name_index 'DSConditionsRequested'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[12] init data ...			*/
    RT_BOOL,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_BlockData, 	/* comp_name_ptr 'BlockData'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_BLOCKDATA_STRING_INDEX, 	/* name_index 'BlockData'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[13] init data ...			*/
    RT_BOOL,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Critical, 	/* comp_name_ptr 'Critical'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_CRITICAL_STRING_INDEX, 	/* name_index 'Critical'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[14] init data ...			*/
    RT_BOOL,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_RBE, 	/* comp_name_ptr 'RBE'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_RBE_STRING_INDEX, 	/* name_index 'RBE'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[15] init data ...			*/
    RT_BOOL,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_AllChangesReported, 	/* comp_name_ptr 'AllChangesReported'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_ALLCHANGESREPORTED_STRING_INDEX, 	/* name_index 'AllChangesReported'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[16] init data ...			*/
    RT_BOOL,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Status, 	/* comp_name_ptr 'Status'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_STATUS_STRING_INDEX, 	/* name_index 'Status'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[17] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_EventCodeRequested, 	/* comp_name_ptr 'EventCodeRequested'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_EVENTCODEREQUESTED_STRING_INDEX, 	/* name_index 'EventCodeRequested'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[18] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      17,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* IMTransferSet :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_8[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_BOOL,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* TATransferSet :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_9[5] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    3,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TAConditionsRequested, 	/* comp_name_ptr 'TAConditionsRequested'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TACONDITIONSREQUESTED_STRING_INDEX, 	/* name_index 'TAConditionsRequested'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_BOOL,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_BlockData, 	/* comp_name_ptr 'BlockData'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_BLOCKDATA_STRING_INDEX, 	/* name_index 'BlockData'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_BOOL,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      1,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Status, 	/* comp_name_ptr 'Status'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_STATUS_STRING_INDEX, 	/* name_index 'Status'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* SupportedFeatures :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_10[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      12,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* TASE2Version :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_11[4] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_MajorVersionNumber, 	/* comp_name_ptr 'MajorVersionNumber'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_MAJORVERSIONNUMBER_STRING_INDEX, 	/* name_index 'MajorVersionNumber'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_MinorVersionNumber, 	/* comp_name_ptr 'MinorVersionNumber'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_MINORVERSIONNUMBER_STRING_INDEX, 	/* name_index 'MinorVersionNumber'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_Real :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_12[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_FLOATING_POINT,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_State :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_13[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_Discrete :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_14[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_RealQ :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_15[4] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    8,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_FLOATING_POINT,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Value, 	/* comp_name_ptr 'Value'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_VALUE_STRING_INDEX, 	/* name_index 'Value'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    4,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_StateQ :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_16[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_DiscreteQ :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_17[4] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    8,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Value, 	/* comp_name_ptr 'Value'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_VALUE_STRING_INDEX, 	/* name_index 'Value'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    4,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_RealQTimeTag :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_18[5] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    12,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_FLOATING_POINT,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Value, 	/* comp_name_ptr 'Value'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_VALUE_STRING_INDEX, 	/* name_index 'Value'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TimeStamp, 	/* comp_name_ptr 'TimeStamp'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TIMESTAMP_STRING_INDEX, 	/* name_index 'TimeStamp'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    4,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_StateQTimeTag :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_19[4] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    8,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TimeStamp, 	/* comp_name_ptr 'TimeStamp'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TIMESTAMP_STRING_INDEX, 	/* name_index 'TimeStamp'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    4,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_DiscreteQTimeTag :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_20[5] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    12,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Value, 	/* comp_name_ptr 'Value'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_VALUE_STRING_INDEX, 	/* name_index 'Value'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TimeStamp, 	/* comp_name_ptr 'TimeStamp'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TIMESTAMP_STRING_INDEX, 	/* name_index 'TimeStamp'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    4,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_RealExtended :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_21[6] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    12,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_FLOATING_POINT,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Value, 	/* comp_name_ptr 'Value'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_VALUE_STRING_INDEX, 	/* name_index 'Value'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TimeStamp, 	/* comp_name_ptr 'TimeStamp'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TIMESTAMP_STRING_INDEX, 	/* name_index 'TimeStamp'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    2,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_UNSIGNED,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_COV, 	/* comp_name_ptr 'COV'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_COV_STRING_INDEX, 	/* name_index 'COV'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[5] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_StateExtended :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_22[5] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    8,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TimeStamp, 	/* comp_name_ptr 'TimeStamp'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TIMESTAMP_STRING_INDEX, 	/* name_index 'TimeStamp'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    2,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_UNSIGNED,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_COV, 	/* comp_name_ptr 'COV'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_COV_STRING_INDEX, 	/* name_index 'COV'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_DiscreteExtended :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_23[6] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    12,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Value, 	/* comp_name_ptr 'Value'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_VALUE_STRING_INDEX, 	/* name_index 'Value'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TimeStamp, 	/* comp_name_ptr 'TimeStamp'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TIMESTAMP_STRING_INDEX, 	/* name_index 'TimeStamp'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    2,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_UNSIGNED,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_COV, 	/* comp_name_ptr 'COV'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_COV_STRING_INDEX, 	/* name_index 'COV'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[5] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_RealQTimeTagExtended :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_24[8] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    16,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      6,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_FLOATING_POINT,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Value, 	/* comp_name_ptr 'Value'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_VALUE_STRING_INDEX, 	/* name_index 'Value'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    8,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TimeStamp, 	/* comp_name_ptr 'TimeStamp'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TIMESTAMP_STRING_INDEX, 	/* name_index 'TimeStamp'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_GMTBasedS, 	/* comp_name_ptr 'GMTBasedS'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_GMTBASEDS_STRING_INDEX, 	/* name_index 'GMTBasedS'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Milliseconds, 	/* comp_name_ptr 'Milliseconds'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_MILLISECONDS_STRING_INDEX, 	/* name_index 'Milliseconds'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[5] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[6] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    4,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[7] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      6,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_StateQTimeTagExtended :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_25[7] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    12,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      5,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    8,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TimeStamp, 	/* comp_name_ptr 'TimeStamp'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TIMESTAMP_STRING_INDEX, 	/* name_index 'TimeStamp'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_GMTBasedS, 	/* comp_name_ptr 'GMTBasedS'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_GMTBASEDS_STRING_INDEX, 	/* name_index 'GMTBasedS'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Milliseconds, 	/* comp_name_ptr 'Milliseconds'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_MILLISECONDS_STRING_INDEX, 	/* name_index 'Milliseconds'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[5] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    4,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[6] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      5,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_DiscreteQTimeTagExtended :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_26[8] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    16,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      6,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Value, 	/* comp_name_ptr 'Value'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_VALUE_STRING_INDEX, 	/* name_index 'Value'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    8,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TimeStamp, 	/* comp_name_ptr 'TimeStamp'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TIMESTAMP_STRING_INDEX, 	/* name_index 'TimeStamp'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_GMTBasedS, 	/* comp_name_ptr 'GMTBasedS'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_GMTBASEDS_STRING_INDEX, 	/* name_index 'GMTBasedS'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Milliseconds, 	/* comp_name_ptr 'Milliseconds'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_MILLISECONDS_STRING_INDEX, 	/* name_index 'Milliseconds'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[5] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[6] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    4,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[7] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      6,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_StateSupplemental :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_27[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_StateSupplementalQ :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_28[4] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Value, 	/* comp_name_ptr 'Value'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_VALUE_STRING_INDEX, 	/* name_index 'Value'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_StateSupplementalQTimeTag :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_29[5] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    12,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    4,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Value, 	/* comp_name_ptr 'Value'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_VALUE_STRING_INDEX, 	/* name_index 'Value'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TimeStamp, 	/* comp_name_ptr 'TimeStamp'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TIMESTAMP_STRING_INDEX, 	/* name_index 'TimeStamp'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    4,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_StateSupplementalExtended :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_30[6] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    12,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    4,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Value, 	/* comp_name_ptr 'Value'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_VALUE_STRING_INDEX, 	/* name_index 'Value'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TimeStamp, 	/* comp_name_ptr 'TimeStamp'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TIMESTAMP_STRING_INDEX, 	/* name_index 'TimeStamp'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    2,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_UNSIGNED,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_COV, 	/* comp_name_ptr 'COV'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_COV_STRING_INDEX, 	/* name_index 'COV'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[5] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Data_StateSupplementalQTimeTagExtended :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_31[8] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    16,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      6,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    4,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Value, 	/* comp_name_ptr 'Value'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_VALUE_STRING_INDEX, 	/* name_index 'Value'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    8,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TimeStamp, 	/* comp_name_ptr 'TimeStamp'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TIMESTAMP_STRING_INDEX, 	/* name_index 'TimeStamp'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_GMTBasedS, 	/* comp_name_ptr 'GMTBasedS'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_GMTBASEDS_STRING_INDEX, 	/* name_index 'GMTBasedS'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Milliseconds, 	/* comp_name_ptr 'Milliseconds'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_MILLISECONDS_STRING_INDEX, 	/* name_index 'Milliseconds'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[5] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[6] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    4,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[7] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      6,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Control_Command :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_32[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Control_Setpoint_Real :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_33[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_FLOATING_POINT,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* Control_Setpoint_Discrete :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_34[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* SBO_CheckBackName :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_35[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    2,				/* el_size				*/
    2,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* TagValue :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_36[4] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    257,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    1,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      3,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Flags, 	/* comp_name_ptr 'Flags'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_FLAGS_STRING_INDEX, 	/* name_index 'Flags'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_VISIBLE_STRING,		/* el_tag				*/
    256,				/* el_size				*/
    256,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      -255,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Reason, 	/* comp_name_ptr 'Reason'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_REASON_STRING_INDEX, 	/* name_index 'Reason'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      2,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* InfoMessHeader :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_37[6] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    16,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_InfoReference, 	/* comp_name_ptr 'InfoReference'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_INFOREFERENCE_STRING_INDEX, 	/* name_index 'InfoReference'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_LocalReference, 	/* comp_name_ptr 'LocalReference'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_LOCALREFERENCE_STRING_INDEX, 	/* name_index 'LocalReference'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_MessageId, 	/* comp_name_ptr 'MessageId'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_MESSAGEID_STRING_INDEX, 	/* name_index 'MessageId'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Size, 	/* comp_name_ptr 'Size'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_SIZE_STRING_INDEX, 	/* name_index 'Size'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[5] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* InfoBuffXXX :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_38[1] =
  {
    {				/* rt[0] init data ...			*/
    RT_OCTET_STRING,		/* el_tag				*/
    128,				/* el_size				*/
    128,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      128,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/
/* TAAccountRequest :  */

SD_CONST static RUNTIME_TYPE mvl_rt_table_39[7] =
  {
    {				/* rt[0] init data ...			*/
    RT_STR_START,		/* el_tag				*/
    0,				/* el_size				*/
    20,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      5,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[1] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TransferAccountReference, 	/* comp_name_ptr 'TransferAccountReference'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TRANSFERACCOUNTREFERENCE_STRING_INDEX, 	/* name_index 'TransferAccountReference'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[2] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_StartTime, 	/* comp_name_ptr 'StartTime'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_STARTTIME_STRING_INDEX, 	/* name_index 'StartTime'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[3] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_Duration, 	/* comp_name_ptr 'Duration'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_DURATION_STRING_INDEX, 	/* name_index 'Duration'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[4] init data ...			*/
    RT_INTEGER,		/* el_tag				*/
    4,				/* el_size				*/
    4,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      4,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_RequestId, 	/* comp_name_ptr 'RequestId'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_REQUESTID_STRING_INDEX, 	/* name_index 'RequestId'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[5] init data ...			*/
    RT_BIT_STRING,		/* el_tag				*/
    4,				/* el_size				*/
    1,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      8,			/*   el_len				*/
      0			/*   pad				*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    mvlCompName_TaConditionsRequested, 	/* comp_name_ptr 'TaConditionsRequested'				*/
#else	/* !USE_RT_TYPE_2	*/
    FO_TACONDITIONSREQUESTED_STRING_INDEX, 	/* name_index 'TaConditionsRequested'				*/
#endif	/* !USE_RT_TYPE_2	*/
    },
    {				/* rt[6] init data ...			*/
    RT_STR_END,		/* el_tag				*/
    0,				/* el_size				*/
    0,				/* offset_to_last			*/
      {				/* u					*/
      {				/* p					*/
      5,			/*   num_rt_blks				*/
      0			/*   pad					*/
      }				/* end 'p'				*/
      },			/* end 'u'				*/
#ifdef USE_RT_TYPE_2
    NULL,				/* comp_name_ptr 				*/
#else	/* !USE_RT_TYPE_2	*/
    0,				/* name_index 				*/
#endif	/* !USE_RT_TYPE_2	*/
    }
  };

/************************************************************************/


ST_INT rt_table_index;

SD_CONST RUNTIME_TYPE * SD_CONST mvl_rt_tables[] =
  {
  mvl_rt_table_0,
  mvl_rt_table_1,
  mvl_rt_table_2,
  mvl_rt_table_3,
  mvl_rt_table_4,
  mvl_rt_table_5,
  mvl_rt_table_6,
  mvl_rt_table_7,
  mvl_rt_table_8,
  mvl_rt_table_9,
  mvl_rt_table_10,
  mvl_rt_table_11,
  mvl_rt_table_12,
  mvl_rt_table_13,
  mvl_rt_table_14,
  mvl_rt_table_15,
  mvl_rt_table_16,
  mvl_rt_table_17,
  mvl_rt_table_18,
  mvl_rt_table_19,
  mvl_rt_table_20,
  mvl_rt_table_21,
  mvl_rt_table_22,
  mvl_rt_table_23,
  mvl_rt_table_24,
  mvl_rt_table_25,
  mvl_rt_table_26,
  mvl_rt_table_27,
  mvl_rt_table_28,
  mvl_rt_table_29,
  mvl_rt_table_30,
  mvl_rt_table_31,
  mvl_rt_table_32,
  mvl_rt_table_33,
  mvl_rt_table_34,
  mvl_rt_table_35,
  mvl_rt_table_36,
  mvl_rt_table_37,
  mvl_rt_table_38,
  mvl_rt_table_39
  };


#endif /* #if defined USR_SUPPLIED_RT */

