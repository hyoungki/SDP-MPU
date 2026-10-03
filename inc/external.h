/*
 * ==============================================================
 * System TARGET : ACE Control SDP-2000 Ver 1.0   
 * Target CPU    : MPC8248, VME6U
 * Main Factors  : 스마트급전용 SDP 자장치 
 *     - MPU 이중화 구성
 *     - ESOP/SIO 연계 : 전력감시, 원격진단, 전력품질, 고장점 등
 *     - 통합 시뮬레이터 : 2013/07/08 V8.2 적용 
 * --------------------------------------------------------------
 * System DESIGN : SANE-SYSTEM   .... by  Lee Ho-Sang
 * Initial-DATA  : 2016,03,25
 * Last Updated  : 2016,03,25
 * ==============================================================
 */

extern  SHM_MEMORY	    *shmPtr;

extern  RTC             *rtc;
extern  OPR_MSG         *opr;
extern  RTU_DATABASE    *rtudb;                     // SDP 데이터베이스
extern  CONSOLE_INFO	*console;

extern  HISTORY_QUE     *hque;                      // CONSOLE 용 이벤트

extern  LINK_MSG        *linkCfg;                   // CPU 이중화 구조체
extern  SCU_MSG         *scuCfg;                    // 이중화 절체장치(SCU) 구조체

extern  ICCP_DCB        *iccpDCB;                   // ICCP-HOST 구조체
extern  ICCP_CONFIG     *iccpCFG;                   // ICCP-HOST 구조체
extern  ICCP_60870_DCB  *iccpInfo;                  // ICCP-HOST 참조용 : 모니터링 구조체

extern  MPU_CONFIG      *mpuCFG;                    // MPU Config
extern  ESIO_CONFIG     *esioCFG[MAX_ESIO];         // ESIO 장치 Config

extern  HOST_DCB        *hostDCB[MAX_HOST];         // HOST 관련 구조체 : ICCP, DNP, HARRIS, LANDIS...
extern	RTU             *rtubuf[MAX_HARRIS_RTU];        /* HARRIS RTU Structure */
extern	PORT_DB         *portdb[MAX_HARRIS_PORT];       /* HARRIS #1 PORT Structure */

extern  POINT_BUF       *devPtBuf[MAX_DEV_POINT];   // SDP 포인트 Config 정보
extern  CAL_POINT_BUF   *calPtBuf[MAX_CAL_POINT];   // SDP 연산 포인트 Config 정보

extern  SCAN_CONFIG     *scanCFG[MAX_SCAN_PORT];    // 하위계전기 SCAN Config
extern  SDP_DEVICE      *deviceCFG[MAX_DEVICE];     // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)

//extern  RTU_CONFIG      *rtuDCB;                    // 하부 RTU 운영 구조체

extern  MPU_SOE_QUEUE   *mpuSOE;                    // MPU SOE Buffer
extern  MPU_COS_QUEUE   *mpuSOE;                    // MPU SOE Buffer

extern  byte    errmsg[NUMBER_OF_EVENT][20];