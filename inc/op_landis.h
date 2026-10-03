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

#ifndef	OP_LANDIS_HEADER_INCLUDED
#define	OP_LANDIS_HEADER_INCLUDED

/* ---------------------------------------------------- */
/* LANDIS HOST 용 STURECTURE 정의 : POINTBUF            */
/* 1. DI/DO/AI등 CU내 포인트 운영을 위한 스트럭쳐       */
/* ---------------------------------------------------- */   
#define MAX_HOST_STS_PT     256            // LANDIS HOST 용 상태 Report 
#define MAX_HOST_ANA_PT     256            // LANDIS HOST 용 아날로그 Report 


/* LANDIS Card 형식 : RTU Simulator Constant */
#define LANDIS_NULL         0
#define LANDIS_ANA          1
#define LANDIS_SOE          2
#define LANDIS_ACC          3

/* LANDIS Chassis-Card Constant */
#define LANDIS_CARD_NULL    0
#define LANDIS_CARD_ANA     1           // Analog Input
#define LANDIS_CARD_ADC     2           // A/D Conveter
#define LANDIS_CARD_AOM     3           // Analog Output
#define LANDIS_CARD_IND     4           // Indication Input
#define LANDIS_CARD_DO24    5           // 24 Bit Digital Output
#define LANDIS_CARD_SBO     7           // SBO Control Output
#define LANDIS_CARD_ACC1    8           // Accumulator form Type-A
#define LANDIS_CARD_DO32    11          // 32 Bit Digital Output
#define LANDIS_CARD_ACC2    12          // Accumulator form Type-A
#define LANDIS_CARD_POM     15          // Pulse Output
#define LANDIS_CARD_SOE     28          // SOE Input
#define LANDIS_CARD_KWH     29          // KWH Input
#define LANDIS_CARD_SDC     30          // Serial Data Collector
#define LANDIS_CARD_EMT     31          // Empty Slot

/* ------------------------ */
/*  LANDIS OPCODE           */
/* ------------------------ */
#define BIT_SHR                 0x80
#define BIT_CON                 0x40
#define BIT_FRZ                 0x20
#define BIT_IND                 0x10
#define BIT_SCH                 0x08
#define BIT_SLG                 0x04

#define BIT_MFC                 0x10
#define BIT_ACK                 0x04

#define FRAME_SHR               0x80
#define FRAME_CON               0x40
#define FRAME_FRZ               0x20
#define FRAME_IND               0x10
#define FRAME_SCH               0x08
#define FRAME_SLG               0x04

/* ------------------------ */
/*  LANDIS OPCODE           */
/* ------------------------ */
#define GLOBAL_ADDR             0
#define LAST_BLOCK_BIT          0x80
#define SHORT_MSG_BIT           0x80

/* EXECPTION CODE ... for LANDIS */
#define EXCEPT_WARMS            0
#define EXCEPT_COLDS            1
#define EXCEPT_INRAM            2
#define EXCEPT_BUSF             3
#define EXCEPT_SBOF             4
#define EXCEPT_ANAF             5
#define EXCEPT_SOEF             6
#define EXCEPT_CARDE            7
#define EXCEPT_INFC             9
#define EXCEPT_INBL             10
#define EXCEPT_NOPT             11
#define EXCEPT_INPA             12
#define EXCEPT_SEMIS            13
#define EXCEPT_NAFC             14
#define EXCEPT_DBCH             16

#define OP_ANA_COS          0       /* Analog Change report */
#define OP_ANA_FORCE        1       /* Analog Force report */ 
#define OP_ANA_GCOS         2       /* Analog Group Change  report */ 
#define OP_ANA_GFORCE       3       /* Analog Group Force report */ 
#define OP_ADC_REF          5       /* ADC Reference Force report */ 
#define OP_IND_COS          6       /* Indication Change report */ 
#define OP_IND_FORCE        7       /* Indication Force report */  
#define OP_SOE_COS          8       /* SOE Change report */  
#define OP_SOE_FORCE        9       /* SOE Force  report */  
#define OP_DIGITAL_FORCE    11      /* Digital Input Force report */  
#define OP_ACC_COS          12      /* Accumulator Change report */  
#define OP_ACC_FORCE        13      /* Accumulator Force report */   
#define OP_SOELOG_COS       14      /* SOE LOG Change report */   
#define OP_ANALOG_OUT       20      /* Analog Output */   
#define OP_SBO_SEL          21      /* SBO Select */    
#define OP_SBO_OPR          22      /* SBO Operate */    
#define OP_DIGITAL_OUT      23      /* Digital Output */    
#define OP_ACC_FRZ          24      /* Accumulator Freeze */    
#define OP_PULSE_OUT        25      /* Pulse Output */    
#define OP_PULSE_TRAIN      26      /* Pulse Train Output */    
#define OP_SBO_EXE          28      /* SBO Immediate Execute */    

#define OP_RESTART          30      /* Restart RTU */    
#define OP_CONFIG           31      /* RTU Configuration */     
#define OP_TIME_SYNC        32      /* Time Sync */     
#define OP_TIME_BIAS        33      /* Time Bias */     
#define OP_DEADBAND         34      /* Analog Deadband */     
#define OP_ANA_GROUP        35      /* Analog Group defination */     
#define OP_ACC_PRESET       36      /* Accumulator Preset */     
#define OP_CONTINUE         37      /* Continuation Request */     
#define OP_REPEAT           38      /* Repeat Last message */     
#define OP_FIRMWARE         39      /* Firmware Configuration */     
#define OP_TREAD            47      /* TABLE Read */     
#define OP_TWRITE           48      /* TABLE Write */     
#define OP_SELF_INTERVAL    50
#define OP_SELF_SEQUENCE    51
#define OP_EXP_REPORT       63      /* Exception Report */



typedef struct
    {
        byte    front;
        byte    rear;
        byte    report;
        byte    dummy;
        byte    que[256+2][2];
    } __attribute__ ((packed)) LANDIS_COS_Q;

typedef struct
    {
        byte    front;
        byte    rear;
        byte    report;
        byte    dummy;
        byte    que[256+2][10];
    } __attribute__ ((packed)) LANDIS_SOE_Q;

typedef struct
        {
        short   rtuid;
        short   hostAddr;
        short   type;
        short   status;
        short   soeChange;

        short   rcvACK;
        short   preOpcode;

        short   maxModule;
        short   maxStsPoint;
        short   maxAnaPoint;
        short   maxAccPoint;
        short   maxCntPoint;
        short   maxRefPoint;

        short   multiMsg;
        short   shortMsg;
        short   freeze;
        short   freezeForce;

        short   exceptFlag;
        short   exceptCode;
        short   exceptPara;

        short   processTime;
        short   timeBias;

        short   broadCast;
        short   rcvPACKET;
        short   rcvOpcode;
        
        short   totalCount;
        short   dataCount;
        short   sendSize;
        short   sendPos; 
        short   anaDumpIndex ;
        
        word    sndCount;
        byte    sndBuffer[512];
        byte    dataBuf[512];
        word    version;
                
        LANDIS_COS_Q   cosQ;
        LANDIS_SOE_Q   soeQ;
        
        CONTROL_INFO    controlInfo[MAX_DNP_DO_POINT];
        //DEVPOINT_INFO   deviceInfo[MAX_DEV_POINT];

        } __attribute__ ((packed)) LANDIS_CFG;


typedef struct
        {
        short   index;
        short   status;             /* Point status */   
        
        /* database parameter */ 
        short   soests;
        short   soechg;
        short   chgTick;
        } __attribute__ ((packed)) HOST_STSBUF;
    
typedef struct
        {
        short   index;
        short   status;             /* Point status */   
        word    pointData;          /* Analog Point Data */
        word    convData;           /* Analog Point Data */

        } __attribute__ ((packed)) HOST_ANABUF;
        
#endif	// OP_LANDIS_HEADER_INCLUDED

/*----------------------------------------- */
