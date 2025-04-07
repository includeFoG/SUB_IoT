#ifndef DLG_def_h
#define DLG_def_h


#define USB_BAUD 115200
#define M1_BAUD 9600 //bc95G: 9600  //ESP32: 115200 //SIM808: 115200  //saraR410M/u201: 115200 //NRF9160:115200 //ESP32 htppfile: AT+UART_CUR=230423,8,1,0,1 //AT+UART_CUR=115200,8,1,0,1 CUR/DEF
#define RS485_BAUD 9600 //estaciones meteo:4800  //Sensor hidrocarburos Disen:9600  //Schneider: 9600/115200?
#define MUX_BAUD RS485_BAUD





//I2C
#define SDA          8
#define SCL          9
//SPI
#define MISO        13
#define MOSI        11
#define SCK         12
//RS485
#define FLOW_RS485   6


#define EN_VCC_MIKROE     3
#define EN_VSys     16   //Enable Vcc -> Vsys out in ALL the JST Connectors


#define URX0            4   //UART IN Multiplexer(MIKROE2,RS232,RS485)
#define UTX0            5
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

#define MUX_SEL_0  19
#define MUX_SEL_1  20


//Puertos Mikro-e

//-------------M1----------------


#define   M1_AN     36
#define   M1_RST    35
#define   M1_CS     45
#define   M1_PWM    14
#define   M1_INT    21
#define   M1_RX     47
#define   M1_TX     48



#endif
