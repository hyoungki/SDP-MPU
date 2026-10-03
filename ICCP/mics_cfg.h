/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*      	      1998, All Rights Reserved		        	*/
/*									*/
/* MODULE NAME : mics_cfg.h    						*/
/* PRODUCT(S)  : MMSEASE-LITE						*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*	NONE								*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 02/10/10  NAV     11    Add StateSupplemental types			*/
/* 05/17/07  LWP     10	   Added MISU_IM and MISU_IM_ASSOC for IM	*/
/* 03/29/06  RKR     09    added mic_ds_in_use                          */
/* 07/26/05  MDE     08    2000-08 work					*/
/* 06/28/05  MDE     07    Added MI_ICFG_USR_CTRL			*/
/* 05/26/05  MDE     06    Added support for user developed ICFG	*/
/* 03/31/04  MDE     05    Removed Remote DV validation			*/
/* 05/07/03  MDE     04    Added assocName to MICU_DSTS			*/
/* 06/20/02  MDE     03    Removed ICFG naming				*/
/* 05/21/02  MDE     02    Added usr pointers				*/
/* 04/08/02  MDE     01    New file					*/
/************************************************************************/

#ifndef MICS_CFG_INCLUDED
#define MICS_CFG_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

	/************************************************************************/

#include "mi.h"
#include "icfg.h"

	/************************************************************************/
	/************************************************************************/

#define MIU_CFG_SOURCE_XML		0
#define MIU_CFG_SOURCE_USR_CLIENT	1
#define MIU_CFG_SOURCE_USR_SERVER	2
#define MIU_CFG_SOURCE_DB		3

	ST_RET miu_icfg_load (ST_INT source, ST_INT mode, ST_CHAR *remoteName);

	/************************************************************************/
	/* MIS USR Data Structures						*/
	/* These structures are used to store the server side information about	*/
	/* Data Values & Devices.						*/

	/* Note that many of these data structures contain an element:		*/
	/*	 ICFG_REF   icfgRef; 						*/
	/* This is a copy of the icfgRef element of the ICFG data structures,	*/
	/* and contains reference information for the ICCP element that may be	*/
	/* useful for runtime linkage of the ICCP element with a real source	*/
	/* or destination. 							*/
	/* The default XML ICFG subsystem does not make use of this element	*/
	/************************************************************************/
	/************************************************************************/

	typedef struct
	{
		DBL_LNK l;			/* Linked list of data values */
		ST_VOID *usr;
		ST_INT mi_type;
		struct 
		{
			union 
			{
				ST_FLOAT r;	/* Real */
				ST_INT32 d;	/* Discrete */
				ST_UCHAR ss;	/* state supplemental */
			} Value;
			ST_UCHAR Flags;
			TIMESTAMP_EX TimeStamp;
			ST_UINT16 COV;
		} data;

		MIS_DV_REF dvRef;
		ICFG_REF   icfgRef;		// 2016.05.11 ChoiBC Modify : icfgRef.ref is used for Mapping
	} MISU_DV;
	extern MISU_DV *misu_dv_list;

	typedef struct
	{
		DBL_LNK l;			/* Linked list of devices */
		ST_VOID *usr;
		ST_INT dev_type;
		ST_LONG selTime;  
		MIS_DEVICE_REF devRef;
		ICFG_REF   icfgRef;
	} MISU_DEV;
	extern MISU_DEV *misu_dev_list;

	typedef struct
	{
		DBL_LNK l;
		MI_ASSOC_CTRL *mi_assoc;
		enum    icfgScopeVal scope;
	}MISU_IM_ASSOC;

	typedef struct
	{
		DBL_LNK l;
		ST_VOID *usr;
		ST_INT32 InfoReference;			      
		ST_INT32 LocalReference;
		ST_INT32 MessageId;
		ST_CHAR mapInfo[ICFG_MAX_MAPINFO_LEN+1];
		ST_LONG maxSize; 
		MISU_IM_ASSOC *hol_mi_assoc;
		ST_INT32 numAssoc;

	} MISU_IM;
	extern MISU_IM *misu_im_list;

	/************************************************************************/
	/************************************************************************/
	/* MIC USR Data Structures						*/
	/* These structures are used to store the client side information to	*/
	/* be used for creating the Data Sets and Data Set Transfer Sets in	*/
	/* remote nodes when a connection has been established.			*/
	/************************************************************************/

	typedef struct
	{
		DBL_LNK l;			/* Linked list of data values */
		ST_VOID *usr;
		ST_INT mi_type;
		union 			/* Data storage for all DV types */
		{
			MI_REAL 			r;
			MI_STATE 	   		s;
			MI_DISCRETE 		d;
			MI_STATE_SUPP		ss;
			MI_REAL_Q 	   		rq;
			MI_STATE_Q 	   		sq;
			MI_DISCRETE_Q 		dq;
			MI_STATE_SUPP_Q		ssq;
			MI_REAL_Q_TIMETAG 		rqt;
			MI_STATE_Q_TIMETAG    	sqt;
			MI_DISCRETE_Q_TIMETAG 	dqt;
			MI_STATE_SUPP_Q_TIMETAG     ssqt;
			MI_REAL_EXTENDED 		re;
			MI_STATE_EXTENDED    	se;
			MI_DISCRETE_EXTENDED 	de;
			MI_STATE_SUPP_EXTENDED	sse;
			MI_REAL_Q_TIMETAG_EXTENDED 	     rqte;
			MI_STATE_Q_TIMETAG_EXTENDED      sqte;
			MI_DISCRETE_Q_TIMETAG_EXTENDED   dqte;
			MI_STATE_SUPP_Q_TIMETAG_EXTENDED ssqte;
		} data;
		MIC_DV *mic_dv;
		ST_UINT32 ir_idx;	/* for redundancy support */
		ICFG_REF   icfgRef;
	} MICU_DV;

	/* Data Set */
	typedef struct
	{
		DBL_LNK       l;		       
		ST_VOID      *usr;
		ST_CHAR       ds_name[MAX_IDENT_LEN+1];		
		ST_INT        ds_scope;
		ST_INT        num_var; 
		MIC_DSTS_VAR *var_tbl;  

		ST_BOOLEAN mic_ds_ok;
		ST_BOOLEAN mic_ds_in_use;
		MIC_DS  *mic_ds;
		ICFG_REF   icfgRef;
	} MICU_DS;

	/* Data Set Transfer Set */
	typedef struct 
	{
		DBL_LNK	 l;
		ST_VOID       *usr;
		ST_CHAR        assocName[MAX_IDENT_LEN+1];
		ST_CHAR	 dsTsName[MAX_IDENT_LEN+1];
		MI_DSTS_DATA   dsts_data;
		MICU_DS	*miu_ds;

		MIC_DSTS *mic_dsts;
		ICFG_REF   icfgRef;
	} MICU_DSTS;

	/* Devices */
	typedef struct
	{
		DBL_LNK l;			/* Linked list of data values */
		ST_VOID *usr;
		ST_INT dev_type;
		ST_BOOLEAN sbo;
		MIC_DEVICE *mic_dev;
		ICFG_REF   icfgRef;
	} MICU_DEV;

	/* Remotes */
	typedef struct
	{
		DBL_LNK l;		       
		ST_VOID *usr;
		ST_CHAR name[MAX_IDENT_LEN+1];
		MICU_DV	*dv_list;	/* List of remote data values 		*/
		MICU_DS   	*ds_list;  	/* List of DS to create at connect time	*/
		MICU_DSTS 	*dsts_list;	/* List of remote DSTS	 		*/
		MICU_DEV	*dev_list ;	/* List of remote devices 		*/

		MI_REMOTE *mi_remote;
		ICFG_REF   icfgRef;
	} MIU_REMOTE;

	typedef struct
	{
		MIU_REMOTE *miuCurrRemote;
	} MI_ICFG_USR_CTRL;

	/* List of remotes */
	extern MIU_REMOTE *miu_remote_list;

	MIU_REMOTE *miuFindRemote (ST_CHAR *remoteName);
	MIU_REMOTE *miuFindMiRemote (MI_REMOTE *mi_remote);

	MICU_DV *micuFindDv (MIU_REMOTE *mi_icfg_remote, 
		ST_CHAR *dvName, ST_INT scope);
	MICU_DS    *micuFindDs (MIU_REMOTE *miuRemote, ST_CHAR *dsName);
	MICU_DSTS  *micuFindDsTs (MIU_REMOTE *miuRemote, ST_CHAR *dsTsName);


	/************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* MICS_CFG_INCLUDED */
/************************************************************************/

