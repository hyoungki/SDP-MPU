/* for TEST... */


//==============================================================================
//
// File:    cliCmds.c
//
// CLI-S2 : CLI-ReadLine의 Cmd*() 함수들을 design/cli-argv-command-dispatch.md
//   설계대로 int do_xxx(int argc, char *argv[]) 로 옮긴 버전.
//
//   - GetNumber() 한두 개짜리 인자만 쓰던 명령(hd, esio, dev, cal ...)은
//     ParseNumber(argv[i], ...)로 완전히 옮겼다.
//   - getDecWord()로 여러 줄에 걸쳐 대화형으로 값을 받던 명령(setrtc, trip,
//     testdo, analog ...)은 본문을 그대로 두고 시그니처만 바꿨다. 이 함수들이
//     쓰는 CheckEOT()/GetToken()은 cliDrv.c의 run_command()가 매 호출마다
//     cmdPtr을 맞춰주는 레거시 브리지 덕분에 그대로 동작한다.
//   - 인자를 아예 안 쓰던 토글성 명령(wdt, soe, iccp ...)은 CheckEOT() 대신
//     argc == 1 검사로 바꿨다.
//
//==============================================================================

#include	"localLib.h"
#include    "external.h"
#include    "cli.h"

//
// External Variables / Functions
//
extern  int CheckEOT(void);
extern  int GetNumber(int low, int high, int *value, char *message);
extern  int GetToken(void);
extern  int ParseNumber(const char *arg, int low, int high, int *value, const char *message);
extern  void PrnLine(char mode);

extern  int  parseOnOff(const char *s);
extern  int  waitLogbitAck(short seq);
extern  int  postLogbitSet(const char *layer, const char *bit, int value);
extern  void postLogbitClearAll(void);

extern  void dispDevStatus(int devid, int point);
extern  void dispDevAnalog(int devid, int point);
extern  int  dispDevice(int devid);
extern  void dispHostData_DNP(int hostid);
extern  void disp_HarrisPort(int port);

extern  void    display_ICCP_INFO(int pointType, int pointInx);


extern  int	termExec;
extern  char	token[TOKEN_LENGTH];
extern  char	command[CMD_BUF_LEN];
extern  char	*cmdPtr;
extern  SHM_MEMORY	*shmPtr;

//
// Function Prototypes
//
int	do_help(int argc, char *argv[]);
int	do_process(int argc, char *argv[]);
int	do_clear(int argc, char *argv[]);
int	do_setrtc(int argc, char *argv[]);
int	do_event(int argc, char *argv[]);
int	do_off(int argc, char *argv[]);
int	do_comlogon(int argc, char *argv[]);
int	do_check(int argc, char *argv[]);
int	do_quit(int argc, char *argv[]);
int	do_exit(int argc, char *argv[]);

int	do_iccp(int argc, char *argv[]);
int	do_mms(int argc, char *argv[]);
int	do_iccpinfo(int argc, char *argv[]);

int	do_hd(int argc, char *argv[]);
int	do_hex(int argc, char *argv[]);
int	do_soe(int argc, char *argv[]);
int	do_esio(int argc, char *argv[]);
int	do_dev(int argc, char *argv[]);
int	do_msg(int argc, char *argv[]);
int	do_sim(int argc, char *argv[]);
int	do_scu(int argc, char *argv[]);
int	do_link(int argc, char *argv[]);
int	do_wdt(int argc, char *argv[]);
int	do_ld(int argc, char *argv[]);
int	do_mpu(int argc, char *argv[]);
int	do_cal(int argc, char *argv[]);
int	do_ntp(int argc, char *argv[]);
int	do_device(int argc, char *argv[]);
int	do_esiosts(int argc, char *argv[]);

int	do_dnphost(int argc, char *argv[]);

int	do_portcfg(int argc, char *argv[]);
int	do_chksum(int argc, char *argv[]);

int	do_trip(int argc, char *argv[]);
int	do_close(int argc, char *argv[]);
int	do_analog(int argc, char *argv[]);
int	do_status(int argc, char *argv[]);
int	do_online(int argc, char *argv[]);
int	do_change(int argc, char *argv[]);
int	do_testdo(int argc, char *argv[]);
int	do_devtest(int argc, char *argv[]);
int	do_getclock(int argc, char *argv[]);
int	do_testset(int argc, char *argv[]);
int	do_testreset(int argc, char *argv[]);

/* cliDisplay.c에 있는 do_xxx (CMD_ENTRY에 등록되는 것들) */
extern int do_version(int argc, char *argv[]);
extern int do_relay(int argc, char *argv[]);
extern int do_mpucfg(int argc, char *argv[]);
extern int do_esiocfg(int argc, char *argv[]);
extern int do_esiosts_disp(int argc, char *argv[]);
extern int do_iccpcfg(int argc, char *argv[]);
extern int do_iccpdata(int argc, char *argv[]);
extern int do_hostcfg(int argc, char *argv[]);
extern int do_modcfg(int argc, char *argv[]);
extern int do_pointcfg(int argc, char *argv[]);
extern int do_dnpdi(int argc, char *argv[]);
extern int do_dnpai(int argc, char *argv[]);
extern int do_dnpdo(int argc, char *argv[]);
extern int do_devcfg(int argc, char *argv[]);
extern int do_calcfg(int argc, char *argv[]);


CMD_ENTRY   cmdTable[MAX_COMMAND] = {
    {"    ",        do_help,	        "   ",	"----------------------------"},
	{"help",        do_help,	        "help  <ENTER>",	    "Command-List Display "},
	{"process",	    do_process,	        "process <ENTER>",	    "PROCESSOR Status Display"},
	{"clear",	    do_clear,	        "clear <ENTER>",	    "Clear EVENT-Queue."},
	{"setrtc",	    do_setrtc,          "setrtc [YYYY-MM-DD] [HH:MM:SS] <ENTER>", "SET CU-Time Sync (인자 없으면 대화형 입력)"},
	{"event",	    do_event,	        "event <ENTER>",        "CU History-Queue Display"},
	{"off",	        do_off,	            "OFF <ENTER>",          "Monitoring Debug Flag OFF (also clears ICCP mi/mvl FLOW,CFG bits)."},
	{"version",	    do_version,	        "version <ENTER>",      "DISPLAY CU-Version"},
	{"comlogon",	do_comlogon,	    "comlogon <ENTER>",     "CONSOLE Message LOGGING..."},
	{"check",	    do_check,	        "check <ENTER>",        "MPU Status Message Debug..."},
	{"quit",	    do_quit,	        "quit <ENTER>",	        "CONSOLE-TASK quit"},
	{"exit",	    do_exit,	        "exit <ENTER>",         "***> VME-CU Exit<***"},
    {"-------",     do_help,	        "   ",	"----------------------------"},
    {"iccp",	    do_iccp,	        "iccp  <ENTER>",	    "ICCP Communication Debug ON"},
    {"mms",	        do_mms,	            "mms [<mi|mvl|mms|acse> <FLOW|CFG> <on|off>]",   "ICCP-MMS Comm Debug ON / layer FLOW,CFG bit control"},
    {"iccpdata",	do_iccpdata,        "iccpdata <ENTER>",	    "ICCP Association Display"},
    {"iccpcfg",	    do_iccpcfg,         "iccpcfg <ENTER>",	    "ICCP Configuration Display"},
    {"iccpinfo",	do_iccpinfo,        "iccpinfo <ENTER>",	    "ICCP PointInfo Display"},
    {"-------",     do_help,	        "   ",	"----------------------------"},
    {"hd",	        do_hd,	            "hd <1..N> <ENTER>",    "HOST Communication Debug ON"},
    {"hex",	        do_hex,	            "hex <1..N> <ENTER>",   "HOST Message Debug ON"},
    {"soe",	        do_soe,	            "soe <ENTER>",	        "[SOE] Monitoring Debug ON"},
    {"esio",	    do_esio,            "esio <1..N><ENTER>",   "ESIO# Comm Debug"},
    {"dev",	        do_dev,	            "dev <0..N> <ENTER>",   "Device Communication Debug ON"},
	{"msg",	        do_msg,	            "msg <1..N> <ENTER>",   "Device Message Debug ON"},
    {"sim",	        do_sim,             "sim <ENTER>",          "SIMULATOR Comm Debug"},
    {"scu",	        do_scu,             "scu <ENTER>",          "SCU Comm Debug"},
    {"link",	    do_link,            "link <ENTER>",         "LINK Comm Debug"},
    {"wdt",	        do_wdt,             "wdt <ENTER>",          "WDT Message Debug"},
    {"ld",	        do_ld,	            "ld <1..N> <ENTER>",    "Comm Line Debug ON"},
    //{"mpu",	        do_mpu,             "mpu <ENTER>",          "MPU Message Debug"},
    {"cal",	        do_cal,             "cal <1..N> <ENTER>",   "CAL Point Message Debug"},
    {"ntp",	        do_ntp,             "ntp <ENTER>",          "NTP Message Debug"},
    {"device",	    do_device,	        "device <1..N> <ENTER>","DEVICE Report-Info Display"},
    {"esiosts",	    do_esiosts,         "esiosts <ENTER>",      "Display ESIO# Status"},
    {"-------",     do_help,	        "   ",	"----------------------------"},
    {"dnphost",	    do_dnphost,         "dnphost <1..N> <ENTER>","DNP-HOST Report-Info Display"},
    {"dnpdi",	    do_dnpdi,	        "dnpdi <ENTER>",        "DNP-HOST DI Config Display"},
    {"dnpdo",	    do_dnpdo,	        "dnpdo <ENTER>",        "DNP-HOST DO Config Display"},
    {"dnpai",	    do_dnpai,	        "dnpai <ENTER>",        "DNP-HOST AI Config Display"},
    {"-------",     do_help,	        "   ",	"----------------------------"},
    {"mpucfg",	    do_mpucfg,          "mpucfg <ENTER>",	    "MPU Configuration Display"},
    {"esiocfg",	    do_esiocfg,         "esiocfg <ENTER>",	    "ESIO Configuration Display"},
    {"esiosts",     do_esiosts_disp,    "esiosts <ENTER>",      "runtime esio status"  }  ,
    {"hostcfg",	    do_hostcfg,         "hostcfg <ENTER>",	    "HOST Configuration Display"},

    {"portcfg",	    do_portcfg,         "portcfg <0..N> <ENTER>","HARRIS-PORT Configuration Display"},
    {"pointcfg",    do_pointcfg,	    "pointcfg <ENTER>",     "POINT Configuration Display"},
    {"modcfg",	    do_modcfg,          "modcfg <ENTER>",	    "DEVICE MODBUS-Config Display"},
    {"devcfg",	    do_devcfg,	        "devcfg <ENTER>",       "DEV-Point Config Display"},
    {"calcfg",	    do_calcfg,	        "calcfg <ENTER>",       "CAL-Point Config Display"},
    {"relay",	    do_relay,           "relay <ENTER>",	    "DEVICE Configuration Display"},
    {"chksum",	    do_chksum,          "chksum <ENTER>",	    "LINK DB Checksum "},
    {"-------",     do_help,	        "   ",	"----------------------------"},
    {"trip",	    do_trip,	        "trip  <ENTER>",        "[TRIP]   Control TEST."},
    {"close",	    do_close,	        "close <ENTER>",        "[CLOSE]  Control TEST."},
    {"analog",	    do_analog,	        "analog <ENTER>",       "[ANALOG] Test Data Input"},
    {"status",	    do_status,	        "status <ENTER>",       "[STATUS] Test Data Input"},
    {"online",	    do_online,	        "online <ENTER>",       "[DEVICE] Test Data Input"},
    {"change",	    do_change,	        "change <ENTER>",       "[CHANGE] Test CPU Change"},
    {"testdo",	    do_testdo,	        "testdo <ENTER>",       "[TEST-DO] Control TEST."},
    {"devtest",	    do_devtest,	        "devtest <ENTER>",      "[TEST-DO] Control TEST."},
    {"getclock",    do_getclock,	    "getclock <ENTER>",     "[GET CLOCK] Control TEST."},
    {"testset",     do_testset,	    	"testset <ENTER>",      "[TEST-WDT] Control TEST."},
    {"testreset",   do_testreset,	   	"testreset <ENTER>",    "[TEST-WDT] Control TEST."},
    {"-------",     do_help,	        "   ",	"----------------------------"},
//    {"priDbsize",   do_print_dbsize,	"priDbsize <ENTER>",    "[TEST-WDT] print DB struct"},

};


/*
 * get decimal word
 */
int getDecWord()
{
    int ch;
    int ret_val = 0;

    while((ch = getchar()) != 0x0a)
    {
        if(isdigit(ch))
            ret_val = ret_val * 10 + (ch - '0');
    }
    return(ret_val);
}



/*
*
*/

void print_RTU_DATABASE_size(void)
{
    printf("sizeof( ESIO_RTU_DATABASE ) =%d\r\n", sizeof ( ESIO_RTU_DATABASE   ));
    printf("sizeof( DB_ESIO_MPU_CONFIG ) =%d\r\n", sizeof ( DB_ESIO_MPU_CONFIG   ));
    
    printf("sizeof( DB_ESIO_CONFIG ) =%d    MAX_DB_ESIO=%d \r\n", sizeof ( DB_ESIO_CONFIG  ) , MAX_DB_ESIO);

    printf("sizeof( DB_ENET_ENTRY ) =%d\r\n", sizeof ( DB_ENET_ENTRY   ));    
    printf("sizeof( DB_PORT_ENTRY ) =%d MAX_ESIO_PORT=%d\r\n", sizeof ( DB_PORT_ENTRY   ) ,MAX_ESIO_PORT  );    
    printf("sizeof( DB_ESIO_CONFIG ) =%d    MAX_DB_ESIO=%d \r\n", sizeof ( DB_ESIO_CONFIG  ) , MAX_DB_ESIO);
    


    printf("sizeof( DB_HOST_CONFIG ) =%d    MAX_DB_HOST=%d \r\n", sizeof ( DB_HOST_CONFIG  ) , MAX_DB_HOST );
    printf("sizeof( DB_ICCP_CONFIG ) =%d\r\n", sizeof ( DB_ICCP_CONFIG   ));
    
    
    printf("sizeof( DB_MODBUS_PROFILE ) =%d MAX_DB_MODBUS_PROFILE=%d\r\n", sizeof ( DB_MODBUS_PROFILE ), MAX_DB_SCAN_PORT);
    
    
    printf("sizeof( DB_SCAN_CONFIG ) =%d    MAX_DB_DEVICE=%d\r\n", sizeof ( DB_SCAN_CONFIG ),MAX_DB_SCAN_PORT );    
    printf("sizeof( DB_SDP_DEVICE ) =%d    MAX_DB_DEVICE=%d\r\n", sizeof ( DB_SDP_DEVICE ),MAX_DB_DEVICE );        
    printf("sizeof( DB_POINT_BUF ) =%d  MAX_DB_POINT=%d\r\n", sizeof ( DB_POINT_BUF  ),MAX_DB_POINT );    
    printf("sizeof( DB_CAL_POINT ) =%d  MAX_DB_CAL_POINT=%d\r\n", sizeof ( DB_CAL_POINT ) ,MAX_DB_CAL_POINT  );
                 
                 
}   
int do_print_dbsize( int argc, char *argv[])
{
    printf("primary Struct Size\r\n");    
	printf("\n----------------------------------------------\n");
    printf("size of TASK_INFO       = %d\n", sizeof(TASK_INFO) *  (MAX_PROCESS+2));
    printf("size of RTC             = %d\n", sizeof(RTC));
    printf("size of OPR_MSG         = %d\n", sizeof(OPR_MSG));
    printf("size of CONSOLE_INFO    = %d\n", sizeof(CONSOLE_INFO));
    printf("size of ICCP_DCB    	= %d\n", sizeof(ICCP_DCB));
    printf("size of ICCP_60870_DCB	= %d\n", sizeof(ICCP_60870_DCB));
    printf("size of MPU_CONFIG      = %d\n", sizeof(MPU_CONFIG));
    printf("size of ESIO_CONFIG     = %d\n", sizeof(ESIO_CONFIG) * MAX_ESIO);
    printf("size of LINK_MSG     	= %d\n", sizeof(LINK_MSG));
    printf("size of SCU_MSG     	= %d\n", sizeof(SCU_MSG));
    printf("size of HOST_DCB     	= %d\n", sizeof(HOST_DCB)*MAX_HOST);
    printf("size of RTU-DB     		= %d\n", sizeof(RTU)*MAX_HARRIS_RTU);
    printf("size of PORT-DB     	= %d\n", sizeof(PORT_DB)*MAX_HARRIS_PORT);
    printf("size of SCAN_CONFIG     = %d\n", sizeof(SCAN_CONFIG) * MAX_SCAN_PORT);
    printf("size of SDP_DEVICE      = %d\n", sizeof(SDP_DEVICE) * MAX_DEVICE);
    printf("size of DEV_POINT_BUF   = %d\n", sizeof(POINT_BUF) * MAX_DEV_POINT);
    printf("size of CAL_POINT_BUF   = %d\n", sizeof(CAL_POINT_BUF) * MAX_CAL_POINT);
    printf("size of RTU_DATABASE   	= %d\n", sizeof(RTU_DATABASE));
    printf("size of MPU_SOE_QUEUE	= %d\n", sizeof(MPU_SOE_QUEUE));
    printf("size of HISTORY_QUE     = %d\n", sizeof(HISTORY_QUE));

    
 print_RTU_DATABASE_size();   
    return 0;
}



int do_testset(int argc, char *argv[])
{
	printf("==> CPU Master TEST.....\n");
	opr->testWDTFlag = 1;
	return(E_OK);
}

int do_testreset(int argc, char *argv[])
{
	printf("==> CPU Slave TEST.....\n");
	opr->testWDTFlag = 2;
	return(E_OK);
}

int do_portcfg(int argc, char *argv[])
{
	int		vmsid;

	if (argc != 2)
	{
		printf("%%%%ERR_Usage: portcfg <0..%d>\n", MAX_HARRIS_PORT);
		return(E_ERR);
	}
	if (ParseNumber(argv[1], 0, MAX_HARRIS_PORT, &vmsid, "HARRIS PORT Config") < 0)
		return(E_ERR);

    disp_HarrisPort(vmsid);
    return(E_OK);
}

int do_dnphost(int argc, char *argv[])
{
	int		vmsid;

	if (argc != 2)
	{
		printf("%%%%ERR_Usage: dnphost <1..%d>\n", MAX_HOST);
		return(E_ERR);
	}
	if (ParseNumber(argv[1], 1, MAX_HOST, &vmsid, "HOST DNP Data Display") < 0)
		return(E_ERR);

    vmsid = vmsid - 1;
    dispHostData_DNP(vmsid);
    return(E_OK);
}

//
// 모듈:	do_dev()
//
int do_dev(int argc, char *argv[])
{
	int		vmsid;

	if (argc != 2)
	{
		printf("%%%%ERR_Usage: dev <0..%d>\n", MAX_DEVICE);
		return(E_ERR);
	}
	if (ParseNumber(argv[1], 0, MAX_DEVICE, &vmsid, "DNP Device Comm Debug") < 0)
		return(E_ERR);

    if(vmsid == 0)  opr->dnpDebug = MAX_DEVICE;
    else            opr->dnpDebug = vmsid - 1;

    printf("[*] DNP Device (%2d) Comm Debug... ...!\n", vmsid);
    return(E_OK);
}

//
// 모듈:	do_msg()
//
int do_msg(int argc, char *argv[])
{
	int		vmsid;

	if (argc != 2)
	{
		printf("%%%%ERR_Usage: msg <1..%d>\n", MAX_DEVICE);
		return(E_ERR);
	}
	if (ParseNumber(argv[1], 1, MAX_DEVICE, &vmsid, "DNP Device Message Debug") < 0)
		return(E_ERR);

    opr->msgDebug = vmsid - 1;
    printf("[*] DNP Device (%2d) Message Debug... ...!\n", vmsid);
    return(E_OK);
}

/*
* esio 계열 커맨드처럼 getDecWord()로 2개 값(TYPE/POINT)을 그대로 받는다.
* 인자 없이 대화형으로만 동작하던 원본 CmdDispICCPInfo() 본문을 그대로 유지.
*/
int do_iccpinfo(int argc, char *argv[])
{
	int		type, point;

    printf("\n[*] ICCP POINT-DATA Display.... !\n");
    printf("=> Select TYPE [1]SDI, [2]SDO, [3]SAI, [4]DDI , [5]DAI, [6]QDI, [7]QAI, [8]TDI, [9]TAI, [10]DEV ...  ");

    type = getDecWord() & 0x3f;

    printf("=> Select POINT [1...] ");
    point = getDecWord();

    display_ICCP_INFO(type, point);

    return(E_OK);
}

//
// 모듈:	do_comlogon()
//  - Console 메시지를 파일로 로깅...
//
int do_comlogon(int argc, char *argv[])
{
    char buffer[256];
    int     length;

	if (argc != 1)
	{
		printf("%%%%ERR_Usage: comlogon\n");
		return(E_ERR);
	}

    if(console->logFileFlag != SET)
    {
        console->logFileFlag = SET;
        printf("\n------------------------------------\n");
        printf("=> CONSOLE Massage LOG: Start... !\n");
        printf("------------------------------------\n");

        sprintf(buffer, "-----------------------------------------------\n");
        length = strlen(buffer);
        LogFile_CONSOLE((char *) buffer, length);

        sprintf(buffer, "%4d-%02d-%2d %02d:%02d:%02d : LOG START ...\n", rtc->year, rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
        length = strlen(buffer);
        LogFile_CONSOLE((char *) buffer, length);

        sprintf(buffer, "-----------------------------------------------\n");
        length = strlen(buffer);
        LogFile_CONSOLE((char *) buffer, length);
    }
    else
    {
        console->logFileFlag = RESET;
        printf("\n------------------------------------\n");
        printf("=> CONSOLE Massage LOG: Stop... !\n");
        printf("------------------------------------\n");

        sprintf(buffer, "-----------------------------------------------\n");
        length = strlen(buffer);
        LogFile_CONSOLE((char *) buffer, length);

        sprintf(buffer, "%4d-%02d-%2d %02d:%02d:%02d : LOG STOP ...\n", rtc->year, rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
        length = strlen(buffer);
        LogFile_CONSOLE((char *) buffer, length);

        sprintf(buffer, "-----------------------------------------------\n");
        length = strlen(buffer);
        LogFile_CONSOLE((char *) buffer, length);
    }

    return(E_OK);
}

//
// 모듈:	do_getclock()
//
int do_getclock(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("%%%%ERR_Usage: getclock\n");
		return(E_ERR);
	}

    opr->getClockNTP = SET;
    printf("> GET NTP-Server .... TIME req...! \n");
    return(E_OK);
}


int do_setrtc(int argc, char *argv[])
{
    int  year, month, day, week, hour, min, sec;

    /*
    * 인자로 날짜/시간이 오면 대화형 프롬프트 없이 바로 처리한다.
    *   setrtc 2026-8-27 14:11:11   (날짜 + 시간)
    *   setrtc 2026-8-27            (날짜만, 시간은 현재값 유지)
    *   setrtc 14:11:11             (시간만, 날짜는 현재값 유지)
    *
    * parse_line()이 ':'를 구분자로 이미 잘라내므로 "14:11:11"은
    * 이 함수에 도착할 때 이미 "14" "11" "12" 세 개의 숫자 토큰으로
    * 나뉘어 있다. '-'가 들어있는 토큰만 날짜로 보고, 나머지 숫자
    * 토큰 3개를 모으면 시/분/초로 취급한다.
    */
    if (argc > 1)
    {
        int         i;
        int         haveDate = 0, haveTime = 0;
        int         numBuf[3];
        int         numCount = 0;
        time_t      now;
        struct tm   tmVal;

        time(&now);
        localtime_r(&now, &tmVal);
        year  = tmVal.tm_year + 1900;
        month = tmVal.tm_mon + 1;
        day   = tmVal.tm_mday;
        hour  = tmVal.tm_hour;
        min   = tmVal.tm_min;
        sec   = tmVal.tm_sec;

        for (i = 1; i < argc; i++)
        {
            if (strchr(argv[i], '-') != NULL)
            {
                if (sscanf(argv[i], "%d-%d-%d", &year, &month, &day) != 3)
                {
                    printf("%%%%ERR_Usage: setrtc [YYYY-MM-DD] [HH:MM:SS]\n");
                    return(E_ERR);
                }
                haveDate = 1;
            }
            else if (numCount < 3)
            {
                numBuf[numCount++] = atoi(argv[i]);
            }
            else
            {
                printf("%%%%ERR_Usage: setrtc [YYYY-MM-DD] [HH:MM:SS]\n");
                return(E_ERR);
            }
        }

        if (numCount == 3)
        {
            hour = numBuf[0];
            min  = numBuf[1];
            sec  = numBuf[2];
            haveTime = 1;
        }
        else if (numCount != 0)
        {
            printf("%%%%ERR_Usage: setrtc [YYYY-MM-DD] [HH:MM:SS] (시간은 시/분/초 3개 모두 입력)\n");
            return(E_ERR);
        }

        if (!haveDate && !haveTime)
        {
            printf("%%%%ERR_Usage: setrtc [YYYY-MM-DD] [HH:MM:SS]\n");
            return(E_ERR);
        }

        /* 요일은 사용자에게 묻지 않고 날짜로부터 계산한다 */
        memset(&tmVal, 0, sizeof(tmVal));
        tmVal.tm_year  = year - 1900;
        tmVal.tm_mon   = month - 1;
        tmVal.tm_mday  = day;
        tmVal.tm_hour  = hour;
        tmVal.tm_min   = min;
        tmVal.tm_sec   = sec;
        tmVal.tm_isdst = -1;
        if (mktime(&tmVal) == (time_t) -1)
        {
            printf("%%%%ERR_Invalid date/time\n");
            return(E_ERR);
        }
        week = tmVal.tm_wday;

        opr->year   = year;
        opr->month  = month;
        opr->day    = day;
        opr->week   = week;
        opr->hour   = hour;
        opr->min    = min;
        opr->sec    = sec;

        opr->rtcUpdateICCP = SET;		// ICCP-HOST Time-Sync 요청
        opr->rtcUpdateFlag = SET;

        printf(">> CU Time SET : %04d/%02d/%02d-%d-%02d:%02d:%02d\n", year, month, day, week, hour, min, sec);
        printf("*clock modified (arg 입력, 확인 프롬프트 없음)\n");

        return(E_OK);
    }

    printf("[*] Clock Setting\n");
    printf("year [2000-2099] = "); year  = getDecWord();
    printf("month[1-12] 	 = "); month = getDecWord();
    printf("day  [1-31] 	 = "); day   = getDecWord();
    printf("hour [0-23] 	 = "); hour  = getDecWord();
    printf("min  [0-59] 	 = "); min   = getDecWord();
    printf("sec  [0-59] 	 = "); sec   = getDecWord();
    printf("week [0 -6] 	 = "); week  = getDecWord();
    printf("correct or not (y/n) ? ");
    if(getchar() == 'y')
    {
        opr->year   = year;
        opr->month  = month;
        opr->day    = day;
        opr->week   = week;
        opr->hour   = hour;
        opr->min    = min;
        opr->sec    = sec;

        opr->rtcUpdateICCP = SET;		// ICCP-HOST Time-Sync 요청
        opr->rtcUpdateFlag = SET;

        printf(">> CU Time SET : %04d/%02d/%02d-%d-%02d:%02d:%02d\n", year, month, day, week, hour, min, sec);
        printf("*clock modified\n");

    }
    else
    {
        printf("*clock cancel\n");
    }

    return(E_OK);
}


//// 모듈:	Test용 TRIP/CLOSE()
//
int do_testdo(int argc, char *argv[])
{
    int     i,j;
    int     devid, point, tcf;
    SDP_DEVICE      *dev;
    SCAN_CONFIG     *scan;
    POINT_BUF       *ptBuf;

	if (CheckEOT() < 0) return(E_ERR);

    printf("[*] Test DOM Control.... !\n");
    printf("1. Control Device# [1..%d] : ", MAX_DEVICE);

    devid = getDecWord() & 0x3f;
    point = 0;
    tcf   = TRIP_CONTROL;

    if((devid < 1) || (devid > MAX_DEVICE))
    {
        printf("*** Invalid Test DOM :  %2d \n", devid);
        return(E_ERR);
    }

    devid = devid - 1;

CONTROL_NEXT:
    printf("\n\n------------------------------------[%4d/%2d/%2d %02d:%02d:%02d]\n",
       	((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
    for(i=0; i< MAX_SCAN_PORT; i++)
    {
        scan = (SCAN_CONFIG *) scanCFG[i];
        if(scan->scanIndex <= 0)   continue;

        for(j=0; j< scan->scanIndex; j++)
        {
            /* 대상 장치를 search... */
            if(scan->scanDevice[j] == devid)
            {
                dev = (SDP_DEVICE *) deviceCFG[devid];
                ptBuf = (POINT_BUF *) &dev->doPtBuf[point];

                dev->cntPoint = point;
                dev->cntTCF   = tcf;   /* TRIP */
                dev->cntrType = ptBuf->ptConfig;
                dev->selectReq  = SET;

                opr->rcvONtime = 500;

                if(tcf == TRIP_CONTROL)
                {
                	logEvent_MPU(shmPtr, ENT_USER_CNTR, devid+1, point+1, TRIP_CONTROL, opr->cpuMode, NULL);
                	printf(">> DOM(%d)- TRIP  Control, point=%d / tcf=%2x ...[%4d/%2d/%2d %02d:%02d:%02d]\n", devid+1, point+1, tcf,
                		((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
                }
                else if(tcf == CLOSE_CONTROL)
                {
                	logEvent_MPU(shmPtr, ENT_USER_CNTR, devid+1, point+1, CLOSE_CONTROL, opr->cpuMode, NULL);
               	    printf(">> DOM(%d)- CLOSE Control, point=%d / tcf=%2x ...[%4d/%2d/%2d %02d:%02d:%02d]\n", devid+1, point+1, tcf,
                	    ((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
               	}


               	if(tcf == TRIP_CONTROL)
		        {
        			tcf = CLOSE_CONTROL;
		        }
        		else if(tcf == CLOSE_CONTROL)
		        {
        			tcf = TRIP_CONTROL;
        			point = point + 1;

        			if(point >= 16)
        			{
        				point = 0;
		        		printf(">> Control DOM(%d) .... Test End...1 !\n", devid+1);
        			}
        		}
            }
        }

    }

    pause(1000);
    pause(1000);

    goto CONTROL_NEXT;

    printf(">> Control DOM(%d) .... Test End...2 !\n", devid+1);
    return(E_OK);
}

/*
*   TRIP-제어실행
*/
int do_trip(int argc, char *argv[])
{
    int     devid, point;
    char    buffer[256];
    struct timeval  ctime;

	if (CheckEOT() < 0) return(E_ERR);

    printf("[*] Test TRIP Control.... !\n");
    printf("1. Control Device# [1..%d] : ", MAX_DEVICE);
    devid = getDecWord() & 0x3f;
    printf("2. Control Point # [1..%d] : ", MAX_DEV_DO_POINT);
    point = getDecWord();

    if((devid < 1) || (devid > MAX_DEVICE) || (point < 1) || (point > MAX_DEV_DO_POINT))
    {
        printf("*** Invalid Control... :  %2d %2d\n", devid, point);
        return(E_ERR);
    }

    printf("==> Control Device-%02d, Point-%02d ... TRIP Control...[%4d/%2d/%2d %02d:%02d:%02d]\n",  devid, point,
       	((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);


    gettimeofday(&ctime, NULL);
    logEvent_MPU(shmPtr, ENT_USER_CNTR, devid, point, TRIP_INFO, opr->cpuMode, &ctime);

    sprintf(buffer, "<-- USER> Control INFO : Dev=%d, PT=%d, TCF[TRIP=1, CLOSE=2]= %2x... ", devid, point, TRIP_INFO);
    LogFile_MPU (shmPtr, ENT_USER_CNTR, buffer, strlen(buffer));

    controlInfo_MPU( shmPtr, devid, point, TRIP_INFO, PASS_USER_CNTR, &ctime);

    return(E_OK);
}

/*
*   CLOSE-제어실행
*/
int do_close(int argc, char *argv[])
{
    int     devid, point;
    char    buffer[256];
    struct timeval  ctime;

	if (CheckEOT() < 0) return(E_ERR);

    printf("[*] Test CLOSE Control.... !\n");
    printf("1. Control Device# [1..%d] : ", MAX_DEVICE);
    devid = getDecWord() & 0x3f;
    printf("2. Control Point # [1..%d] : ", MAX_DEV_DO_POINT);
    point = getDecWord();

    if((devid < 1) || (devid > MAX_DEVICE) || (point < 1) || (point > MAX_DEV_DO_POINT))
    {
        printf("*** Invalid Control... :  %2d %2d\n", devid, point);
        return(E_ERR);
    }

    printf("==> Control Device-%02d, Point-%02d ... CLOSE Control...[%4d/%2d/%2d %02d:%02d:%02d]\n",  devid, point,
       	((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);

    gettimeofday(&ctime, NULL);
    logEvent_MPU(shmPtr, ENT_USER_CNTR, devid, point, CLOSE_INFO, opr->cpuMode, &ctime);

    sprintf(buffer, "<-- USER> Control INFO : Dev=%d, PT=%d, TCF[TRIP=1, CLOSE=2]= %2x... ", devid, point, CLOSE_INFO);
    LogFile_MPU (shmPtr, ENT_USER_CNTR, buffer, strlen(buffer));

    controlInfo_MPU( shmPtr, devid, point, CLOSE_INFO, PASS_USER_CNTR, &ctime);

    return(E_OK);
}

//
// 모듈:	do_change()
//
int do_change(int argc, char *argv[])
{
    char    buffer[256];

	if (argc != 1)
	{
		printf("%%%%ERR_Usage: change\n");
		return(E_ERR);
	}

    if((opr->dualCpuSts == SET) && (scuCfg->remoteMode == AUTO_MODE))
    {
        opr->cpuChange = SET;
        printf("TEST>> CPU Change...!\n");

        logEvent_MPU(shmPtr, ENT_CHANGE_CPU, CPU_CHG_USER, 0, 0, opr->cpuMode, NULL);

        sprintf(buffer, "<-- USER> CPU CHANGE cmd ... " );
        LogFile_MPU (shmPtr, ENT_CHANGE_CPU, buffer, strlen(buffer));
    }
    else
    {
        printf("TEST>> *** Invalid CPU Change...!\n");
    }

    return(E_OK);
}

int do_analog(int argc, char *argv[])
{
    int     devid, point;
    float   aiData;

	if (CheckEOT() < 0) return(E_ERR);

    printf("[*] Test ANALOG Point Update.... !\n");
    printf("1. Input Device# [1..%d] : ", MAX_DEVICE);
    devid = getDecWord() & 0x3f;
    printf("2. Input Point # [1..80] : ");
    point = getDecWord();
    printf("3. Input Analog Data     : ");
    aiData = getDecWord() * 1.0 ;

    if((devid < 1) || (devid > MAX_DEVICE) || (point < 1) || (point > MAX_DEV_DI_POINT))
    {
        return(E_ERR);
    }

    printf("\n----------------------------------------------------->>\n");
    printf("==> ANALOG Info Device-%02d, Point-%02d ... Data = %4.2f !\n", devid, point, aiData);
    printf("----------------------------------------------------->>\n");

    opr->testDevid = devid - 1;
    opr->testPoint = point - 1;

    opr->testAIData = aiData;

    opr->testAnalogFLAG = SET;

    return(E_OK);
}

/*
* SDP 장치 포인트 상태입력....
*/
int do_devtest(int argc, char *argv[])
{
    int     devIndex, state, point;
    int     devNo;

    POINT_BUF   *devPoint, *diPoint;
    SDP_DEVICE  *dev;

	if (CheckEOT() < 0) return(E_ERR);

    printf("[*] Test DEVICE Point Update.... !\n");
    printf("1. SDP-Device Point # [1..%d] : ", MAX_DEV_POINT);
    devIndex = getDecWord();
    printf("2. Input Status [0 / 1] : ");
    state = getDecWord() & 0x01;

    if((devIndex < 1) || (devIndex > MAX_DEV_POINT))
    {
        printf("*** Invalid DEVICE-Point ...%2d \n", devIndex);
        return(E_ERR);
    }

    opr->devTestFlag = SET;

    devPoint = (POINT_BUF *) devPtBuf[devIndex - 1];
    if(devPoint->config == 0)
    {
        printf("*** No Define... DEVICE-Point = %d\n", devIndex);
        return(E_ERR);
    }

    devNo = devPoint->devNo;
    point = devPoint->devPt;
    devPoint->status = state;

    if((devNo > 0) && (point > 0))
    {
        dev = (SDP_DEVICE *) deviceCFG[devNo - 1];
        diPoint = (POINT_BUF *) &dev->diPtBuf[point - 1];
        diPoint->status = state;

        printf(">> DEVICE-POINT (%3d) : %s .... status = %d \n",   devIndex, diPoint->ptNameStr, diPoint->status);
    }

    return(E_OK);
}


int do_status(int argc, char *argv[])
{
    int     devid, point, state;

	if (CheckEOT() < 0) return(E_ERR);

    printf("[*] Test STATUS Point Update.... !\n");
    printf("1. Input Device# [1..%d] : ", MAX_DEVICE);
    devid = getDecWord() & 0x3f;
    printf("2. Input Point # [1..80] : ");
    point = getDecWord();
    printf("3. Input Status# [0...1] : ");
    state = getDecWord() & 0x01;

    if((devid < 1) || (devid > MAX_DEVICE) || (point < 1) || (point > MAX_DEV_DI_POINT))
    {
        return(E_ERR);
    }

    printf("\n----------------------------------------------------->>\n");
    printf("==> Status Info Device-%02d, Point-%02d ... state = %d !\n", devid, point, state);
    printf("----------------------------------------------------->>\n");

    opr->testDevid = devid - 1;
    opr->testPoint = point - 1;
    opr->testState = state;

    opr->testStatusFLAG = SET;

    return(E_OK);
}


int do_online(int argc, char *argv[])
{
    int     devid, state;

	if (CheckEOT() < 0) return(E_ERR);

    printf("[*] Test DEVICE Online/Offline Update.... !\n");
    printf("1. Input Device# [1..%d] : ", MAX_DEVICE);
    devid = getDecWord() & 0x3f;

    printf("1. Input Status# [0] offline, [1] Online : ");
    state = getDecWord() & 0x01;

    if((devid < 1) || (devid > MAX_DEVICE))
    {
        return(E_ERR);
    }

    printf("\n----------------------------------------------------->>\n");
    printf("==> Device Offline/Online ... devid= %2d, state = %d !\n", devid, state);
    printf("----------------------------------------------------->>\n");

    opr->testDevid = devid - 1;
    opr->testPoint = 0;
    opr->testState = state;

    opr->testDeviceFLAG = SET;

    return(E_OK);
}


//
// 모듈:	do_clear()
//
int do_clear(int argc, char *argv[])
{
    word    chksum;

	if (argc != 1)
	{
		printf("%%%%ERR_Usage: clear\n");
		return(E_ERR);
	}

    /* 콘솔 HISTORY & EVENT Queue 전체 Clear... */
    printf("[*] HISTORY Queue Cleared.... !\n");
    bzero8248((byte *) hque, sizeof(HISTORY_QUE));
    chksum = gensum((byte *) hque, sizeof(HISTORY_QUE) - 2);

    hque->chksum = chksum;
    hque->clear  = 255;

    return(E_OK);
}


int do_hd(int argc, char *argv[])
{
	int		vmsid;

	if (argc != 2)
	{
		printf("%%%%ERR_Usage: hd <1..%d>\n", MAX_HOST);
		return(E_ERR);
	}
	if (ParseNumber(argv[1], 1, MAX_HOST, &vmsid, "HOST Comm Debug") < 0)
		return(E_ERR);

    opr->hostDebug = vmsid - 1;
    printf("[*] HOST (%2d) Comm Debug... ...!\n", vmsid);
    return(E_OK);
}

int do_ld(int argc, char *argv[])
{
	int		vmsid;

	if (argc != 2)
	{
		printf("%%%%ERR_Usage: ld <1..%d>\n", MAX_HOST);
		return(E_ERR);
	}
	if (ParseNumber(argv[1], 1, MAX_HOST, &vmsid, "Comm Line Debug") < 0)
		return(E_ERR);

    opr->lineDebug = vmsid - 1;
    printf("[*] Comm[%d] Line Debug On ...!\n", vmsid);
    return(E_OK);
}

int do_soe(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("%%%%ERR_Usage: soe\n");
		return(E_ERR);
	}

    opr->soeDebug = SET;
    printf("[*] SOE Queue ... Message Debug ON...!\n");
    return(E_OK);
}

int do_ntp(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("%%%%ERR_Usage: ntp\n");
		return(E_ERR);
	}

    opr->ntpDebug = SET;
    printf("[*] NTP ... Message Debug ON...!\n");
    return(E_OK);
}

int do_check(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("%%%%ERR_Usage: check\n");
		return(E_ERR);
	}

    opr->checkDebug = SET;
    printf("[*] CHECK ... Message Debug ON...!\n");
    return(E_OK);
}

/* cmdTable에는 등록돼 있지 않음 (원본 CLI-ReadLine도 주석 처리) */
int do_mpu(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("%%%%ERR_Usage: mpu\n");
		return(E_ERR);
	}

    opr->mpuDebug = SET;
    printf("[*] MPU... RUN Message Debug ON...!\n");
    return(E_OK);
}


int do_hex(int argc, char *argv[])
{
	int		vmsid;

	if (argc != 2)
	{
		printf("%%%%ERR_Usage: hex <1..%d>\n", MAX_HOST);
		return(E_ERR);
	}
	if (ParseNumber(argv[1], 1, MAX_HOST, &vmsid, "HOST Message Debug") < 0)
		return(E_ERR);

    opr->hexDebug = vmsid - 1;
    printf("[*] HOST (%2d) DNP-Message Debug... ...!\n", vmsid);
    return(E_OK);
}


/*
*  esio #0 = SIO, esio #1-계전기감시, esio #2-통신제어, esio #3-품질진단, esio #4-61850, esio #5-RTU
*/
int do_esio(int argc, char *argv[])
{
	int		esioid;

	if (argc != 2)
	{
		printf("%%%%ERR_Usage: esio <1..%d>\n", MAX_ESIO);
		return(E_ERR);
	}
	if (ParseNumber(argv[1], 1, MAX_ESIO, &esioid, "ESIO# Comm Debug") < 0)
		return(E_ERR);

    opr->esioDebug = esioid;
    printf("[*] ESIO (%2d) Comm Debug... ...!\n", esioid);
    return(E_OK);
}


int do_cal(int argc, char *argv[])
{
	int		esioid;

	if (argc != 2)
	{
		printf("%%%%ERR_Usage: cal <1..%d>\n", MAX_CAL_POINT);
		return(E_ERR);
	}
	if (ParseNumber(argv[1], 1, MAX_CAL_POINT, &esioid, "CAL Point Debug") < 0)
		return(E_ERR);

    opr->calDebug = esioid - 1;
    printf("[*] CAL POINT Message Debug...%d !\n", esioid);
    return(E_OK);
}

//
// 모듈:	do_esiosts()
//
int do_esiosts(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("%%%%ERR_Usage: esiosts\n");
		return(E_ERR);
	}
    printf("[*] Display ESIO# Status ...!\n");
    opr->esioStsDebug = 1;

    return(E_OK);
}

//
// 모듈:	do_wdt()
//
int do_wdt(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("%%%%ERR_Usage: wdt\n");
		return(E_ERR);
	}
    printf("[*] WDT Message Debug ON ...!\n");
    opr->wdtDebug = 1;

    return(E_OK);
}

//
// 모듈:	do_iccp()
//
int do_iccp(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("%%%%ERR_Usage: iccp\n");
		return(E_ERR);
	}
    printf("[*] ICCP HOST Comm Debug ON ...!\n");
    opr->iccpDebug = 1;

    return(E_OK);
}

//
// 모듈:	do_mms()
//
int do_mms(int argc, char *argv[])
{
	int value;

	if (argc == 1)
	{
	    printf("[*] ICCP-MMS Comm Debug ON ...!\n");
	    opr->mmsDebug = 1;
	    return(E_OK);
	}

	if (argc != 4)
	{
		printf("%%%%ERR_Usage: mms | mms <mi|mvl|mms|acse> <FLOW|CFG> <on|off>\n");
		return(E_ERR);
	}

	value = parseOnOff(argv[3]);
	if (value < 0)
	{
		printf("%%%%ERR_'%s' is not on/off\n", argv[3]);
		return(E_ERR);
	}

	return postLogbitSet(argv[1], argv[2], value);
}

//
// ---------------------------------------------------------------------
// 2026-10-02 : CLI-S2 <-> ICCP layer FLOW/CFG bit control helpers
// See design/20261002_iccp_log_control_S2.md for the full design.
// ---------------------------------------------------------------------
//

//
// 모듈:	parseOnOff()
//   "on" -> 1, "off" -> 0, 그 외 -> -1
//
int parseOnOff(const char *s)
{
	if (strcmp(s, "on") == 0)  return 1;
	if (strcmp(s, "off") == 0) return 0;
	return -1;
}

//
// 모듈:	waitLogbitAck()
//   opr->logbitCtl.ack가 seq와 같아질 때까지 최대 약 1초간 폴링한다.
//   ack 확인 시 1, 타임아웃 시 0을 반환한다.
//
int waitLogbitAck(short seq)
{
	int i;

	for (i = 0; i < 100; i++)
	{
		if (opr->logbitCtl.ack == seq)
			return 1;
		pause(10);
	}
	return 0;
}

//
// 모듈:	postLogbitSet()
//   공유메모리를 통해 ICCP 프로세스에 레이어의 FLOW/CFG 비트 on/off를
//   요청하고 결과를 출력한다.
//
int postLogbitSet(const char *layer, const char *bit, int value)
{
	short seq;

	opr->logbitCtl.cmd = LOGBIT_CMD_SET;
	strncpy(opr->logbitCtl.layer, layer, sizeof(opr->logbitCtl.layer)-1);
	opr->logbitCtl.layer[sizeof(opr->logbitCtl.layer)-1] = 0;
	strncpy(opr->logbitCtl.bit, bit, sizeof(opr->logbitCtl.bit)-1);
	opr->logbitCtl.bit[sizeof(opr->logbitCtl.bit)-1] = 0;
	opr->logbitCtl.value = value;
	seq = ++opr->logbitCtl.seq;

	if (!waitLogbitAck(seq))
	{
		printf("%%%%ERR_no response from ICCP (timeout)\n");
		return (E_ERR);
	}

	switch (opr->logbitCtl.result)
	{
	case 0:
		printf("[*] %s %s set to %s\n", layer, bit, value ? "ON" : "OFF");
		return (E_OK);
	case -1:
		printf("%%%%ERR_unknown layer '%s'\n", layer);
		return (E_ERR);
	case -2:
		printf("%%%%ERR_layer '%s' has no '%s' bit\n", layer, bit);
		return (E_ERR);
	default:
		printf("%%%%ERR_unexpected result (%d)\n", opr->logbitCtl.result);
		return (E_ERR);
	}
}

//
// 모듈:	postLogbitClearAll()
//   "off" 명령에서 사용: ICCP 쪽 모든 레이어의 FLOW/CFG 비트를 끈다.
//   opr->*Debug 플래그는 호출자가 이미 0으로 돌려놓았으므로, 타임아웃이
//   나더라도 치명적이지 않다 (경고만 출력).
//
void postLogbitClearAll(void)
{
	short seq;

	opr->logbitCtl.cmd = LOGBIT_CMD_CLEAR_ALL;
	seq = ++opr->logbitCtl.seq;

	if (!waitLogbitAck(seq))
		printf("%%%%ERR_no response from ICCP (timeout) while clearing layer FLOW/CFG bits\n");
}

//
// 모듈:	do_sim()
//
int do_sim(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("%%%%ERR_Usage: sim\n");
		return(E_ERR);
	}
    printf("[*] SIMULATOR Comm Debug ON ...!\n");
    opr->simDebug = 1;

    return(E_OK);
}

//
// 모듈:	do_chksum()
//
int do_chksum(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("%%%%ERR_Usage: chksum\n");
		return(E_ERR);
	}
    printf("[*] LINK DB Checksum REQ ...!\n");

    if(opr->cpuMode == MPU_B)
    {
        linkCfg->cpuStatus = 0x02;      // DB-CHK set
        rtudb->chassisNum += 1;
    }
    else if(opr->cpuMode == MPU_A)
    {
        linkCfg->chksumReq = 1;
    }

    return(E_OK);
}

//
// 모듈:	do_scu()
//
int do_scu(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("%%%%ERR_Usage: scu\n");
		return(E_ERR);
	}
    printf("[*] SCU Comm Debug ON ...!\n");
    opr->scuDebug = 1;

    return(E_OK);
}

//
// 모듈:	do_link()
//
int do_link(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("%%%%ERR_Usage: link\n");
		return(E_ERR);
	}
    printf("[*] LINK Comm Debug ON ...!\n");
    opr->linkDebug = 1;

    return(E_OK);
}

//
// 모듈:	do_off()
//
int do_off(int argc, char *argv[])
{
    char    buffer[256];
    int     length;

	if (argc != 1)
	{
		printf("%%%%ERR_Usage: off\n");
		return(E_ERR);
	}

    printf("[*] Debug OFF ...!\n");

    opr->wdtDebug   = 0;
    opr->simDebug   = 0;
    opr->scuDebug   = 0;
    opr->linkDebug  = 0;
    opr->esioStsDebug = 0;
    opr->iccpDebug  = 0;
    opr->mmsDebug   = 0;
    opr->soeDebug   = 0;
    opr->ntpDebug   = 0;
    opr->checkDebug = 0;

    opr->testWDTFlag  = 0;

    opr->lineDebug  = 0xff;
    opr->hostDebug  = 0xff;
    opr->hexDebug   = 0xff;
    opr->esioDebug  = 0xff;
    opr->dnpDebug   = 0xff;
    opr->msgDebug   = 0xff;
    opr->mpuDebug    = 0;
    opr->calDebug    = 0xff;

    opr->armPoint = 0;

    opr->devTestFlag = 0;

    /* CONSOLE Message : LOG 중지 Flag */
    if(console->logFileFlag == SET)
    {
        console->logFileFlag = RESET;
        printf("\n------------------------------------\n");
        printf("=> CONSOLE Massage LOG: Stop... !\n");
        printf("------------------------------------\n");

        sprintf(buffer, "-----------------------------------------------\n");
        length = strlen(buffer);
        LogFile_CONSOLE((char *) buffer, length);

        sprintf(buffer, "%4d-%02d-%2d %02d:%02d:%02d : LOG STOP ...\n", rtc->year, rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
        length = strlen(buffer);
        LogFile_CONSOLE((char *) buffer, length);

        sprintf(buffer, "-----------------------------------------------\n");
        length = strlen(buffer);
        LogFile_CONSOLE((char *) buffer, length);
    }

    postLogbitClearAll();              /* 2026-10-02 : clear ICCP mi/mvl FLOW,CFG bits too */

    return(E_OK);
}

//
// 모듈:	do_quit()
//
int do_quit(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("%%%%ERR_Usage: quit\n");
		return(E_ERR);
	}

	termExec = 0;

	do_off(1, argv);

	return(E_OK);
}

//
// 모듈:	do_exit()
//
int do_exit(int argc, char *argv[])
{
    char    buffer[256];
    TASK_INFO	*wdtPtr = NULL;

	if (argc != 1)
	{
		printf("%%%%ERR_Usage: exit\n");
		return(E_ERR);
	}

    printf(">> Program Terminate.... Are You Sure...? ");
    if(getchar() == 'y')
    {
        opr->userReset = SET;

        logEvent_MPU(shmPtr, ENT_RESET_USER, 30, 0, 0, opr->cpuMode, NULL);

        sprintf(buffer, "%s", "----------------------------------------------------");
        LogFile_MPU(shmPtr, ENT_RESET_USER, buffer, strlen(buffer));

        pause(1000);

        wdtPtr = (TASK_INFO *) &shmPtr->taskInfo[WDT_PROCESS];
        printf("[*] WDT Process ... pid= %d\n", wdtPtr->pid);
        ExecCommand("kill %d \n", wdtPtr->pid);

	    termExec = 0;
    }

    return(E_OK);
}

//
// 모듈:	do_help()
//
// 원본 CmdHelp()는 GetToken()으로 두번째 토큰을 다시 읽어 특정 명령의
// usage/help만 필터링했다. 여기서는 그 토큰을 argv[1]로 그대로 받는다.
//
int do_help(int argc, char *argv[])
{
	int		index, fIndex, length;
	CMD_ENTRY	*cp;

	if (argc > 1)
	{
		cp = cmdTable;
		for (index = 0, fIndex = 0; index < MAX_COMMAND; index++, cp++)
		{
			if (strlen(cp->name) == 0)
				continue;
			length = strlen(cp->name) < strlen(argv[1]) ? strlen(cp->name) :
				strlen(argv[1]);
			if (strncmp(cp->name, argv[1], length) != 0)
				continue;
			printf("Command %s:\n", cp->name);
			printf("\tDescription: %s\n", cp->help);
			printf("\tUsage: %s\n", cp->usage);
			fIndex++;
		}
		if (fIndex == 0)
			printf("%%%%WRN_Invalid Command %s\n", argv[1]);
		return(E_OK);
    }

	for (index = 0, cp = cmdTable; index < MAX_COMMAND; index++, cp++)
	{
		if (strlen(cp->name) == 0)
		{
			break;
        }
		printf("%-10s %s\n", cp->name, cp->help);
    }

	return(E_OK);
}


//
// 모듈:	do_process()
//
int do_process(int argc, char *argv[])
{
    int     ch;
	int		idx;
	TASK_INFO	*ptr;

    while(1)
    {
	PrnLine('=');
	printf(" Process DEF ACT ARG DBG CHK %-11s RST %-8s Last %-6s %-6s\n", 	"WDT-Count", "PID", "MaxET", "PrevET");
	PrnLine('-');
	for (idx = 0, ptr = shmPtr->taskInfo; idx < MAX_PROCESS; idx++, ptr++)
	{
		if (strlen(ptr->name) == 0)
			continue;
		printf(" %-7s %-3d %-3d %-3d %-3d %-3d %4d/%-6d %-3d %-8d %-4d %-6d %-6d\n",
			ptr->name,
			 ptr->define, ptr->active, ptr->hostid, ptr->debug,
			ptr->check, ptr->wdtCount, ptr->wdtEnable, ptr->restart,
			ptr->pid, ptr->lastWdt, ptr->maxEtime / 1000,
			ptr->prevEtime / 1000);
	}
	PrnLine('=');

	ch = getchar();
	if((ch == 'q') || (ch == 'x'))  break;

    }

	return(E_OK);
}

/*
*   GIPAM2000 - DEVICE
*/
int do_device(int argc, char *argv[])
{
    char    type;
    int     point;
	int		vmsid;
	int devid;

	if (GetNumber(1, MAX_DEVICE, &vmsid, "DNP Device Data Display...? (1 to N )  ") < 0)	return(E_ERR);
	if (CheckEOT() < 0) return(E_ERR);

    devid = vmsid - 1;

    printf("=>Select DEVICE = %d\n", devid +1);
    printf("=>Select TYPE  [0: ALL, 1: STATUS, 2: ANALOG] = ");
    type   = getDecWord();

    printf("=>Select Device (%2d)/Point Number (1 to Max 1024) = ", devid + 1);
    point = getDecWord() - 1;

    if(point < 0)   type = 0;

    /* -------------------------------------------- */
    /*  DISPLAY Type에 따라 실제 정보를 출력 ...  */
    /* -------------------------------------------- */
    if(type == 1)       dispDevStatus(devid, point);
    else if(type == 2)  dispDevAnalog(devid, point);
    else                dispDevice(devid);

    return(E_OK);
}

/*
 * ====================================================================
 *  PrintHistoryQueue()                                    2026-09-03
 * ====================================================================
 *  History-Queue(공유메모리의 살아있는 큐, hque)를 최신 이벤트부터
 *  화면에 출력한다. do_event() 전용 로직이라 이 파일 안에 static 으로
 *  둔다.
 *
 *  [2026-09-03 설계 변경] 원래는 이 함수를 lib_localLib.c(모든
 *  실행파일이 링크하는 libLocal.a)로 옮겨서 CLI-S2/histView.c 와
 *  공용으로 썼었다. 하지만:
 *   - deviceCFG[]/devPtBuf[]/esioCFG[] 는 공유메모리(shmPtr)에
 *     attach 해서 초기화해야만 의미 있는 값이 되는데(cliMain.c 의
 *     for(...) esioCFG[i] = &shmPtr->esioConfig[i]; 등 참고),
 *     histView 는 SDP 프로세스 없이도 동작해야 해서 애초에 공유메모리에
 *     attach 하지 않는다 - 즉 histView 안에서는 이 배열들이 항상
 *     NULL이라 이름 조회 코드가 죽은 코드였다.
 *   - 그런데도 lib_localLib.c 가 이 심볼들을 extern 참조하는 바람에,
 *     WDT/SCAN/HOST/LINK/SCU/SIM/ICCP/EXIT 등 이 함수와 무관한 모든
 *     실행파일까지 이 심볼에 얽히게 됐고, 그중 이 배열들을 안 쓰는
 *     ICCP(#if 0 // CHOIBC DELETE 로 꺼둔 상태)에서 실제로 링크 에러가
 *     났다.
 *  그래서 이 함수는 CLI-S2 전용(do_event() 가 쓰는 "이름까지 붙여
 *  보여주는" 버전)으로 다시 이 파일로 옮기고, histView 용은 이름 조회
 *  없이 독립적으로 동작하는 별도 함수(histView.c 의 PrintHistoryFile())
 *  로 분리했다. lib_localLib.c 는 이 기능과 관련해 더 이상 아무것도
 *  건드리지 않는다. (design/history-fram-to-file.md 참고)
 *
 *  hq   : 출력할 HISTORY_QUE (여기서는 항상 공유메모리의 hque)
 *  mode : 0=전체, 1=SOE, 2=CONTROL(제어), 3=COMM(통신) 필터
 *  32건 출력마다 키 입력을 기다리고 'q'를 누르면 중단한다(CLI 페이징).
 */
static void PrintHistoryQueue(HISTORY_QUE *hq, int mode)
{
    int             j;
    int             ch;
    int             id, ioid, point, state, hostid;
    word            front;
    SYSLOG_FORM     *log;
    struct tm       cTime, sTime;
    int             milisec1, milisec2;

    SDP_DEVICE      *dev;
    POINT_BUF       *ptBuf;
    ESIO_CONFIG     *esio;

    printf(">> Event Number   : front = %3d ... over =%d [MAX %d]\n",
           hq->front, hq->overlab, HISTORY_QUE_MAX);
    printf("\n");

    front = hq->front & HISTORY_QUE_MASK;

    for (j = 0; ; )
    {
        /* ------------------------------------ */
        /* 최신 이벤트부터 거꾸로 Display...    */
        /* ------------------------------------ */
        front = (front - 1) & HISTORY_QUE_MASK;

        if ((hq->overlab == 0) && (front == HISTORY_QUE_MASK))   break;
        else if (front == hq->front)                             break;

        log = (SYSLOG_FORM *) &hq->queue[front];

        id     = log->logid;
        ioid   = log->ioid;
        point  = log->point;
        state  = log->state;
        hostid = log->hostid;

        localtime_r(&log->logTime.tv_sec, &cTime);
        milisec1 = log->logTime.tv_usec / 1000;

        if ((id <= 0) || (id >= NUMBER_OF_EVENT))   continue;

        /* ------------------------------------------------ */
        /*  이벤트 코드에 따른 필터링 (mode)                */
        /* ------------------------------------------------ */
        if (mode == 1)                     /* SOE 이벤트만 */
        {
            if (id != ENT_SOE)   continue;
        }
        else if (mode == 2)                /* CONTROL 이벤트만 */
        {
            if ((id < ENT_ICCP_CNTR) || (id > ENT_LINK_CNTR))   continue;
        }
        else if (mode == 3)                /* COMM(통신) 이벤트만 */
        {
            if ((id < ENT_ICCP_ONLINE) || (id > ENT_VME_OFFLINE))   continue;
        }

        /* ------------------------------------------------ */
        /*  이벤트 코드별 상세 출력                          */
        /* ------------------------------------------------ */
        if (id == ENT_SOE)
        {
            localtime_r(&log->soeTime.tv_sec, &sTime);
            milisec2 = log->soeTime.tv_usec / 1000;

            if ((ioid <= 0) || (point <= 0))   continue;

            dev   = (SDP_DEVICE *) deviceCFG[ioid - 1];
            ptBuf = (POINT_BUF *) &dev->diPtBuf[point - 1];

            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] SOE:%04d/%02d/%02d-%02d:%02d:%02d-%03d %s \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon + 1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid,
                sTime.tm_year + 1900, sTime.tm_mon + 1, sTime.tm_mday, sTime.tm_hour, sTime.tm_min, sTime.tm_sec, milisec2, ptBuf->ptNameStr);
        }
        else if (id == ENT_DEVICE_SOE)
        {
            localtime_r(&log->soeTime.tv_sec, &sTime);
            milisec2 = log->soeTime.tv_usec / 1000;

            if ((ioid < 0) || (point < 0))   continue;

            ptBuf = (POINT_BUF *) devPtBuf[ioid];

            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] DEV:%04d/%02d/%02d-%02d:%02d:%02d-%03d %s \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon + 1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid,
                sTime.tm_year + 1900, sTime.tm_mon + 1, sTime.tm_mday, sTime.tm_hour, sTime.tm_min, sTime.tm_sec, milisec2, ptBuf->ptNameStr);
        }
        else if ((id >= ENT_ICCP_CNTR) && (id <= ENT_LINK_CNTR))     /* 원격제어 이벤트... */
        {
            localtime_r(&log->soeTime.tv_sec, &sTime);
            milisec2 = log->soeTime.tv_usec / 1000;

            if ((ioid <= 0) || (point <= 0))   continue;

            dev   = (SDP_DEVICE *) deviceCFG[ioid - 1];
            ptBuf = (POINT_BUF *) &dev->doPtBuf[point - 1];
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] CNT:%04d/%02d/%02d-%02d:%02d:%02d-%03d %s \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon + 1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid,
                sTime.tm_year + 1900, sTime.tm_mon + 1, sTime.tm_mday, sTime.tm_hour, sTime.tm_min, sTime.tm_sec, milisec2, ptBuf->ptNameStr);
        }
        else if ((id >= ENT_VME_ONLINE) && (id <= ENT_VME_UNINSTALL))     /* VME 관련 이벤트... */
        {
            if ((ioid < 0) || (point < 0))   continue;

            esio = (ESIO_CONFIG *) esioCFG[ioid - 1];
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] VME: %s \n", front + 1, cTime.tm_year + 1900, cTime.tm_mon + 1,
                cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid, esio->esioNameStr);
        }
        else if ((id >= ENT_ESIO_ONLINE) && (id <= ENT_ESIO_OFFLINE))     /* ESIO 채널 상태 이벤트... */
        {
            if ((ioid < 0) || (point < 0))   continue;

            esio = (ESIO_CONFIG *) esioCFG[ioid - 1];
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] COM: %s \n", front + 1, cTime.tm_year + 1900, cTime.tm_mon + 1,
                cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid, esio->esioNameStr);
        }
        else if ((id == ENT_DEV_ONLINE) || (id == ENT_DEV_OFFLINE))     /* 종속기기 On/Offline 이벤트... */
        {
            localtime_r(&log->soeTime.tv_sec, &sTime);
            milisec2 = log->soeTime.tv_usec / 1000;

            if ((ioid < 0) || (point < 0))   continue;

            dev = (SDP_DEVICE *) deviceCFG[ioid - 1];

            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] COM:%04d/%02d/%02d-%02d:%02d:%02d-%03d %s \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon + 1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid,
                sTime.tm_year + 1900, sTime.tm_mon + 1, sTime.tm_mday, sTime.tm_hour, sTime.tm_min, sTime.tm_sec, milisec2, dev->devNameStr);
        }
        else                                                            /* 그 외 시스템 이벤트... */
        {
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon + 1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid);
        }

        if (((++j) % 32) == 0)
        {
            j = 0; ch = 0;
            ch = getchar();
            if (ch == 'q')   break;
        }
    }

    printf("\n [ EVENT-Q ] end ---------------------------------------------\n");
}

//
// ==============================================================
// 모듈:	do_event()
// SDP 통합 이벤트 History 정보 조회
// ==============================================================
//
int do_event(int argc, char *argv[])
{
    int mode;

	if (CheckEOT() < 0)	return(E_ERR);

    printf("\n*** CU-HISTORY Data Display ***\n");
    printf(">> SDP Controller : %s \n", TARGET_NAME);
    printf(">> Manufacture    : ACE Control Co.,Ltd \n");
    printf(">> Model Name     : %s\n", VERSION_STRING);
    printf(">> Update Date    : %s\n", DATE_STRING);

    printf("\n--------------------------------------------------------------\n");
    printf("[SELECT Info :  [1] SOE, [2] CONTROL, [3] COMM ... ");

    mode = getDecWord();
    if((mode != 1) && (mode != 2) && (mode != 3))   mode = 0;
    printf("--------------------------------------------------------------\n");

    PrintHistoryQueue(hque, mode);

    return(E_OK);
}
