//==============================================================================
//
// File:    cli.h
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

//
// Type Declaration(s)
//
typedef struct
{
	char	name[TOKEN_LENGTH];
	void	(*function)();
	char	usage[USAGE_LENGTH];
	char	help[USAGE_LENGTH];
} CMD_ENTRY;


#endif

//
// End of cli.h
//
