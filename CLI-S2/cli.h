//==============================================================================
//
// File:    cli.h
//
// CLI-S2 : design/cli-argv-command-dispatch.md 설계를 반영한 CLI-ReadLine 포팅본.
//          CMD_ENTRY.function 이 void(*)()에서 int(*)(int argc, char *argv[])로
//          바뀐 것이 CLI-ReadLine과의 핵심 차이.
//
//==============================================================================

#ifndef CLI_HEADER_INCLUDED

#define CLI_HEADER_INCLUDED

//
// Include File(s)
//
#include    "localLib.h"

#include <ctype.h>

//
// External Function(s)
//
extern void ShmDetach(SHM_DESC *sp);

//
// General Definition(s)
//
#define	MAX_COMMAND		64
#define	CMD_BUF_LEN		256
#define	TOKEN_LENGTH	128
#define	USAGE_LENGTH	256
#define	MAX_DEBUG_LEVEL	9

#define MAX_ARGS		16		// 한 줄에서 허용하는 최대 토큰(argv) 개수

//
// Type Declaration(s)
//
typedef struct
{
	char	name[TOKEN_LENGTH];
	int		(*function)(int argc, char *argv[]);
	char	usage[USAGE_LENGTH];
	char	help[USAGE_LENGTH];
} CMD_ENTRY;


#endif

//
// End of cli.h
//
