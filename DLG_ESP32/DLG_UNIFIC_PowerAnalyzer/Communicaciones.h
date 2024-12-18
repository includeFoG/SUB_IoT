#define TINY_GSM_MODEM_BC95G

#define MAX_RETRAY        1


#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <TimeLib.h>

#include <ArduinoJson.h>



char* ArriveTopicMQTT;
byte* payloadArrive;
unsigned int len;

void mqttCallback(char* ArriveTopicMQTT, byte* payloadArrive, unsigned int len)
{
  Serial.print("Message MQTT arrived [");
  Serial.print(ArriveTopicMQTT);
  Serial.print("]: ");
  Serial.write(payloadArrive, len);
  Serial.println();
}


int retry_connect = 0;
int retry_connect2 = 0;
int retry_send = 0;

int err_com = 0;
int retorno = 33;

int sigQ = 0 ;

int f_cont = 0;

TinyGsm modem(SerialM1);

TinyGsmClientSecure clientSSL(modem, M1_BAUD);
TinyGsmClient client(modem);

PubSubClient mqtt(client); //mqtt(DIR_BROKER_MQTT,MQTT_PORT,mqttCallback,client);



bool f_topic_Analiz = false;
bool f_topic_Analog = false;
String finalTopic = "";




void ARDUINO_ISR_ATTR timeSend() {
  a_sendTime = true;
  //Serial.println("ALARMA DE TIEMPO DE ENVIO, estado cambiado a: " + String(a_sendTime));
}

hw_timer_t *alarmTimer = NULL;

void createSendAlarm() {  //configuración de alarma de RTC que avisa de cuándo se cumple el tiempo entre envíos. fija una alrma en el tiempo actual
  if (alarmTimer) {
    // Stop and free timer
    timerEnd(alarmTimer);
    alarmTimer = NULL;
  }
    
  uint8_t wait_hora = localConfig.waitTime / 3600;
  uint8_t wait_min  =  ((localConfig.waitTime - (wait_hora * 3600)) / 60);
  uint8_t wait_seg = (localConfig.waitTime - (wait_hora * 3600 + wait_min * 60));

  current_timestamp = rtc.getEpoch();

  uint8_t a_seg = second(current_timestamp) + wait_seg;
  uint8_t a_min = minute(current_timestamp) + wait_min;
  uint8_t a_hora =   hour(current_timestamp) + wait_hora;
  uint8_t aux = 0;

  if (a_seg > 59) {
    aux = a_seg;
    a_seg = a_seg - (a_seg / 60) * 60;
    a_min = a_min + (aux / 60);
  }
  if (a_min > 59) {
    aux = a_min ;
    a_min = a_min - (a_min / 60) * 60;
    a_hora = a_hora + (aux / 60);
  }
  if (a_hora > 23) {
    a_hora = a_hora - (a_hora / 23) * 24;
  }
  /*
    rtc.setAlarmTime(a_hora, a_min, a_seg);
    rtc.enableAlarm(rtc.MATCH_HHMMSS);
    rtc.attachInterrupt(timeSend);*/
  long alarmMicroseconds = localConfig.waitTime * 1000000;
  alarmTimer = timerBegin(1000000);  //seteamos frecuencia del timer 1Mhz
  timerAttachInterrupt(alarmTimer, &timeSend);  // Adjuntar la ISR
  timerAlarm(alarmTimer, alarmMicroseconds, false, 0); // Configura la alarma para el tiempo de espera microsegundos

  Serial.println();
  Serial.println("estableciendo tiempo de alarma (microsegundos): "+ String(alarmMicroseconds));
  Serial.print("Alarma de tiempo de envío fijada a las: " + String(a_hora) + ":" + String(a_min) + ":" + String(a_seg));
  Serial.print("    Hora actual:" + String(hour(current_timestamp)) + ":" + String(minute(current_timestamp)) + ":" + String(second(current_timestamp)));
  Serial.println();
}



String DefTopic(String Topic1, String Topic2, String Topic3, String Topic4)
{
  //String TOPICS =  String(Topic1) + "/" + String(Topic2) + "/" + String(Topic3) + "/" + String(Topic4);
  String TOPICS;

#ifdef JSON_SIEMENSPAC
  TOPICS =  String(Topic1) + "/" + String(Topic2) + "/" + String(Topic3) + "/CT/" + String("Analiz") + String(n_Analiz + 1);
  //#endif

#elif defined(JSON_RED) && defined(JSON_ANALOG005)

  if (f_topic_Analiz == true) {
    Serial.println("PREPARANDO TOPIC DE ANALIZADORES");
    String nombretopic = "/ARS/EXTERIOR/ANALIZADORESRED/Analiz";
    TOPICS =  String(Topic1) + "/" + String(Topic2) + String(nombretopic) + String(n_Analiz);
  }
  else if (f_topic_Analog == true) {
    Serial.println("PREPARANDO TOPIC DE SENSORES");
    String nombretopic2 = "/ARS/EXTERIOR/SENSORES/";
    TOPICS =  String(Topic1) + "/" + String(Topic2) + String(nombretopic2) + String(Topic4);
  }

#else
  TOPICS =  String(Topic1) + "/" + String(Topic2) + "/" + String(Topic3) + "/" + String(Topic4);
#endif

  Serial.println("TOPICS devuelto: " + String(TOPICS));
  return TOPICS;
}





int ConectaMQTT ()
{
  if (modem.mqttConnect(localConfigMQTT.brokerID.c_str(), localConfigMQTT.port, localConfig.sensorID.c_str(), localConfigMQTT.user.c_str(), localConfigMQTT.pass.c_str()))
  {
    Serial.println("Conectado con Broker MQTT mediante BC95");
    F_BrokerMQTT = true;
    return 0;
  }
  else
    return 10;
}



String buildGenJSON() {
  String _output = "";
  int _nParametros = 0;

  _nParametros = NUMITEMS(s_ParamJSON);

  const size_t _capJSON = JSON_OBJECT_SIZE(_nParametros + 1) + JSON_ARRAY_SIZE(2) + 20; //+ 200;
  DynamicJsonDocument _JSON(_capJSON);

  for (int i = 0; i < NUMITEMS(s_ValJSON); i++) {
    if (s_ValJSON[i] != "Nan")
      _JSON[s_ParamJSON[i]] = s_ValJSON[i];
  }
  serializeJson(_JSON, _output);

  delay(100);

  _JSON.clear();

  LogJSON = _output;
  return _output;
}






int EnviaMQTT(String MessageToSend)
{
  finalTopic = DefTopic(localConfigMQTT.topic1 , localConfigMQTT.topic2 , localConfigMQTT.topic3, localConfigMQTT.topic4);

  Serial.println("Enviando mensaje al broker");

  //MessageToSend = "TEST: "+String(millis());

  if (modem.mqttSend(finalTopic.c_str(), MessageToSend.c_str()))
  {
    current_timestamp = rtc.getEpoch();
    SendTime = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));

    a_sendTime = false;
    Serial.println("Paquete MQTT enviado a topic : " + finalTopic + "  ||  Con el mensaje: " + String(MessageToSend));
    Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>estado de flag cambiado a: " + String(a_sendTime));
    createSendAlarm();

    Serial.println("Tiempo de envio actualizado");
    return 0;
  }
  else
  {
    Serial.println("NO SE HA DEVUELTO TRUE DE LA FUNCION MQTTSEND() DE LA LIBRERIA TINYGSM");
    delay(500);
    modem.mqttDisc();  //HAY QUE QUITARLO DE AQUI Y PONERLO EN 'S'
    return 11;
  }

  return 0;
}



String TakeTime()
{

  String timestamp = "NoInit_1";
  if (modem.isNetworkConnected())
  {
    Serial.println("");
    Serial.println("Obteniendo fecha y hora...");

    timestamp = modem._TakeTime();
    if (timestamp == "Error1!") {
      Serial.println("Error al actualizar la fecha con BC95G, comprobar conexión");
    }


    if ((((timestamp.substring(0, 4)).toInt()) < 2021 || ((timestamp.substring(0, 4)).toInt()) > 2050)  && f_cont < MAX_RETRAY)
    {
      f_cont++;
      Serial.println("Fecha no obtenida o no valida, reintento:" + String(f_cont));
      timestamp = "NoInit_2";
    }

    else if (f_cont == MAX_RETRAY)
    {
      Serial.println("ERROR AL TOMAR VALOR DE FECHA");
      f_cont = 0;
    }

    //f_cont = 0;
  }
  return timestamp;
}




int EnviaMIKROE(String MessageToSend)
{
  Serial.println("Preparando envío, SSL: " + String(localConfig.f_ssl));
  if (ConectaMQTT() == 10)
  {
    return 10;
  }
  else
  {
    if (EnviaMQTT(MessageToSend) == 11)
      return 11;
  }
  return 0;
}



void initComPort() {
  pinMode(M1_AN, INPUT_PULLDOWN);
  pinMode(M1_RST, OUTPUT);
  digitalWrite(M1_RST, LOW);

}


int startComPort() {
  unsigned long _timeout = millis();

  String buffer1 = "";
  if (!modem.isNetworkConnected())
  {
    modem.restart();

    String modemInfo = modem.getModemInfo();
    Serial.println("ModemInfo: ");
    Serial.println(modemInfo);

    if ( modem.gprsConnect(localConfig.APN.c_str(), localConfig.APNuser.c_str(), localConfig.APNpass.c_str()))
    {

      Serial.println("Esperando a la red [BC95].....");

      if (!modem.waitForNetwork()) {
        err_com++;
        contFallos++;
        Serial.println("Error al conectar con la red, tiempo de espera superado");
        Serial.println(millis() - _timeout);
        return 1;
      }
      else
        Serial.println("Red encontrada");

      if (modem.isNetworkConnected())
        Serial.println("Conectado!!");

      err_com = 0;
    }
    else
    {
      err_com++;
      contFallos++;
      Serial.println("Conexión con la red no establecida");
      return 1;
    }


  }
  return 0;
}



int envioDatosRS485Muutech(String JSONENVIO) {
  current_timestamp = rtc.getEpoch();
  sum_SecAct = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));

  Serial.print("Tiempo desde el último envío: " );
  Serial.println(( sum_SecAct - SendTime));
  Serial.print("Tiempo entre envíos: ");
  Serial.println(String(_sendTime) + "  s.");

  if (a_sendTime) {
    //  unsigned long _timeout=millis();

    if (modem.isNetworkConnected()) {
      retorno = EnviaMIKROE(JSONENVIO);
      return retorno;

    } else
    {
      Serial.println("---------------------------------------------DISPOSITIVO NO CONECTADO A LA RED "); //NO RECUERDO ORIGEN DE LOS TEST ,PARECE QUE FALTAN FILTROS DE MODULO Y TIPO DE COMUNICACIONES, NO RECUERDO LA NECESIDAD DE ESTE ELSE

      EnableMIKRO();//!!!!TEST
      retorno = EnviaMIKROE(JSONENVIO);//!!!!TEST
      return retorno;//!!!!TEST
      //return 1;
    }
  }
  else {
    Serial.print("Envío suspendido hasta cumplir tiempo - Tiempo esperado: " );
    Serial.print(String(_sendTime) + "  s.");
    Serial.print("   Tiempo trascurrido: ");
    Serial.println(( sum_SecAct - SendTime));
    return 99;
  }
}
