/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*      	1998 - 2004, All Rights Reserved		        */
/*									*/
/* MODULE NAME : mis_usr.c  						*/
/* PRODUCT(S)  : ICCP TASE.2 Extensions for MMS-EASE Lite		*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 07/17/10  NAV           Klocwork Changes				*/
/* 02/10/10  NAV     35    Add StateSupplemental types			*/
/* 01/21/10  RKR     33	   added tag checking to operate indication     */
/* 01/12/10  JRB     34    Fix log messages.				*/
/* 08/28/09  NAV     33    compatability mode & CurveType changes	*/
/* 08/27/09  NAV     32    #1610 change float increment for floatMatrix	*/
/* 08/26/09  NAV     31    Add (long) cast for Linux			*/
/* 08/15/09  NAV     30    Add Block 8 Support				*/
/* 08/25/08  RKR     29    cleaned up warning                           */
/* 05/17/07  LWP     28	   reworked misuSendMessage			*/
/* 06/23/06  MDE     27    VS2005 porting changes			*/
/* 03/29/06  RKR     26    changed gettag to use miuAlwaysChangeDev     */
/* 12/14/05  MDE     25    Added ACR test				*/
/* 12/07/05  MDE     24    Check return values from mis_change_xxx_ex	*/
/* 11/01/05  MDE     23    Device (GetTag) handling corrections		*/
/* 10/26/05  MDE     22    Tweaked auto change				*/
/* 07/26/05  MDE     21    2000-08 work					*/
/* 06/07/05  MDE     20    Linux warning cleanup			*/
/* 04/29/05  MDE     19    MIU_IR naming update				*/
/* 04/25/05  MDE     18    General cleanup 				*/
/* 04/06/05  MDE     17    Added quiet mode				*/
/* 02/10/04  MDE     16	   Redundancy cleanup				*/
/* 08/09/04  EJV     15	   Added typecast in strlen call.		*/
/* 01/20/04  MDE     14	   Added redundancy support code		*/
/* 12/03/03  MDE     13    Linux warning cleanup			*/
/* 12/02/03  MDE     12	   Minor tweak to u_mis_rd_dv			*/
/* 10/06/03  EJV     11    u_mis_rd_dv: cor ret to SD_FAILURE if miuDv=0*/
/* 06/20/02  RKR     10    Copyright year update                        */
/* 06/20/02  MDE     09	   Use cleaned up mics_icfg names		*/
/* 04/16/02  MDE     08	   Moved miu_dv_list, miu_dev_list to mics_cfg	*/
/* 01/25/02  MDE     07	   Added redundancy support code		*/
/* 12/07/01  MDE     06	   Don't alloc 0 size conn array for no assoc	*/
/* 10/29/01  MDE     05	   Added USE_ICFG code				*/
/* 10/19/01  MDE     04	   Dynamic local object reconfiguration changes	*/
/* 10/11/99  NAV     03    Add Block 4 and 5 support			*/
/* 04/01/99  MDE     02    Use MI_ASSOC_CTRL for connection management	*/
/* 12/31/98  MDE     01    Created					*/
/************************************************************************/

#include "glbtypes.h"
#include "sysincs.h"
#include "mi_usr.h"
#if defined(_WIN32)		// 2016.04.26 ChoiBC Modify
#include "miusrobj.WIN.h"
#elif defined(_PLATFORM_LINUX)
#include "miusrobj.LINUX.h"
#else
#include "miusrobj.PPC.h"
#endif
#include "mi_icfg.h"
#include "mics_cfg.h"

#if 1	// 2016.05.12 ChoiBC Modify : DataValue Read 요구에 대한 응답
#include "iccpShm.h"
#endif

extern  ICCP_60870_DCB  *iccpInfo;                 // ICCP-HOST 참조용 : 모니터링 구조체

/************************************************************************/

#ifdef DEBUG_SISCO
static ST_CHAR *thisFileName = __FILE__;
#endif

/************************************************************************/
/* SAMPLE MI SERVER CODE 						*/
/************************************************************************/

#if 0	// CHOIBC DELETE
static ST_INT misuChangeDv (MISU_DV *miuDv, ST_BOOLEAN changeValueOnly);
static ST_INT misuChangeDev (MISU_DEV *miuDev, ST_BOOLEAN changeValueOnly);
#endif

/************************************************************************/
/* DATA VALUE HANDLING							*/

/* In our sample a configurable number of data points. The data storgae	*/
/* for these data points will be is stored locally in memory in 	*/
/* structures of type 'MISU_DV', which is defined in mi_usr.h and 	*/
/* contains a union of all extended data types.				*/

/* We will use the MIS_DV_ACCESS_INFO as a ST_VOID * that will point	*/
/* to the MISU_DV, which makes it easy to retrieve the data as required */

/************************************************************************/
/************************************************************************/
/* USER DEFINED DATA ACCESS FUNCTIONS 					*/
/************************************************************************/
/************************************************************************/
/*			misu_get_dv_data				*/
/************************************************************************/

/* This function transforms the local storage form of DV data to that	*/
/* required by ICCP Lite. In this sample our local storage is MISU_DV	*/

ST_RET misu_get_dv_data (MISU_DV *miuDv, ST_VOID *data_buf)
{
	MI_REAL 		*rDest; /* Destination data pointers		*/
	MI_STATE 	   	*sDest;
	MI_DISCRETE 	   	*dDest;
	MI_STATE_SUPP		*ssDest;
	MI_REAL_Q 	   	*rqDest;
	MI_STATE_Q 	   	*sqDest;
	MI_DISCRETE_Q 	   	*dqDest;
	MI_STATE_SUPP_Q		*ssqDest;
	MI_REAL_Q_TIMETAG 	*rqtDest;
	MI_STATE_Q_TIMETAG    	*sqtDest;
	MI_DISCRETE_Q_TIMETAG 	*dqtDest;
	MI_STATE_SUPP_Q_TIMETAG *ssqtDest;
	MI_REAL_EXTENDED 	*reDest;
	MI_STATE_EXTENDED    	*seDest;
	MI_DISCRETE_EXTENDED 	*deDest;
	MI_STATE_SUPP_EXTENDED  *sseDest;
	MI_REAL_Q_TIMETAG_EXTENDED 	 *rqteDest;
	MI_STATE_Q_TIMETAG_EXTENDED    	 *sqteDest;
	MI_DISCRETE_Q_TIMETAG_EXTENDED 	 *dqteDest;
	MI_STATE_SUPP_Q_TIMETAG_EXTENDED *ssqteDest;
	ST_RET rc;

	/* For our sample, the MIS_DV (Data Value control) has the 		*/
	/* 'access_info' element set to point to the data structure containing	*/
	/* all information about the point. We will use this to fill in the 	*/
	/* destination data, depending on the types involved.			*/

	rc = SD_SUCCESS;

	/* Make sure this one that we have linked, and if not fail ..		*/
	if (miuDv == NULL)
		return (SD_FAILURE);

	switch (miuDv->mi_type)
	{
	case MI_TYPEID_STATE_SUPP_Q_TIMETAG_EXTENDED:
		ssqteDest = (MI_STATE_SUPP_Q_TIMETAG_EXTENDED *) data_buf;
		ssqteDest->TimeStamp = miuDv->data.TimeStamp;
		ssqteDest->Flags = miuDv->data.Flags;
		ssqteDest->Value = miuDv->data.Value.ss;
		break;

	case MI_TYPEID_REAL_Q_TIMETAG_EXTENDED:
		rqteDest = (MI_REAL_Q_TIMETAG_EXTENDED *) data_buf;
		rqteDest->Value = miuDv->data.Value.r;
		rqteDest->TimeStamp = miuDv->data.TimeStamp;
		rqteDest->Flags = miuDv->data.Flags;
		break;

	case MI_TYPEID_REAL:
		rDest = (MI_REAL *) data_buf;
		*rDest = miuDv->data.Value.r;
		break;

	case MI_TYPEID_STATE:
		sDest = (MI_STATE *) data_buf;
		*sDest = miuDv->data.Flags;
		break;

	case MI_TYPEID_DISCRETE:
		dDest = (MI_DISCRETE *) data_buf;
		*dDest = miuDv->data.Value.d;
		break;

	case MI_TYPEID_STATE_SUPP:
		ssDest = (MI_STATE_SUPP *) data_buf;
		*ssDest = miuDv->data.Value.ss;
		break;

	case MI_TYPEID_REAL_Q:
		rqDest = (MI_REAL_Q *) data_buf;
		rqDest->Value = miuDv->data.Value.r;
		rqDest->Flags = miuDv->data.Flags;
		break;

	case MI_TYPEID_STATE_Q:
		sqDest = (MI_STATE_Q *)  data_buf;
		*sqDest = miuDv->data.Flags;
		break;

	case MI_TYPEID_DISCRETE_Q:
		dqDest = (MI_DISCRETE_Q *) data_buf;
		dqDest->Value = miuDv->data.Value.d;
		dqDest->Flags = miuDv->data.Flags;
		break;

	case MI_TYPEID_STATE_SUPP_Q:
		ssqDest = (MI_STATE_SUPP_Q *)  data_buf;
		ssqDest->Flags = miuDv->data.Flags;
		ssqDest->Value = miuDv->data.Value.ss;
		break;

	case MI_TYPEID_REAL_Q_TIMETAG:
		rqtDest = (MI_REAL_Q_TIMETAG *) data_buf;
		rqtDest->Value = miuDv->data.Value.r;
		rqtDest->TimeStamp = miuDv->data.TimeStamp.GMTBasedS;
		rqtDest->Flags = miuDv->data.Flags;
		break;

	case MI_TYPEID_STATE_Q_TIMETAG:
		sqtDest = (MI_STATE_Q_TIMETAG *) data_buf;
		sqtDest->TimeStamp = miuDv->data.TimeStamp.GMTBasedS;
		sqtDest->Flags = miuDv->data.Flags;
		break;

	case MI_TYPEID_DISCRETE_Q_TIMETAG:
		dqtDest = (MI_DISCRETE_Q_TIMETAG *) data_buf;
		dqtDest->Value = miuDv->data.Value.d;
		dqtDest->TimeStamp = miuDv->data.TimeStamp.GMTBasedS;
		dqtDest->Flags = miuDv->data.Flags;
		break;

	case MI_TYPEID_STATE_SUPP_Q_TIMETAG:
		ssqtDest = (MI_STATE_SUPP_Q_TIMETAG *) data_buf;
		ssqtDest->TimeStamp = miuDv->data.TimeStamp.GMTBasedS;
		ssqtDest->Flags = miuDv->data.Flags;
		ssqtDest->Value = miuDv->data.Value.ss;
		break;

	case MI_TYPEID_REAL_EXTENDED:
		reDest = (MI_REAL_EXTENDED *) data_buf;
		reDest->Value = miuDv->data.Value.r;
		reDest->TimeStamp = miuDv->data.TimeStamp.GMTBasedS;
		reDest->Flags = miuDv->data.Flags;
		reDest->COV = miuDv->data.COV;
		break;

	case MI_TYPEID_STATE_EXTENDED:
		seDest = (MI_STATE_EXTENDED *) data_buf;
		seDest->TimeStamp = miuDv->data.TimeStamp.GMTBasedS;
		seDest->Flags = miuDv->data.Flags;
		seDest->COV = miuDv->data.COV;
		break;

	case MI_TYPEID_DISCRETE_EXTENDED:
		deDest = (MI_DISCRETE_EXTENDED *) data_buf;
		deDest->Value = miuDv->data.Value.d;
		deDest->TimeStamp = miuDv->data.TimeStamp.GMTBasedS;
		deDest->Flags = miuDv->data.Flags;
		deDest->COV = miuDv->data.COV;
		break;

	case MI_TYPEID_STATE_SUPP_EXTENDED:
		sseDest = (MI_STATE_SUPP_EXTENDED *) data_buf;
		sseDest->TimeStamp = miuDv->data.TimeStamp.GMTBasedS;
		sseDest->Flags = miuDv->data.Flags;
		sseDest->COV = miuDv->data.COV;
		sseDest->Value = miuDv->data.Value.ss;
		break;

	case MI_TYPEID_STATE_Q_TIMETAG_EXTENDED:
		sqteDest = (MI_STATE_Q_TIMETAG_EXTENDED *) data_buf;
		sqteDest->TimeStamp = miuDv->data.TimeStamp;
		sqteDest->Flags = miuDv->data.Flags;
		break;

	case MI_TYPEID_DISCRETE_Q_TIMETAG_EXTENDED:
		dqteDest = (MI_DISCRETE_Q_TIMETAG_EXTENDED *) data_buf;
		dqteDest->Value = miuDv->data.Value.d;
		dqteDest->TimeStamp = miuDv->data.TimeStamp;
		dqteDest->Flags = miuDv->data.Flags;
		break;

	default:
		printf ("\nError - Check Me Out!") ;
		rc = SD_FAILURE;
		break;
	}
	return (rc);
}

/*
* 2026-02-24 오전 9:23:53  이름은 dv에  값은 pData의 ..이것을 iccpPtr에 기록한다.
*/
int check_iccp_report(MIS_DV *dv, ICCP_POINT_DATA *pData)
{
    char *bfptr, temp1[4], temp2[4];
    int  pointIndex;
    I60870_DATA *iccpPtr;

#ifdef	VITZRO_FEP_ENABLE
	/* ------------------------------------------------ */
	/*  2017-11-08, LHS...                              */
	/*  - ICCP Report 정보 확인...                      */
	/* ------------------------------------------------ */
	bfptr = (char *) dv->dv_name;
    memcpy(temp1, & bfptr[0], 3);
    memcpy(temp2, &bfptr[3], 4);
    pointIndex = atoi(temp2) - 1; // 비츠로는 이름의 4자리ㅇ군.
    
    if(pointIndex < 0)  return (0);
    if(memcmp(temp1, "SDI", 3) == 0)        iccpPtr = (I60870_DATA *) &iccpInfo->iccp_sdi[pointIndex];
    else if(memcmp(temp1, "SAI", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_sai[pointIndex];
    else if(memcmp(temp1, "DDI", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_ddi[pointIndex]; 
    else if(memcmp(temp1, "DAI", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_dai[pointIndex];  
    else if(memcmp(temp1, "QDI", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_qdi[pointIndex]; 
    else if(memcmp(temp1, "QAI", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_qai[pointIndex];          
    else if(memcmp(temp1, "TDI", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_tdi[pointIndex]; 
    else if(memcmp(temp1, "TAI", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_tai[pointIndex]; 
    else if(memcmp(temp1, "CDS", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_dev[pointIndex];
    else
    {
        return (0);
    }           
#else    
	/* ------------------------------------------------ */
	/*  2017-11-08, LHS...                              */
	/*  - ICCP Report 정보 확인...                      */
	/* ------------------------------------------------ */
	bfptr = (char *) dv->dv_name;
    memcpy(temp1, & bfptr[0], 3);
    memcpy(temp2, &bfptr[3], 3);
    pointIndex = atoi(temp2) - 1;
    
    if(pointIndex < 0)  return (0);
    if(memcmp(temp1, "SDI", 3) == 0)        iccpPtr = (I60870_DATA *) &iccpInfo->iccp_sdi[pointIndex];
    else if(memcmp(temp1, "SAI", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_sai[pointIndex];
    else if(memcmp(temp1, "DDI", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_ddi[pointIndex]; 
    else if(memcmp(temp1, "DAI", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_dai[pointIndex];  
    else if(memcmp(temp1, "QDI", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_qdi[pointIndex]; 
    else if(memcmp(temp1, "QAI", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_qai[pointIndex];          
    else if(memcmp(temp1, "TDI", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_tdi[pointIndex]; 
    else if(memcmp(temp1, "TAI", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_tai[pointIndex]; 
    else if(memcmp(temp1, "DEV", 3) == 0)   iccpPtr = (I60870_DATA *) &iccpInfo->iccp_dev[pointIndex];
    else
    {
        return (0);
    }                 
#endif
    
    iccpPtr->report =  pData->value;        // 현재 값...
    iccpPtr->flag   =  pData->flag;         //  Offline 상태
    iccpPtr->accessTick++;                  // ICCP HOST 전송 Count...증가
    
    /* ---------------------------------------- */
	/*	상위전송 포인트 ... Display 제한 				*/  
	/* ---------------------------------------- */  
    if(pointIndex >= 8)  return (2);
    	
    	
    return(1); 
}

#if 1	// 2016.05.12 ChoiBC Modify : DataValue Read 요구에 대한 응답, 값이 바뀐경우 호출
// set pointData to miudv 인가..
static void _SetPointData2Miudv (MISU_DV *miuDv, ICCP_POINT_DATA *pData, ST_BOOLEAN bSoe)
{
    
   	int	retVal;
   	     
#if 0	// 2016.11 ChoiBC Modify : SOE
	switch (miuDv->mi_type)
	{
	case MI_TYPEID_REAL:
	case MI_TYPEID_REAL_Q:
	case MI_TYPEID_REAL_Q_TIMETAG:
	case MI_TYPEID_REAL_EXTENDED:
	case MI_TYPEID_REAL_Q_TIMETAG_EXTENDED:
		miuDv->data.Value.r = pData->value;
		break;

	case MI_TYPEID_DISCRETE:
	case MI_TYPEID_DISCRETE_Q:
	case MI_TYPEID_DISCRETE_Q_TIMETAG:
	case MI_TYPEID_DISCRETE_EXTENDED:
	case MI_TYPEID_DISCRETE_Q_TIMETAG_EXTENDED:
		miuDv->data.Value.d = (ST_INT32) pData->value;
		break;

	case MI_TYPEID_STATE:
	case MI_TYPEID_STATE_Q:
	case MI_TYPEID_STATE_Q_TIMETAG:
	case MI_TYPEID_STATE_EXTENDED:
	case MI_TYPEID_STATE_Q_TIMETAG_EXTENDED:
		if (pData->value == 0)
		{
			MI_SET_DATA_STATE (miuDv->data.Flags, MI_DATA_STATE_OFF);
		}
		else
		{
			MI_SET_DATA_STATE (miuDv->data.Flags, MI_DATA_STATE_ON);
		}
		break;

	case MI_TYPEID_STATE_SUPP:
	case MI_TYPEID_STATE_SUPP_Q:
	case MI_TYPEID_STATE_SUPP_Q_TIMETAG:
	case MI_TYPEID_STATE_SUPP_EXTENDED:
	case MI_TYPEID_STATE_SUPP_Q_TIMETAG_EXTENDED:

		if (pData->value == 0)
		{
			MI_SET_DATA_STATE (miuDv->data.Flags, MI_DATA_STATE_OFF);
			MI_SET_DATA_STATE (miuDv->data.Value.ss, MI_DATA_STATE_OFF);
		}
		else
		{
			MI_SET_DATA_STATE (miuDv->data.Flags, MI_DATA_STATE_ON);
			MI_SET_DATA_STATE (miuDv->data.Value.ss, MI_DATA_STATE_ON);
		}
		break;
	}
#else	// 2016.11 ChoiBC Modify : SOE
	switch (miuDv->mi_type)
	{
	case MI_TYPEID_REAL_Q_TIMETAG_EXTENDED:                 // 포인트 정보...?
		miuDv->data.Value.r = pData->value;
		break;

	case MI_TYPEID_STATE_SUPP_Q_TIMETAG_EXTENDED:           // 이벤트 정보...?
#ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
		if (pData->value == 0)
		{
			miuDv->data.Value.ss = 0;
		}
		else
		{
			miuDv->data.Value.ss = 1;
		}
#else
		if (pData->value == 0)
		{
			MI_SET_DATA_STATE (miuDv->data.Flags, MI_DATA_STATE_OFF);
			MI_SET_DATA_STATE (miuDv->data.Value.ss, MI_DATA_STATE_OFF);
		}
		else
		{
			MI_SET_DATA_STATE (miuDv->data.Flags, MI_DATA_STATE_ON);
			MI_SET_DATA_STATE (miuDv->data.Value.ss, MI_DATA_STATE_ON);
		}
		if (bSoe)
			miuDv->data.Value.ss |= MI_DATA_STATE_SUPP_SOE;
		else
			miuDv->data.Value.ss &= ~MI_DATA_STATE_SUPP_SOE;
#endif
		break;
	default:
		return;
	}
#endif	// 2016.11 ChoiBC Modify : SOE

	miuDv->data.TimeStamp.GMTBasedS    = (ST_INT32) pData->updateTime.tv_sec;
	miuDv->data.TimeStamp.Milliseconds = pData->updateTime.tv_usec/1000;
	MI_QSET_TIMESTAMP (miuDv->data.Flags, MI_QFLAG_TIMESTAMP_VALID);

	/* ------------------------------------------------- */
	/*	2020.09.04 최병철 소장님 가이드 ....					*/
	/* 	비츠로시스 -  Flags  : (0: Offline, 1: Online)		*/
	/*	에이스   -  Flags  : (2: Offline, 0: Online)		*/
	/* ------------------------------------------------- */
#ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
	if (pData->flag)	// offline
	{
		miuDv->data.Flags = 0;
	}
	else
	{
		miuDv->data.Flags = 1;
	}
#else
	if (pData->flag)	// offline
	{
		MI_QSET_NORMAL_VAL  (miuDv->data.Flags, MI_QFLAG_NORMAL_VAL_ABNORMAL);
	}
	else
	{
		MI_QSET_NORMAL_VAL  (miuDv->data.Flags, MI_QFLAG_NORMAL_VAL_NORMAL);
	}
#endif		
	{
		time_t lastTime = time (NULL);
		iccpShmSetLastDataSendTime (lastTime);
	}
	/* ------------------------------------------------ */
	/*  2017-11-08, LHS...                              */
	/*  - ICCP Report 정보 확인...                      */
	/* ------------------------------------------------ */
	MIS_DV *dv = MIS_DV_REF_TO_DV (miuDv->dvRef);
	
#if 1	
	retVal = check_iccp_report(dv, pData);

	/* TYPE 별 인덱스 16까지 표시.... */
	if(retVal == 2)	return;
#endif

    /* ------------------------------------------------------------ */
	/*  2017-11-14, LHS...                                          */
	/*  - ICCP Debug시... printf() 문제로 전체 출력은않됨... ㅠㅠ   */
	/* ------------------------------------------------------------ */      
	if (opr->iccpDebug)	// QPRINTF
	{
		MIS_DV *dv = MIS_DV_REF_TO_DV (miuDv->dvRef);
		char buf[100];
        
		if (miuDv->mi_type == MI_TYPEID_REAL_Q_TIMETAG_EXTENDED)
		{
			Debug(console, "    DV:%s\tvalue: %-10.3f %s  %s\n", 
				dv->dv_name, pData->value,
				sprt_timeval (buf, sizeof(buf), &pData->updateTime),
				(pData->flag == 0) ? "online " : "offline");
		}
		else
		{
			Debug(console, "    DV:%s\tvalue: %d          %s  %s%s\n", 
				dv->dv_name, (int)pData->value,
				sprt_timeval (buf, sizeof(buf), &pData->updateTime),
				(pData->flag == 0) ? "online " : "offline",
				bSoe ? "--> SOE" : "");
		}
	}
}
#endif

/************************************************************************/
/*			u_mis_rd_dv 					*/
/************************************************************************/

/* This function is called when MI needs to get Data Value data, either	*/
/* for responding to a read indication or for sending an information	*/
/* report (Data Set Transfer Set).					*/

// user  iccp server read  data value 
ST_VOID u_mis_rd_dv (MIS_RD_DV_CTRL *rd_dv_ctrl)
{
	MISU_DV *miuDv;
	MIS_DV *dv;
	ST_RET rc;
	ICCP_POINT_DATA	data;
	ICCP_POINT_INFO info;

	/* For our sample, the MIS_DV (Data Value control) has the 		*/
	/* 'access_info' element set to point to the data structure containing	*/
	/* all information about the point. We will use this to fill in the 	*/
	/* destination data, depending on the types involved.			*/

	rc = SD_SUCCESS;
	dv = MIS_DV_REF_TO_DV (rd_dv_ctrl->mis_dv_ref);
	miuDv = dv->access_info;

	/* Make sure this one that we have mapped, and if not just let it go ..	*/
	if (miuDv == NULL)
	{
		mis_rd_dv_done (rd_dv_ctrl, SD_FAILURE);	
		return;
	}

#if 1	// 2016.05.12 ChoiBC Modify : DataValue Read 요구에 대한 응답
#if 1
	if (iccpShmGetPointInfo (miuDv->icfgRef.ref, &info) == 0) 
	                            //ref는 ICCP_POINT_INFO 배열의 번호인데..그렇게 위에서 내려오나? SAI DSA 이런 이르밍아니고?
	{
		mis_rd_dv_done (rd_dv_ctrl, SD_FAILURE);	
		return;
	}
	if (getDeviceData (&info, &data) == 0)
	{
		mis_rd_dv_done (rd_dv_ctrl, SD_FAILURE);	
		return;
	}
#else
	// shared memory에서 포인트 데이터를 가져온다.
	if (iccpShmGetPointData (miuDv->icfgRef.ref, &data) == 0)
	{
		mis_rd_dv_done (rd_dv_ctrl, SD_FAILURE);	
		return;
	}
#endif
	// 포인트 데이터를 miuDv 데이터로 설정한다.
	_SetPointData2Miudv (miuDv, &data, SD_FALSE);
#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
	{
		INFO_ICCP_ST *pI = getIccpInfo ();
		pI->dTimeLastDataReceived = sGetMsTime ();
	}
#endif
#if 0	// CHOIBC DELETE
	switch (miuDv->mi_type)
	{
	case MI_TYPEID_REAL:
	case MI_TYPEID_REAL_Q:
	case MI_TYPEID_REAL_Q_TIMETAG:
	case MI_TYPEID_REAL_EXTENDED:
	case MI_TYPEID_REAL_Q_TIMETAG_EXTENDED:
		miuDv->data.Value.r = data.value;
		break;

	case MI_TYPEID_DISCRETE:
	case MI_TYPEID_DISCRETE_Q:
	case MI_TYPEID_DISCRETE_Q_TIMETAG:
	case MI_TYPEID_DISCRETE_EXTENDED:
	case MI_TYPEID_DISCRETE_Q_TIMETAG_EXTENDED:
		miuDv->data.Value.d = (ST_INT32) data.value;
		break;

	case MI_TYPEID_STATE:
	case MI_TYPEID_STATE_Q:
	case MI_TYPEID_STATE_Q_TIMETAG:
	case MI_TYPEID_STATE_EXTENDED:
	case MI_TYPEID_STATE_Q_TIMETAG_EXTENDED:
		if (data.value == 0)
		{
			MI_SET_DATA_STATE (miuDv->data.Flags, MI_DATA_STATE_OFF);
		}
		else
		{
			MI_SET_DATA_STATE (miuDv->data.Flags, MI_DATA_STATE_ON);
		}
		break;

	case MI_TYPEID_STATE_SUPP:
	case MI_TYPEID_STATE_SUPP_Q:
	case MI_TYPEID_STATE_SUPP_Q_TIMETAG:
	case MI_TYPEID_STATE_SUPP_EXTENDED:
	case MI_TYPEID_STATE_SUPP_Q_TIMETAG_EXTENDED:

		if (data.value == 0)
		{
			MI_SET_DATA_STATE (miuDv->data.Flags, MI_DATA_STATE_OFF);
			MI_SET_DATA_STATE (miuDv->data.Value.ss, MI_DATA_STATE_OFF);
		}
		else
		{
			MI_SET_DATA_STATE (miuDv->data.Flags, MI_DATA_STATE_ON);
			MI_SET_DATA_STATE (miuDv->data.Value.ss, MI_DATA_STATE_ON);
		}
		break;
	}
	miuDv->data.TimeStamp.GMTBasedS    = (ST_INT32) data.updateTime.tv_sec;
	miuDv->data.TimeStamp.Milliseconds = data.updateTime.tv_usec/1000;
	if (data.flag)	// offline
	{
		MI_QSET_NORMAL_VAL  (miuDv->data.Flags, MI_QFLAG_NORMAL_VAL_ABNORMAL);
	}
	else
	{
		MI_QSET_NORMAL_VAL  (miuDv->data.Flags, MI_QFLAG_NORMAL_VAL_NORMAL);
	}
#endif
#endif

	rc = misu_get_dv_data (miuDv, rd_dv_ctrl->data_buf);

	/* When the data has been put in the destination, we let MI know. In	*/
	/* our sample this happens immediately, but could be done some time 	*/
	/* later as apprioriate.						*/
	mis_rd_dv_done (rd_dv_ctrl, rc);

#if 0	// CHOIBC DELETE
	/* IF we are supposed to provide constantly changing data, do it	*/
	if (miuAlwaysChangeDv)
		misuChangeDv (miuDv, SD_TRUE);
#endif
}

/************************************************************************/
/*			u_mis_wr_dv 					*/
/************************************************************************/

ST_VOID u_mis_wr_dv (MIS_WR_DV_CTRL *wr_dv_ctrl)
{
	MISU_DV *miuDv;
	MIS_DV *dv;

	MI_REAL 		*rSrc; 		/* Source data pointers		*/
	MI_STATE 	   	*sSrc;
	MI_DISCRETE 	   	*dSrc;
	MI_STATE_SUPP		*ssSrc;
	MI_REAL_Q 	   	*rqSrc;
	MI_STATE_Q 	   	*sqSrc;
	MI_DISCRETE_Q 	   	*dqSrc;
	MI_STATE_SUPP_Q		*ssqSrc;
	MI_REAL_Q_TIMETAG 	*rqtSrc;
	MI_STATE_Q_TIMETAG    	*sqtSrc;
	MI_DISCRETE_Q_TIMETAG 	*dqtSrc;
	MI_STATE_SUPP_Q_TIMETAG *ssqtSrc;
	MI_REAL_EXTENDED 	*reSrc;
	MI_STATE_EXTENDED    	*seSrc;
	MI_DISCRETE_EXTENDED 	*deSrc;
	MI_STATE_SUPP_EXTENDED  *sseSrc;
	MI_REAL_Q_TIMETAG_EXTENDED 	 *rqteSrc;
	MI_STATE_Q_TIMETAG_EXTENDED    	 *sqteSrc;
	MI_DISCRETE_Q_TIMETAG_EXTENDED 	 *dqteSrc;
	MI_STATE_SUPP_Q_TIMETAG_EXTENDED *ssqteSrc;
	ST_RET rc;

	rc = SD_SUCCESS;
	dv = MIS_DV_REF_TO_DV (wr_dv_ctrl->mis_dv_ref);
	miuDv = dv->access_info;

	/* Make sure this one that we have mapped, and if not just let it go ..	*/
	if (miuDv == NULL)
	{
		mis_wr_dv_done (wr_dv_ctrl, SD_SUCCESS);	
		return;
	}

	switch (wr_dv_ctrl->mi_type)
	{
	case MI_TYPEID_REAL:
		rSrc = (MI_REAL *) wr_dv_ctrl->data_buf;
		miuDv->data.Value.r = *rSrc;
		break;

	case MI_TYPEID_STATE:
		sSrc = (MI_STATE *) wr_dv_ctrl->data_buf;
		miuDv->data.Flags = *sSrc;
		break;

	case MI_TYPEID_DISCRETE:
		dSrc = (MI_DISCRETE *) wr_dv_ctrl->data_buf;
		miuDv->data.Value.d = *dSrc;
		break;

	case MI_TYPEID_STATE_SUPP:
		ssSrc = (MI_STATE_SUPP *) wr_dv_ctrl->data_buf;
		miuDv->data.Value.ss = *ssSrc;
		break;

	case MI_TYPEID_REAL_Q:
		rqSrc = (MI_REAL_Q *) wr_dv_ctrl->data_buf;
		miuDv->data.Value.r = rqSrc->Value;
		miuDv->data.Flags = rqSrc->Flags;
		break;

	case MI_TYPEID_STATE_Q:
		sqSrc = (MI_STATE_Q *)  wr_dv_ctrl->data_buf;
		miuDv->data.Flags = *sqSrc;
		break;

	case MI_TYPEID_DISCRETE_Q:
		dqSrc = (MI_DISCRETE_Q *) wr_dv_ctrl->data_buf;
		miuDv->data.Value.d = dqSrc->Value;
		miuDv->data.Flags = dqSrc->Flags;
		break;

	case MI_TYPEID_STATE_SUPP_Q:
		ssqSrc = (MI_STATE_SUPP_Q *)  wr_dv_ctrl->data_buf;
		miuDv->data.Flags = ssqSrc->Flags;
		miuDv->data.Value.ss = ssqSrc->Value;
		break;

	case MI_TYPEID_REAL_Q_TIMETAG:
		rqtSrc = (MI_REAL_Q_TIMETAG *) wr_dv_ctrl->data_buf;
		miuDv->data.Value.r = rqtSrc->Value;
		miuDv->data.TimeStamp.GMTBasedS = rqtSrc->TimeStamp;
		miuDv->data.Flags = rqtSrc->Flags;
		break;

	case MI_TYPEID_STATE_Q_TIMETAG:
		sqtSrc = (MI_STATE_Q_TIMETAG *) wr_dv_ctrl->data_buf;
		miuDv->data.TimeStamp.GMTBasedS = sqtSrc->TimeStamp;
		miuDv->data.Flags = sqtSrc->Flags;
		break;

	case MI_TYPEID_DISCRETE_Q_TIMETAG:
		dqtSrc = (MI_DISCRETE_Q_TIMETAG *) wr_dv_ctrl->data_buf;
		miuDv->data.Value.d = dqtSrc->Value;
		miuDv->data.TimeStamp.GMTBasedS = dqtSrc->TimeStamp;
		miuDv->data.Flags = dqtSrc->Flags;
		break;

	case MI_TYPEID_STATE_SUPP_Q_TIMETAG:
		ssqtSrc = (MI_STATE_SUPP_Q_TIMETAG *) wr_dv_ctrl->data_buf;
		miuDv->data.TimeStamp.GMTBasedS = ssqtSrc->TimeStamp;
		miuDv->data.Flags = ssqtSrc->Flags;
		miuDv->data.Value.ss = ssqtSrc->Value;
		break;

	case MI_TYPEID_REAL_EXTENDED:
		reSrc = (MI_REAL_EXTENDED *) wr_dv_ctrl->data_buf;
		miuDv->data.Value.r = reSrc->Value;
		miuDv->data.TimeStamp.GMTBasedS = reSrc->TimeStamp;
		miuDv->data.Flags = reSrc->Flags;
		miuDv->data.COV = reSrc->COV;
		break;

	case MI_TYPEID_STATE_EXTENDED:
		seSrc = (MI_STATE_EXTENDED *) wr_dv_ctrl->data_buf;
		miuDv->data.TimeStamp.GMTBasedS = seSrc->TimeStamp;
		miuDv->data.Flags = seSrc->Flags;
		miuDv->data.COV = seSrc->COV;
		break;

	case MI_TYPEID_DISCRETE_EXTENDED:
		deSrc = (MI_DISCRETE_EXTENDED *)wr_dv_ctrl->data_buf;
		miuDv->data.Value.d = deSrc->Value;
		miuDv->data.TimeStamp.GMTBasedS = deSrc->TimeStamp;
		miuDv->data.Flags = deSrc->Flags;
		miuDv->data.COV = deSrc->COV;
		break;

	case MI_TYPEID_STATE_SUPP_EXTENDED:
		sseSrc = (MI_STATE_SUPP_EXTENDED *) wr_dv_ctrl->data_buf;
		miuDv->data.TimeStamp.GMTBasedS = sseSrc->TimeStamp;
		miuDv->data.Flags = sseSrc->Flags;
		miuDv->data.COV = sseSrc->COV;
		miuDv->data.Value.ss = sseSrc->Value;
		break;

	case MI_TYPEID_REAL_Q_TIMETAG_EXTENDED:
		rqteSrc = (MI_REAL_Q_TIMETAG_EXTENDED *) wr_dv_ctrl->data_buf;
		miuDv->data.Value.r = rqteSrc->Value;
		miuDv->data.TimeStamp = rqteSrc->TimeStamp;
		miuDv->data.Flags = rqteSrc->Flags;
		break;

	case MI_TYPEID_STATE_Q_TIMETAG_EXTENDED:
		sqteSrc = (MI_STATE_Q_TIMETAG_EXTENDED *) wr_dv_ctrl->data_buf;
		miuDv->data.TimeStamp = sqteSrc->TimeStamp;
		miuDv->data.Flags = sqteSrc->Flags;
		break;

	case MI_TYPEID_DISCRETE_Q_TIMETAG_EXTENDED:
		dqteSrc = (MI_DISCRETE_Q_TIMETAG_EXTENDED *) wr_dv_ctrl->data_buf;
		miuDv->data.Value.d = dqteSrc->Value;
		miuDv->data.TimeStamp = dqteSrc->TimeStamp;
		miuDv->data.Flags = dqteSrc->Flags;
		break;

	case MI_TYPEID_STATE_SUPP_Q_TIMETAG_EXTENDED:
		ssqteSrc = (MI_STATE_SUPP_Q_TIMETAG_EXTENDED *) wr_dv_ctrl->data_buf;
		miuDv->data.TimeStamp = ssqteSrc->TimeStamp;
		miuDv->data.Flags = ssqteSrc->Flags;
		miuDv->data.Value.ss = ssqteSrc->Value;
		break;

	default:
		printf ("\nError - Check Me Out!");
		rc = SD_FAILURE;
		break;
	}
	mis_wr_dv_done (wr_dv_ctrl, rc);
}

/************************************************************************/
/************************************************************************/
/* MIS COMMUNICATIONS ACTIVITY INDICATION				*/
/* These functions are called by MI to tell the application about 	*/
/* server communications activity. Typically, no action is required ...	*/
/************************************************************************/
/************************************************************************/
/* DATA SET ACTIVITY							*/
/************************************************************************/
/************************************************************************/
/*			u_mis_create_ds_ind 				*/
/************************************************************************/

/* This function is called by MI when a client is creating a data set.	*/
/* No activity is required on our part ...				*/

ST_RET u_mis_create_ds_ind (MIS_VCC *vcc, DEFVLIST_REQ_INFO *req_info)
{
	return (SD_SUCCESS);
}

/************************************************************************/
/*			u_mis_delete_ds_ind 				*/
/************************************************************************/

/* This function is called by MI when a client is deleting a data set.	*/
/* No activity is required on our part ...				*/

ST_RET u_mis_delete_ds_ind (MIS_VCC *vcc, DELVLIST_REQ_INFO *req_info)
{
	return (SD_SUCCESS);
}

/************************************************************************/
/*			_misPrintDstsData				*/
/************************************************************************/

static ST_VOID _misPrintDstsData (const ST_CHAR *title, MI_DSTS_DATA *d)
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
/*			_misSendInitialIntegrity			*/
/* DSTS 시작 시 데이터셋의 모든 DV를 "changed"로 마킹하여		*/
/* 첫 번째 DSTS 리포트에 전체 현재값이 포함되도록 한다.		*/
/* (calling device가 RBE=T + Change 조건만 설정했을 때의 초기 전송)	*/
/************************************************************************/

static ST_VOID _misSendInitialIntegrity (MIS_DSTS *misDsts)
	{
	MIS_DS       *mis_ds;
	MIS_DSTS_VAR *var;
	MIS_DV       *dv;
	MISU_DV      *miuDv;
	MI_DV_DATA_UNION dvData;
	ST_INT        i;
	ST_INT        count = 0;

	mis_ds = misDsts->mis_ds;
	if (mis_ds == NULL)
		{
		printf ("\n[INTEGRITY] mis_ds is NULL, skipping\n");
		return;
		}

	printf ("\n[INTEGRITY] Initial integrity: marking all %d vars in dataset '%s'\n",
		mis_ds->num_var, mis_ds->ds_name ? mis_ds->ds_name : "?");

	for (i = 0; i < mis_ds->num_var; i++)
		{
		var = &mis_ds->mis_var_tbl[i];
		if (!var->dv)
			continue;		/* Device 항목은 건너뜀 */

		dv = MIS_DV_REF_TO_DV (var->v.dv_ref);
		if (dv == NULL)
			continue;

		miuDv = (MISU_DV *) dv->access_info;
		if (miuDv == NULL)
			continue;

		if (misu_get_dv_data (miuDv, &dvData) == SD_SUCCESS)
			{
			mis_dv_change_ex (var->v.dv_ref, miuDv->mi_type, &dvData);
			printf ("\n  -> [%d] marked: %s", i, dv->dv_name ? dv->dv_name : "?");
			count++;
			}
		}

	printf ("\n[INTEGRITY] Done: %d DVs marked for initial report\n", count);
	}

/************************************************************************/
/*			u_mis_dsts_activity 				*/
/************************************************************************/

/* This function is called by MI when there is activity associated with	*/
/* a Data Set Transfer Set. No activity is required on our part ...	*/

ST_RET u_mis_dsts_activity (MIS_VCC *vcc, MIS_DSTS *misDsts, 
	ST_INT activity_type, 
	MI_TS_TIMESTAMP ts_timestamp)
{
	ST_CHAR *tsName;

	tsName = misDsts->ts_name;
	switch (activity_type)
	{
	case MIS_DSTS_START_TRANSFER :
#if 1	// 2016.11 ChoiBC Modify : Start Transfer ==> commMaster
		{
			//time_t lastTime = time (NULL);
			iccpShmSetCommMode (1, 0);
		}
#endif
		printf(">> Start Transfer for DSTS '%s'\n", tsName);
		QPRINTF (">> Start Transfer for DSTS '%s'\n", tsName);
		_misPrintDstsData ("[CDSTS START]", &misDsts->mi_dsts_data);
		// 20270707 for KM-ELC
		_misSendInitialIntegrity (misDsts);
		break;

	case MIS_DSTS_STOP_TRANSFER :
#if 1	// 2020.09 ChoiBC Modify : NO_ICCP_DUAL_LINK (ICCP 통신링크 이중화 제거)
#else
#if 1	// 2016.11 ChoiBC Modify : Stop Transfer ==> commSlave
		{
			//time_t lastTime = time (NULL);
			iccpShmSetCommMode (0, 0);
		}
#endif
#endif	// 2020.09 ChoiBC Modify : NO_ICCP_DUAL_LINK (ICCP 통신링크 이중화 제거)
		QPRINTF (">> Stop Transfer for DSTS '%s'\n", tsName);
		break;

	case MIS_DSTS_BEGIN_REPORT :
		QPRINTF ("<< Begin Report for DSTS '%s'\n", tsName);
		break;

	case MIS_DSTS_END_REPORT :
		QPRINTF ("<< End Report for DSTS '%s'\n", tsName);
		break;

	case MIS_DSTS_ACK_RCVD : 
		QPRINTF (">> ACK received for DSTS '%s'\n", tsName);
		break;

	case MIS_DSTS_NACK_RCVD :
		QPRINTF (">> NACK received for DSTS '%s'\n", tsName);
		break;

	default:
		break;
	}
	return (SD_SUCCESS);
}


/************************************************************************/
/************************************************************************/
/* INFORMATION MESSAGE OPERATIONS					*/
/************************************************************************/
/*			u_mis_start_IM_Transfer				*/
/* notification that the transfer set has been enabled by the client	*/
/************************************************************************/

ST_VOID u_mis_start_IM_Transfer (MI_CONN *mi_conn)
{
	printf ("\n IM_Transfer_Set enabled for remote AR %s", mi_conn->remote_ar);
}

/************************************************************************/
/*			u_mis_stop_IM_Transfer				*/
/* notification that the transfer set has been disabled by the client	*/
/************************************************************************/

ST_VOID u_mis_stop_IM_Transfer (MI_CONN *mi_conn)
{
	printf ("\n IM_Transfer_Set disabled for remote AR %s", mi_conn->remote_ar);
}


/************************************************************************/
/************************************************************************/
/* DEVICE OPERATIONS							*/
/************************************************************************/
/*			u_mis_device_select				*/
/* notification of device select request				*/
/************************************************************************/

ST_RET u_mis_device_select (MIS_DEVICE_IND_CTRL *dev_ind_ctrl)
{
	char * devName;
#if 1	// 2016.05.25 ChoiBC Modify : Device Select notification
	ICCP_CONTROL_DATA control;

#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
	{
		INFO_ICCP_ST *pI = getIccpInfo ();
		pI->dTimeLastCommandReceived = sGetMsTime ();
	}
#endif
	devName = (char *) dev_ind_ctrl->mis_device->device_name;

	iccpShmGetControlData (&control);
	if (control.cntrFlag)
	{
		printf ("Select Error : Control In Progress, device(%s)\n", devName);
		return(MIS_SEND_UNAVAIL_RESP);
	}
#endif
	QPRINTF (">> Device Select for %s, send default response.\n", devName);
	return (MIS_SEND_DEFAULT_RESP);
}

/************************************************************************/
/*			u_mis_device_operate				*/
/* notification of device operate request				*/
/************************************************************************/

ST_RET u_mis_device_operate (MIS_DEVICE_IND_CTRL *dev_ind_ctrl)
{   
	char * devName;
	MI_CONTROL_COMMAND  *pCommand;
	//MI_CONTROL_SETPOINT_REAL *pReal;
	//MI_CONTROL_SETPOINT_DISCRETE *pDiscrete;
	MIS_DEVICE *dev;
	MIS_DEVICE *mis_device;
#if 1	// 2016.05.25 ChoiBC Modify : Device Operate notification
	ICCP_POINT_INFO info;
	ICCP_CONTROL_DATA control;
	MISU_DEV *miuDv;
#endif

#if 1	// 2017.11 ChoiBC Modify : 일정 시간동안 링크로 Identify가 오지 않으면, 접속을 끊는다.
	{
		INFO_ICCP_ST *pI = getIccpInfo ();
		pI->dTimeLastCommandReceived = sGetMsTime ();
	}
#endif

	devName = (char *) dev_ind_ctrl->mis_device->device_name;
	QPRINTF (">> Device Operate for %s.\n", devName);

	/* The value returned from this function depends on if the control point is selectable.	*/
	/* When a Device is modelled SBO, the application must review that remote site operating	*/
	/* the device is the same remote site that selected the device. If not it means one site	*/
	/* is selecting, and another is operating, not a good situation.				*/
	/* A second problem for SBO devices is that they must be selected or ARMED			*/
	/* prior to being operated. The ARMED state is associated with a timer, the timer prevents	*/
	/* a client from selecting the device then having mutually exclusinve access to Device	*/
	/* A client has to operate the device according to a configured time limit or it looses ARMED	*/
	/* access to the device									*/

	dev = MIS_DEV_REF_TO_DEV (dev_ind_ctrl->mis_device_ref);
	mis_device = dev_ind_ctrl->mis_device;

	if (mis_device->sbo == SD_TRUE)
	{


		/* The first problem is latency between selecting and operating	  */
		/* timers associated with individual devices are updated when	  */
		/* the application calls mi_service. The state of the device may	  */
		/* transition from ARMED to IDLE during some call to mi_service	  */
		/* the application may set the device_state to a value other than	  */
		/* ARMED for example BUSY or SELECTING. Its either ARMED or its not!  */
		/* The second check is to see who performed the select operation	  */
		if ((mis_device->device_state != MIS_DEVICE_STATE_ARMED) ||
			(mis_device->selecter != dev_ind_ctrl->mis_vcc->mi_remote))
		{
			return(MIS_SEND_UNAVAIL_RESP);
		}

		/* Another application specific check is required here to determine if the	   */
		/* device is Busy or in process of Selecting. No sample code exists as this is */
		/* application specific. If either case were true, this function should return */
		/* value MIS_SEND_UNAVAIL_RESP						   */
	}

#if 0	// 2016.05.25 ChoiBC Modify : Device Operate notification - tag is not supported
	/* The value returned from this functon depends on if the control point is taggable */
	/* The application must determine if the device is taggable. If it is not taggable  */
	/* the application can return value MIS_SEND_DEFAULT_RESP			      */
	if (dev->taggable == SD_TRUE)
	{
		/* Only examine the tag value if the Device is taggable. */
		MI_TAG_FLAGS tagFlags;

		tagFlags = dev->tag_value.TagFlags;
		QPRINTF ("\n TagFlags = 0x%x", tagFlags);

		switch (tagFlags)
		{

			/* The spec says: A value of NO-TAG never prevents operation. */
		case MI_TAG_FLAG_NO_TAG:
			return (MIS_SEND_DEFAULT_RESP);    
			break;

			/* If it is tagged the application must examine the value of the tag and take one of		  */
			/* two actions.											  */

			/* action 1) If the Device is tagged OPEN-CLOSE inhibit, better not allow any operate to take place */
			/* the function has to return MIS_SEND_UNAVAIL_RESP . The spec says: A value of OPEN-AND-CLOSE	  */
			/* INHIBIT always prevents operation.								  */

		case MI_TAG_FLAG_OPEN_CLOSE_INHIBIT:
			return (MIS_SEND_UNAVAIL_RESP);
			break;

			/* action 2) If the Device is tagged CLOSE inhibit, the application must examine the value being	  */
			/* written to the real device and decide if it means open or close. Opearting with an Open value is OK,,,  */
			/* The application is NOT supposed to close or raise the control point due to its tag. The MI library */
			/* exports this decision to the application. If the application decides the value			  */
			/* used in the operate indication means "CLOSE" and the tag clearly means do not close, then	  */
			/* the application should return value MIS_SEND_UNAVAIL_RESP. The spec says:			  */
			/* A value of CLOSE-ONLY-INHIBIT only prevents Close or Raise commands.				  */
		case MI_TAG_FLAG_CLOSE_INHIBIT:
			switch (dev_ind_ctrl->device_type)
			{
			case MI_TYPEID_DEV_COMMAND:
				pCommand = (MI_CONTROL_COMMAND *) dev_ind_ctrl->data_buf;
				if (*pCommand <= 0)  /* open */
					return (MIS_SEND_DEFAULT_RESP);
				else /* close */
					return (MIS_SEND_UNAVAIL_RESP);
				break;

			case MI_TYPEID_DEV_DISCRETE:
				pDiscrete = (MI_CONTROL_SETPOINT_DISCRETE *) dev_ind_ctrl->data_buf;
				if (*pDiscrete <= 0)  /* open */
					return (MIS_SEND_DEFAULT_RESP);
				else /* close */
					return (MIS_SEND_UNAVAIL_RESP);
				break;

			case MI_TYPEID_DEV_REAL:
				pReal = (MI_CONTROL_SETPOINT_REAL *) dev_ind_ctrl->data_buf;
				if (*pReal <= 0.0)  /* open */
					return (MIS_SEND_DEFAULT_RESP);
				else /* close */
					return (MIS_SEND_UNAVAIL_RESP);

				break;

			default:
				printf ("\n Unknown OperateValue Type.");
				break;
			} 
			break;

		default :
			QPRINTF ("\n Unexpected Tag value assigned to this device! = 0x%x", tagFlags);
			break;
		} /* end of CLOSE inhibit */
	} /* end if taggable */
#endif

#if 1	// 2016.05.25 ChoiBC Modify : Device Operate notification
	iccpShmGetControlData (&control);
	if (control.cntrFlag)
	{
		printf ("Operate Error : Control In Progress, device(%s)\n", devName);
		return(MIS_SEND_UNAVAIL_RESP);
	}
#if 1	// 2016.11 ChoiBC MPU 절체 명령 추가
    #ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
	if (strcmp (devName, getMpuSwitchDevName ()) == 0)
#else
	if (strcmp (devName, MPU_SWITCH_DEV_NAME) == 0)
#endif
	{
		memset (&control, 0, sizeof(control));
		control.cntrType = 1;
		QPRINTF ("    MPU Switch Command\n");
		iccpShmSetControlData (&control);
		return (MIS_SEND_DEFAULT_RESP);
	}
#endif
	miuDv = dev->access_info;
	/* Make sure this one that we have mapped, and if not just let it go ..	*/
	if (miuDv == NULL)
	{
		printf ("Operate Error : miuDv is Null, device(%s)\n", devName);
		return(MIS_SEND_UNAVAIL_RESP);
	}
	if (iccpShmGetPointInfo (miuDv->icfgRef.ref, &info) == 0)
	{
		printf ("Operate Error : illegal ref(%d), device(%s)\n", miuDv->icfgRef.ref, devName);
		return(MIS_SEND_UNAVAIL_RESP);
	}
	if (strcmp (devName, info.iccpName))
	{
		printf ("Operate Error : illegal Name, ref(%d), name(%s), device(%s)\n",
			miuDv->icfgRef.ref, info.iccpName, devName);
		return(MIS_SEND_UNAVAIL_RESP);
	}
	if (dev_ind_ctrl->device_type != MI_TYPEID_DEV_COMMAND)
	{
		printf ("Operate Error : illegal device type(%d), device(%s)\n",
			dev_ind_ctrl->device_type, devName);
		return(MIS_SEND_UNAVAIL_RESP);
	}
	control.cntrType  = 0;	// Point 제어
	control.cntrDev   = info.devNo;
	control.cntrPoint = info.devPt;
	control.cntrTime  = 0;
	pCommand = (MI_CONTROL_COMMAND *) dev_ind_ctrl->data_buf;
	QPRINTF ("     OperateValue = %d\n", *pCommand);
	control.cntrState = (*pCommand == 0) ? 1 : 2;	// 1 : TRIP, 2: CLOSE

	iccpShmSetControlData (&control);
#endif

	return (MIS_SEND_DEFAULT_RESP);
}

/************************************************************************/
/*		      u_mis_device_settag				*/
/* notification of a device set tag request				*/
/************************************************************************/

ST_RET u_mis_device_settag (MIS_DEVICE_IND_CTRL *dev_ind_ctrl)
{ 
	char * devName;
	MI_TAG_VALUE *pTagValue;

#if 1	// 2016.01.22 ChoiBC Modify : Device Set Tag
#endif
	devName = (char *) dev_ind_ctrl->mis_device->device_name;
	pTagValue = (MI_TAG_VALUE *) dev_ind_ctrl->data_buf;

	QPRINTF (">> Device Set Tag for %s, send default response.\n", devName);
	QPRINTF ("     TagFlags = 0x%x, Reason = %s\n", pTagValue->TagFlags, pTagValue->Reason);

	return (MIS_SEND_DEFAULT_RESP);
}

/************************************************************************/
/*		      u_mis_device_gettag				*/
/* notification of a device get tag request				*/
/************************************************************************/

ST_RET u_mis_device_gettag (MIS_DEVICE_IND_CTRL *dev_ind_ctrl)
{
	//MISU_DEV *miuDev;
	MIS_DEVICE *dev;

#if 1	// 2016.01.22 ChoiBC Modify : Device Get Tag
#endif
	dev = MIS_DEV_REF_TO_DEV (dev_ind_ctrl->mis_device_ref);
	//miuDev = dev->access_info;
	QPRINTF (">> Device Get Tag for %s, send default response.\n", dev->device_name);

#if 0	// CHOIBC DELETE
	/* If we are supposed to provide constantly changing data, do it	*/
	if (miuAlwaysChangeDev)
		misuChangeDev (miuDev, SD_TRUE);
#endif

	return (MIS_SEND_DEFAULT_RESP);
}

/************************************************************************/
/************************************************************************/
/* Server Activity Simulation 						*/
/************************************************************************/

#if 0	// CHOIBC DELETE
#define MISU_AUTO_DV_UPDATE_TIME	500.0
#define MISU_AUTO_IM_SEND_TIME	 	10000.0

/************************************************************************/
/*			miuAutoService 					*/
/************************************************************************/

ST_VOID miuAutoService ()
{
	static ST_DOUBLE lastMisuDvUpdateTime;
	static ST_DOUBLE lastMisuImSendTime;
	ST_DOUBLE currTime = 0;

	currTime = sGetMsTime ();
	if (miuFullSpeed || (currTime - lastMisuDvUpdateTime > MISU_AUTO_DV_UPDATE_TIME))
	{
		misuChangeNextDv ();
		misuChangeNextDev ();
		lastMisuDvUpdateTime = currTime;
	}
	if (currTime - lastMisuImSendTime > MISU_AUTO_IM_SEND_TIME)
	{
		misuSendMessage ();
		lastMisuImSendTime = currTime;
	}
}			      

/************************************************************************/
/*			misuChangeNextDv				*/
/************************************************************************/

ST_VOID misuChangeNextDv ()
{
	static MISU_DV *miuDv = NULL;
	ST_INT numToChange;
	ST_INT i;
	ST_INT dvChangeRslt;

	if (misu_dv_list == NULL)
		return;

	if (miuAcrTest)
	{
		numToChange = 10000;
		for (i = 0; i < numToChange; ++i)
		{
			if (miuDv == NULL)
				miuDv = misu_dv_list;

			dvChangeRslt = misuChangeDv (miuDv, SD_FALSE);
			if (dvChangeRslt == MIS_DV_CHANGE_LOST)
				break;

			if (dvChangeRslt == MIS_DV_CHANGE_FULL)
				break;
		}
		miuDv = list_get_next (misu_dv_list, miuDv);
	}
	else
	{
		numToChange = 1;
		for (i = 0; i < numToChange; ++i)
		{
			if (miuDv == NULL)
				miuDv = misu_dv_list;

			dvChangeRslt = misuChangeDv (miuDv, SD_FALSE);
			if (dvChangeRslt != MIS_DV_CHANGE_OK)
			{
				printf ("\nLocal DV Change error: %s", 
					dvChangeRslt == MIS_DV_CHANGE_LOST ? "Lost" : "Full");
			}
			miuDv = list_get_next (misu_dv_list, miuDv);
		}
	}
}
#endif	// CHOIBC DELETE


#if 1	// 2016.05.13 ChoiBC Modify : 값이 바뀐경우 호출
/************************************************************************/
/*			misuChangeDataValue					*/
/************************************************************************/
// return MY_DV_CHANGE_OK, MY_DV_CHANGE_LOST, MY_DV_CHANGE_FULL
ST_INT misuChangeDataValue (ICCP_POINT_INFO *pPnt, ICCP_POINT_DATA *pData, ST_BOOLEAN bSoe)
{
	MI_DV_DATA_UNION mis_dv_val;
	MIS_DV *dv;
	ST_INT dvChangeRslt;
	MISU_DV *miuDv;
	ST_INT found = 0;
	char buf[100];

	// find miuDv
	for (miuDv = misu_dv_list; miuDv != NULL; miuDv = list_get_next (misu_dv_list, miuDv))
	{
		if (miuDv->icfgRef.ref == pPnt->index)
		{
			found = 1;
			break;
		}
	}
	if (found == 0)
	{
		printf ("misuChangeDataValue : Not found at misu_dv_list, ref(%d), name(%s)\n",
			pPnt->index, pPnt->iccpName);
		return MY_DV_CHANGE_NOT_FOUND;
	}
	dv = MIS_DV_REF_TO_DV (miuDv->dvRef);
	if (dv->mis_dv_dsts_ref == NULL)	// DsTs 등록된 포인트만 전송한다. StartDsTs가 안되어 있어도 이변수는 NULL이다.
	{
		printf ("misuChangeDataValue : Not DsTs Dv, ref(%d), name(%s)\n",
			pPnt->index, pPnt->iccpName);
		return MY_DV_CHANGE_NOT_DSTS;
	}

	QPRINTF ("<< Changing DV '%s' offline(%d) value(%d) time(%s)\n",
		dv->dv_name, pData->flag, (int)pData->value,
		sprt_timeval (buf, sizeof(buf), &pData->updateTime));

	// 포인트 데이터를 miuDv 데이터로 설정한다.
	_SetPointData2Miudv (miuDv, pData, bSoe);

#ifdef MUI_REDUNDANCY_SUPPORT
	miuIrMisDvUpdate (miuDv->dvRef, miuDv->mi_type, &miuDv->data);
#endif

	misu_get_dv_data (miuDv, &mis_dv_val);
	dvChangeRslt = mis_dv_change_ex (miuDv->dvRef, miuDv->mi_type,  &mis_dv_val);
	// SOE가 두번 올라가는걸 막는다.
	if (bSoe && miuDv->mi_type == MI_TYPEID_STATE_SUPP_Q_TIMETAG_EXTENDED)
		miuDv->data.Value.ss &= ~MI_DATA_STATE_SUPP_SOE;

	if (dvChangeRslt == MIS_DV_CHANGE_LOST)
	{
		printf ("misuChangeDataValue : Event LOST, ref(%d), name(%s)\n",
			pPnt->index, pPnt->iccpName);
		return MY_DV_CHANGE_LOST;
	}
	if (dvChangeRslt == MIS_DV_CHANGE_FULL)
	{
		printf ("misuChangeDataValue : Event FULL, ref(%d), name(%s)\n",
			pPnt->index, pPnt->iccpName);
		return MY_DV_CHANGE_FULL;
	}
	return (dvChangeRslt);
}

#endif

#ifdef _VITZROSYS_FEP	// 2019.09 ChoiBC Modify : 비츠로시스 FEP 연동
/************************************************************************/
/*			misuCheckCOA					*/
/************************************************************************/
// return count of COA
ST_INT misuCheckCOA (ST_INT iccpPointType)
{
	MI_DV_DATA_UNION mis_dv_val;
	MIS_DV *dv;
	ST_INT dvChangeRslt;
	ST_INT count = 0;
	MISU_DV *miuDv;
	char buf[100];
	ICCP_POINT_DATA	data;
	ICCP_POINT_INFO info;

	// find miuDv
	for (miuDv = misu_dv_list; miuDv != NULL; miuDv = list_get_next (misu_dv_list, miuDv))
	{
		if (miuDv->mi_type != MI_TYPEID_REAL_Q_TIMETAG_EXTENDED)
			continue;
		dv = MIS_DV_REF_TO_DV (miuDv->dvRef);
		if (dv->mis_dv_dsts_ref == NULL)	// DsTs 등록된 포인트만 전송한다. StartDsTs가 안되어 있어도 이변수는 NULL이다.
			continue;
		if (iccpShmGetPointInfo (miuDv->icfgRef.ref, &info) == 0)
			continue;
		if (info.iccpPointType != iccpPointType)
			continue;
		if (getDeviceData (&info, &data) == 0)
			continue;
		if (data.flag != miuDv->data.Flags &&	// 비츠로시스 Flags와 flag는 반전
												// data.flag (0:online, 1:offline)
												// miuDv->data.Flags (1:online, 0:offline)
			data.value == miuDv->data.Value.r)
			continue;

		if(opr->iccpDebug)			
		QPRINTF ("<< Changing DV '%s' offline(%d) value(%f) time(%s)\n",
			dv->dv_name, data.flag, data.value,
			sprt_timeval (buf, sizeof(buf), &data.updateTime));

		// 포인트 데이터를 miuDv 데이터로 설정한다.
		_SetPointData2Miudv (miuDv, &data, SD_FALSE);

#ifdef MUI_REDUNDANCY_SUPPORT
		miuIrMisDvUpdate (miuDv->dvRef, miuDv->mi_type, &miuDv->data);
#endif
		misu_get_dv_data (miuDv, &mis_dv_val);
		dvChangeRslt = mis_dv_change_ex (miuDv->dvRef, miuDv->mi_type,  &mis_dv_val);
		if (dvChangeRslt == MIS_DV_CHANGE_LOST)
		{
			printf ("misuCheckCOA : COA LOST, name(%s)\n",
				dv->dv_name);
			continue;
		}
		if (dvChangeRslt == MIS_DV_CHANGE_FULL)
		{
			printf ("misuCheckCOA : COA FULL, name(%s)\n",
				dv->dv_name);
			continue;
		}
		count++;
	}
	return (count);
}
#endif



#if 0	// CHOIBC DELETE
/************************************************************************/
/*			misuChangeDv					*/
/************************************************************************/

static ST_INT misuChangeDv (MISU_DV *miuDv, ST_BOOLEAN changeValueOnly)
{
	MI_DV_DATA_UNION mis_dv_val;
	MIS_DV *dv;
	static ST_INT16 Milliseconds;
	ST_INT dvChangeRslt;

	dv = MIS_DV_REF_TO_DV (miuDv->dvRef);
	QPRINTF ("\nChanging DV '%s'", dv->dv_name);

	switch (miuDv->mi_type)
	{
	case MI_TYPEID_REAL:
	case MI_TYPEID_REAL_Q:
	case MI_TYPEID_REAL_Q_TIMETAG:
	case MI_TYPEID_REAL_EXTENDED:
	case MI_TYPEID_REAL_Q_TIMETAG_EXTENDED:
		miuDv->data.Value.r += (ST_FLOAT) 0.01;
		break;

	case MI_TYPEID_DISCRETE:
	case MI_TYPEID_DISCRETE_Q:
	case MI_TYPEID_DISCRETE_Q_TIMETAG:
	case MI_TYPEID_DISCRETE_EXTENDED:
	case MI_TYPEID_DISCRETE_Q_TIMETAG_EXTENDED:
		miuDv->data.Value.d += 1;
		break;

	case MI_TYPEID_STATE:
	case MI_TYPEID_STATE_Q:
	case MI_TYPEID_STATE_Q_TIMETAG:
	case MI_TYPEID_STATE_EXTENDED:
	case MI_TYPEID_STATE_Q_TIMETAG_EXTENDED:
		if ((miuDv->data.Flags & MI_DATA_STATE_MASK) == MI_DATA_STATE_OFF)
		{
			MI_SET_DATA_STATE (miuDv->data.Flags, MI_DATA_STATE_ON);
		}
		else
		{
			MI_SET_DATA_STATE (miuDv->data.Flags, MI_DATA_STATE_OFF);
		}
		break;

	case MI_TYPEID_STATE_SUPP:
	case MI_TYPEID_STATE_SUPP_Q:
	case MI_TYPEID_STATE_SUPP_Q_TIMETAG:
	case MI_TYPEID_STATE_SUPP_EXTENDED:
	case MI_TYPEID_STATE_SUPP_Q_TIMETAG_EXTENDED:

		if ((miuDv->data.Flags & MI_DATA_STATE_MASK) == MI_DATA_STATE_OFF)
		{
			MI_SET_DATA_STATE (miuDv->data.Flags, MI_DATA_STATE_ON);
			MI_SET_DATA_STATE (miuDv->data.Value.ss, MI_DATA_STATE_ON);
		}
		else
		{
			MI_SET_DATA_STATE (miuDv->data.Flags, MI_DATA_STATE_OFF);
			MI_SET_DATA_STATE (miuDv->data.Value.ss, MI_DATA_STATE_OFF);
		}
		break;
	}
	miuDv->data.TimeStamp.GMTBasedS    = (ST_INT32) time (NULL);
	miuDv->data.TimeStamp.Milliseconds = Milliseconds;
	++Milliseconds;
	if (Milliseconds > 1000)
		Milliseconds = 0;


	if (!changeValueOnly)
	{
#ifdef MUI_REDUNDANCY_SUPPORT
		miuIrMisDvUpdate (miuDv->dvRef, miuDv->mi_type, &miuDv->data);
#endif
		misu_get_dv_data (miuDv, &mis_dv_val);
		dvChangeRslt = mis_dv_change_ex (miuDv->dvRef, miuDv->mi_type,  &mis_dv_val);
	}
	else
		dvChangeRslt = -1;

	return (dvChangeRslt);
}


/************************************************************************/
/*			misuChangeNextDev				*/
/************************************************************************/

ST_VOID misuChangeNextDev ()
{
	static MISU_DEV *miuDev = NULL;
	static ST_INT numToChange;
	ST_INT i;
	ST_INT devChangeRslt;

	if (misu_dev_list == NULL)
		return;

	if (miuAcrTest)
	{
		numToChange = 10000;
		for (i = 0; i < numToChange; ++i)
		{
			if (miuDev == NULL)
				miuDev = misu_dev_list;

			devChangeRslt = misuChangeDev (miuDev, SD_FALSE);
			if (devChangeRslt == MIS_DV_CHANGE_LOST)
				break;

			if (devChangeRslt == MIS_DV_CHANGE_FULL)
				break;
		}
		miuDev = list_get_next (misu_dev_list, miuDev);
	}
	else
	{
		numToChange = 1;
		for (i = 0; i < numToChange; ++i)
		{
			if (miuDev == NULL)
				miuDev = misu_dev_list;

			devChangeRslt = misuChangeDev (miuDev, SD_FALSE);
			if (devChangeRslt != MIS_DV_CHANGE_OK)
			{
				printf ("\nLocal TAG Change error: %s", 
					devChangeRslt == MIS_DV_CHANGE_LOST ? "Lost" : "Full");
			}
			miuDev = list_get_next (misu_dev_list, miuDev);
		}
	}
}

/************************************************************************/
/*			misuChangeDev					*/
/************************************************************************/

static ST_INT misuChangeDev (MISU_DEV *miuDev, ST_BOOLEAN changeValueOnly)
{
	MIS_DEVICE *dev;
	ST_INT dvChangeRslt;

	dev = MIS_DEV_REF_TO_DEV (miuDev->devRef);
	QPRINTF ("\nChanging DEV '%s'", dev->device_name);

	/* Change the tag value */
	switch (dev->tag_value.TagFlags & MI_TAG_FLAG_TAG_MASK)
	{
	case MI_TAG_FLAG_NO_TAG:
		dev->tag_value.TagFlags &= ~ MI_TAG_FLAG_TAG_MASK;
		dev->tag_value.TagFlags |= MI_TAG_FLAG_OPEN_CLOSE_INHIBIT;
		strcpy (dev->tag_value.Reason, "No maintenance allowed");
		break;

	case MI_TAG_FLAG_OPEN_CLOSE_INHIBIT:
		dev->tag_value.TagFlags &= ~ MI_TAG_FLAG_TAG_MASK;
		dev->tag_value.TagFlags |= MI_TAG_FLAG_CLOSE_INHIBIT;
		strcpy (dev->tag_value.Reason, "No close allowed");
		break;

	case MI_TAG_FLAG_CLOSE_INHIBIT:
		dev->tag_value.TagFlags &= ~ MI_TAG_FLAG_TAG_MASK;
		dev->tag_value.TagFlags |= MI_TAG_FLAG_NO_TAG;
		strcpy (dev->tag_value.Reason, "All Clear");
		break;

	default:
		break;
	}

	if (!changeValueOnly)
		dvChangeRslt = mis_tag_value_change_ex (miuDev->devRef, &dev->tag_value);
	else
		dvChangeRslt = -1;

	return dvChangeRslt;
}
#endif	// CHOIBC DELETE

/************************************************************************/
/*			misuSendMessage					*/
/************************************************************************/
/* Send an IMTransfer Report
*/
#if 0
static ST_INT32 misuMessageId = 0;
#endif


#if 0	// CHOIBC DELETE
ST_VOID misuSendMessage (ST_VOID)
{

	ST_RET ret;
	MI_INFO_BUF_HEADER buf_header;
	MI_CONN **connArray;
	ST_UCHAR *info_buf;
	/* slog out pim  */ 
	ST_INT i;
	MISU_IM *pim;
	MISU_IM_ASSOC *im_assoc; 
	MI_ASSOC_CTRL *mi_assoc; 
	FILE * pFile;
	ST_INT32 size;

	pim = misu_im_list;
	while(pim != NULL)
	{
		SLOGALWAYS2("The Info Reference is %ld and the Size is %ld ", (ST_LONG) pim->InfoReference, (ST_LONG) pim->maxSize);
		SLOGCALWAYS1("there are %ld associations with this pim.",(ST_LONG)pim->numAssoc);
		if (pim->numAssoc > 0) /* Are there assocaitons */
		{
			im_assoc = pim->hol_mi_assoc;
			for (i = 1; i <= pim->numAssoc; i++)
			{
				if (im_assoc == NULL)			/* Klocwork Recommended */
					continue;

				mi_assoc = im_assoc->mi_assoc;
				if (pim->mapInfo[0] != 0) /* did someone map a file to this info message? */
				{
					SLOGCALWAYS2("im_assoc %s and the scope %d", im_assoc->mi_assoc->name,im_assoc->scope);
					SLOGCALWAYS1("the Message ID is %ld",(ST_LONG)pim->MessageId);
					pFile = fopen (pim->mapInfo,"r");
					if (pFile==NULL) continue; /* if the file mapping is bad skip this one */
					/* for every pim create a connection array and send out an info message */
					info_buf = chk_calloc(1,pim->maxSize);	
					connArray = chk_calloc(1, sizeof (MI_CONN *));
					size = pim->maxSize;
					while ((size == pim->maxSize) && (mi_assoc->state == MI_ASSOC_STATE_ACTIVE))
					{  
						size = fread (info_buf,1,(ST_LONG)pim->maxSize,pFile);

						connArray[0] = mi_assoc->mi_conn;
						/* send the information message */
						buf_header.InfoReference = pim->InfoReference;
						buf_header.MessageId =  pim->MessageId++;
						buf_header.Size = size;
						if (size == pim->maxSize)
							buf_header.LocalReference = 1;
						else
							buf_header.LocalReference = 0; 
						ret = mis_send_IMTransfer (1, connArray,
							&buf_header, size, info_buf, im_assoc->scope); 


					} /* end while */
					fclose (pFile);
					chk_free (info_buf);
					chk_free (connArray);
				} /* end if */
				else
				{  /* if no file is mapped in the configuation then a hard programmed message of all A's is sent */
					SLOGCALWAYS2("im_assoc %s and the scope %d", im_assoc->mi_assoc->name,im_assoc->scope);
					/* for every pim create a connection array and send out an info message */
					SLOGCALWAYS1("the Message ID is %ld",(ST_LONG)pim->MessageId);
					info_buf = chk_calloc(1,pim->maxSize);
					/* now open the file as specified in the map and put store the contents into info_buf */
					sprintf ((ST_CHAR *) info_buf, "This is an Information Message #%ld", (long) pim->MessageId);
					connArray = chk_calloc(1, sizeof (MI_CONN *));
					/* slog out associations*/
					if (mi_assoc->state == MI_ASSOC_STATE_ACTIVE)
					{
						connArray[0] = mi_assoc->mi_conn;
						/* send the information message */
						buf_header.InfoReference = pim->InfoReference;
						buf_header.LocalReference = pim->LocalReference;
						buf_header.MessageId =  pim->MessageId++;
						buf_header.Size = strlen ((ST_CHAR *) info_buf) + 1;

						ret = mis_send_IMTransfer (1, connArray,
							&buf_header, pim->maxSize, info_buf, im_assoc->scope);
					} /* end if */
					chk_free (info_buf);
					chk_free (connArray);
				} /* end else */
				im_assoc = list_get_next(pim->hol_mi_assoc,im_assoc);
			} /* end for */
		} /* end if */
		pim =list_get_next(misu_im_list,pim);
	} /* end while */
}
#endif	// CHOIBC DELETE




/************************************************************************/
/*			u_mis_general_data_resp				*/
/************************************************************************/

ST_VOID u_mis_general_data_resp (MI_CONN *mi_conn, MI_GEN_DATA_RESP *gen_data_resp)
{
	time_t tt;
	int i;

	SLOGALWAYS1 ("General DataResponse received from remote %s", mi_conn->mi_remote->name);

	SLOGCALWAYS1 ("      ReportReferenceNumber:  %d", gen_data_resp->ReportReferenceNumber);
	SLOGCALWAYS1 ("                 ReportName:  %s", gen_data_resp->ReportName);
	tt = (time_t) gen_data_resp->ReportTimeStamp;
	SLOGCALWAYS1 ("            ReportTimeStamp:  %s", ctime(&tt));
	SLOGCALWAYS1 ("     NumberOfLocalReference:  %d", gen_data_resp->NumberOfLocalReference);
	if (gen_data_resp->NumberOfLocalReference > 0)
	{
		for (i = 0; i < gen_data_resp->NumberOfLocalReference; i++)
		{
			SLOGCALWAYS2 ("     LocalReference %d:  %d", i, gen_data_resp->ListOfLocalReference[i]);
		}
	}

	SLOGCALWAYS1 ("               ResponseData:  %d", gen_data_resp->ResponseData);
	SLOGCALWAYS1 ("               ResponseCode:  %d", gen_data_resp->ResponseCode);
	SLOGCALWAYS1 ("               ResponseText:  %s", gen_data_resp->ResponseText);
}


/************************************************************************/
/*		      u_mis_start_transfer_accounts			*/
/************************************************************************/

ST_RET u_mis_start_transfer_accounts (MI_CONN *mi_conn, ST_UCHAR conditions_requested)
{
	return SD_SUCCESS;
}

/************************************************************************/
/*		      u_mis_stop_transfer_accounts			*/
/************************************************************************/
ST_RET u_mis_stop_transfer_accounts (MI_CONN *mi_conn)
{
	mi_set_compatability_mode (mi_conn->mi_remote, 0);
	/*  mi_set_compatability_mode (mi_conn->mi_remote, MI_B8_MODE_SISCO_ICCP_TOOLKIT); */
	return SD_SUCCESS;
}

/************************************************************************/
/*			    u_mis_query					*/
/************************************************************************/

MIS_QUERY_ERROR u_mis_query (MI_CONN *mi_conn, MI_ACCOUNT_REQUEST *acct_request)
{
	return mis_query_error_none;
}


/************************************************************************/
/*			misuSetUpFloatMatrix				*/
/************************************************************************/

static ST_FLOAT lastFloat = (ST_FLOAT) 1.11;

ST_VOID misuSetUpFloatMatrix (MI_FLOAT_MATRIX *floatMatrix)
{
	ST_INT numElements;
	int i;

	floatMatrix->MatrixIds = chk_calloc (floatMatrix->NumCols, sizeof (ST_INT32));
	for (i = 0; i < floatMatrix->NumCols; i++)
		floatMatrix->MatrixIds[i] = i;

	numElements = floatMatrix->NumCols * floatMatrix->NumRows;
	floatMatrix->FloatArray = chk_calloc (numElements, sizeof (ST_FLOAT));
	for (i = 0; i < numElements; i++)
	{
		floatMatrix->FloatArray[i] = lastFloat;
		lastFloat += (ST_FLOAT) 1.1;
	}
}

/************************************************************************/
/*			misuSetUpIntMatrix				*/
/************************************************************************/

static ST_INT32 lastInt = 0;

ST_VOID misuSetUpIntMatrix (MI_INTEGER_MATRIX *intMatrix)
{
	ST_INT numElements;
	int i;

	intMatrix->MatrixIds = chk_calloc (intMatrix->NumCols, sizeof (ST_INT32));
	for (i = 0; i < intMatrix->NumCols; i++)
		intMatrix->MatrixIds[i] = i;

	numElements = intMatrix->NumCols * intMatrix->NumRows;
	intMatrix->IntegerArray = chk_calloc (numElements, sizeof (ST_INT32));
	for (i = 0; i < numElements; i++)
	{
		intMatrix->IntegerArray[i] = lastInt++;
	}
}

/************************************************************************/
/*			misuFreeFloatMatrix				*/
/************************************************************************/

ST_VOID misuFreeFloatMatrix (MI_FLOAT_MATRIX *floatMatrix)
{
	chk_free (floatMatrix->FloatArray);
	chk_free (floatMatrix->MatrixIds);
}

/************************************************************************/
/*			misuFreeIntMatrix				*/
/************************************************************************/

ST_VOID misuFreeIntMatrix (MI_INTEGER_MATRIX *intMatrix)
{
	chk_free (intMatrix->IntegerArray);
	chk_free (intMatrix->MatrixIds);
}

#if 0	// CHOIBC DELETE
/************************************************************************/
/*			misuSendSegsPeriodic				*/
/************************************************************************/

ST_VOID misuSendSegsPeriodic (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_TA_REPORT *taRpt;
	time_t curTime;
	ST_RET rc;
	int i;
	MI_TA_PERIODIC *per;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_REPORT;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	taRpt = &acct.acct.taReport;
	per = &taRpt->Data.Periodic;

	taRpt->BuyingAgent = 1000;
	taRpt->DataType = TA_ACCT_DATA_TYPE_PERIODIC;
	per->NumberOfPeriods = 6;
	per->PeriodResolution = 3;
	per->StartTime = (ST_INT32) curTime++;
	per->ListOfPeriodicValues.floatMatrix.NumRows = per->NumberOfPeriods;
	per->ListOfPeriodicValues.floatMatrix.NumCols = 2;
	misuSetUpFloatMatrix (&per->ListOfPeriodicValues.floatMatrix);

	per->ListOfPeriodicValues.integerMatrix.NumCols = 4;
	per->ListOfPeriodicValues.integerMatrix.NumRows = per->NumberOfPeriods;
	misuSetUpIntMatrix (&per->ListOfPeriodicValues.integerMatrix);

	taRpt->TransmissionSegmentIncluded = SD_TRUE;
	taRpt->NumberofTransSegments = 2;
	taRpt->ListOfTransmissionSegment = chk_calloc (taRpt->NumberofTransSegments, sizeof (MI_TA_SEGMENT));
	for (i = 0; i < taRpt->NumberofTransSegments; i++)
	{
		per = &taRpt->ListOfTransmissionSegment[i].Data.Periodic;
		per->NumberOfPeriods = 6;
		per->PeriodResolution = 3;
		per->StartTime = (ST_INT32) curTime++;
		per->ListOfPeriodicValues.floatMatrix.NumCols = 3;
		per->ListOfPeriodicValues.floatMatrix.NumRows = per->NumberOfPeriods;
		misuSetUpFloatMatrix (&per->ListOfPeriodicValues.floatMatrix);

		per->ListOfPeriodicValues.integerMatrix.NumCols = 3;
		per->ListOfPeriodicValues.integerMatrix.NumRows = per->NumberOfPeriods;
		misuSetUpIntMatrix (&per->ListOfPeriodicValues.integerMatrix);

		taRpt->ListOfTransmissionSegment[i].TransmissionReference = 1001;
		taRpt->ListOfTransmissionSegment[i].TransmissionSegType = TA_SEG_TYPE_INOUT;
		taRpt->ListOfTransmissionSegment[i].TransmissionSegUnion.inOut.InterchangePtIn = 1234;
		taRpt->ListOfTransmissionSegment[i].TransmissionSegUnion.inOut.InterchangePtOut = 2345;
		taRpt->ListOfTransmissionSegment[i].TransmissionSegUnion.inOut.UtilIn = 3456;
		taRpt->ListOfTransmissionSegment[i].TransmissionSegUnion.inOut.UtilOut = 4567;
		taRpt->ListOfTransmissionSegment[i].UtilPaying = 2001;
		taRpt->ListOfTransmissionSegment[i].UtilWheeling = 3001;
	}

	taRpt->LocalRef.NumberOfLocalReference = 4;
	taRpt->LocalRef.ListOfLocalReference = chk_calloc (taRpt->LocalRef.NumberOfLocalReference, sizeof (ST_INT32));
	for (i = 0; i < taRpt->LocalRef.NumberOfLocalReference; i++)
		taRpt->LocalRef.ListOfLocalReference[i] = i;

	strcpy (taRpt->Name, "TA Segments Periodic");
	taRpt->ReceiveUtility = 2000;
	taRpt->SellingAgent = 3000;
	taRpt->SendUtility = 4000;
	taRpt->TimeStamp = (ST_INT32) curTime++;
	taRpt->TransactionCode = TA_TRAN_CODE_CONFIRMED;
	taRpt->TransferAccountReference = 100;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent TA Segments Periodic Report successfully");
	else
		SLOGALWAYS0 ("Send TA Segments Periodic Report FAILED");

	chk_free (taRpt->LocalRef.ListOfLocalReference);
	misuFreeFloatMatrix (&taRpt->Data.Periodic.ListOfPeriodicValues.floatMatrix);
	misuFreeIntMatrix (&taRpt->Data.Periodic.ListOfPeriodicValues.integerMatrix);
	for (i = 0; i < taRpt->NumberofTransSegments; i++)
	{
		per = &taRpt->ListOfTransmissionSegment[i].Data.Periodic;
		misuFreeFloatMatrix (&per->ListOfPeriodicValues.floatMatrix);
		misuFreeIntMatrix (&per->ListOfPeriodicValues.integerMatrix);
	}
	chk_free (taRpt->ListOfTransmissionSegment);
}

/************************************************************************/
/*			misuSendNoSegsPeriodic				*/
/************************************************************************/

ST_VOID misuSendNoSegsPeriodic (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_TA_REPORT *taRpt;
	time_t curTime;
	ST_RET rc;
	int i;
	MI_TA_PERIODIC *per;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_REPORT;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	taRpt = &acct.acct.taReport;
	per = &taRpt->Data.Periodic;

	taRpt->BuyingAgent = 1000;
	taRpt->DataType = TA_ACCT_DATA_TYPE_PERIODIC;
	per->NumberOfPeriods = 6;
	per->PeriodResolution = 3;
	per->StartTime = (ST_INT32) curTime++;
	per->ListOfPeriodicValues.floatMatrix.NumRows = per->NumberOfPeriods;
	per->ListOfPeriodicValues.floatMatrix.NumCols = 2;
	misuSetUpFloatMatrix (&per->ListOfPeriodicValues.floatMatrix);

	per->ListOfPeriodicValues.integerMatrix.NumCols = 4;
	per->ListOfPeriodicValues.integerMatrix.NumRows = per->NumberOfPeriods;
	misuSetUpIntMatrix (&per->ListOfPeriodicValues.integerMatrix);

	taRpt->TransmissionSegmentIncluded = SD_FALSE;
	taRpt->NumberofTransSegments = 0;

	taRpt->LocalRef.NumberOfLocalReference = 4;
	taRpt->LocalRef.ListOfLocalReference = chk_calloc (taRpt->LocalRef.NumberOfLocalReference, sizeof (ST_INT32));
	for (i = 0; i < taRpt->LocalRef.NumberOfLocalReference; i++)
		taRpt->LocalRef.ListOfLocalReference[i] = i;

	strcpy (taRpt->Name, "TA NO Segments Periodic");
	taRpt->ReceiveUtility = 2000;
	taRpt->SellingAgent = 3000;
	taRpt->SendUtility = 4000;
	taRpt->TimeStamp = (ST_INT32) curTime++;
	taRpt->TransactionCode = TA_TRAN_CODE_CONFIRMED;
	taRpt->TransferAccountReference = 200;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent TA NO Segments Periodic Report successfully");
	else
		SLOGALWAYS0 ("Send TA NO Segments Periodic Report FAILED");

	chk_free (taRpt->LocalRef.ListOfLocalReference);
	misuFreeFloatMatrix (&taRpt->Data.Periodic.ListOfPeriodicValues.floatMatrix);
	misuFreeIntMatrix (&taRpt->Data.Periodic.ListOfPeriodicValues.integerMatrix);
}

/************************************************************************/
/*			misuSendSegsProfile				*/
/************************************************************************/

ST_VOID misuSendSegsProfile (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_TA_REPORT *taRpt;
	time_t curTime;
	ST_RET rc;
	MI_TA_PROFILE_VALUE *profile;
	int i, j;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_REPORT;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	taRpt = &acct.acct.taReport;

	taRpt->DataType = TA_ACCT_DATA_TYPE_PROFILE;
	taRpt->BuyingAgent = 1000;
	taRpt->Data.Profile.NumberOfProfiles = 3;
	taRpt->Data.Profile.ListOfProfileValues = chk_calloc (taRpt->Data.Profile.NumberOfProfiles, sizeof (MI_TA_PROFILE_VALUE));
	for (i = 0; i < taRpt->Data.Profile.NumberOfProfiles; i++)
	{
		profile = &taRpt->Data.Profile.ListOfProfileValues[i];
		profile->Price = (ST_FLOAT) (52.52 + i);
		profile->Profile = (ST_FLOAT) (123.45 + i);
		profile->RampDuration = 123 + i;
		profile->RampStartTime = (ST_INT32) curTime++;
		profile->TargetClass = 10 + i;
	}

	taRpt->TransmissionSegmentIncluded = SD_TRUE;
	taRpt->NumberofTransSegments = 2;
	taRpt->ListOfTransmissionSegment = chk_calloc (taRpt->NumberofTransSegments, sizeof (MI_TA_SEGMENT));
	for (i = 0; i < taRpt->NumberofTransSegments; i++)
	{
		taRpt->ListOfTransmissionSegment[i].Data.Profile.NumberOfProfiles = 3;
		taRpt->ListOfTransmissionSegment[i].Data.Profile.ListOfProfileValues = chk_calloc (3, sizeof (MI_TA_PROFILE_VALUE));
		for (j = 0; j < taRpt->ListOfTransmissionSegment[i].Data.Profile.NumberOfProfiles; j++)
		{
			profile = &taRpt->ListOfTransmissionSegment[i].Data.Profile.ListOfProfileValues[j];
			profile->Price = (ST_FLOAT) (12.93 + i +j);
			profile->Profile = (ST_FLOAT) (501.23 + i + j);
			profile->RampDuration = 83 + i + j;
			profile->RampStartTime = (ST_INT32) curTime++;
			profile->TargetClass = 20 + i + j;
		}

		taRpt->ListOfTransmissionSegment[i].TransmissionReference = 5002;
		taRpt->ListOfTransmissionSegment[i].TransmissionSegType = TA_SEG_TYPE_DIRECT;
		taRpt->ListOfTransmissionSegment[i].TransmissionSegUnion.direct.InterchangePt = 1122;
		taRpt->ListOfTransmissionSegment[i].UtilPaying = 6002;
		taRpt->ListOfTransmissionSegment[i].UtilWheeling = 7002;
	}

	taRpt->LocalRef.NumberOfLocalReference = 4;
	taRpt->LocalRef.ListOfLocalReference = chk_calloc (taRpt->LocalRef.NumberOfLocalReference, sizeof (ST_INT32));
	for (i = 0; i < taRpt->LocalRef.NumberOfLocalReference; i++)
		taRpt->LocalRef.ListOfLocalReference[i] = i*3;

	strcpy (taRpt->Name, "Ta Segments Profile");
	taRpt->ReceiveUtility = 6000;
	taRpt->SellingAgent = 7000;
	taRpt->SendUtility = 8000;
	taRpt->TimeStamp = (ST_INT32) curTime++;
	taRpt->TransactionCode = TA_TRAN_CODE_REVISED;
	taRpt->TransferAccountReference = 300;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent Ta Segments Profile Report successfully");
	else
		SLOGALWAYS0 ("Send Ta Segments Profile Report FAILED");

	for (i = 0; i < taRpt->NumberofTransSegments; i++)
	{
		chk_free (taRpt->ListOfTransmissionSegment[i].Data.Profile.ListOfProfileValues);
	}

	chk_free (taRpt->ListOfTransmissionSegment);
	chk_free (taRpt->LocalRef.ListOfLocalReference);
	chk_free (taRpt->Data.Profile.ListOfProfileValues);
}

/************************************************************************/
/*			misuNoSendSegsProfile				*/
/************************************************************************/

ST_VOID misuSendNoSegsProfile (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_TA_REPORT *taRpt;
	time_t curTime;
	ST_RET rc;
	MI_TA_PROFILE_VALUE *profile;
	int i;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_REPORT;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	taRpt = &acct.acct.taReport;

	taRpt->DataType = TA_ACCT_DATA_TYPE_PROFILE;
	taRpt->BuyingAgent = 1000;
	taRpt->Data.Profile.NumberOfProfiles = 3;
	taRpt->Data.Profile.ListOfProfileValues = chk_calloc (taRpt->Data.Profile.NumberOfProfiles, sizeof (MI_TA_PROFILE_VALUE));
	for (i = 0; i < taRpt->Data.Profile.NumberOfProfiles; i++)
	{
		profile = &taRpt->Data.Profile.ListOfProfileValues[i];
		profile->Price = (ST_FLOAT) (52.52 + i);
		profile->Profile = (ST_FLOAT) (123.45 + i);
		profile->RampDuration = 123 + i;
		profile->RampStartTime = (ST_INT32) curTime++;
		profile->TargetClass = 10 + i;
	}

	taRpt->TransmissionSegmentIncluded = SD_FALSE;
	taRpt->NumberofTransSegments = 0;

	taRpt->LocalRef.NumberOfLocalReference = 4;
	taRpt->LocalRef.ListOfLocalReference = chk_calloc (taRpt->LocalRef.NumberOfLocalReference, sizeof (ST_INT32));
	for (i = 0; i < taRpt->LocalRef.NumberOfLocalReference; i++)
		taRpt->LocalRef.ListOfLocalReference[i] = i*3;

	strcpy (taRpt->Name, "Ta NoSegments Profile");
	taRpt->ReceiveUtility = 6000;
	taRpt->SellingAgent = 7000;
	taRpt->SendUtility = 8000;
	taRpt->TimeStamp = (ST_INT32) curTime++;
	taRpt->TransactionCode = TA_TRAN_CODE_REVISED;
	taRpt->TransferAccountReference = 300;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent Ta NO Segments Profile Report successfully");
	else
		SLOGALWAYS0 ("Send Ta NO Segments Profile Report FAILED");

	chk_free (taRpt->LocalRef.ListOfLocalReference);
	chk_free (taRpt->Data.Profile.ListOfProfileValues);
}

/************************************************************************/
/*			misuSendDevOutNew				*/
/************************************************************************/

ST_VOID misuSendDevOutNew (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_DEVICE_OUTAGE *devOut;
	time_t curTime;
	ST_RET rc;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_DEVICE_OUTAGE;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	devOut = &acct.acct.deviceOutage;

	devOut->Activity = MI_DO_ACTIVITY_NEWPLAN;
	devOut->ActivityData.NewRevise.Class = MI_DO_CLASS_OUTSERVICE;
	devOut->ActivityData.NewRevise.OutageAmountType = MI_DO_OUTAGE_AMT_TYPE_FULL;
	devOut->ActivityData.NewRevise.OutagePeriod = 123;
	devOut->ActivityData.NewRevise.OutageType = MI_DO_OUTAGE_TYPE_UNPLANNED;
	devOut->ActivityData.NewRevise.PlannedCloseOrInServiceDateAndTime = (ST_INT32) curTime++;
	devOut->ActivityData.NewRevise.PlannedOpenOrOutOfServiceDateAndTime = (ST_INT32) curTime++;
	devOut->ActivityData.NewRevise.PlanType = MI_DO_NEW_REVISE_PLAN_SCHEDULED;
	devOut->ActivityDateAndTime = (ST_INT32) curTime++;
	strcpy (devOut->Comments, "New Comments");
	strcpy (devOut->DeviceName, "New Device Name");
	devOut->DeviceNumber = 4233;
	devOut->DeviceRating = (ST_FLOAT) 14.92;
	devOut->DeviceType = MI_DO_DEV_TYPE_TRANSFORMER;
	strcpy (devOut->OutageEffect, "New Outage Effect");
	devOut->OutageReferenceId = 200;
	devOut->OwningUtilityID = 2000;
	strcpy (devOut->StationName, "New Station Name");
	devOut->TimeStamp = (ST_INT32) curTime;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent Device Outage New Report successfully");
	else
		SLOGALWAYS0 ("Send Device Outage New Report FAILED");
}

/************************************************************************/
/*			misuSendDevOutRevised				*/
/************************************************************************/

ST_VOID misuSendDevOutRevised (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_DEVICE_OUTAGE *devOut;
	time_t curTime;
	ST_RET rc;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_DEVICE_OUTAGE;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	devOut = &acct.acct.deviceOutage;

	devOut->Activity = MI_DO_ACTIVITY_REVISE;
	devOut->ActivityData.NewRevise.Class = MI_DO_CLASS_OUTSERVICE;
	devOut->ActivityData.NewRevise.OutageAmountType = MI_DO_CLASS_INSERVICE;
	devOut->ActivityData.NewRevise.OutagePeriod = 321;
	devOut->ActivityData.NewRevise.OutageType = MI_DO_OUTAGE_TYPE_ECONOMY;
	devOut->ActivityData.NewRevise.PlannedCloseOrInServiceDateAndTime = (ST_INT32) curTime++;
	devOut->ActivityData.NewRevise.PlannedOpenOrOutOfServiceDateAndTime = (ST_INT32) curTime++;
	devOut->ActivityData.NewRevise.PlanType = MI_DO_NEW_REVISE_PLAN_ESTIMATED;
	devOut->ActivityDateAndTime = (ST_INT32) curTime++;
	strcpy (devOut->Comments, "Revised Comments");
	strcpy (devOut->DeviceName, "Revised Device Name");
	devOut->DeviceNumber = 5323;
	devOut->DeviceRating = (ST_FLOAT) 92.14;
	devOut->DeviceType = MI_DO_DEV_TYPE_CAPACITOR;
	strcpy (devOut->OutageEffect, "Revised Outage Effect");
	devOut->OutageReferenceId = 300;
	devOut->OwningUtilityID = 3000;
	strcpy (devOut->StationName, "Revised Station Name");
	devOut->TimeStamp = (ST_INT32) curTime;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent Device Outage Revised Report successfully");
	else
		SLOGALWAYS0 ("Send Device Outage Revised Report FAILED");
}

/************************************************************************/
/*			misuSendDevOutCancel				*/
/************************************************************************/

ST_VOID misuSendDevOutCancel (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_DEVICE_OUTAGE *devOut;
	time_t curTime;
	ST_RET rc;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_DEVICE_OUTAGE;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	devOut = &acct.acct.deviceOutage;

	devOut->Activity = MI_DO_ACTIVITY_CANCEL;
	devOut->ActivityDateAndTime = (ST_INT32) curTime++;
	strcpy (devOut->Comments, "Device Outage Cancel Report Comments");
	strcpy (devOut->DeviceName, "DeviceNameCancel");
	devOut->DeviceNumber = 111;
	devOut->DeviceRating = (ST_FLOAT) 2.345;
	devOut->DeviceType = MI_DO_DEV_TYPE_TRANSMISSION_CIRCUIT;
	strcpy (devOut->OutageEffect, "Device Outage Cancel Report OutageEffect");
	devOut->OutageReferenceId = 400;
	devOut->OwningUtilityID = 555;
	strcpy (devOut->StationName, "StationNameCancel");
	devOut->TimeStamp = (ST_INT32) curTime;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent Device Outage Cancel Report successfully");
	else
		SLOGALWAYS0 ("Send Device Outage Cancel Report FAILED");
}

/************************************************************************/
/*			misuSendDevOutActual				*/
/************************************************************************/

ST_VOID misuSendDevOutActual (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_DEVICE_OUTAGE *devOut;
	time_t curTime;
	ST_RET rc;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_DEVICE_OUTAGE;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	devOut = &acct.acct.deviceOutage;

	devOut->Activity = MI_DO_ACTIVITY_ACTUAL;
	devOut->ActivityData.Actual.Action = MI_DO_ACTION_ONLINE;
	devOut->ActivityDateAndTime = (ST_INT32) curTime++;
	strcpy (devOut->Comments, "Actual Comments");
	strcpy (devOut->DeviceName, "Actual Device Name");
	devOut->DeviceNumber = 5280;
	devOut->DeviceRating = (ST_FLOAT) 24.7;
	devOut->DeviceType = MI_DO_DEV_TYPE_CAPACITOR;
	strcpy (devOut->OutageEffect, "Acutal Outage Effect");
	devOut->OutageReferenceId = 500;
	devOut->OwningUtilityID = 1000;
	strcpy (devOut->StationName, "Acutal StationName");
	devOut->TimeStamp = (ST_INT32) curTime;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent Device Outage Actual Report successfully");
	else
		SLOGALWAYS0 ("Send Device Outage Actual Report FAILED");
}

/************************************************************************/
/*			misuSendAvailable				*/
/************************************************************************/

ST_VOID misuSendAvailable (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_AVAILABILITY *avail;
	time_t curTime;
	ST_RET rc;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_AVAILABILITY;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	avail = &acct.acct.availability;

	avail->AvailabilityStatus = MI_AVAIL_STATUS_AVAILABLE;
	avail->AvailabilityData.Available.TypeOfAvailability = MI_AVAIL_TYPE_STANDBY;
	avail->AvailabilityData.Available.AvailData.TimeToOnline = (ST_INT32) curTime++;
	avail->AvailabilityData.Available.UnitCapacity = MI_UNIT_CAPACITY_GROSS;
	avail->AvailabilityData.Available.Capacity.Gross.GrossMaxCapacity  = 4523;
	avail->AvailabilityData.Available.Capacity.Gross.GrossMinCapacity =  2345;
	avail->AvailabilityData.Available.CapacityImpact = SD_TRUE;
	strcpy (avail->AvailabilityData.Available.Comment, "Available Comment");
	avail->AvailabilityData.Available.EconomicImpact = SD_TRUE;
	avail->AvailabilityData.Available.LFC = SD_FALSE;
	avail->AvailabilityData.Available.PriceImpact = (ST_FLOAT) 22.33;
	avail->AvailabilityData.Available.RampRateImpact = SD_TRUE;
	avail->AvailabilityData.Available.RampRates.MaxRampRateDown = (ST_FLOAT)1234.56;
	avail->AvailabilityData.Available.RampRates.MaxRampRateUp = (ST_FLOAT) 2345.67;
	avail->AvailabilityReferenceID = 600;
	avail->Duration = 111;
	avail->PlantReferenceID = 9000;
	avail->ProvidingReserve = SD_TRUE;
	avail->ReportStatus = MI_AVAIL_RPT_STATUS_CONFIRMED;
	avail->StartDateAndTime = (ST_INT32) curTime++;
	avail->StopDateAndTime = (ST_INT32) curTime++;
	avail->TimeStamp = (ST_INT32) curTime++;
	avail->UnitID = 1000;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent Available Report successfully");
	else
		SLOGALWAYS0 ("Send Available Report FAILED");
}

/************************************************************************/
/*			misuSendUnavailable				*/
/************************************************************************/

ST_VOID misuSendUnavailable (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_AVAILABILITY *avail;
	time_t curTime;
	ST_RET rc;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_AVAILABILITY;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	avail = &acct.acct.availability;

	avail->AvailabilityStatus = MI_AVAIL_STATUS_UNAVAILABLE;
	strcpy (avail->AvailabilityData.Unavailable.Comment, "Unavailable Comment");
	avail->AvailabilityData.Unavailable.ReasonForUnavailable = MI_UNAVAIL_REASON_SCHEDULED;
	avail->AvailabilityReferenceID = 700;
	avail->Duration = 55;
	avail->PlantReferenceID = 9000;
	avail->ProvidingReserve = SD_TRUE;
	avail->ReportStatus = MI_AVAIL_RPT_STATUS_PROPOSED;
	avail->StartDateAndTime = (ST_INT32) curTime++;
	avail->StopDateAndTime = (ST_INT32) curTime++;
	avail->TimeStamp = (ST_INT32) curTime++;;
	avail->UnitID = 1000;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent Unavailable Report successfully");
	else
		SLOGALWAYS0 ("Send Unavailable Report FAILED");
}

/************************************************************************/
/*			misuSendStatusAvail				*/
/************************************************************************/

ST_VOID misuSendStatusAvail (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_REAL_TIME_STATUS *rtsAvail;
	time_t curTime;
	ST_RET rc;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_REAL_TIME_STATUS;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	rtsAvail = &acct.acct.realTimeStatus;

	rtsAvail->AvailabilityStatus = MI_AVAIL_STATUS_AVAILABLE;
	rtsAvail->AvailData.Available.AvailData.LFC_yes.Dispatchable = SD_TRUE;
	rtsAvail->AvailData.Available.AvailData.LFC_yes.Manually_Loaded = SD_TRUE;
	rtsAvail->AvailData.Available.AvailData.LFC_yes.Regulating = SD_TRUE;
	rtsAvail->AvailData.Available.UnitCapacity = MI_UNIT_CAPACITY_NET;
	rtsAvail->AvailData.Available.Capacity.Net.NetMaxCapacity = (ST_FLOAT) 234.56;
	rtsAvail->AvailData.Available.Capacity.Net.NetMinCapacity = (ST_FLOAT)  10.01;
	rtsAvail->AvailData.Available.ExternallyBlockedHigh = SD_TRUE;
	rtsAvail->AvailData.Available.ExternallyBlockedLow = SD_TRUE;
	rtsAvail->AvailData.Available.LFC = SD_TRUE;
	rtsAvail->AvailData.Available.MaxRampRateDown = (ST_FLOAT) 111.22;
	rtsAvail->AvailData.Available.MaxRampRateUp = (ST_FLOAT) 222.33;
	rtsAvail->AvailData.Available.TypeOfAvailability = MI_AVAIL_TYPE_ONLINE;
	rtsAvail->PlantReferenceID = 9000;
	rtsAvail->ProvidingReserve = SD_TRUE;
	rtsAvail->RealTimeStatusReferenceID = 800;
	rtsAvail->TimeStamp = (ST_INT32) curTime++;
	rtsAvail->UnitID = 1000;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent Status Available Report successfully");
	else
		SLOGALWAYS0 ("Send Status Available Report FAILED");
}

/************************************************************************/
/*			misuSendStatusUnavail				*/
/************************************************************************/

ST_VOID misuSendStatusUnavail (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_REAL_TIME_STATUS *rtsAvail;
	time_t curTime;
	ST_RET rc;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_REAL_TIME_STATUS;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	rtsAvail = &acct.acct.realTimeStatus;

	rtsAvail->AvailabilityStatus = MI_AVAIL_STATUS_UNAVAILABLE;
	rtsAvail->AvailData.ReasonForUnavailable = MI_UNAVAIL_REASON_TESTING;
	rtsAvail->PlantReferenceID = 9000;
	rtsAvail->ProvidingReserve = SD_TRUE;
	rtsAvail->RealTimeStatusReferenceID = 900;
	rtsAvail->TimeStamp = (ST_INT32) curTime;
	rtsAvail->UnitID = 1000;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent Status UnAvailable Report successfully");
	else
		SLOGALWAYS0 ("Send Status UnAvailable Report FAILED");
}

/************************************************************************/
/*			misuSendForecast				*/
/************************************************************************/

ST_VOID misuSendForecast (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_FORCAST_SCHEDULE *forecast;
	time_t curTime;
	ST_RET rc;
	int i;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_FORECAST;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	forecast = &acct.acct.forecast;

	forecast->NumberOfPeriods = 5;
	forecast->ForecastScheduleReferenceID = 1000;
	forecast->ForecastType = MI_FORECAST_TYPE_BOTH;
	forecast->ListOfForecasts.LFC_Code.NumCols = 1;     /* must be 1  */
	forecast->ListOfForecasts.LFC_Code.NumRows = (ST_INT16) forecast->NumberOfPeriods;
	forecast->ListOfForecasts.LFC_Code.MatrixIds = chk_calloc (1, sizeof (ST_INT32));
	forecast->ListOfForecasts.LFC_Code.MatrixIds[0] = 53;
	forecast->ListOfForecasts.LFC_Code.IntegerArray = 
		chk_calloc (forecast->ListOfForecasts.LFC_Code.NumRows, sizeof (ST_INT32));
	for (i = 0; i < forecast->ListOfForecasts.LFC_Code.NumRows; i++)
		forecast->ListOfForecasts.LFC_Code.IntegerArray[i] = i;

	forecast->ListOfForecasts.MW.NumCols = 1;
	forecast->ListOfForecasts.MW.NumRows = (ST_INT16) forecast->NumberOfPeriods;
	forecast->ListOfForecasts.MW.MatrixIds = chk_calloc (1, sizeof (ST_INT32));
	forecast->ListOfForecasts.MW.MatrixIds[0] = 2;
	forecast->ListOfForecasts.MW.FloatArray = 
		chk_calloc (forecast->ListOfForecasts.MW.NumRows, sizeof (ST_FLOAT));
	for (i = 0; i < forecast->ListOfForecasts.MW.NumRows; i++)
		forecast->ListOfForecasts.MW.FloatArray[i] = (ST_FLOAT) (i * 1.25);

	forecast->PeriodResolution = 3;
	forecast->PlantReferenceID = 9000;
	forecast->StartTime = (ST_INT32) curTime++;
	forecast->UnitID = 1000;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent Status Forecast Report successfully");
	else
		SLOGALWAYS0 ("Send Status Forecast Report FAILED");

	chk_free (forecast->ListOfForecasts.LFC_Code.MatrixIds);
	chk_free (forecast->ListOfForecasts.LFC_Code.IntegerArray);
	chk_free (forecast->ListOfForecasts.MW.MatrixIds);
	chk_free (forecast->ListOfForecasts.MW.FloatArray);
}

/************************************************************************/
/*			misuSendCurve					*/
/************************************************************************/

ST_VOID misuSendCurve (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_CURVE *curve;
	time_t curTime;
	ST_RET rc;
	int i, j;
	MI_CURVE_SEGMENT_DESCR *seg;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_CURVE;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	curve = &acct.acct.curve;

	strcpy (curve->CurveName, "Curve Name");
	BSTR_BIT_SET_ON (curve->CurveType, MI_CURVE_TYPE_MVAR_CAP);
	curve->NumberOfSegments = 3;
	curve->PlantReferenceID = 9000;
	curve->SequenceOfCurveSegmentDescription = chk_calloc (curve->NumberOfSegments, sizeof (MI_CURVE_SEGMENT_DESCR));
	for (i = 0; i < curve->NumberOfSegments; i++)
	{
		seg = &curve->SequenceOfCurveSegmentDescription[i];
		seg->HighRange = (ST_FLOAT) 1234.5;
		seg->LowRange = (ST_FLOAT) 34.5;
		seg->Order = 4;
		seg->SequenceOfCoefficients = chk_calloc (seg->Order, sizeof (ST_FLOAT));
		for (j = 0; j < seg->Order; j++)
			seg->SequenceOfCoefficients[j] = (ST_FLOAT) (j * 33.93);
	}

	curve->UnitID = 1000;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent Status Curve Report successfully");
	else
		SLOGALWAYS0 ("Send Status Curve Report FAILED");

	for (i = 0; i < curve->NumberOfSegments; i++)
	{
		seg = &curve->SequenceOfCurveSegmentDescription[i];
		chk_free (seg->SequenceOfCoefficients);
	}

	chk_free (curve->SequenceOfCurveSegmentDescription);
}

/************************************************************************/
/*			misuSendGenData					*/
/************************************************************************/

ST_VOID misuSendGenData (MI_CONN *mi_conn)
{
	MI_TRANSFER_ACCOUNT acct;
	MI_GEN_DATA_RPT *genData;
	time_t curTime;
	ST_RET rc;
	int i, numFloats, numInts, numText;

	memset (&acct, 0, sizeof (MI_TRANSFER_ACCOUNT));
	curTime = time (NULL);
	acct.type = MI_TA_TYPE_GEN_DATA;
	acct.RequestId = 0;
	acct.TAConditionsDetected = TA_COND_DuringTheHour;
	genData = &acct.acct.generalData;

	genData->FloatingPoint1.NumCols = 3;
	genData->FloatingPoint1.NumRows = 4;
	numFloats = genData->FloatingPoint1.NumCols *genData->FloatingPoint1.NumRows;
	genData->FloatingPoint1.MatrixIds = chk_calloc (genData->FloatingPoint1.NumCols, sizeof (ST_INT32));
	for (i = 0; i < genData->FloatingPoint1.NumCols; i++)
		genData->FloatingPoint1.MatrixIds[i] = i;

	genData->FloatingPoint1.FloatArray = chk_calloc (numFloats, sizeof (ST_FLOAT));
	for (i = 0; i < numFloats; i++)
		genData->FloatingPoint1.FloatArray[i] = (ST_FLOAT) (i * 5.16);

	genData->GeneralDataReportReferenceNumber = 1100;
	genData->IntegerValues1.NumCols = 2;
	genData->IntegerValues1.NumRows = 3;
	numInts = genData->IntegerValues1.NumCols * genData->IntegerValues1.NumRows;
	genData->IntegerValues1.MatrixIds = chk_calloc (genData->IntegerValues1.NumCols, sizeof (ST_INT32));
	for (i = 0; i < genData->IntegerValues1.NumCols; i++)
		genData->IntegerValues1.MatrixIds[i] = 3 * i;

	genData->IntegerValues1.IntegerArray = chk_calloc (numInts, sizeof (ST_INT32));
	for (i = 0; i < numInts; i++)
		genData->IntegerValues1.IntegerArray[i] = i;

	genData->LocalRef.NumberOfLocalReference = 4;
	genData->LocalRef.ListOfLocalReference = chk_calloc (genData->LocalRef.NumberOfLocalReference, sizeof (ST_INT32));
	for (i = 0; i < genData->LocalRef.NumberOfLocalReference; i++)
		genData->LocalRef.ListOfLocalReference[i] = 10 * i;

	genData->ReportDateAndTime = (ST_INT32) curTime++;
	strcpy (genData->ReportName, "General Data Report Name");
	genData->Text1.NumCols = 4;
	genData->Text1.NumRows = 2;
	numText = genData->Text1.NumCols * genData->Text1.NumRows;
	genData->Text1.MatrixIds = chk_calloc (genData->Text1.NumCols, sizeof (ST_INT32));
	for (i = 0; i < genData->Text1.NumCols; i++)
		genData->Text1.MatrixIds[i] = i + 500;

	genData->Text1.Text32Array = chk_calloc (numText, sizeof (MI_NAME_STRING));
	for (i = 0; i < numText; i++)
	{
		sprintf (genData->Text1.Text32Array[i], "TextArrayElement %d", i);
	}

	genData->TransactionCode = 254;

	rc = mis_send_transfer_report (mi_conn, &acct);
	if (rc == SD_SUCCESS)
		SLOGALWAYS0 ("Sent General Data Report successfully");
	else
		SLOGALWAYS0 ("Send General Data Report FAILED");
	chk_free (genData->FloatingPoint1.MatrixIds);
	chk_free (genData->FloatingPoint1.FloatArray);
	chk_free (genData->IntegerValues1.MatrixIds);
	chk_free (genData->IntegerValues1.IntegerArray);
	chk_free (genData->LocalRef.ListOfLocalReference);
	chk_free (genData->Text1.MatrixIds);
	chk_free (genData->Text1.Text32Array);
}

/************************************************************************/
/*			misuSendBlock8					*/
/************************************************************************/

static int lastSent = 0;

ST_VOID misuSendBlock8 (MI_CONN *mi_conn)
{
	mms_debug_sel |= MMS_LOG_MIDE;

	switch (lastSent)
	{
	case 0:
		printf ("\n Sending TA_Segements_Periodic");
		misuSendSegsPeriodic (mi_conn);
		lastSent++;
		break;
	case 1:
		printf ("\n Sending TA_No_Sements_Periodic");
		misuSendNoSegsPeriodic (mi_conn);
		lastSent++;
		break;
	case 2:
		printf ("\n Sending TA_Segments_Profile");
		misuSendSegsProfile (mi_conn);
		lastSent++;
		break;
	case 3:
		printf ("\n Sending TA_No_Segments_Profile");
		misuSendNoSegsProfile (mi_conn);
		lastSent++;
		break;
	case 4:
		printf ("\n Sending Device Outage New");
		misuSendDevOutNew (mi_conn);
		lastSent++;
		break;
	case 5:
		printf ("\n Sending Device Outage Revised");
		misuSendDevOutRevised (mi_conn);
		lastSent++;
		break;
	case 6:
		printf ("\n Sending Device Outage Cancel");
		misuSendDevOutCancel (mi_conn);
		lastSent++;
		break;
	case 7:
		printf ("\n Sending Device Outage Actual");
		misuSendDevOutActual (mi_conn);
		lastSent++;
		break;
	case 8:
		printf ("\n Sending Available");
		misuSendAvailable (mi_conn);
		lastSent++;
		break;
	case 9:
		printf ("\n Sending UnAvailable");
		misuSendUnavailable (mi_conn);
		lastSent++;
		break;
	case 10:
		printf ("\n Sending Status_Available");
		misuSendStatusAvail (mi_conn);
		lastSent++;
		break;
	case 11:
		printf ("\n Sending Status_Unavailable");
		misuSendStatusUnavail (mi_conn);
		lastSent++;
		break;
	case 12:
		printf ("\n Sending Forecast");
		misuSendForecast (mi_conn);
		lastSent++;
		break;
	case 13:
		printf ("\n Sending Curve");
		misuSendCurve (mi_conn);
		lastSent++;
		break;
	case 14:
		printf ("\n Sending General_Data_Report");
		misuSendGenData (mi_conn);
		lastSent++;
		break;
	}

	if (lastSent > 14)
		lastSent = 0;
}
#endif	// CHOIBC DELETE

