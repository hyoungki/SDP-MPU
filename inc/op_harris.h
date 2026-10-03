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

#ifndef	OP_HARRIS_HEADER_INCLUDED
#define	OP_HARRIS_HEADER_INCLUDED

#define MAX_HARRIS_RTU  	2
#define MAX_HARRIS_PORT     16
#define MAX_HARRIS_POINT    64

/* ------------------------ */
/*  HARRIS BIT Constant     */
/* ------------------------ */
#define OFFLINE     1
#define OL          0x01        // Offline
#define PF          0x02        // Power Fail Reset
#define MF          0x04        // Message Fail
#define SOE_S       0x08        // SOE set
#define SOEOF       0x10        // SOE Overflow
#define COS_S       0x20        // COS set
#define COSOF       0x40        // COS Overflow
#define CNTFALL     0x80        // Control Fail


/* ------------------------------------------------ */
/* HARRIS - PORT Type defination                    */
/* ------------------------------------------------ */
//#define NULL        0       /* Null Port        */
#define CAI         1       /* C&I Port         */
#define ANA         2       /* Analog Port      */
#define ACC         3       /* Accumulator Port */

/* Control Constant Defination */
#define ARM             0x6
#define OPR             0x7
#define ARM_RUN         0x8
#define OPR_RUN         0x9


/* ------------------------ */
/*  HARRIS OPCODE           */
/* ------------------------ */

/* ---------------------------------------------------- */
/* STURECTURE 정의 : COS_Q                              */
/* 1. HARRIS 용 COS 이벤트 저장용 Buffer 관련 스트럭쳐  */
/* ---------------------------------------------------- */   
typedef struct
        {
        byte    front;
        byte    rear;
        byte    report;
        byte    dummy;
        
        byte    que[256+2][2];
        } __attribute__ ((packed)) HARRIS_COS_Q ;


/* ---------------------------------------------------- */
/* STURECTURE 정의 : SOE_Q                              */
/* 1. HARRIS 용 SOE 이벤트 저장용 Buffer 관련 스트럭쳐  */
/* ---------------------------------------------------- */    
typedef struct
        {
        byte    front;
        byte    rear;
        byte    report;
        byte    dummy;
        
        byte    que[256+2][8];
        byte    lastSOE[8];
        } __attribute__ ((packed)) HARRIS_SOE_Q ;

/* ---------------------------------------------------- */
/* STURECTURE 정의 : PORT_DB                            */
/* 1. HARRIS 용 포트 내부 데이터 관련 스트럭쳐          */
/* ---------------------------------------------------- */   
typedef struct
        {
        short   type;                       /* C&I, IND, ANA, ACC, CTL */
        short   portStatus;
        short   rtuid;
        short   harrisPort;
        short   pointData[MAX_HARRIS_POINT];       /* current scaned data */

        } __attribute__ ((packed)) PORT_DB;

/* ---------------------------------------------------- */
/* STURECTURE 정의 : RTU                                */
/* 1. HARRIS 용 원격소장치별 내부 데이터 관련 스트럭쳐  */
/* ---------------------------------------------------- */   
typedef struct
        {
        short   id;
        short   type;
        short   status;
        short   max_port;
        short   dataPortCnt;
        short   ciPortCnt;
        short   rts_delay;
        short   tx_delay;
        short   preOpcode;
        short   reportcnt;

        byte    rxbuf[32];
        byte    reportbuf[1024];
        byte    cmdSize[34];

        HARRIS_COS_Q   cosq;
        HARRIS_SOE_Q   soeq;
        
        } __attribute__ ((packed)) RTU;

#endif	// OP_HARRIS_HEADER_INCLUDED
