#include <Arduino.h>

#include "Config.h"

#include "defpin_GP.h"
#include "F_Aux.h"
#include <Arduino.h>


long actutime = 0;      //usado en distintas partes del código para tomar el tiempo actual y operar en consecuencia
long lastMeasure = 0;   //usado para comprobar que las lecutras del SGP30 sean al menos con un intervalo de 1 seg
long lastHeatTime = -999999999;  //usado para activar el heater del sht30
bool timeToSend = true;
uint8_t cnt_err = 0;
uint8_t cnt_retry = 0;

bool f_firstTime = true; //Indica que se trata de la primera medición


#include "wiring_private.h"
#include "AnalogReading.h"
#include "Comms.h"

//#include <SPI.h>
#include <Wire.h>

#include <WDTZero.h>
WDTZero wtd; // Define WDT


void RESET_GP();

char stat = 'D';

void setup() {
  Serial.begin(115200);
  wtd.setup(WDT_TIME);
  wtd.attachShutdown(ISR_WDT);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(PIN_FAN, OUTPUT);

  ligthHouse(stat, 1);

  Wire.begin();
  delay(1000);

  Serial.println(F("\n******************************"));
  Serial.println(FILENAME);
  Serial.println(DEF_SENSOR_ID);
  Serial.println(F("******************************"));

  if (!initComPort()) {
    ligthHouse(stat, 3);
    RESET_GP();
  }

#ifdef AWS_CLOUD
  do {
    Serial.println(F("Habilitando encriptación..."));
  } while (!tryECCX8());
#endif

  while ((!startComPort()) && (cnt_retry < MAX_ERROR)) {
    cnt_retry++;
    Serial.println(F("REINTENTANDO"));
    ligthHouse(stat, 3);
  }

  if (cnt_retry >= MAX_ERROR) {
    Serial.println(F("ERROR CONECTANDO CON RED"));
    RESET_GP();
  }
  cnt_retry = 0;

  actutime = millis();

#ifdef AIR_QUALITY_4
  initVector(v_measCO2, 0);
  do {
    Serial.println(F("Conectando con SGP30"));
    delay(500);
  } while (!startSGP30() && ((millis() - actutime) < 60000));

  if ((millis() - actutime) > 60000) {
    ligthHouse(stat, 4);
    RESET_GP();
  }
#endif

#ifdef SHT30
  initVector(v_measTemp, 0);
  initVector(v_measHum, 0);
  do {
    Serial.println(F("Conectando con SHT30"));
    delay(500);
  } while (!startSHT30() && ((millis() - actutime) < 60000));

  if ((millis() - actutime) > 60000) {
    ligthHouse(stat, 4);
    RESET_GP();
  }
#endif

#ifdef TINY_GSM_MQTT
  configMQTT();
#endif

  Wire.end();
}



void loop() {
  Wire.begin();
  init_v_JSON();

#ifdef SHT30
  measureSHT30();
#endif
#ifdef AIR_QUALITY_4
  stat = measureSGP30();
#endif

  Serial.println(buildGenJSON());


#ifdef TINY_GSM_MQTT
  if (timeToSend) {
    bool f_time = true;
    while (((ConnectMQTT() != 0)) && (cnt_retry < MAX_RETRY)) {
      ligthHouse(stat, 5);
      cnt_retry++;
      if (!f_time) { //la primera vez omite delay después mete delay de 10seg por intento
        delay(10000);
      }
      f_time = false;
    }
    if (cnt_retry >= MAX_RETRY) {
      cnt_err++;
    }
    else {
      cnt_retry = 0;

      if (sendMQTT(buildGenJSON()) != 0) {
        ligthHouse(stat , 2); //ERROR EN EL ENVIO
        cnt_err++;
      }
      else {
        ligthHouse(stat, 1); //ENVIO CORRECTO
      }
    }
#ifdef MINI_BREAKOUT
    if (cnt_err >= MAX_ERROR) {
      RESET_GP();
    }
#else
    if (cnt_err >= (MAX_ERROR * 6)) {
      RESET_GP();
    }
#endif
  }

  if (f_newFreq) {
    DEF_WAIT_TIME = SECONDARY_FREQ;
  }
  else {
    DEF_WAIT_TIME = PRINCIPAL_FREQ;
  }

  if ((millis() - LAST_SEND_TIME) < (DEF_WAIT_TIME * 1000))
  {
    Serial.print(F("Esperando: "));
    Serial.print(((DEF_WAIT_TIME * 1000) - (millis() - LAST_SEND_TIME )) / 1000);
    Serial.println(F("s. (siguiente envío)\n\n"));
    timeToSend = false;
#if  not defined(SHT30)
    while ((millis() - LAST_SEND_TIME) < (DEF_WAIT_TIME * 1000));
#endif
  }
  else {
    Serial.println("PASO POR AQUI");
    timeToSend = true;
  }



#endif

  wtd.clear();
  Wire.end();

}

void ISR_WDT() {
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_R, LOW);
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(LED_B, HIGH);
    delay(100);
    digitalWrite(LED_B, LOW);
    delay(100);
  }
}


void RESET_GP() {
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_R, LOW);
  Serial.print(F("Reiniciando"));
  ligthHouse('D', 6);

  NVIC_SystemReset();
}
