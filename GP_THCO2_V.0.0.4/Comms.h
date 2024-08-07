
// #include <ArduinoJson.h>


bool listNetworks();
void printWifiStatus();


bool initComPort() {
#ifdef ETH_MODEM_W5500
  pinMode(M1_INT, INPUT_PULLUP);
  pinMode(M1_RST, OUTPUT);
  digitalWrite(M1_RST, HIGH);
  pinMode(M1_CS, OUTPUT);
  digitalWrite(M1_CS, LOW);

  pinMode(M1_PWM, INPUT);
  pinMode(M1_RX, INPUT);
  pinMode(M1_TX, INPUT);

  Serial.println(F("Mikroe ETH WIZ configurado"));

  digitalWrite(M1_RST, LOW);
  delay(2000);
  digitalWrite(M1_RST, HIGH);
#endif


#ifdef WIFI_NINA
  byte mac[6];

  Serial.println(F("//////////////////////////////////"));
  if (WiFi.status() == WL_NO_MODULE) {
    return false;
  }
  else
  {
    Serial.println(F("UBLOX NINA configurado"));
    WiFi.macAddress(mac);
    Serial.print(F("MAC: "));
    printMacAddress(mac);

  }
#endif
  delay(10);
  return true;
}


bool startComPort() {
#ifdef  ETH_MODEM_W5500
  IPAddress DHCP_IP (0, 0, 0, 0);
  SPI.begin();
  delay(4000);
  Serial.println(F("Configuración de eth"));

  Ethernet.init(M1_CS);
  delay(1000);

  Ethernet.begin(macTEST, IP_Master);   //, dns_, gate_, subnet);//*mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);

  delay(2000);

  Serial.print(F("Iniciando configuración Ethernet, con IP:"));
  DHCP_IP = Ethernet.localIP();
  Serial.println(DHCP_IP);

  if ((DHCP_IP != (IPAddress) {
  0, 0, 0, 0
}) && (DHCP_IP != (IPAddress) {
  255, 255, 255, 255
})) {
    actutime = millis();
    while ((millis() - actutime) < 10000);
    return true;
  }
  return false;
#endif

#ifdef WIFI_NINA
#ifndef DEF_NINA_DHCP
  Serial.println(F("Static IP"));
  WiFi.config(IP_MEET, _DNS, _GATEWAY, _SUBNET); //ASTICAN:172.19.1.14 Meets del 14 al 21//marinePark:192, 168, 10, 56
#else
  Serial.println(F("DHCP"));
#endif
  if (WiFi.status() != WL_CONNECTED) {
    actutime = millis();
    if (!listNetworks()) {
      Serial.println(F("NO SE ENCUENTRAN REDES"));
      return false;
    }

    while ((WiFi.status() != WL_CONNECTED) && ((millis() - actutime) < (DEF_TRY_TIME * 1000))) {
      Serial.print(F("Intentando conectar con SSID: "));
      Serial.println(DEF_SSID);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(DEF_SSID, DEF_PASS);

      // wait 10 seconds for connection:
      delay(10000);
    }
  }
  else {
    Serial.println(F("Modem WiFi conectado"));

    IPAddress ip_ = WiFi.localIP();
    Serial.print(F("IP Address: "));
    Serial.println(ip_);
    return true;
  }
  printWifiStatus();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("Modem WiFi conectado"));
    return true;
  }
  Serial.println(F("ERROR WIFI NO CONECTA"));
  return false;
#endif
}






#ifdef TINY_GSM_MQTT
char* ArriveTopicMQTT;
byte* payloadArrive;
unsigned int len;


String DefTopic(String Topic1, String Topic2, String Topic3, String Topic4)
{
  String TOPICS =  String(Topic1) + "/" + String(Topic2) + "/" + String(Topic3) + "/" + String(Topic4);
  //Serial.print(F("TOPICS devuelto: "));
  //Serial.println(TOPICS);
  return TOPICS;
}


void mqttCallback(char* ArriveTopicMQTT, byte* payloadArrive, unsigned int len)
{
  Serial.print(F("Message MQTT arrived ["));
  Serial.print(ArriveTopicMQTT);
  Serial.print(F("]: "));
  Serial.write(payloadArrive, len);
  Serial.println();
}

void configMQTT() {

  mqtt.setServer(DEF_DIR_BROKER_MQTT, DEF_MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  Serial.println(F("MQTT CONFIGURADO"));
}

int ConnectMQTT ()
{
#ifdef WIFI_NINA
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(F("EQUIPO DESCONECTADO DE LA RED, ERROR AL CONECTAR CON BROKER"));
    startComPort();
    return 10;
  }
  else {
#endif

    //boolean statusMQTT = mqtt.connect( DEF_SENSOR_ID, DEF_USER_BROKER_MQTT, DEF_PASS_BROKER_MQTT);
    boolean statusMQTT = mqtt.connect(DEF_SENSOR_ID);
    //delay(300);

    if (statusMQTT == false)
    {
      Serial.println(F("ERROR al conectar con Broker MQTT!!! "));
      Serial.println(mqtt.state());

      return 10; //ANTES 1
    }
    else
    {
      if (mqtt.connected())
      {
        Serial.println(F("Conectado con Broker MQTT"));
        return 0;
      }
    }
#ifdef WIFI_NINA
  }
#endif

}


int sendMQTT(String MessageToSend)
{
  String finalTopic = "";
  finalTopic = DefTopic(DEF_TOPIC1_MQTT , DEF_TOPIC2_MQTT , DEF_TOPIC3_MQTT, DEF_TOPIC4_MQTT);

  mqtt.publish(finalTopic.c_str(), MessageToSend.c_str());
  //mqtt.subscribe("#");
  Serial.print(F("[Paquete MQTT] topic : "));
  Serial.print(finalTopic);
  Serial.print(F("  ||  [mensaje]: "));
  Serial.println(MessageToSend);
  delay(150);
  Serial.println(mqtt.state());
  if (mqtt.state() == 0) //Siempre que haya enviado
  {
    if (!f_firstTime) // si no se trata del primer envío actualiza el tiempo (para esperar hasta el siguiente envío)
    {
      LAST_SEND_TIME = millis();
      Serial.println(F("Tiempo de envio actualizado"));
    }
    else { //si es el primer envío el siguiente se realizará cuando se complete un vector, a 1 medición por segundo (teniendo en cuenta de que antes tiene que pasar 1 INTERVAL_HEATER completo)
      LAST_SEND_TIME = (millis() - (DEF_WAIT_TIME * 1000) + ((TIME_HEAT + TIME_COLD + NUM_MEASURES_V + 1) * 1000));
      f_firstTime = false; //limpiamos flag de primer envío
      Serial.print(F("Tiempo de primer envio actualizado")); //cuando se cumpla deberá estar completo 1 vector de medidas;
    }
  }
  else
  {
    return 11;
    mqtt.disconnect();
  }
  mqtt.disconnect();
  return 0;
}

#endif

String buildGenJSON() {
  String _output = "";
  int _nParametros = 0;

  _nParametros = NUMITEMS(s_ParamJSON);

  String json_A_Mano = "{";

  for ( uint8_t indx = 0 ; indx < NUMITEMS(s_ParamJSON); indx++) {
    if (s_ValJSON[indx] != "Nan") {
      json_A_Mano += "\"" + String(s_ParamJSON[indx]) + "\":\"" + String(s_ValJSON[indx] + "\"");
      if (indx < NUMITEMS(s_ParamJSON) - 1) {
        json_A_Mano += ",";
      }
    }
  }
  json_A_Mano += "}";
  return json_A_Mano;

  /* CON LIBRERIA JSON
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
  */

}


#ifdef NANO33IoT
bool listNetworks() {
  // scan for nearby networks:
  Serial.println(F("** Escaneando Redes **"));
  uint8_t numSsid = WiFi.scanNetworks();
  if (numSsid == -1)
  {
    Serial.println(F("No se encuentran redes"));
    return false;
  }

  // print the list of networks seen:
  Serial.print(F("Redes disponibles: "));
  //Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    byte bssid[6];
    Serial.print(thisNet + 1);
    Serial.print(F(") "));
    Serial.print(F("SSID: "));
    Serial.println(WiFi.SSID(thisNet));
    Serial.print(F("\tSignal: "));
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(F(" dBm"));
    Serial.print(F("\tChannel: "));
    Serial.print(WiFi.channel(thisNet));
    Serial.print(F("\t\tBSSID: "));
    printMacAddress(WiFi.BSSID(thisNet, bssid));
    Serial.flush();
    }
  
  return true;
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP: "));
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print(F("RSSI:"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
}
#endif
