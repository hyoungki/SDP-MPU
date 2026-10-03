//==============================================================================
//
// 파일:    cliMain.c
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
/*  TTMS THREAD Start Routine ....             */
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
			//usleep(1000);
			printf("%s", console->logData[rear]);
			
			/* ------------------------------------------------------- */
            /*  CONSOLE 메세지 ... LOG 저장 */
            /* ------------------------------------------------------- */
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
