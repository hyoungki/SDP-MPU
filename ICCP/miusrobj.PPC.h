/*  This file created from input file '../../iccp/usr/miusrobj.odf'
    Leaf Access Parameter (LAP) File: 'Not Used'
	Created Tue Apr 26 15:36:43 2016
*/

#ifndef FOUNDRY_OUTPUT_HEADER_INCLUDED
#define FOUNDRY_OUTPUT_HEADER_INCLUDED

#include "mvl_uca.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/* MVL Type Control Information */
extern MVL_TYPE_CTRL *mvl_type_ctrl;
extern ST_INT mvl_num_types;

/************************************************************************/


/* MVL Type ID's */
#define Integer16_TYPEID		0
#define Visible_String_32_TYPEID		1
#define MMSObjectName_TYPEID		2
#define GMTBasedS_TYPEID		3
#define DSConditions_TYPEID		4
#define TAConditions_TYPEID		5
#define DSTransferSet1996_TYPEID		6
#define DSTransferSet2000_TYPEID		7
#define IMTransferSet_TYPEID		8
#define TATransferSet_TYPEID		9
#define SupportedFeatures_TYPEID		10
#define TASE2Version_TYPEID		11
#define Data_Real_TYPEID		12
#define Data_State_TYPEID		13
#define Data_Discrete_TYPEID		14
#define Data_RealQ_TYPEID		15
#define Data_StateQ_TYPEID		16
#define Data_DiscreteQ_TYPEID		17
#define Data_RealQTimeTag_TYPEID		18
#define Data_StateQTimeTag_TYPEID		19
#define Data_DiscreteQTimeTag_TYPEID		20
#define Data_RealExtended_TYPEID		21
#define Data_StateExtended_TYPEID		22
#define Data_DiscreteExtended_TYPEID		23
#define Data_RealQTimeTagExtended_TYPEID		24
#define Data_StateQTimeTagExtended_TYPEID		25
#define Data_DiscreteQTimeTagExtended_TYPEID		26
#define Data_StateSupplemental_TYPEID		27
#define Data_StateSupplementalQ_TYPEID		28
#define Data_StateSupplementalQTimeTag_TYPEID		29
#define Data_StateSupplementalExtended_TYPEID		30
#define Data_StateSupplementalQTimeTagExtended_TYPEID		31
#define Control_Command_TYPEID		32
#define Control_Setpoint_Real_TYPEID		33
#define Control_Setpoint_Discrete_TYPEID		34
#define SBO_CheckBackName_TYPEID		35
#define TagValue_TYPEID		36
#define InfoMessHeader_TYPEID		37
#define InfoBuffXXX_TYPEID		38
#define TAAccountRequest_TYPEID		39


/************************************************************************/

/************************************************************************/


/*	Common Strings Index Defines	*/

#ifndef USE_RT_TYPE_2

#define FO_SCOPE_STRING_INDEX	 1
#define FO_DOMAINNAME_STRING_INDEX	 2
#define FO_NAME_STRING_INDEX	 3
#define FO_DATASETNAME_STRING_INDEX	 4
#define FO_STARTTIME_STRING_INDEX	 5
#define FO_INTERVAL_STRING_INDEX	 6
#define FO_TLE_STRING_INDEX	 7
#define FO_BUFFERTIME_STRING_INDEX	 8
#define FO_INTEGRITYCHECK_STRING_INDEX	 9
#define FO_DSCONDITIONSREQUESTED_STRING_INDEX	 10
#define FO_BLOCKDATA_STRING_INDEX	 11
#define FO_CRITICAL_STRING_INDEX	 12
#define FO_RBE_STRING_INDEX	 13
#define FO_STATUS_STRING_INDEX	 14
#define FO_EVENTCODEREQUESTED_STRING_INDEX	 15
#define FO_ALLCHANGESREPORTED_STRING_INDEX	 16
#define FO_TACONDITIONSREQUESTED_STRING_INDEX	 17
#define FO_MAJORVERSIONNUMBER_STRING_INDEX	 18
#define FO_MINORVERSIONNUMBER_STRING_INDEX	 19
#define FO_VALUE_STRING_INDEX	 20
#define FO_FLAGS_STRING_INDEX	 21
#define FO_TIMESTAMP_STRING_INDEX	 22
#define FO_COV_STRING_INDEX	 23
#define FO_GMTBASEDS_STRING_INDEX	 24
#define FO_MILLISECONDS_STRING_INDEX	 25
#define FO_REASON_STRING_INDEX	 26
#define FO_INFOREFERENCE_STRING_INDEX	 27
#define FO_LOCALREFERENCE_STRING_INDEX	 28
#define FO_MESSAGEID_STRING_INDEX	 29
#define FO_SIZE_STRING_INDEX	 30
#define FO_TRANSFERACCOUNTREFERENCE_STRING_INDEX	 31
#define FO_DURATION_STRING_INDEX	 32
#define FO_REQUESTID_STRING_INDEX	 33
#define FO_TACONDITIONSREQUESTED_STRING_INDEX	 34
#endif


/************************************************************************/
/* TYPEDEFS for MMS TYPES	                                              */
/************************************************************************/

/* Use ":CF", "-notypedefs" in the ODF file to not include this line */
#define USE_MMS_TYPEDEFS

#if defined (USE_MMS_TYPEDEFS)

typedef ST_INT16 Integer16_TDEF;

typedef ST_CHAR Visible_String_32_TDEF[33];  /* Visible String */

typedef struct
  {
  ST_UINT8 Scope;
  ST_CHAR DomainName[65];
  ST_CHAR Name[65];
  } MMSObjectName_TDEF;

typedef ST_INT32 GMTBasedS_TDEF;

typedef ST_UCHAR DSConditions_TDEF[1];  /* Bitstring */

typedef ST_UCHAR TAConditions_TDEF[1];  /* Bitstring */

typedef struct
  {
  struct
    {
    ST_UINT8 Scope;
    ST_CHAR DomainName[65];
    ST_CHAR Name[65];
    } DataSetName;
  ST_INT32 StartTime;
  ST_INT16 Interval;
  ST_INT16 TLE;
  ST_INT16 BufferTime;
  ST_INT16 IntegrityCheck;
  ST_UCHAR DSConditionsRequested[1];
  ST_BOOLEAN BlockData;
  ST_BOOLEAN Critical;
  ST_BOOLEAN RBE;
  ST_BOOLEAN Status;
  ST_INT16 EventCodeRequested;
  } DSTransferSet1996_TDEF;

typedef struct
  {
  struct
    {
    ST_UINT8 Scope;
    ST_CHAR DomainName[65];
    ST_CHAR Name[65];
    } DataSetName;
  ST_INT32 StartTime;
  ST_INT16 Interval;
  ST_INT16 TLE;
  ST_INT16 BufferTime;
  ST_INT16 IntegrityCheck;
  ST_UCHAR DSConditionsRequested[1];
  ST_BOOLEAN BlockData;
  ST_BOOLEAN Critical;
  ST_BOOLEAN RBE;
  ST_BOOLEAN AllChangesReported;
  ST_BOOLEAN Status;
  ST_INT16 EventCodeRequested;
  } DSTransferSet2000_TDEF;

typedef ST_BOOLEAN IMTransferSet_TDEF;

typedef struct
  {
  ST_UCHAR TAConditionsRequested[1];
  ST_BOOLEAN BlockData;
  ST_BOOLEAN Status;
  } TATransferSet_TDEF;

typedef ST_UCHAR SupportedFeatures_TDEF[2];  /* Bitstring */

typedef struct
  {
  ST_INT16 MajorVersionNumber;
  ST_INT16 MinorVersionNumber;
  } TASE2Version_TDEF;

typedef ST_FLOAT Data_Real_TDEF;

typedef ST_UCHAR Data_State_TDEF[1];  /* Bitstring */

typedef ST_INT32 Data_Discrete_TDEF;

typedef struct
  {
  ST_FLOAT Value;
  ST_UCHAR Flags[1];
  } Data_RealQ_TDEF;

typedef ST_UCHAR Data_StateQ_TDEF[1];  /* Bitstring */

typedef struct
  {
  ST_INT32 Value;
  ST_UCHAR Flags[1];
  } Data_DiscreteQ_TDEF;

typedef struct
  {
  ST_FLOAT Value;
  ST_INT32 TimeStamp;
  ST_UCHAR Flags[1];
  } Data_RealQTimeTag_TDEF;

typedef struct
  {
  ST_INT32 TimeStamp;
  ST_UCHAR Flags[1];
  } Data_StateQTimeTag_TDEF;

typedef struct
  {
  ST_INT32 Value;
  ST_INT32 TimeStamp;
  ST_UCHAR Flags[1];
  } Data_DiscreteQTimeTag_TDEF;

typedef struct
  {
  ST_FLOAT Value;
  ST_INT32 TimeStamp;
  ST_UCHAR Flags[1];
  ST_UINT16 COV;
  } Data_RealExtended_TDEF;

typedef struct
  {
  ST_INT32 TimeStamp;
  ST_UCHAR Flags[1];
  ST_UINT16 COV;
  } Data_StateExtended_TDEF;

typedef struct
  {
  ST_INT32 Value;
  ST_INT32 TimeStamp;
  ST_UCHAR Flags[1];
  ST_UINT16 COV;
  } Data_DiscreteExtended_TDEF;

typedef struct
  {
  ST_FLOAT Value;
  struct
    {
    ST_INT32 GMTBasedS;
    ST_INT16 Milliseconds;
    } TimeStamp;
  ST_UCHAR Flags[1];
  } Data_RealQTimeTagExtended_TDEF;

typedef struct
  {
  struct
    {
    ST_INT32 GMTBasedS;
    ST_INT16 Milliseconds;
    } TimeStamp;
  ST_UCHAR Flags[1];
  } Data_StateQTimeTagExtended_TDEF;

typedef struct
  {
  ST_INT32 Value;
  struct
    {
    ST_INT32 GMTBasedS;
    ST_INT16 Milliseconds;
    } TimeStamp;
  ST_UCHAR Flags[1];
  } Data_DiscreteQTimeTagExtended_TDEF;

typedef ST_UCHAR Data_StateSupplemental_TDEF[1];  /* Bitstring */

typedef struct
  {
  ST_UCHAR Value[1];
  ST_UCHAR Flags[1];
  } Data_StateSupplementalQ_TDEF;

typedef struct
  {
  ST_UCHAR Value[1];
  ST_INT32 TimeStamp;
  ST_UCHAR Flags[1];
  } Data_StateSupplementalQTimeTag_TDEF;

typedef struct
  {
  ST_UCHAR Value[1];
  ST_INT32 TimeStamp;
  ST_UCHAR Flags[1];
  ST_UINT16 COV;
  } Data_StateSupplementalExtended_TDEF;

typedef struct
  {
  ST_UCHAR Value[1];
  struct
    {
    ST_INT32 GMTBasedS;
    ST_INT16 Milliseconds;
    } TimeStamp;
  ST_UCHAR Flags[1];
  } Data_StateSupplementalQTimeTagExtended_TDEF;

typedef ST_INT16 Control_Command_TDEF;

typedef ST_FLOAT Control_Setpoint_Real_TDEF;

typedef ST_INT16 Control_Setpoint_Discrete_TDEF;

typedef ST_INT16 SBO_CheckBackName_TDEF;

typedef struct
  {
  ST_UCHAR Flags[1];
  ST_CHAR Reason[256];
  } TagValue_TDEF;

typedef struct
  {
  ST_INT32 InfoReference;
  ST_INT32 LocalReference;
  ST_INT32 MessageId;
  ST_INT32 Size;
  } InfoMessHeader_TDEF;

typedef ST_UCHAR InfoBuffXXX_TDEF[128];  /* Octetstring */

typedef struct
  {
  ST_INT32 TransferAccountReference;
  ST_INT32 StartTime;
  ST_INT32 Duration;
  ST_INT32 RequestId;
  ST_UCHAR TaConditionsRequested[1];
  } TAAccountRequest_TDEF;

/************************************************************************/
#endif /* if defined (USE_MMS_TYPEDEFS) */
/************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* #ifndef FOUNDRY_OUTPUT_HEADER_INCLUDED */
