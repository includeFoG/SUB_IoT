#ifndef TINY_GSM_MODEM_SARAR5
#define TINY_GSM_MODEM_SARAR5
#endif

#define GSM_PIN ""
#define TINY_GSM_RX_BUFFER 64
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#define USE_SSL true
#define FLOW_CONTROL

#define MAX_RETRAY        3

#include <TinyGsmClient.h>
//#include <PubSubClient.h>

#include <ArduinoHttpClient.h>
//#include <ArduinoJson.h>

//int err_com = 0;

TinyGsm modem(SerialM1);
TinyGsmClientSecure clientSSL(modem, M1_BAUD);
TinyGsmClient client(modem);
//
//const size_t capacity = JSON_OBJECT_SIZE(NUM_VALORES) * 6 + JSON_ARRAY_SIZE(2);
//StaticJsonDocument<capacity> doc;
//String mydata = "";

void powerOffComPort()
{
  if (!modem.poweroff())
  {
    digitalWrite(M1_RST, HIGH);
    myDelayMs(2000);
    digitalWrite(M1_RST, LOW);
    myDelayMs(2000);
  }

  pinMode(M1_AN, INPUT_PULLDOWN);
  pinMode(M1_RST, INPUT_PULLDOWN);
  pinMode(M1_CS, INPUT_PULLDOWN);
  pinMode(M1_PWM, INPUT_PULLDOWN);
  pinMode(M1_INT, INPUT_PULLDOWN);

}
void initComPort()
{
  DEBUG("<INITCOMPORT>");

  //ADDED MODIF
  pinMode(M1_AN, OUTPUT);
  digitalWrite(M1_AN, HIGH); //antes LOW
  myDelayMs(500); //antes 10
  //ENDADDED

  pinMode(M1_AN, INPUT_PULLDOWN);
  pinMode(M1_RST, OUTPUT);
  digitalWrite(M1_RST, LOW);
  pinMode(M1_INT, INPUT);
}
//
//int startComPort() { //ADDED MODIF
//  unsigned long _timeout = millis();
//  //  digitalWrite(M1_RST, HIGH);
//  myDelayMs(4000);
//  if (!modem.isNetworkConnected())
//  {
//    if (!modem.restart())
//    {
//      digitalWrite(M1_RST, HIGH);
//      myDelayMs(2000);
//      digitalWrite(M1_RST, LOW);
//      return -1;
//    }
//    String modemInfo = modem.getModemInfo();
//    SERIAL.println("ModemInfo: ");
//    SERIAL.println(modemInfo);
//    if ( modem.gprsConnect(APN, APN_USER, APN_PASS))
//    {
//      SERIAL.println("Esperando a la red.....");
//      if (!modem.waitForNetwork()) {
//        SERIAL.println("Error al conectar con la red, tiempo de espera superado");
//        SERIAL.println(millis() - _timeout);
//      }
//      else
//        SERIAL.print("Red encontrada...");
//
//      if (modem.isNetworkConnected())
//        SERIAL.println("Conectado!!");
//    }
//    else
//    {
//      return 1;
//    }
//  }
//  return 0;
//}

String updateTime()
{
#ifdef DEBUG_LOGEVENTS
  byte b_infoLog = NULL;
#endif

  String timeStamp = "";
  uint8_t retry_updt = 0;

  //timeStamp = modem._TakeTime();
  myDelayMs(2000);
  do {
    timeStamp = modem._TakeTime();
    SERIAL.println(timeStamp);
    myDelayMs(2000);
    retry_updt++;
  } while (((timeStamp == "Error1!") || (timeStamp == "Error2!")) && (retry_updt <= RETRY_CONNECTION + 1));

  // DEBUG(timeStamp);
  if (timeStamp == "Error1!" || timeStamp == "Error2!") {
    DEBUG("Error al actualizar la fecha, comprobar conexión");
#ifdef DEBUG_LOGEVENTS
    b_infoLog = B10000111; //ERROR AL TOMAR HORA
#endif
    return "-1";
  }
  return timeStamp;
}


uint32_t parseDate(String date)
{
  String _month="";
  struct tm tm;
  tm.tm_isdst = -1;
  tm.tm_yday = 0;
  tm.tm_wday = 0;
  tm.tm_isdst = -1;
  tm.tm_yday = 0;
  tm.tm_wday = 0;
  tm.tm_year = (uint8_t)(date.substring(0, 4).toInt() - 1900);
  tm.tm_mon  = (uint8_t)(date.substring(5, 7).toInt() - 1);
  tm.tm_mday = (uint8_t) date.substring(8, 10).toInt();
  tm.tm_hour = (uint8_t) date.substring(11, 13).toInt();
  tm.tm_min  = (uint8_t) date.substring(14, 16).toInt();
  tm.tm_sec  = (uint8_t) date.substring(17).toInt();

  return mktime(&tm);
}
