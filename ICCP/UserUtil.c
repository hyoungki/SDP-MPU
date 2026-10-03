#include <time.h>
#include "mi_usr.h"
#include "UserUtil.h"
#include "iccpShm.h"

#define	CONTROL_LOG_NAME	"control.log"
#define	CONTROL_LOG_SIZE	409600	// 400 kbytes
// logging file이 filesize 보다 크면, 백업을 만든다.

char *LogFile_Control (char *out, int outSize,
			struct timeval *pTime, char *pIccpName, byte devNo, word devPt,
			CONTROL_DIR_ENUM eDir, char *pValue, char *pMsg)
{
	ST_CHAR time[64], dir[32];
	char fname[128], fnameOld[128], *fopen_type;
	struct stat stat_buf;
	FILE *fp;

	sprt_timeval (time, sizeof(time), pTime);
	if (eDir == CONTROL_DIR_REQ) // request
		strcpy (dir, "<--");
	else if (eDir == CONTROL_DIR_RESP) //response
		strcpy (dir, "-->");
	else 
		strcpy (dir, "   "); // 이도 저도 아님

	snprintf (out, outSize-1,
			"%s\t%s\t%-10s\t%2d-%-4d\t%-5s\t%s\n",
			time, dir, pIccpName, devNo, devPt, pValue, pMsg);

	sprintf (fname, "%s/%s", LOG_DIR, CONTROL_LOG_NAME);
	/* check filesize */
	if (stat (fname, &stat_buf) >= 0)
    {
		if (stat_buf.st_size > CONTROL_LOG_SIZE)
		{
			sprintf (fnameOld, "%s.old", fname);
			if (rename (fname, fnameOld) < 0)
			{
				printf ("file : rename (%s --> %s) error, %d : %s\n",
						fname, fnameOld, errno, strerror(errno));
			}
			fopen_type = "w";
		}
		else
			fopen_type = "a+";
	}
	else
	{
        //printf ("file : stat (%s) error, %d : %s\n",
        //        fname, errno, strerror(errno));
		fopen_type = "w";
    }

	if ((fp = fopen (fname, fopen_type)) == NULL)
	{
		printf ("file : file open (%s,%s) error,  %d : %s\n",
				fname, fopen_type, errno, strerror(errno));
		return out;
	}
	fprintf (fp, "%s", out);
	fclose(fp);

	return out;
}
