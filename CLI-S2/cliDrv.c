//==============================================================================
//
// File:	cliDrv.c
//
// CLI-S2 : design/cli-argv-command-dispatch.md 설계의 핵심부.
//   readline() -> run_command(line) -> [parse_line + find_cmd + do_xxx(argc,argv)]
//
// GetToken()/GetNumber()/CheckEOT()는 CLI-ReadLine 원본과 동일하게 남겨두었다.
// 아직 do_xxx(argc,argv) 인자로 완전히 옮기지 않은(getDecWord() 기반 다단계
// 프롬프트) 명령들이 여전히 이 함수들을 쓰기 때문이다. run_command()가 매
// 호출마다 cmdPtr을 "명령어 이름 다음 토큰"으로 맞춰주는 레거시 브리지를
// 두어, 그 함수들의 본문은 손대지 않고도 그대로 동작한다.
//
//==============================================================================

//
// Include File
//
#include    "cli.h"




#include <readline/readline.h>
#include <readline/history.h>


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
// 모듈:	CheckEOT()
//
// 레거시 브리지 전용 - getDecWord() 기반 다단계 명령이 계속 쓴다.
// 새로 옮긴 do_xxx(argc, argv) 명령은 이 대신 argc 개수를 직접 검사한다.
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
// 모듈:	GetToken()
//
// cmdPtr이 가리키는 위치부터 토큰 하나를 잘라 token[]에 담고 cmdPtr을 전진시킨다.
// 레거시 브리지(run_command가 세팅하는 cmdPtr) 전용 - CheckEOT()/GetNumber()가 쓴다.
//
int GetToken(void)
{
	int		index;
	int     byteCount;

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
// 모듈:	GetNumber()
//
// 레거시 브리지 전용(cmdPtr 기반). 새 do_xxx(argc,argv) 명령은 ParseNumber()를 쓴다.
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
// 모듈:	GetNumber2()
//
int GetNumber2(int low, int high, int *value1, int *value2, char *message)
{
	int		index, length;

	if ((length = GetToken()) <= 0)
	{
		printf("GetNumber2> [ERROR] Need %s Input\n", message);
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
// 모듈:	ParseNumber()
//
// GetNumber()와 동일한 검증을 하되, 전역 커서(cmdPtr) 대신 argv[i] 문자열
// 하나를 직접 받는다. 전역 상태가 없어 do_xxx(argc, argv) 명령에서 바로 쓸 수 있다.
//
int ParseNumber(const char *arg, int low, int high, int *value, const char *message)
{
	int		i;

	if (arg == NULL)
	{
		printf("%%%%ERR_Need %s Input\n", message);
		return(E_ERR);
	}

	for (i = 0; arg[i]; i++)
	{
		if (isdigit((unsigned char) arg[i]) == 0)
		{
			printf("%%%%ERR_Invalud Character %c input\n", arg[i]);
			return(E_ERR);
		}
	}

	*value = atoi(arg);

	if (*value < low || *value > high)
	{
		printf("%%%%ERR_Input must be in %d to %d\n", low, high);
		return(E_ERR);
	}
	return(E_OK);
}

//
// 모듈:	parse_line()
//
// line을 in-place로 토큰화해서 argv[]가 그 안을 가리키게 한다 (malloc 없음).
// 구분자는 기존 GetToken()과 동일하게 공백/','/'':'/';'까지 포함한다
// (user.c의 parse_line()은 공백/탭만 구분자로 봤지만, 콤마 등으로 입력하던
//  기존 CLI-ReadLine 사용 습관을 깨지 않기 위해 여기서는 기존 구분자를 유지).
//
int parse_line(char *line, char *argv[])
{
	int argc = 0;

	while (argc < MAX_ARGS)
	{
		while (*line == ' ' || *line == ',' || *line == ':' || *line == ';')
			line++;

		if (*line == 0)
		{
			argv[argc] = NULL;
			return(argc);
		}

		argv[argc++] = line;

		while (*line && *line != ' ' && *line != ',' && *line != ':' && *line != ';')
			line++;

		if (*line == 0)
		{
			argv[argc] = NULL;
			return(argc);
		}
		*line++ = 0;
	}

	printf("%%%%ERR_Too many args (max=%d)\n", MAX_ARGS);
	argv[argc] = NULL;
	return(argc);
}

//
// 모듈:	find_cmd()
//
// cmdTable에서 name과 일치하는 항목을 찾는다. CLI-ReadLine 기존 동작대로
// 접두사(prefix) 매칭을 유지한다 ("he" 입력만으로 "help" 인식 등).
//
int find_cmd(char *name)
{
	int         index;
	CMD_ENTRY   *cp;
	int         nameLen = strlen(name);

	for (index = 0, cp = cmdTable; index < MAX_COMMAND; index++, cp++)
	{
		if (strlen(cp->name) == 0)
			break;
		if (strncmp(cp->name, name, nameLen) == 0)
			return(index);
	}
	return(-1);
}

//
// 모듈:	run_command()
//
// 원본 줄 문자열 하나를 받아 파싱부터 디스패치까지 전부 처리한다
// (user.c의 run_command(char *line) 계층 구조를 그대로 이식).
//
int run_command(char *line)
{
	static char scratch[CMD_BUF_LEN];
	char        *argv[MAX_ARGS + 1];
	int         argc, index;

	if (strlen(line) >= CMD_BUF_LEN)
	{
		printf("CLI> [ERROR] Command too long\n");
		return(E_ERR);
	}

	strncpy(scratch, line, CMD_BUF_LEN - 1);
	scratch[CMD_BUF_LEN - 1] = 0;

	argc = parse_line(scratch, argv);
	if (argc == 0)
		return(E_OK);				// 빈 줄은 조용히 무시 (기존 동작과 동일)

	if ((index = find_cmd(argv[0])) < 0)
	{
		printf("CLI> [ERROR] Invalid Command[%s] Input\n", argv[0]);
		return(E_ERR);
	}

	/* 레거시 브리지: line(=command 전역)은 parse_line()이 건드리지 않은
	   원본이므로, 아직 getDecWord() 기반인 명령이 내부에서 CheckEOT()/
	   GetToken()/GetNumber()를 불러도 명령어 이름 다음 토큰부터 정확히
	   읽는다. */
	cmdPtr = line;
	GetToken();

	return(cmdTable[index].function(argc, argv));
}


/* readline completion */
/* Generator function for command completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. */
char *
command_generator (text, state)
     const char *text;
     int state;
{
  static int list_index, len;

  if (!state)
    {
      list_index = 0;
      len = strlen (text);
    }

  while ( strlen(cmdTable[list_index].name) !=0  )
    {
      list_index++;

      if (strncmp (( char *)cmdTable[list_index].name, text, len) == 0)
      {
        return (strdup(   (char *)cmdTable[list_index].name    ));
       }
    }

  return ((char *)NULL);
}


/* Attempt to complete on the contents of TEXT.  START and END bound the
   region of rl_line_buffer that contains the word to complete.  TEXT is
   the word to complete.  We can use the entire contents of rl_line_buffer
   in case we want to do some simple parsing.  Return the array of matches,
   or NULL if there aren't any. */
char **
fileman_completion (text, start, end)
     const char *text;
     int start, end;
{
  char **matches;

  matches = (char **)NULL;

  if (start == 0)
    matches = rl_completion_matches (text, command_generator);

  return (matches);
}


//
// 모듈:	InputProcessing()
//

/* Tell the GNU Readline library how to complete.  We want to try to complete
   on command names if this is the first word in the line, or on filenames
   if not. */
void initialize_readline (void)
{
  rl_readline_name = "FileMan";
  rl_attempted_completion_function = fileman_completion;
}


static RTC             local_rtc;

int	readClock()
{
    time_t  tm;
    struct  tm  localtm;

    time(&tm);
    localtime_r (&tm, &localtm);

	local_rtc.year 	= localtm.tm_year + 1900;
	local_rtc.month 	= localtm.tm_mon + 1;
	local_rtc.day 	= localtm.tm_mday;
	local_rtc.week 	= localtm.tm_wday;
	local_rtc.hour 	= localtm.tm_hour;
	local_rtc.min 	= localtm.tm_min;
	local_rtc.sec 	= localtm.tm_sec;

    return(0);
}


//
// 모듈:	InputProcessing()
//
// readline()으로 한 줄을 받아 run_command()에 그대로 넘긴다. 토큰화/명령
// 검색/호출은 전부 run_command() 내부 책임이다 (예전엔 여기서 첫 토큰만
// 잘라 함수 포인터를 인자 없이 호출했었다).
//
void InputProcessing(void)
{
	char *line;

	bzero(command, CMD_BUF_LEN);

	readClock();

    if(opr->runMode == LOCAL_MASTER)    printf("[M-%02d:%02d:%02d] ", local_rtc.hour, local_rtc.min, local_rtc.sec);
    else                                printf("[S-%02d:%02d:%02d] ", local_rtc.hour, local_rtc.min, local_rtc.sec);

    line = readline (": ");
    if (line == NULL)
        return;

    strncpy(command, line, CMD_BUF_LEN - 1);
    command[CMD_BUF_LEN - 1] = 0;
    add_history (line);
    free (line);

    run_command(command);
}
