/*
   Configurar alerta de presión máxima medible 16bar
   Configurar alerta de presión minima deseada según el equipo
*/

#define TINY_GSM_DEBUG Serial


#define NUM_FALLOS_MAX 30

#define CONFIG_DATA_DEFAULT

#define DEF_APN           "astican.movistar.es"//"internet.easym2m.eu"//"astican.movistar.es"//
#define DEF_APN_USER      "movistar"//""//"movistar"
#define DEF_APN_PASS      "movistar"//""//"movistar"
#define DEF_SSID          "ASTICAN_IOT"            //"JLP_AP"      //"MarinePark"      //"ASTICAN_IOT"               //"AstWiFi"
#define DEF_PASS          "em7cKNXVTH5KW6U7bpknyZZH"       //"0123456789"  //"Marine20_Park16" //"em7cKNXVTH5KW6U7bpknyZZH"  //"WiFi2013"
#define DEF_SERVER        "apidemo.muutech.com" //const char* "apidemo.muutech.com" //"industrial.ubidots.com"
#define DEF_PORT          80 //SSL 443 NOSSL 80 //UBIDOTS: SSL:9812
#define DEF_FLAG_SSL      false
#define DEF_PATH          "/astican.php" //
#define DEF_SENSOR_ID     "SSM-Aport-006"   //"SSM-sens-002" // "SSM-whst-002" //"SSM-pres-002" //"SSM-desa-001" //"SSM-comp-001" // "SSM-test1"  //"SSM-test2"  //"SSM-hidroc-001" //"SSM-AnalizRed-002" 
#define DEF_MEASURE_FREQ  30// TIEMPO ENTRE MEDICIONES (continuas) minimo 10 segundos (para menos modificar numero de lecturas) en segundos
#define DEF_SLEEP_TIME    5// SleepTime LowPower en segundos
#define DEF_WAIT_TIME     60 //TIEMPO ENTRE ENVIOS en segundos 
#define DEF_MIN_SPACE     65000 //KB espacio mínimo en la SD antes de empezar a borrar archivos (65000 espacio aprox 1 mes wavepiston)
#define DEF_PROGRAM       "LIOQfSU"//"IOQfSU"//"GLDIOQvHU"  
//"GLBDIOQveBArOPSHVvU"    
#define DEF_DESV          0 //porcentaje de desviación de medida respecto al rango del SENSOR



#define DEF_PORT_SLAVE_RJ45 502

#define NUMSLAVES_RJ45  1 // SIEMENS CT:3 , SCHNEIDER ARSExt:8


//#include <PubSubClient.h>
//CONFIG COMUNICACIONES MQTT
#define DEF_DIR_BROKER_MQTT  "172.19.1.200" //"mqtt.astican.muutech.com";// "172.19.1.200"; asticanmuubox
#define DEF_MQTT_PORT  12010 //8883 SSL // 1883 NO SSL //12040 MUUTECH //12010 asticanmuubox
#define DEF_USER_BROKER_MQTT  "astican"
#define DEF_PASS_BROKER_MQTT "eAnfpyPLYGfaPhkA5TAb"


#define DEF_TOPIC1_MQTT "ASTICAN"
#define DEF_TOPIC2_MQTT "IoT"
#define DEF_TOPIC3_MQTT "ANALIZADORESRED/PORTATIL"    // "TANQUES" //"CPD" //"ANALIZADORESRED" // "PRESION" //"SUBSEA" //"ESTACIONES" //"COMPRESORES" //"HIDROCARB"
#define DEF_TOPIC4_MQTT DEF_SENSOR_ID



#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))



#define JSON_RED       //usado para Schneider pm2200
//{"xh","xT","xV","xA","f1","Vff","Al1","Al2","Al3","An","Agnd","Aavg","Wt"}
//[0] y [1]en MAIN case 12 [2] y [3] en case 7





//MUUTECH
String param01 = "VFF";
String param02 = "Freq";
String param03 = "AmpA";
String param04 = "AmpB";
String param05 = "AmpC";
String param06 = "AmpN";
String param07 = "AmpGND";
String param08 = "AmpAVG";
String param09 = "ActE";

String param10 = "ReaE";
String param11 = "ApP";

String param12 = "Pact";
String param13 = "Prea";
String param14 = "Pap";

String param15 = "4QFPPF";
String param16 = "Alarm";




static const String s_ParamJSON[70] = {"xh", "xT", "xV", "xA", "S1", "S2", "S3", "S4", "S5", "S6", "S7", "S8", "wSP", "wSPM", "wPW", "wD", "wGD", "H", "T", "N", "pm2", "pm10", "r", "L", "sT", "Prcn", "dp", "prd", "cP", "Al1", "Al2", "Al3", "Al4", "f1", "f2", "f3", "f4", "Vff", "A1", "A2", "A3", "A4", "An", "Agnd", "Aavg", "Wt", "Qt", "St", "C1", "C2", "C3", "C4", "P1", "P2", "P3", "P4", "N1", "N2", "N3", "UV", "UVi", "hC","Er","Eac","Ere","Eap","Wac","Wre","Wap","QFP"};
//{"xh":"SSM-test1","xT":"22.50","xV":"32.76","xA":"-0.10","f1":"401.00","Vff":"400.00","Al1":"402.00","Al2":"403.00","Al3":"404.00","An":"405.00","Wt":"406.00","Qt":"407.00","St":"408.00"}

String s_ValJSON[NUMITEMS(s_ParamJSON)];

void init_v_JSON()
{
  for (int j = 0; j < NUMITEMS(s_ParamJSON); j++)
  {
    s_ValJSON[j] = "Nan";
  }



}



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
        Serial.println("ERROR, no se encuentra el nombre del parámetro: " + String(nombre) + " en s_paramJSON, valor alcanzado de i: " + String(indx));
        match = true;
        indx = NUMITEMS(s_ParamJSON)-1; //añadido para ARS, NUMITEMS(s_ParamJSON)-1 es el valor de la key "Er"
        break;
      }
      yield(); //test
    }
  }
   Serial.flush(); //Test
  return indx;
}


//lectura parametros de la placa GL
//medidas para lectura analogica BA
//conecto a internet O
//envío los datos analogicos P
//gestiono el tiempo de espera T
//Realizo el Log de datos  V
//espero para el siguiente ciclo U

//MINSENSC y MAXSENSC son los valores de mapeo del 4-20
//MINC y MAXC son valores de aumento de frecuencia

#define USEC1 0
#define MINSENSC1 0
#define MAXSENSC1 16
#define MINC1 0
#define MAXC1 16
#define USEC2 0
#define MINSENSC2 0
#define MAXSENSC2 16
#define MINC2 0
#define MAXC2 16
#define USEC3 0
#define MINSENSC3 0
#define MAXSENSC3 16
#define MINC3 6
#define MAXC3 16
#define USEC4 0
#define MINSENSC4 0
#define MAXSENSC4 16
#define MINC4 2
#define MAXC4 16


//#ifdef SSM_005
#define USEC5 0
#define MINSENSC5 0
#define MAXSENSC5 16
#define MINC5 0
#define MAXC5 5
#define USEC6 0
#define MINSENSC6 0
#define MAXSENSC6 16
#define MINC6 0
#define MAXC6 5
#define USEC7 0
#define MINSENSC7 0
#define MAXSENSC7 16
#define MINC7 0
#define MAXC7 5
#define USEC8 0
#define MINSENSC8 0
#define MAXSENSC8 16
#define MINC8 0
#define MAXC8 5
//#endif


typedef struct {
  String APN;
  String APNuser;
  String APNpass;
  String ssid;
  String password;
  String server;
  int port;
  bool f_ssl;
  String path;
  String sensorID;
  int measureFreq;
  int sleepTime;
  int waitTime;
  int minSpaceSD;
  String programa;

  int desviacion;

  int portRJ45;

} configData;

configData defaultConfig {DEF_APN, DEF_APN_USER, DEF_APN_PASS, DEF_SSID, DEF_PASS, DEF_SERVER, DEF_PORT, DEF_FLAG_SSL, DEF_PATH, DEF_SENSOR_ID, DEF_MEASURE_FREQ, DEF_SLEEP_TIME, DEF_WAIT_TIME, DEF_MIN_SPACE, DEF_PROGRAM, DEF_DESV,  DEF_PORT_SLAVE_RJ45};
configData localConfig;

typedef struct {
  bool usedch1;
  float minSensch1;
  float maxSensch1;
  float minch1;
  float maxch1;
  bool usedch2;
  float minSensch2;
  float maxSensch2;
  float minch2;
  float maxch2;
  bool usedch3;
  float minSensch3;
  float maxSensch3;
  float minch3;
  float maxch3;
  bool usedch4;
  float minSensch4;
  float maxSensch4;
  float minch4;
  float maxch4;

  //#ifdef SSM_005
  bool usedch5;
  float minSensch5;
  float maxSensch5;
  float minch5;
  float maxch5;
  bool usedch6;
  float minSensch6;
  float maxSensch6;
  float minch6;
  float maxch6;
  bool usedch7;
  float minSensch7;
  float maxSensch7;
  float minch7;
  float maxch7;
  bool usedch8;
  float minSensch8;
  float maxSensch8;
  float minch8;
  float maxch8;
  //  #endif

} analogMap;

analogMap defaultAnaMap{USEC1, MINSENSC1, MAXSENSC1, MINC1, MAXC1, USEC2, MINSENSC2, MAXSENSC2, MINC2, MAXC2, USEC3, MINSENSC3, MAXSENSC3, MINC3, MAXC3, USEC4, MINSENSC4, MAXSENSC4, MINC4, MAXC4, USEC5, MINSENSC5, MAXSENSC5, MINC5, MAXC5, USEC6, MINSENSC6, MAXSENSC6, MINC6, MAXC6, USEC7, MINSENSC7, MAXSENSC7, MINC7, MAXC7, USEC8, MINSENSC8, MAXSENSC8, MINC8, MAXC8};
analogMap localAnaMap;


typedef struct {
  String brokerID;
  int   port;
  String user;
  String pass;

  String topic1;
  String topic2;
  String topic3;
  String topic4;
} configMQTT;

configMQTT defaultConfigMQTT{DEF_DIR_BROKER_MQTT, DEF_MQTT_PORT, DEF_USER_BROKER_MQTT, DEF_PASS_BROKER_MQTT, DEF_TOPIC1_MQTT, DEF_TOPIC2_MQTT, DEF_TOPIC3_MQTT, DEF_TOPIC4_MQTT};
configMQTT localConfigMQTT;





void loadDefualt( configData*_localData, analogMap* _localAnaMap, configMQTT* _localConfigMQTT) {
  _localData->APN = DEF_APN;
  _localData->APNuser = DEF_APN_USER;
  _localData->APNpass = DEF_APN_PASS;
  _localData->ssid = DEF_SSID;
  _localData->password = DEF_PASS;
  _localData->server = DEF_SERVER;
  _localData->port = DEF_PORT;
  _localData->f_ssl = DEF_FLAG_SSL;
  _localData->path = DEF_PATH;
  _localData->sensorID = DEF_SENSOR_ID;
  _localData->measureFreq = DEF_MEASURE_FREQ;
  _localData->sleepTime = DEF_SLEEP_TIME;
  _localData->waitTime = DEF_WAIT_TIME;
  _localData->minSpaceSD = DEF_MIN_SPACE;
  _localData->programa = DEF_PROGRAM;
  _localData->desviacion = DEF_DESV;
  _localData->portRJ45 = DEF_PORT_SLAVE_RJ45;

  _localConfigMQTT->brokerID = DEF_DIR_BROKER_MQTT;
  _localConfigMQTT->port = DEF_MQTT_PORT;
  _localConfigMQTT->user = DEF_USER_BROKER_MQTT;
  _localConfigMQTT->pass = DEF_PASS_BROKER_MQTT;
  _localConfigMQTT->topic1 = DEF_TOPIC1_MQTT;
  _localConfigMQTT->topic2 = DEF_TOPIC2_MQTT;
  _localConfigMQTT->topic3 = DEF_TOPIC3_MQTT;
  _localConfigMQTT->topic4 = DEF_TOPIC4_MQTT;

  _localAnaMap->usedch1 = USEC1;
  _localAnaMap->minSensch1 = MINSENSC1;
  _localAnaMap->maxSensch1 = MAXSENSC1;
  _localAnaMap->minch1 = MINC1;
  _localAnaMap->maxch1 = MAXC1;
  _localAnaMap->usedch2 = USEC2;
  _localAnaMap->minSensch2 = MINSENSC2;
  _localAnaMap->maxSensch2 = MAXSENSC2;
  _localAnaMap->minch2 = MINC2;
  _localAnaMap->maxch2 = MAXC2;
  _localAnaMap->usedch3 = USEC3;
  _localAnaMap->minSensch3 = MINSENSC3;
  _localAnaMap->maxSensch3 = MAXSENSC3;
  _localAnaMap->minch3 = MINC3;
  _localAnaMap->maxch3 = MAXC3;
  _localAnaMap->usedch4 = USEC4;
  _localAnaMap->minSensch4 = MINSENSC4;
  _localAnaMap->maxSensch4 = MAXSENSC4;
  _localAnaMap->minch4 = MINC4;
  _localAnaMap->maxch4 = MAXC4;


  //#ifdef SSM_005
  _localAnaMap->usedch5 = USEC5;
  _localAnaMap->minSensch5 = MINSENSC5;
  _localAnaMap->maxSensch5 = MAXSENSC5;
  _localAnaMap->minch5 = MINC5;
  _localAnaMap->maxch5 = MAXC5;
  _localAnaMap->usedch6 = USEC6;
  _localAnaMap->minSensch6 = MINSENSC6;
  _localAnaMap->maxSensch6 = MAXSENSC6;
  _localAnaMap->minch6 = MINC6;
  _localAnaMap->maxch6 = MAXC6;
  _localAnaMap->usedch7 = USEC7;
  _localAnaMap->minSensch7 = MINSENSC7;
  _localAnaMap->maxSensch7 = MAXSENSC7;
  _localAnaMap->minch7 = MINC7;
  _localAnaMap->maxch7 = MAXC7;
  _localAnaMap->usedch8 = USEC8;
  _localAnaMap->minSensch8 = MINSENSC8;
  _localAnaMap->maxSensch8 = MAXSENSC8;
  _localAnaMap->minch8 = MINC8;
  _localAnaMap->maxch8 = MAXC8;
  //#endif
}
void printConig(configData*_localData, analogMap* _localAnaMap, configMQTT* _localConfigMQTT) {
  Serial.print(" APN "); Serial.println(_localData->APN);
  Serial.print(" APNuser "); Serial.println(_localData->APNuser);
  Serial.print(" APNpass "); Serial.println(_localData->APNpass);
  Serial.print(" ssid "); Serial.println(_localData->ssid);
  Serial.print(" password "); Serial.println(_localData->password);
  Serial.print(" server "); Serial.println(_localData->server);
  Serial.print(" port "); Serial.println(_localData->port);
  Serial.print(" flagSSL "); Serial.println( _localData->f_ssl);
  Serial.print(" path "); Serial.println(_localData->path);
  Serial.print(" sensorID "); Serial.println(_localData->sensorID);
  Serial.print(" measureFreq "); Serial.println(_localData->measureFreq);
  Serial.print(" sleepTime "); Serial.println(_localData->sleepTime);
  Serial.print(" waitTime "); Serial.println(_localData->waitTime);
  Serial.print(" minSpaceSD "); Serial.println(_localData->minSpaceSD);
  Serial.print(" programa "); Serial.println(_localData->programa);
  Serial.print(" desviación "); Serial.println(_localData->desviacion);
  Serial.print(" portRJ45 "); Serial.println(_localData->portRJ45);

  Serial.print(" MQTT_brokerID "); Serial.println( _localConfigMQTT->brokerID);
  Serial.print(" MQTT_port "); Serial.println( _localConfigMQTT->port);
  Serial.print(" MQTT_user "); Serial.println(_localConfigMQTT->user);
  Serial.print(" MQTT_pass "); Serial.println(_localConfigMQTT->pass);
  Serial.print(" MQTT_Topic1 "); Serial.println(_localConfigMQTT->topic1);
  Serial.print(" MQTT_Topic2 "); Serial.println(_localConfigMQTT->topic2);
  Serial.print(" MQTT_Topic3 "); Serial.println(_localConfigMQTT->topic3);
  Serial.print(" MQTT_Topic4 "); Serial.println(_localConfigMQTT->topic4);

  Serial.print(" usedch1 "); Serial.println(_localAnaMap->usedch1);
  Serial.print(" minSensch1 "); Serial.println(_localAnaMap->minSensch1);
  Serial.print(" maxSensch1 "); Serial.println(_localAnaMap->maxSensch1);
  Serial.print(" minch1 "); Serial.println(_localAnaMap->minch1);
  Serial.print(" maxch1 "); Serial.println(_localAnaMap->maxch1);
  Serial.print(" usedch2 "); Serial.println(_localAnaMap->usedch2);
  Serial.print(" minSensch2 "); Serial.println(_localAnaMap->minSensch2);
  Serial.print(" maxSensch2 "); Serial.println(_localAnaMap->maxSensch2);
  Serial.print(" minch2 "); Serial.println(_localAnaMap->minch2);
  Serial.print(" maxch2 "); Serial.println(_localAnaMap->maxch2);
  Serial.print(" usedch3 "); Serial.println(_localAnaMap->usedch3);
  Serial.print(" minSensch3 "); Serial.println(_localAnaMap->minSensch3);
  Serial.print(" maxSensch3 "); Serial.println(_localAnaMap->maxSensch3);
  Serial.print(" minch3 "); Serial.println(_localAnaMap->minch3);
  Serial.print(" maxch3 "); Serial.println(_localAnaMap->maxch3);
  Serial.print(" usedch4 "); Serial.println(_localAnaMap->usedch4);
  Serial.print(" minSensch4 "); Serial.println(_localAnaMap->minSensch4);
  Serial.print(" maxSensch4 "); Serial.println(_localAnaMap->maxSensch4);
  Serial.print(" minch4 "); Serial.println(_localAnaMap->minch4);
  Serial.print(" maxch4 "); Serial.println(_localAnaMap->maxch4);

  //#ifdef SSM_005
  Serial.print(" usedch5 "); Serial.println(_localAnaMap->usedch5);
  Serial.print(" minSensch5 "); Serial.println(_localAnaMap->minSensch5);
  Serial.print(" maxSensch5 "); Serial.println(_localAnaMap->maxSensch5);
  Serial.print(" minch5 "); Serial.println(_localAnaMap->minch5);
  Serial.print(" maxch5 "); Serial.println(_localAnaMap->maxch5);
  Serial.print(" usedch6 "); Serial.println(_localAnaMap->usedch6);
  Serial.print(" minSensch6 "); Serial.println(_localAnaMap->minSensch6);
  Serial.print(" maxSensch6 "); Serial.println(_localAnaMap->maxSensch6);
  Serial.print(" minch6 "); Serial.println(_localAnaMap->minch6);
  Serial.print(" maxch6 "); Serial.println(_localAnaMap->maxch6);
  Serial.print(" usedch7 "); Serial.println(_localAnaMap->usedch7);
  Serial.print(" minSensch7 "); Serial.println(_localAnaMap->minSensch7);
  Serial.print(" maxSensch7 "); Serial.println(_localAnaMap->maxSensch7);
  Serial.print(" minch7 "); Serial.println(_localAnaMap->minch7);
  Serial.print(" maxch7 "); Serial.println(_localAnaMap->maxch7);
  Serial.print(" usedch8 "); Serial.println(_localAnaMap->usedch8);
  Serial.print(" minSensch8 "); Serial.println(_localAnaMap->minSensch8);
  Serial.print(" maxSensch8 "); Serial.println(_localAnaMap->maxSensch8);
  Serial.print(" minch8 "); Serial.println(_localAnaMap->minch8);
  Serial.print(" maxch8 "); Serial.println(_localAnaMap->maxch8);
  //#endif
}
