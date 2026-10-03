
/***********************************************************************
 *
 *
 ***********************************************************************/
#include	"localLib.h"

extern  MPC860IO_DESC    ledPort;
extern  MPC860IO_DESC    rtcPort;

extern  RTC             *rtc;
extern  OPR_MSG         *opr;
extern  CONSOLE_INFO	*console;

extern  SDP_DEVICE      *deviceCFG[MAX_DEVICE];         // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)
extern  ESIO_CONFIG     *esioCFG[MAX_ESIO];             // ESIO 장치 Config
extern  LINK_MSG        *linkCfg;                       // CPU 이중화 구조체;

//int rtc_get_time(struct tm *tmval);
//int rtc_set_time(struct tm *tmval, int century);


/*
*   RTC-DS-1340 에서 시각정보 Read
*/
void read_RTC1340()
{
    struct rtc_time rtc_tm;

	if( ioctl  (rtcPort.id, RTC_RD_TIME,  &rtc_tm) < 0)
	{
	    Debug(console,"*ioctl RTC_RD_TIME  %s\n",strerror(errno)) ;
    }
    
    rtc->year 	= rtc_tm.tm_year + 1900;
	rtc->month 	= rtc_tm.tm_mon + 1;
	rtc->day 	= rtc_tm.tm_mday;
	rtc->week 	= rtc_tm.tm_wday;
	rtc->hour 	= rtc_tm.tm_hour;
	rtc->min 	= rtc_tm.tm_min;
	rtc->sec 	= rtc_tm.tm_sec;	   
	
	printf ("read-1340 : %04d/%02d/%02d/ week=%d %02d:%02d:%02d\n", rtc->year, rtc->month, rtc->day, rtc->week, rtc->hour, rtc->min, rtc->sec);
	     
}


/*
*/
int	readClock()
{
    time_t  tm;
    struct  tm  localtm;

    time(&tm);
    //localtm = localtime(&tm);
    localtime_r (&tm, &localtm);
    

	rtc->year 	= localtm.tm_year + 1900;
	rtc->month 	= localtm.tm_mon + 1;
	rtc->day 	= localtm.tm_mday;
	rtc->week 	= localtm.tm_wday;
	rtc->hour 	= localtm.tm_hour;
	rtc->min 	= localtm.tm_min;
	rtc->sec 	= localtm.tm_sec;

    return(0);                
}


/*
writeClock 아.  현재으로 update....
*   RTC-DS-1340 에서 시각정보 Write
*   - year : 
*/
int	rtc_TimeUpdate()
{
    int     retVal;
    time_t  tm;
    struct  tm  localtm;

    time(&tm);
    //localtm = localtime(&tm);
    localtime_r (&tm, &localtm);
    
    // RTC Write ... 
    //if(opr->wdtDebug)
	printf("[RTC-Update] %04d:%02d:%02d-%02d-%02d:%02d:%02d\n", 
	    localtm.tm_year + 1900, localtm.tm_mon + 1, localtm.tm_mday, localtm.tm_wday,localtm.tm_hour, localtm.tm_min, localtm.tm_sec);  

	retVal = ioctl(rtcPort.id, RTC_SET_TIME, &localtm);
	
	if (retVal == -1) 
	{
    	Debug(console,"wdt> *ioctl RTC_SET_TIME  %s\n",strerror(errno)) ;
    	return (-1);
    }
    
    return (0);
}


/*
*   RTC-DS-1340 에서 시각정보 Write
*   - year : 
*/
int	writeClock( int year, int month, int day, int hour, int min, int sec, int week)
{
    int     i;
    
    time_t	systime;
	struct	tm tmval ;  
    struct	timeval tv;  
	struct	timezone tz; 
    int		retVal;
	
	SDP_DEVICE  *dev;
	ESIO_CONFIG *esio;
	
	/* ------------------------------------ */
	/* [1]. RTC SYSTEM Clock Setting ....   */
	/* ------------------------------------ */
	//year = year % 100;
	tmval.tm_sec  = sec;
	tmval.tm_min  = min;
	tmval.tm_hour = hour;
	tmval.tm_mday = day;
	tmval.tm_mon  = month - 1;
	//tmval.tm_year = year + 100;
	tmval.tm_year = year - 1900;
	
	tmval.tm_wday = week;
	tmval.tm_yday = 0;
	tmval.tm_isdst= 0;

#if 1
    // RTC Write ... 
    //if(opr->wdtDebug)
	printf("[SET-RTC] %04d:%02d:%02d-%02d-%02d:%02d:%02d\n", year, month, day, week, hour, min, sec);


	retVal = ioctl(rtcPort.id, RTC_SET_TIME, &tmval);
	
	if (retVal == -1) 
	{
    	Debug(console,"WDT-ERR> *ioctl RTC_SET_TIME  %s\n",strerror(errno)) ;
    }
#endif
    
    /* ------------------------------------ */
	/* [2]. LINUX SYSTEM Clock Setting .... */
	/* ------------------------------------ */
    systime = mktime(&tmval);
    
	//if(opr->wdtDebug)
    // FYT 2026-08-27 오후 4:50:03
    Debug(console,"wdt> rtc time %s\n",ctime( &systime)) ;
    
	tv.tv_sec = systime;
	tv.tv_usec = 0;
	
	tz.tz_minuteswest = 0;
	tz.tz_dsttime = 0;

#if 0 // 2026-08-27 오후 4:55:56
	if (settimeofday (&tv, &tz) != 0) 
#else 
	if (settimeofday (&tv,NULL) != 0) 
#endif 
	{
	    //if(opr->wdtDebug)
		//printf ("wdt> *unable to set time -- probably you are not root\n");
    	Debug(console,"WDT-ERR> settimeofday  %s\n",strerror(errno)) ;		
		return (0);
	}


    readClock();

    /* ------------------------------------ */
	/* [3]. Time 변경후  모듈 초기화  ....  */
	/* ------------------------------------ */
    for(i = 0; i < MAX_DEVICE; i++)
    {
        dev = (SDP_DEVICE *) deviceCFG[i];

        dev->timeSyncReq = SET;
        dev->statusDump  = SET;
    }

    for(i = 0; i < MAX_ESIO; i++)
    {
        esio = (ESIO_CONFIG *) esioCFG[i];

        esio->timeSyncReq = SET;
    }

    /* ------------------------------------ */
    /*  CPU 이중화 고려...                  */
    /* ------------------------------------ */
    if(opr->dualCpuSts == SET)
    {
    	if(opr->runMode == LOCAL_MASTER)		
    	linkCfg->timeSyncReq  = SET;
	}
	
    return (0);
}



