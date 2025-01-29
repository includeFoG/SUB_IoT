#include "Arduino.h"

#include "DLG_def.h"
#include "Config.h"

void delayMs(int ms)
{
  vTaskDelay( ms  / portTICK_PERIOD_MS );
}
void delayMsUntil(TickType_t *previousWakeTime, int ms)
{
  vTaskDelayUntil( previousWakeTime, ms / portTICK_PERIOD_MS );
}


void programStart() {
  Serial.println("\n\n******************************");
  Serial.println("        Program start         ");
  Serial.println(FILENAME);
  Serial.println(DEVICE_NAME);
  Serial.println("******************************");
}


void startPinConfig() {
  //Configuración de los pines de selección del multiplexor
  pinMode(SEL0, OUTPUT);
  pinMode(SEL1, OUTPUT);
  
  //Configuracion pin enable Vsys
  pinMode(EN_VSys, OUTPUT);

  //preparamos modulo de comunicaciones
  pinMode(EN_VCC_MIKROE, OUTPUT);
  pinMode(MK1_AN, INPUT);
  pinMode(MK1_RST, OUTPUT);

  //preparamos rs485
  pinMode(FC_485, OUTPUT);

  // Init in receive mode
  digitalWrite(FC_485, LOW);
  digitalWrite(MK1_RST, LOW);
}


void selectUart(uint8_t dir)
{
  switch (dir)
  {
    case MIKROE_2:      //MIKROE2                                  UART1   LOW-LOW
      digitalWrite(SEL0, LOW);
      digitalWrite(SEL1, LOW);
      break;
    case RS_232_ch1:   //RS232_channel1 U2Rx(R1Out) U2Tx(T1In)     UART2   LOW-HIGH
      digitalWrite(SEL0, LOW);
      digitalWrite(SEL1, HIGH);
      break;
    case RS_232_ch2:    //RS232_channel2 U3Rx(R2Out) U3Tx(T2In)    UART3   HIGH-LOW
      digitalWrite(SEL0, HIGH);
      digitalWrite(SEL1, LOW);
      break;
    case RS_485:        //RS485                                    UART4   HIGH-HIGH
      digitalWrite(SEL0, HIGH);
      digitalWrite(SEL1, HIGH);

      break;
  }
}


void initBoard() {
  
#ifdef DEBUG
  Serial.begin(ESP32_BAUDRATE);
#endif

  startPinConfig();

#ifdef USE_MIKROE_1
  Serial2.begin(MIKROE1_BAUDRATE, SERIAL_8N1, UMRX, UMTX);
#endif

  mux_uart starter_uart = RS_485; //definimos una uart de inicio para selectUart()
  selectUart(starter_uart);




  delayMs(50);
  programStart();
}
