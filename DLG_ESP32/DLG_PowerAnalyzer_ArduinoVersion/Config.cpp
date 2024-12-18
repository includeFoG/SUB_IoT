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

void scanI2CDevices() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning for I2C devices ...");
  for (address = 0x01; address < 0x7f; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if (error != 2) {
      Serial.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  }
}



void initBoard() {
  Wire.begin(SDA, SCL); //

#ifdef DEBUG
  Serial.begin(ESP32_BAUDRATE);
#endif

  //Configuración de los pines de selección del multiplexor
  pinMode(SEL0, OUTPUT);
  pinMode(SEL1, OUTPUT);
  //inicialización del multiplexor con salida 0 -> Mikroe2
  digitalWrite(SEL0, LOW);
  digitalWrite(SEL1, LOW);

#ifdef USE_ESP32_UART
  Serial1.begin(ESP32_BAUDRATE, SERIAL_8N1, URX0, UTX0); //ESTO DEBERIA ESTAR DONDE SE INICIALICE
#endif
#ifdef USE_MIKROE_1
  Serial2.begin(MIKROE1_BAUDRATE, SERIAL_8N1, UMRX, UMTX);
#endif
  mux_uart starter_uart = MIKROE_2; //definimos una uart de inicio para selectUart()
  selectUart(starter_uart);

  delay(1000);
  scanI2CDevices();

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


// ***********************************************************************
//  ESPACIO DE CLASE: [PowerManager]
// ***********************************************************************

bool PowerManager::initialized = false;  // Inicializamos en false
bool PowerManager::initZeroFlag = false; // Inicializamos en false

PowerManager::PowerManager(uint16_t powers, uint16_t initStates)
  : powers(powers), powerStates(initStates), expGPIOS( shrPrta::getInstance(EXP_GPIO_ADDR, TCA6408))
{
  if(!initZeroFlag){
    initialize(powers,initStates,true); //efecto de "softreset" al TCA con los valores establecidos
  }

}


uint8_t PowerManager::reverseByte(byte byteToReverse)
{
  byte reversedByte = 0;
  for (int i = 0; i < 8 ; i++)
  {
    reversedByte |= (((byteToReverse >> i) & 1) << (7 - i));
  }
  return reversedByte;
}


PowerManager&  PowerManager::getInstance()
{
  static PowerManager instance(0, 0); // Instancia estática
  return instance;
}

/*
   Esta función se utiliza para inicializar el expansor a un estado deseado.
   initZero = true; -> realiza una configuración CERO , como si se quitase la alimentación al TCA, no modifica el valor de initialized
   initZero = false; -> configura el TCA con los valores enviados, establece initialized a true
*/
void PowerManager::initialize(uint16_t powers, uint16_t initStates, bool initZero){
  if (!this->initialized || initZero) {
    this->powers = powers;
    this->powerStates = initStates;

    if (0x80 && powers) //habilitado bit Vsys
    {
      pinMode(EN_VSys, OUTPUT); //CONFIGURAMOS COMO SALIDA
      if (0x80 && initStates)
      {
        digitalWrite(EN_VSys, true);
      }
    }

    byte TPS4H160Powers = powers & 0xFF; //conservamos solo el primer byte
    byte TPS4H160InitState = initStates & 0xFF;

    //en primer lugar inicializamos la comunicación con el expansor de GPIOS:
    // expGPIOS = shrPrta::getInstance(EXP_GPIO_ADDR, TCA6408);
    expGPIOS.begin();
    //nos aseguramos de configurar según nos informan de los powers
    //ya que solo hay 4 salidas, eliminamos de powers los posibles bytes enviados fuera del rango
    this->powers = powers & CONFIG_TCA_FOR_BOARD;

    Serial.println("TPS4H160Powers:" + String(TPS4H160Powers, BIN));
    Serial.println("TPS4H160InitState:" + String(TPS4H160InitState, BIN));

    TPS4H160InitState = (PowerManager::reverseByte(TPS4H160InitState)) >> 4; //el >>4 es DEPENDIENTE DE HARDWARE por como se ha ordenado para que cuadre la nomenclatura en placa

    uint8_t maskStart = 0b1;

    for (int i = 0; i < 4; i++) {
      if (TPS4H160Powers & maskStart) {
        uint8_t auxBinResult = 0;
        expGPIOS.pin_mode(i, OUTPUT);                                            //definimos el DOut como salida power
        Serial.println("Inicializando como power DIGOut: " + String(i));
        auxBinResult = TPS4H160InitState & maskStart;

        expGPIOS.pin_write(i, bool(auxBinResult));                     //configuramos el estado inicial
        Serial.println("Configurando DIDOut a nivel: " + String(bool(TPS4H160InitState & maskStart)) + "\t mediante maskStart: " + String(maskStart, BIN) + " TPS4H160InitState & maskStart: " + String(auxBinResult, BIN));
      }
      maskStart <<= 1;
    }

    if (!initZero)
    {
      initialized = true;
    }
    else{
      this->initZeroFlag = true;
    }
  }
  else {
    Serial.println("[PowerManager]-ERROR: SINGLETON CLASS INITIALIZED");
  }
}
