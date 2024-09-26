#ifndef DLG_def_h
#define DLG_def_h

#include <Arduino.h>
extern const char* FILENAME;


// ***********************************************************************
//  Pin declaration SSM-DLG-001
// ***********************************************************************
//      POWER RELATED
#define EN_VCC_MIKROE   3   //Enable Mikroe Ports 5V and 3.3V 
#define EN_VExt        16   //Enable Vcc out in ALL the JST Connectors


//      UART RELATED
#define URX0            4   //UART IN Multiplexer(MIKROE2,RS232,RS485)
#define UTX0            5

#define SEL0           19   //Multiplexer UART Selectors (MIKROE2,RS232,RS485)
#define SEL1           20

#define UMRX           47   //MIKROE1 UART
#define UMTX           48


//      COMMS RELATED
#define FC_485          6   //FlowControl RS485
#define CANTX           7   //BUS CAN
#define CANRX          15


//       SPI RELATED
#define CS_FLASH       46
#define CS_SD          10

#define MOSI           11
#define SCK            12
#define MISO           13


// ***********************************************************************
//     I2C RELATED
// ***********************************************************************
/* GPIO EXPANDER        | TCA6408A       | con R12(GND): b0100000, con R12(Vcc): b0100001 |  0x20, 0x21
 * POWER MONITOR        | INA220AIDGSR   |                   b1000000                     |  0x40
 * ADC                  | MCP3424-E/SL   |                   b1101100                     |  0x6C
 * RTC                  | DS1339U-33+T&R |                   b1101000                     |  0x68
 * CRYPTO AUTHENTICATOR | ATSHA204A      |              b1100100, b0110100                |  0xC8 , 0x64
 */
#define SDA             8
#define SCL             9

//          DIO
#define PIOE2          17   //Pin directo a conector J4
#define PIOE1          18   //Pin directo a conector J4 y a interrupción RTC

#define MPIO8          14   //Connected to J1_INT
#define MPIO9          21   //Connected to J1_PWM
#define MPIO7          45   //Connected to J1_CS
#define MPIO6          35   //Connected to J1_RST
#define MPIO5          36   //Connected to J1_AN

#define MPIO4          37   //Connected to J2_INT
#define MPIO3          38   //Connected to J2_PWM
#define MPIO2          39   //Connected to J2_CS
#define MPIO1          40   //Connected to J2_RST
#define MPIO0          41   //Connected to J2_AN

#define PIO_INT        42   //[INVERTED logic] GPIO EXPANDER  open-drain Interrupt output. Connect to VCCI through a pull-up resistor.
                            //INT is LOW when a IN Port change the status, need a PULLUP
                            
                     

//BUTTON
#define BTN0            2   
//NEOPIXEL
#define NEOX            1



// ***********************************************************************
//  Mikroe Pin config
// ***********************************************************************
#ifdef USE_MIKROE_1 //J3
#define MK1_AN         MPIO5
#define MK1_RST        MPIO6
#define MK1_CS         MPIO7
#define MK1_PWM        MPIO8
#define MK1_INT        MPIO9
#endif

#ifdef USE_MIKROE_2 //J2
#define MK2_AN         MPIO0
#define MK2_RST        MPIO1
#define MK2_CS         MPIO2
#define MK2_PWM        MPIO3
#define MK2_INT        MPIO4
#endif


// ***********************************************************************
//  UARTS
//              Multiplexor
//  -----------------------------------
//  -   SEL0  -   SEL1   -     Port     
//  -----------------------------------
//  -    0    -    0   -        M2       
//  -----------------------------------
//  -    0    -    1   -     RS232-ch1 
//  -----------------------------------
//  -    1    -    0   -     RS232-ch2    
//  -----------------------------------
//  -    1    -    1   -      RS485    
//  -----------------------------------
// ***********************************************************************

typedef enum{
  MIKROE_2,   //0
  RS_232_ch1,  //1 
  RS_232_ch2,  //2
  RS_485,     //3
} mux_uart;


// ***********************************************************************
//  SWITCHS SELECTORS
// ***********************************************************************
/*                                       U20
 *                      ON                |               OFF             
 *     ---------------------------------------------------
 *  1 | CH4- to GND                       | CH4- not in GND
 *  2 | R42 (120 Ohms) parallel with CH4  | R42 out of circuit CH4
 *  3 | Short R40 (300 Ohms) in CH4+      | R40 (300 Ohms) in line with CH4+
 *  4 | Short R37 (300 Ohms) in CH3+      | R37 (300 Ohms) in line with CH3+
 *  5 | R39 (120 Ohms) parallel with CH3  | R39 out of circuit CH3
 *  6 | CH3- to GND                       | CH3- not in GND
 *  
 *                                       U21
 *                      ON                |           OFF             
 *     ---------------------------------------------------
 *  1 | CH2- to GND                       | CH2- not in GND
 *  2 | R36 (120 Ohms) parallel with CH2  | R36 out of circuit CH2
 *  3 | Short R34 (300 Ohms) in CH2+      | R34 (300 Ohms) in line with CH2+
 *  4 | Short R29 (300 Ohms) in CH1+      | R29 (300 Ohms) in line with CH1+
 *  5 | R32 (120 Ohms) parallel with CH1  | R32 out of circuit CH1
 *  6 | CH1- to GND                       | CH1- not in GND
 */
 
/*   PUERTOS: 
 *  ---------------------------------       CN4: RS232_ch1, CAN
 *  |-              [CN4/C2][CN2/C1]|       CN2: RS232_ch2, RS485
 *  |J|             [CN3/S2][CN1/S1]|       CN3: ADC_ch2, DIn_2, DOut_2
 *  |4|             [CN5/S3][CN6/S4]|       CN1: ADC_ch1, DIn_1, DOut_1 
 *  ---------------------------------       CN5: ADC_ch3, DIn_3, DOut_3
 *  J4: I2C,PIO1,PIO2                       CN6: ADC_ch4, DIn_4, DOut_4
 */

 /*SOLDER JUMPER U11
  *  Es un Jumper de selección de tensión para la salida del TPS4H160AQPWPRQ1 
  * 
  */


#endif //DLG_def_h
