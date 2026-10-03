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

#ifndef	OP_RTUSIM_HEADER_INCLUDED
#define	OP_RTUSIM_HEADER_INCLUDED

#define CCU_STS_PFR         0x01        /* Status of POWER-FAIL */
#define CCU_STS_OFL         0x02        /* Status of OFFLINE-DEVICE */

#define SIM_ICCP_DEL_SOE    0x42
#define SIM_ICCP_DEL_ACK    0x43
    
/* ------------------------------------ */
/* RTU SIMULATOR Protocol Constant      */
/* ------------------------------------ */
#define SIM_SDP_GPOLL       0x00        // SDP 장치 - GPOLL

#define SIM_SDP_STATUS      0x01        // SDP 장치상태정보 Dump
#define SIM_EVENT_DUMP      0x02        // SDP 상태 EVENT Dump
#define SIM_EVENT_ACK       0x03        // SDP 상태 EVENT-ACK
#define SIM_POINT_SCAN      0x04        // 계전기 POINT 정보 Scan
#define SIM_POINT_CNTR      0x05        // 계전기 POINT 제어
#define SIM_SYSTEM_CNTR     0x06        // SYSTEM 절체 제어

#define SIM_MANUAL_STATUS   0x07        // 수동기입 : 상태 포인트
#define SIM_MANUAL_ANALOG   0x08        // 수동기입 : 계측 포인트

#define SIM_COA_DUMP        0x0a        // 아날로그 COS 전송
#define SIM_COA_ACK         0x0b        // 아날로그 COS 확인


#define SIM_TIME_DOWN       0x11        // Time Sync Down
#define SIM_TIME_UP         0x12        // Time Sync Up

#define SIM_ANALOG_SCAN     0x14        // ESIO : 계전기 아날로그 정보 Scan
#define SIM_CHKSUM_DUMP     0x16        // ESIO : 데이터베이스 Chksum Scan
#define SIM_RESTART_ESIO    0x18        // ESIO : 재기동 

#define SIM_MPUCFG_DOWN     0x20        // DB : MPU Config Down
#define SIM_MPUCFG_UP       0x21        // DB : MPU Config UP
#define SIM_ESIOCFG_DOWN    0x22        // DB : ESIO Config Down
#define SIM_ESIOCFG_UP      0x23        // DB : ESIO Config UP

#define SIM_HOST_DOWN       0x24        // DB : HOST Config Down
#define SIM_HOST_UP         0x25        // DB : HOST Config UP
#define SIM_ICCP_DOWN       0x26        // DB : ICCP-HOST Config Down
#define SIM_ICCP_UP         0x27        // DB : ICCP-HOST Config UP

#define SIM_HARRIS_DOWN     0x28        // DB : HARRIS Config Down
#define SIM_HARRIS_UP       0x29        // DB : HARRIS Config UP
#define SIM_LANDIS_DOWN     0x2A        // DB : LANDIS Config Down
#define SIM_LANDIS_UP       0x2B        // DB : LANDIS Config UP
#define SIM_MODBUS_DOWN     0x2C        // DB : MODBUS Config Down
#define SIM_MODBUS_UP       0x2D        // DB : MODBUS Config UP

#define SIM_SCAN_DOWN       0x30        // DB : SCAN Config Down
#define SIM_SCAN_UP         0x31        // DB : SCAN Config UP
#define SIM_DEVICE_DOWN     0x32        // DB : DEVICE Config Down
#define SIM_DEVICE_UP       0x33        // DB : DEVICE Config UP
#define SIM_POINT_DOWN      0x34        // DB : POINT Config Down
#define SIM_POINT_UP        0x35        // DB : POINT Config UP

#define SIM_CAL_POINT_DOWN  0x36        // DB : 연산포인트 구성정보 Down
#define SIM_CAL_POINT_UP    0x37        // DB : 연산포인트 구성정보 Up

#endif	// OP_RTUSIM_HEADER_INCLUDED

