#define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))



static const String s_ParamJSON[7] = {"xh", "CO2", "H2", "VOC", "Eth", "T", "H"};
String s_ValJSON[NUMITEMS(s_ParamJSON)];
static const String LogJSON = "{NO INIT}";

uint8_t MAX_ERROR = 300;
uint8_t MAX_RETRY = 5; 
long LAST_SEND_TIME = 999999999999;



int takeParam(String nombre)
{
  bool match = false;
  int indx = 0;

  //Serial.println("Analizando : " + nombre);

  while (!match)
  {
    if (s_ParamJSON[indx] == nombre)
    {
      match = true;
      break;
    }
    else {
      // Serial.println(s_ParamJSON[indx]); //TEST
      indx++;
      if (indx >= NUMITEMS(s_ParamJSON))
      {
        Serial.print(F("ERROR, no se encuentra el nombre del parámetro: "));
        Serial.print(nombre);
        Serial.print(F(" en s_paramJSON, valor alcanzado de i: "));
        Serial.println(indx);
        match = true;
        break;
      }
      yield(); //test
    }
    //delay(50); //TEST
  }
  Serial.flush(); //Test
  return indx;
}

void init_v_JSON()
{
  for (int j = 0; j < NUMITEMS(s_ParamJSON); j++)
  {
    s_ValJSON[j] = "Nan";
  }

  s_ValJSON[takeParam("xh")] = DEF_SENSOR_ID;
}



void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print(F("0"));
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(F(":"));
    }
  }
  Serial.println();
}

void ligthHouse(char state, uint8_t code) {
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);

  /*
     CODIGOS:
        1: ENVIO CORRECTO
        2: ERROR EN EL ENVIO
        3: ERROR AL CONECTARSE A INTERNET
        4: ERROR EN LOS SENSORES
        5: ERROR AL CONECTAR AL BROKER
  */
  if (code <= 4) {
    for (uint8_t i = 0; i < code; i++) {
      digitalWrite(LED_B, HIGH);
      delay(300);
      digitalWrite(LED_B, LOW);
      delay(100);
    }
  }
  else {
    switch (state) {
      case '5':
        digitalWrite(LED_B, HIGH);
        delay(400);
        digitalWrite(LED_B, LOW);
        delay(100);
        for (uint8_t i = 0; i < 2; i++) {
          digitalWrite(LED_B, HIGH);
          delay(200);
          digitalWrite(LED_B, LOW);
          delay(100);
        }
        break;
      case '6':
        for (uint8_t i = 0; i < 2; i++) {
          digitalWrite(LED_B, HIGH);
          delay(100);
          digitalWrite(LED_B, LOW);
          delay(100);
        }
        digitalWrite(LED_B, HIGH);
        delay(500);
        break;
    }
  }



  digitalWrite(LED_R, LOW);
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_G, LOW);

  switch (state) {
    case 'G':
      digitalWrite(LED_G, HIGH);
      break;
    case 'M':
      digitalWrite(LED_G, HIGH);
      digitalWrite(LED_R, HIGH);
      break;
    case 'B':
      digitalWrite(LED_R, HIGH);
      break;
    default:
      break;
  }
}


unsigned long getTime() {
  // get the current time from the WiFi module
  return WiFi.getTime();
}

void initVector(float *vector , float values) {
  for (int i_v = 0; i_v < NUM_MEASURES_V; i_v++) {
    vector[i_v] = values;
  }
}

#ifdef AWS_CLOUD
bool tryECCX8() {
  if (!ECCX08.begin(ECCX08_ADDR)) {//if (!ECCX08.begin()) {
    Serial.println(F("No ECCX08 present!"));
    while (1); //fuerza que salte WTD
  }
  else {
    Serial.println(F("ECCX08 inicializado"));
  }

  const char* certificate  = AWS_CERT;

  // Set a callback to get the current time
  // used to validate the servers certificate
  ArduinoBearSSL.onGetTime(getTime);

  // Set the ECCX08 slot to use for the private key
  // and the accompanying public certificate for it
  // Note how the certificate is formed
  //  String(certificate) = String(client_cert) + String(root_ca);

  bearSSLclient.setEccSlot(0, certificate);
  return true;
}
#endif
