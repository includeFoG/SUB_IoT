
//#define NO_DISCONNECT  //determina si el módulo puede apagarse/desconectarse de la red después del envío no influye a apagados por reintentos de envío

//MIKROE M1
//#define TINY_GSM_MODEM_UBLOX
//#define TINY_GSM_MODEM_SIM808
//#define TINY_GSM_MODEM_ESP32
//#define TINY_GSM_MODEM_SARAR4
#define TINY_GSM_MODEM_BC95G
//#define TINY_GSM_MODEM_SARAR5
//#define TINY_GSM_MODEM_NRF9160


//MIKROE M2
//#define ETH_MODEM_W5500

//COMUNICACIONES
#define GSM_PIN ""


#ifdef  TINY_GSM_MODEM_ESP32
#define TINY_GSM_RX_BUFFER 650
#define TINY_GSM_USE_GPRS false
#define TINY_GSM_USE_WIFI true
#else
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#define TINY_GSM_RX_BUFFER 1024 //sarar410:650 //esp32:512 //DEFAULT: 1024
#endif


#define TINY_GSM_MQTT

#define MAX_RETRAY        3


//#if defined(TINY_GSM_MODEM_SARAR4)||defined(TINY_GSM_MODEM_BC95G)
//#define FLOW_CONTROL
//#endif



#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <TimeLib.h>


#ifdef TINY_GSM_MQTT
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
#endif

#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

int retry_connect = 0;
int retry_connect2 = 0;
int retry_send = 0;
int retry_actu = 0;
int err_com = 0;
int retorno = 33;

int sigQ = 0 ;
int rfPower = 6;//11

int f_cont = 0;

TinyGsm modem(SerialM1);

TinyGsmClientSecure clientSSL(modem, M1_BAUD);
TinyGsmClient client(modem);

#ifdef TINY_GSM_MQTT
PubSubClient mqtt(client); //mqtt(DIR_BROKER_MQTT,MQTT_PORT,mqttCallback,client);
#endif


TinyGsmClient ftpControl(modem, 2); //Ftp1 cliente de control en socket 2
TinyGsmClient ftpData(modem, 3); //Ftp1 cliente de datos en socket 3
FTP ftp(ftpControl, ftpData);//Ftp1


////////DWLD///// =)

const char serverDwld[] = "mytest-003.s3-website-us-east-1.amazonaws.com"; //"vsh.pp.ua";
const char resourceDwld[] = "/"; // "/TinyGSM/logo.txt";
const int portDwld = 80;
HttpClient clienteHTTPS(client, serverDwld, portDwld);
///////////////////////////////////////////////////////




String mydata = "";

String header_01  = "Content-Type: application/json";
String header_02  = "Accept: */*";
String header_03  = "Cache-Control: no-cache";
String header_04  = "Content-Length: ";
String header_05  = "Connection: keep-alive";



//bool f_topic = false;
bool f_topic_Analiz = false;
bool f_topic_Analog = false;
String finalTopic = "";


void makeHeader( const char server[], const char resource[], int fileSize);


void a_TimeSend() {
  a_sendTime = true;
  Serial.println("ALARMA DE TIEMPO DE ENVIO, estado cambiado a: " + String(a_sendTime));
}

void createSendAlarm() {  //configuración de alarma de RTC que avisa de cuándo se cumple el tiempo entre envíos. fija una alrma en el tiempo actual
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

  rtc.setAlarmTime(a_hora, a_min, a_seg);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
  rtc.attachInterrupt(a_TimeSend);

  Serial.println();
  Serial.print("Alarma de tiempo de envío fijada a las: " + String(a_hora) + ":" + String(a_min) + ":" + String(a_seg));
  Serial.print("    Hora actual:" + String(hour(current_timestamp)) + ":" + String(minute(current_timestamp)) + ":" + String(second(current_timestamp)));
  Serial.println();
}



//String DefTopic(String Topic1 = localConfigMQTT.topic1 , String Topic2 = localConfigMQTT.topic2 , String Topic3 = localConfigMQTT.topic3, String Topic4 = localConfigMQTT.topic4)
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
#ifdef TINY_GSM_MQTT
#if defined(TINY_GSM_MODEM_BC95G) //|| defined(TINY_GSM_MODEM_SARAR4)
  //if (modem.mqttConnect(DIR_BROKER_MQTT, MQTT_PORT, DEF_SENSOR_ID , USER_BROKER_MQTT, PASS_BROKER_MQTT, localConfig.APN.c_str() ))
  // if (modem.mqttConnect(DIR_BROKER_MQTT, MQTT_PORT, DEF_SENSOR_ID , USER_BROKER_MQTT, PASS_BROKER_MQTT))

  if (modem.mqttConnect(localConfigMQTT.brokerID.c_str(), localConfigMQTT.port, localConfig.sensorID.c_str(), localConfigMQTT.user.c_str(), localConfigMQTT.pass.c_str()))
  {
    Serial.println("Conectado con Broker MQTT mediante BC95");
    F_BrokerMQTT = true;
    return 0;
  }
  else
    return 10;
#else

  Serial.print("CONECTANDO MEDIANTE MQTT (NO BC95):");
  mqtt.setServer(localConfigMQTT.brokerID.c_str(), localConfigMQTT.port);
  mqtt.setCallback(mqttCallback);

  Serial.print("Conectando con broker MQTT: ");
  Serial.println(localConfigMQTT.brokerID.c_str());

  boolean statusMQTT = mqtt.connect( localConfig.sensorID.c_str(), localConfigMQTT.user.c_str(), localConfigMQTT.pass.c_str());
  delay(300);

  if (statusMQTT == false)
  {
    Serial.println("ERROR al conectar con Broker MQTT!!! ");
    Serial.println(mqtt.state());

    return 10; //ANTES 1
  }
  else
  {
    if (mqtt.connected())
    {
      Serial.println("Conectado con Broker MQTT");
      return 0;
    }
  }
#endif
}
#endif

String extractJSON()
{
  String v_index[NUMITEMS(s_ParamJSON)];

  for (int j = 0; j < NUMITEMS(s_ParamJSON); j++)
  {
    v_index[j] = "Nan";
  }

  int k = 0;

  for (int i = 0; i < NUMITEMS(s_ValJSON); i++)
  {
    if (s_ValJSON[i] != "Nan")
    {
      v_index[k] = i;
      k++;
    }
  }

  String def_ParamJSON[k];
  String def_ValJSON[k];

  for (int q = 0; q < k ; q++)
  {
    String aux = v_index[q];
    def_ParamJSON[q] = s_ParamJSON[v_index[q].toInt()];
    def_ValJSON[q] = s_ValJSON[v_index[q].toInt()];
  }


}


//String buildGenJSON(String *_nombres, String *_valores, int num1, int num2) {
String buildGenJSON() {
  String _output = "";
  int _nParametros = 0;

  //  if (num1 != num2)
  //  {
  //    Serial.println("ERROR AL GENERAR EL JSON! No coinciden los tamaños de los vectores");
  //    err_code += " | x3004 | ";
  //    _error = true;
  //  }
  //  else
  //  {
  //  Serial.print("////////////////////VALJSON0:");
  //  for (int i = 0; i < NUMITEMS(s_ValJSON); i++)
  //  {
  //    Serial.print(s_ValJSON[i]);
  //    Serial.print(";");
  //  }
  //  Serial.println();

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
  //}


  LogJSON = _output;
  return _output;
}





String buildPost(String host, String path, String data) {
  String temporal = "POST ";
  temporal += path + " HTTP/1.1\r\n";
  temporal += "Host: " + host + "\r\n";
  temporal += header_01 + "\r\n";
  //temporal += header_02 + "\r\n";
  //temporal += header_03 + "\r\n";
  temporal += header_04 + String(data.length()) + "\r\n\r\n";
  temporal += data + "\r\n";
  return temporal;
}



String getpet () {

  String peticion = "GET /test.txt HTTP/1.1";
  peticion += "Host: mytest-003.s3.amazonaws.com";
  peticion += header_05 + "\r\n";  //cabecera keepalive
  // peticion += "Accept-Ranges: bytes\r\n"; //comprueba si se pueden realizar solicitudes por rango
  return peticion;
}

uint8_t testBinDwld(const char serv[], int port, const char res[]) { //PRUEBAS DE DESCARGA DE BINARIO =)
  Serial.println("INICIANDO CONSULTA DE ACTUALIZACION...");
  Serial.println("Intentando conectar con: " + String(serv) + "   Mediante puerto: " + port + " ...");

  File outFile;

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin();

  delay(10);

  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card Error!");
    delay(1000);
    err_code += "| x4000 |";
    _error = true;
    SPI.end();
    pinMode(SD_CS, INPUT_PULLDOWN);
    placa.SetPrevStatus();
  }

  client.connect(serv, port, 10000);
  delay(1);

  client.println(getpet());

  unsigned long t_ = millis();

  outFile = SD.open("prueba.txt", FILE_WRITE);

  Serial.println("Generando nuevo archivo: " + String("prueba.txt"));

  uint8_t datas[1024];

  while (millis() - t_ < 180000) {
    outFile.write(client.read());
  }

  outFile.close();

  Serial.println();
  Serial.println("ARCHIVO COMPLETADO, INICIANDO IMPRESION:");
  delay(3000);

  outFile = SD.open("prueba.txt", FILE_READ);

  while (outFile.available()) {

    Serial.write(outFile.read());
  }


  outFile.close();

  delay(1000);
  Serial.println("FIN");
  placa.DisableMIKRO();
  while (true);
}


void printPercent(uint32_t readLength, uint32_t contentLength) { // PRUEBAS CON CRC
  // If we know the total length
  if (contentLength != -1) {
    Serial.println("");
    Serial.print("\r ");
    Serial.print((100.0 * readLength) / contentLength);
    Serial.print('%');
    Serial.println("");
  } else {
    Serial.println("");
    Serial.println(readLength);
    Serial.println("");
  }
}


uint8_t testBinDwld2(const char serv[], int port, const char res[]) { //PRUEBAS CON CRC =)

  Serial.println("INICIANDO CONSULTA DE ACTUALIZACION...");
  Serial.println("Intentando conectar con: " + String(serv) + "   Mediante puerto: " + port + " ...");

  client.connect(serv, port);

  delay(1);

  Serial.println("REALIZANDO SOLICITUD GET");

  client.println(getpet());

  Serial.println("Get finalizado");

  long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000L) {
      Serial.println(">>> Client Timeout !");
      client.stop();

      while (true);
    }
  }

  Serial.println("Reading response header");
  uint32_t contentLength = 0;


  while (client.available()) {
    String line = client.readStringUntil('\n');
    line.trim();
    Serial.println(line);    // Uncomment this to show response header
    line.toLowerCase();
    if (line.startsWith("content-length:")) {
      contentLength = line.substring(line.lastIndexOf(':') + 1).toInt();
    }
    else if (line.length() == 0) {
      break;
    }
  }

  Serial.println("Reading response data");
  timeout = millis();
  uint32_t readLength = 0;
  CRC32 crc;

  printPercent(readLength, contentLength);
  unsigned long timeElapsed = millis();


  while (readLength < contentLength && client.connected() && millis() - timeout < 60000L) {
    while (client.available()) {
      yield();
      uint8_t c = client.read();
      //Serial.print((char)c);       // Uncomment this to show data
      crc.update(c);
      readLength++;
      //      if (readLength % (contentLength / 13) == 0) {
      //        printPercent(readLength, contentLength);
      //      }

      timeout = millis();
    }
  }

  if (readLength == contentLength)
    Serial.println("ContentLength=readlength <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  else if ( !client.connected())
    Serial.println("Client disconnected <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  else if (millis() - timeout > 60000L)
    Serial.println("TIMEOUT <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");

  Serial.println();
  Serial.println();
  printPercent(readLength, contentLength);
  timeElapsed = millis() - timeElapsed;
  Serial.println();

  // Shutdown

  client.stop();
  Serial.println(F("Server disconnected"));

  float duration = float(timeElapsed) / 1000;

  Serial.println();
  Serial.print("Content-Length: ");   Serial.println(contentLength);
  Serial.print("Actually read:  ");   Serial.println(readLength);
  Serial.print("Calc. CRC32:    0x"); Serial.println(crc.finalize(), HEX);
  Serial.print("Duration:       ");   Serial.print(duration); Serial.println("s");

  Serial.println("FIN");
  placa.DisableMIKRO();
  while (true);
}



//
//uint8_t checkActu(const char serv[], int port, const char res[]) { //Consulta si existe actualización mediante HTTPS =)
//  Serial.println("INICIANDO CONSULTA DE ACTUALIZACION...");
//  Serial.println("Intentando conectar con: " + String(serv) + "   Mediante puerto: " + port + " ...");
//
//  clienteHTTPS.connectionKeepAlive();
//
//  clientSSL.connect(serv, port, 10000);
//  delay(1);
//
//  clientSSL.println(getpet());
//
//  unsigned long t_ = millis();
//String  response;
//  while (millis() - t_ < 60000) {
//    if (clientSSL.available()) {
//      Serial.println("Leyendo respuesta...");
//      response += clientSSL.readString();
//
//
//     File   outFile = SD.open("prueba.txt", FILE_WRITE);
//
//        Serial.println("Generando nuevo archivo: " + String("prueba.txt"));
//        if (!outFile) {
//          Serial.println("Error generando el archivo para escritura:" + String(nameFile));
//          err_code += "| x4104 |";
//          _error = true;
//          return 2;
//        } else { //Si se genera
//          outFile.println(body);
//          outFile.println();
//          Serial.println("Se ha generado el fichero:" + String(nameFile));
//          outFile.close();
//
//          return 0;
//        }
//
//      int errHTTPS = clienteHTTPS.get(res);
//
//      if (errHTTPS != 0) {
//        Serial.println("Error al conectar mediante HTTPS con:" + String(serv));
//        return 1;
//      }
//
//      int status = clienteHTTPS.responseStatusCode();
//      Serial.println("Respuesta del servidor: " + status);
//      if (!status) {
//        return 2;
//      }
//
//      String body = clienteHTTPS.responseBody();
//      Serial.println("Response:");
//      Serial.println(body);
//
//      if (body == "TRUE")
//        return 1;
//      else
//        return 0;
//    }


String downloadFileActu() { //Descarga el archivo de actualizacion mediante HTTPS =)
  //CONEXION CLIENTE WEB
  Serial.println("INICIANDO DESCARGA DE ARCHIVO \"UPDATE.BIN\"");
  Serial.println("Intentando conectar con: " + String(serverDwld) + "   Mediante puerto: " + portDwld + " ...");

  clienteHTTPS.connectionKeepAlive();
  int errHTTPS = clienteHTTPS.get(resourceDwld);

  if (errHTTPS != 0) {
    Serial.println("Error al conectar mediante HTTPS con:" + String(serverDwld));
    return "1";
  }

  int status = clienteHTTPS.responseStatusCode();
  Serial.println("Respuesta del servidor: " + status);
  if (!status)
    return "2";

  Serial.println("Cabeceras de respuesta");

  while (clienteHTTPS.headerAvailable()) {
    String headerName = clienteHTTPS.readHeaderName();
    String headerValue = clienteHTTPS.readHeaderValue();
    Serial.println("    " + headerName + " : " + headerValue);
  }

  int length = clienteHTTPS.contentLength();
  if (length >= 0) {
    Serial.print("Content length is: ");
    Serial.println(length);
  }
  if (clienteHTTPS.isResponseChunked()) {
    Serial.println("The response is chunked");
  }

  String body = clienteHTTPS.responseBody();

  Serial.println("Body length: ");
  Serial.println(body.length());

  Serial.println("Response:");
  Serial.println(body);

  clienteHTTPS.stop();
  Serial.println("Cliente HTTPS desconectado");

  return body;
}




int parseResponse(bool SSL = false) {
  unsigned long _timeout = millis();
  String response = "";
  int idx1, idx2 = 0 ;
  int out = 0;

  if (SSL == false) {
    while (( millis() - _timeout < 10000)) {
      if (client.available()) {
        response += client.readString();
        break;
      }
    }
  } else {
    while (( millis() - _timeout < 60000)) {
      if (clientSSL.available()) {
        Serial.println("Leyendo respuesta...");
        response += clientSSL.readString();
        Serial.println(response);
        break;
      }
    }
  }
  if (response.length() > 0) {
    idx1 = response.indexOf(' ');
    idx2 = response.indexOf(' ', idx1 + 1);
    out = response.substring(idx1, idx2).toInt();
  }
  Serial.println(out);
  Serial.println(response);
  return out;
}


#ifdef TINY_GSM_MQTT
int EnviaMQTT(String MessageToSend)
{
  finalTopic = DefTopic(localConfigMQTT.topic1 , localConfigMQTT.topic2 , localConfigMQTT.topic3, localConfigMQTT.topic4);

#if defined(TINY_GSM_MODEM_BC95G) //|| defined(TINY_GSM_MODEM_SARAR4)
  Serial.println("Enviando mensaje al broker");

  if (modem.mqttSend(finalTopic.c_str(), MessageToSend.c_str()))
  {
    current_timestamp = rtc.getEpoch();
    SendTime = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));

    a_sendTime = false;
    //  Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>estado de flag cambiado a: "+String(a_sendTime));
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

#else
  mqtt.publish(finalTopic.c_str(), MessageToSend.c_str());
  //mqtt.subscribe("#");
  Serial.println("Paquete MQTT enviado a topic : " + finalTopic + "  ||  Con el mensaje: " + String(MessageToSend));
  delay(150);
  Serial.println(mqtt.state());
  if (mqtt.state() == 0)
  {
    current_timestamp = rtc.getEpoch();
    SendTime = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));

    a_sendTime = false;
    //      Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>estado de flag cambiado a: "+String(a_sendTime));
    createSendAlarm();

    Serial.println("Tiempo de envio actualizado");
  }
  else
  {
    return 11;
    mqtt.disconnect();
  }
  mqtt.disconnect();

  if (localConfig.f_ssl)
    clientSSL.stop();
  else
    client.stop();
#endif

  return 0;
}
#endif

String TakeTime()
{

  int _res;
  String timestamp = "NoInit_1";
  if (modem.isNetworkConnected())
  {
    Serial.println("");
    Serial.println("Obteniendo fecha y hora...");

#ifdef TINY_GSM_MODEM_UBLOX
    timestamp = modem._TakeTime();
    if (timestamp == "Error1!") {
      Serial.println("Error al actualizar la fecha con UBLOX SARA U201, comprobar conexión");
      err_code += "| x3101 |";
      _error = true;
    }
#endif
#ifdef TINY_GSM_MODEM_SIM808
    timestamp = modem._TakeTime();
    if (timestamp == "e1" || timestamp == "Error1!") {
      Serial.println("Error al actualizar la fecha con SIM808, comprobar conexión");
      err_code += "| x3501 |";
      _error = true;
    }
#endif

#ifdef TINY_GSM_MODEM_ESP32
    _res = modem._ActuTime(); //SET THE TIME ZONE
    if (_res != 1) {
      Serial.println("Error al establecer la franja horaria con ESP32");
      err_code += "| x3202 |";
      _error = true;
    }
    else {
      Serial.println();
      delay(350);

      if (SerialM1.available())
        timestamp = modem._TakeTime();

      if (timestamp == "Error1!" || timestamp == "Error2!") {
        Serial.println("Error al actualizar la fecha con ESP32, comprobar conexión");
        err_code += "| x3201 |";
        _error = true;
      }
    }
#endif

#if defined(TINY_GSM_MODEM_SARAR4) || defined(TINY_GSM_MODEM_SARAR5)
    timestamp = modem._TakeTime();
    if (timestamp == "Error1!") {
      Serial.println("Error al actualizar la fecha con SARAR4, comprobar conexión");
      err_code += "| x3301 |";
      _error = true;
    }
#endif

#ifdef TINY_GSM_MODEM_BC95G
    timestamp = modem._TakeTime();
    if (timestamp == "Error1!") {
      Serial.println("Error al actualizar la fecha con BC95G, comprobar conexión");
      err_code += "| x3401 |";
      _error = true;
    }
#endif

    if ((((timestamp.substring(0, 4)).toInt()) < 2021 || ((timestamp.substring(0, 4)).toInt()) > 2050)  && f_cont < MAX_RETRAY)
    {
      f_cont++;
      Serial.println("Fecha no obtenida o no valida, reintento:" + String(f_cont));
      timestamp = "NoInit_2";
    }

    else if (f_cont == MAX_RETRAY)
    {
      Serial.println("ERROR AL TOMAR VALOR DE FECHA");
      err_code += "| x3X01 |";
      _error = true;
      f_cont = 0;
    }

    //f_cont = 0;
  }
  return timestamp;
}




int EnviaMIKROE(String MessageToSend)
{
  Serial.println("Preparando envío, SSL: " + String(localConfig.f_ssl));
#ifdef TINY_GSM_MQTT   //SI EL ENVIO ES POR PROTOCOLO MQTT

  if (ConectaMQTT() == 10)
  {
    return 10;
  }
  else
  {
    if (EnviaMQTT(MessageToSend) == 11)
      return 11;
  }

#else//CONEXION CLIENTE WEB   
  Serial.println("CONECTANDO COMO CLIENTE WEB");
  Serial.println("Intentando conectar con: " + localConfig.server + "   Mediante puerto: " + localConfig.port + " ...");

  if (localConfig.f_ssl == true)
  {
    if (clientSSL.connect(localConfig.server.c_str(), localConfig.port, 10000)) {
      Serial.println("Conectado con: " + localConfig.server);
      Serial.println("Preparando para enviar: \r\n" + buildPost(localConfig.server, localConfig.path, MessageToSend));

      clientSSL.println(buildPost(localConfig.server, localConfig.path, MessageToSend));

      if (parseResponse(localConfig.f_ssl) != 200) {
        clientSSL.stop();
        Serial.println("ERROR AL ENVIAR EL MENSAJE");
        return 5;
      }
      else {
        clientSSL.stop();
        Serial.println("El mensaje ha sido enviado!");
        //  if(localConfig.waitTime>=60)
        //     placa.DisableMIKRO();

        current_timestamp = rtc.getEpoch();
        SendTime = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));
        a_sendTime = false;
        //          Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>estado de flag cambiado a: "+String(a_sendTime));
        createSendAlarm();

        Serial.println("Tiempo de envio actualizado");
      }
    }
    else {
      Serial.println(localConfig.server + " not Connected");
      return 4; //ANTES 3
    }
  }
  else //SSL FALSE (HTTP)
  {
    if (client.connect(localConfig.server.c_str(), localConfig.port, 10000)) {
      Serial.println("Conectado con: " + localConfig.server);
      Serial.println("Preparando para enviar: \r\n" + buildPost(localConfig.server, localConfig.path, MessageToSend));

      client.println(buildPost(localConfig.server, localConfig.path, MessageToSend));

      if (parseResponse(localConfig.f_ssl) != 200) {
        client.stop();
        Serial.println("ERROR AL ENVIAR EL MENSAJE");
        return 3;
      }
      else {
        client.stop();
        Serial.println("El mensaje ha sido enviado!");
        //  if(localConfig.waitTime>=60)
        //    placa.DisableMIKRO();

        current_timestamp = rtc.getEpoch();
        SendTime = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));
        a_sendTime = false;
        //          Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>estado de flag cambiado a: "+String(a_sendTime));
        createSendAlarm();

        Serial.println("Tiempo de envio actualizado");
      }
    }
    else {
      Serial.println(localConfig.server + " not Connected");
      return 2; //return 1;
    }
  }
#endif
  return 0;
}



int uploadMIKROE(const char nameFileToSend[], char protocolo, const char server[], const char resource[] = NULL , const int securePort = 443) //ENVIO DE ARCHIVOS (M:MQTT, H:HTTP, S:HTTPS, F:FTTP)        PARA AWS: resource:/NOMBRERECURSOAPI/NOMBREBUCKET/NOMBREARCHIVO CON EXTENSION
{
  File fileToSend;
  char bufferSend[TINY_GSM_RX_BUFFER]; //buffer de envío HTTP/S
  int fileSize = 0;
  int i = 0;

  // uint8_t err_findFile = findFile(nameFileToSend, fileToSend);

  //  if (err_findFile != 0) {
  //    Serial.println("ERROR AL BUSCAR EL ARCHIVO EN LA SD");
  //    ////////CREAR NUEVO ERROR DE NO SE PUDO ENVIAR EL ARCHIVO
  //    return 1 ;
  //  }

#ifdef DEBUGCODE
  Serial.print("Preparando envío de archivo, Protocolo: ");
#endif

  switch (protocolo) {
    case 'M': //ENVIO DE ARCHIVO POR MQTT
#ifdef DEBUGCODE
      Serial.println("[MQTT]");
      Serial.println("AUN NO SE HA CONFIGURADO SUBIDA DE ARCHIVOS POR MQTT,ABORTANDO OPERACION");
#endif

      break;
    case 'H': //ENVIO DE ARCHIVO POR HTTP
#ifdef DEBUGCODE
      Serial.println("[HTTP]");
      Serial.println("CONECTANDO COMO CLIENTE WEB");
      Serial.println("Intentando conectar con: " + String(server) + "   Mediante puerto: 80  ...");
#endif
      break;
    case 'S': //ENVIO DE ARCHIVO POR HTTPS
#ifdef DEBUGCODE
      Serial.println("[HTTPS]");
      Serial.println("CONECTANDO COMO CLIENTE WEB");
      Serial.println("Intentando conectar con: " + String(server) + "   Mediante puerto: " + String(securePort));
#endif
      fileSize = fileToSend.size();

      if (clientSSL.connect(server, securePort, 10000)) {
        Serial.println();
        Serial.println("Conectado con: " + String(server) + " | Enviando cabeceras");

        makeHeader(server, resource, fileSize);

        //        Serial.println("Enviando contenido del archivo...");
        //
        //        while (fileToSend.available()) {
        //          while ((i < TINY_GSM_RX_BUFFER - 2) && fileToSend.available()) //SIN EL -2 EL BUFFER LLEGA A 1027
        //          {
        //            bufferSend[i] = fileToSend.read();
        //            i++;
        //          }
        //          clientSSL.print(bufferSend);
        //          memset(bufferSend, 0, TINY_GSM_RX_BUFFER);
        //          i = 0;
        //        }
        //
        //        fileToSend.close();
        //        SPI.end();
        //        pinMode(SD_CS, INPUT_PULLDOWN);

        Serial.println("ESPERANDO RESPUESTA");
        Serial.println("");
        if (parseResponse(true) != 200) {
          clientSSL.stop();
          Serial.println("ERROR AL ENVIAR EL ARCHIVO");
          return 5;
        }
        else {
          clientSSL.stop();
          Serial.println("El archivo ha sido enviado!");
        }
      }
      else {
        Serial.println(String(server) + " not Connected");
        return 4;
      }

      break;
    case 'F': //ENVIO DE ARCHIVO POR FTP
#ifdef DEBUGCODE
      Serial.println("[FTP]");
      Serial.println("AUN NO SE HA CONFIGURADO SUBIDA DE ARCHIVOS POR FTP,ABORTANDO OPERACION");
#endif
      return 1;
      break;
    default:
#ifdef DEBUGCODE
      Serial.print("ERROR EN LA SELECCION DEL PROTOCOLO, ABORTANDO");
#endif
      return 1;
  }



  //  else //SSL FALSE (HTTP)
  //  {
  //    if (client.connect(localConfig.server.c_str(), localConfig.port, 10000)) {
  //      Serial.println("Conectado con: " + localConfig.server);
  //      Serial.println("Preparando para enviar: \r\n" + buildPost(localConfig.server, localConfig.path, MessageToSend));
  //
  //      client.println(buildPost(localConfig.server, localConfig.path, MessageToSend));
  //
  //      if (parseResponse(localConfig.f_ssl) != 200) {
  //        client.stop();
  //        Serial.println("ERROR AL ENVIAR EL MENSAJE");
  //        return 3;
  //      }
  //      else {
  //        client.stop();
  //        Serial.println("El mensaje ha sido enviado!");
  //        //  if(localConfig.waitTime>=60)
  //        //    placa.DisableMIKRO();
  //
  //        current_timestamp = rtc.getEpoch();
  //        SendTime = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));
  //        a_sendTime = false;
  //        //          Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>estado de flag cambiado a: "+String(a_sendTime));
  //        createSendAlarm();
  //
  //        Serial.println("Tiempo de envio actualizado");
  //      }
  //    }
  //    else {
  //      Serial.println(localConfig.server + " not Connected");
  //      return 2; //return 1;
  //    }
  //  }
  //  return 0;
}



void initComPort() {
#ifdef TINY_GSM_MODEM_ESP32
  pinMode(M1_AN, INPUT_PULLDOWN);
  pinMode(M1_RST, OUTPUT);
  pinMode(M1_CS, OUTPUT);
  pinMode(M1_PWM, INPUT_PULLDOWN);
  pinMode(M1_INT, INPUT_PULLDOWN);

  digitalWrite(M1_RST, LOW);
  digitalWrite(M1_CS, LOW);
#endif
#ifdef TINY_GSM_MODEM_UBLOX
  pinMode(M1_AN, INPUT);
  pinMode(M1_RST, OUTPUT);
  pinMode(M1_CS, INPUT);
  pinMode(M1_PWM, INPUT_PULLDOWN);
  pinMode(M1_INT, INPUT_PULLDOWN);

  digitalWrite(M1_RST, LOW);
  //  digitalWrite(M1_CS,LOW);
#endif
#ifdef TINY_GSM_MODEM_SIM808
  pinMode(M1_AN, INPUT_PULLDOWN);
  pinMode(M1_RST, OUTPUT);
  digitalWrite(M1_RST, HIGH);
  pinMode(M1_CS, INPUT_PULLDOWN);
  pinMode(M1_PWM, INPUT_PULLDOWN);
  pinMode(M1_INT, INPUT_PULLDOWN);
#endif

#ifdef TINY_GSM_MODEM_SARAR4   //FUNCIONA BIEN
  pinMode(M1_AN, INPUT_PULLDOWN);
  pinMode(M1_RST, OUTPUT);
  digitalWrite(M1_RST, HIGH);
#endif

#ifdef TINY_GSM_MODEM_SARAR5
  pinMode(M1_AN, INPUT_PULLDOWN);
  pinMode(M1_RST, OUTPUT);
  digitalWrite(M1_RST, LOW);
  pinMode(M1_INT, INPUT);
#endif

#ifdef TINY_GSM_MODEM_BC95G
  pinMode(M1_AN, INPUT_PULLDOWN);
  pinMode(M1_RST, OUTPUT);
  digitalWrite(M1_RST, LOW);
#endif


#ifdef TINY_GSM_MODEM_NRF9160
  pinMode(M1_RST, OUTPUT);
  digitalWrite(M1_RST, HIGH);
  pinMode(M1_AN, INPUT_PULLDOWN);

#endif



  //MIKROE M2
#ifdef ETH_MODEM_W5500
  pinMode(M2_INT, INPUT);
  digitalWrite(M2_INT, INPUT_PULLUP);
  pinMode(M2_RST, OUTPUT);
  digitalWrite(M2_RST, HIGH);
  pinMode(M2_CS, OUTPUT);
  digitalWrite(M2_CS, LOW);
  Serial.println("modulo Ethernet configurado");
#endif

}

int startComPort() {
  unsigned long _timeout = millis();
#ifdef TINY_GSM_MODEM_UBLOX
  digitalWrite(M1_RST, HIGH);
  if (!modem.isNetworkConnected()) {
    while (digitalRead(M1_AN) == 0);
    Serial.println("Reset");
    modem.restart();
    Serial.println("Conectando");
    if (modem.gprsConnect(localConfig.APN.c_str(), localConfig.APNuser.c_str(), localConfig.APNpass.c_str()))
    {
      Serial.println("Esperando a la red....");
      if (!modem.waitForNetwork()) {
        err_com++;
        contFallos++;
        err_code += "| x3101 |";
        _error = true;
        Serial.println("Error al conectar con la red, tiempo de espera superado");
        //  Serial.println(millis() - _timeout);
      } else Serial.println("Red Encontrada");

      if (modem.isNetworkConnected())
        Serial.println("Conectado!!");

      err_com = 0;

    } else {
      err_com++;
      contFallos++;
      err_code += "| x3100 |";
      _error = true;
      Serial.println("UBLOX No se ha podido conectar a la red GPRS");
      return 1;
    }
  }
#endif
#ifdef TINY_GSM_MODEM_SIM808
  digitalWrite(M1_RST, HIGH);
  if (!modem.isNetworkConnected()) {
    while ((digitalRead(M1_AN) == 0) && ((millis() - _timeout) < 10000));
    if (digitalRead(M1_AN) == 0) {
      Serial.println("fail");
      Serial.println(millis() - _timeout);
      return 2;
    }
    modem.restart();
    if (modem.gprsConnect(localConfig.APN.c_str(), localConfig.APNuser.c_str(), localConfig.APNpass.c_str()))
    {
      if (!modem.waitForNetwork()) {
        err_com++;
        contFallos++;
        err_code += "| x3501 |";
        _error = true;
        Serial.println("Error al conectar con la red, tiempo de espera superado");
        Serial.println(millis() - _timeout);
      } else Serial.println("Red Encontrada");

      if (modem.isNetworkConnected())
        Serial.println("Conectado!!");

      err_com = 0;

    } else {
      err_com++;
      contFallos++;
      err_code += "| x3500 |";
      _error = true;
      Serial.println("SIM808 No se ha podido conectar a la red GPRS");
      return 1;
    }
  }
#endif
#ifdef  TINY_GSM_MODEM_ESP32
  digitalWrite(M1_RST, HIGH);
  digitalWrite(M1_CS, HIGH);
  delay(1000);
  if (!modem.isNetworkConnected()) {
    modem.restart();
    modem.setTxPwr(rfPower);
    delay(1000);
    String nearestMAC = modem.searchNearestAp(localConfig.ssid.c_str());
    if (!modem.networkConnect(localConfig.ssid.c_str(), localConfig.password.c_str(), nearestMAC.c_str())) {
      err_com++;
      contFallos++;
      err_code += "| x3200 |";
      _error = true;
      Serial.println("ESP32 No se ha podido conectar a la red ");
      if ( rfPower != 0)rfPower--;
      return 1;
    }
    if (!modem.waitForNetwork()) {
      if ( rfPower != 0)rfPower--;
      err_com++;
      contFallos++;
      err_code += "| x3201 |";
      _error = true;
      Serial.println("Error al conectar con la red, tiempo de espera superado");
      Serial.println(millis() - _timeout);
      return 1;
    } else Serial.println("Network connected");

    err_com = 0;
    //if (modem.ping("8.8.8.8")== 0)
  }
#endif
#ifdef  TINY_GSM_MODEM_BC95G
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
        err_code += "| x3401 |";
        _error = true;
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
      err_code += "| x3400 |";
      _error = true;
      Serial.println("Conexión con la red no establecida");
      return 1;
    }


  }
#endif

#ifdef  TINY_GSM_MODEM_SARAR4
  digitalWrite(M1_RST, HIGH);
#endif
#if defined(TINY_GSM_MODEM_SARAR4) || defined(TINY_GSM_MODEM_SARAR5)

  delay(4000); // en ocasiones el CEREG? no responde correctamente los comandos AT del SARAR4 tardan algo en inicializarse
  if (!modem.isNetworkConnected())
  {
    modem.restart();

    String modemInfo = modem.getModemInfo();
    Serial.println("ModemInfo: ");
    Serial.println(modemInfo);

    if (!modem.hasSSL())
    {
      Serial.println("SSL is not supported by this modem!! ERROR");
      err_code += "| x3003 |";
      _error = true;
    }

    if ( modem.gprsConnect(localConfig.APN.c_str(), localConfig.APNuser.c_str(), localConfig.APNpass.c_str()))
    {

      Serial.println("Esperando a la red.....");

      if (!modem.waitForNetwork()) {
        err_com++;
        contFallos++;
        err_code += "| x3301 |";
        _error = true;
        Serial.println("Error al conectar con la red, tiempo de espera superado");
        Serial.println(millis() - _timeout);
      }
      else
        Serial.print("Red encontrada...");

      if (modem.isNetworkConnected())
        Serial.println("Conectado!!");

      err_com = 0;
    }
    else
    {
      err_com++;
      contFallos++;
      err_code += "| x3300 |";
      _error = true;
      Serial.println("SARAR4 No se ha podido conectar a la red GPRS");
      return 1;
    }
  }
#endif
  return 0;
}


String ModbusEthernet() {
  err_analiz = "0"; //codigo de error de los analizadores

  SPI.begin();
  delay(10);
  Serial.println("Iniciando conexión Ethernet...");
  //Ethernet.begin(mac, industruino_ip);
  Ethernet.init(M2_CS);
  Ethernet.begin(Mac_Master_RJ45, IP_Master_RJ45);
  Serial.println("Iniciando configuración Ethernet, con IP:" + String(Ethernet.localIP()));

  //timeouts para conexión ethernet
  // client_modbus_RJ45.setTimeout(1000);
  // client_tcp_RJ45.setTimeout(1000);
  // modbusTCPClient.setTimeout(1000);

  //   if (!modbusTCPClient.connected()) {
  //     // modbusTCPClient.stop();
  //      // client not connected, start the Modbus TCP client
  //      Serial.println("Cliente Modbus no conectado, intentando conectarse a SiemensPAC");

  if (!modbusTCPClient.begin(RJ45IP_addr[n_Analiz], localConfig.portRJ45)) {
    Serial.println("ERROR modbus, no se ha podido establecer la comunicación con el analizador: " + String(n_Analiz + 1) + " con IP: " + String(RJ45IP_addr[n_Analiz]));
    err_analiz = String((0x2106 + (n_Analiz + 1, HEX)), HEX);

  } else {
    // modbusTCPClient.setTimeout(1000);  // no es necesario
    if (modbusTCPClient.connected()) {
      Serial.println("Conectado a: " + String(RJ45IP_addr[n_Analiz]));
      //tiempopasado = millis();//TESTXAVI
    }
    else {
      Serial.println("Error conectando con: " + String(RJ45IP_addr[n_Analiz]));
      err_code += "| x2106 |";
      _error = true;
    }
  }
  //    } else
  //      Serial.println("Modbus OK");


  if (modbusTCPClient.connected()) {  // otherwise read gets stuck
    GET_VALUERJ45();
  }
  modbusTCPClient.stop();

  return err_analiz;
}



int envioDatosAnalogMuutech() {
#if defined(JSON_ANALOG003) || defined(JSON_ANALOG005)

  Serial.print("Tiempo desde el último envío : " );
  current_timestamp = rtc.getEpoch();
  sum_SecAct = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));

  Serial.println(String( sum_SecAct - SendTime) + "  s.");
  Serial.println("Tiempo entre envíos: " + String(_sendTime) + "  s.");


  //if ( sum_SecAct - SendTime >= _sendTime) {
  if (a_sendTime) {
    unsigned long _timeout = millis();
    if (modem.isNetworkConnected()) {

      retorno = EnviaMIKROE(buildGenJSON());
      return retorno;

    } else
      return 1; //NO HAY COMUNICACION CON LA RED
  }
  else {
    Serial.print("Envío suspendido hasta cumplir tiempo| Tiempo esperado: " );
    Serial.print(_sendTime);
    Serial.print(" s.  Tiempo trascurrido: ");
    Serial.println(( sum_SecAct - SendTime));
    return 99;
  }
#else
  Serial.println("No ha sido definido un JSON para los sensores analógicos");
#endif
}

int envioDatosRS485Muutech(String JSONENVIO) {
  current_timestamp = rtc.getEpoch();
  sum_SecAct = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));

  Serial.print("Tiempo desde el último envío: " );
  Serial.println(( sum_SecAct - SendTime));
  Serial.print("Tiempo entre envíos: ");
  Serial.println(String(_sendTime) + "  s.");

  //if ( sum_SecAct - SendTime >= _sendTime) {
  if (a_sendTime) {
    //  unsigned long _timeout=millis();

    if (modem.isNetworkConnected()) {
      retorno = EnviaMIKROE(JSONENVIO);
      return retorno;

    } else
    {
      Serial.println("---------------------------------------------DISPOSITIVO NO CONECTADO A LA RED "); //NO RECUERDO ORIGEN DE LOS TEST ,PARECE QUE FALTAN FILTROS DE MODULO Y TIPO DE COMUNICACIONES, NO RECUERDO LA NECESIDAD DE ESTE ELSE

      placa.EnableMIKRO();//!!!!TEST
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




String buildGenGET_Header ( String path, String host, String port, int app) {
  String temporal = "GET ";
  if ( app == 0) {
    temporal += path;
    temporal += "?api_key=0963d83d-5f46-4b24-b453-fc7ec584f671";
    temporal += "&SENSOR=1";
    temporal += "&AUX1=60.23";
    temporal += "&AUX2=42.23";
    temporal += "&AUX3=0.16";
    temporal += " HTTP/1.1\r\n";
    temporal += "Host: " + host + ":" + port + "\r\n";
  } else {
    temporal += path;
    temporal += " HTTP/1.1\r\n";
    temporal += "Host: " + host + "\r\n";
  }


  return temporal;
}


//CONFIGURACION DE PETICION
String UrlstringMuutech(String _path, String _host, String _sensorID, float Data_Meter [Total_Reg], String Inf_Alarm)
{
  String  temp = "GET ";
  temp += _path;//"/astican.php";
  temp += "?hostname=" + _sensorID;//NombreDisp;
  temp += "&" + param01 + "=" + String(Data_Meter [0]);
  temp += "&" + param02 + "=" + String(Data_Meter [1]);
  temp += "&" + param03 + "=" + String(Data_Meter [2]);
  temp += "&" + param04 + "=" + String(Data_Meter [3]);
  temp += "&" + param05 + "=" + String(Data_Meter [4]);
  temp += "&" + param06 + "=" + String(Data_Meter [5]);
  temp += "&" + param07 + "=" + String(Data_Meter [6]);
  temp += "&" + param08 + "=" + String(Data_Meter [7]);
  temp += "&" + param09 + "=" + String(Data_Meter [8]);
  temp += "&" + param10 + "=" + String(Inf_Alarm);
  temp += " HTTP/1.1\r\n";
  temp += "Host: " + _host + "\r\n";
  String urlmuutech = (peticion + NombreDisp + "&" + param01 + "=" + String(Data_Meter [0]) + "&" + param02 + "=" + String(Data_Meter [1]) + "&" + param03 + "=" + String(Data_Meter [2]) + "&" + param04 + "=" + String(Data_Meter [3]) + "&" + param05 + "=" + String(Data_Meter [4]) + "&" + param06 + "=" + String(Data_Meter [5]) + "&" + param07 + "=" + String(Data_Meter [6]) + "&" + param08 + "=" + String(Data_Meter [7]) + "&" + param09 + "=" + String(Data_Meter [8]) + "&" + param10 + "=" + String(Inf_Alarm));

  return temp;
}


int EnvioMuutech(float DATA_METER[], String textoAlarma)
{
  current_timestamp = rtc.getEpoch();
  sum_SecAct = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));

  //if ( sum_SecAct - SendTime >= _sendTime) {
  if (a_sendTime) {

    Serial.println(UrlstringMuutech(localConfig.path, localConfig.server, localConfig.sensorID, DATA_METER, textoAlarma).c_str());
    //    unsigned long _timeout = millis();
    if (modem.isNetworkConnected()) {

      retorno = EnviaMIKROE(UrlstringMuutech(localConfig.path, localConfig.server, localConfig.sensorID, DATA_METER, textoAlarma).c_str());
      return retorno;

    } else return 1;
  }
  else {
    Serial.print("Envío suspendido hasta cumplir tiempo - Tiempo esperado: " );
    Serial.print(_sendTime);
    Serial.print("  s. Tiempo trascurrido: ");
    Serial.println(( sum_SecAct - SendTime));
    return 99;
  }
}




int sendGET_generic(String & _Server, String & _Port, String & _path, String & local_body , int app) {
  current_timestamp = rtc.getEpoch();
  sum_SecAct = ((hour(current_timestamp) * 3600) + (minute(current_timestamp) * 60) + second(current_timestamp));

  //if ( sum_SecAct - SendTime >= _sendTime) {
  if (a_sendTime) {
    // int retry = 1;
    String response_status = "-1";
    String response_header = "";
    local_body = "";

    if (clientSSL.connect(_Server.c_str(), _Port.toInt(), 10000)) {
      Serial.println("Client connected");
      // Make a HTTP request:
      Serial.println(buildGenGET_Header( _path, _Server, _Port, app));
      //Serial.println(data);
      Serial.println();
      clientSSL.println(buildGenGET_Header( _path, _Server, _Port , app));
      //client.println(data);
      clientSSL.println();

      unsigned long mytimerwait = millis();
      while (( millis() - mytimerwait < 10000) && !(response_header.endsWith("\r\n\r\n")) ) {
        if (clientSSL.available()) {
          response_header += (char) clientSSL.read();
        }
      }
      Serial.println(response_header);
      mytimerwait = millis();
      while (( millis() - mytimerwait < 10000) && !(local_body.endsWith("\r\n\r\n")) ) {
        if (clientSSL.available()) {
          local_body += (char) clientSSL.read();
        }
      }
      Serial.println(local_body);
      int index = response_header.indexOf("OK\r\n");
      response_status = response_header.substring(0, index - 1);
      local_body = local_body.substring(local_body.indexOf('{'), local_body.indexOf('}') + 1);
    } else {
      Serial.println("Connection refused");
      return -1;
    }
    return response_status.substring(9).toInt();
  }
  else {
    Serial.print("Envío suspendido hasta cumplir tiempo - Tiempo esperado: " );
    Serial.print(_sendTime);
    Serial.print("  s.  Tiempo trascurrido: ");
    Serial.println(( sum_SecAct - SendTime));
    return 4;
  }
}

#if defined(TINY_GSM_MODEM_ESP32)
#else
void startFTPFile() { //ftp1
  IPAddress servidorFTP (3, 209, 73, 20);     //(35, 163, 228, 146) //(212,227,247,203)
  const char *userFTP = "X1test";         //"dlpuser" //"u47750976-ssm-tech "
  const char *passFTP = "prueba.tech.1";     //"rNrKYTX9g7z3RgJRmxWuGHbeu" //"Qwertyui.2022"
  const char *fileNameFTP = "ArchivoDesdeSD(28-01-2022).txt";
  const uint16_t portFTP = 21;
  delay(1000);

  File outFile;

  String filename_ = "textX1FTP.txt";

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
    outFile = SD.open(filename_, FILE_READ );

    int fileSize = outFile.fileSize();

    if (!ftp.connect(servidorFTP, portFTP, userFTP, passFTP)) {
      Serial.println("Error connecting to FTP server");
      while (true);
    }
    delay(1000);

    Serial.println("Preparando envío");

    int sizeFileSend = ftp.store(fileNameFTP, outFile);

    ftp.stop();
    outFile.close();

    Serial.println("FileSize: " + String(fileSize));
    Serial.println("SizeFileSent: " + String(sizeFileSend));

    if (sizeFileSend >= fileSize) {
      Serial.println("Envio de archivo completado");
    }
    else if (sizeFileSend == 0) {
      Serial.println("!!Error al subir el archivo");
    }
    else if (sizeFileSend < fileSize) {
      Serial.println("!!Error, subida de archivo no completada");
    }

    Serial.println("FIN");

    while (1);
  }
}
#endif


void makeHeader( const char server[], const char resource[], int fileSize) {

  //  if ( String(server).indexOf("amazonaws")!=-1) {
  //    Serial.println("PUT " + String(resource) + " HTTP/1.1");
  //    Serial.println("Host: " + String(server));
  //    Serial.println("Content-Type: text/plain");
  //    Serial.println("Content-Length: " + String(fileSize));
  //    Serial.println();
  //
  //    clientSSL.println("PUT " + String(resource) + " HTTP/1.1");
  //    clientSSL.println("Host: " + String(server));
  //    clientSSL.println("Content-Type: text/plain");
  //    clientSSL.println("Content-Length: " + String(fileSize));
  //    clientSSL.println();
  //  }
  //  else if (String(server).indexOf("ubidots")!=-1) {
  //  String ubiBody = "{\"Esto\":{\"value\":1},\"es\":{\"value\":2},\"una\":{\"value\":3},\"prueba\":{\"value\":4},\"desde\":{\"value\":5},\"u201\":{\"value\":6},\"con\":{\"value\":6},\"X1\":{\"value\":7},\"modo\":{\"value\":8},\"seguro\":{\"value\":9}}";
  //
  //  Serial.println("POST " + String(resource) + " HTTP/1.1");
  //  Serial.println("Host: " + String(server));
  //  Serial.println("X-Auth-Token: BBFF-ls5FeJFqGnSofHF0zA7hYiPMQ8TXau");  //TOKEN WAVEPISTON
  //  Serial.println("Accept: */*");
  //  Serial.println("Content-Type: application/json");
  //  Serial.println("Content-Length: " + String(ubiBody.length()));
  //  Serial.println();  Serial.println();
  //  Serial.println(ubiBody);
  //  Serial.println();
  //
  //  clientSSL.println("POST " + String(resource) + " HTTP/1.1");
  //  clientSSL.println("Host: " + String(server));
  //  clientSSL.println("X-Auth-Token: BBFF-ls5FeJFqGnSofHF0zA7hYiPMQ8TXau");  //TOKEN WAVEPISTON
  //  clientSSL.println("Accept: */*");
  //  clientSSL.println("Content-Type: application/json");
  //  clientSSL.println("Content-Length: " + String(ubiBody.length() ));
  //  clientSSL.println();
  //  clientSSL.println(ubiBody);
  //  clientSSL.println();
  //  clientSSL.println();
  //
  //  //  }


  Serial.println("ubidots/1.0|POST|BBFF-d2f6hRO3a4F8QY9RttGnIByrbKpjOm|SSM-Device_03=>temp:20$day-hour=11$day-minute=23|end");

  clientSSL.println("ubidots/1.0|POST|BBFF-d2f6hRO3a4F8QY9RttGnIByrbKpjOm|SSM-Device_03=>temp:20$day-hour=11$day-minute=23|end");
}
