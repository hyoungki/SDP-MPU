//==============================================================================
//
// File:    cliDebug.c
//
// CLI-S2 : CLI-ReadLine의 debug_thread()를 그대로 유지. 명령 디스패치와는
//          무관한 NETWORK_CONSOLE 로그 출력 스레드다.
//
//==============================================================================
#include	"localLib.h"
#include    "cli.h"

extern	int	 termExec;

extern	RTC 		*rtc;
extern	OPR_MSG     *opr;
extern	CONSOLE_INFO	*console;

extern	THREAD_ENTRY    debugTHREAD;

/* ------------------------------------------------------- */
/*  NETWORK_CONSOLE 로그 출력 THREAD                        */
/* ------------------------------------------------------- */
void    *debug_thread(int  *arg)
{
    short   length;
	word	front, rear;

	printf(" %s DEBUG-MAIN PROCESS Activated ... !\n", TARGET_NAME);

	console->front = 0;
	console->rear  = 0;
	console->active = SET;

	while(termExec)
	{
		pause(10);

		front = console->front & LOG_MASK_NUM;
		rear  = console->rear & LOG_MASK_NUM;

		while(front != rear)
		{
			printf("%s", console->logData[rear]);

			if(console->logFileFlag == SET)
            {
                length = strlen((char *) console->logData[rear]);
                LogFile_CONSOLE((char *) console->logData[rear], length);
            }

			console->rear = (rear + 1) & LOG_MASK_NUM;

			front = console->front & LOG_MASK_NUM;
			rear  = console->rear & LOG_MASK_NUM;
		}

	}

	console->active = RESET;

	return 0;
}
