//==============================================================================
//
// File:	detExec.c
//
//==============================================================================

//
// Include File
//
#include    "cli.h"

extern  int GetToken(void);

//
// External Variables
//
extern  RTC         *rtc;
extern  OPR_MSG     *opr;

extern  CMD_ENTRY	cmdTable[MAX_COMMAND];
extern  TTY_DESC    consolePort;

//
// Global Variables
//
char	command[CMD_BUF_LEN], *cmdPtr;
char	token[TOKEN_LENGTH];

//
// 모듈:	GetWord()
//
WORD GetWord1(char *ptr)
{
	WORD	value;

	value = (BYTE)*ptr + (BYTE)*(ptr+1) * 256;
	return(value);
}

//
// 모듈:	PrnLine()
//
void PrnLine(char mode)
{
	char	buffer[84];

	memset(buffer, mode, 80);
	buffer[80] = 0;
	printf("%s\r\n", buffer);
}

//
// 모듈:	PrnSubTitle()
//
void PrnSubTitle(char *title)
{
	int		idx;
	char	*ptr;

	printf("%s\r\n", title);
	for (idx = 0, ptr = title; idx < strlen(title); idx++, ptr++)
		printf("%c", *ptr != ' ' ? '-' : *ptr);
	printf("\r\n");
}

//
//
// 모듈:	CheckEOT()
//
int CheckEOT(void)
{
	if (GetToken() > 0)
	{
		printf("%%%%ERR_입력 %s 이상\n", token);
		return(-1);
	}
	return(0);
}

//
// Module:	GetToken()
//
int GetToken(void)
{
	int		index, byteCount;

	memset(token, 0, TOKEN_LENGTH);
	for (index = 0, byteCount = 0;; index++, cmdPtr++)
	{
		switch (*cmdPtr)
		{
		case ',':
		case ' ':
		case ':':
		case ';':
		case 0x0D:
		case 0x0A:
			if (byteCount == 0)
				break;
			else
			{
				token[byteCount] = 0;
				return(byteCount);
			}
			break;
		case 0x00:
			if (byteCount == 0)
				return(0);
			else
			{
				token[byteCount] = 0;
				return(byteCount);
			}
			break;
		default:
			if (byteCount <= TOKEN_LENGTH - 1)
			{
				token[byteCount] = *cmdPtr;	
				byteCount++;
			}
			break;
		}
	}	
}

//
// Module:	GetNumber()
//
int GetNumber(int low, int high, int *value, char *message)
{
	int		index, length;
 
	if ((length = GetToken()) <= 0)
	{
		printf("%%%%ERR_Need %s Input\n", message);
		return(E_ERR);
	}

	for (index = 0; index < length; index++)
	{
		if (token[index] == 0)
			break;
		if (isdigit(token[index]) == 0)
		{
			printf("%%%%ERR_Invalud Character %c input\n", token[index]);
			return(E_ERR);
		}
	}
 
	if ((*value = atoi(token)) < 0)
	{
		printf("%%%%ERR_Input must be in %d to %d\n", low, high);
		return(E_ERR);
	}
 
	if (*value < low || *value > high)
	{
		printf("%%%%ERR_Input must be in %d to %d\n", low, high);
		return(E_ERR);
	}
	return(E_OK);
}

//
// Module:	GetNumber()
//
int GetNumber2(int low, int high, int *value1, int *value2, char *message)
{
	int		index, length;
 
    /* ---------------------------- */
    /* INPUT#1 에 대한 입력 처리    */
    /* ---------------------------- */
	if ((length = GetToken()) <= 0)
	{
		printf("%%%%ERR_Need %s Input\n", message);
		return(E_ERR);
	}

	for (index = 0; index < length; index++)
	{
		if (token[index] == 0)
			break;
		if (isdigit(token[index]) == 0)
		{
			printf("%%%%ERR_Invalud Character %c input\n", token[index]);
			return(E_ERR);
		}
	}
 
	if ((*value1 = atoi(token)) < 0)
	{
		printf("%%%%ERR_Input must be in %d to %d\n", low, high);
		return(E_ERR);
	}
 
	if (*value1 < low || *value1 > high)
	{
		printf("%%%%ERR_Input must be in %d to %d\n", low, high);
		return(E_ERR);
	}
	
    /* ---------------------------- */
    /* INPUT#2 에 대한 입력 처리    */
    /* ---------------------------- */
	if ((length = GetToken()) <= 0)
	{
		printf("%%%%ERR_Need %s Input\n", message);
		return(E_ERR);
	}

	for (index = 0; index < length; index++)
	{
		if (token[index] == 0)
			break;
		if (isdigit(token[index]) == 0)
		{
			printf("%%%%ERR_Invalud Character %c input\n", token[index]);
			return(E_ERR);
		}
	}
 
	if ((*value2 = atoi(token)) < 0)
	{
		printf("%%%%ERR_Input must be in %d to %d\n", low, high);
		return(E_ERR);
	}
 
	if (*value2 < low || *value2 > high)
	{
		printf("%%%%ERR_Input must be in %d to %d\n", low, high);
		return(E_ERR);
	}
	
	return(E_OK);

}

//
// Module:	InputProcessing()
//
void InputProcessing(void)
{
    int     ch;
	int		index, fIndex, tokenLen, length;
	CMD_ENTRY	*cp;

    bzero(command, CMD_BUF_LEN);
    
    if(opr->runMode == LOCAL_MASTER)    printf("[M-%02d:%02d:%02d] ", rtc->hour, rtc->min, rtc->sec); 
    else                                printf("[S-%02d:%02d:%02d] ", rtc->hour, rtc->min, rtc->sec); 
	
	index = 0;
	while(1)
	{
	    ch = getchar();
	    command[index++] = ch;
	    if(ch == 0x0a)  break;
	}
	
	cmdPtr = command;
	if ((tokenLen = GetToken()) <= 0)   return;

    printf("(1)cmd: %s\n", command);
    
	cp = cmdTable;
	for (index = 0, fIndex = -1; index < MAX_COMMAND; index++, cp++)
	{
		if (strlen(cp->name) == 0)
			break;
		length = strlen(cp->name) > tokenLen ? strlen(cp->name) : tokenLen;
		if (strncmp(cp->name, token, length) == 0)
		{
			fIndex = index;
			break;
		}
	}
	if (fIndex == -1)
	{
		printf("%%%%ERR_Invalid Command %s Input\n", token);
		return;
	}
	
	(*cp->function)();
}
