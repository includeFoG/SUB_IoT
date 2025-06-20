#ifndef TINY_GSM_MODEM_ESP32
#define TINY_GSM_MODEM_ESP32
#endif

#define GSM_PIN ""
#define TINY_GSM_RX_BUFFER 64
#define TINY_GSM_USE_GPRS false
#define TINY_GSM_USE_WIFI true
#define USE_SSL true


#define MAX_RETRAY        3

#include <TinyGsmClient.h>
//#include <PubSubClient.h>

#include <ArduinoHttpClient.h>
//#include <ArduinoJson.h>

int err_com = 0;
int rfPower = 11;

TinyGsm modem(SerialM1);
TinyGsmClient client(modem);
TinyGsmClientSecure clientSSL(modem, M1_BAUD);


//const size_t capacity = JSON_OBJECT_SIZE(NUM_VALORES) * 6 + JSON_ARRAY_SIZE(2);
//StaticJsonDocument<capacity> doc;
//String mydata = "";



void powerOffComPort()
{
  pinMode(M1_AN, INPUT_PULLDOWN);
  pinMode(M1_RST, INPUT_PULLDOWN);
  pinMode(M1_CS, INPUT_PULLDOWN);
  pinMode(M1_PWM, INPUT_PULLDOWN);
  pinMode(M1_INT, INPUT_PULLDOWN);

}
void initComPort()
{
  pinMode(M1_AN, INPUT_PULLDOWN);
  pinMode(M1_RST, OUTPUT);
  pinMode(M1_CS, OUTPUT);
  pinMode(M1_PWM, INPUT_PULLDOWN);
  pinMode(M1_INT, INPUT_PULLDOWN);

  digitalWrite(M1_RST, LOW);
  digitalWrite(M1_CS, LOW);
}

int startComPort() {
  unsigned long _timeout = millis();

  digitalWrite(M1_RST, HIGH);
  digitalWrite(M1_CS, HIGH);
  myDelayMs(1);
  if (!modem.isNetworkConnected()) {

    if (!modem.restart())
    {
      return -1;
    }
    DEBUG("Restarting Modem");
    modem.setTxPwr(rfPower);
    myDelayMs(1);
    String nearestMAC = modem.searchNearestAp("MarinePark");
    if (!modem.networkConnect("MarinePark", "Marine20_Park16", nearestMAC.c_str()))
    {
      DEBUG("ESP32 No se ha podido conectar a la red ");
      if ( rfPower != 0)rfPower--;
      return 1;
    }
    if (!modem.waitForNetwork())
    {
      if ( rfPower != 0)rfPower--;
      DEBUG("Error al conectar con la red, tiempo de espera superado");
      DEBUG(millis() - _timeout);
      return 1;
    }
    else DEBUG("Network connected");

    err_com = 0;
  }
  return 0;
}
//
//String buildGenPOST_Header (String path, String host, String device, String token)
//{
//  //  Serial.print("2");
//  String temporal = "POST ";
//  temporal += path + device + " " + header_01 + "\r\n";
//  temporal += "Host: " + host + "\r\n";
//  temporal +=  header_07 + token + "\r\n";
//  temporal +=  header_02 + "\r\n";
//  //temporal +=  header_05 + "\r\n";
//  temporal +=  header_06 + "\r\n";
//  //    Serial.print("3");
//  //  temporal += header_04 + String(50) + "\r\n";
//  temporal += header_04 + String(measureJson(doc)) + "\r\n";
//  //    Serial.print("4");
//  return temporal;
//}



String updateTime()
{
  String timeStamp = "";
  int _res = modem._ActuTime(); //SET THE TIME ZONE

#ifdef DEBUG_LOGEVENTS
  byte b_infoLog = NULL;
#endif

  if (_res != 1) {
    DEBUG("Error al establecer la franja horaria con ESP32");

    //ADDED
#ifdef DEBUG_LOGEVENTS
    b_infoLog = B10000111; //ERROR AL TOMAR HORA
#endif
    //ENDADDED
  }
  else {
    //    if (SerialM1.available())
    timeStamp = modem._TakeTime();
    if (timeStamp == "Error1!" || timeStamp == "Error2!") {
      DEBUG("Error al actualizar la fecha con ESP32, comprobar conexión");

      //ADDED
#ifdef DEBUG_LOGEVENTS
      b_infoLog = B10000111; //ERROR AL TOMAR HORA
#endif
      //ENDADDED
    }
    
    //ADDED
#ifdef DEBUG_LOGEVENTS
    else {
      b_infoLog = B00000111; //HORA COGIDA CORRECTAMENTE
    }
    sendToEventsLogQueue(b_infoLog);
#endif
    //ENDADDED
  }

  DEBUG(timeStamp);
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
  tm.tm_year = (uint8_t)((date.substring(16, 20)).toInt() - 1900);
  _month = date.substring(0, 3);
  if      (_month == "Jan") tm.tm_mon =  0;
  else if (_month == "Feb") tm.tm_mon =  1;
  else if (_month == "Mar") tm.tm_mon =  2;
  else if (_month == "Apr") tm.tm_mon =  3;
  else if (_month == "May") tm.tm_mon =  4;
  else if (_month == "Jun") tm.tm_mon =  5;
  else if (_month == "Jul") tm.tm_mon =  6;
  else if (_month == "Aug") tm.tm_mon =  7;
  else if (_month == "Sep") tm.tm_mon =  8;
  else if (_month == "Oct") tm.tm_mon =  9;
  else if (_month == "Nov") tm.tm_mon = 10;
  else if (_month == "Dec") tm.tm_mon = 11;
  tm.tm_mday = (uint8_t)date.substring(4, 6).toInt();
  tm.tm_hour = (uint8_t)date.substring(7, 9).toInt();
  tm.tm_min  = (uint8_t)date.substring(10, 12).toInt();
  tm.tm_sec  = (uint8_t)date.substring(13, 15).toInt();

  return mktime(&tm);
}

//
//int sendPostToUbidots(float *datos, qPosition pos, bool SSL = true)
//{
//
//  const size_t CAP2 = JSON_OBJECT_SIZE(1);
//  StaticJsonDocument<CAP2> doc1;
//  // create an object
//  JsonObject object = doc1.to<JsonObject>();
//  object["value"] = 0;
//  // serialize the object and send the result to Serial
//  String response = "";
//  String response_status = "-1";
//  String response_header = "";
//  String local_body = "";
//  String value = "";
//  doc.clear();
////  SERIAL.print("1");
//  for (int i = 0; i < NUM_VALORES; i++)
//  {
//    String aux ;
//    if ( i <10) aux = "adc_0" + String(i);
//    else        aux = "adc_"  + String(i);
//    JsonObject object = doc.createNestedObject(aux);
//    object["value"] = datos[i];
//  }
////  SERIAL.print("2");
//  if (pos.fix == 1)
//  {
//    SERIAL.print("3");
//    JsonObject position = doc.createNestedObject("position");
//    position["lat"] = pos.latitude;
//    position["lng"] = pos.longitude;
//  }
////SERIAL.print("4");
//  //DEBUG(buildGenPOST_Header(PATH, HOST, DEVICE, TOKEN));
////  DEBUG("Send data: ");
//  serializeJson(doc, SERIAL);
//  SERIAL.println();
//  //  client.println("{\"adc_01\": {\"value\": 0.0}, \"adc_02\": {\"value\": 4}}");
//
//  unsigned long mytimerwait;
//  char c;
////  SERIAL.print("5");
//  if ( SSL == false)
//  {
//    String temp = "";
// //   Serial.print("SEND DATA");
//// SERIAL.print("6");
//    client.println(buildGenPOST_Header(PATH, HOST, DEVICE, TOKEN));
//    serializeJson(doc, temp);
//    client.println(temp);
//
//    client.println();
//    temp = "";
//    mytimerwait = millis();
////Serial.print("Read response");
//    while (( millis() - mytimerwait < 30000))
//    {
//    //  Serial.print(".");
//      if ( client.available())
//      {
//    //    Serial.print("Datos en el buffer ");
//     //   Serial.println(client.available());
//        c = client.read();
//     //   Serial.write(c);
//        response += (char) c;
//        if ( response.endsWith("HTTP/1.1 "))
//        {
//          response = "";
//        }
//        else if (response.endsWith(" "))
//        {
//          //        response.remove(response.length() - 3);
//          response.trim();
//          SERIAL.println("7");
//          return  response.toInt();
//        }
//        if (response.length() == 64)
//        {
//          response = "";
//        }
//      }
//      myDelayMs(10);
//    }
//    SERIAL.println("8");
//
//    return -1;
//  }
//  else
//  {
//    clientSSL.println(buildGenPOST_Header(PATH, HOST, DEVICE, TOKEN));
//    String temp = "";
//    serializeJson(doc, temp);
//    clientSSL.println(temp);
//    temp = "";
//    mytimerwait = millis();
//
//    while (( millis() - mytimerwait < 30000))
//    {
//      if ( clientSSL.available())
//      {
//        c = clientSSL.read();
//        response += (char) c;
//        if ( response.endsWith("HTTP/1.1 "))
//        {
//          response = "";
//        }
//        else if (response.endsWith(" OK"))
//        {
//          response.remove(response.length() - 3);
//          return  response.toInt();
//        }
//        if (response.length() == 64)
//        {
//          response = "";
//        }
//      }
//    }
//  }
//  return -1;
//}
