#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define	LOG_BUF_LEN		2048

typedef enum
{
	CONTROL_DIR_REQ,
	CONTROL_DIR_RESP,
	CONTROL_DIR_NONE
}	CONTROL_DIR_ENUM;  // Control Direction for Log

/*
*	proto-type definition for UserUtil.c
*/

char *LogFile_Control (char *out, int outSize,
			struct timeval *pTime, char *pIccpName, byte devNo, word devPt,
			CONTROL_DIR_ENUM eDir, char *pValue, char *pMsg);

#ifdef __cplusplus
}
#endif


