#ifndef DLG_def_h
#define DLG_def_h

#include <Arduino.h>
extern const char* FILENAME;

#define SERIAL_RS485 Serial1

// ***********************************************************************
//  Pin declaration SSM-DLG-001
// ***********************************************************************
//      POWER RELATED
#define EN_VCC_MIKROE   3   //Enable Mikroe Ports 5V and 3.3V 
#define EN_VSys        16   //Enable Vcc -> Vsys out in ALL the JST Connectors


//      UART RELATED
#define UESP32RX       43   //UART de programación del microcontrolador
#define UESP32TX       44


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
/* GPIO EXPANDER        | TCA6408A       | con R12(GND): b0100000, con R12(Vcc): b0100001 |  [0x20], 0x21  
 * POWER MONITOR        | INA220AIDGSR   |                   b1000000                     |  0x40          
 * ADC                  | MCP3424-E/SL   |                   b1101100                     |  0x6C          
 * RTC                  | DS1339U-33+T&R |                   b1101000                     |  0x68          
 * CRYPTO AUTHENTICATOR | ATSHA204A      |              b1100100, b0110100                |  0xC8 , [0x64] 
 */
#define SDA             8
#define SCL             9

#define EXP_GPIO_ADDR 0x20
#define INA_ADDR      0x40
#define ADC_ADDR      0x60
#define RTC_ADDR      0x68
#define SHA_ADDR      0xC8

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
                            
                     

//BUTTON (J4)
#define BTN0            2   
//NEOPIXEL
#define NEOX            1


// ***********************************************************************
//  Pines del EXP-GPIO  
// ***********************************************************************
#define EXPGPIO_O1 3
#define EXPGPIO_O2 2
#define EXPGPIO_O3 1
#define EXPGPIO_O4 0
#define EXPGPIO_I1 4
#define EXPGPIO_I2 5
#define EXPGPIO_I3 6
#define EXPGPIO_I4 7

// ***********************************************************************
//  Pines del J4 "PUERTO EXTERNO" 
// ***********************************************************************


// ***********************************************************************
//  Mikroe Pin config
// ***********************************************************************
//#ifdef USE_MIKROE_1 //J3
#define MK1_AN         MPIO5
#define MK1_RST        MPIO6
#define MK1_CS         MPIO7
#define MK1_PWM        MPIO8
#define MK1_INT        MPIO9
//#endif

//#ifdef USE_MIKROE_2 //J2
#define MK2_AN         MPIO0
#define MK2_RST        MPIO1
#define MK2_CS         MPIO2
#define MK2_PWM        MPIO3
#define MK2_INT        MPIO4
//#endif


// ***********************************************************************
//  "VIRTUAL PIN PORTS"
// ***********************************************************************
#define NUM_VIRTUAL_PORTS 5 
/*
 * Enumeración para definir los diversos puertos virtuales de la placa
 */
enum idPort{
  PORT_J4,       //0-puerto de conexión externa junto a alimentación
  PORT_PROG,     //1-puerto de programación ESP32
  PORT_MK1,      //2-puerto Mikroe 1 J3
  PORT_MK2,      //3-puerto Mikroe 2 J2
  PORT_EXP      //4-puerto expansor GPIO CN1/S1/DIGIN1,DIGOUT1 - CN3/S2/DIGIN2,DIGOUT2  - CN5/S3/DIGIN3,DIGOUT3 - CN6/S4/DIGIN4,DIGOUT4
};

/*
 * Estructura que contiene la información necesaria para definir un puerto
 * idPort : identifica el puerto para poder buscarlo
 */
struct virtualPort{
  idPort ID;
  uint8_t pines[8];     //cada puerto virtual puede tener hasta 8 pines, aquí se almacena el número asociado el pin
  
  uint8_t vR_DIR = 0x0; //registro virtual de direcciones 16bits 0:entrada 1:salida
  uint8_t vR_ST = 0x0;  //informa del estado del pin sea de entrada o salida 0:nivel bajo 1: nivel alto
};

extern virtualPort v_virtualPort[NUM_VIRTUAL_PORTS]; //puntero usado en DigitalComm para poder acceder a los pines digitales de los distintos puertos virtuales


// ***********************************************************************
//  UARTS
//              Multiplexor
//  -------------------------------------------------
//  ID(byte)  -   SEL0  -   SEL1   -     Port     
//  -------------------------------------------------
//      0     -    0    -    0   -        M2       
//  -------------------------------------------------
//      1     -    0    -    1   -     RS232-ch1 
//  -------------------------------------------------
//      2     -    1    -    0   -     RS232-ch2    
//  -------------------------------------------------
//      3     -    1    -    1   -      RS485    
//  -------------------------------------------------
// ***********************************************************************

enum mux_uart{
  MIKROE_2,   //0
  RS_232_ch1,  //1 
  RS_232_ch2,  //2
  RS_485,     //3
};


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
 
/*   PUERTOS:  SN74CBTLV
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

  
// ***********************************************************************
//     POWER RELATED
// ***********************************************************************
/*
 * #define EN_VCC_MIKROE   3   //Enable Mikroe Ports 5V and 3.3V 
 * #define EN_VSys        16   //Enable Vcc out in ALL the JST Connectors
 */
#define BIT_0_Power_Vsys        (1<<0) //alimenta todos los Vsys de los JST
#define BIT_1_Power_DOut1       (1<<1) //alimenta CN1/S1
#define BIT_2_Power_DOut2       (1<<2) //alimenta CN3/S2
#define BIT_3_Power_DOut3       (1<<3) //alimenta CN5/S3
#define BIT_4_Power_DOut4       (1<<4) //alimenta CN6/S4
#define BIT_5_Power_Mikroe      (1<<5) //alimenta 5V y 3.3V en puerto mikroe 1 y 2

#define CONFIG_TCA_FOR_BOARD 0b00001111 //En el hardware del P0 al P3 son salidas y del P4 al P7 son entradas

 

#endif //DLG_def_h
