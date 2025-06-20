
#define USB_BAUD 230400
#define M1_BAUD 115200
#define M2_BAUD 9600

int min_(int a, int b) {
  if (a >= b)return b;
  return a;
}

#ifndef MAIN_SHIFFTER
#include "myShifft.h"
#endif

#define LED_BUILTIN 25
#define DIGI0       LED_BUILTIN
#define DIGI1       15
#define DIGI2       27
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
#define EN_ANALOGS   BIT_Q7

#define SHFT_STCP   23
#define SHFT_SHCP   24
#define SHFT_DATA   0

myShifft shiffty(SHFT_STCP, SHFT_SHCP, SHFT_DATA);
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

//Without Flow control
Uart SerialM1(&sercom2, M1_RX, M1_TX, SERCOM_RX_PAD_1, UART_TX_PAD_0);
//With Flow control
//Uart SerialM1(&sercom2, M1_RX, M1_TX, SERCOM_RX_PAD_1, UART_TX_RTS_CTS_PAD_0_2_3, 2, 5);

void SERCOM2_Handler()
{
  SerialM1.IrqHandler();
}

//-------------M2----------------
//Puerto M2+

#define   M2_AN     18
#define   M2_RST    17
#define   M2_CS     9
#define   M2_PWM    3
#define   M2_INT    1
#define   M2_RX     7
#define   M2_TX     6

Uart SerialM2 (&sercom5, M2_RX, M2_TX, SERCOM_RX_PAD_3, UART_TX_PAD_2);

void SERCOM5_Handler()
{
  SerialM2.IrqHandler();
}

 
////////////Ina input/////////////
#include <Adafruit_INA219.h>

//Adafruit_INA219 ina_input(0x40);
Adafruit_INA219 ina_input(0x41);



void init_IO () {
  Serial.begin(USB_BAUD);
  SerialM1.begin(M1_BAUD);
  SerialM2.end();
  shiffty.initialize();
  shiffty.set(EN_5VCC);
//  pinMode(  M1_RX, INPUT_PULLDOWN);
//  pinMode(  M1_TX, INPUT_PULLDOWN);
  pinMode(  M2_RX, INPUT_PULLDOWN);
  pinMode(  M2_TX, INPUT_PULLDOWN);
  pinMode(  MUX_SEL_A , INPUT_PULLDOWN);
  pinMode(  MUX_SEL_B , INPUT_PULLDOWN);
//  pinMode(  SHFT_STCP  , INPUT_PULLDOWN);
//  pinMode(  SHFT_SHCP  , INPUT_PULLDOWN);
//  pinMode(  SHFT_DATA   , INPUT_PULLDOWN);
  //pinMode( DIGI0      ,INPUT_PULLDOWN);
  pinMode(  DIGI1       , INPUT_PULLDOWN);
  pinMode(  DIGI2       , INPUT_PULLDOWN);
  pinMode(  SD_CS      , INPUT_PULLDOWN);
  pinMode(  CAM_SS     , INPUT_PULLDOWN);
  pinMode(  SERVO_PWM   , INPUT_PULLDOWN);
  //pinMode(  EN_5VCC    , INPUT_PULLDOWN);
  pinMode(  TEMP       , INPUT_PULLDOWN);
  //I2C
  // pinMode(  SDA        , INPUT_PULLDOWN);
  // pinMode(  SCL        , INPUT_PULLDOWN);
  //SPI
  pinMode(  MISO      , INPUT_PULLDOWN);
  pinMode(  MOSI      , INPUT_PULLDOWN);
  pinMode(  SCK        , INPUT_PULLDOWN);
  //RS485
  pinMode(  FLOW_RS485  , INPUT_PULLDOWN);
  pinMode(  M1_AN, INPUT);
  pinMode(  M1_RST, OUTPUT);
  //_5volts.initialize();
  //pinMode(EN_5VCC,OUTPUT);
  pinMode(  MUX_SEL_A, OUTPUT);
  pinMode(  MUX_SEL_B, OUTPUT);
  pinMode(  FLOW_RS485, OUTPUT);


  // Init in receive mode
  digitalWrite(FLOW_RS485, LOW);
  digitalWrite(M1_RST, LOW);
  //digitalWrite(EN_5VCC,LOW);
  digitalWrite(MUX_SEL_A, LOW);
  digitalWrite(MUX_SEL_B, LOW);
}

float loacl_pcbTemp = 0;
float MeasureTemperature() {
  float RawTemperature = ((3100 * analogRead(TEMP) / 4096.0) - 500) / 10.000 ;
  return RawTemperature;
}

typedef struct {
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;
} inaData;

inaData powerIn;
