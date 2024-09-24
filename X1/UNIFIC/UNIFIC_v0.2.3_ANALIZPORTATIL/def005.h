#ifndef _SSM_def005_
#define _SSM_def005_


#define USB_BAUD 115200
#define M1_BAUD 9600 //bc95G: 9600  //ESP32: 115200 //SIM808: 115200  //saraR410M/u201: 115200 //NRF9160:115200 //ESP32 htppfile: AT+UART_CUR=230423,8,1,0,1 //AT+UART_CUR=115200,8,1,0,1 CUR/DEF
#define M2_BAUD 9600 //estaciones meteo:4800  //Sensor hidrocarburos Disen:9600  //Schneider: 9600/115200?



#ifndef MAIN_SHIFFTER
#include "myShifft.h"
#endif

#define LED_BUILTIN 25
#define DIGI0       LED_BUILTIN
#define DIGI1       15
#define SD_CS       10
#define CAM_SS      16
#define SERVO_PWM   4
//#define EN_5VCC     27
#define TEMP        19
//I2C
#define SDA         20
#define SCL         21
//SPI
#define MISO        34
#define MOSI        35
#define SCK         37
//RS485
#define FLOW_RS485  26


//Shifter

#define EN_5VCC     BIT_Q0
#define EN_MIKRO    BIT_Q1
#define EN_SERVO    BIT_Q2
#define EN_ARDUCAM  BIT_Q3
#define EN_RS232_1  BIT_Q4
#define EN_RS232_0  BIT_Q5
#define EN_RS485    BIT_Q6
#define EN_ANALOGS  BIT_Q7

#define SHFT_STCP   23
#define SHFT_SHCP   24
#define SHFT_DATA   0

//myShifft shiffty(SHFT_STCP,SHFT_SHCP,SHFT_DATA);
//digiSmart _5volts(EN_5VCC,LOW);

//Multiplexor

//  ---------------------------------
//  -   B    -   A   -     Port     -
//  ---------------------------------
//  -   0    -   0   -     M2       -
//  ---------------------------------
//  -   0    -   1   -     RS232-0  -
//  ---------------------------------
//  -   1    -   0   -     RS485    -
//  ---------------------------------
//  -   1    -   1   -     RS232-1  -
//  ---------------------------------



#define MUX_SEL_A  30
#define MUX_SEL_B  31


//Puertos Mikro-e

//-------------M1----------------
//Puerto M1+
#define TEST  M1_TX

#define   M1_AN     14
#define   M1_RST    42
#define   M1_CS     2
#define   M1_PWM    8
#define   M1_INT    5
#define   M1_RX     38
#define   M1_TX     22


//-------------M2----------------
//Puerto M2+

#define   M2_AN     18
#define   M2_RST    17
#define   M2_CS     9
#define   M2_PWM    3
#define   M2_INT    1
#define   M2_RX     7
#define   M2_TX     6


#endif
