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

#ifndef	OP_DNP_HEADER_INCLUDED
#define	OP_DNP_HEADER_INCLUDED

typedef struct
{
    ulong   mostSignificant;
    word    leastSignificant;
}PDPDTIME_MS_SINCE_70;

typedef struct
{
    word    year;         /* 1970-2099 (the range these functions support) */
    byte    month;        /* 1-12 */
    char    day;          /* 1-31 */
    char    hour;         /* 0-23 */
    char    minute;       /* 0-59 */
    char    second;       /* 0-59 */
    word    millisecond;  /* 0-999 */
}PDPDTIME_DATE_TIME;


  
#define RX_MODE             1
#define TX_MODE             2

#define MAX_USER_BUFFER     512
#define MAX_APP_BUFFER      2048
#define MAX_TRANS_BUFFER    512
#define MAX_DATA_BUFFER     512

#define MAX_DNP_FRAME       255

#define TH_FIN_BIT          0x80
#define TH_FIR_BIT          0x40

#define AC_FIR_BIT          0x80
#define AC_FIN_BIT          0x40
#define CONFIRM_BIT         0x20

#define DIR_BIT             0x80
#define PRM_BIT             0x40

#define FCB_BIT             0x20
#define FCV_BIT             0x10
#define DFC_BIT             0x10

#define POS_USER_DATA       10

#define APP_SEND_LEN        249 /* 255(max length) - 6 (control,des,src) */
#define FRAME_LENGTH        16

#define E_TIMEOUT           -1
#define E_CRC               -2

/* ------------------------------------ */
/*  DNP OPCODE constant ...             */
/* ------------------------------------ */
#define FUNC_RESET_LINK     0
#define FUNC_RESET_USER     1
#define FUNC_TEST           2
#define FUNC_USER_DATA      3
#define FUNC_UNCONFIRM      4
#define FUNC_REQUEST        9

#define RCV_ACK             0
#define RCV_NACK            1
#define RCV_RESPONSE        11

/* ------------------------------------ */
/*  DNP OPCODE constant ...             */
/*  APPLICATION Layer                   */
/* ------------------------------------ */
#define APP_CONFIRM         0
#define APP_READ            1
#define APP_WRITE           2
#define APP_SELECT          3
#define APP_OPR             4
#define APP_DIRECT          5
#define APP_DIRECT_NOACK    6
#define APP_FREEZE          7   
#define APP_FREEZE_NOACK    8
#define APP_FREEZE_CLEAR    9
#define APP_FCLEAR_NOACK    10
#define APP_FREEZE_TIME     11
#define APP_FTIME_NOACK     12
#define APP_COLD_RESTART    13
#define APP_WARM_RESTART    14
#define APP_INIT_DATA       15
#define APP_INIT_APP        16
#define APP_START_APP       17
#define APP_STOP_APP        18
#define APP_SAVE_CONFIG     19
#define APP_ENB_UNSOLICIT   20
#define APP_DISB_UNSOLICIT  21
#define APP_ASSIGN_CLASS    22
#define APP_DELAY_MEASURE   23

#define APP_RCV_CONFIRM     0
#define APP_RCV_RESPONSE    129
#define APP_RCV_UNSOLICIT   130
        

/* ------------------------------------ */
/*  DNP IIN-BIT constant ...            */
/* ------------------------------------ */
#define BIT_BROADCAST       0x01
#define BIT_CLASS1_REQ      0x02
#define BIT_CLASS2_REQ      0x04
#define BIT_CLASS3_REQ      0x08
#define BIT_TIME_SYNC_REQ   0x10
#define BIT_CNTR_LOCAL      0x20        // Local 모드로 제어 금지시....
#define BIT_DEV_FAIL        0x40
#define BIT_DEV_RESTART     0x80        // 프로그램 재 기동시...

#define BIT_FUNC_FAIL       0x01        // Function Code 이상시...
#define BIT_OBJECT_FAIL     0x02        // Object 가 없는 경우...
#define BIT_OUT_RANGE       0x04        // Qualifier, Range 가 유효하지 않은 경우...
#define BIT_BUFF_OVER       0x08


/* DNP Relative Constant Define */

#define DNP_CLASS0          1
#define DNP_CLASS1          2
#define DNP_CLASS2          3
#define DNP_CLASS3          4

/*
*   APPLICATION OBJECT 
*/
#define OBJ_BINARY_INPUT        1           // Single-BIT Binary Input 
#define OBJ_BINARY_CHANGE       2           // Binary Input Change Without Time
#define OBJ_BINARY_OUTPUT       10          // Binary Output 
#define OBJ_CONTROL_RELAY       12          // Control Relay Output
#define OBJ_BINARY_COUNTER      20          // Binary Counter 
#define OBJ_FROZEN_COUNTER      21          // 32-Bit Frozen Counter
#define OBJ_COUNTER_CHANGE      22          // 32-Bit Counter Change Event Without Time
#define OBJ_FC_CHANGE           23          // 32-Bit Frozen Counter Change Event Without Time
#define OBJ_ANALOG_INPUT        30          // 32-Bit Analog Input
#define OBJ_FROZEN_ANALOG       31          // 32-Bit Frozen Analog Input
#define OBJ_ANALOG_CHANGE       32          // 32-Bit Analog Change Event Without Time
#define OBJ_FA_CHANGE           33          // 32-Bit Frozen Analog Change Event Without Time
#define OBJ_ANALOG_OUTPUT       40          // 32-Bit Analog Output
#define OBJ_AOUT_BLOCK          41          // 32-Bit Analog Output Block
#define OBJ_TIME_AND_DATE       50          // Time and Date
#define OBJ_TIME_AND_DATE_CTO   51          // Time and Date with CTO
#define OBJ_TIME_DELAY_COA      52          // Time Delay COARSE
#define OBJ_CLASS               60          // Classs 
#define OBJ_FILE                70          // File Identifier
#define OBJ_DEVICE_IIN          80          // Device Object : Internal Indication
#define OBJ_DEVICE_STORAGE      81          // Device Object : Storage 
#define OBJ_DEVICE_PROFILE      82          // Device Object : Profile
#define OBJ_DEVICE_REG          83          // Device Object : Private Registration 
#define OBJ_APP_IDEN            90          // Application Identifier
#define OBJ_ALT_NUMERIC         100         // Alternate Numeric : Short Floating Point
#define OBJ_PACKED_BCD          101         // Packed Binary Coded Decimal


/*
*   POINT STATUS  
*/

#define DEV_OFFLINE         0x01

#define PT_FLAG_ONLINE      0x01
#define PT_FLAG_OFFLINE     0x00

#define PT_FLAG_RESTART     0x02
#define PT_FLAG_COMLOST     0x04
#define PT_FLAG_REMOTE      0x08
#define PT_FLAG_LOCAL       0x10
#define PT_FLAG_CHATTER     0x20
#define PT_FLAG_RESERVE     0x40

#define PT_FLAG_STS_ON      0x80
#define PT_FLAG_STS_OFF     0x00



typedef struct
    {
        int     flag;
        
        int     pointData;              // Varaition #1 : 32Bit Analog Input with Flag
        //word    pointDataV2;          // Varaition #2 : 16Bit Analog Input with Flag   
        //float   pointDataV5;          // Varaition #5 : 32Bit Float Analog Input
        byte    lowdata[4];

    } __attribute__ ((packed)) DNP_ANA_INPUT;


/* ------------------------------------ */
/*  CLASS1 : SOE Queue                  */
/* ------------------------------------ */    
typedef struct
    {
        /* QCode 7 : Single field Quantity, 포인트가 256 이하인 경우    */
        byte    soeBuf[12];         
    } __attribute__ ((packed)) DNP_SOEQ_ENTRY;  

typedef struct
    {
        byte    front;      // HOST 보고용 Flag
        byte    rear;
        
        byte    front1;     // CPU 링크용 Flag
        byte    rear1;
        
        DNP_SOEQ_ENTRY  soeQueue[256+2];
    } __attribute__ ((packed)) DNP_SOE_QUEUE;  


/* ------------------------------------ */
/*  CLASS2 : COS Queue                  */
/* ------------------------------------ */
typedef struct
    {
        byte    cosBuf[4];
    } __attribute__ ((packed)) DNP_COSQ_ENTRY;  

typedef struct
    {
        byte    front;
        byte    rear;
        DNP_COSQ_ENTRY  cosQueue[256+2];

    } __attribute__ ((packed)) DNP_COS_QUEUE;                  

/* ------------------------------------ */
/*  CLASS3 : COA Queue                  */
/* ------------------------------------ */
typedef struct
    {
        byte    pointIndex;             // HOST Point Number                
        byte    pointFlag;              //      Point Status
        byte    pointData[4];           //      Point Data
        byte    dnpTime[6];             //      DNP Time
    } __attribute__ ((packed)) DNP_COAQ_ENTRY;  

typedef struct
    {
        byte    front;
        byte    rear;
        DNP_COAQ_ENTRY  coaQueue[256+2];

    } __attribute__ ((packed)) DNP_COA_QUEUE;  
    
    
        
typedef struct
    {
        byte    start1;
        byte    start2;
        byte    length;
        byte    control;
        byte    desAddr[2];
        byte    srcAddr[2];
        word    crc;
        byte    userData[300];

    } __attribute__ ((packed)) DL_FRAME;


typedef struct
    {
        byte    dlControl;
        byte    broadCast;

        word    fruid;
        word    hostid;
        word    rcvCount;


        /* ---------------------- */
        DL_FRAME    sndFrame;
        DL_FRAME    rcvFrame;

    } __attribute__ ((packed)) DNP_DATA_LINK;
    
typedef struct
    {
        short   rcvPos;
        short   rcvSize;

        short   sndPos;
        short   sndSeqno;

        word    desAddr;
        word    srcAddr;

        short   sndCount;
        short   sndFrame;

        byte    userData[MAX_APP_BUFFER];

    } __attribute__ ((packed)) DNP_APP_FRAME;

#endif	// OP_DNP_HEADER_INCLUDED

