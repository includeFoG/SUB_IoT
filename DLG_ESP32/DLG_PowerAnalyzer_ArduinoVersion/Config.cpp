#include "Config.h"
#include "Arduino.h"

#define DEVICE_NAME "SSM_DLG_PowerAnalyzer_0001"

void programStart() {
  Serial.println("\n\n******************************");
  Serial.println("        Program start         ");
  Serial.println(FILENAME);
  Serial.println(DEVICE_NAME);
  Serial.println("******************************");
}


void initBoard() {
  //Configuración de los pines de selección del multiplexor
  pinMode(SEL0, OUTPUT);
  pinMode(SEL1, OUTPUT);
  //inicialización del multiplexor con salida 0 -> Mikroe2
  digitalWrite(SEL0, LOW);
  digitalWrite(SEL1, LOW);

#ifdef DEBUG
Serial.begin(ESP32_BAUDRATE);
#endif

#ifdef USE_ESP32_UART
  Serial1.begin(ESP32_BAUDRATE, SERIAL_8N1, URX0, UTX0);
#endif
#ifdef USE_MIKROE_1
  Serial2.begin(MIKROE1_BAUDRATE, SERIAL_8N1, UMRX, UMTX);
#endif
  mux_uart starter_uart = MIKROE_2; //definimos una uart de inicio para selectUart()
  selectUart(starter_uart);

  /*// Inicio de I2C para conectarse con el teclado
    Wire.begin();
    // Inicio de la comunicación con el pixel RGB de la placa
    pixels.begin();
    // Configura el botón de la placa
    pinMode(BTN0, INPUT_PULLUP);
    // Configuración del pin que activa la alimentación en los conectores auxiliares (jst)
    pinMode(EN_VO, OUTPUT);
    // Configuración del pin que activa la alimentación del Mikroe
    pinMode(EN_VMK, OUTPUT);

    // Inicialización del módulo TCA6408 que expande los puertos por medio de I2C
    prt.begin( );
    prt.pin_mode( 0, OUTPUT );
    prt.pin_mode( 1, OUTPUT );
    prt.pin_mode( 2, OUTPUT );
    prt.pin_mode( 3, OUTPUT );
    prt.pin_mode( 4, INPUT );
    prt.pin_mode( 5, INPUT );
    prt.pin_mode( 6, INPUT );
    prt.pin_mode( 7, INPUT );*/
  delay(50);
  programStart();
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
