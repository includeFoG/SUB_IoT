#define TINY_GSM_MODEM_SARAR5
//#define TINY_GSM_MODEM_SARAR4
//#define TINY_GSM_MODEM_UBLOX
//#define TINY_GSM_MODEM_ESP32


const char * header_01  = "HTTP/1.1";
const char * header_02  = "Accept: */*";
const char * header_03  = "Cache-Control: no-cache";
const char * header_04  = "Content-Length: ";
const char * header_05  = "Connection: keep-alive";
const char * header_06  = "Content-Type: application/json";
const char * header_07  = "X-Auth-Token: ";

const char * closeHTTP = "0\r\n";


//const char * head_01  = "HTTP/1.1";
//const char * head_02  = "Accept: */*";
/*const char * head_03  = "Cache-Control: no-cache";
const char * head_04  = "Content-Length: ";
const char * head_05  = "Connection: keep-alive";
const char * head_06  = "Content-Type: application/json";
const char * head_07  = "X-Auth-Token: ";*/

const char * clHTTP = "0\r\n";

//long getDecimal(float val);



#ifdef TINY_GSM_MODEM_SARAR5
#include "commR5.h"
#endif
#ifdef TINY_GSM_MODEM_ESP32
#include "commESP32.h"
#endif

/*
   PUT /pruebas/mytest-003/directorio/miTexto.txt HTTP/1.1
   Host: f8i1bdejsc.execute-api.us-east-1.amazonaws.com
   Content-Type: text/plain
   Content-Length: 8

   my texto
*/

static int startComPort(char* _apn, char* _apn_user, char* _apn_pass) {
  //  unsigned long _timeout = millis();
  //  digitalWrite(M1_RST, HIGH);
  myDelayMs(4000);
  SERIAL.println("ISNETWORKCON Starcomport1");
  //    DEBUG("START GPS");
  //  modem.enableGPS();
  //if (!modem.isNetworkConnected()) { #MODIF 03-02-23 //TIENE QUE ESTAR MUY BIEN CONFIGURADO EL MÓDULO PARA QUE NO SE TENGA QUE USAR GPRSCONNECT EN CADA ENVIO
  //    while (digitalRead(M1_AN) == 0)myDelayMs(5);
  DEBUG("Reset Modulo de comunicaciones");
  if (!modem.restart()) //ADDED MODIF  if (!modem.restart())
  {
    digitalWrite(M1_RST, HIGH); //ADDED
    myDelayMs(2000); //ADDED
    digitalWrite(M1_RST, LOW); //ADDED

    return 2;
  }
  //    DEBUG("START GPS");
  //    modem.enableGPS();
  DEBUG("Conectando");
  if (modem.gprsConnect(_apn, _apn_user, _apn_pass))
  {
    DEBUG("Esperando a la red....");
    if (!modem.waitForNetwork()) {
      DEBUG("Error al conectar con la red, tiempo de espera superado");
    }
    else{
      DEBUG("Red Encontrada");
    }
  } //ADDED MODIF
  SERIAL.println("ISNETWORKCON Starcomport2");
  if (modem.isNetworkConnected()) {
    DEBUG("Conectado");
    //return 0;
  }
  else {
    DEBUG("<UBLOX> No se ha podido conectar a la red GPRS");
    modem.gprsDisconnect();

    return 1;
  }
  // }
  return 0;
}


int startComPort(const char* _apn, const char* _apn_user, const char* _apn_pass)
{
  return  startComPort((char*) _apn, (char*) _apn_user, (char*) _apn_pass);
}




bool serverConnect(const char* localHost, bool SSL = false, int port = 80)
{
  if ( SSL == false)
  {
    // client.stop();
    // myDelayMs(1000); //ADDED TEST
    return (client.connect(localHost, port, 60)); //return (client.connect(localHost, port, 10000)); //recibe el timeout en segundos
  }
  //clientSSL.stop();
  //myDelayMs(1000); //ADDED TEST
  return (clientSSL.connect(localHost, port, 60)); // return (clientSSL.connect(localHost, port, 10000));//recibe el timeout en segundos
}

/*long getDecimal(float val) //MAX 5 decimales
  {
  int intPart = int(val);
  long decPart = 100000 * (val - intPart); //I am multiplying by 100000 assuming that the foat values will have a maximum of 5 decimal places.
  //Change to match the number of decimal places you need
  if (decPart > 0)return (decPart);       //return the decimal part of float number if it is available
  else if (decPart < 0)return ((-1) * decPart); //if negative, multiply by -1
  else if (decPart = 0)return (00);       //return 0 if decimal part of float number is not available
  }*/



String floatToString(float val, int numDec) {
  float valDecimals = 0.0;
  if (val < 0)
    valDecimals = - (val - int(val));
  else
    valDecimals = val - int(val);
    
  String outString = String(int(val)) + ".";

  for (int iter = 0; iter < numDec; iter++) {
    valDecimals = valDecimals * 10.0; //rotamos la coma 1 a la derecha 
    outString.concat(char('0'+int(valDecimals))); //outString += String(int(valDecimals));
    valDecimals = valDecimals - int(valDecimals);
  }
  return outString;
}




String buildPut(int sizeFile, String fileName)
{
  String temp = "PUT ";
  String root = "%2F" + String(DEVICE) + "%2F" + fileName.substring(0, 8) + "%2F" + String(DATALOG) + "%2F";
  //  if (fileName.endsWith(".csv"))
  //  {
  //    temp +=   path  + root + fileName + " " + header_01 + "\r\n";
  //    temp += "Host: " + host + "\r\n";
  //    temp += "Content-Type: text/plain\r\n";
  //    //    temp += "Connection: keep-alive\r\n";
  //    temp += header_04 + String(sizeFile) + "\r\n";
  //  }
  //  else
  //  {
  temp +=   AWS_PATH;
  temp += root;
  temp += fileName;
  temp +=" ";
  temp +=header_01;
  temp += "\r\n";
  temp += "Host: ";
  temp += String(AWS_HOST);
  temp +="\r\n";
  
  if (sizeFile > 0)
  {
    temp += "Content-Type: text/plain\r\n";
    //      temp += "Connection: keep-alive\r\n";
    temp += header_04;
    temp += String(sizeFile);
    temp += "\r\n";
  }
  //  }
  return temp;
}

/*
String buildPut(char * path, char *host, int sizeFile, char *fileName)
{
  //  char *strcat(char *dest, const char *src)
  String temp = "PUT ";
  String root = "%2F" + String(DEVICE) + "%2F" + String(fileName).substring(0, 8) + "%2F" + String(DATALOG) + "%2F";
  temp +=   String(path)  + root + String(fileName) + " " + String(head_01) + "\r\n";
  temp += "Host: " + String(host) + "\r\n";
  if (sizeFile > 0)
  {
    temp += "Content-Type: text/plain\r\n";
    //      temp += "Connection: keep-alive\r\n";
    temp += String(head_04) + String(sizeFile) + "\r\n";
  }
  return temp;
}
*/




//ADDED MODIF
//uint8_t postTCPToUbidots(float *datos, qPosition pos, bool SSL, char* token, const char* deviceName, int signalQuality, int state, bool switchVar = false)
//{
//  static String payload = "";
//  static String response = "";
//#ifdef DEBUG_LOGEVENTS
//  byte b_infoLog = NULL;//ADDEDLOG
//#endif
//  uint8_t numVar = 0; //ADDED
//  uint8_t numOK = 0; //ADDED   numero de OK obtenidos
//  uint16_t index = 0; //ADDED
//
//  payload = "ubidots/1.0|POST|" + String(token) + "|" + String(deviceName) + "=>";
//
//  if (switchVar)
//  {
//    payload += VAR_LABEL;
//    payload += ":0";
//  }
//  else {
//
//
//    for (int i = 0; i < NUM_VALORES; i++)
//    {
//      if ( i < 10) payload += "adc_0" + String(i);
//      else        payload += "adc_"  + String(i);
//      DEBUG(".");
//      payload += ":";
//      payload += String(int(datos[i])) + "." + String(getDecimal(datos[i]));
//      payload += ",";
//    }
//    payload += "sigQ:" + String(signalQuality) + ",";
//    payload += "Vaux:" + String(int(datos[NUM_VALORES])) + "." + String(getDecimal(datos[NUM_VALORES])) + ",";
//    payload += "Vmain:" + String(int(datos[NUM_VALORES + 1])) + "." + String(getDecimal(datos[NUM_VALORES + 1])) + ",";
//    payload += "Stat:" + String(state) + ",";
//
//    payload += "position:1";
//    payload += "$lat=";
//    payload += String(int(pos.latitude)) + "." + String(getDecimal(pos.latitude));
//    payload += "$lng=";
//    payload += String(int(pos.longitude)) + "." + String(getDecimal(pos.longitude));
//  }
//  payload += "|end";
//
//  SERIAL.println();
//  SERIAL.print("--->payload0: ");
//  SERIAL.println(payload);
//
//  //ADDED
//  for (int j = 0; j < payload.length(); j++)
//  {
//    if (payload[j] == ',')
//      numVar++;
//  }
//  numVar++; //ultima variable que no tiene coma despues
//
//  //DEBUG("NUMERO DE VARIABLES: " + String(numVar));
//  //ENDADDED
//
//
//  unsigned long mytimerwait;
//  char c;
//
//  mytimerwait = millis();
//
//  if ( SSL == false)
//  {
//    client.print(payload);
//
//    while (( millis() - mytimerwait < 30000))
//    {
//      //  Serial.print(".");
//      if ( client.available())
//      {
//        c = client.read();
//        //Serial.write(c);
//        //Serial.println();
//        response += (char) c;
//      }
//      if (response.length() >= (numVar * 2 + numVar - 1))
//      {
//        for (int i = 0; i <= numVar; i++) {
//          if (index = response.indexOf("OK", index) != -1) {
//            numOK++;
//          }
//          if (numOK == numVar) {
//            break;
//          }
//        }
//      }
//    }
//  }
//  else
//  {
//    clientSSL.print(payload);
//
//    //ADDED MODIFIED
//    while (( millis() - mytimerwait < 30000))
//    {
//      //  Serial.print(".");
//      if ( clientSSL.available())
//      {
//        c = clientSSL.read();
//        SERIAL.write(c);
//        response += (char) c;
//      }
//      if (response.length() >= (numVar * 2 + numVar - 1))
//      {
//        for (int i = 0; i <= numVar; i++) {
//          if (index = response.indexOf("OK", index) != -1) {
//            numOK++;
//          }
//          if (numOK == numVar) {
//            break;
//          }
//        }
//        break;
//      }
//    }
//    SERIAL.println();
//  }
//
//  if (response.indexOf("OK") != -1)
//  {
//    //ADDEDLOG
//#ifdef DEBUG_LOGEVENTS
//    b_infoLog = B00000011; //ENVIO UBIDOTS CORRECTO
//    sendToEventsLogQueue(b_infoLog);
//#endif
//    //ENDADDED
//
//    return  200;
//  }
//  else
//  {
//    //ADDEDLOG
//#ifdef DEBUG_LOGEVENTS
//    b_infoLog = B10000011; //ERROR ENVIO UBIDOTS
//    sendToEventsLogQueue(b_infoLog);
//#endif
//    //ENDADDED
//  }
//  return -1;
//}
////ENDADDED


//NUEVA VERSION 26/05/22
uint8_t postTCPToUbidots(float *datos, qPosition pos, bool SSL,const char* token, const char* deviceName, int signalQuality, int state, bool switchVar = false)
{
  DEBUG("postToUbi");
  static String payload="";
  payload = "";
  static String response="";
  response = "";
#ifdef DEBUG_LOGEVENTS
  byte b_infoLog = NULL;//ADDEDLOG
#endif
  uint8_t numVar = 0; //ADDED
  uint8_t numOK = 0; //ADDED   numero de OK obtenidos
  int index = 0; //ADDED

  payload = "ubidots/1.0|POST|" + String(token) + "|" + String(deviceName) + "=>";

  if (switchVar)
  {
    payload += VAR_LABEL;
    payload += ":0";
  }
  else {
    for (int i = 0; i < NUM_VALORES; i++)
    {
      if ( i < 10){
        payload += "adc_0";
        payload += String(i);
      }
      else{
        payload += "adc_";
        payload += String(i);
      }
      DEBUG(".");
      payload += ":";

      payload += floatToString(datos[i], 2);    //String(int(datos[i])) + "." + String(getDecimal(datos[i]));
      payload += ",";
    }
    payload += "sigQ:";
    payload += String(signalQuality);
    payload += ",";
    payload += "Vaux:";
    payload += floatToString(datos[NUM_VALORES], 2);
    payload += ",";       //String(int(datos[NUM_VALORES])) + "." + String(getDecimal(datos[NUM_VALORES])) + ",";
    payload += "Vmain:";
    payload += floatToString(datos[NUM_VALORES + 1], 2);
    payload += ",";  //String(int(datos[NUM_VALORES + 1])) + "." + String(getDecimal(datos[NUM_VALORES + 1])) + ",";
    payload += "Stat:";
    payload += String(state);

    //  if (pos.fix == 1)
    //  {
    payload += ",";
    payload += "position:1";
    payload += "$lat=";
    payload += floatToString(pos.latitude, 5); //String(int(pos.latitude)) + "." + String(getDecimal(pos.latitude)); //AQUI ESTA EL PROBLEMA DE DERIVA DE GPS
    payload += "$lng=";

    payload += floatToString(pos.longitude, 5); //String(int(pos.longitude)) + "." + String(getDecimal(pos.longitude));
    //    }
    //    else {
    //      payload += ",";
    //      payload += "position:0";
    //      payload += "$lat=";
    //      payload += String(int(pos.latitude)) + "." + String(getDecimal(pos.latitude));
    //      payload += "$lng=";
    //      payload += String(int(pos.longitude)) + "." + String(getDecimal(pos.longitude));
    //    }
  }
  payload += "|end";

  SERIAL.print("\n-->payload0: ");
  SERIAL.println(payload);

  //ADDED
  for (int j = 0; j < payload.length(); j++)
  {
    if (payload[j] == ',')
      numVar++;
  }
  numVar++; //ultima variable que no tiene coma despues

  //ENDADDED


  unsigned long mytimerwait;
  static char c;

  mytimerwait = millis();

  // if (modem.isNetworkConnected()) { //QUITADO EN v046
  if ( SSL == false)
  {
    client.print(payload);
    SERIAL.print(">"); //TESTEO
    //ADDED MODIFIED
    while ((response.length() < (numVar * 2 + (numVar - 1)))  && ((millis() - mytimerwait) < 30000))
    {
      //  Serial.print(".");
      if ( client.available())
      {
        c = client.read();
        response += (char) c;
        SERIAL.write(c);
      }
    }
    if (response.length() >= 2)
    {
      for (int i = 0; i < numVar; i++) {
        index = response.indexOf("OK", index);
        if (index != -1) {
          numOK++;
        }
      }
    }

    SERIAL.println();
  }
  else
  {
    clientSSL.print(payload);
    SERIAL.print(">>"); //TESTEO
    //ADDED MODIFIED
    while ((response.length() < (numVar * 2 + numVar - 1))  && ((millis() - mytimerwait) < 30000))
    {
      //  Serial.print(".");
      if ( clientSSL.available())
      {
        c = clientSSL.read();
        response += (char) c;
        SERIAL.write(c);             //HAY VECES QUE NO LO IMPRIME
      }
    }
    while (clientSSL.available() && ((millis() - mytimerwait) < 5000)) {
      clientSSL.read();
      //SERIAL.println("ESTABA DISPO");
    }
    clientSSL.flush();

    SERIAL.print("Resp length:");
    SERIAL.println(response.length()); //TESTEO
    if (response.length() >= 2)
    {
      for (int i = 0; i < numVar; i++) {
        index = response.indexOf("OK", index);
        if (index != -1) {
          numOK++;
        }
      }
    }

    SERIAL.println();
  }
  payload="";
  response="";

  if (numOK == numVar)
  {
    //ADDEDLOG
#ifdef DEBUG_LOGEVENTS
    b_infoLog = B00000011; //ENVIO UBIDOTS CORRECTO
    sendToEventsLogQueue(b_infoLog);
#endif
    //ENDADDED
    return  200; //Todo el mensaje se ha enviado correctamente
  }
  else if ( numOK > 0)
  {
    //ADDEDLOG
#ifdef DEBUG_LOGEVENTS
    b_infoLog = B10000101; //ERROR PARCIAL ENVIO UBIDOTS
    sendToEventsLogQueue(b_infoLog);
#endif
    //ENDADDED
    return 100; //Parte del mensaje se ha enviado correctamente
  } else {
    //ADDEDLOG
#ifdef DEBUG_LOGEVENTS
    b_infoLog = B10000011; //ERROR ENVIO UBIDOTS
    sendToEventsLogQueue(b_infoLog);
#endif
    //ENDADDED
  }
  // }
  return -1; //El mensaje no se ha enviado correctamente
}
//ENDADDED



uint8_t getTCPFromUbidots(bool SSL) //ADDED VarID (Switch): 624b1d570dcd2a30c4112697
{
  String payload = ""; //MODIFICAR A CHAR[]
  String response_ = "";
#ifdef DEBUG_LOGEVENTS
  byte b_infoLog = NULL;//ADDEDLOG
#endif

  const uint8_t numVar = 1;

  //WITH VARIABLE LABEL (es modificable y adaptable a otras etiquetas)

  payload = "ubidots/1.0|LV|" + String(TOKEN) + "|" + String(DEVICE) + ":" + String(VAR_LABEL) + "|end"; //VARIABLE label|end "nombre de variable"
  //payload = "ubidots/1.0|GET|" + String(token) + "|" + String(varLabel) + "|end"; //VARIABLE ID|end "Codigo numerico"

  SERIAL.print("payload: ");
  SERIAL.println(payload);

  unsigned long mytimerwait;
  char c;
  mytimerwait = millis();

  if ( SSL == false)
  {
    client.print(payload);

    while ( (millis() - mytimerwait) < 30000)
    {
      //  Serial.print(".");
      if ( client.available())
      {
        c = client.read();
        //Serial.write(c);
        //Serial.println();
        response_ += (char) c;
      }

      if ((response_.length() >= (numVar * 2 + 2)) && (response_.indexOf("OK") != -1)) { //en este caso OK+2 ("|" y "bool") -> OK|0
        //MODIFICAR POR VARIABLES
        //client.stop();
        break;
      }

    }
    // client.stop();
  }
  else
  {
    clientSSL.println(payload);

    while ( (millis() - mytimerwait) < 30000)
    {
      //  Serial.print(".");
      if ( clientSSL.available())
      {
        c = clientSSL.read();
        SERIAL.write(c);
        response_ += (char) c;
      }

      if ((response_.length() >= (numVar * 2 + 2)) && (response_.indexOf("OK") != -1)) { //en este caso +2 "|" y "bool"
        // clientSSL.stop();
        break;
      }
    }

    while (clientSSL.available() && ((millis() - mytimerwait) < 5000)) {
      clientSSL.read();
      //SERIAL.println("ESTABA DISPO2");
    }
    clientSSL.flush();
    // clientSSL.stop();
    SERIAL.println();
  }

  payload="";

  DEBUG("TCP RESPONSE: " + response_);

  if (response_.indexOf("OK") != -1)
  {
    //ADDED LOG
#ifdef DEBUG_LOGEVENTS
    b_infoLog = B00001001; //PETICION UBIDOTS CORRECTO
    sendToEventsLogQueue(b_infoLog);
#endif
    //ENDADDED
    SERIAL.println(response_.substring(3, 4).toInt());

    return   response_.substring(3, 4).toInt();
  }
  else
  {
#ifdef DEBUG_LOGEVENTS
    b_infoLog = B10001001; //ERROR PETICION UBIDOTS
    sendToEventsLogQueue(b_infoLog);
#endif
  }
  return -1;

  //WITH VARIABLE ID (no es modificable código unico)

  //
  //  payload = "ubidots/1.0|GET|" + String(token) + "|" + String(varID) + "|end"; //VARIABLE ID|end
  //
  //  Serial.print("--->payload0: ");
  //  Serial.println(payload);
  //
  //  unsigned long mytimerwait;
  //  char c;
  //  if ( SSL == false)
  //  {
  //    client.print(payload);
  //
  //    while (( millis() - mytimerwait < 30000))
  //    {
  //      //  Serial.print(".");
  //      if ( client.available())
  //      {
  //        c = client.read();
  //        response_ += (char) c;
  //      }
  //    }
  //  }
  //  else
  //  {
  //    clientSSL.println(payload);
  //    mytimerwait = millis();
  //
  //    while (( millis() - mytimerwait < 30000))
  //    {
  //      if ( clientSSL.available())
  //      {
  //        c = clientSSL.read();
  //        response_ += (char) c;
  //      }
  //    }
  //  }
  //
  //  Serial.print("RESPONSE: ");
  //  Serial.println(response_);
  //  return response_;
}
//ENDADDED





//
//
//char* buildGet( ) { //ADDED   !!!PARAMETRIZAR
//  char* getPet =    "GET /test0/ftp-test-private/WP-test%2FConfig%2FConfigAWS.txt HTTP/1.1\r\n"
//                    "Host: qp29wxo1pf.execute-api.us-east-1.amazonaws.com\r\n"
//                    "Content-Type: text/plain\r\n"
//                    "Content-Length: 8\r\n"
//                    "\r\n"
//                    "my texto\r\n";
//  return getPet;
//}
//
//char* buildHTTP(uint8_t type, char* secondPath) { //ADDED   path es a partir de APIPATH: /test0/ftp-test-private/WP-test
//  //  char* command = "";
//  //char  petition[400] = "";
//  //  char* c_http =  " HTTP/1.1\r\n";
//  //  char* c_host = "Host: ";
//  //  char* c_final = "\r\n";
//  //
//  //  char* header = "Content-Type: text/plain\r\n"
//  //                 "Content-Length: 8\r\n"
//  //                 "\r\n"
//  //                "mi texto";
//  //
//  //  switch (type) {
//  //    case 1:
//  //      command = "GET ";
//  //
//  //      break;
//  //    case 2:
//  //      command = "DELETE ";
//  //      break;
//  //  }
//  //
//  //  strcpy(petition, command);
//  //  strcat(petition, localConfig.APIPATH);
//  //  strcat(petition, secondPath);
//  //  strcat(petition, c_http);
//  //  strcat(petition, c_host);
//  //  strcat(petition, localConfig.BUCKET);
//  //  strcat(petition, c_final);
//  //  strcat(petition, header);
//
//
//  char* getPet =    "DELETE /test0/ftp-test-private/WP-test%2FConfig%2FConfigAWS.txt HTTP/1.1\r\n"
//                    "Host: qp29wxo1pf.execute-api.us-east-1.amazonaws.com\r\n"
//                    "Content-Type: text/plain\r\n"
//                    "Content-Length: 8\r\n"
//                    "\r\n"
//                    "my texto\r\n";
//
//
//  //return petition;
//  return getPet;
//}
//
//



//String buildPut()
//{
//  String temp =    "PUT /pruebas/mytest-003/miTexto.txt HTTP/1.1\r\n"
//                   "Host: f8i1bdejsc.execute-api.us-east-1.amazonaws.com\r\n"
//                   "Content-Type: text/plain\r\n"
//                   "Content-Length: 8\r\n"
//                   "\r\n"
//                   "my texto\r\n";
//  return temp;
//}
