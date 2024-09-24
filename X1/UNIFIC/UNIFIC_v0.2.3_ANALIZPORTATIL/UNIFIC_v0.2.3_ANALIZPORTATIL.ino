
#define DEBUGCODE

//#define SSM_003
#define SSM_005

#include "SDU1.h" //TIENE QUE ESTAR COMO PRIMER INCLUDE

#define PCF8575_INITIAL_VALUE 0x0000
#include <PCF8575.h> //=] I2C 16chnl
PCF8575 PCF(0x20);  //=]

#include <CRC32.h> //=)
CRC32 crc_updt; //ELIMINAR CUANDO SE REDUZCA CASO 33
#include <Arduino.h>
#include <RTCZero.h>
RTCZero rtc;
#include <WDTZero.h>
#include <Wire.h>
#include <SPI.h>
#include <Ethernet2.h>,,
#include <ArduinoRS485.h> //ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
//#include <SD.h>
#include <SdFat.h>
SdFat SD;

#include <FTP.h>//Ftp1



#include "gVariables.h"
#include "config.h"
#include "gestionTiempo.h"
#include "myROM.h"
#include "myShifft.h"
#include "SSM_IoT.h"
placa_IoT placa(5);

#include "definicionesSensorBoxV3.h"
#include "myRS485.h"
#include "RS485astican.h"
#include "analogReading.h"
//#include <SparkFunBQ27441.h>  //librería de gestión de energía de baterías
#include "powerInput.h"
#include "Filesfunctions.h"
#include "Communicaciones.h"
#include "FilesFormat.h"
#include "forecast.h"
#include "z_Actualizacion.h"
#include "zonetest.h"



WDTZero wtd;
bool cont = false;

int state = 0;

File outFile;
File testFile;
String nameFile;

const size_t capacityResponse = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(2) + 40;
DynamicJsonDocument docResponse(capacityResponse);
String body_response = "";
long localEstado = 0;
int estableGPScont = 0;

bool iddle = false;
unsigned long lastTime;
String ecoCommand = "";




void setup() {
  pinMode(DIGI1, INPUT_PULLUP);
  //attachInterrupt(DIGI1,button,FALLING);

  init_IO ();
  initComPort();

  Serial.begin(USB_BAUD);
  SerialM1.begin(M1_BAUD); //Mikro 1
  SerialM2.begin(M2_BAUD); //Mikro 2

  analogReadResolution(12);

  Wire.begin();
  delay(2000);


  //CALLBACKSMODBUS
  initCallbacks(&node1);
  initCallbacks(&node2);
  initCallbacks(&node3);

  //setupBQ27441();   //relacionado con la gestión de energía de la batería

  ina_input.begin();  //analisis de consumo de la placa
  ina_input.setpowerdown();

  placa.EnableConmutado3v();

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin();

  delay(10);

  while (!SD.begin(SD_CS)) {
    Serial.println(F("SD Card Error!")); delay(1000);
    err_code += "| x4000 |";
    _error = true;
    break;
  }
  SPI.end();
  pinMode(SD_CS, INPUT_PULLDOWN);
  Serial.println("V initialize");
  placa.Initialize();

  //initRTC();

  init_forecast();

#ifndef DEBUGCODE
  //USBDevice.detach();
#endif
  wtd.setup(WDT_TIME);
  wtd.attachShutdown(ISR_WDT);

  rtc.begin();

  init_v_JSON();

  // Serial.println("////////////NEW JSON5:" + String(buildGenJSON()));

  //  Serial.print("////////////VALJSON:");
  //  for (int i = 0; i < NUMITEMS(s_ValJSON); i++)
  //  {
  //    Serial.print(s_ValJSON[i]);
  //    Serial.print(";");
  //  }
  //  Serial.println();

}

void loop() {
  String programa = "";
  if (! _configLoaded ) {

    //programa = "c";               //SI NO HAY CONFIGURACION X: LEE CONFIGURACION V:imprime logdata
#if defined(NO_DISCONNECT)
    programa = "XOtv";
#else
    programa = "XOtSv";               //SI NO HAY CONFIGURACION X: LEE CONFIGURACION
#endif

    _configLoaded = true;
  } else {
    programa = localConfig.programa;
    if (contFallos > NUM_FALLOS_MAX) {
      programa = "VZ";  //reinicio
    }
    //Serial.print("Contador de fallos: ");
    //Serial.println(contFallos);
  }
  String cmd = "";
  unsigned int longitud = 0;
  wtd.clear();
#ifdef DEBUGCODE
  if (Serial.available()) {
    programa = Serial.readString();
  }
#endif

  while (longitud < programa.length()) {
    wtd.clear();
    Wire.begin();
    delay(1);
    String tempV, tempA, tempS = "100";
    //   printBatteryStats(tempS,tempV,tempA);
    delay(1);
    Wire.end();
    delay(1);
    pinMode(20, INPUT_PULLUP);
    pinMode(21, INPUT_PULLUP);
    if ( tempS.toFloat() <= 30) { //Condición de minimo consumo y apagado
      cmd = "M";
    } else {
      if (Serial.available()) {
        String tempSerial = Serial.readString();
        cmd = tempSerial[0];
      } else {
        cmd = programa[longitud];
      }
#ifdef DEBUGCODE
      //Serial.print("comando: ");Serial.println(cmd);
#endif
    }
    state = selectState(cmd[0]);
    //  unsigned long nowTime = millis();
    String body_response = "";
    String serv = "oceancloudbi.eu";
    String pathhh = "/b2sensor/add";
    String porti = "3000";
    //  String serv="www.google.com";
    //  String pathhh="/";
    //  String porti="443";
    int ana_result = 0;

    if (MeasureTemperature() > 60)
    {
      Serial.print("Atención!!!, temperatura de la placa elevada");
      err_code += "| x1000 |";
      _error = true;
      insertarComando("HUZ", longitud + 1, programa);
    }

    switch (state) {
      case 0:
        break;
      case 1:
        for (int j = 4; j < NUMITEMS(s_ValJSON); j++) {
          s_ValJSON[j] = "Nan";
        }
        ana_result = analogMCPRead();

        f_topic_Analog = true;
        f_topic_Analiz = false;

        // Serial.println("////////////////////NEW JSON2:" + String(buildGenJSON()));
        //        Serial.print("////////////////////VALJSON:");
        //        for (int i = 0; i < NUMITEMS(s_ValJSON); i++)
        //        {
        //          Serial.print(s_ValJSON[i]);
        //          Serial.print(";");
        //        }
        //        Serial.println();

        if (ana_result != 0) {
          retry_analog++;
          if ( ana_result > 4) Serial.println("Error en el ADC");
          else Serial.println("Error en los sensores");

          if (retry_analog == 3) {
            contFallos++;
            err_code += "| x2200 |";
            _error = true;
            insertarComando("Cv", longitud + 1, programa);
          }
          else {
            if ( retry_analog > 3 )retry_analog = 1;
            insertarComando("BA", longitud + 1, programa);
          }
        } else {
          retry_analog = 0;
          if (localConfig.desviacion != 0)
          {
            if (localAnaMap.usedch1 == 1)
              numDesv = DesviaMedida(sensCh1, prev_Ch1, 1);
            if (localAnaMap.usedch2 == 1)
              numDesv = DesviaMedida(sensCh2, prev_Ch2, 2);
            if (localAnaMap.usedch3 == 1)
              numDesv = DesviaMedida(sensCh3, prev_Ch3, 3);
            if (localAnaMap.usedch4 == 1)
              numDesv = DesviaMedida(sensCh4, prev_Ch4, 4);

#ifdef SSM_005
            if (localAnaMap.usedch5 == 1)
              numDesv = DesviaMedida(sensCh5, prev_Ch5, 5);
            if (localAnaMap.usedch6 == 1)
              numDesv = DesviaMedida(sensCh6, prev_Ch6, 6);
            if (localAnaMap.usedch7 == 1)
              numDesv = DesviaMedida(sensCh7, prev_Ch7, 7);
            if (localAnaMap.usedch8 == 1)
              numDesv = DesviaMedida(sensCh8, prev_Ch8, 8);
#endif

            if (numDesv != 0)
            {
              Serial.print("Sensor: " + String(numDesv) + "   MEDIDA OBTENIDA DESVIADA: ");
              switch (numDesv) {
                case 1:
                  Serial.print(String(sensCh1) + " respecto a la anterior: " + String(prev_Ch1));
                  break;
                case 2:
                  Serial.print(String(sensCh2) + " respecto a la anterior: " + String(prev_Ch2));
                  break;
                case 3:
                  Serial.print(String(sensCh3) + " respecto a la anterior: " + String(prev_Ch3));
                  break;
                case 4:
                  Serial.print(String(sensCh4) + " respecto a la anterior: " + String(prev_Ch4));
                  break;
#ifdef SSM_005
                case 5:
                  Serial.print(String(sensCh5) + " respecto a la anterior: " + String(prev_Ch5));
                  break;
                case 6:
                  Serial.print(String(sensCh6) + " respecto a la anterior: " + String(prev_Ch6));
                  break;
                case 7:
                  Serial.print(String(sensCh7) + " respecto a la anterior: " + String(prev_Ch7));
                  break;
                case 8:
                  Serial.print(String(sensCh8) + " respecto a la anterior: " + String(prev_Ch8));
                  break;
#endif
              }
              Serial.println(" en un margen superior al : " + String(localConfig.desviacion) + " %" );
              insertarComando("rOPS", longitud + 1, programa);
            }
            prev_Ch1 = sensCh1;
            prev_Ch2 = sensCh2;
            prev_Ch3 = sensCh3;
            prev_Ch4 = sensCh4;

#ifdef SSM_005
            prev_Ch5 = sensCh5;
            prev_Ch6 = sensCh6;
            prev_Ch7 = sensCh7;
            prev_Ch8 = sensCh8;
#endif
            numDesv = 0;
          }
        }

        delay(1);
        break;

      case 2: //'B'
        _5volts.high();
        placa.EnableANA();
        Serial.println("Alimentación de sensores analógicos encendida");
        delay(250);
        break;

      case 3: //'C'
        placa.DisableANA();
        delay(5);
        break;

      case 4://'D' Activa RS485
        //Serial.println("EL NOMBRE DEL FICHERO ES >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(logName));
        Serial.println("Alimentando RS485");
        placa.EnableRS485();
#ifdef WEATHER_ADDR
        delay(5000);
#endif

        break;


      case 5: //'E' lee estacion meteo (RS485)+DISEN
        //Serial.println("EL NOMBRE DEL FICHERO ES >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(logName));
        placa.RecStatus();
        placa.EnableConmutado3v();
        digitalWrite(MUX_SEL_A, LOW);
        digitalWrite(MUX_SEL_B, HIGH);
        delay(500);
        //clearRegister(0x0004,0x0001, 0x07D0);
        cont_errores = 0;

        ////////Medida de estacion
#ifdef WEATHER_ADDR
        retry_rs485 = 0;
        err485[0] = 0;

        do {
          err485[0] = read485data(data485, WEATHER_ADDR, TOTAL_REG485, BASE_REG);
          retry_rs485++;
          delay(100);
        } while ((err485[0] != 0) and (retry_rs485 < RETRY_485));
        if (err485[0] != 0) {
          Serial.println("Error comunicando con la estacion");
          cont_errores++;
          err_code += "| x2103 |";
          _error = true;
        }
        Serial.println(retry_rs485);
#endif

        ////////Medida de rainGauge
#ifdef RAIN_ADDR
        retry_rs485 = 0;
        err485[1] = 0;
        do {
          err485[1] = read485data(&datarain485, RAIN_ADDR, TOTAL_RAIN_REG, BASE_RAIN_REG);
          retry_rs485++;
          delay(100);
        } while ((err485[1] != 0) and (retry_rs485 < RETRY_485));
        if (err485[1] != 0) {
          Serial.println("Error comunicando con rainGauge");
          cont_errores++;
          err_code += "| x2104 |";
          _error = true;
        }
        Serial.println(retry_rs485);
#endif

        //////Medida de Intensidad luminica
#ifdef LIGHT_ADDR
        retry_rs485 = 0;
        err485[2] = 0;
        do {
          err485[2] = read485data(&dataligth485, LIGHT_ADDR, TOTAL_LIGHT_REG, BASE_LIGHT_REG);
          retry_rs485++;
          delay(100);
        } while ((err485[2] != 0) and (retry_rs485 < RETRY_485));
        if (err485[2] != 0) {
          Serial.println("Error comunicando con Sensor de luz");
          cont_errores++;
          err_code += "| x2105 |";
          _error = true;
        }
        Serial.println(retry_rs485);
#endif

        ////////Medida de UV
#ifdef UV_ADDR
        retry_rs485 = 0;
        err485[2] = 0;
        do {
          err485[2] = read485data(dataUV485, UV_ADDR, TOTAL_UV_REG, BASE_UV_REG);
          retry_rs485++;
          delay(100);
        } while ((err485[2] != 0) and (retry_rs485 < RETRY_485));
        if (err485[2] != 0) {
          Serial.println("Error comunicando con Sensor UV");
          cont_errores++;
          err_code += "| x210B |";
          _error = true;
        }
        Serial.println(retry_rs485);
#endif


        //MEDIDA DE HC_DISEN HEX big to little endian
#ifdef HC_DISEN_ADDR

        if (millis() - time_ >= 300000) {
          time_ = millis();
          limpiaDisen(HC_DISEN_ADDR); //OJO! TRAS LA LIMPIEZA TIENE UN TIEMPO DE APROX 40 SEG EN EL QUE NO ACTUALIZA EL VALOR DEL REGISTRO DE PPM HIDROC, SI ACTUALIZA TEMP

          delay(3000); //espera después de limpiar para no afectar a la lectura
        }
        err485[2] = read485HEX(dataHC485, HC_DISEN_ADDR, TOTAL_HC_REG, BASE_HC_REG);

        retry_rs485++;
        delay(100);

        if (err485[2] != 0) {
          Serial.println("Error comunicando con Sensor Hidrocarburos: " + String(HC_DISEN_ADDR));
        }
        else {
          Serial.println("HIDROCARBUROS DETECTADOS (ppm): " + String(dataHC485[1]));
          //          if (dataHC485[1] > 0) { //si se detectan hidrocarburos fuerza envío de mensaje
          //            Serial.println(">>FORZANDO ENVIO!!");
          //            insertarComando("rOQS", longitud + 1, programa);
          //          }
        }

#endif
        //////////Gestion de errores

        if (cont_errores >= 3) {
          err_code += "| x2100 |";
          _error = true;
          Serial.println("ERROR comunicando mediante RS485");
          insertarComando("FD", longitud + 1, programa); //??? CUIDADO!! PUEDE DESABILITAR LA ESTACION PRINCIPAL Y ESTROPEAR LAS MEDICIONES DE PM
          contFallos++;
          cont_errores = 0;
          break;
        }


#if defined(JSON_METEOFCAST)|| defined(JSON_METEO) //ALMACENAMIENTO EN VECTORES DE VALORES PARA JSON METEO FORECAST Y JSON METEO
#ifdef RAIN_ADDR
        s_ValJSON[takeParam("r")] = datarain485 / 10.0;
#endif
#ifdef LIGHT_ADDR
        s_ValJSON[takeParam("L")] = dataligth485;
#endif

        Serial.println(buildGenJSON());
#endif
        delay(100);
        SerialM2.end();
        placa.SetPrevStatus();
        break;


      case 6:
        placa.DisableRS485();
        delay(5);
        break;

      case 7://'G' PwrEntrada
#ifdef SSM_005
        // Serial.println("EL NOMBRE DEL FICHERO ES >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(logName));
        placa.EnableConmutado3v();// NO PONER CON 003
#endif

        Wire.begin();
        delay(1);
        ina_input.begin();
        powerIn.shuntvoltage = ina_input.getShuntVoltage_mV();
        powerIn.busvoltage = ina_input.getBusVoltage_V();
        powerIn.current_mA = ina_input.getCurrent_mA();
        powerIn.power_mW = ina_input.getPower_mW();
        powerIn.loadvoltage = powerIn.busvoltage + (powerIn.shuntvoltage / 1000);
        ina_input.setpowerdown();

        s_ValJSON[takeParam("xV")] = powerIn.busvoltage;
        s_ValJSON[takeParam("xA")] = powerIn.current_mA;

        // Serial.println("/////////////////////NEW JSON3:" + String(buildGenJSON()));
        //        Serial.print("/////////////////////VALJSON:");
        //        for (int i = 0; i < NUMITEMS(s_ValJSON); i++)
        //        {
        //          Serial.print(s_ValJSON[i]);
        //          Serial.print(";");
        //        }
        //        Serial.println();


#ifdef DEBUGCODE
        Serial.print("V.shnt: "); Serial.print(powerIn.shuntvoltage); Serial.print("v");
        Serial.print(", V.bus: "); Serial.print(powerIn.busvoltage); Serial.print("v");
        Serial.print(", Current: "); Serial.print(powerIn.current_mA); Serial.print("mA");
        Serial.print(" ,Power: "); Serial.print(powerIn.power_mW); Serial.println("mW");
#endif
        delay(1);
        Wire.end();
        delay(1);
        break;

      case 8: //'H'
#if defined(NO_DISCONNECT)
        Serial.println("Case \'H\' - can`t power off for NO_DISCONNECT");
#else
        placa.Initialize();
        placa.Disable5V();
#endif
        break;

      case 9: //'I' leeAnalizSch  (ANTES SENDSINCRODATA)
        placa.RecStatus();

        placa.EnableConmutado3v();

        //HABILITA RS485
        digitalWrite(MUX_SEL_A, LOW);
        digitalWrite(MUX_SEL_B, HIGH);
        delay(500);

        SerialM2.begin(M2_BAUD); //Schneider 9600/115200

        uint8_t contador;

        err_analiz = "0"; //codigo error analizador

        for (int j = 4; j < NUMITEMS(s_ValJSON); j++) {
          s_ValJSON[j] = "Nan";
        }
        Serial.println();
        Serial.println(" Iniciando proceso de lectura Schneider PM2200, para analizador: " + String(n_Analiz));

        err_RS = GET_METER_SYNCRO(n_Analiz);

        f_topic_Analiz = true;
        f_topic_Analog = false;

        if  (err_RS == "1") {
          err_analiz = String((0x2106 + (n_Analiz, HEX)), HEX);
        }

        Gest_Alarm_Comms();

        if (err_RS != "0")
        {
          contador++;
          if (contador < MAX_RETRAY) {
            insertarComando("I", longitud + 1, programa);
          }
          else
          {
            err_code += " | x" + String(err_analiz) + " | ";
            _error = true;
          }
        }
        else
        {
          contador = 0;
        }
        SerialM2.end();
        placa.SetPrevStatus();

        break;


      case 10:
        Serial.println("list");
        placa.RecStatus();
        placa.EnableConmutado3v();
        listaArchivos();
        placa.SetPrevStatus();
        delay(1000);
        break;

      case 11: //'K' lectura de archivo
#ifdef DEBUGCODE
        Serial.println("lectura de archivo");
        placa.RecStatus();
        placa.EnableConmutado3v();
        leerArchivo();
        placa.SetPrevStatus();
        delay(1000);
#endif
        break;

      case 12: //'L' TemperaturaPCB
        //Serial.println("EL NOMBRE DEL FICHERO ES >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >>> >> : " + String(logName));
        placa.RecStatus();
        placa.EnableConmutado3v();
        delay(5);
        sensorData.pcbTemp = MeasureTemperature();

        s_ValJSON[takeParam("xh")] = localConfig.sensorID;
        s_ValJSON[takeParam("xT")] = sensorData.pcbTemp;
        //   Serial.println("/////////////////NEW JSON1:" + String(buildGenJSON()));

        //        Serial.print("/////////////////VALJSON:");
        //        for (int i = 0; i < NUMITEMS(s_ValJSON); i++)
        //        {
        //          Serial.print(s_ValJSON[i]);
        //          Serial.print(";");
        //        }
        //        Serial.println();

#ifdef DEBUGCODE
        Serial.print("Temperatura PCB: "); Serial.println(sensorData.pcbTemp);
#endif
        placa.SetPrevStatus();
        break;


      case 13:
        // Serial.println("EL NOMBRE DEL FICHERO ES >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(logName));
#ifdef JSON_METEOFCAST
        placa.RecStatus();
        placa.EnableConmutado3v();
        pinMode(SD_CS, OUTPUT);
        digitalWrite(SD_CS, HIGH);
        SPI.begin();
        delay(10);
        Serial.println("Iniciando FORECAST");
        Serial.println(buildGenJSON());

        if (SD.begin(SD_CS)) {
          printTime();
          measurementEvent();
          funcionesSD();
          prediccion();
        }
        else {
          Serial.println(F("SD Card Error!"));
          err_code += "| x4000 |";
          _error = true;
        }
        Serial.println(buildGenJSON());
        SPI.end();
        pinMode(SD_CS, INPUT_PULLDOWN);
        placa.SetPrevStatus();
#else
        Serial.println("El JSON de METEOFORECAST no se encuentra definido");
#endif
        break;

      case 14://'N' borrar archivo
#ifdef DEBUGCODE
        placa.RecStatus();
        placa.EnableConmutado3v();
        borrarArchivo();
        placa.SetPrevStatus();
        delay(1000);
#endif
        break;

      case 15: //'O' conecta modem
        //Serial.println("EL NOMBRE DEL FICHERO ES >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(logName));
        Serial.println();
        Serial.println("/////////////////////////////////////////////////////////////////////////////////");
        Serial.println("             Estado del contador de fallos:  " + String(contFallos) + "  | de: " + String(NUM_FALLOS_MAX));
        Serial.println("/////////////////////////////////////////////////////////////////////////////////");
        Serial.println();

        wtd.clear();
        placa.RecStatus();

        current_timestamp = rtc.getEpoch();
        sum_SecAct = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));


        //if (sum_SecAct - SendTime >= _sendTime) {
        if (a_sendTime) {
          placa.EnableMIKRO();

          if (startComPort() != 0) {
            retry_connect++;
            if (retry_connect == MAX_RETRAY) {
              Serial.println("Número máximo de intentos de conexión alcanzado");
              insertarComando("Sv", longitud + 1, programa);
              contFallos++;
              err_code += "| x3000 |";
              _error = true;
            }
            else {
              if ( retry_connect > MAX_RETRAY )retry_connect = 1;
              insertarComando("SO", longitud + 1, programa);
            }
          } else retry_connect = 0;
          Serial.print("Retry connection: "); Serial.println(retry_connect);
          Serial.println();
        }
        else
        {
          Serial.println("No se ha cumplido el tiempo de envío");
          placa.DisableMIKRO();
          placa.SetPrevStatus();
        }
        break;


      case 16: //'P' Send Analog
        if (retry_connect >= MAX_RETRAY)break;
        err_com = envioDatosAnalogMuutech();
        if (err_com == 33)
        {
          Serial.println("ERROR DE COMUNICACIONES (no se ha modificado el parámetro de retorno=33)");
          contFallos++;
          err_code += "| x3FFF |";
          _error = true;

          retry_send++;
        }

        else if (err_com != 0 && err_com != 99) {
          retry_send++;
          if (retry_send == MAX_RETRAY) {
            insertarComando("Sv", longitud + 1, programa);
            contFallos++;
            err_code += "| x3001 |"; //NO SE HA ENVIADO EL MENSAJE
            _error = true;
            Serial.println("ERROR DE COMUNICACIONES (no se ha enviado el mensaje)");
          } else {
            if ( retry_send > MAX_RETRAY )retry_send = 1;
            if ( err_com == 1) { //ANTES 2
              err_code += "| x3002 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (no hay conexión con la red)");
              insertarComando("SOP", longitud + 1, programa);
            }
            if ( err_com == 2) {
              err_code += "| x3010 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (no se ha podido conectar con el servidor web HTTP)");
              insertarComando("SOP", longitud + 1, programa);
            }
            if ( err_com == 3) { //ANTES 3
              err_code += "| x3011 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al enviar el mensaje HTTP)");
              insertarComando("SOP", longitud + 1, programa);
            }
            if ( err_com == 4) { //ANTES 3
              err_code += "| x3020 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (no se ha podido conectar con el servidor web HTTPS)");
              insertarComando("SOP", longitud + 1, programa);
            }
            if ( err_com == 5) { //ANTES 3
              err_code += "| x3021 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al enviar el mensaje HTTPS)");
              insertarComando("SOP", longitud + 1, programa);
            }
            if ( err_com == 10) { //ANTES 1
              err_code += "| x3030 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al conectar con el broker MQTT)");
              insertarComando("P", longitud + 1, programa);
            }
            if ( err_com == 11) { //ANTES 1
              err_code += "| x3031 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al enviar el mensaje MQTT)");
              insertarComando("P", longitud + 1, programa);
            }
          }
        } else retry_send = 0;
        if (err_com == 99) //ANTES 4
          Serial.println("Envío suspendido hasta cumplir tiempo");
        else
          Serial.print("Retry send RS: "); Serial.println(retry_send);
        break;

      case 17:  //'Q' envio RS485
        //Serial.println("EL NOMBRE DEL FICHERO ES >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(logName));
        if (retry_connect >= MAX_RETRAY)break;

        err_com = envioDatosRS485Muutech(buildGenJSON());

        Serial.println("El err_com de envio RS es:" + String(err_com));
        if (err_com == 33)
        {
          Serial.println("ERROR DE COMUNICACIONES (no se ha modificado el parámetro de retorno=33)");
          contFallos++;
          err_code += "| x3FFF |";
          _error = true;
          retry_send++;
        }
        else if (err_com != 0 && err_com != 99) {
          retry_send++;
          if (retry_send == MAX_RETRAY) {
            insertarComando("Sv", longitud + 1, programa);
            contFallos++;
            err_code += "| x3001 |"; //NO SE HA ENVIADO EL MENSAJE
            _error = true;
          }
          else {
            if ( retry_send > MAX_RETRAY )retry_send = 1;
            if ( err_com == 1) { //ANTES 2
              err_code += "| x3002 |";
              _error = true;
              Serial.println("ERROR! El dispositivo no se encontraba conectado cuando se intentó hacer el envío(No hay conexión con la red)");
              insertarComando("SOQ", longitud + 1, programa);
            }
            if ( err_com == 2) {
              err_code += "| x3010 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al conectar con el servidor web HTTP)");
              insertarComando("SOQ", longitud + 1, programa);
            }
            if ( err_com == 3) { //ANTES 3
              err_code += "| x3011 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al enviar mensaje HTTP)");
              insertarComando("SOQ", longitud + 1, programa);
            }
            if ( err_com == 4) { //ANTES 3
              err_code += "| x3020 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al conectar con el servidor web HTTPS)");
              insertarComando("SOQ", longitud + 1, programa);
            }
            if ( err_com == 5) { //ANTES 3
              err_code += "| x3021 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al enviar mensaje HTTPS)");
              insertarComando("SOQ", longitud + 1, programa);
            }
            if ( err_com == 10) { //ANTES 1
              err_code += "| x3030 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al conectar con broker MQTT)");
              insertarComando("fQ", longitud + 1, programa);
            }
            if ( err_com == 11) { //ANTES 1
              err_code += "| x3031 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al enviar el mensaje MQTT)");
              insertarComando("fQ", longitud + 1, programa);
            }
          }
        } else retry_send = 0;
        if (err_com == 99) //ANTES 4
          Serial.println("Envío suspendido hasta cumplir tiempo");
        else
          Serial.print("Retry send RS: "); Serial.println(retry_send);

        if (err_com == 0) //SI SE HA ENVIADO EL DATO//CLEAR CONTINUOUS REGISTER RAIN ACUMULADO
        {
#ifdef RAIN_ADDR
          err485[1] = clearRegister(CLEAR_DAT, RAIN_ADDR, BASE_RAIN_REG);
#endif
          WMaxPrev = 0;
        }
        break;


      case 18:// 'R' EnviaAnalizPM (SYNCRO?)
        if (retry_connect >= MAX_RETRAY)break;
        err_com = EnvioMuutech(DATA_METER_PM2200, String(INF_ALARM[c_alarm_comms]));
        if (err_com == 33)
        {
          Serial.println("ERROR DE COMUNICACIONES (no se ha modificado el parámetro de retorno=33)");
          contFallos++;
          err_code += "| x3FFF |";
          _error = true;
          retry_send++;
        }
        else if (err_com != 0 && err_com != 99) {
          retry_send++;
          if (retry_send == MAX_RETRAY) {
            insertarComando("Sv", longitud + 1, programa);
            contFallos++;
            err_code += "| x3001 |";//NO SE HA ENVIADO EL MENSAJE
            _error = true;
          } else {
            if ( retry_send > MAX_RETRAY )retry_send = 1;
            if ( err_com == 1) { //ANTES 2
              err_code += "| x3002 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (no hay conexión con la red)");
              insertarComando("SOR", longitud + 1, programa);
            }
            if ( err_com == 2) {
              err_code += "| x3010 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al conectar con el servidor web HTTP)");
              insertarComando("SOR", longitud + 1, programa);
            }
            if ( err_com == 3) { //ANTES 3
              err_code += "| x3011 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al enviar el mensaje HTTP)");
              insertarComando("SOR", longitud + 1, programa);
            }
            if ( err_com == 4) { //ANTES 3
              err_code += "| x3020 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al conectar con servidor web HTTPS)");
              insertarComando("SOR", longitud + 1, programa);
            }
            if ( err_com == 5) { //ANTES 3
              err_code += "| x3021 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al enviar el mensaje HTTPS)");
              insertarComando("SOR", longitud + 1, programa);
            }
            if ( err_com == 10) { //ANTES 1
              err_code += "| x3030 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al conectar con broker MQTT)");
              insertarComando("R", longitud + 1, programa);
            }
            if ( err_com == 11) { //ANTES 1
              err_code += "| x3031 |";
              _error = true;
              Serial.println("ERROR DE COMUNICACIONES (error al enviar mensaje MQTT)");
              insertarComando("R", longitud + 1, programa);
            }
          }
        } else retry_send = 0;
        if (err_com == 99) //ANTES 4
          Serial.println("Envío suspendido hasta cumplir tiempo");
        else
          Serial.print("Retry send Muutech: "); Serial.println(retry_send);
        break;


      case 19://'S'
        // Serial.println("EL NOMBRE DEL FICHERO ES >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(logName));
#if defined(TINY_GSM_MODEM_SARAR4)&&defined(TINY_GSM_MQTT)
        //   delay(150);
        //    modem.gprsDisconnect();
        //   delay(150);
        ///   modem.radioOff();
        //    delay(150);
        //   modem.poweroff();
        //    delay(150);
        placa.DisableMIKRO();
#else
        //   modem.gprsDisconnect();
        //   delay(100);
        //   modem.poweroff();
        //  delay(100);
        placa.DisableMIKRO();
        delay(3000); //antes 3000
#endif
        Serial.println("MIKROE desconectado");
        break;


      case 20:
        gestionFrecuencias(sensCh1, sensCh2, sensCh3, sensCh4);
        break;

      case 21: //'U'
        //  Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>estado de flag: "+String(a_sendTime));
        // Serial.println("EL NOMBRE DEL FICHERO ES >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(logName));

        //current_timestamp = rtc.getEpoch();

        //sum_SecAct = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));

        // if (sum_SecAct - SendTime < _sendTime) { //SI NO SE HA CUMPLIDO EL TIEMPO DE ENVIO
        if (!a_sendTime) { //SI NO SE HA CUMPLIDO EL TIEMPO DE ENVIO
          if (iddle) {
            if ( millis() - lastTime >= (iddleTime * 1000)) {
              iddle = false;
            }
            // Serial.println("millis()-lastTime= " + String(millis() - lastTime) + "   lastTime: " + String(lastTime) + "  de:  " + String(iddleTime * 1000));
          } else {
            lastTime = millis();
            iddle = true;
#ifdef DEBUGCODE
            Serial.print("Esperando: "); Serial.print(iddleTime); Serial.println(" s  para la siguiente medida");
#endif
          }
        }
        else {
          iddle = false;
          Serial.println("Tiempo entre envíos: " + String(_sendTime) + "  s.");
          Serial.print("Suspendido tiempo entre mediciones debido a Alarma de envios");
          Serial.println();
          //Serial.print("Suspendido tiempo entre mediciones | Tiempo desde el último envío : " );
          // Serial.println(String( sum_SecAct - SendTime) + "  s.");
        }
        break;

      case 22: //'V' logdata
        //Serial.println("EL NOMBRE DEL FICHERO ES >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(logName));
        placa.RecStatus();
        placa.EnableConmutado3v();
        Serial.println();
        Serial.print("Time: "); Serial.print(localTime);
        Serial.print("\t BoardTemp: "); Serial.print(MeasureTemperature());
        Serial.print("\t Input V: "); Serial.print(powerIn.busvoltage);
        Serial.print("\t Input mA: "); Serial.print(powerIn.current_mA);
        Serial.print("\t Sensor ID: "); Serial.print(localConfig.sensorID);
        Serial.print("\t SensCh1 (4-20mA): "); Serial.print(sensCh1);
        Serial.print("\t SensCh2 (4-20mA): "); Serial.print(sensCh2);
        Serial.print("\t SensCh3 (4-20mA): "); Serial.print(sensCh3);
        Serial.print("\t SensCh4 (4-20mA): "); Serial.println(sensCh4);

        pinMode(SD_CS, OUTPUT);
        digitalWrite(SD_CS, HIGH);
        SPI.begin();

        delay(10);
        //(!card.init(SPI_HALF_SPEED, SD_CS)) {
        if (!SD.begin(SD_CS)) {
          Serial.println(F("SD Card Error!")); delay(1000);
          err_code += "| x4000 |";
          _error = true;
          SPI.end();
          pinMode(SD_CS, INPUT_PULLDOWN);
          placa.SetPrevStatus();
          break;
        }
        Serial.println("Buscando el fichero: " + String(logName));
        if (!SD.exists(logName)) {  // Si no existe el fichero
          outFile = SD.open(logName, FILE_WRITE); //Lo crea
          if (!outFile) { //Si no se genera
            Serial.println("Error abriendo el archivo para escritura: " + String(logName));
            err_code += "| x4102 |";
            _error = true;
            break;
          }
          else  //Si se genera
          {
            outFile.println(logFileText);
            outFile.println();
            Serial.println("Se ha generado el fichero: " + String(logName) + " desde case V");
            outFile.close();
          }
        } //else {  //Si ya existia
        outFile = SD.open(logName, FILE_WRITE);
        if (!outFile) {
          Serial.println("Error abriendo el archivo para escritura: " + String(logName));
          err_code += "| x4102 |";
          _error = true;
          break;
        }
        else
        {


          current_timestamp = rtc.getEpoch();

          localTime = String(year(current_timestamp));
          if (month(current_timestamp) > 9) {
            localTime += ("/" + String(month(current_timestamp)));
          }
          else
            localTime += ("/0" + String(month(current_timestamp)));

          if (day(current_timestamp) > 9) {
            localTime += ("/" + String(day(current_timestamp)));
          }
          else
            localTime += ("/0" + String(day(current_timestamp)));


          localTime += (" " + String(hour(current_timestamp)));
          localTime += (":" + String(minute(current_timestamp)));
          localTime += (":" + String(second(current_timestamp)));
          Serial.println("El localtime de V case 22 es: " + String(localTime));
          Serial.println("GUARDADO DESDE CASE 22 'V'");
          writeLogFile(outFile, localTime, LogJSON, &_reset, &_error, &err_code, programa);
          outFile.close();
        }
        //  }

        // writeLogFile(outFile, localTime, LogJSON, &_reset, &_error, &err_code, programa);
        //writeLogFile(outFile,localTime,sensorData.pcbTemp,powerIn.busvoltage,powerIn.current_mA,localConfig.sensorID, sensCh1,sensCh2,sensCh3,sensCh4,&_failConnect,&_sendData,&_reset,&_error,programa);
        //writeLogFile(outFile,localTime,sensorData.pcbTemp,powerIn.busvoltage,powerIn.current_mA,localConfig.sensorID, sensCh1,sensCh2,sensCh3,sensCh4, sensCh5,sensCh6,sensCh7,sensCh8,&_failConnect,&_sendData,&_reset,&_error,programa);


        SPI.end();
        pinMode(SD_CS, INPUT_PULLDOWN);
        placa.SetPrevStatus();
        break;



      case 23:
#ifdef DEBUGCODE
        Serial.println(commandMenu);
#endif
        break;
      case 24:// 'X' ReadConfig
        placa.RecStatus();
        placa.EnableConmutado3v();
        pinMode(SD_CS, OUTPUT);
        digitalWrite(SD_CS, HIGH);
        SPI.begin();
        delay(10);

        if (!SD.begin(SD_CS)) {
          Serial.println(F("SD Card Error!")); delay(1000);
          err_code += "| x4000 |";
          _error = true;

          Serial.println("Failing reading SD card.");
          SPI.end();
          pinMode(SD_CS, INPUT_PULLDOWN);
          placa.SetPrevStatus();
          loadDefualt(&localConfig, &localAnaMap, &localConfigMQTT);
          updatefreq(localConfig.measureFreq, localConfig.waitTime);
          printConig(&localConfig, &localAnaMap, &localConfigMQTT);
          break;
        } else {
          //volume.init(card);
          Serial.println("Wiring is correct and a card is present.");
        }
        if (!SD.exists(configName)) {
          Serial.println("config.txt does not exist");
          Serial.print("creating file");
          outFile = SD.open(configName, FILE_WRITE);
          if (!outFile) {
            Serial.println("Error abriendo el archivo para escritura: " + String(configName));
            err_code += "| x4102 |";
            _error = true;
          }
          Serial.print(".");
          _createConfigFile(outFile, &defaultConfig, &defaultAnaMap, &defaultConfigMQTT);
          Serial.print(".");
          outFile.close();
          Serial.println(".Done");
        }
        outFile = SD.open(configName, FILE_READ);
        if (!outFile) {
          Serial.println("Error abriendo el archivo para lectura: " + String(configName));
          err_code += "| x4101 |";
          _error = true;
        }
        _readConfigFile(outFile, &localConfig, &localAnaMap, &localConfigMQTT);
#ifdef DEBUGCODE
        outFile = SD.open(configName, FILE_READ);
        if (!outFile) {
          Serial.println("Error abriendo el archivo para lectura: " + String(configName));
          err_code += "| x4101 |";
          _error = true;
        }
        while (outFile.available()) {
          Serial.write(outFile.read());
        }
#endif
        updatefreq(localConfig.measureFreq, localConfig.waitTime);

        SPI.end();
        pinMode(SD_CS, INPUT_PULLDOWN);
        placa.SetPrevStatus();
        break;

      case 25:
        GEST_FREQ();
        break;

      case 26:
        placa.DisableMIKRO();
        Serial.print("Reiniciando el dispositivo");
        for (int i = 0; i < 5; i++)
        {
          Serial.print(".");
          delay(1000);
        }

        NVIC_SystemReset();
        break;

      case 27://'a' cambia dirección: estacion:2 pluviometro:3 sensor luz:4 uv:5
        Serial.println("Modificando dirección");
        placa.RecStatus();
        placa.EnableConmutado3v();
        digitalWrite(MUX_SEL_A, LOW);
        digitalWrite(MUX_SEL_B, HIGH);
        delay(500);
        err485[0] = clearRegister(0x0003, 0x0001, 0x07D0); // asigna nueva ADDR(DIRECCION NUEVA,DIRECCION ANTERIOR,REGISTRO DE ID)
        Serial.println(err485[0]);

        break;
      case 28: //'b'
        placa.EnableMIKRO();
        digitalWrite(M1_RST, HIGH);
        digitalWrite(M1_CS, HIGH);
        delay(2000);

#ifdef TINY_GSM_MODEM_ESP32
        modem.getListAp();  //SOLO DISPONIBLE CON ESP32 NO CONFIGURADA PARA EL RESTO
        modem.searchNearestAp(localConfig.ssid.c_str());  //SOLO DISPONIBLE CON ESP32 NO CONFIGURADA PARA EL RESTO
        sigQ = modem.getSignalQuality();
        Serial.print("Signtal quality: ");
        Serial.println(sigQ);
#endif
        break;

      case 29: //'c' ecomode ECOCOMMAND
        bool F_wtd;
        F_wtd = true;
        placa.EnableMIKRO();

        //   digitalWrite(M1_RST,HIGH);
        //  digitalWrite(M1_CS,HIGH);
        Serial.println("Starting Eco");
        while (true) {
          if (F_wtd == false)
            wtd.clear();
          if (Serial.available()) {
            ecoCommand = Serial.readString();
            if (ecoCommand[0] == 'Q') {
              Serial.println("Finish");
              Serial.println("Reestableciendo WatchDog");
              F_wtd = true;
              break;
            }
            else if (ecoCommand[0] == 'W') {
              Serial.println("Quitando WatchDog");
              F_wtd = false;
            }
            SerialM1.println(ecoCommand);
          }
          if (SerialM1.available()) {
            Serial.write(SerialM1.read());
          }
        }
        break;

      case 30:
#ifdef JSON_AMPRS
        placa.RecStatus();
        placa.EnableConmutado3v();
        digitalWrite(MUX_SEL_A, LOW);
        digitalWrite(MUX_SEL_B, HIGH);
        delay(500);
        cont_errores = 0;

        uint8_t ret;
        uint8_t retry;
        ret = 0;
        retry = 0;

        Serial.println("Iniciando lectura de amperimetros...");

        ////////////Medida de amperimetros
        retry_rs485 = 0;
        err485[3] = 0;
        err485[4] = 0;
        err485[5] = 0;
        err485[6] = 0;

        float datosAmp[8]; //4 amperimetros 2 datos por amperimetro amp/frec

        //AMP1
        ret = Read_amp_RS485(&datosAmp[0], AMP1_ADDR);
        if (ret == 2)
        {
          delay(100);
          Read_amp_RS485(&datosAmp[0], AMP1_ADDR);
          ret = 0;
        }

        //AMP2
        ret = Read_amp_RS485(&datosAmp[2], AMP2_ADDR);
        if (ret == 2)
        {
          delay(100);
          Read_amp_RS485(&datosAmp[2], AMP2_ADDR);
          ret = 0;
        }

        //AMP3
        ret = Read_amp_RS485(&datosAmp[4], AMP3_ADDR);
        if (ret == 2)
        {
          delay(100);
          Read_amp_RS485(&datosAmp[4], AMP3_ADDR);
          ret = 0;
        }

        //AMP4
        ret = Read_amp_RS485(&datosAmp[6], AMP4_ADDR);
        if (ret == 2)
        {
          delay(100);
          Read_amp_RS485(&datosAmp[6], AMP4_ADDR);
          ret = 0;
        }

        Serial.println("Retry_rs485: " + String(retry_rs485));

        //////////Gestion de errores
        if (cont_errores >= 4) {
          Serial.println("ERROR!!! PROBLEMA CON COMUNICACION RS485 No se puede comunicar con ninguno de los amperimetros");
          contFallos++;
          cont_errores = 0;
          err_code += "| x2101 |";
          _error = true;
          break;
        }

        Serial.println(buildGenJSON());

        delay(100);
        SerialM2.end();
        placa.SetPrevStatus();
#else
        Serial.println("El JSON de AMPERIMETROS no se encuentra definido");
#endif
        break;


      case 31: //'e' Gestión RS485-ETH Analiz y Schneider RS485

        //        bool f_send;
        //        f_send = false;
        //
        //        Serial.print("EL ERR_COM PARA CASE 'e' es:");
        //        Serial.println(err_com);
        //
        //        if (err_com == 0) //si se ha enviado todo correctamente
        //        {
        //          n_Analiz++;    //aumentamos al siguiente analizador
        //          f_send = true;
        //        }
        //        else if (err_com != 0 && err_com != 99) { // si hay error de coms
        //          if (retry_send == MAX_RETRAY) {  //si hemos llegado al maximo de intentos de reenvio
        //            n_Analiz++; //aumentamos analizador
        //            f_send = true;
        //          }
        //        }

        n_Analiz++;
        //        if (f_send == true)
        //        {
        if ((n_Analiz > 0) && (n_Analiz <= NUMSLAVES_RJ45))
        {
          Serial.println("Siguiente analizador: " + String(n_Analiz));
#ifdef ETH_MODEM_W5500
          insertarComando("rsOQve", longitud + 1, programa);
          Serial.println("Añadiendo \"rsOQve\"");

#elif defined(JSON_RED)
          insertarComando("rIOQve", longitud + 1, programa);
          Serial.println("Añadiendo \"rIOQve\"");
#else
          Serial.println("NO SE HA DEFINIDO JSON_RED ni ETH_MODEM_W5500");
#endif
        }
        else {
          Serial.println("Todos los analizadores leidos\r\n\r\n");
          n_Analiz = 1;

          for (int j = 4; j < NUMITEMS(s_ValJSON); j++) {
            s_ValJSON[j] = "Nan-";
          }
        }
        //        }
        break;

      case 32: //'f' MQTT Discconect
#if defined(TINY_GSM_MODEM_SARAR4)||defined(TINY_GSM_MODEM_BC95G)
        if (F_BrokerMQTT) {
          Serial.println("Desconectando de broker MQTT");

          if (modem.mqttDisc())
            Serial.println("Desconexión de broker exitosa");
          else
            Serial.println("Error al desconectar el broker");

          F_BrokerMQTT = false;
        }
        else
          Serial.println("Broker no conectado");

#else
        Serial.println("ERROR!!!!! Asegurese de que el modem es compatible con la función mqttdisc()");
#endif
        break;



      case 33: //'g' COMPROBACION DE ACTUALIZACION HTTP =)
        //Una vez descargado el archivo y reiniciado el dispositivo, este puede tardar unos minutos hasta volver a estar operativo
        wtd.clear();    //REFRESCA WATCHDOG PARA EVITAR REINICIOS POR DEMORAS EN EL ENVIO
        updateFirmware();



        while (1);


        //////////////////////////////////////////////////////////////////////////////
#if defined(TINY_GSM_MODEM_SARAR4)
        crc_updt.reset();

        Serial.println("Iniciando busqueda de actualización");

        //known_CRC32 = 0x967D5005;  //CRC CONOCIDO DEL ARCHIVO A DESCARGAR

        dwld_fN = "/UPDATE.bin"; //"/2k-CRC.txt";  //archivo a descargar del servidor
        save_fN = "NEWUPDATE.bin"; //archivo donde se guardará inicialmente

        pinMode(SD_CS, OUTPUT);
        digitalWrite(SD_CS, HIGH);
        SPI.begin();

        delay(10);

        if (!SD.begin(SD_CS)) {
          Serial.println("Error al iniciar SD, saliendo de busqueda de actualización");
          delay(1000);
          err_code += "| x4000 |";
          _error = true;
          SPI.end();
          pinMode(SD_CS, INPUT_PULLDOWN);
          placa.SetPrevStatus();
        }
        else {

          if (SD.exists(save_fN))
            SD.remove(save_fN);

          outFile = SD.open(save_fN, FILE_WRITE );

          //PETICION
          if (client.connect("mytest-003.s3.amazonaws.com", 80, 20000)) {
            Serial.println("Conectado con: mytest-003.s3.amazonaws.com");
            Serial.println("Preparando para enviar: \r\n" );

            Serial.println("GET " + dwld_fN + " HTTP/1.1"); //GET /prueba07-01.txt HTTP/1.1   //GET / HTTP/1.1
            Serial.println("Host: mytest-003.s3.amazonaws.com");//Host: mytest-003.s3.amazonaws.com  //Host: mytest-003.s3-website-us-east-1.amazonaws.com
            Serial.println("Connection: keep-alive");
            Serial.println();

            client.println("GET " + dwld_fN + " HTTP/1.1"); //GET /prueba07-01.txt HTTP/1.1   //GET / HTTP/1.1
            client.println("Host: mytest-003.s3.amazonaws.com");//Host: mytest-003.s3.amazonaws.com  //Host: mytest-003.s3-website-us-east-1.amazonaws.com
            client.println("Connection: keep-alive");
            client.println();

            content_L = 0;

            //LECTURA DE CABECERA
            Serial.println(">CABECERA:");
            while (true) {
              if (client.available()) {
                Serial.print('>');
                String line = client.readStringUntil('\n');
                line.trim();
                line.toLowerCase();
                Serial.println(line);
                if (line.startsWith("content-length")) {
                  content_L = line.substring(line.lastIndexOf(':') + 1).toInt();
                  Serial.println(content_L);
                }
                else if (line.length() == 0) {
                  Serial.println('<');
                  break;
                }
              }
            }





            Serial.println("Generando nuevo archivo: " + dwld_fN);

            t_ = 0;
            time_ = millis();

            //LECTURA DE BODY Y GENERACION DE ARCHIVO
            while (t_ < content_L && (millis() - time_) < 20000) {
              if (client.available()) {
                outFile.write(client.read());
                t_++;
              }
              if (t_ % 5000 == 0) {
                Serial.print("/////////////////   ");
                Serial.print(t_);
                Serial.println("   ///////////////");
                time_ = millis();
              }
            }
            outFile.close();

            Serial.println();
            Serial.println("Leido: " + String(t_));
            Serial.println();

            //IMPRESION DE ARCHIVO Y CALCULO DE CRC32
            Serial.println("DESCARGA COMPLETADA, INICIANDO IMPRESION:");
            delay(3000);

            outFile = SD.open(save_fN, FILE_READ);

            t_ = 0;
            known_CRC32 = "";

            while (t_ < content_L - 8) {
              char c = outFile.read();
              Serial.write(c);
              crc_updt.update(c);
              t_++;
            }
            Serial.println("\r\n");

            Serial.print("CheckSum: ");

            while (outFile.available() && known_CRC32.length() <= 8 ) {
              char c = outFile.read();
              Serial.write(c);
              known_CRC32 += c;
            }
            outFile.close();
            checkSumCRC32 = crc_updt.finalize();

            Serial.println();

            str_aux = String(checkSumCRC32, HEX);
            str_aux.toUpperCase();

            //COMPROBACIÓN DE CRC32
            // known_CRC32 = "543C962B"; //SOLO PARA PRUEBAS, PONER DIRECTAMENTE EL VALOR
            Serial.print("El CRC32 calculado es: ");
            Serial.print(str_aux);
            Serial.print("\t\tEl CRC32 leido es: ");
            Serial.println(known_CRC32);

            if (str_aux == known_CRC32.c_str()) {
              Serial.println();
              Serial.println("PRUEBA DE CRC SUPERADA");
              Serial.println(checkSumCRC32, HEX);
            }
            else {
              Serial.println("NO HA PASADO EL TEST DE CRC");
              Serial.println(checkSumCRC32, HEX);
              Serial.println(known_CRC32);

              //meter flag de repetición hasta mayor que 3 meter error de checksum
              //preparar recopilación de errores de actualización para enviar por http

              insertarComando("g", longitud + 1, programa);
              break;
            }

            while (1);

            /////////////////////////////////////////borrado de config.txt///////////////////////
            //            if (SD.begin(SD_CS)) {
            //              Serial.println("Wiring is correct and a card is present.");
            Serial.println("Eliminando archivo de config:" + configName);
            if (!SD.exists(configName)) Serial.println("File does not exist");
            else {
              SD.remove(configName);
              delay(1000);
              if (!SD.exists(configName)) Serial.println("File removed");
              else Serial.println("Fail removing file");
            }
            //            }
            //            else {
            //              Serial.println(F("SD Card Error!")); delay(1000);
            //              err_code += "| x4000 |";
            //              _error = true;
            //            }
            ///////////////////////////////////////////////////////////////////////////

            outFile = SD.open(save_fN,  O_WRONLY);

            Serial.println("Renombrando el archivo de descarga");
            if (!outFile.rename("UPDATE.bin")) {
              Serial.println("No se ha podido renombrar el archivo");
            }
            else {
              if (SD.exists(configName))
                Serial.println("Archivo renombrado");
              else
                Serial.println("Error al renombrar el archivo");
            }
            Serial.println();
            Serial.println("FIN");
            placa.DisableMIKRO();
            Serial.println("Reiniciando... rezando por correcta actualización");
            //while (true);
            insertarComando("Z", longitud + 1, programa); //REINICIAR PARA QUE SE REALICE LA ACTUALIZACION
            while (true);
          }
          else {
            retry_actu++;
            if (retry_actu == MAX_RETRAY) {
              contFallos++;
              err_code += "| x1101 |"; //NO SE HA PODIDO GUARDAR LA ACTUALIZACION DEL SISTEMA
              _error = true;
              Serial.println("No se ha podido descargar el archivo de actualización");
              break;
            }
            else {
              if ( retry_actu < MAX_RETRAY ) {
                err_code += "| x3020 |";
                _error = true;
                Serial.println("ERROR DE COMUNICACIONES (no se ha podido conectar con el servidor web HTTPS), reintentando...");
                insertarComando("g", longitud + 1, programa); //VOLVER A INTENTARLO
              }
            }
          }
        }
#else
        Serial.println("La actualización FOTA solo se encuentra disponible para el módulo SaraR410m");
#endif

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //  if (modem.isNetworkConnected())
        //     testBinDwld2(serverDwld, 80, resourceDwld); //argumentos: servidor , puerto, recurso
        ////////////////////////////////////////////////////////////////////////////////////////////////
        //        if (modem.isNetworkConnected()) {
        //          //
        //          // COMPROBAR SI HAY ACTUALIZACION
        //          // bool f_Actu = checkActu(); //argumentos: servidor , puerto, recurso
        //          // if (f_Actu==0){
        //          //    Serial.println("No hay actualizaciones disponibles");
        //          //    break;
        //          // }
        //          // else{
        //          //    Serial.println("Actualizaciones disponibles");   -----> OBTENER TAMBINE INFORMACION SOBRE ARCHIVOS FRAGMENTADOS, NUMERO DE PARTES Y CRC32 DE CADA UNA DE ELLAS
        //
        //          String body2 = downloadFileActu();
        //
        //          if (body2.length() == 1) {
        //            retry_send2++;
        //            if (retry_send2 == MAX_RETRAY) {
        //              contFallos++;
        //              err_code += "| x1100 |"; //NO SE HA PODIDO DESCARGAR LA ACTUALIZACION  DEL SISTEMA
        //              _error = true;
        //
        //              //
        //              //ENVIAR UN MENSAJE AVISANDO DE QUE NO SE HA PODIDO ACTUALIZAR
        //              //
        //              break;
        //            }
        //            else {
        //              if ( retry_send2 > MAX_RETRAY ) retry_send2 = 1;
        //
        //              if (body2 == "1") {  //Error al conectar mediante HTTPS
        //                err_code += "| x3020 |";
        //                _error = true;
        //                Serial.println("ERROR DE COMUNICACIONES (no se ha podido conectar con el servidor web HTTPS), reintentando...");
        //                insertarComando("g", longitud + 1, programa); //VOLVER A INTENTARLO
        //              }
        //              else if (body2 == "2") { //Error en la respuesta del servidor
        //                err_code += "| x3021 |";
        //                _error = true;
        //                Serial.println("ERROR DE COMUNICACIONES (error al enviar el mensaje HTTPS), reintentando...");
        //                insertarComando("g", longitud + 1, programa); //VOLVER A INTENTARLO
        //              }
        //            }
        //          }
        //          else { //SI EL ARCHIVO DESCARGADO ES VALIDO
        //            uint8_t err_WrtUpdt = writeUpdateFile(outFile, body2);
        //
        //
        //            if (err_WrtUpdt == 0) {
        //
        //             //
        //             // ENVIAR UN MENSAJE AVISANDO DE QUE EL ARCHIVO SE HA GUARDADO DE FORMA CORRECTA
        //             //
        //
        //              retry_send2 = 0;
        //              Serial.println("GUARDADO FINALIZADO.");
        //              delay(10000);
        //           //   insertarComando("Z", longitud + 1, programa); //REINICIAR PARA QUE SE REALICE LA ACTUALIZACION
        //              break;
        //            } else {
        //              retry_send2++;
        //
        //              if (retry_send2 == MAX_RETRAY) {
        //                contFallos++;
        //                err_code += "| x1101 |"; //NO SE HA PODIDO GUARDAR LA ACTUALIZACION DEL SISTEMA
        //                _error = true;
        //                Serial.println("No se ha podido guardar el archivo de actualización");
        //
        //                //
        //                //ENVIAR UN MENSAJE AVISANDO DE QUE EL ARCHIVO NO SE HA GUARDADO DE FORMA CORRECTA
        //                //
        //
        //              } else {
        //                if ( retry_send2 > MAX_RETRAY ) retry_send2 = 1;
        //                if ( err_WrtUpdt != 0 ) {
        //                  Serial.println("Error al guardar el archivo de actualización, reintentando...");
        //                  insertarComando("g", longitud + 1, programa); //VOLVER A INTENTARLO
        //                }
        //              }
        //
        //              //
        //              //TRAS LA ACTUALIZACION SI ES SATISFACTORIA ENVIAR UN MENSAJE INDICANDO QUE EL DISPOSITIVO SE HA ACTUALIZADO COMPLETAMENTE
        //              //
        //
        //            }
        //          }
        //        }
        //        else {
        //          Serial.println("ERROR MODEM NO CONECTADO, reintentando");
        //          contFallos++;
        //          insertarComando("SOg", longitud + 1, programa);
        //        }
        break;



      case 44: // 'r' Reset sendTime
        SendTime = -214783647;
        a_sendTime = true;
        Serial.println("Tiempo de envío reseteado");
        break;

      case 45: // 's' lectura SiemensPAC
#ifdef ETH_MODEM_W5500
        uint8_t f_count;
        // tiempopasado=0; //TESTXAVI

        for (int j = 4; j < NUMITEMS(s_ValJSON); j++)
        {
          s_ValJSON[j] = "Nan";
        }
        // Serial.println("/////////////////////////////////////////NEW JSON4:" + String(buildGenJSON()));
        //        Serial.print("/////////////////////////////////////////////VALJSON:");
        //        for (int i = 0; i < NUMITEMS(s_ValJSON); i++)
        //        {
        //          Serial.print(s_ValJSON[i]);
        //          Serial.print(";");
        //        }
        //        Serial.println();

        Serial.println();
        Serial.println("Inician do proceso de lectura Siemens PAC... para Analizador" + String(n_Analiz + 1));
        placa.EnableMIKRO();
        err_ME =  ModbusEthernet();
        Serial.println("err_ME:" + String(err_ME));
        Serial.println(buildGenJSON());

        if (err_ME != "0")
        {
          f_count++;
          if (f_count < MAX_RETRAY)
            insertarComando("s", longitud + 1, programa);
          else
          {
            err_code += "| x" + String(err_ME) + " |"; //Return err_analiz
            _error = true;
          }
        }
        else
        {
          f_count = 0;
        }

#else
        Serial.println("ETH_MODEM_W5500 no ha sido definido");
#endif
        break;


      case 46: // 't'
        Serial.println("///////VERIFICANDO FECHA/////////");

        current_timestamp = rtc.getEpoch();
        Serial.println(day(current_timestamp));
        Serial.println((localTime.substring(8, 10)).toInt());
        Serial.println(month(current_timestamp));
        Serial.println((localTime.substring(5, 7)).toInt());
        Serial.println(year(current_timestamp));
        Serial.println((2000 + ((localTime.substring(2, 4)).toInt())));

        if ((!(year(current_timestamp) >= 2021)) && (day(current_timestamp) == ((localTime.substring(8, 10)).toInt()) && month(current_timestamp) == ((localTime.substring(5, 7)).toInt()) && year(current_timestamp) == (2000 + ((localTime.substring(2, 4)).toInt()))))
        {
          //   modem.poweroff();
          //   delay(100);
          //   placa.DisableMIKRO();
          break;
        }
        else
        {
          Serial.println("");
          Serial.println("Iniciando actualización de tiempo diaria");
          Serial.println("");
          placa.RecStatus();

          localTime = TakeTime();
          if (localTime == "NoInit_2")
            TakeTime();
          Serial.print("TimeStamp:");
          Serial.println(localTime);

          if (localTime == "NoInit_0" || localTime == "NoInit_1" || localTime == "NoInit_2") {
            Serial.println("retryconnect2:" + String(retry_connect2));
            retry_connect2++;
            contFallos++;
            err_code += "| x3X02 |";
            _error = true;
            if ( retry_connect2 > MAX_RETRAY) Serial.println("Fallo en MIKROE");
            else Serial.println("Fallo en la comunicacion, retry connect numero: " + String(retry_connect2) + " DE MAX_RETRAY: " + String(MAX_RETRAY));
            if (retry_connect2 >= MAX_RETRAY)
            {
              contFallos++;
              err_code += "| x3X03 |";
              _error = true;
              Serial.println("Número máximo de reintentos alcanzado");

              insertarComando("S", longitud + 1, programa);
              retry_connect2 = 1;
              //METER AQUI ERROR POR NO ACTUALIZACION DE TIEMPO
              Serial.println("No ha sido posible actualizar el tiempo, estableciendo tiempo 2021/01/01 0:0:0");

              const byte year = 21;
              const byte month = 01;
              const byte day = 01;

              const byte hours = 0;
              const byte minutes = 0;
              const byte seconds = 0;

              localTime = "2021/01/01 00:00:00";

              initRTC(localTime);

            }
            else {
#if defined(NO_DISCONNECT)
              insertarComando("rOt", longitud + 1, programa);
#else
              insertarComando("SrOt", longitud + 1, programa);
#endif
            }
          } else
          {
            Serial.println("Tiempo actualizado");
            retry_connect2 = 0;
            Serial.println("retryconnect3:" + String(retry_connect));
            initRTC(localTime);
            placa.SetPrevStatus();
          }
        }
        break;


      case 47: //'u' UploadFile
        Serial.println();
        Serial.println("Iniciando subida de archivos...");

        if (modem.isNetworkConnected()) {
          //  uploadMIKROE("pruebaX1UP.txt", 'S', "f8i1bdejsc.execute-api.us-east-1.amazonaws.com", "/pruebas/mytest-003/X1send.txt"); //ENVIO DE ARCHIVOS (M:MQTT, H:HTTP, S:HTTPS, F:FTTP)
          uploadMIKROE("pruebaX1UP.txt", 'S', "industrial.api.ubidots.com", "/api/v1.6/devices/ssm-device_02", 9812);
        }
        else
          Serial.println("MIKROE NO CONECTADO A LA RED");

        while (1);
        //              Serial.println("PRUEBA SUBIDA ARCHIVO A S3 COMO EN POSTMAN APIREST");
        //
        //#if defined(TINY_GSM_MODEM_ESP32)
        //        digitalWrite(M1_RST, HIGH);
        //        digitalWrite(M1_CS, HIGH);
        //#endif
        //#if defined(TINY_GSM_MODEM_SARAR4)
        //        digitalWrite(M1_RST, HIGH);
        //#endif
        //#if defined(TINY_GSM_MODEM_UBLOX)
        //        digitalWrite(M1_RST, HIGH);
        //#endif
        //
        //        if (clientSSL.connect("f8i1bdejsc.execute-api.us-east-1.amazonaws.com", 443, 10000)) {
        //          Serial.println("PUT /pruebas/mytest-003/pruebaDESDEX1.txt HTTP/1.1"); //GET /prueba07-01.txt HTTP/1.1   //GET / HTTP/1.1
        //          Serial.println("Host: f8i1bdejsc.execute-api.us-east-1.amazonaws.com");//Host: mytest-003.s3.amazonaws.com  //Host: mytest-003.s3-website-us-east-1.amazonaws.com
        //          Serial.println("Content-Type: text/plain");
        //          Serial.println("Content-Length: 22");
        //          Serial.println();
        //          Serial.println("A<file contents here>A");
        //
        //          clientSSL.println("PUT /pruebas/mytest-003/pruebaDESDEX1.txt HTTP/1.1"); //GET /prueba07-01.txt HTTP/1.1   //GET / HTTP/1.1
        //          clientSSL.println("Host: f8i1bdejsc.execute-api.us-east-1.amazonaws.com");//Host: mytest-003.s3.amazonaws.com  //Host: mytest-003.s3-website-us-east-1.amazonaws.com
        //          clientSSL.println("Content-Type: text/plain");
        //          clientSSL.println("Content-Length: 22");
        //          clientSSL.println();
        //          clientSSL.println("A<file contents here>A");
        //        }
        //        else
        //          Serial.println("error al conectar el cliente");
        //
        //
        //
        //
        //
        //        while (true) {
        //          wtd.clear();
        //          if (Serial.available()) {
        //            SerialM1.write(Serial.read());
        //          }
        //          if (SerialM1.available()) {
        //            Serial.write(SerialM1.read());
        //          }
        //        }

        break;

      case 48: //'v' LogData (2.0) WavePiston version
        //Serial.println("EL NOMBRE DEL FICHERO ES >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(logName));

        placa.RecStatus();
        placa.EnableConmutado3v();

        uint8_t err_SD;
        err_SD = 0;
        int spaceKB;
        spaceKB = 0;
        int numeroDirectorios;
        numeroDirectorios = 0;
        //f_list=NULL;
        //// char** f_list;
        //// f_list = (char**)calloc(64, sizeof(char));
        directorios = "";

        int oldestDir;
        oldestDir = 0;

        pinMode(SD_CS, OUTPUT);
        digitalWrite(SD_CS, HIGH);
        SPI.begin();
        delay(10);

        spaceKB = SD._freeSpace_(SD_CS);  //OBTIENE EL ESPACIO DISPONIBLE

        if (spaceKB == -1) {
          Serial.println(F("SD Card Error! case 'v'")); delay(1000);
          err_code += "| x4000 |";
          _error = true;
          break;
        }
        else
          Serial.println("ESPACIO LIBRE EN LA SD: " + String(spaceKB) + "KB");



        if (spaceKB < DEF_MIN_SPACE) { //SI EL ESPACIO DISPONIBLE ES POCO
          Serial.println("Queda poco espacio de almacenamiento, liberando archivos más antiguos");
          Serial.println("4.5");

          err_SD = SD._listaDataLog_(logNameDir, SD_CS, directorios, numeroDirectorios);

          if (err_SD == 1) {
            Serial.println(F("SD Card Error!")); delay(1000);
            err_code += "| x4000 |";
            _error = true;

          }
          else if (err_SD == 2) {
            Serial.println("ERROR, no existe el directorio indicado:" + String(logNameDir));
            err_code += "| x4110 |";
            _error = true;

          }
          else if (err_SD == 3) {
            Serial.println("Error abriendo el archivo para escritura: " + String(logNameDir));
            err_code += "| x4102 |";
            _error = true;
          }
          else { //SI LA LISTA SE OBTIENE CORRECTAMENTE

            oldestDir = SD._findFirstFile_(directorios); //BUSCA ENTRE LA LISTA DE DATALOGS (int) EL MAS ANTIGUO (POR DIA)

            if (oldestDir == -1) {
              Serial.println("Error al obtener el archivo más antiguo, numdir=0 (sdfat.h)");
              err_code += "| x4113 |";
              _error = true;
            }
            else { //SI SE ENCUENTRA EL ARCHIVO MAS ANTIGUO
              Serial.println("Procediendo a eliminar directorio más antiguo");
              err_SD = SD._deleteDirectory_(SD_CS, logNameDir + "/", oldestDir); //ELIMINA EL DIRECTORIO DEL DATALOG MAS ANTIGUO (DIA COMPLETO)

              if (err_SD == 1) {
                Serial.println(F("SD Card Error!")); delay(1000);
                err_code += "| x4000 |";
                _error = true;
                break;

              }
              else if (err_SD == 2) {
                Serial.println("ERROR, no existe el directorio indicado:" + String(logNameDir));
                err_code += "| x4110 |";
                _error = true;
                break;

              }
              else if (err_SD == 3) {
                Serial.println("Error abriendo el archivo para escritura: " + String(logNameDir));
                err_code += "| x4102 |";
                _error = true;
                break;
              }
            }
          }
        }
        //SI HAY ESPACIO LIBRE O SE HA LIBERADO EL ESPACIO

        Serial.println("Espacio disponible: " + String(SD._freeSpace_(SD_CS)) + " KB");
        err_SD = _SaveDataLog_(outFile, programa); //GUARDA LOS NUEVOS DATOS

        if (err_SD == 1) {
          if (programa.indexOf("O") != -1) {
            Serial.println("Contiene O"); //ELIMINAR
            insertarComando("OtSv", longitud + 1, programa);
          }
          else {
            Serial.println("Actualizando fecha sin conectar a internet");
            insertarComando("tv", longitud + 1, programa);
          }
        }

        SPI.end();
        pinMode(SD_CS, INPUT_PULLDOWN);
        placa.SetPrevStatus();

        break;

      case 50: //'h' TESTEOS TESTZONE
        ///////////////////////TESTZONE//////////////////////
        Serial.println("ENTRANDO EN TESTZONE");

        //PRUEBAS R5
#ifdef TINY_GSM_MODEM_SARAR5
        modem.enableGPS();
        while (true) {

          delay(5000);
          Serial.println("--->gps_NME: " + String(modem.getGPS_NMEA()));
          Serial.println("--->getGPS: " + String( modem.getGPS()));
        }
#endif


        ///////////I2C IO 16 channel///////////////////=]

        //        Serial.println("Alimentando placa (5V y 3.3V)");
        //        placa.EnableARDUCAM();
        //        placa.EnableConmutado3v();
        //
        //        PCF.begin(); //SDA,SCL pin, initial value
        //
        //        x = PCF.read16();
        //        Serial.print("Read ");
        //        printHex(x);
        //        delay(1000);
        //
        //        while (1) {
        //          wtd.clear();
        //          Serial.println("HLT");
        //          while (Serial.available() == 0);
        //          Serial.println("Introduzca valor para :" + String(outnum));
        //          switch (Serial.read())
        //          {
        //            case 'H': doHigh(outnum); break;
        //            case 'L': doLow(outnum); break;
        //            case 'T': doToggle(outnum); break;
        //            case '+':
        //              if (outnum < 16)
        //                outnum++;
        //              else
        //                outnum = 0;
        //              break;
        //          }
        //        }


        /////    startFTPFile();


        //////////////////PRUEBA SUBIDA ARCHIVO A S3 COMO EN POSTMAN APIREST//////////////////////////////////

        //        Serial.println("PRUEBA SUBIDA ARCHIVO A S3 COMO EN POSTMAN APIREST");
        //
        //#if defined(TINY_GSM_MODEM_ESP32)
        //        digitalWrite(M1_RST, HIGH);
        //        digitalWrite(M1_CS, HIGH);
        //#endif
        //#if defined(TINY_GSM_MODEM_SARAR4)
        //        digitalWrite(M1_RST, HIGH);
        //#endif
        //#if defined(TINY_GSM_MODEM_UBLOX)
        //        digitalWrite(M1_RST, HIGH);
        //#endif
        //
        //        if (clientSSL.connect("f8i1bdejsc.execute-api.us-east-1.amazonaws.com", 443, 10000)) {
        //          Serial.println("PUT /pruebas/mytest-003/pruebaDESDEX1.txt HTTP/1.1"); //GET /prueba07-01.txt HTTP/1.1   //GET / HTTP/1.1
        //          Serial.println("Host: f8i1bdejsc.execute-api.us-east-1.amazonaws.com");//Host: mytest-003.s3.amazonaws.com  //Host: mytest-003.s3-website-us-east-1.amazonaws.com
        //          Serial.println("Content-Type: text/plain");
        //          Serial.println("Content-Length: 22");
        //          Serial.println();
        //          Serial.println("A<file contents here>A");
        //
        //          clientSSL.println("PUT /pruebas/mytest-003/pruebaDESDEX1.txt HTTP/1.1"); //GET /prueba07-01.txt HTTP/1.1   //GET / HTTP/1.1
        //          clientSSL.println("Host: f8i1bdejsc.execute-api.us-east-1.amazonaws.com");//Host: mytest-003.s3.amazonaws.com  //Host: mytest-003.s3-website-us-east-1.amazonaws.com
        //          clientSSL.println("Content-Type: text/plain");
        //          clientSSL.println("Content-Length: 22");
        //          clientSSL.println();
        //          clientSSL.println("A<file contents here>A");
        //        }
        //        else
        //          Serial.println("error al conectar el cliente");
        //
        //
        //
        //
        //
        //        while (true) {
        //          wtd.clear();
        //          if (Serial.available()) {
        //            SerialM1.write(Serial.read());
        //          }
        //          if (SerialM1.available()) {
        //            Serial.write(SerialM1.read());
        //          }
        //        }






        /////////////////////////////////////////////////////////////////ESP32-DWLD-HTTP
        //#if defined(TINY_GSM_MODEM_ESP32)
        //        buff_ = "";
        //        tam_paq = 0;
        //
        //        //         while (true) {
        //        //                  if (Serial.available()) {
        //        //                    SerialM1.write(Serial.read());
        //        //                  }
        //        //                  if (SerialM1.available()) {
        //        //                    Serial.write(SerialM1.read());
        //        //                  }
        //        //                }
        //
        //        Serial.println("Testing Board");
        //        SerialM1.println("ATE1");
        //        Serial.println(SerialM1.readString());
        //        delay(10);
        //
        //        SerialM1.println("AT+CIPSTART=1,\"TCP\",\"mytest-003.s3.amazonaws.com\",80,240"); //mytest-003.s3.amazonaws.com  //mytest-003.s3-website-us-east-1.amazonaws.com
        //        Serial.println(SerialM1.readString());
        //        delay(10);
        //
        //        SerialM1.println("at+cipsend=1,92"); //92 con keep //68 sin keep
        //        delay(5);
        //        while (SerialM1.available()) {
        //          Serial.println(SerialM1.readString());
        //        }
        //        delay(100);
        //
        //        Serial.println("GET /prueba12-01.txt HTTP/1.1");  //GET /prueba07-01.txt HTTP/1.1   //GET / HTTP/1.1
        //        Serial.println("Host: mytest-003.s3.amazonaws.com");//Host: mytest-003.s3.amazonaws.com  //Host: mytest-003.s3-website-us-east-1.amazonaws.com
        //        Serial.println("Connection: keep-alive");
        //        Serial.println();
        //        SerialM1.println("GET /prueba12-01.txt HTTP/1.1");  //GET /prueba07-01.txt HTTP/1.1   //GET / HTTP/1.1
        //        SerialM1.println("Host: mytest-003.s3.amazonaws.com");//Host: mytest-003.s3.amazonaws.com  //Host: mytest-003.s3-website-us-east-1.amazonaws.com
        //        SerialM1.println("Connection: keep-alive");
        //        SerialM1.println();
        //
        //
        //        while (true) {
        //          while (SerialM1.available()){
        //            Serial.write(SerialM1.read());
        //          }
        //        }
        //
        //
        //        while (SerialM1.available()) {
        //          Serial.println(SerialM1.readString());//POR QUE NO SE VE EL [Recv 92 bytes] Y EL [SEND OK]?
        //        }
        //
        //        Serial.println(">CABECERA:");
        //        contentLength = 0;
        //        // while(!SerialM1.available());
        //
        //        //LECTURA DE CABECERA   -> NO ESTA ENTRANDO
        //        while (true) {
        //          if (SerialM1.available()) {
        //            Serial.print('>');
        //            String line = SerialM1.readStringUntil('\n');
        //            line.trim();
        //            line.toLowerCase();
        //            Serial.println(line);
        //            if (line.startsWith("content-length:")) {
        //              contentLength = line.substring(line.lastIndexOf(':') + 1).toInt();
        //              Serial.println(contentLength);
        //            }
        //            else if (line.length() == 0) {
        //              Serial.println('<');
        //              break;
        //            }
        //          }
        //        }
        //
        //        //LECTURA DE DATOS
        //        while (true) {
        //          if (SerialM1.available() == 63) {
        //            Serial.println("Overflow");
        //            while (true);
        //          }
        //          if (SerialM1.available()) {
        //            //            Serial.println(SerialM1.available());
        //
        //            //char c =  SerialM1.read();
        //            //Serial.write(c);
        //            // buff_ += (char) c;
        //
        //            buff_ += (char)SerialM1.read();
        //            if (buff_.endsWith("CLOSE"))break;
        //            //if (buff_.endsWith("+IPD,")) {
        //            if (buff_.endsWith("IPD,")) {
        //              //Serial.println();
        //              //Serial.print("PAQUETE:");
        //              int mux = SerialM1.readStringUntil(',').toInt();
        //              int len = SerialM1.readStringUntil(':').toInt();
        //              len = len + 2;
        //              tam_paq += len;
        //              //Serial.println("  tamaño: " + String(len));
        //
        //              //while (len--) {
        //              while (len) {
        //                if (len % 5 == 0)
        //                  ind_ = 5;
        //                else if (len % 4 == 0)
        //                  ind_ = 4;
        //                else if (len % 3 == 0)
        //                  ind_ = 3;
        //                else if (len % 2 == 0)
        //                  ind_ = 2;
        //                else
        //                  ind_ = 1;
        //
        //                ///while (!SerialM1.available());
        //                for (int i = 0; i < ind_; i++) {
        //                  while (!SerialM1.available());
        //                  c_2[i] = SerialM1.read();
        //                  len--;
        //                }
        //                for (int i = 0; i < ind_; i++) {
        //                  Serial.write(c_2[i]);
        //                }
        //                ///c_c = SerialM1.read();
        //                ///Serial.print(c_c);
        //              }
        //
        //
        //              //              while (c_c != '+') {
        //              //                Serial.write(c_c);
        //              //                while (!SerialM1.available());
        //              //                c_c = SerialM1.read();
        //              //              }
        //
        //              buff_ = "";
        //              //c_c = ' ';
        //
        //              Serial.println("__________________________________________________________________________________________");
        //              Serial.println("Descargados: " + String(tam_paq) + "  |  De: " + contentLength);
        //            }
        //
        //            if (buff_.length() > 45 && String(buff_.indexOf('+')) == "-1")
        //              buff_ = "";
        //          }
        //        }
        //
        //
        //        Serial.println();
        //        Serial.println("Fin del fichero, tamaño total: " + String(tam_paq));
        //#endif

        /////////////////////////ECOCOMAND//////////////////////

        initComPort();
        placa.EnableMIKRO();

#if defined(TINY_GSM_MODEM_ESP32)
        digitalWrite(M1_RST, HIGH);
        digitalWrite(M1_CS, HIGH);
#endif
#if defined(TINY_GSM_MODEM_SARAR4)
        digitalWrite(M1_RST, HIGH);
#endif
#if defined(TINY_GSM_MODEM_UBLOX)
        digitalWrite(M1_RST, HIGH);
#endif

        Serial.println("Testing Board");
        while (true) {
          wtd.clear();
          if (Serial.available()) {
            SerialM1.write(Serial.read());
          }
          if (SerialM1.available()) {
            Serial.write(SerialM1.read());
          }
        }

        ///////////////////////////////////////////////////////


        //        Serial.println("Comprobando archivos de actualización en SD...");
        //        String message;
        //        message += "Sketch compile date and time: ";
        //        message += __DATE__;
        //        message += " ";
        //        message += __TIME__;
        //
        //        // print out the sketch compile date and time on the serial port
        //        Serial.println(message);




        /*       RELACIONADO CON LISTAS

                ////////////////////////////////////////////////////////////////
                ////////////////////////////////////////////////////////////////

                placa.EnableMIKRO();
                delay(6000);
                modem.mqttConnect("mqtt.astican.muutech.com", 12040, "SSM-test1", "astican", "eAnfpyPLYGfaPhkA5TAb", localConfig.APN.c_str());
                break;
                Serial.println("CASO 50 TESTEOS");
                delay(3000);
                //////////CREA LISTA////////////
                //bool f_nodo=true;
                nuevo_nodo( "ADC", 1, "S1", "test1", "Topic1");
                nuevo_nodo( "ADC", 2, "S2", "test2", "Topic");
                nuevo_nodo( "ADC", 3, "S3", "test1", "Nan");
                nuevo_nodo( "ADC", 4, "S4", "test2", "Nan");
                nuevo_nodo( "ADC", 5, "S5", "test1", "Nan");
                nuevo_nodo( "ADC", 6, "S6", "test2", "Nan");
                nuevo_nodo( "ADC", 7, "S7", "test1", "Nan");
                nuevo_nodo( "ADC", 8, "S8", "test2", "Nan");

                Serial.println("marca1");
                printlist();

                while (Serial.available()) {
                  Serial.read();
                }
                Serial.println("Numero de nodos: " + String(num_nodos()));
                while (Serial.available()) {
                  Serial.read();
                }
                topics_nodos();
                while (true);
        */
        break;


      case 60: //LP
        placa.RecStatus();
        wtd.clear();
        wtd.setup(WDT_OFF);

        if ((day(current_timestamp) == ((localTime.substring(8, 10)).toInt()) && month(current_timestamp) == ((localTime.substring(5, 7)).toInt())) && ((year(current_timestamp) == (2000 + ((localTime.substring(2, 4)).toInt()))) || year(current_timestamp) == ((localTime.substring(2, 4)).toInt())))
        {
          Serial.println("------------------------------Entrando en modo LowPower------------------------------");

          //CONFIGURACION
          for (int i = 0; i <= 42; i++) {
            if ((i != 22) && (i != 28) && (i != 29) && (i != 38))
              pinMode(i, INPUT_PULLDOWN);
          }
          SerialM1.end();
          pinMode(M1_RX, INPUT_PULLDOWN);
          pinMode(M1_TX, INPUT_PULLDOWN);
          //i2c
          pinMode(20, INPUT); //SDA
          pinMode(21, INPUT); //SCL
          //Shift
          pinMode(0, OUTPUT);
          pinMode(23, OUTPUT);
          pinMode(24, OUTPUT);
          digitalWrite(0, HIGH);
          digitalWrite(23, HIGH);
          digitalWrite(24, HIGH);
          placa.Initialize();
          //LED
          pinMode(25, INPUT_PULLUP);
          //5V

          pinMode(27, OUTPUT);
          digitalWrite(27, LOW);

          //printBatteryStats(BatteryStatus,BatteryVolt,BatteryAmp);

          Wire.begin();
          ina_input.begin();

          ina_input.setpowerdown();

          //setupBQ27441();


          if (f_FTsleep == true)
          {
            _h = localConfig.sleepTime / 3600;
            _m = ((localConfig.sleepTime - (_h * 3600)) / 60);
            _s = (localConfig.sleepTime - (_h * 3600 + _m * 60));

            current_timestamp = rtc.getEpoch();
            al_hour = hour(current_timestamp) + _h;
            al_minute = minute(current_timestamp) + _m;
            al_second = second(current_timestamp) + _s;
            f_FTsleep = false;
          }

          CtrlAlarm();

          if (((sum_al - sum_act <= 0) && (abs(sum_al - sum_act) < 72000)) || ((sum_al - sum_act >= 0) && (abs(sum_al - sum_act) > localConfig.sleepTime)))
          {
            Serial.println("ATENCION!!! LA ALARMA ES ANTERIOR A LA HORA ACTUAL, actualizando hora de alarma...");

            int aux_mult = 0;

            aux_mult = ((abs(sum_act - sum_al) / localConfig.sleepTime) + 1); //obtenemos el número de veces que hay que añadirle el sleeptime para llegar a la nueva alarma
            sum_al = (sum_al + (localConfig.sleepTime * aux_mult));  //calculamos el tiempo de la nueva alarma en segundos

            al_hour = sum_al / 3600;
            al_minute = ((sum_al - (al_hour * 3600)) / 60);
            al_second = (sum_al - (al_hour * 3600 + al_minute * 60));

            CtrlAlarm();
          }

          rtc.disableAlarm();
          rtc.enableAlarm(rtc.MATCH_MMSS);
          rtc.setAlarmTime(al_hour, al_minute, al_second);
          rtc.attachInterrupt(a_goToLowPower);
          al_hour = al_hour + _h;
          al_minute = al_minute + _m;
          al_second = al_second + _s;

          placa.DisableANA();
          placa.DisableMIKRO();

          Wire.end();

          //100u y 170u
          placa.EnableI2C();

          Serial.end();
          USBDevice.detach();

          // Set sleep mode to deep sleep
          SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
          //        //  //Disable USB port (to disconnect correctly from host
          //        //  //USB->DEVICE.CTRLA.reg &= ~USB_CTRLA_ENABLE;
          //Enter sleep mode and wait for interrupt (WFI)
          __DSB();
          __WFI();

          //--------------------------------------------------------
          //        cont = false;
          //
          //        while ( cont == false){
          //          Serial.print(".");
          //          delay(1000);
          //        }

          USBDevice.init();
          delay(10);
          USBDevice.attach();
          Serial.begin(USB_BAUD);

          pinMode(M1_RX, OUTPUT);
          pinMode(M1_TX, INPUT);
          SerialM1.begin(M1_BAUD);
          Serial.println();
          delay(10);
          Serial.println("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^DESPERTANDO^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");

          //RESTABLECIMIENTO DE CONFIGURACIÓN
          placa.SetPrevStatus();
          rtc.disableAlarm();
          wtd.setup(WDT_TIME);
          wtd.attachShutdown(ISR_WDT);

          ina_input.begin();  //analisis de consumo de la placa
          ina_input.setpowerdown();

          placa.EnableConmutado3v();

          for (int i = 0; i < 2 ; i++)
          {
            pinMode(DIGI0, OUTPUT);
            digitalWrite(DIGI0, LOW);
            delay(50);
            digitalWrite(DIGI0, HIGH);
            delay(200);
          }

          placa.EnableD1();
          placa.EnableTEMP(); //REVISAR PIN
          init_IO ();
          initComPort();

        } else
        {
          Serial.println("Error en la fecha desconocido, reiniciando");
          contFallos++;
          err_code += "| x3X03 |";
          _error = true;
          insertarComando("vZ", longitud + 1, programa);
        }

        break;

    }
    if (!iddle)longitud++;
  }
}



int selectState(char c) {
  switch (c) {
    case 'A': return 1;
    case 'B': return 2;
    case 'C': return 3;
    case 'D': return 4;
    case 'E': return 5;
    case 'F': return 6;
    case 'G': return 7;
    case 'H': return 8;
    case 'I': return 9;
    case 'J': return 10;
    case 'K': return 11;
    case 'L': return 12;
    case 'M': return 13;
    case 'N': return 14;
    case 'O': return 15;
    case 'P': return 16;
    case 'Q': return 17;
    case 'R': return 18;
    case 'S': return 19;
    case 'T': return 20;
    case 'U': return 21;
    case 'V': return 22;
    case 'W': return 23;
    case 'X': return 24;
    case 'Y': return 25;
    case 'Z': return 26;
    case 'a': return 27;
    case 'b': return 28;
    case 'c': return 29;
    case 'd': return 30;
    case 'e': return 31;
    case 'f': return 32;
    case 'g': return 33;
    case 'r': return 44;
    case 's': return 45;
    case 't': return 46;
    case 'u': return 47;
    case 'v': return 48;
    case 'h': return 50;  //TESTEOS
    case '-': return 60;  //LP
    default : return 0;
  }
}

//int takePositionGPS(String _myGGA,String &_time,String &_lat,String &_lon,String &_fixed){
int takePositionGPS(String _myGGA, String & _lat, String & _lon, String & _fixed) {
  int idx1, idx2 = 0;
  String _temp;
  //Serial.println(_myGGA);
  for (int i = 0 ; i < 7; i++) {
    idx1 = _myGGA.indexOf(',', idx2 + 1);
    _temp = _myGGA.substring(idx2 + 1, idx1);
    idx2 = idx1;
    //Serial.print(temp2);Serial.print('\t');
    //if ( i == 1 ) _time = _temp;
    if ( i == 2 ) _lat = _temp;
    if ( i == 3 ) _lat += " " + _temp;
    if ( i == 4 ) _lon = _temp;
    if ( i == 5 ) _lon += " " + _temp;
    if ( i == 6 ) _fixed =  _temp;

  }
  return _fixed.toInt();
}

void endSPI_Bus() {
  SPI.end();
  //  pinMode(SD_CS,INPUT_PULLDOWN);
  //  pinMode(CAM_SS,INPUT_PULLDOWN);

}

//LOWPOWER
void a_goToLowPower()
{
  //digitalWrite(DIGI0,LOW);
  //cont = true;
  //----------------------------------------
}

void CtrlAlarm()
{
  int aux = 0;
  if (al_second > 59)
  {
    aux = al_second / 60;
    al_minute = al_minute + aux;
    al_second = (al_second - (aux * 60));
    aux = 0;
  }
  if (al_minute > 59)
  {
    aux = al_minute / 60;
    al_hour = al_hour + aux;
    al_minute = al_minute - aux * 60;
    aux = 0;
  }
  if (al_hour > 23) //ver si el formato esta en 12 o en 24
  {
    al_hour = al_hour - 24;

    if (al_hour != 0)
      Serial.println("Asegurese de que el programa está configurado con MATCH_HHMMSS");
  }

  current_timestamp = rtc.getEpoch();
  Serial.println("////////////////////////////////////////////////////////////////////////////////////");
  Serial.println("SleepTime: " + String(_h) + ":" + String(_m) + ":" + String(_s));
  Serial.println("Hora actual: " + String(hour(current_timestamp)) + ":" + String(minute(current_timestamp)) + ":" + String(second(current_timestamp)));
  Serial.println("Despertará a las: " + String(al_hour) + ":" + String(al_minute) + ":" + String(al_second));

  //Control de hora Alarma-Actual
  //pasamos los tiempos a segundos
  sum_al = ((al_hour * 3600) + (al_minute * 60) + al_second);
  sum_act = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));

  Serial.println("Tiempo que estará dormido: " + String(abs(sum_al - sum_act)) + " s.");
  Serial.println("SUM_AL: " + String(sum_al) + "     | SUM_ACT: " + String(sum_act) + "   | AL-ACT=" + String(sum_al - sum_act) + "   | abs=" + String(abs(sum_al - sum_act)));

}

void ISR_WDT() {

}
