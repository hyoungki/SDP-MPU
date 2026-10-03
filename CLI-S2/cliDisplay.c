/*
 * ==============================================================
 * File : cliDisplay.c
 *
 * CLI-S2 : CLI-ReadLine의 조회/디스플레이 계열 Cmd*() 함수들을
 *   int do_xxx(int argc, char *argv[]) 시그니처로 옮긴 버전.
 *   대부분 인자를 쓰지 않던 명령이라 본문은 그대로이고, 인자 개수를
 *   확인하던 CheckEOT()만 있는 경우 argc==1 검사로 바꿨다. getDecWord()로
 *   호스트 번호를 대화형으로 받던 dnpdi/dnpai/dnpdo는 본문을 그대로 두고
 *   시그니처만 바꿨다 (design/cli-argv-command-dispatch.md 참고).
 * ==============================================================
 */

#include	"localLib.h"
#include    "external.h"

#include <stddef.h>

#include "iccpShm.h"

extern  int CheckEOT(void);
extern  int getDecWord();

static char scanComMode[4][8]   = {{"RS232"},{"MODEM"}, {"RS485"}, {"TCPIP"}};
static char scanPortType[8][8]  = {{" COM1"}, {" COM2"},{" COM3"}, {" COM4"}, {" COM5"}, {" COM6"}, {" COM7"}, {" COM8"} };

static char scanModType[10][8]  = {{"*NOT "}, {" SIO "},{"ESIO1"}, {"ESIO2"}, {"ESIO3"}, {"ESIO4"}, {" RTU "}, {" MPU1"}, {" MPU2"} };
static char scanESIOType[8][8]  = {{"*NOT "}, {" SIO "},{"ESIO1"}, {"ESIO2"}, {"ESIO3"}, {"ESIO4"}, {" RTU "} };

static char scanProtType[16][8] = {{" *NOT "}, {"  DNP "}, {"MODBUS"}, {"  BUS "}, {"RM-DGA"}, {"RM-GAS"}, {"REMOTE"}, {"EL-EQ1"},{"EL-EQ2"},
                                   {"EL-EQ3"}, {"61850 "}, {"LOCAT1"}, {"LOCAT2"}, {"*RESE1"}, {"*RESE2"}, {"*RESE3"} };

static char speedType[8][10]    = {{"1200B "}, {"2400B "}, {"4800B "}, {"9600B "},{"19200 "},{"38400 "},{"57600 "}, {"115200"}};
static char scanChgMode[4][10]  = {{"*NOT"}, {" ALL"}, {"EACH"}, {"****"}};

static char scanCfgType[20][10] = {{"*NO"}, {"S01"}, {"S02"}, {"S03"},{"S04"},{"S05"},{"S06"},{"S07"},{"S08"},
                                   {"S09"}, {"S10"}, {"S11"}, {"S12"},{"S13"},{"S14"},{"S15"},{"S16"},{"***"}};

static char scanDevType[32][12] = {{"  *NULL  "},{"RTU-DIM  "},{"RTU-DOM  "},{"RTU-AIM  "},{"RTU-AOM  "},{"DNP-GIPAM"},{"DNP-SIEMN"},{"DNP-HIMAP"},{"DNP-FRTU "},{"DNP-PROPC"},
                                   {"DNP-VIPAM"},{"DNP-LBS  "},{"MOD-LG   "},{"MOD-SIEMN"},{"MOD-SUB  "},{"LOC-TYPE1"},{"LOC-TYPE2"},{"REM-UHF  "},{"REM-DGA  "},{"REM-LA   "},
                                   {"REM-TYPE1"},{"REM-TYPE2"},{"REM-TYPE3"},{"REM-TYPE4"},{"EQM-TYPE1"},{"EQM-TYPE2"},{"EQM-TYPE3"},{"EQM-TYPE4"},{"61850-D#1"},{"61850-D#2"},
                                   {"61850-D#3"} };

static char devTypeStr[32][8]  = { {"------ "}, {" DIM   "}, {" DOM   "}, {" AIM   "}, {" AOM   "}, {" VDI   "}, {" VAI   "}, {" VDO   "},
                                   {" DEV   "}, {"SDP-STS"}, {"CPU-RUN"}, {"CPU-STS"}, {"RTU-RUN"}, {"RTU-STS"}, {" CU-RUN"}, {" CU-STS"},
                                   {"DIG-RUN"}, {"DIG-STS"}, {"EQM-RUN"}, {"EQM-STS"}, {"IEC-RUN"}, {"IEC-STS"}};

static char iccpPTypeStr[16][8]= { {"---"},{"SDI"}, {"SDO"}, {"SAI"}, {"DDI"}, {"DAI"},
                                   {"QDI"},{"QAI"}, {"TDI"}, {"TAI"}, {"DEV"}};

static char pointTypeStr[16][8]= { {"--- "},{"COS "}, {"SOE "}, {"ALL "}, {"ACC "}, {"SBO "},
                                   {"DCO "},{"1MA "}, {"20MA"}, {"10V "}, {"RALL"}, {"RDEV"}, {"MCPU"},{"SCPU"},{"CNTR"}};

static char calPtType[4][10]   = {{"  NULL "}, {" STATUS"}, {" ANALOG"}, {"CONTROL"}};
static char onoffType[2][8]    = {{"OFF"},   {"ON "}};
static char cpuTypeStr[2][12]  = {{"MASTER-CPU"}, {"SLAVE-CPU"}};

static char modbusType[8][8]   = {{"NULL"}, {"DIM "}, {"AIM "}, {"DOM "},{"AOM "}};
static char modbusFormat[8][10]= {{"BIT  "}, {"BYTE "}, {"SHORT"},{"WORD "},{"INT  "}, {"UINT "}, {"FLOAT"}};

static char hostMode[4][10]    = {{" NULL "}, {"SINGLE"}, {" DUAL "}};
static char hostChgMode[4][10] = {{" NULL  "},{"RESTART"},{"CHANGE "}};
static char hostPTType[10][8]  = {{" NULL  "}, {"HARRIS "}, {"LANDIS "}, {"DNP    "},{"MODBUS "},{"*ICCP-6"}, {"IEC-101"}, {"RES-TY1"}, {"RES-TY2"}};

static char comPortType[16][8] = {{" COM1"}, {" COM2"},{" COM3"}, {" COM4"}, {" COM5"}, {" COM6"}, {" COM7"}, {" COM8"},
                                  {" SIO1"}, {" SIO2"},{" SIO3"}, {" SIO4"}, {" SIO5"}, {" SIO6"}, {" SIO7"}, {" SIO8"}};
static char netType[8][8]      = {{"NET#1"}, {"NET#2"},{"NET#3"},{"NET#4"},{"NET#5"},{"NET#6"},{"NET#7"},{"NET#8"}};
static char classString[4][10] = {{" NULL "},{"CLASS1"},{"CLASS2"}, {"CLASS3"}};

static  char *MY_ENDIAN[3]={ "NONE","LITTLE","BIG"} ;


//
// 모듈:	do_version()
//
int do_version(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("%%%%ERR_Usage: version\n");
		return(E_ERR);
	}

    printf("\n*** Display CU History ***\n");
    printf(" 1. Target Name : [%s] \n", TARGET_NAME);
    printf(" 2. Manufacture : ACE Control System Co.,Ltd\n");
    printf(" 3. Model Name  : %s\n", VERSION_STRING);
    printf(" 4. Simulator   : %s\n", SIMULATOR_NAME);
    printf(" 5. Update Date : %s\n", DATE_STRING);
    printf(" 6. Print  Date : %4d/%2d/%2d %02d:%02d:%02d\n", ((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
    printf("[프로그램 Update]------------------------------------\n");
    printf("-----------------------------------------------------\n");
    printf("[2016/04/22 - SDP-2000 Version 1.0\n");
    printf("-----------------------------------------------------\n");
    printf("[2018/08/28 - Database Update 이벤트 추가 \n");
    printf("[2019/07/19 - CPU-Change, CPU-RESET 리포트 형식추가 \n");
    printf("[2019/08/22 - CPU 이중화계통(양방향)-SDP 동작상태 SOE 발생 형식추가 \n");
    printf("[2020/03/20 - control_MPU() ... 원격제어 시점 시분초단위 정보처리 \n");
    printf("[2020/03/31 - ICCP-TASK Start Time ... Deley (20sec)  \n");
    printf("[2020/04/09 - LINK-SOE 입력시... HOST 상태 Update 시 Reject \n");
    printf("[2020/04/23 - HISTORY-Queue Update... 무한대기 오류 수정 \n");
    printf("[2020/05/18 - CPU 이중화 화일로그 추가 \n");
    printf("[2020/06/01 - 통신제어체계-RTU 지원 \n");
    printf("-----------------------------------------------------\n");
    printf("[2020/09/15 - SDP-2000 Version 3.0\n");
    printf("-----------------------------------------------------\n");
    printf("[2020/09/15 - VITZRO-SYS 자보호계전기 지원 추가 \n");
    printf("[2022/06/29 - DEVICE Point 용량(AI-256, DO-200) 증대  \n");
    printf("[2022/07/08 - CPU 이중화: DNP-HOST(modem) 무응답시 => Channel RESET\n");
    printf("[2024/11/15 - 원거리감시제어(TCPIP): LOCAL-SLAVE 방식 HOST 지원 \n");
    printf("\n");

    return(E_OK);
}

/*
* 원본 signature 그대로 유지되는 helper - cmdTable에 등록되지 않는다.
*/
void    display_ICCP_INFO(int pointType, int pointInx)
{
    int             point, pointMax;
    int     count, ch;
    I60870_DATA     *iccpPt=NULL;
    char buf[100];
    char    headLine[128];

    if(pointType == SDP_POINT_TYPE_SDI)
    {
        if((pointInx < 1) || (pointInx >= (iccpInfo->maxIndex_sdi))) return;
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_sdi[pointInx - 1];
        pointMax = iccpInfo->maxIndex_sdi;
        sprintf(headLine, "\n[ICCP : SDI List MAX=%d] -----------------------------------------------\n", pointMax );
    }
    else if(pointType == SDP_POINT_TYPE_SDO)
    {
        if((pointInx < 1) || (pointInx >= (iccpInfo->maxIndex_sdo))) return;
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_sdo[pointInx - 1];
        pointMax = iccpInfo->maxIndex_sdo;
        sprintf(headLine, "\n[ICCP : SDO List MAX=%d] -----------------------------------------------\n", pointMax );
    }
    else if(pointType == SDP_POINT_TYPE_SAI)
    {
        if((pointInx < 1) || (pointInx >= (iccpInfo->maxIndex_sai))) return;
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_sai[pointInx - 1];
        pointMax = iccpInfo->maxIndex_sai;
        sprintf(headLine, "\n[ICCP : SAI List MAX=%d] -----------------------------------------------\n", pointMax );
    }
    else if(pointType == SDP_POINT_TYPE_DDI)
    {
        if((pointInx < 1) || (pointInx >= (iccpInfo->maxIndex_ddi))) return;
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_ddi[pointInx - 1];
        pointMax = iccpInfo->maxIndex_ddi;
        sprintf(headLine, "\n[ICCP : DDI List MAX=%d] -----------------------------------------------\n", pointMax );
    }
    else if(pointType == SDP_POINT_TYPE_DAI)
    {
        if((pointInx < 1) || (pointInx >= (iccpInfo->maxIndex_dai))) return;
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_dai[pointInx - 1];
        pointMax = iccpInfo->maxIndex_dai;
        sprintf(headLine, "\n[ICCP : DAI List MAX=%d] -----------------------------------------------\n", pointMax );
    }
    else if(pointType == SDP_POINT_TYPE_QDI)
    {
        if((pointInx < 1) || (pointInx >= (iccpInfo->maxIndex_qdi))) return;
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_qdi[pointInx - 1];
        pointMax = iccpInfo->maxIndex_qdi;
        sprintf(headLine, "\n[ICCP : QDI List MAX=%d] -----------------------------------------------\n", pointMax );
    }
    else if(pointType == SDP_POINT_TYPE_QAI)
    {
        if((pointInx < 1) || (pointInx >= (iccpInfo->maxIndex_qai))) return;
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_qai[pointInx - 1];
        pointMax = iccpInfo->maxIndex_qai;
        sprintf(headLine, "\n[ICCP : QAI List MAX=%d] -----------------------------------------------\n", pointMax );
    }
    else if(pointType == SDP_POINT_TYPE_TDI)
    {
        if((pointInx < 1) || (pointInx >= (iccpInfo->maxIndex_tdi))) return;
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_tdi[pointInx - 1];
        pointMax = iccpInfo->maxIndex_tdi;
        sprintf(headLine, "\n[ICCP : TDI List MAX=%d] -----------------------------------------------\n", pointMax );
    }
    else if(pointType == SDP_POINT_TYPE_TAI)
    {
        if((pointInx < 1) || (pointInx >= (iccpInfo->maxIndex_tai))) return;
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_tai[pointInx - 1];
        pointMax = iccpInfo->maxIndex_tai;
        sprintf(headLine, "\n[ICCP : TAI List MAX=%d] -----------------------------------------------\n", pointMax );
    }
    else if(pointType == SDP_POINT_TYPE_DEV)
    {
        if((pointInx < 1) || (pointInx >= (iccpInfo->maxIndex_dev))) return;
        iccpPt = (I60870_DATA *) &iccpInfo->iccp_dev[pointInx - 1];
        pointMax = iccpInfo->maxIndex_dev;
        sprintf(headLine, "\n[ICCP : DEV List MAX=%d] -----------------------------------------------\n", pointMax );
    }
    else
        return;

    printf("%s", headLine);
    printf(" C.No [dNo dPt] CFG    OFL VALUE    [ OFL VALUE ] TICK \n");
    printf("-----------------------------------------------------------------------\n");

    count=0;
    for(point= pointInx; point <= pointMax; point++)
    {
        if(iccpPt->config != SET)
        {
            iccpPt++;
            continue;
        }

        printf("  %02d  [%2d  %3d] %2d     %2d   %04.2f     %2d  %04.2f   %03d   %ss   %s\n", point, iccpPt->devNo, iccpPt->devPt, iccpPt->config,
            iccpPt->offline, iccpPt->value, iccpPt->flag, iccpPt->report, iccpPt->accessTick, sprt_timeval (buf, sizeof(buf), &iccpPt->updateTime), iccpPt->ptNameStr);

        if(++count == 32)
        {
            count = 0;
            printf("[q]... quit!\n");
            ch=getchar();
            if(ch == 'q')  break;
            printf("%s", headLine);
            printf(" C.No [dNo dPt] CFG    OFL VALUE    [ OFL VALUE ] TICK \n");
            printf("-----------------------------------------------------------------------\n");
        }

        iccpPt++;
    }
    printf("-----------------------------------------------------------------------\n\n");
    return ;
}

/*----------------------------------------------------------------------------
* Function : do_relay()
* 내용: CCU에서 계전기 설정정보를 Console에 출력하는 함수
---------------------------------------------------------------------------- */
int do_relay(int argc, char *argv[])
{
    short i;

    SDP_DEVICE  *dev;
    SCAN_CONFIG *scan;

    printf("\n============================================================================\n");
    printf(" %s : SCAN Configuration ...\n", TARGET_NAME);
    printf("  No.  ACT  PROTO  MODE   TARGET PORT  SPEED [DELAY OFFT] CH-M  FT1 FT2 FT3  NAME \n");
    printf("============================================================================\n");

    for(i = 0; i < MAX_SCAN_PORT; i++)
    {
    	scan = (SCAN_CONFIG *) scanCFG[i];

        if(scan->useFlag == 0)  continue;

        printf("  %02d.   %d  %s  %s   %5s  %5s  %s   %02d   %02d   %s    %d  %d  %d    %s\n",
            i + 1,  scan->useFlag, scanProtType[scan->protocol], scanComMode[scan->comMode], scanModType[scan->targetID], scanPortType[scan->comPort],
            speedType[scan->comSpeed], scan->comDelay, scan->offCount, scanChgMode[scan->chgMode], scan->function1, scan->function2, scan->function3, scan->scanNameStr);
    }

    printf("\n\n----------------------------------------------------------------------------\n");
    printf(" %s : RELAY Configuration ...(Max SDP Point = %d)\n", TARGET_NAME, opr->max_sdpPoint);
    printf(" No. SCN ID Target PORT  Type      USEF [ DI/ DO/ AI/ AO] MOD ONL [SND /RCV]  PRE [IP_ADDR     Port]\n");
    printf("----------------------------------------------------------------------------\n");
    for(i = 0; i < MAX_DEVICE; i++)
    {
        dev = (SDP_DEVICE *) deviceCFG[i];

        if((dev->type == 0) && (dev->scan == 0))    continue;

        scan = (SCAN_CONFIG *) scanCFG[dev->scanPort - 1];

        printf(" %02d. %s %02d %s %s  %s  %02d  [%3d/%3d/%3d/%3d] %2d  %02x  %04d-%04d %4d [%s,%4d] %s \n",
            i + 1, scanCfgType[dev->scanPort], dev->comDevID, scanModType[scan->targetID],
            scanPortType[scan->comPort], scanDevType[dev->type], dev->scan,
            dev->devDiPoint, dev->devDoPoint, dev->devAiPoint, dev->devAoPoint,
            dev->modbusFileNo,
            dev->online,
            dev->comSndCount,
            dev->comRcvCount,
            dev->oldRcvCount,
            dev->ipString1,
            dev->netPort,
            dev->devNameStr);
    }

    printf("----------------------------------------------------------------------------\n");
    printf("\n... Configuration End \n\n");

    return(E_OK);
}


/*----------------------------------------------------------------------------
* Function : do_mpucfg()
* 내용: MCU 설정정보 Display
---------------------------------------------------------------------------- */
int  do_mpucfg(int argc, char *argv[])
{
    MPU_NET_ENTRY   *mNet1, *mNet2,*mNet3, *sNet1, *sNet2,*sNet3;
    MPU_NET_ENTRY    *mNet4, *sNet4;

    mNet1 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[0];
    mNet2 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[1];
    mNet3 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[2];
    mNet4 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[3];

    sNet1 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[0];
    sNet2 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[1];
    sNet3 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[2];
    sNet4 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[3];


    printf("\n============================================================================\n");
    printf(" %s : MCU Configuration  : %s \n", TARGET_NAME, mpuCFG->sdpNameStr);
    printf("============================================================================\n");
    printf("[*] DUAL-CPU Run Mode  = %s , CPU = %s\n", onoffType[mpuCFG->dualMpu], cpuTypeStr[opr->cpuMode]);
    printf("[*] DUAL-RTU Run Mode  = %s \n", onoffType[mpuCFG->dualModule]);
    printf("[*] USE MMI Module     = %s \n", onoffType[mpuCFG->mmiUseFlag]);
    printf("[*] USE SCU Module     = %s \n", onoffType[mpuCFG->scuUseFlag]);
    printf("[*] Status Dump Period = %2d [sec]\n", mpuCFG->statusDump);
    printf("[*] Analog Dump Period = %2d [sec]\n", mpuCFG->analogDump);
    printf("[*] Modbus Debounce T  = %2d \n", mpuCFG->debounce);
    printf("[*] WDT DB-Check Flag  = %2d \n", mpuCFG->dbCheck);
    printf("[*] HOST Comm-WDT Flag = %2d \n", mpuCFG->hostWDT);
    printf("[*] ICCP-HOST Flag     = %2d \n", mpuCFG->iccpEnbFlag);
    printf("[*] Function Code#     = %02d %02d %02d %02d %02d %02d\n", mpuCFG->func1, mpuCFG->func2, mpuCFG->func3, mpuCFG->func4, mpuCFG->func5, mpuCFG->func6);
    printf("[%] Endian             = %s[%d]\r\n",MY_ENDIAN[ opr->endian & 0x03 ] ,  opr->endian );
    printf("============================================================================\n");
    printf("[*] Master Network#1 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet1->useFlag, mNet1->ipAddr, mNet1->gwAddr, mNet1->subMask);
    printf("           Network#2 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet2->useFlag, mNet2->ipAddr, mNet2->gwAddr, mNet2->subMask);
    printf("           Network#3 Info(IP/GW/SUB): use = %d, %s/%s/%s\n", mNet3->useFlag, mNet3->ipAddr, mNet3->gwAddr, mNet3->subMask);
    printf("           Network#4 Info(IP/GW/SUB): use = %d, %s/%s/%s\n\n", mNet4->useFlag, mNet4->ipAddr, mNet4->gwAddr, mNet4->subMask);
    printf("[*] Slave  Network#1 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet1->useFlag, sNet1->ipAddr, sNet1->gwAddr, sNet1->subMask);
    printf("           Network#2 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet2->useFlag, sNet2->ipAddr, sNet2->gwAddr, sNet2->subMask);
    printf("           Network#3 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet3->useFlag, sNet3->ipAddr, sNet3->gwAddr, sNet3->subMask);
    printf("           Network#4 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet4->useFlag, sNet4->ipAddr, sNet4->gwAddr, sNet4->subMask);
    printf("============================================================================\n\n");

    return(E_OK);
}


/*----------------------------------------------------------------------------
* Function : do_esiocfg()
* 내용: ESIO 설정정보 Display
---------------------------------------------------------------------------- */
int do_esiocfg(int argc, char *argv[])
{
    char    ch;
    short i, j,id;
    ESIO_NET_ENTRY   *mNet1, *mNet2,*mNet3, *sNet1, *sNet2,*sNet3;
    ESIO_PORT_ENTRY  *port;
    ESIO_CONFIG      *esio;

#ifdef __ARM_ARCH__
 ESIO_NET_ENTRY   *mNet4,*sNet4;
#endif

    for( i=0; i < MAX_ESIO; i++)
    {
        esio = (ESIO_CONFIG *) esioCFG[i];

        mNet1 = (ESIO_NET_ENTRY *) &esio->mstNetConfig[0];
        mNet2 = (ESIO_NET_ENTRY *) &esio->mstNetConfig[1];
        mNet3 = (ESIO_NET_ENTRY *) &esio->mstNetConfig[2];

        sNet1 = (ESIO_NET_ENTRY *) &esio->slvNetConfig[0];
        sNet2 = (ESIO_NET_ENTRY *) &esio->slvNetConfig[1];
        sNet3 = (ESIO_NET_ENTRY *) &esio->slvNetConfig[2];

#ifdef __ARM_ARCH__
        mNet4 = (ESIO_NET_ENTRY *) &esio->mstNetConfig[3];
        sNet4 = (ESIO_NET_ENTRY *) &esio->slvNetConfig[3];
#endif

        printf("\n============================================================================\n");
        printf("  ESIO(%d) Configuration  : %s \n", i+1, esio->esioNameStr);
        printf("============================================================================\n");
        printf("[*] ESIO USE Flag  = %s \n", onoffType[esio->useFlag]);
        printf("[*] ESIO Target ID = %s \n", scanESIOType[esio->targetID]);
        printf("[*] ESIO AUTO CHG  = %s \n", onoffType[esio->autoChgFlag]);
        printf("[*] ESIO ComDelay  = %d \n", esio->comDelay);

        printf("[*] ESIO Device : ");
        for(j=0; j< esio->scanMaxNum; j++)  printf(" %2d", esio->scanDevice[j]);
        printf("\n");

        printf("----------------------------------------------------------------------------\n");
        printf("[*] Master Network#1 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet1->useFlag, mNet1->ipAddr, mNet1->gwAddr, mNet1->subMask);
        printf("           Network#2 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet2->useFlag, mNet2->ipAddr, mNet2->gwAddr, mNet2->subMask);
        printf("           Network#3 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet3->useFlag, mNet3->ipAddr, mNet3->gwAddr, mNet3->subMask);
#ifdef __ARM_ARCH__
        printf("           Network#4 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet4->useFlag, mNet4->ipAddr, mNet4->gwAddr, mNet4->subMask);
#endif

        printf("[*] Slave  Network#1 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet1->useFlag, sNet1->ipAddr, sNet1->gwAddr, sNet1->subMask);
        printf("           Network#2 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet2->useFlag, sNet2->ipAddr, sNet2->gwAddr, sNet2->subMask);
        printf("           Network#3 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet3->useFlag, sNet3->ipAddr, sNet3->gwAddr, sNet3->subMask);

#ifdef __ARM_ARCH__
        printf("           Network#4 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet4->useFlag, sNet4->ipAddr, sNet4->gwAddr, sNet4->subMask);
#endif

        printf("----------------------------------------------------------------------------\n");

        printf("  No.  SCAN  Func   MODE  SPEED     NAME \n");

        for(id=0; id < MAX_ESIO_PORT; id++)
        {
            port = (ESIO_PORT_ENTRY *) &esio->portConfig[id];
            printf( "  %02d    %d     %d    %s  %s  : %s\n", id, port->useFlag, port->function,
                scanComMode[port->comMode], speedType[port->comSpeed], port->portNameStr);
        }
        printf("[q] Quit--------------------------------------------------------------------\n\n");

        ch = getchar();
        if(ch == 'q')   break;

    }

    return(E_OK);
}


int do_esiosts_disp(int argc, char *argv[])
{
    ESIO_CONFIG      *esio;
    int i;

    printf("   connect  online  rackinstall  siorunFail  sioValid  siocount\r\n");
    for( i=0; i < MAX_ESIO; i++)
    {
        esio = (ESIO_CONFIG *) esioCFG[i];

        printf("%d %d %d %d\r\n", esio->connectStatus, esio->online ,esio->rackInstall,esio->sioRunFail ) ;
    }

    return(E_OK);
}


int    do_iccpcfg(int argc, char *argv[])
{
    ICCP_UNIT       *iccp;
    ICCP_CONFIG     *iccpCfg;

    iccpCfg = (ICCP_CONFIG *) &iccpDCB->config;

    printf("\n--------[ ICCP ] Parameter setting.---------\n");

    iccp = (ICCP_UNIT *) &iccpCfg->FEP_A;
    printf("[FEP_A] Configuration----------------------------------\n");
    printf(" IP-ADDR[16] = %s\n", iccp->IPAddress);
    printf(" P_SEL[48]   = %s\n", iccp->P_Sel);
    printf(" S_SEL[48]   = %s\n", iccp->S_Sel);
    printf(" T_SEL[96]   = %s\n", iccp->T_Sel);
    printf(" AP_Title[96]= %s\n", iccp->AP_Title);
    printf(" AP_Quali[11]= %s\n\n", iccp->AE_Qualifier);

    iccp = (ICCP_UNIT *) &iccpCfg->FEP_B;
    printf("[FEP_B] Configuration----------------------------------\n");
    printf(" IP-ADDR[16] = %s\n", iccp->IPAddress);
    printf(" P_SEL[48]   = %s\n", iccp->P_Sel);
    printf(" S_SEL[48]   = %s\n", iccp->S_Sel);
    printf(" T_SEL[96]   = %s\n", iccp->T_Sel);
    printf(" AP_Title[96]= %s\n", iccp->AP_Title);
    printf(" AP_Quali[11]= %s\n\n", iccp->AE_Qualifier);

    iccp = (ICCP_UNIT *) &iccpCfg->SDP_A;
    printf("[SDP_A] Configuration----------------------------------\n");
    printf(" IP-ADDR[16] = %s\n", iccp->IPAddress);
    printf(" P_SEL[48]   = %s\n", iccp->P_Sel);
    printf(" S_SEL[48]   = %s\n", iccp->S_Sel);
    printf(" T_SEL[96]   = %s\n", iccp->T_Sel);
    printf(" AP_Title[96]= %s\n", iccp->AP_Title);
    printf(" AP_Quali[11]= %s\n\n", iccp->AE_Qualifier);

    iccp = (ICCP_UNIT *) &iccpCfg->SDP_B;
    printf("[SDP_B] Configuration----------------------------------\n");
    printf(" IP-ADDR[16] = %s\n", iccp->IPAddress);
    printf(" P_SEL[48]   = %s\n", iccp->P_Sel);
    printf(" S_SEL[48]   = %s\n", iccp->S_Sel);
    printf(" T_SEL[96]   = %s\n", iccp->T_Sel);
    printf(" AP_Title[96]= %s\n", iccp->AP_Title);
    printf(" AP_Quali[11]= %s\n\n", iccp->AE_Qualifier);
    printf("-------------------------------------------------------\n");

	printf("=> POINT Config End...(CU Point = %2d)!\n", opr->max_sdpPoint);
    printf(" [*] ICCP-SDI = %3d   [*] ICCP-SAI = %3d    [*] ICCP-SDO = %3d \n", iccpInfo->maxIndex_sdi, iccpInfo->maxIndex_sai, iccpInfo->maxIndex_sdo);
    printf(" [*] ICCP-DDI = %3d   [*] ICCP-DAI = %3d  \n", iccpInfo->maxIndex_ddi, iccpInfo->maxIndex_dai);
    printf(" [*] ICCP-QDI = %3d   [*] ICCP-QAI = %3d  \n", iccpInfo->maxIndex_qdi, iccpInfo->maxIndex_qai);
    printf(" [*] ICCP-TDI = %3d   [*] ICCP-TAI = %3d  \n", iccpInfo->maxIndex_tdi, iccpInfo->maxIndex_tai);
    printf(" [*] ICCP-DEV = %3d \n", iccpInfo->maxIndex_dev);

    printf("-------------------------------------------------------\n\n");

    return(E_OK);
}


int    do_iccpdata(int argc, char *argv[])
{
    ICCP_SOE_QUEUE          *iccpSoe;
    ICCP_SOE_DELETE_QUEUE   *iccpDSoe;
	char buf[100];

    iccpSoe  = (ICCP_SOE_QUEUE *) &iccpDCB->soeQueue;
    iccpDSoe = (ICCP_SOE_DELETE_QUEUE *) &iccpDCB->soeDeleteQueue;

    printf("\n--------[ ICCP ] DCB Status Display.---------\n");
    printf(" CommMode          = %s \n", iccpDCB->commMaster ? "Master" : "Slave");
    printf(" AssocStatus       = %s \n", iccpDCB->assocStatus ? "Active" : "Inactive");
    printf(" ActiveArIndex     = %d \n", iccpDCB->activeArIndex);
    printf(" InactiveCount     = %d \n", iccpDCB->inactiveCount);
    printf(" LastActiveTime    = %s \n", sprt_time_t (buf, sizeof(buf), &iccpDCB->lastActiveTime));
    printf(" LastInactiveTime  = %s \n", sprt_time_t (buf, sizeof(buf), &iccpDCB->lastInactiveTime));
    printf(" LastDataSendTime  = %s \n", sprt_time_t (buf, sizeof(buf), &iccpDCB->lastDataSendTime));
    printf(" identifyRecvCount = %d \n", iccpDCB->identifyRecvCount);
    printf("--------------------------------\n");
    printf(" SOE-Q Count      = %d \n", iccpSoe->count);
    printf(" Delete SOE-Q     = [ %3d / %3d ]\n", iccpDSoe->front,iccpDSoe->rear );
    printf("--------------------------------\n");

    return(E_OK);
}


/*----------------------------------------------------------------------------
* Function : do_hostcfg()
* 내용: HOST 통신 설정정보를 Console에 출력하는 함수
---------------------------------------------------------------------------- */
int do_hostcfg(int argc, char *argv[])
{
    int             hostid;
    HOST_DCB        *host;
    HOST_NET_ENTRY  *mNet1, *mNet2, *sNet1, *sNet2;

    printf("\n--------[ HOST ] Parameter setting.---------\n");

	printf(" No. Mode   Protoc  ComMod   Speed   Mport  Sport  Host   Rtu   Time  TCP-Port  \n");

	for(hostid = 0; hostid < MAX_HOST; hostid++)
    {
        host = (HOST_DCB *) hostDCB[hostid];
        printf(" %d  %s %s  %s  %s  %s  %s %5d  %5d    %4d  %4d \n", hostid+1,
        	hostMode[host->hostDualMode], hostPTType[host->hostProtocol],
        	scanComMode[host->hostComType],	speedType[host->hostComSpeed],
        	comPortType[host->masterChan], comPortType[host->slaveChan],
        	host->hostid, host->rtuAddr, host->timeSyncDISB, host->tcpPort);
    }
    printf("==================================================\n");
    printf(" No.  NET#(M1)   IP-ADDR        NET#(M2)   IP-ADDR       NET#(S1)   IP-ADDR       NET#(S2)   IP-ADDR \n");
    printf("------------------------------------------------------------------------------------------------------------\n");

	for(hostid = 0; hostid < MAX_HOST; hostid++)
    {
        host = (HOST_DCB *) hostDCB[hostid];

        mNet1 = (HOST_NET_ENTRY *) &host->masterNetCfg[0];
        mNet2 = (HOST_NET_ENTRY *) &host->masterNetCfg[1];
        sNet1 = (HOST_NET_ENTRY *) &host->slaveNetCfg[0];
        sNet2 = (HOST_NET_ENTRY *) &host->slaveNetCfg[1];

        printf(" %d     %5s  %16s  %5s  %16s  %5s  %16s  %5s  %16s\n", hostid+1,
        	netType[mNet1->netPort], mNet1->ipAddr,
        	netType[mNet2->netPort], mNet2->ipAddr,
        	netType[sNet1->netPort], sNet1->ipAddr,
        	netType[sNet2->netPort], sNet2->ipAddr);
    }

	printf("==================================================\n");

	printf(" No. SOE-C   COS-C   COA-C  Unsol  Change    Delay  Offline [ dipt dopt aipt aopt ]\n");
	for(hostid = 0; hostid < MAX_HOST; hostid++)
    {
        host = (HOST_DCB *) hostDCB[hostid];
        printf(" %d  %s  %s  %s   %s    %s   %3d      %3d    %3d  %3d  %3d  %3d\n", hostid+1,
        	classString[host->soeClass],
        	classString[host->cosClass],
        	classString[host->coaClass],
        	onoffType[host->unsolEvent],
        	hostChgMode[host->chgMode],
        	host->comDelay,
        	host->offCount,
        	host->diPtNum, host->doPtNum, host->aiPtNum, host->aoPtNum);
    }

    return(E_OK);
}


/*----------------------------------------------------------------------------
* Function : disp_HarrisPort()
* 내용: PORT 설정 및 PORTDB 상태정보 조회결과를 Console에 출력하는 함수
* helper - cmdTable에 등록되지 않는다.
---------------------------------------------------------------------------- */
void disp_HarrisPort(int port)
{
    short   i;
    byte    *bfptr;

    if(port == 0)
    {
        printf("\n Port Status DISPLAY\n");
        printf("  No.   RTU  TYPE \n");
        for(i = 0; i < MAX_HARRIS_PORT; i++)
        {
            bfptr  = (byte *) &rtudb->portdb[i*2];

            printf(" [%02d]   %02d   %02d  \n", i + 1,  bfptr[0],  bfptr[1]);
        }

        return;
    }
    else if(port > MAX_HARRIS_PORT)
    {
        printf("\n ... invalid Port Number[16] = %d \n", port);
    }
}

/*----------------------------------------------------------------------------
* Function : do_modcfg()
---------------------------------------------------------------------------- */
int do_modcfg(int argc, char *argv[])
{
    short i, j;
    word	baseAddr,baseOffset, dataNum,  tripData, closeData;

    DB_MODBUS_PROFILE  *dbBlock;
	DB_MODBUS_READ	*readBlock, *timeBlock;
	DB_MODBUS_WRITE	*writeBlock;


    printf("\n %s : MODBUS  Configuration ...\n", TARGET_NAME);
    printf("BLK  No. USE  OPC   Addr  Offset  Data   Type   Format ... Trip Close   [Swep:Func]\n");
    for(i = 0; i < MAX_MODBUS_PROFILE; i++)
    {
    	printf("-----------------------------------------------------------\n");
        dbBlock = (DB_MODBUS_PROFILE *) &rtudb->modbusProfile[i];

        for(j=0; j < 8; j++)
        {
        	readBlock = (DB_MODBUS_READ *) &dbBlock->readBlock[j];
        	baseAddr    = readBlock->baseAddr[0]*256 + readBlock->baseAddr[1];
        	baseOffset  = readBlock->baseOffset[0]*256 + readBlock->baseOffset[1];
        	dataNum     = readBlock->dataNum[0]*256 + readBlock->dataNum[1];
        	if(readBlock->useFlag == 0) continue;

        	printf("%02d.  R%d   %d   %2x   %06d %06d  %03d    %s   %s  ...  %04x %04x   [%2d/%2d]\n",
            	i + 1, j+1,
            	readBlock->useFlag, readBlock->opcode, baseAddr, baseOffset, dataNum,
            	modbusType[readBlock->dataType], modbusFormat[readBlock->dataFormat], 0, 0, readBlock->swepEnb, readBlock->function);
        }

        for(j=0; j < 4; j++)
        {
        	writeBlock = (DB_MODBUS_WRITE *) &dbBlock->writeBlock[j];
        	baseAddr    = writeBlock->baseAddr[0]*256 + writeBlock->baseAddr[1];
        	baseOffset  = writeBlock->baseOffset[0]*256 + writeBlock->baseOffset[1];
        	dataNum     = writeBlock->dataNum[0]*256 + writeBlock->dataNum[1];

        	tripData = writeBlock->tripData[0]*256 + writeBlock->tripData[1];
        	closeData= writeBlock->closeData[0]*256 + writeBlock->closeData[1];

        	if(writeBlock->useFlag == 0) continue;

        	printf("%02d.  W%d   %d   %2x   %06d %06d  %03d    %s   %s  ...  %04x %04x\n",
            	i + 1, j+1,
            	writeBlock->useFlag, writeBlock->opcode, baseAddr, baseOffset, dataNum,
            	modbusType[writeBlock->dataType], modbusFormat[writeBlock->dataFormat], tripData, closeData);
        }

        timeBlock = (DB_MODBUS_READ *) &dbBlock->timeBlock;
       	baseAddr    = timeBlock->baseAddr[0]*256 + timeBlock->baseAddr[1];
      	baseOffset  = timeBlock->baseOffset[0]*256 + timeBlock->baseOffset[1];
       	dataNum     = timeBlock->dataNum[0]*256 + timeBlock->dataNum[1];

       	if(timeBlock->useFlag == 0) continue;

       	printf("%02d.  T%d   %d   %2x   %06d %06d  %03d    %s   %s  ...  %04x %04x\n",
           	i + 1, 1,
            timeBlock->useFlag, timeBlock->opcode, baseAddr, baseOffset, dataNum,
            modbusType[timeBlock->dataType], modbusFormat[timeBlock->dataFormat], 0, 0);

    }

    printf("-----------------------------------------------------------\n");
    printf("... Configuration End \n");

    return(E_OK);
}


/*----------------------------------------------------------------------------
* Function : do_pointcfg()
* 내용: CCU에서 장치에 대한 포인트 정보를 담은 정보를 Console에 출력하는 함수
---------------------------------------------------------------------------- */
int do_pointcfg(int argc, char *argv[])
{
    int     i,ch, count=0 ;
    int     devNo, devPt;
    int     ptType, devType, ptConfig;
    int		port1, point1;
    int     dnp_index[MAX_HOST];
    int     dbmax, dbhostpt, modBase, modIndex;
    int     iccpType, iccpIndex;
    byte    ptname[40];

    DB_POINT_BUF    *devPoint;

    printf("\n%s : RTUDB Point Configure ...\n", TARGET_NAME);

    printf("---------------------------------------------------------------------------------------------------------------\n");
    printf(" No.  [ dNo dPt]  TYPE  PT-TY  ICCP-INDX  M-base M-inx Cfg DBMAX [PO PT] LOC HST1 HST2 HST3 HST4 HST5 \n");
    printf("---------------------------------------------------------------------------------------------------------------\n");

    for(i = 0; i < MAX_DBASE_POINT; i++)
    {
        devPoint = (DB_POINT_BUF *) &rtudb->pointBuf[i];
        devNo = devPoint->devNo;
        devPt = (devPoint->devPt[0]*256) + devPoint->devPt[1];

        bzero8248(ptname, 40);

        devType    = devPoint->devType;
        ptType     = devPoint->ptType;
        ptConfig   = devPoint->ptConfig;

        iccpType   = devPoint->iccpPointType;
        iccpIndex  = (devPoint->iccpPointIndex[0]*256) + devPoint->iccpPointIndex[1];

        modBase    = (devPoint->modBase[0]*256) + devPoint->modBase[1];
        modIndex   = (devPoint->modIndex[0]*256) + devPoint->modIndex[1];
        dbmax      = (devPoint->pointMax[0]*256) + devPoint->pointMax[1];
        port1      = devPoint->port;
        point1     = devPoint->point;

        dbhostpt   = (devPoint->localIndex[0]*256) + devPoint->localIndex[1];
        dnp_index[0]= (devPoint->hostIndex[0][0]*256) + devPoint->hostIndex[0][1];
        dnp_index[1]= (devPoint->hostIndex[1][0]*256) + devPoint->hostIndex[1][1];
        dnp_index[2]= (devPoint->hostIndex[2][0]*256) + devPoint->hostIndex[2][1];
        dnp_index[3]= (devPoint->hostIndex[3][0]*256) + devPoint->hostIndex[3][1];
        dnp_index[4]= (devPoint->hostIndex[4][0]*256) + devPoint->hostIndex[4][1];

        memcpy(ptname, devPoint->ptNameStr, 40);
        ptname[39] = '\0';

        printf(" %03d   [%2d %4d] %7s %4s |%4s %4d| %6d  %3d  %2d   %4d  %2d %2d   %2d %4d %4d %4d %4d %4d  %s \n",
            i+1, devNo,  devPt,
            devTypeStr[devType], pointTypeStr[ptType], iccpPTypeStr[iccpType], iccpIndex, modBase, modIndex, ptConfig, dbmax,
            port1, point1, dbhostpt,
            dnp_index[0], dnp_index[1], dnp_index[2],dnp_index[3],dnp_index[4], ptname);


        if(++count == 32)
        {
            count = 0;
            printf("[q]... quit!\n");
            ch=getchar();
            if(ch == 'q')  break;

    printf("---------------------------------------------------------------------------------------------------------------\n");
    printf(" No.  [ dNo dPt]  TYPE  PT-TY  ICCP-INDX  M-base M-inx Cfg DBMAX [PO PT] LOC HST1 HST2 HST3 HST4 HST5 \n");
    printf("---------------------------------------------------------------------------------------------------------------\n");

        }

    }
    printf("\n");

    return(E_OK);
}

/*----------------------------------------------------------------------------
* Function : dispDevice()
* 내용: 각 장치 별 지정된 DI/AI/AO 정보를 표기함
* helper - cmdTable에 등록되지 않는다.
---------------------------------------------------------------------------- */
int dispDevice(int devid)
{
    int     i, j;
    int     ptStart, ptStop;
    char    ch;

    SDP_DEVICE *dev;
    POINT_BUF *diPoint;

    if((devid < 0) || (devid >= MAX_DEVICE))
    {
        printf("*Invalid devid (%d) ...!\n", devid);
        return (0);
    }

    ch = 0;
    while(ch != 'q')
    {
        dev = (SDP_DEVICE *) deviceCFG[devid];

        printf("\n[DEVICE Information] ------------------------[%4d/%2d/%2d %02d:%02d:%02d]\n",
        	((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
        printf(" Device Address   : %2d\n", dev->ioid);
        printf(" [*] Device TYPE  : [%s]\n", scanDevType[dev->type]);
        printf(" [*] Point Config : [DI/DO/AI/Count] - %2d %2d %2d %2d \n", dev->devDiPoint, dev->devDoPoint,dev->devAiPoint, dev->devAoPoint);

        ptStart = 0;
        ptStop  = dev->devDiPoint;
        printf("=> DI Data : [%d - %d] \n", ptStart, ptStop);

        for(i = ptStart, j=0; i < ptStop; i++, j++)
        {
            diPoint = (POINT_BUF *) &dev->diPtBuf[i];

            if(j > 9)
            {
                j = 0;
                printf("\n");
            }

            if(dev->online) printf("[%3d] %2x  ", i+1, diPoint->status);
            else            printf("[%3d] %2x* ", i+1, diPoint->status);

        }

        printf("\n");

        ptStart = 0;
        ptStop  = dev->devAiPoint;
        printf("\n=> AI Data : [%d - %d] \n", ptStart, ptStop);

        dispDevAnalog( devid ,  ptStart);

        printf("to stop  press 'q'\r\n");
        ch = getchar();
    }

    printf("\n");
    return (0);
}


/*----------------------------------------------------------------------------
* Function : dispDevStatus()
* 내용: 각 장치 별 지정된 DI 정보를 표기함
* devid 와 point는 0 기준. helper - cmdTable에 등록되지 않는다.
---------------------------------------------------------------------------- */
void dispDevStatus(int devid, int point)
{
    int     i, j;
    int     ptStart, ptStop;
    char    ch=0;

    SDP_DEVICE  *dev;
    POINT_BUF *diPoint;

    printf("[DEVICE-%02d / Status POINT-%02d]\n", devid+1, point + 1);

    dev = (SDP_DEVICE *) deviceCFG[devid];
    printf("\n[DEVICE Information] ---------------------------[%4d/%2d/%2d %02d:%02d:%02d]\n",
        	((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
    printf(" Device Address   : %2d\n", dev->ioid);
    printf(" [*] Device TYPE  : [%s]\n", scanDevType[dev->type]);
    printf(" [*] Point Config : [DI/DO/AI/Count] - %2d %2d %2d %2d \n", dev->devDiPoint, dev->devDoPoint,dev->devAiPoint, dev->devAoPoint);
    printf("--------------------------------------------------------------------\n");

    while(ch != 'q')
    {
        ptStart = point;
        ptStop  = point + 8;
        printf("=> STATUS Data [%3d - %3d] : ", ptStart+1, ptStop);

        for(i = ptStart, j=0; i < ptStop; i++, j++)
        {
            diPoint = (POINT_BUF *) &dev->diPtBuf[i];

            if(j > 9)
            {
                j = 0;
                printf("\n");
            }

            if(dev->online) printf("[%3d] %2x  ", i+1, diPoint->status);
            else            printf("[%3d] %2x* ", i+1, diPoint->status);

        }
        printf("\n");
        ch = getchar();
    }

    printf("\n\n");
}

/*----------------------------------------------------------------------------
* Function : dispDevAnalog()
* helper - cmdTable에 등록되지 않는다.
---------------------------------------------------------------------------- */
void dispDevAnalog(int devid, int point)
{
    int     i, j;

    SDP_DEVICE  *dev;
    POINT_BUF *aiPoint;

    printf("[DEVICE-%02d / Analog POINT-%02d]\n", devid+1, point + 1);

    dev = (SDP_DEVICE *) deviceCFG[devid];
    printf("\n[DEVICE Information] ---------------------------[%4d/%2d/%2d %02d:%02d:%02d]\n",
        	((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
    printf(" Device Address   : %2d\n", dev->ioid);
    printf(" [*] Device TYPE  : [%s]\n", scanDevType[dev->type]);
    printf(" [*] Point Config : [DI/DO/AI/Count] - %2d %2d %2d %2d \n", dev->devDiPoint, dev->devDoPoint,dev->devAiPoint, dev->devAoPoint);
    printf("--------------------------------------------------------------------\n");

    for(i = point,j=1; i < dev->devAiPoint; i++, j++)
    {
        char *ptrChar ;
        float  fvalue  ;
        size_t offset;

        ptrChar = (char *)&dev->aiPtBuf[i];
        offset = offsetof(POINT_BUF, floatData);
        memcpy(&fvalue, ptrChar + offset, sizeof(float));
        printf("[%3d] %4.2f ", i+1, fvalue);

        if( (j % 10) == 0 )
        {
            printf("\n");
        }

    }

    printf("\n\n");
}


/*----------------------------------------------------------------------------
* Function : do_dnpdi()
* getDecWord()로 HOST 번호를 대화형으로 받는다 - 본문은 원본 그대로 유지.
---------------------------------------------------------------------------- */
int do_dnpdi(int argc, char *argv[])
{
    int     hostid,point;
    int     index;
    int     devNo, pointNo;
    char            ch;
    HOST_DCB        *host;
    DIPOINT_INFO    *hostSts;
    SDP_DEVICE      *dev;
    POINT_BUF       *diPoint;

    printf("======> HOST Number [MAX = %d] : ", MAX_HOST);
    hostid = getDecWord();

    printf("\n============================================\n");
    printf(" DNP-HOST(%d) 상세포인트 Config 정보... \n", hostid);
    printf("============================================\n");

    if(hostid <= MAX_HOST)  hostid = hostid - 1;
    else                    return (E_OK);

    index = 0;
    host = (HOST_DCB *) hostDCB[hostid];

    printf("\n[HOST %02d] State Info--------------\n", hostid+1);
    printf(" DNP-Inx. [ dNo dPt] config \n");
    printf("----------------------------------\n");

    if(host->hostProtocol != HOST_DNP)      return (E_OK);

    for(point = 0; point < MAX_DNP_DI_POINT; point++)
    {
        hostSts = (DIPOINT_INFO *) &host->stateInfo[point];
        if(hostSts->config == RESET)   continue;

        devNo   = hostSts->devNo;
        pointNo = hostSts->devPt;

        dev = (SDP_DEVICE *) deviceCFG[devNo - 1];
        diPoint = (POINT_BUF *) &dev->diPtBuf[pointNo - 1];

        printf("   %02d     [%3d  %3d]   %2d    %s \n", point+1, hostSts->devNo, hostSts->devPt, hostSts->config, diPoint->ptNameStr);

        if(++index == 32)
        {
            index = 0;
            ch= getchar();
            if(ch == 'q')   break;
        }
    }
    return (E_OK);
}

int    do_dnpai(int argc, char *argv[])
{
    int     hostid,point;
    int     index;
    int     devNo, pointNo;
    char            ch;
    HOST_DCB        *host;
    AIPOINT_INFO    *hostAna;
    SDP_DEVICE      *dev;
    POINT_BUF       *aiPoint;

    printf("======> HOST Number [MAX = %d] : ", MAX_HOST);
    hostid = getDecWord();

    printf("\n============================================\n");
    printf(" DNP-HOST(%d) 아날로그포인트 Config 정보... \n", hostid);
    printf("============================================\n");

    if(hostid <= MAX_HOST)  hostid = hostid - 1;
    else                    return (E_OK);

    index =0;
    host = (HOST_DCB *) hostDCB[hostid];

    printf("\n[HOST %02d] Analog Info--------------\n", hostid+1);
    printf(" DNP-Inx. [ dNo dPt] config \n");
    printf("----------------------------------\n");

    if(host->hostProtocol != HOST_DNP)      return (E_OK);

    for(point = 0; point < MAX_DNP_AI_POINT; point++)
    {
        hostAna = (AIPOINT_INFO *) &host->analogInfo[point];
        if(hostAna->config == RESET)   continue;

        devNo   = hostAna->devNo;
        pointNo = hostAna->devPt;

        dev = (SDP_DEVICE *) deviceCFG[devNo - 1];
        aiPoint = (POINT_BUF *) &dev->aiPtBuf[pointNo - 1];

        printf("   %02d     [%3d  %3d]   %2d   %s \n", point+1, hostAna->devNo, hostAna->devPt, hostAna->config, aiPoint->ptNameStr);

        if(++index == 32)
        {
            index = 0;
            ch= getchar();
            if(ch == 'q')   break;
        }
    }
    return (E_OK);
}

int do_dnpdo(int argc, char *argv[])
{
    int     hostid,point;
    int     index;
    int     devNo, pointNo;
    char            ch;
    HOST_DCB        *host;
    CONTROL_INFO    *hostCntr;
    SDP_DEVICE      *dev;
    POINT_BUF       *doPoint;

    printf("======> HOST Number [MAX = %d] : ", MAX_HOST);
    hostid = getDecWord();

    printf("\n============================================\n");
    printf(" DNP-HOST(%d) 제어포인트 Config 정보... \n", hostid);
    printf("============================================\n");

    if(hostid <= MAX_HOST)  hostid = hostid - 1;
    else                    return (E_OK);

    host = (HOST_DCB *) hostDCB[hostid];

    printf("\n[HOST %02d] Control Info--------------\n", hostid+1);
    printf(" DNP-Inx. [ dNo dPt] config  dbMax \n");
    printf("----------------------------------\n");

	if(host->hostProtocol != HOST_DNP)      return (E_OK);

    index = 0;
    for(point = 0; point < MAX_DNP_DO_POINT; point++)
    {
        hostCntr = (CONTROL_INFO *) &host->controlInfo[point];
        if(hostCntr->config == RESET)   continue;

        devNo   = hostCntr->devNo;
        pointNo = hostCntr->devPt;

        dev = (SDP_DEVICE *) deviceCFG[devNo - 1];
        doPoint = (POINT_BUF *) &dev->doPtBuf[pointNo - 1];

        printf("   %02d     [%3d  %3d]   %2d    %3d  %s\n", point+1, hostCntr->devNo, hostCntr->devPt, hostCntr->config, hostCntr->dbmax, doPoint->ptNameStr);

        if(++index == 32)
        {
            index = 0;
            ch= getchar();
            if(ch == 'q')   break;
        }
    }
    return (E_OK);
}


/*----------------------------------------------------------------------------
* Function : do_devcfg()
---------------------------------------------------------------------------- */
int do_devcfg(int argc, char *argv[])
{
    int         index;
    int             point;
    char            ch;
    POINT_BUF       *devPt;

    printf("\n[SDP-DEVICE Point List] -----------------------------------------------\n" );
    printf(" D.No [ dNo dPt] cfg  ICCP Index HST1 HST2 HST3 HST4 HST5 [STS] \n");
    printf("-----------------------------------------------------------------------\n");
    index = 0;
    for(point = 0; point < MAX_DEV_POINT; point++)
    {
        devPt = (POINT_BUF *) devPtBuf[point];

        if(devPt->config == 0)  continue;
        printf(" %02d    [%2d  %3d]   %2d  %s  %3d  %3d  %3d  %3d  %3d  %3d  [ %d ]  %s\n", point +1, devPt->devNo, devPt->devPt, devPt->config,
            iccpPTypeStr[devPt->iccpType], devPt->iccpIndex, devPt->hostIndex[0], devPt->hostIndex[1],
            devPt->hostIndex[2], devPt->hostIndex[3], devPt->hostIndex[4], devPt->status, devPt->ptNameStr);

        if(++index == 48)
        {
            index = 0;
            ch= getchar();
            if(ch == 'q')   break;
        }
    }
    printf("-----------------------------------------------------------------------\n\n");
    return (E_OK);
}

/*----------------------------------------------------------------------------
* Function : do_calcfg()
---------------------------------------------------------------------------- */
int do_calcfg(int argc, char *argv[])
{
    int         index;
    int             point;
    char            ch;
    CAL_POINT_BUF       *calPt;

    printf("\n[SDP-CAL Point List] -----------------------------------------------\n" );
    printf(" C.No [dNo dPt] CFG USE [ TYPE ] Time Fun : CAL-String\n");
    printf("-----------------------------------------------------------------------\n");
    index = 0;
    for(point = 0; point < MAX_CAL_POINT; point++)
    {
        calPt = (CAL_POINT_BUF *) calPtBuf[point];

        printf("  %02d  [%2d  %3d] %2d  %2d  %7s  %2d   %2d  %s\n", point +1, calPt->devNo, calPt->devPt, calPt->config,
            calPt->useFlag, calPtType[calPt->pointType], calPt->calcTime, calPt->function, calPt->calString);

        if(++index == 32)
        {
            index = 0;
            ch= getchar();
            if(ch == 'q')   break;
        }
    }
    printf("-----------------------------------------------------------------------\n\n");
    return (E_OK);
}


/*----------------------------------------------------------------------------
* Function : dispHostData_DNP()
* 내용: 각 HOST 별 지정된 DI/AI/AO 정보를 표기함
* helper - cmdTable에 등록되지 않는다.
---------------------------------------------------------------------------- */
void dispHostData_DNP(int hostid)
{
    int     i, j;
    int     online;
    int     ptStart, ptStop;
    char    ch;
    byte    temp[4];
    float   *fptr;

    HOST_DCB        *host;
    DNP_ANA_INPUT   *anaPoint;
    DNP_SOE_QUEUE   *dnpSOEQ;

    ch = 0;
    while(ch != 'q')
    {
        host = (HOST_DCB *) hostDCB[hostid];

        if(host->hostDualMode == HOST_NOT_USE)	break;
        if(host->hostProtocol != HOST_DNP)      break;

        dnpSOEQ = (DNP_SOE_QUEUE *) &host->dnpSOEQ;

        printf("\n[HOST%2d Information] ------------------------[%4d/%2d/%2d %02d:%02d:%02d]\n",
        	hostid +1, ((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
        printf("[*] HOST RUN Mode = [%s]\n", hostMode[host->hostDualMode]);
        printf("[*] HOST Protocol = [%s]\n", hostPTType[host->hostProtocol]);
        printf("[*] HOST Address  = HOST  [%d]/CU   [%d]\n", host->hostid, host->rtuAddr);
        printf("[*] HOST Word Rpt = [DI=%d, AI=%d]\n", host->diIndexWord, host->aiIndexWord);
        printf("[*] HOST SOE-Queue= front=%2d/rear=%2d \n", dnpSOEQ->front, dnpSOEQ->rear);
        printf("[*] PT [DI/DO/AI/AO]= %2d/%2d/%2d/%2d\n\n", host->diPtNum,host->doPtNum,host->aiPtNum, host->aoPtNum);

        ptStart = 0;
        ptStop  = host->diPtNum;

        printf("=> DI Data : [%d - %d] \n", ptStart, ptStop);

        for(i = ptStart, j=0; i < ptStop; i++, j++)
        {
            online = host->sts_pointData[i] & PT_FLAG_ONLINE;

            if(j > 9)
            {
                j = 0;
                printf("\n");
            }

            if(online)  printf("[%3d] %2x  ", i+1, host->sts_pointData[i]);
            else        printf("[%3d] %2x* ", i+1, host->sts_pointData[i]);

        }

        printf("\n");

        ptStart = 0;
        ptStop  = host->aiPtNum;
        printf("\n\n=> AI Data : [%d - %d] \n", ptStart, ptStop);
        for(i = ptStart,j=1; i < ptStop; i++, j++)
        {
            anaPoint = (DNP_ANA_INPUT *) &hostDCB[hostid]->ana_pointData[i];

            {
                float a ;

                a=BITS_TO_FLOAT (anaPoint->lowdata[0],anaPoint->lowdata[1],anaPoint->lowdata[2],anaPoint->lowdata[3]);
                printf("[%3d] %05.2f(%d) ", i, a, anaPoint->flag);
            }

            if(j == 8)
            {
                j = 0;
                printf("\n");

            }
        }
        printf("\n");

        ch = getchar();
    }

    printf("\n");
}
