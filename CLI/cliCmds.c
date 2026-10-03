/* for TEST... */


//==============================================================================
//
// 파일:    cliCmds.c
//
//==============================================================================

//
// 헤더 파일
//
#include	"localLib.h"
#include    "external.h"
//#include    "eventmsg.h"
#include    "cli.h"

//#include    "cli.h"
//#include    "external.h"
//#include    "eventmsg.h"

//
// External Variables / Functions
//
extern  int CheckEOT(void);
extern  int GetNumber(int low, int high, int *value, char *message);
extern  int GetToken(void);
extern  int  parseOnOff(const char *s);
extern  int  waitLogbitAck(short seq);
extern  void postLogbitSet(const char *layer, const char *bit, int value);
extern  void postLogbitClearAll(void);
extern  void PrnLine(char mode);

extern  void dispDevStatus(int devid, int point);
extern  void dispDevAnalog(int devid, int point);
extern  int  dispDevice(int devid);
extern  void dispHostData_DNP(int hostid);

extern  void    display_ICCP_INFO(int pointType, int pointInx);


extern  int	termExec;
extern  char	token[TOKEN_LENGTH];
extern  char	command[CMD_BUF_LEN];
extern  char	*cmdPtr;
extern  SHM_MEMORY	*shmPtr;

//
// Function Type Definition
//
void	CmdClear();
void	CmdHelp(), CmdProcess(), CmdQuit(), CmdExit();
void    CmdSETRTC();

void    CmdOffDebug(void);
void    CmdDispVERSION(void);

void    CmdEvent(void);
void    CmdSIMDebug(void);
void    CmdSCUDebug(void);
void    CmdLINKDebug(void);
void    CmdDispRelayCfg(void);
void    CmdDispMPUCfg(void);
void    CmdDispESIOCfg(void);
void    CmdDispHOSTCfg(void);
void    CmdDispPORT(void);
void    disp_HarrisPort(int port);

void    CmdDispMODBUSCfg(void);
void    CmdDispPOINTConfig(void);

void    CmdCHKSUM(void);
void    CmdESIOComDebug(void);
void    CmdESIOStatus(void);
void    CmdWDTDebug(void);
void    CmdCALDebug(void);

void    CmdLineDebug(void); // 2026-02-04 오후 1:24:11 hkkim
void    CmdHostDebug(void);
void    CmdHexDebug(void);
void    CmdDispDevice(void);

void    CmdDispICCPCfg(void);
void    CmdDispICCPData(void);
void    CmdMMSDebug(void);
void    CmdICCPDebug(void);

void    CmdSOEDebug(void);
void    CmdMPUDebug(void);

void    CmdDevDebug(void);
void    CmdMessageDebug(void);

void    CmdDispDDIConfig(void);     // DNP 구성정보
void    CmdDispDAIConfig(void);     // DNP 구성정보
void    CmdDispDDOConfig(void);     // DNP 구성정보

void    CmdDispDEVConfig(void);
void    CmdDispCALConfig(void);

void    CmdTRIPControl(void);
void    CmdCLOSEControl(void);
void    CmdSETAnalog(void);
void    CmdSETStatus(void);
void    CmdSETDevice(void);

void    CmdCHANGEcpu(void);
void    CmdTEST_DOM(void);
void    CmdTEST_Device(void);
void    CmdGET_Clock(void);
void    CmdLOG_ON(void);

void    CmdNTPDebug(void);
void    CmdCHECKDebug(void);

void    CmdDispDHostData(void);
void    CmdDispICCPInfo(void);
void	CmdTESTSET(void);
void	CmdTESTRESET(void);


CMD_ENTRY   cmdTable[MAX_COMMAND] = {
    {"    ",        CmdHelp,	        "   ",	"----------------------------"},
	{"help",        CmdHelp,	        "help  <ENTER>",	    "Command-List Display "},
	{"process",	    CmdProcess,	        "process <ENTER>",	    "PROCESSOR Status Display"},
	{"clear",	    CmdClear,	        "clear <ENTER>",	    "Clear EVENT-Queue."},
	{"setrtc",	    CmdSETRTC,          "setrtc <ENTER>",       "SET CU-Time Sync"},	
	{"event",	    CmdEvent,	        "event <ENTER>",        "CU History-Queue Display"},	
	{"off",	        CmdOffDebug,	    "OFF <ENTER>",          "Monitoring Debug Flag OFF (also clears ICCP mi/mvl FLOW,CFG bits)."},	
	{"version",	    CmdDispVERSION,	    "version <ENTER>",      "DISPLAY CU-Version"},	
	{"comlogon",	CmdLOG_ON,	        "comlogon <ENTER>",     "CONSOLE Message LOGGING..."},
	{"check",	    CmdCHECKDebug,	    "check <ENTER>",        "MPU Status Message Debug..."},		
	{"quit",	    CmdQuit,	        "quit <ENTER>",	        "CONSOLE-TASK quit"},
	{"exit",	    CmdExit,	        "exit <ENTER>",         "***> VME-CU Exit<***"},
    {"-------",     CmdHelp,	        "   ",	"----------------------------"},
    {"iccp",	    CmdICCPDebug,	    "iccp  <ENTER>",	    "ICCP Communication Debug ON"}, 
	{"mms",	        CmdMMSDebug,	    "mms [<mi|mvl|mms|acse> <FLOW|CFG> <on|off>]",   "ICCP-MMS Comm Debug ON / layer FLOW,CFG bit control"},
    {"iccpdata",	CmdDispICCPData,    "iccpdata <ENTER>",	    "ICCP Configuration Display"}, 
    {"iccpcfg",	    CmdDispICCPCfg,     "iccpcfg <ENTER>",	    "ICCP Configuration Display"}, 
    {"iccpinfo",	CmdDispICCPInfo,    "iccpinfo <ENTER>",	    "ICCP Point Display"}, 
    {"-------",     CmdHelp,	        "   ",	"----------------------------"},
    {"hd",	        CmdHostDebug,	    "hd # <ENTER>",	        "HOST Communication Debug ON"}, 
    {"hex",	        CmdHexDebug,	    "hex # <ENTER>",	    "HOST Message Debug ON"}, 
    {"soe",	        CmdSOEDebug,	    "soe <ENTER>",	        "[SOE] Monitoring Debug ON"}, 
    {"esio",	    CmdESIOComDebug,    "esio #<ENTER>",        "ESIO# Comm Debug"},
    {"dev",	        CmdDevDebug,	    "dev # <ENTER>",	    "Device Communication Debug ON"},  
	{"msg",	        CmdMessageDebug,	"msg <ENTER>",	        "Device Message Debug ON"},  
    {"sim",	        CmdSIMDebug,        "sim <ENTER>",          "SIMULATOR Comm Debug"},
    {"scu",	        CmdSCUDebug,        "scu <ENTER>",          "SCU Comm Debug"},
    {"link",	    CmdLINKDebug,       "link <ENTER>",         "LINK Comm Debug"},
    {"wdt",	        CmdWDTDebug,        "wdt <ENTER>",          "WDT Message Debug"},
    {"ld",	        CmdLineDebug,	    "ld # <ENTER>",	        "Comm Line Debug ON"},     
    //{"mpu",	        CmdMPUDebug,        "mpu <ENTER>",          "MPU Message Debug"},
    {"cal",	        CmdCALDebug,        "cal <ENTER>",          "CAL Point Message Debug"},
    {"ntp",	        CmdNTPDebug,        "ntp <ENTER>",          "NTP Message Debug"},
    {"device",	    CmdDispDevice,	    "device # <ENTER>",     "DEVICE Report-Info Display"},	 
    {"esiosts",	    CmdESIOStatus,      "esiosts <ENTER>",      "Display ESIO# Status"},
    {"-------",     CmdHelp,	        "   ",	"----------------------------"},
    {"dnphost",	    CmdDispDHostData,   "dnphost # <ENTER>",	"DNP-HOST Report-Info Display"},   
    {"dnpdi",	    CmdDispDDIConfig,	"dnpdi <ENTER>",        "DNP-HOST DI Config Display"},	
    {"dnpdo",	    CmdDispDDOConfig,	"dnpdo <ENTER>",        "DNP-HOST DO Config Display"},
    {"dnpai",	    CmdDispDAIConfig,	"dnpai <ENTER>",        "DNP-HOST AI Config Display"},	
    {"-------",     CmdHelp,	        "   ",	"----------------------------"},
    {"mpucfg",	    CmdDispMPUCfg,      "mpucfg <ENTER>",	    "MPU Configuration Display"}, 
    {"esiocfg",	    CmdDispESIOCfg,     "esiocfg <ENTER>",	    "ESIO Configuration Display"}, 
    {"hostcfg",	    CmdDispHOSTCfg,     "hostcfg <ENTER>",	    "HOST Configuration Display"}, 
    
    {"portcfg",	    CmdDispPORT,        "portcfg <ENTER>",	    "HARRIS-PORT Configuration Display"}, 
    {"pointcfg",    CmdDispPOINTConfig,	"pointcfg <ENTER>",     "POINT Configuration Display"},
    {"modcfg",	    CmdDispMODBUSCfg,   "modcfg <ENTER>",	    "DEVICE MODBUS-Config Display"},   
    {"devcfg",	    CmdDispDEVConfig,	"devcfg <ENTER>",       "DEV-Point Config Display"},
    {"calcfg",	    CmdDispCALConfig,	"calcfg <ENTER>",       "CAL-Point Config Display"},
    {"relay",	    CmdDispRelayCfg,    "relay <ENTER>",	    "DEVICE Configuration Display"},
    {"chksum",	    CmdCHKSUM,          "chksum <ENTER>",	    "LINK DB Checksum "},  
    {"-------",     CmdHelp,	        "   ",	"----------------------------"},
    {"trip",	    CmdTRIPControl,	    "trip  <ENTER>",        "[TRIP]   Control TEST."},	
    {"close",	    CmdCLOSEControl,	"close <ENTER>",        "[CLOSE]  Control TEST."},	
    {"analog",	    CmdSETAnalog,	    "analog <ENTER>",       "[ANALOG] Test Data Input"},	
    {"status",	    CmdSETStatus,	    "status <ENTER>",       "[STATUS] Test Data Input"},
    {"online",	    CmdSETDevice,	    "online <ENTER>",       "[DEVICE] Test Data Input"},
    {"change",	    CmdCHANGEcpu,	    "change <ENTER>",       "[CHANGE] Test CPU Change"},
    {"testdo",	    CmdTEST_DOM,	    "testdo <ENTER>",       "[TEST-DO] Control TEST."},		 
    {"devtest",	    CmdTEST_Device,	    "devtest <ENTER>",      "[TEST-DO] Control TEST."},		 
    {"getclock",    CmdGET_Clock,	    "getclock <ENTER>",     "[GET CLOCK] Control TEST."},	
    {"testset",     CmdTESTSET,	    	"testset <ENTER>",      "[TEST-WDT] Control TEST."},	
    {"testreset",   CmdTESTRESET,	   	"testreset <ENTER>",    "[TEST-WDT] Control TEST."},		 
    {"-------",     CmdHelp,	        "   ",	"----------------------------"},
   
    		
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
        //printf("%c", ch);
        if(isdigit(ch))
            ret_val = ret_val * 10 + (ch - '0');
    }
    //printf("\n\r");
    return(ret_val);
}

void CmdTESTSET(void)
{
	int		vmsid;

	// 명령 형식을 점검한다.

	printf("==> CPU Master TEST.....\n");
	opr->testWDTFlag = 1;
    
}

void CmdTESTRESET(void)
{
	int		vmsid;

	// 명령 형식을 점검한다.

	printf("==> CPU Slave TEST.....\n");
	opr->testWDTFlag = 2;
    
}

void CmdDispPORT(void)
{
	int		vmsid;

	// 명령 형식을 점검한다.
	if (GetNumber(0, MAX_HARRIS_PORT, &vmsid, "HARRIS PORT Config...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    disp_HarrisPort(vmsid);
    
}

void CmdDispDHostData(void)
{
	int		vmsid;

	// 명령 형식을 점검한다.
	if (GetNumber(1, MAX_HOST, &vmsid, "HOST DNP Data Display...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    vmsid = vmsid - 1;
    dispHostData_DNP(vmsid);
    
}

//
// 모듈:	CmdDevDebug()
//
void CmdDevDebug(void)
{
	int		vmsid;

	// 명령 형식을 점검한다.
	if (GetNumber(0, MAX_DEVICE, &vmsid, "DNP Device Comm Debug...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    if(vmsid == 0)  opr->dnpDebug = MAX_DEVICE;
    else            opr->dnpDebug = vmsid - 1;
    
    printf("[*] DNP Device (%2d) Comm Debug... ...!\n", vmsid);
    
}

//
// 모듈:	CmdMessageDebug()
//
void CmdMessageDebug(void)
{
	int		vmsid;

	// 명령 형식을 점검한다.
	if (GetNumber(1, MAX_DEVICE, &vmsid, "DNP Device Message Debug...?  ") < 0)	return;
	if (CheckEOT() < 0) return;
	
    opr->msgDebug = vmsid - 1;
    printf("[*] DNP Device (%2d) Message Debug... ...!\n", vmsid);
    
}

/*
*
*/
void CmdDispICCPInfo(void)
{
	int		type, point;

#if 0
#define	SDP_POINT_NULL      0	// 미지정
#define	SDP_POINT_TYPE_SDI	1	// SCADA Digital Input
#define	SDP_POINT_TYPE_SDO	2	// SCADA Digital Output
#define	SDP_POINT_TYPE_SAI	3	// SCADA Analog  Input

#define	SDP_POINT_TYPE_DDI	4	// 원격진단 : Diagnosis Digital Input
#define	SDP_POINT_TYPE_DAI	5	// 원격진단 : Diagnosis Analog  Input
#define	SDP_POINT_TYPE_QDI	6	// 전력품질 : Quality Digital Input
#define	SDP_POINT_TYPE_QAI	7	// 전력품질 : Quality Analog  Input
#define	SDP_POINT_TYPE_TDI	8	// 고장점   : Train Digital Input
#define	SDP_POINT_TYPE_TAI	9	// 고장점   : Train Analog  Input
#define	SDP_POINT_TYPE_DEV	10	// 장치     : Device (가상포인트)
#endif
    
    printf("\n[*] ICCP POINT-DATA Display.... !\n");
    printf("=> Select TYPE [1]SDI, [2]SDO, [3]SAI, [4]DDI , [5]DAI, [6]QDI, [7]QAI, [8]TDI, [9]TAI, [10]DEV ...  ");

    type = getDecWord() & 0x3f;

    printf("=> Select POINT [1...] ");
    point = getDecWord();
    
    display_ICCP_INFO(type, point);
    
}

//
// 모듈:	CmdLOG_ON()
//  - Console 메세지를 화일로 저장...
//
void CmdLOG_ON(void)
{
    char buffer[256];
    int     length;
   
	if (CheckEOT() < 0) return;

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
          
}

//
// 모듈:	CPU CmdGET_Clock()
//
void CmdGET_Clock(void)
{
    //int     i,j;
    //int     devid, point;
    //int     index;
    //word    chksum;
    //DNP_DEVICE  *dnp; 
    //SCAN_CONFIG     *scan;
    
	if (CheckEOT() < 0) return;

    opr->getClockNTP = SET;    
    printf("> GET NTP-Server .... TIME req...! \n");       
}


void CmdSETRTC(void)
{

    int  year, month, day, week, hour, min, sec;

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
        
        opr->rtcUpdateICCP = SET;		// ICCP-HOST Time-Sync 정보 
        opr->rtcUpdateFlag = SET;
        
        printf(">> CU Time SET : %04d/%02d/%02d-%d-%02d:%02d:%02d\n", year, month, day, week, hour, min, sec);	        
        printf("*clock modified\n");
        
    }
    else
    {
        printf("*clock cancel\n");
    }

}


//// 모듈:	제어모듈 TRIP/CLOSE()
//
void CmdTEST_DOM(void)
{
    int     i,j;
    int     devid, point, tcf;
    //int     localIndex;
    //int     index;
    //word    chksum;
    SDP_DEVICE      *dev; 
    SCAN_CONFIG     *scan;
    POINT_BUF       *ptBuf;
    
	if (CheckEOT() < 0) return;

    printf("[*] Test DOM Control.... !\n");
    printf("1. Control Device# [1..%d] : ", MAX_DEVICE);

    devid = getDecWord() & 0x3f;
    point = 0;    
    tcf   = TRIP_CONTROL;
    
    if((devid < 1) || (devid > MAX_DEVICE))
    {
        printf("*** Invalid Test DOM :  %2d \n", devid);
        return;
    }
    
    //printf("==> Control Device-%02d, Point-%02d ... TRIP Control...!\n", devid, point);
    
    devid = devid - 1;

CONTROL_NEXT:        
    /* ---------------------------------------- */
    /* SCAN Task 제어정보 Setting ...           */
    /* ---------------------------------------- */
    printf("\n\n------------------------------------[%4d/%2d/%2d %02d:%02d:%02d]\n", 
       	((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
    for(i=0; i< MAX_SCAN_PORT; i++)
    {
        scan = (SCAN_CONFIG *) scanCFG[i];
        if(scan->scanIndex <= 0)   continue;
        
        for(j=0; j< scan->scanIndex; j++)
        {
            /* 제어대상 모듈을 search... */
            if(scan->scanDevice[j] == devid)
            {
                dev = (SDP_DEVICE *) deviceCFG[devid];
                ptBuf = (POINT_BUF *) &dev->doPtBuf[point];
                
                dev->cntPoint = point;
                dev->cntTCF   = tcf;   /* TRIP */
                dev->cntrType = ptBuf->ptConfig;         // 제어속성 : Pulse, Latch 제어
                dev->selectReq  = SET;
                
                opr->rcvONtime = 500;
                //opr->cntrDevId[i]   = devid;
                //opr->cntrComTick[i] = 0;
                //opr->cntrFlag[i]    = SET;

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
        				//break;
        				//return;
        			}	
        		}
            }                            
        }
      	
    }
    
    pause(1000);
    pause(1000);
    //pause(1000);
        
    goto CONTROL_NEXT;
        
    printf(">> Control DOM(%d) .... Test End...2 !\n", devid+1);
}

/* 
*   TRIP-제어시험
*/
void CmdTRIPControl(void)
{
    int     devid, point;
    char    buffer[256];
    struct timeval  ctime;
    
	if (CheckEOT() < 0) return;

    printf("[*] Test TRIP Control.... !\n");
    printf("1. Control Device# [1..%d] : ", MAX_DEVICE);
    devid = getDecWord() & 0x3f;
    printf("2. Control Point # [1..%d] : ", MAX_DEV_DO_POINT);
    point = getDecWord();    
    
    if((devid < 1) || (devid > MAX_DEVICE) || (point < 1) || (point > MAX_DEV_DO_POINT))
    {
        printf("*** Invalid Control... :  %2d %2d\n", devid, point);
        return;
    }
    
    printf("==> Control Device-%02d, Point-%02d ... TRIP Control...[%4d/%2d/%2d %02d:%02d:%02d]\n",  devid, point,
       	((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
    

    /* ------------------------------------ */
    /* ESIO 제어정보 연계...                */
    /* ------------------------------------ */
    gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
    logEvent_MPU(shmPtr, ENT_USER_CNTR, devid, point, TRIP_INFO, opr->cpuMode, &ctime);
    
        /* -------------------------------- */
    /* LOG File 저장                    */
    /* -------------------------------- */
    sprintf(buffer, "<-- USER> Control INFO : Dev=%d, PT=%d, TCF[TRIP=1, CLOSE=2]= %2x... ", devid, point, TRIP_INFO);
    LogFile_MPU (shmPtr, ENT_USER_CNTR, buffer, strlen(buffer));       
    
    controlInfo_MPU( shmPtr, devid, point, TRIP_INFO, PASS_USER_CNTR, &ctime);

}

/* 
*   CLOSE-제어시험
*/
void CmdCLOSEControl(void)
{
    int     devid, point;
    char    buffer[256];
    struct timeval  ctime;
    
	if (CheckEOT() < 0) return;

    printf("[*] Test CLOSE Control.... !\n");
    printf("1. Control Device# [1..%d] : ", MAX_DEVICE);
    devid = getDecWord() & 0x3f;
    printf("2. Control Point # [1..%d] : ", MAX_DEV_DO_POINT);
    point = getDecWord();    
    
    if((devid < 1) || (devid > MAX_DEVICE) || (point < 1) || (point > MAX_DEV_DO_POINT))
    {
        printf("*** Invalid Control... :  %2d %2d\n", devid, point);
        return;
    }
    
    printf("==> Control Device-%02d, Point-%02d ... CLOSE Control...[%4d/%2d/%2d %02d:%02d:%02d]\n",  devid, point,
       	((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);

    /* ------------------------------------ */
    /* ESIO 제어정보 연계...                */
    /* ------------------------------------ */
    gettimeofday(&ctime, NULL);  //제어정보 갱신시간 추출...timeval  Form    
    logEvent_MPU(shmPtr, ENT_USER_CNTR, devid, point, CLOSE_INFO, opr->cpuMode, &ctime);

    /* -------------------------------- */
    /* LOG File 저장                    */
    /* -------------------------------- */
    sprintf(buffer, "<-- USER> Control INFO : Dev=%d, PT=%d, TCF[TRIP=1, CLOSE=2]= %2x... ", devid, point, CLOSE_INFO);
    LogFile_MPU (shmPtr, ENT_USER_CNTR, buffer, strlen(buffer));            
        
        
    controlInfo_MPU( shmPtr, devid, point, CLOSE_INFO, PASS_USER_CNTR, &ctime);


}

//
// 모듈:	CPU Change()
//
void CmdCHANGEcpu(void)
{
    //int     i,j;
    //int     devid, point;
    //int     index;
    //word    chksum;
    //DNP_DEVICE  *dnp; 
    //SCAN_CONFIG     *scan;
    char    buffer[256];
    
	if (CheckEOT() < 0) return;

    if((opr->dualCpuSts == SET) && (scuCfg->remoteMode == AUTO_MODE))
    {
        opr->cpuChange = SET; 
        printf("TEST>> CPU Change...!\n");
        
        logEvent_MPU(shmPtr, ENT_CHANGE_CPU, CPU_CHG_USER, 0, 0, opr->cpuMode, NULL);
        
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "<-- USER> CPU CHANGE cmd ... " );
        LogFile_MPU (shmPtr, ENT_CHANGE_CPU, buffer, strlen(buffer));      
    }
    else
    {
        printf("TEST>> *** Invalid CPU Change...!\n");
    }      
    
}

void CmdSETAnalog(void)
{
    //int     i,j;
    int     devid, point;
    float   aiData;
    

	if (CheckEOT() < 0) return;

    printf("[*] Test ANALOG Point Update.... !\n");
    printf("1. Input Device# [1..%d] : ", MAX_DEVICE);
    devid = getDecWord() & 0x3f;
    printf("2. Input Point # [1..80] : ");
    point = getDecWord();    
    printf("3. Input Analog Data     : ");
    aiData = getDecWord() * 1.0 ;
     
    if((devid < 1) || (devid > MAX_DEVICE) || (point < 1) || (point > MAX_DEV_DI_POINT))
    {
        //printf("*** Invalid Status Information ... :  %2d %2d\n", devid, point);
        return;
    }

    printf("\n----------------------------------------------------->>\n");
    printf("==> ANALOG Info Device-%02d, Point-%02d ... Data = %4.2f !\n", devid, point, aiData);
    printf("----------------------------------------------------->>\n");
    
    opr->testDevid = devid - 1;
    opr->testPoint = point - 1;

    opr->testAIData = aiData;


    opr->testAnalogFLAG = SET;
    
 //   printf("out CmdSETAnalog\r\n");                    
}

/*
* SDP 장치 포인트 수동입력....
*/
void CmdTEST_Device(void)
{
    int     devIndex, state, point;
    int     devNo;
    
    POINT_BUF   *devPoint, *diPoint;
    SDP_DEVICE  *dev;
	
	if (CheckEOT() < 0) return;

    printf("[*] Test DEVICE Point Update.... !\n");
    printf("1. SDP-Device Point # [1..%d] : ", MAX_DEV_POINT);
    devIndex = getDecWord();
    printf("2. Input Status [0 / 1] : ");
    state = getDecWord() & 0x01;    
    
    if((devIndex < 1) || (devIndex > MAX_DEV_POINT))
    {
        printf("*** Invalid DEVICE-Point ...%2d \n", devIndex);
        return;
    }
    
    opr->devTestFlag = SET;
    
    /* 장치 포인트 정보 참조 */
    devPoint = (POINT_BUF *) devPtBuf[devIndex - 1];
    if(devPoint->config == 0)
    {
        printf("*** No Define... DEVICE-Point = %d\n", devIndex);
        return ;
    }
          
    devNo = devPoint->devNo;        // 해당 계전기 번호, [1,2...64]
    point = devPoint->devPt;        // 해당 포인트 번호, [1,2..1024];
    devPoint->status = state;
    
    /* 계전기 포인트 참조 */
    if((devNo > 0) && (point > 0))
    {
        dev = (SDP_DEVICE *) deviceCFG[devNo - 1];
        diPoint = (POINT_BUF *) &dev->diPtBuf[point - 1];
        diPoint->status = state;
    
        printf(">> DEVICE-POINT (%3d) : %s .... status = %d \n",   devIndex, diPoint->ptNameStr, diPoint->status);
    }
}


void CmdSETStatus(void)
{
    //int     i,j;
    int     devid, point, state;
    
	if (CheckEOT() < 0) return;

    printf("[*] Test STATUS Point Update.... !\n");
    printf("1. Input Device# [1..%d] : ", MAX_DEVICE);
    devid = getDecWord() & 0x3f;
    printf("2. Input Point # [1..80] : ");
    point = getDecWord();    
    printf("3. Input Status# [0...1] : ");
    state = getDecWord() & 0x01;    
    
    if((devid < 1) || (devid > MAX_DEVICE) || (point < 1) || (point > MAX_DEV_DI_POINT))
    {
        //printf("*** Invalid Status Information ... :  %2d %2d\n", devid, point);
        return;
    }

    printf("\n----------------------------------------------------->>\n");
    printf("==> Status Info Device-%02d, Point-%02d ... state = %d !\n", devid, point, state);
    printf("----------------------------------------------------->>\n");
    
    opr->testDevid = devid - 1;
    opr->testPoint = point - 1;
    opr->testState = state;

    opr->testStatusFLAG = SET;
                        
}


void CmdSETDevice(void)
{
    //int     i,j;
    int     devid, state;
    
	if (CheckEOT() < 0) return;

    printf("[*] Test DEVICE Online/Offline Update.... !\n");
    printf("1. Input Device# [1..%d] : ", MAX_DEVICE);
    devid = getDecWord() & 0x3f;
    
    printf("1. Input Status# [0] offline, [1] Online : ");
    state = getDecWord() & 0x01;    
    
    if((devid < 1) || (devid > MAX_DEVICE))
    {
        //printf("*** Invalid Status Information ... :  %2d %2d\n", devid, point);
        return;
    }

    printf("\n----------------------------------------------------->>\n");
    printf("==> Device Offline/Online ... devid= %2d, state = %d !\n", devid, state);
    printf("----------------------------------------------------->>\n");
    
    opr->testDevid = devid - 1;
    opr->testPoint = 0;
    opr->testState = state;

    opr->testDeviceFLAG = SET;
                        
}


//
// 모듈:	CmdClear()
//
void CmdClear(void)
{
    
    word    chksum;
	if (CheckEOT() < 0) return;

    /* -------------------------------------------- */
    /* 내부 History & Event Queue 내용 Clear...     */
    /* -------------------------------------------- */
    printf("[*] HISTORY Queue Cleared.... !\n");
    bzero8248((byte *) hque, sizeof(HISTORY_QUE));
    chksum = gensum((byte *) hque, sizeof(HISTORY_QUE) - 2);

    hque->chksum = chksum;
    hque->clear  = 255;  
    
}


void CmdHostDebug(void)
{
	int		vmsid;

	// 명령 형식을 점검한다.
	if (GetNumber(1, MAX_HOST, &vmsid, "HOST Comm Debug...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    opr->hostDebug = vmsid - 1;
    printf("[*] HOST (%2d) Comm Debug... ...!\n", vmsid);
}
void CmdLineDebug(void)
{
	int		vmsid;

	// 명령 형식을 점검한다.
	if (GetNumber(1, MAX_HOST, &vmsid, "Comm Line Debug...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    opr->lineDebug = vmsid - 1;
    printf("[*] Comm[%d] Line Debug On ...!\n", vmsid);
}

/*
*
*/
void CmdSOEDebug(void)
{
	//int		vmsid, value;

	// 명령 형식을 점검한다.
	//if (GetNumber(1, 32, &vmsid, "Display RTU Information...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    opr->soeDebug = SET;
    printf("[*] SOE Queue ... Message Debug ON...!\n");    
}


/*
*
*/
void CmdNTPDebug(void)
{
	//int		vmsid, value;

	// 명령 형식을 점검한다.
	//if (GetNumber(1, 32, &vmsid, "Display RTU Information...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    opr->ntpDebug = SET;
    printf("[*] NTP ... Message Debug ON...!\n");    
}

/*
*
*/
void CmdCHECKDebug(void)
{
	//int		vmsid, value;

	// 명령 형식을 점검한다.
	//if (GetNumber(1, 32, &vmsid, "Display RTU Information...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    opr->checkDebug = SET;
    printf("[*] CHECK ... Message Debug ON...!\n");    
}

/*
*
*/
void CmdMPUDebug(void)
{
	//int		vmsid, value;

	// 명령 형식을 점검한다.
	//if (GetNumber(1, 32, &vmsid, "Display RTU Information...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    opr->mpuDebug = SET;
    printf("[*] MPU... RUN Message Debug ON...!\n");    
}


void CmdHexDebug(void)
{
	int		vmsid;

	// 명령 형식을 점검한다.
	if (GetNumber(1, MAX_HOST, &vmsid, "HOST Message Debug...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    opr->hexDebug = vmsid - 1;
    printf("[*] HOST (%2d) DNP-Message Debug... ...!\n", vmsid);
}


/*
*  esio #0 = SIO, esio #1-전력감시, esio #2-원격진단, esio #3- 전력품질, esio #4-61850, esio #5-RTU
*/
void CmdESIOComDebug(void)
{
	int		esioid;

	// 명령 형식을 점검한다.
	if (GetNumber(1, MAX_ESIO, &esioid, "ESIO# Comm Debug...?  ") < 0)	return;
	    
	if (CheckEOT() < 0) return;

    opr->esioDebug = esioid;
    printf("[*] ESIO (%2d) Comm Debug... ...!\n", esioid);
}


void CmdCALDebug(void)
{
	int		esioid;

	// 명령 형식을 점검한다.
	if (GetNumber(1, MAX_CAL_POINT, &esioid, "CAL Point Debug...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    opr->calDebug = esioid - 1;
    printf("[*] CAL POINT Message Debug...%d !\n", esioid);
}

//
// 모듈:	CmdESIOStatus()
//
void CmdESIOStatus(void)
{
	if (CheckEOT() < 0) return;
    printf("[*] Display ESIO# Status ...!\n");
    opr->esioStsDebug = 1;
    
}

//
// 모듈:	CmdWDTDebug()
//
void CmdWDTDebug(void)
{
	if (CheckEOT() < 0) return;
    printf("[*] WDT Message Debug ON ...!\n");
    opr->wdtDebug = 1;
    
}

//
// 모듈:	CmdICCPDebug()
//
void CmdICCPDebug(void)
{
	if (CheckEOT() < 0) return;
    printf("[*] ICCP HOST Comm Debug ON ...!\n");
    opr->iccpDebug = 1;
    
}

/* ----------------------------------------------------------------------- */
/* 2026-10-02 : CLI <-> ICCP layer FLOW/CFG bit control helpers            */
/* See design/20261002_iccp_log_control_S2.md for the full design.        */
/* ----------------------------------------------------------------------- */

//
// Module: parseOnOff()
//   "on" -> 1, "off" -> 0, anything else -> -1
//
int parseOnOff(const char *s)
{
    if (strcmp(s, "on") == 0)  return 1;
    if (strcmp(s, "off") == 0) return 0;
    return -1;
}

//
// Module: waitLogbitAck()
//   Poll opr->logbitCtl.ack until it matches seq, or give up after ~1 second.
//   Returns 1 if acked, 0 on timeout.
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
// Module: postLogbitSet()
//   Ask the ICCP process (via shared memory) to turn a layer's FLOW/CFG
//   bit on or off, and print the result.
//
void postLogbitSet(const char *layer, const char *bit, int value)
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
        printf("[*] ERR: no response from ICCP (timeout)\n");
        return;
    }

    switch (opr->logbitCtl.result)
    {
    case 0:
        printf("[*] %s %s set to %s\n", layer, bit, value ? "ON" : "OFF");
        break;
    case -1:
        printf("[*] ERR: unknown layer '%s'\n", layer);
        break;
    case -2:
        printf("[*] ERR: layer '%s' has no '%s' bit\n", layer, bit);
        break;
    default:
        printf("[*] ERR: unexpected result (%d)\n", opr->logbitCtl.result);
        break;
    }
}

//
// Module: postLogbitClearAll()
//   Ask the ICCP process to clear every layer's FLOW/CFG bit (used by the
//   "off" command). Best-effort: a timeout is reported but not fatal, since
//   opr->*Debug flags are already cleared by the caller.
//
void postLogbitClearAll(void)
{
    short seq;

    opr->logbitCtl.cmd = LOGBIT_CMD_CLEAR_ALL;
    seq = ++opr->logbitCtl.seq;

    if (!waitLogbitAck(seq))
        printf("[*] ERR: no response from ICCP (timeout) while clearing layer FLOW/CFG bits\n");
}


//
// 모듈:	CmdICCPDebug()
//
void CmdMMSDebug(void)
{
	char layerBuf[16];
	char bitBuf[16];
	char valBuf[16];
	int  value;

	if (GetToken() <= 0)
	{
		printf("[*] ICCP-MMS Comm Debug ON ...!\n");
		opr->mmsDebug = 1;
		return;
	}

	strncpy(layerBuf, token, sizeof(layerBuf)-1);
	layerBuf[sizeof(layerBuf)-1] = 0;

	if (GetToken() <= 0)
	{
		printf("[*] ERR: usage: mms <mi|mvl|mms|acse> <FLOW|CFG> <on|off>\n");
		return;
	}
	strncpy(bitBuf, token, sizeof(bitBuf)-1);
	bitBuf[sizeof(bitBuf)-1] = 0;

	if (GetToken() <= 0)
	{
		printf("[*] ERR: usage: mms <mi|mvl|mms|acse> <FLOW|CFG> <on|off>\n");
		return;
	}
	strncpy(valBuf, token, sizeof(valBuf)-1);
	valBuf[sizeof(valBuf)-1] = 0;

	if (CheckEOT() < 0) return;

	value = parseOnOff(valBuf);
	if (value < 0)
	{
		printf("[*] ERR: '%s' is not on/off\n", valBuf);
		return;
	}

	postLogbitSet(layerBuf, bitBuf, value);
}

//
// 모듈:	CmdSimDebug()
//
void CmdSIMDebug(void)
{
	if (CheckEOT() < 0) return;
    printf("[*] SIMULATOR Comm Debug ON ...!\n");
    opr->simDebug = 1;
    
}

//
// 모듈:	CmdCHKSUM()
//
void CmdCHKSUM(void)
{
	if (CheckEOT() < 0) return;
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
    
}

//
// 모듈:	CmdSCUDebug()
//
void CmdSCUDebug(void)
{
	if (CheckEOT() < 0) return;
    printf("[*] SCU Comm Debug ON ...!\n");
    opr->scuDebug = 1;
    
}

//
// 모듈:	CmdLINKDebug()
//
void CmdLINKDebug(void)
{
	if (CheckEOT() < 0) return;
    printf("[*] LINK Comm Debug ON ...!\n");
    opr->linkDebug = 1;
    
    
}

//
// 모듈:	CmdQuit()
//
void CmdOffDebug(void)
{
    char    buffer[256];
    int     length;
    
	if (CheckEOT() < 0) return;

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
    
    /* CONSOLE Message : LOG 저장 Flag */
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
}

//
// 모듈:	CmdQuit()
//
void CmdQuit(void)
{
	if (CheckEOT() < 0)
		return;

	termExec = 0;
	
	CmdOffDebug();
	
}

//
// 모듈:	CmdExit()
//
void CmdExit(void)
{
    char    buffer[256];
    TASK_INFO	*wdtPtr = NULL;
    
	if (CheckEOT() < 0)
		return;
		
    printf(">> Program Terminate.... Are You Sure...? ");
    if(getchar() == 'y')
    {
        opr->userReset = SET;
        
        /* 운영자 종료명령 */
        logEvent_MPU(shmPtr, ENT_RESET_USER, 30, 0, 0, opr->cpuMode, NULL);
        
        sprintf(buffer, "%s", "----------------------------------------------------");
        LogFile_MPU(shmPtr, ENT_RESET_USER, buffer, strlen(buffer));
        
        pause(1000);
    
        wdtPtr = (TASK_INFO *) &shmPtr->taskInfo[WDT_PROCESS];
        printf("[*] WDT Process ... pid= %d\n", wdtPtr->pid);
        ExecCommand("kill %d \n", wdtPtr->pid);
    
	    termExec = 0;
    }
    	    
}

//
// 모듈:	CmdHelp()
//
void CmdHelp(void)
{
	int		index, fIndex, length;
	CMD_ENTRY	*cp;

	if (GetToken() > 0)
	{
		cp = cmdTable;
		for (index = 0, fIndex = 0; index < MAX_COMMAND; index++, cp++)
		{
			if (strlen(cp->name) == 0)
				continue;
			length = strlen(cp->name) < strlen(token) ? strlen(cp->name) :
				strlen(token);
			if (strncmp(cp->name, token, length) != 0)
				continue;
			printf("Command %s:\n", cp->name);
			printf("\tDescription: %s\n", cp->help);
			printf("\tUsage: %s\n", cp->usage);
			fIndex++;
		}
		if (fIndex == 0)
			printf("%%%%WRN_Invalid Command %s\n", token);
		return;
    }

	for (index = 0, cp = cmdTable; index < MAX_COMMAND; index++, cp++)
	{
		if (strlen(cp->name) == 0)
		{
			break;
        }			
		printf("%-10s %s\n", cp->name, cp->help);
    }
}


//
// 모듈:	CmdProcess()
//
void CmdProcess_LHS(void)
{
	int		idx;
	TASK_INFO	*ptr;

    //word    millisecond;
    //struct timeval rv; 
    //time_t  tm;
    //struct  tm  *localtm;
    
	// 전체 프로세스의 상태를 표시한다.
	PrnLine('=');
	printf(" Process DEF ACT HST DBG CHK %-11s RST %-8s Last %-6s %-6s\n", 	"WDT-Count", "PID", "MaxET", "PrevET");
	PrnLine('-');
	for (idx = 0, ptr = shmPtr->taskInfo; idx < MAX_PROCESS; idx++, ptr++)
	{
		if (strlen(ptr->name) == 0)
			continue;
		printf(" %-7s %-3d %-3d %-3d %-3d %-3d %4d/%-6d %-3d %-8d %-4d %-6d %-6d\n",
			ptr->name, ptr->define, ptr->active, ptr->hostid, ptr->debug,
			ptr->check, ptr->wdtCount, ptr->wdtEnable, ptr->restart,
			ptr->pid, ptr->lastWdt, ptr->maxEtime / 1000,
			ptr->prevEtime / 1000);
	}
	PrnLine('=');
                    
}

//
// 모듈:	CmdProcess()
//
void CmdProcess(void)
{
    int     ch;
	int		idx;
	TASK_INFO	*ptr;

    //word    millisecond;
    //struct timeval rv; 
    //time_t  tm;
    //struct  tm  *localtm;
    
    while(1)
    {   
	// 전체 프로세스의 상태를 표시한다.
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
                  
}
/*
*   GIPAM2000 - DEVICE 
*/
void CmdDispDevice(void)
{
    char    type;
    int     point;
	int		vmsid;
	int devid;

	// 명령 형식을 점검한다.
	if (GetNumber(1, MAX_DEVICE, &vmsid, "DNP Device Data Display...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    devid = vmsid - 1;
    
    printf("=>Select DEVICE = %d\n", devid +1);
    printf("=>Select TYPE  [0: ALL, 1: STATUS, 2: ANALOG] = ");
    type   = getDecWord();
    //printf(" type = %d\n", type);
    
    printf("=>Select Device (%2d)/Point Number (Max 1024) = ", devid + 1); 
    point = getDecWord() - 1;
    //printf(" point = %d\n", point+1);    
    
    if(point < 0)   type = 0;
        
    /* -------------------------------------------- */
    /*  DISPLAY Type에 따른 계전기 데이터 출력 ...  */
    /* -------------------------------------------- */
    if(type == 1)       dispDevStatus(devid, point);
    else if(type == 2)  dispDevAnalog(devid, point);     
    else                dispDevice(devid);       
            
}

//
// ==============================================================
// 모듈:	CmdEvent()
// SDP 내부 운영 History 정보 참조
// ==============================================================
//
void CmdEvent(void)
{
    int  j;
    int ch, mode;
    int id, ioid, point, state, hostid;
    word    front;
    SYSLOG_FORM   *log;
    struct tm cTime, sTime;
    int milisec1, milisec2;
    
    SDP_DEVICE  *dev;
    POINT_BUF   *ptBuf;
    ESIO_CONFIG *esio;    
        
	// 명령 형식을 점검한다.
	if (CheckEOT() < 0)	return;

    printf("\n*** CU-HISTORY Data Display ***\n");
    printf(">> SDP Controller : %s \n", TARGET_NAME);
    printf(">> Manufacture    : ACE Control Co.,Ltd \n");
    printf(">> Model Name     : %s\n", VERSION_STRING);
    printf(">> Update Date    : %s\n", DATE_STRING);
    printf(">> Event Number   : front = %3d ... over =%d [MAX %d]\n", hque->front, hque->overlab, HISTORY_QUE_MAX );
    printf("\n");

    printf("\n--------------------------------------------------------------\n");
    printf("[SELECT Info :  [1] SOE, [2] CONTROL, [3] COMM ... ");

    /* Select EVENT-CODE */            
    mode = getDecWord();
    if((mode != 1) && (mode != 2) && (mode != 3))   mode = 0;
    printf("--------------------------------------------------------------\n");
    
    
    front = hque->front & HISTORY_QUE_MASK;
    
    for(j = 0; ;)
    {
        /* ------------------------------------ */
        /* 최신 이벤트부터 역순으로 Display...  */
        /* ------------------------------------ */
        front = (front - 1) & HISTORY_QUE_MASK;
        
        if((hque->overlab == 0) && (front == HISTORY_QUE_MASK))    break;
        else if(front == hque->front) break; 

        /* ------------------------------------ */
        /* 이벤트 Queue 정보 추출...            */
        /* ------------------------------------ */                  
        log = (SYSLOG_FORM *) &hque->queue[front];
        
        id      = log->logid;       // 이벤트 코드 구분 : SOE/Control/System ...
        ioid    = log->ioid;        // 대상 계전기 번호 [1...112]
        point   = log->point;       // 대상 포인트 번호 [1...1024]
        state   = log->state;       // 이벤트/제어 상태
        hostid  = log->hostid;      // 발생주체, 0: MPU, 1 ~ 8 : HOST

        localtime_r (&log->logTime.tv_sec, &cTime);     // 이벤트 로깅 Time정보
        milisec1 = log->logTime.tv_usec / 1000;
        
        if((id <= 0) || (id >= NUMBER_OF_EVENT))	continue;
        	
        //printf("event> front=%d...id=%d (%02x), ioid=%d, point=%d, state=%d, hostid=%d ... j=%d \n", front, id, id, ioid, point, state, hostid, j);
        
        /* ------------------------------------------------ */
        /*  이벤트 코드에 따른 분류...                      */
        /* ------------------------------------------------ */
        if(mode == 1)                   // SOE 이벤트                                         
        {
            if(id != ENT_SOE)   continue;    
        }   
        else if(mode == 2)              // CONTROL 이벤트
        {
            if((id < ENT_ICCP_CNTR) || (id > ENT_LINK_CNTR))    continue;    
        }   
        else if(mode == 3)              // 통신관련 이벤트
        {
            if((id < ENT_ICCP_ONLINE) || (id > ENT_VME_OFFLINE))    continue;    
        }   
        
        /* ------------------------------------------------ */
        /*  이벤트 코드별 출력형식 분류...                  */
        /* ------------------------------------------------ */    
        if(id == ENT_SOE)
        {   
            /* SOE 생성시각 표시... */
            localtime_r (&log->soeTime.tv_sec, &sTime);
            milisec2 = log->soeTime.tv_usec / 1000;
            
            if((ioid <= 0) ||( point <= 0))	continue;
            	
            dev = (SDP_DEVICE *) deviceCFG[ioid -1];
            ptBuf = (POINT_BUF *) &dev->diPtBuf[point -1];
               
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] SOE:%04d/%02d/%02d-%02d:%02d:%02d-%03d %s \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon +1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid,
                sTime.tm_year + 1900, sTime.tm_mon +1, sTime.tm_mday, sTime.tm_hour, sTime.tm_min, sTime.tm_sec, milisec2, ptBuf->ptNameStr);
        }
        else if(id == ENT_DEVICE_SOE)           // 장치포인트 운영 이벤트
        {   
            /* SOE 생성시각 표시... */
            localtime_r (&log->soeTime.tv_sec, &sTime);
            milisec2 = log->soeTime.tv_usec / 1000;
            
            if((ioid < 0) ||( point < 0))	continue;
            	
            ptBuf = (POINT_BUF *) devPtBuf[ioid];      // Device Point 정보
               
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] DEV:%04d/%02d/%02d-%02d:%02d:%02d-%03d %s \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon +1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid, 
                sTime.tm_year + 1900, sTime.tm_mon +1, sTime.tm_mday, sTime.tm_hour, sTime.tm_min, sTime.tm_sec, milisec2, ptBuf->ptNameStr);
        }
        else if((id >= ENT_ICCP_CNTR) && (id <= ENT_LINK_CNTR))     // 제어관련 이벤트...
        {   
            /* SOE 생성시각 표시... */
            localtime_r (&log->soeTime.tv_sec, &sTime);
            milisec2 = log->soeTime.tv_usec / 1000;
            
            if((ioid <= 0) ||( point <= 0))	continue;
            	
            dev   = (SDP_DEVICE *) deviceCFG[ioid -1];
            ptBuf = (POINT_BUF *) &dev->doPtBuf[point -1];
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] CNT:%04d/%02d/%02d-%02d:%02d:%02d-%03d %s \n", front + 1, 
                cTime.tm_year + 1900, cTime.tm_mon +1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid, 
                sTime.tm_year + 1900, sTime.tm_mon +1, sTime.tm_mday, sTime.tm_hour, sTime.tm_min, sTime.tm_sec, milisec2, ptBuf->ptNameStr);
        }
        else if((id >= ENT_VME_ONLINE) && (id <= ENT_VME_UNINSTALL))     // VME 관련 이벤트...
        {   
        	if((ioid < 0) ||( point < 0))	continue;
        		
            esio = (ESIO_CONFIG *) esioCFG[ioid-1];
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] VME: %s \n", front + 1, cTime.tm_year + 1900, cTime.tm_mon +1, 
                cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid, esio->esioNameStr);
        }
        else if((id >= ENT_ESIO_ONLINE) && (id <= ENT_ESIO_OFFLINE))     // ESIO 통신 관련 이벤트...
        {   
        	if((ioid < 0) ||( point < 0))	continue;
        		
            esio = (ESIO_CONFIG *) esioCFG[ioid-1];
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] COM: %s \n", front + 1, cTime.tm_year + 1900, cTime.tm_mon +1, 
                cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid, esio->esioNameStr);
        }
        else if((id == ENT_DEV_ONLINE) || (id == ENT_DEV_OFFLINE))
        {
            /* SOE 생성시각 표시... */
            localtime_r (&log->soeTime.tv_sec, &sTime);
            milisec2 = log->soeTime.tv_usec / 1000;
            
            if((ioid < 0) ||( point < 0))	continue;
            	
            dev = (SDP_DEVICE *) deviceCFG[ioid -1];

            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] COM:%04d/%02d/%02d-%02d:%02d:%02d-%03d %s \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon +1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid,
                sTime.tm_year + 1900, sTime.tm_mon +1, sTime.tm_mday, sTime.tm_hour, sTime.tm_min, sTime.tm_sec, milisec2, dev->devNameStr);
        }       
        else
        {
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon +1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid);
        }
                      
        if(((++j)%32) == 0) 
        {
        	j= 0; ch=0;
            ch = getchar();
            if(ch == 'q')   break;     
        }            
        
        /* end 지정 */
        //front = (front - 1) & HISTORY_QUE_MASK;
        
    }
    
    printf("\n [ EVENT-Q ] end ---------------------------------------------\n");
}
