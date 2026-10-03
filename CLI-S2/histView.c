//==============================================================================
//
// File:    histView.c                                          2026-09-03
//
// FRAM이 없는 신규 보드에서 /mnt/bin/History.Bin(design/history-fram-to-file.md
// 참고)을 "SDP 프로세스가 하나도 안 떠 있어도" 직접 읽어서 화면에 보여주는
// 독립 실행 조회 전용 도구.
//
//  - CLI-S2 의 do_event() 는 살아있는 프로세스의 공유메모리(hque)를 봐야
//    하므로 SDP 시스템이 기동돼 있어야만 이력을 볼 수 있다.
//  - 이 histView 는 그 반대로, WDT/CLI/그 외 어떤 SDP 프로세스도 떠 있지
//    않은 상태(정비 중, 혹은 방금 재부팅 직후 진단하려는 상황)에서도
//    /mnt/bin/History.Bin 파일 하나만 읽어서 이력을 보여준다.
//
//  [2026-09-03 설계 변경] 처음에는 이벤트 출력 로직(PrintHistoryQueue())을
//  CLI-S2/cliCmds.c 의 do_event() 와 공용으로 쓰려고 lib/lib_localLib.c
//  (모든 실행파일이 링크하는 libLocal.a)로 옮겼었다. 하지만:
//   - deviceCFG[]/devPtBuf[]/esioCFG[] 는 공유메모리(shmPtr)에 attach 해서
//     초기화해야만 의미 있는 값이 된다(cliMain.c 의
//     for(...) esioCFG[i] = &shmPtr->esioConfig[i]; 등 참고).
//   - 이 histView 는 공유메모리에 아예 attach 하지 않으므로(그게 이
//     도구의 존재 이유다), 이 배열들은 여기서는 항상 NULL이라 장치/
//     포인트 이름 조회 코드는 원래부터 죽은 코드였다.
//   - 그런데도 lib_localLib.c 가 이 심볼들을 extern 참조하는 바람에
//     WDT/SCAN/HOST/LINK/SCU/SIM/ICCP/EXIT 등 이 기능과 무관한 모든
//     실행파일까지 얽혔고, 그중 이 배열들을 안 쓰는 ICCP(예전 개발자가
//     #if 0 // CHOIBC DELETE 로 꺼둔 상태)에서 실제로 링크 에러가 났다.
//  그래서 lib_localLib.c 는 원래대로 되돌리고(이 기능과 관련해 더 이상
//  아무것도 안 건드림), CLI-S2/cliCmds.c 의 do_event() 는 자기 파일 안의
//  static PrintHistoryQueue() 를 쓰고, 이 histView 는 장치/포인트 이름
//  조회 없이 History.Bin 의 원시 정보(이벤트 코드/장치번호/포인트번호/
//  상태/시각)만 출력하는 완전히 독립적인 PrintHistoryFile() 을 쓴다.
//  다른 실행파일이 정의하는 전역변수를 전혀 참조하지 않으므로 이제
//  이 파일 하나만으로(그리고 libLocal.a 의 gensum/errmsg 정도만 빌려서)
//  컴파일·링크가 끝난다. (design/history-fram-to-file.md 참고)
//
//==============================================================================

#include    "localLib.h"

/* 2026-09-03 : errmsg[] (이벤트 이름 문자열 테이블, lib/lib_logFile.c 에서
 * 실제 정의됨) 을 참조하기 위해 추가. localLib.h 는 이 extern 선언을
 * 포함하지 않는다. deviceCFG[]/devPtBuf[]/esioCFG[] 도 같이 선언되지만
 * PrintHistoryFile() 은 그 심볼들을 쓰지 않으므로 링크 요구사항이
 * 생기지 않는다(extern 선언만으로는 링커가 실제 정의를 요구하지 않음). */
#include    "external.h"


/*
 * PrintUsage() : 잘못된 인자로 실행했을 때 도움말 출력
 */
static void PrintUsage(const char *prog)
{
    printf("Usage: %s [all|soe|control|comm]\n", prog);
    printf("  History.Bin(%s) 을 직접 읽어 이력을 출력합니다.\n", HIST_FILE_PATH);
    printf("  all     : 전체 이력 출력 (기본값)\n");
    printf("  soe     : SOE 이벤트만 출력\n");
    printf("  control : 제어(CONTROL) 이벤트만 출력\n");
    printf("  comm    : 통신(COMM) 이벤트만 출력\n");
}


/*
 * ParseMode() : 커맨드라인 인자를 mode 값(0=전체, 1=SOE, 2=CONTROL, 3=COMM)
 *               으로 변환한다. 인자가 없거나 알 수 없는 값이면 0(전체).
 */
static int ParseMode(int argc, char *argv[])
{
    if (argc < 2)   return (0);

    if (strcmp(argv[1], "soe") == 0)        return (1);
    if (strcmp(argv[1], "control") == 0)    return (2);
    if (strcmp(argv[1], "comm") == 0)       return (3);
    if (strcmp(argv[1], "all") == 0)        return (0);

    /* "-h"/"--help" 등 그 외 값은 도움말 출력 후 종료시키기 위해
     * ParseMode 가 아니라 main() 쪽에서 먼저 걸러낸다. 여기까지 왔다는
     * 것은 알 수 없는 값이 들어왔다는 뜻이므로 기본값(전체)으로 처리. */
    printf("[WARN] unknown mode '%s' -> treat as 'all'\n", argv[1]);
    return (0);
}


/*
 * ====================================================================
 *  PrintHistoryFile()                                      2026-09-03
 * ====================================================================
 *  History.Bin 에서 읽어들인 HISTORY_QUE 스냅샷을 최신 이벤트부터
 *  화면에 출력한다. CLI-S2/cliCmds.c 의 PrintHistoryQueue() 와 필터링/
 *  이벤트별 분기 구조는 같지만, 장치/포인트/ESIO 이름 조회(deviceCFG[]/
 *  devPtBuf[]/esioCFG[])는 하지 않는다 - histView 는 공유메모리에
 *  attach 하지 않아서 그 배열들이 항상 NULL이므로, 애초에 조회할 수
 *  있는 이름이 없다(위 파일 상단 주석의 "2026-09-03 설계 변경" 참고).
 *  대신 이벤트 코드/장치번호/포인트번호/상태/호스트/시각 등 파일에
 *  그대로 들어있는 원시 정보만 출력한다 - 진단 목적에는 이걸로 충분하다.
 *
 *  hq   : 출력할 HISTORY_QUE (History.Bin 에서 읽어들인 스냅샷)
 *  mode : 0=전체, 1=SOE, 2=CONTROL, 3=COMM 필터
 *  파일 리다이렉트/스크립트에서 쓰기 좋도록 페이징 없이 끝까지 한
 *  번에 출력한다(대화형 키 입력 대기 없음).
 */
static void PrintHistoryFile(HISTORY_QUE *hq, int mode)
{
    int             id, ioid, point, state, hostid;
    word            front;
    SYSLOG_FORM     *log;
    struct tm       cTime, sTime;
    int             milisec1, milisec2;
    const char      *tag;

    printf(">> Event Number   : front = %3d ... over =%d [MAX %d]\n",
           hq->front, hq->overlab, HISTORY_QUE_MAX);
    printf("\n");

    front = hq->front & HISTORY_QUE_MASK;

    for ( ; ; )
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
        /*  이벤트 코드에 따라 태그(SOE/DEV/CNT/VME/COM)와   */
        /*  SOE 시각 유무만 구분한다 - 이름 조회는 없다.      */
        /* ------------------------------------------------ */
        if (id == ENT_SOE)                                    tag = "SOE";
        else if (id == ENT_DEVICE_SOE)                         tag = "DEV";
        else if ((id >= ENT_ICCP_CNTR) && (id <= ENT_LINK_CNTR))    tag = "CNT";
        else if ((id >= ENT_VME_ONLINE) && (id <= ENT_VME_UNINSTALL)) tag = "VME";
        else if ((id >= ENT_ESIO_ONLINE) && (id <= ENT_ESIO_OFFLINE)) tag = "COM";
        else if ((id == ENT_DEV_ONLINE) || (id == ENT_DEV_OFFLINE))   tag = "COM";
        else                                                    tag = NULL;

        if (tag == NULL)
        {
            /* 그 외 시스템 이벤트... 태그/2차 시각 없이 기본 정보만 */
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon + 1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1,
                errmsg[id], ioid, point, state, hostid);
        }
        else if ((id >= ENT_VME_ONLINE) && (id <= ENT_VME_UNINSTALL))
        {
            /* VME 관련 이벤트는 원래도 2차(soe) 시각이 없다 */
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] %s: (device name unavailable) \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon + 1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1,
                errmsg[id], ioid, point, state, hostid, tag);
        }
        else
        {
            /* SOE/DEV/CNT/COM 은 2차(soe) 시각까지 함께 찍힌다 */
            localtime_r(&log->soeTime.tv_sec, &sTime);
            milisec2 = log->soeTime.tv_usec / 1000;

            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] %s:%04d/%02d/%02d-%02d:%02d:%02d-%03d (device name unavailable) \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon + 1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1,
                errmsg[id], ioid, point, state, hostid, tag,
                sTime.tm_year + 1900, sTime.tm_mon + 1, sTime.tm_mday, sTime.tm_hour, sTime.tm_min, sTime.tm_sec, milisec2);
        }
    }

    printf("\n [ EVENT-Q ] end ---------------------------------------------\n");
}


int main(int argc, char *argv[])
{
    int             fd;
    ssize_t         rdLen;
    word            calcSum;

    /* 44KB 가까이 되는 HISTORY_QUE 를 스택에 두면(특히 임베디드 보드의
     * 작은 기본 스택 사이즈에서) 스택 오버플로우 위험이 있으므로 static
     * (BSS) 으로 잡는다. */
    static HISTORY_QUE  loadedHque;

    if ((argc >= 2) && ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)))
    {
        PrintUsage(argv[0]);
        return (0);
    }

    /* ------------------------------------------------------------ */
    /* (1) History.Bin 을 읽기 전용으로 연다. WDT 프로세스가 쓰고     */
    /*     있는 도중에 읽어도(O_RDONLY 는 WDT 의 쓰기 fd 와 별개이므로) */
    /*     문제 없다 - 다만 아주 드물게 WDT가 pwrite 하는 중간에      */
    /*     읽으면 그 한 건만 절반만 갱신된 상태로 보일 수 있는데,      */
    /*     그건 아래 (3) 체크섬 검증으로 걸러낸다.                    */
    /* ------------------------------------------------------------ */
    fd = open(HIST_FILE_PATH, O_RDONLY);
    if (fd < 0)
    {
        printf("[ERROR] %s open fail - WDT가 아직 한 번도 기동한 적이 없거나,\n", HIST_FILE_PATH);
        printf("        /mnt 파티션이 마운트되지 않은 상태일 수 있습니다.\n");
        return (1);
    }

    /* ------------------------------------------------------------ */
    /* (2) 파일 전체(HISTORY_QUE 구조체 크기)를 통째로 읽어들인다.    */
    /* ------------------------------------------------------------ */
    rdLen = read(fd, &loadedHque, sizeof(HISTORY_QUE));
    close(fd);

    if (rdLen != (ssize_t) sizeof(HISTORY_QUE))
    {
        printf("[ERROR] %s read fail (read=%ld byte, need=%d byte)\n",
               HIST_FILE_PATH, (long) rdLen, (int) sizeof(HISTORY_QUE));
        printf("        파일 크기가 HISTORY_QUE 구조체 크기와 다릅니다 - 아직 WDT가\n");
        printf("        한 번도 초기화하지 않았거나 파일이 손상됐을 수 있습니다.\n");
        return (1);
    }

    /* ------------------------------------------------------------ */
    /* (3) 체크섬 검증 - WDT/wdtMain.c 의 updateHistoryQ()/clearHistoryQ() */
    /*     가 쓸 때와 동일한 방식(gensum)으로 계산해서 비교한다.       */
    /*     깨진 파일이라도 일단 읽을 수 있는 데이터는 최대한 보여주고 */
    /*     경고만 띄운다(진단 목적이므로 "안전하게 실패"보다는          */
    /*     "가능한 만큼 보여주기"가 더 유용하다고 판단).                */
    /* ------------------------------------------------------------ */
    calcSum = gensum((byte *) &loadedHque, sizeof(HISTORY_QUE) - 2);
    if (calcSum != loadedHque.chksum)
    {
        printf("*** WARNING: checksum mismatch (file=%04x, calc=%04x) ***\n",
               loadedHque.chksum, calcSum);
        printf("*** History.Bin 이 손상됐거나, WDT가 쓰는 도중에 읽었을 수 있습니다. ***\n");
        printf("*** 아래 내용은 참고용으로만 사용하세요.                              ***\n\n");
    }

    printf("\n*** History.Bin File Viewer (%s) ***\n", HIST_FILE_PATH);
    printf(">> SDP Controller : %s \n", TARGET_NAME);
    printf(">> Model Name     : %s\n", VERSION_STRING);
    printf(">> Update Date    : %s\n", DATE_STRING);
    printf("\n");

    PrintHistoryFile(&loadedHque, ParseMode(argc, argv));

    return (0);
}
