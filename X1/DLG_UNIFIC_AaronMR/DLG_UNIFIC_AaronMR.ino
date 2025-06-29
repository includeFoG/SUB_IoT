
#define DEBUGCODE

#define SSM_DLG

#include <Arduino.h>

#include <esp_task_wdt.h> //#include <WDTZero.h>
#include <Wire.h>
#include <SPI.h>

#include <ESP32Time.h>

ESP32Time rtc(0); //offset in seconds GMT SE LE HA QUITADO

#define SerialM1 Serial1
#define SerialMux Serial2


#include "DDM_DLG.h"
#include "gVariables.h"
#include "config.h"
#include "gestionTiempo.h"


#include "config_DLG.h"
#include "myRS485.h"
#include "RS485astican.h"

#include "Communicaciones.h"
#include "FilesFormat.h"


bool cont = false;

int state = 0;

bool iddle = false;
unsigned long lastTime;
String ecoCommand = "";




#define EN_VO     16//


void setup() {
  init_IO ();
  initComPort();

  Serial.begin(USB_BAUD);
  SerialM1.begin(M1_BAUD, SERIAL_8N1, M1_RX, M1_TX);   //Mikro 1
  SerialMux.begin(MUX_BAUD, SERIAL_8N1, URX0, UTX0); //Mikro 2

  pinMode(EN_VO, OUTPUT);
  digitalWrite(EN_VO, HIGH);

  delay(1000);

  //CALLBACKSMODBUS
  initCallbacks(&node3);

  esp_task_wdt_deinit();
  esp_task_wdt_init(&configWDT);
  esp_task_wdt_add(NULL);

  init_v_JSON();
}




void loop() {
  Serial.println("************************loop************************");
  String programa = "";

  if (! _configLoaded ) {
    Serial.println("aki dentro");
#if defined(NO_DISCONNECT)
//    serial.println("aki_1");
    programa = "XOt";
#else
//    Serial.println("aki_2");
    // programa = "Hq";//"XOtS";               //este programa
    programa = "XOtS"; //"XOtHqS"; //
    /*
      'X' ReadConfig
      'O' conecta modem
      't'TakeTime
      'H' lee los datos del RS485
      'q' genera el Json
      'S' conexion close
    */
#endif
    _configLoaded = true;
  } else {
    programa = localConfig.programa;
    //Serial.println("CONFIGURADO");
    if (contFallos > NUM_FALLOS_MAX) {
      programa = "Z";  //reinicio
    }
  }
  Serial.print("programa = ");
  Serial.println(programa);
  String cmd = "";
  unsigned int longitud = 0;
  esp_task_wdt_reset();
#ifdef DEBUGCODE
  if (Serial.available()) {
    programa = Serial.readString();
  }
#endif

  Serial.print("longitud = "); Serial.println(longitud);
  Serial.print("programa.length() = "); Serial.println(programa.length());

  while (longitud < programa.length()) {
//    Serial.println("aki_3");
    esp_task_wdt_reset();

    if (Serial.available()) {
      String tempSerial = Serial.readString();
      cmd = tempSerial[0];
    } else {
      Serial.print("longitud = "); Serial.println(longitud);
      cmd = programa[longitud];
    }
#ifdef DEBUGCODE
    Serial.print("comando: "); Serial.println(cmd);
#endif

    state = selectState(cmd[0]);
    int ana_result = 0;

    //    String output; // salida del json calculado

    Serial.print("state = "); Serial.println(state);
    Serial.print("longitud = "); Serial.println(longitud);
    Serial.print("programa = "); Serial.println(programa);

    //    ejecutaCase(state, longitud, programa);

    Serial.println("ejecutaCase");
    //************************* Estructura switch case *************************
    //  unsigned int longitud = 0;

    switch (state) {

      case 0:
        break;

      /*********************************************************************************************************************
                                                     [8] 'H' Lee Meteo  RK900 y RK300
      *********************************************************************************************************************/
      case 8: //'H' Lee Meteo RK900 y RK300
        Serial.println("caso 8 o H, leemos Meteo");
        //HABILITA RS485
        digitalWrite(MUX_SEL_0, HIGH);
        digitalWrite(MUX_SEL_1, HIGH);
        delay(500);

        SerialMux.begin(MUX_BAUD, SERIAL_8N1, URX0, UTX0); //Mikro 2

        uint8_t contador_1;

        for (int j = 4; j < NUMITEMS(s_ValJSON); j++) {
          s_ValJSON[j] = "Nan";
        }
        Serial.println();
        procesaJson(69);
        //        err_RS = GET_METEO_RK900(2);
        //        err_RS = GET_METEO_RK300(1);

        f_topic_Analiz = true;
        f_topic_Analog = false;

        if  (err_RS == "1") {
          err_analiz = String((0x2106 + (n_Analiz, HEX)), HEX);
        }

        //        Gest_Alarm_Comms();

        if (err_RS != "0")
        {
          contador_1++;
          if (contador_1 < MAX_RETRAY) {
            insertarComando("I", longitud + 1, programa);
          }
        }
        else
        {
          contador_1 = 0;
        }
        SerialMux.end();

        break;


      /*********************************************************************************************************************
                                                     [9] 'I' leeAnalizSch
      *********************************************************************************************************************/
      case 9: //'I' leeAnalizSch
        Serial.println("case 9");
        //HABILITA RS485
        digitalWrite(MUX_SEL_0, HIGH);
        digitalWrite(MUX_SEL_1, HIGH);
        delay(500);

        SerialMux.begin(MUX_BAUD, SERIAL_8N1, URX0, UTX0); //Mikro 2

        uint8_t contador;

        err_analiz = "0"; //codigo error analizador

        for (int j = 4; j < NUMITEMS(s_ValJSON); j++) {
          s_ValJSON[j] = "Nan";
        }
        Serial.println();
        //        Serial.println(" Iniciando proceso de lectura Schneider PM2200, para analizador: " + String(n_Analiz));

        //        err_RS = GET_METER_SYNCRO(n_Analiz);
        err_RS = GET_METEO_RK900(2);
        err_RS = GET_METEO_RK300(1);

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
        }
        else
        {
          contador = 0;
        }
        SerialMux.end();

        break;

      /*********************************************************************************************************************
                                                          [12] 'O' TemperaturaPCB
           *********************************************************************************************************************/
      case 12: //'L' conecta modem
        s_ValJSON[takeParam("xh")] = localConfig.sensorID;
      /*********************************************************************************************************************
                                                     [15] 'O' conecta modem
      *********************************************************************************************************************/
      case 15: //'O' conecta modem
        //Serial.println("EL NOMBRE DEL FICHERO ES >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(logName));
        Serial.println();
        Serial.println("/////////////////////////////////////////////////////////////////////////////////");
        Serial.println("             Estado del contador de fallos:  " + String(contFallos) + "  | de: " + String(NUM_FALLOS_MAX));
        Serial.println("/////////////////////////////////////////////////////////////////////////////////");
        Serial.println();

        esp_task_wdt_reset();

        //current_timestamp = rtc.getEpoch();
        //sum_SecAct = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));
        //if (sum_SecAct - SendTime >= _sendTime) {
        if (a_sendTime)
        {
          EnableMIKRO();
          if (startComPort() != 0) {
            retry_connect++;
            if (retry_connect == MAX_RETRAY) {
              Serial.println("Número máximo de intentos de conexión alcanzado");
              insertarComando("S", longitud + 1, programa);
              contFallos++;
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
          DisableMIKRO();
        }
        break;

      /*********************************************************************************************************************
                                                         [16] 'q' envio RS485  Esto es envio del MQTT???  o la creacion el mensaje??
          *********************************************************************************************************************/
      case 16:  {//'q' envio RS485
          // ahora mismo estoy usando la variable doc que es el Json deserializado, es global, creo que es un error, pero no se como solucionarlo.....
          Serial.println("case 16");

          String mensajeEnviar = buildGenJSON_2();  // crear el mensaje a enviar , el json.

          Serial.print(" mensajeEnviar = "); Serial.println(mensajeEnviar);
          err_com = envioDatosRS485Muutech_2(mensajeEnviar);  // se crea el mensaje?

          break;
        }
      /*********************************************************************************************************************
                                                     [17] 'Q' envio RS485  Esto es envio del MQTT???  o la creacion el mensaje??
      *********************************************************************************************************************/
      case 17:  //'Q' envio RS485
        //Serial.println("EL NOMBRE DEL FICHERO ES >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(logName));
        Serial.println(" estoy en case Q,  case 17");
        if (retry_connect >= MAX_RETRAY)
        {
          break;
        }

        Serial.println("aki1");
        err_com = envioDatosRS485Muutech(buildGenJSON());  // se crea el mensaje?
        Serial.println("aki2");
        Serial.println("El err_com de envio RS es:" + String(err_com));
        if (err_com == 33)
        {
          Serial.println("ERROR DE COMUNICACIONES (no se ha modificado el parámetro de retorno=33)");
          contFallos++;
          retry_send++;
        }
        else if (err_com != 0 && err_com != 99) {
          retry_send++;
          if (retry_send == MAX_RETRAY) {
            insertarComando("S", longitud + 1, programa);
            contFallos++;
          }
          else {
            if ( retry_send > MAX_RETRAY )retry_send = 1;
            if ( err_com == 1) { //ANTES 2
              Serial.println("ERROR! El dispositivo no se encontraba conectado cuando se intentó hacer el envío(No hay conexión con la red)");
              insertarComando("SOQ", longitud + 1, programa);
            }
            if ( err_com == 2) {
              Serial.println("ERROR DE COMUNICACIONES (error al conectar con el servidor web HTTP)");
              insertarComando("SOQ", longitud + 1, programa);
            }
            if ( err_com == 3) { //ANTES 3
              Serial.println("ERROR DE COMUNICACIONES (error al enviar mensaje HTTP)");
              insertarComando("SOQ", longitud + 1, programa);
            }
            if ( err_com == 4) { //ANTES 3
              Serial.println("ERROR DE COMUNICACIONES (error al conectar con el servidor web HTTPS)");
              insertarComando("SOQ", longitud + 1, programa);
            }
            if ( err_com == 5) { //ANTES 3
              Serial.println("ERROR DE COMUNICACIONES (error al enviar mensaje HTTPS)");
              insertarComando("SOQ", longitud + 1, programa);
            }
            if ( err_com == 10) { //ANTES 1
              Serial.println("ERROR DE COMUNICACIONES (error al conectar con broker MQTT)");
              insertarComando("fQ", longitud + 1, programa);
            }
            if ( err_com == 11) { //ANTES 1
              Serial.println("ERROR DE COMUNICACIONES (error al enviar el mensaje MQTT)");
              insertarComando("fQ", longitud + 1, programa);
            }
          }
        } else retry_send = 0;
        if (err_com == 99) //ANTES 4
          Serial.println("Envío suspendido");
        else
          Serial.print("Retry send RS: "); Serial.println(retry_send);

        if (err_com == 0) //SI SE HA ENVIADO EL DATO//CLEAR CONTINUOUS REGISTER RAIN ACUMULADO
        {
          WMaxPrev = 0;
        }
        break;


      /*********************************************************************************************************************
                                                     [19] 'S' conexion close
      *********************************************************************************************************************/

      case 19://'S'
        Serial.println(" 'S' conexion closed ");
        // Serial.println("EL NOMBRE DEL FICHERO ES >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>: " + String(logName));

        //   modem.gprsDisconnect();
        //   delay(100);
        //   modem.poweroff();
        //  delay(100);
        DisableMIKRO();
        delay(3000); //antes 3000
        Serial.println("");
        Serial.println("MIKROE desconectado");
        break;


      /*********************************************************************************************************************
                                                     [21] 'U' Wait(iddle)
      *********************************************************************************************************************/
      case 21: //'U'   Wait(iddle)

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

      /*********************************************************************************************************************
                                                     [23]  'W' Printmenu
      *********************************************************************************************************************/
      case 23: //'W' Printmenu
#ifdef DEBUGCODE
        Serial.println(commandMenu);
#endif
        break;
      /*********************************************************************************************************************
                                                     [24]  'X' ReadConfig
      *********************************************************************************************************************/
      case 24:// 'X' ReadConfig
        Serial.println(" X readconfig");
//        Serial.println("aki_11");
        loadDefualt(&localConfig, &localAnaMap, &localConfigMQTT);
//        Serial.println("aki_12");
        updatefreq(localConfig.measureFreq, localConfig.waitTime);
//        Serial.println("aki_13");
        printConig(&localConfig, &localAnaMap, &localConfigMQTT);
//        Serial.println("aki_14");

        break;
      /*********************************************************************************************************************
                                                      [25] 'Z' Reset
      *********************************************************************************************************************/
      case 25:// 'Z' Reset
        ESP.restart();
        break;
      /*********************************************************************************************************************
                                              [32] 'F' MQTT DISC
      *********************************************************************************************************************/
      case 32: //'f' MQTT Discconect

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
        break;
      /*********************************************************************************************************************
                                                      [44]  'r' Reset sendTime
       *********************************************************************************************************************/
      case 44: // 'r' Reset sendTime
        SendTime = -214783647;
        a_sendTime = true;
        Serial.println("Tiempo de envío reseteado");
        break;


      /*********************************************************************************************************************
                                                      [46]  't'TakeTime
       *********************************************************************************************************************/

      case 46: // 't'
        Serial.println("///////VERIFICANDO FECHA/////////");

        current_timestamp = rtc.getEpoch();
        Serial.println(day(current_timestamp));
        Serial.println((localTime.substring(8, 10)).toInt());
        Serial.println(month(current_timestamp));
        Serial.println((localTime.substring(5, 7)).toInt());
        Serial.println(year(current_timestamp));
        Serial.println((2000 + ((localTime.substring(2, 4)).toInt())));

        if ((!(year(current_timestamp) > 2021)) && (day(current_timestamp) == ((localTime.substring(8, 10)).toInt()) && month(current_timestamp) == ((localTime.substring(5, 7)).toInt()) && year(current_timestamp) == (2000 + ((localTime.substring(2, 4)).toInt()))))
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

          localTime = TakeTime();
          if (localTime == "NoInit_2")
            TakeTime();
          Serial.print("TimeStamp:");
          Serial.println(localTime);

          if (localTime == "NoInit_0" || localTime == "NoInit_1" || localTime == "NoInit_2") {
            Serial.println("retryconnect2:" + String(retry_connect2));
            retry_connect2++;
            contFallos++;
            if ( retry_connect2 > MAX_RETRAY) Serial.println("Fallo en MIKROE");
            else Serial.println("Fallo en la comunicacion, retry connect numero: " + String(retry_connect2) + " DE MAX_RETRAY: " + String(MAX_RETRAY));
            if (retry_connect2 >= MAX_RETRAY)
            {
              contFallos++;
              Serial.println("Número máximo de reintentos alcanzado");

              insertarComando("S", longitud + 1, programa);
              retry_connect2 = 1;
              //METER AQUI ERROR POR NO ACTUALIZACION DE TIEMPO
              Serial.println("No ha sido posible actualizar el tiempo, estableciendo tiempo 2021/01/01 0:0:0");

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
          }
        }
        break;
    }

    Serial.println("aki detras");
    if (!iddle)
    {
      Serial.print("iddle = "); Serial.println(iddle);
      longitud++;
    }
  }
}


int ejecutaCase(int state, unsigned int longitud, String &programa)
{

}
//*************************



int selectState(char c) {
  switch (c) {
    case 'H': return 8;
    case 'I': return 9;
    case 'L': return 12;
    case 'O': return 15;
    case 'q': return 16;   // esto es un clonado de Q, para intentar hacerlo a mi manera,  AaronMR
    case 'Q': return 17;
    case 'S': return 19;
    case 'T': return 20;
    case 'U': return 21;
    case 'W': return 23;
    case 'X': return 24;
    case 'Z': return 26;
    //case 'a': return 27;
    //case 'd': return 30;
    case 'f': return 32;
    case 'r': return 44;
    case 't': return 46;
    //case '-': return 60;  //LP
    default : return 0;
  }
}




void ISR_WDT() {

}
