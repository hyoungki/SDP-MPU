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

//#define NUMBER_OF_EVENT     (0x30 + 2)

byte    errmsg[NUMBER_OF_EVENT][20] = {
        " ------------- ",      /* id = 0x00 */
        "*MCU Restart   ",      /* id = 0x01 */
        "*ROMDB Fail    ",      /* id = 0x02 */
        "*Reset by USER ",      /* id = 0x03 */
        "*Reset by WDT  ",      /* id = 0x04 */
        "MASTER-CPU ACT ",      /* id = 0x05 */
        "SLAVE-CPU ACT  ",      /* id = 0x06 */
        "MAN-CHG SDP-A  ",      /* id = 0x07 */
        "MAN-CHG SDP-B  ",      /* id = 0x08 */
        "SCU-CHG AUTO   ",      /* id = 0x09 */
        "SCU-CHG MANUAL ",      /* id = 0x0a */
        "LOCAL Time-SET ",      /* id = 0x0b */
        "SIM  Time-SET  ",      /* id = 0x0c */
        "HOST Time-SET  ",      /* id = 0x0d */
        "*CHG CPU-ACT   ",      /* id = 0x0e */
        "*Host WDT-RESET",      /* id = 0x0f */
        
        "(ICCP) Control ",      /* id = 0x10 */
        "(HOST) Control ",      /* id = 0x11 */
        "(SIM) Control  ",      /* id = 0x12 */
        "(USER) Control ",      /* id = 0x13 */
        "(LINK) Control ",      /* id = 0x14 */
        "DATABASE Update",      /* id = 0x15 */
        " ------------- ",      /* id = 0x16 */
        " ------------- ",      /* id = 0x17 */
        " ------------- ",      /* id = 0x18 */
        " ------------- ",      /* id = 0x19 */
        " ------------- ",      /* id = 0x1a */
        " ------------- ",      /* id = 0x1b */
        " ------------- ",      /* id = 0x1c */
        " ------------- ",      /* id = 0x1d */
        " ------------- ",      /* id = 0x1e */
        " ------------- ",      /* id = 0x1f */
        
        "(SOE) Event    ",      /* id = 0x20 */
        "(DEVICE) Event ",      /* id = 0x21 */
        " ------------- ",      /* id = 0x22 */
        " ------------- ",      /* id = 0x23 */
        " ------------- ",      /* id = 0x24 */
        " ------------- ",      /* id = 0x25 */
        " ------------- ",      /* id = 0x26 */
        " ------------- ",      /* id = 0x27 */
        " ------------- ",      /* id = 0x28 */
        " ------------- ",      /* id = 0x29 */
        " ------------- ",      /* id = 0x2a */
        " ------------- ",      /* id = 0x2b */
        " ------------- ",      /* id = 0x2c */
        " ------------- ",      /* id = 0x2d */
        " ------------- ",      /* id = 0x2e */
        " ------------- ",      /* id = 0x2f */
        
        "(ICCP) Online  ",      /* id = 0x30 */
        "(ICCP)*Offline ",      /* id = 0x31 */
        "(HOST) Online  ",      /* id = 0x32 */
        "(HOST)*Offline ",      /* id = 0x33 */
        "(IED)  Online  ",      /* id = 0x34 */
        "(IED) *Offline ",      /* id = 0x35 */
        "(ESIO) Online  ",      /* id = 0x36 */
        "(ESIO)*Offline ",      /* id = 0x37 */
        "(MMI)  Online  ",      /* id = 0x38 */
        "(MMI) *Offline ",      /* id = 0x39 */
        "(SCU)  Online  ",      /* id = 0x3a */
        "(SCU) *Offline ",      /* id = 0x3b */
        "(LINK) Online  ",      /* id = 0x3c */
        "(LINK)*Offline ",      /* id = 0x3d */
        "(VME)  Online  ",      /* id = 0x3e */
        "(VME) *Offline ",      /* id = 0x3f */
        
        "(VME)  INSTALL ",      /* id = 0x40 */
        "(VME)*UNINSTALL",      /* id = 0x41 */
        " ------------- ",      /* id = 0x42 */
        " ------------- ",      /* id = 0x43 */
        " ------------- ",      /* id = 0x44 */
        " ------------- ",      /* id = 0x45 */
        " ------------- ",      /* id = 0x46 */
        " ------------- ",      /* id = 0x47 */
        " ------------- ",      /* id = 0x48 */
        " ------------- ",      /* id = 0x49 */
        " ------------- ",      /* id = 0x4a */
        " ------------- ",      /* id = 0x4b */
        " ------------- ",      /* id = 0x4c */
        " ------------- ",      /* id = 0x4d */
        " ------------- ",      /* id = 0x4e */
        " ------------- "       /* id = 0x4f */
        };
