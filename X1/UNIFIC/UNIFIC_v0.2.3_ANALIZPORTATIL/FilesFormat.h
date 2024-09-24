#ifndef CONFIG_DATA_DEFAULT
#include "config.h"
#endif

//Sd2Card card;
//SdVolume volume;
//SdFile root;

//Config File
String configName = "config.txt";
String commandMenu = "*******************************************************************************************************\n"
                     "** A -> Medida Analog 1     B -> AlimentoAna 2      C -> ApagoAna   3         D -> Alimento RS485 4  **\n"
                     "** E -> Mido Estacion 5     F -> Apago estacion 6   G -> PwrEntrada 7         H -> Apago todo 8      **\n"
                     "** I -> leeAnalizSch 9      J -> ListaArchivos 10   K -> LecturaArchivo 11    L -> TemperaturaPCB 12 **\n"
                     "** M -> Forecast 13         N -> BorrarArchivo 14   O -> connexion Inet 15    P -> Send analog 16    **\n"
                     "** Q -> Send 485 17         R -> EnviaAnalizPM 18   S -> conexion close 19    T -> Gestion tiempo 20 **\n"
                     "** U -> Wait(iddle) 21      V -> Log data 22        W -> PrintMenu 23         X -> ReadConfig  24    **\n"
                     "** Y ->  25                 Z -> Reset 26           a ->     27               b ->     28            **\n"
                     "** c -> ecomode             d -> leeAmpRs485 30     e -> Gestión RS485-ETH    f -> MQTT Disc         **\n"
                     "** g -> Firmwr updt Http 33 r -> Reset SendTime 44  s -> LecturaSiemens 45    t -> TakeTime 46       **\n"
                     "** u -> UploadFile  47      v -> LogData(2.0) 48    h -> TESTEOS 50           - -> LowPower 60       **\n"
                     "*******************************************************************************************************\n";
bool ToBoolean(String valueRead);


inline void ToCharArray (char* buf, File fileToRead) {
  char c_aux;
  uint8_t i = 0;
  Serial.println("EL VALOR0 ES: " + String(buf));

  c_aux = fileToRead.read();
  Serial.print("VALUE: ");
  //  while (c_aux != '\r') {
  //    Serial.print(c_aux);
  //    const char* posi;
  //    posi = llega + i;
  //    i++;
  //    c_aux = fileToRead.read();
  //  }

  while (c_aux != '\r') {
    Serial.print(c_aux);
    const char* posi;
    buf[i] = c_aux;
    i++;
    c_aux = fileToRead.read();
  }

  Serial.println("");
  Serial.println("EL VALOR ES: " + String(buf));
}


int _createConfigFile(File myLocalFile, configData*_config, analogMap* _anaMap, configMQTT* _configMQTT) {
  myLocalFile.print("APN: "); myLocalFile.println(_config->APN);
  myLocalFile.print("APNUser: "); myLocalFile.println(_config->APNuser);
  myLocalFile.print("APNPass: "); myLocalFile.println(_config->APNpass);
  myLocalFile.print("ssid: "); myLocalFile.println(_config->ssid);
  myLocalFile.print("password: "); myLocalFile.println(_config->password);
  myLocalFile.print("Server: "); myLocalFile.println(_config->server);
  myLocalFile.print("port: "); myLocalFile.println(_config->port);
  myLocalFile.print("flag_SSL: "); myLocalFile.println(_config->f_ssl);
  myLocalFile.print("path: "); myLocalFile.println(_config->path);
  myLocalFile.print("SensorID: "); myLocalFile.println(_config->sensorID);
  myLocalFile.print("MeasureFreq: "); myLocalFile.println(_config->measureFreq);
  myLocalFile.print("SleepTime: "); myLocalFile.println(_config->sleepTime);
  myLocalFile.print("WaitTime: "); myLocalFile.println(_config->waitTime);
  myLocalFile.print("minSpaceInSD: "); myLocalFile.println(_config->minSpaceSD);
  myLocalFile.print("Programa: "); myLocalFile.println(_config->programa);
  myLocalFile.print("Desviación: "); myLocalFile.println(_config->desviacion);
  myLocalFile.print("portRJ45: "); myLocalFile.println(_config->desviacion);

  myLocalFile.print("MQTT_brokerID: "); myLocalFile.println(_configMQTT->brokerID);
  myLocalFile.print("MQTT_port: "); myLocalFile.println(_configMQTT->port);
  myLocalFile.print("MQTT_user: "); myLocalFile.println(_configMQTT->user);
  myLocalFile.print("MQTT_pass: "); myLocalFile.println(_configMQTT->pass);
  myLocalFile.print("MQTT_Topic1: "); myLocalFile.println(_configMQTT->topic1);
  myLocalFile.print("MQTT_Topic2: "); myLocalFile.println(_configMQTT->topic2);
  myLocalFile.print("MQTT_Topic3: "); myLocalFile.println(_configMQTT->topic3);
  myLocalFile.print("MQTT_Topic4: "); myLocalFile.println(_configMQTT->topic4);

  myLocalFile.print("ch1used: ");
  if ( _anaMap->usedch1 == 1) myLocalFile.println('Y');
  else myLocalFile.println('N');
  myLocalFile.print("ch1minSens: "); myLocalFile.println(_anaMap->minSensch1);
  myLocalFile.print("ch1maxSens: "); myLocalFile.println(_anaMap->maxSensch1);
  myLocalFile.print("ch1min: "); myLocalFile.println(_anaMap->minch1);
  myLocalFile.print("ch1max: "); myLocalFile.println(_anaMap->maxch1);

  myLocalFile.print("ch2used: ");//myLocalFile.println(_anaMap->usedch2);
  if ( _anaMap->usedch2 == 1) myLocalFile.println('Y');
  else myLocalFile.println('N');
  myLocalFile.print("ch2minSens: "); myLocalFile.println(_anaMap->minSensch2);
  myLocalFile.print("ch2maxSens: "); myLocalFile.println(_anaMap->maxSensch2);
  myLocalFile.print("ch2min: "); myLocalFile.println(_anaMap->minch2);
  myLocalFile.print("ch2max: "); myLocalFile.println(_anaMap->maxch2);

  myLocalFile.print("ch3used: ");//myLocalFile.println(_anaMap->usedch3);
  if ( _anaMap->usedch3 == 1) myLocalFile.println('Y');
  else myLocalFile.println('N');
  myLocalFile.print("ch3minSens: "); myLocalFile.println(_anaMap->minSensch3);
  myLocalFile.print("ch3maxSens: "); myLocalFile.println(_anaMap->maxSensch3);
  myLocalFile.print("ch3min: "); myLocalFile.println(_anaMap->minch3);
  myLocalFile.print("ch3max: "); myLocalFile.println(_anaMap->maxch3);

  myLocalFile.print("ch4used: ");//myLocalFile.println(_anaMap->usedch4);
  if ( _anaMap->usedch4 == 1) myLocalFile.println('Y');
  else myLocalFile.println('N');
  myLocalFile.print("ch4minSens: "); myLocalFile.println(_anaMap->minSensch4);
  myLocalFile.print("ch4maxSens: "); myLocalFile.println(_anaMap->maxSensch4);
  myLocalFile.print("ch4min: "); myLocalFile.println(_anaMap->minch4);
  myLocalFile.print("ch4max: "); myLocalFile.println(_anaMap->maxch4);

#ifdef SSM_005
  myLocalFile.print("ch5used: ");
  if ( _anaMap->usedch5 == 1) myLocalFile.println('Y');
  else myLocalFile.println('N');
  myLocalFile.print("ch5minSens: "); myLocalFile.println(_anaMap->minSensch5);
  myLocalFile.print("ch5maxSens: "); myLocalFile.println(_anaMap->maxSensch5);
  myLocalFile.print("ch5min: "); myLocalFile.println(_anaMap->minch5);
  myLocalFile.print("ch5max: "); myLocalFile.println(_anaMap->maxch5);
  myLocalFile.print("ch6used: ");
  if ( _anaMap->usedch6 == 1) myLocalFile.println('Y');
  else myLocalFile.println('N');
  myLocalFile.print("ch6minSens: "); myLocalFile.println(_anaMap->minSensch6);
  myLocalFile.print("ch6maxSens: "); myLocalFile.println(_anaMap->maxSensch6);
  myLocalFile.print("ch6min: "); myLocalFile.println(_anaMap->minch6);
  myLocalFile.print("ch6max: "); myLocalFile.println(_anaMap->maxch6);
  myLocalFile.print("ch7used: ");
  if ( _anaMap->usedch7 == 1) myLocalFile.println('Y');
  else myLocalFile.println('N');
  myLocalFile.print("ch7minSens: "); myLocalFile.println(_anaMap->minSensch7);
  myLocalFile.print("ch7maxSens: "); myLocalFile.println(_anaMap->maxSensch7);
  myLocalFile.print("ch7min: "); myLocalFile.println(_anaMap->minch7);
  myLocalFile.print("ch7max: "); myLocalFile.println(_anaMap->maxch7);
  myLocalFile.print("ch8used: ");
  if ( _anaMap->usedch8 == 1) myLocalFile.println('Y');
  else myLocalFile.println('N');
  myLocalFile.print("ch8minSens: "); myLocalFile.println(_anaMap->minSensch8);
  myLocalFile.print("ch8maxSens: "); myLocalFile.println(_anaMap->maxSensch8);
  myLocalFile.print("ch8min: "); myLocalFile.println(_anaMap->minch8);
  myLocalFile.print("ch8max: "); myLocalFile.println(_anaMap->maxch8);
#endif

  myLocalFile.println(); myLocalFile.println();
  myLocalFile.println("Menu"); myLocalFile.println();
  myLocalFile.println(commandMenu);
  myLocalFile.close();
  return 0;
}

int _readConfigFile(File myLocalFile, configData*_config, analogMap* _anaMap, configMQTT*  _configMQTT) {
  _config->APN = "";
  _config->APNuser = "";
  _config->APNpass = "";
  _config->ssid = "";
  _config->password = "";
  _config->server = "";
  _config->port = 0;
  _config->f_ssl = false;
  _config->path = "";
  _config->sensorID = "";
  _config->measureFreq = 0;
  _config->sleepTime = 0;
  _config->waitTime = 0;
  _config->minSpaceSD = 0;
  _config->programa = "";
  _config->desviacion = 0;
  _config->portRJ45 = 0;

  _configMQTT->brokerID = "";
  _configMQTT->port = 0;
  _configMQTT->user = "";
  _configMQTT->pass = "";;
  _configMQTT->topic1 = "";
  _configMQTT->topic2 = "";
  _configMQTT->topic3 = "";
  _configMQTT->topic4 = "";

  _anaMap->usedch1 = 1;
  _anaMap->minSensch1 = 0;
  _anaMap->maxSensch1 = 0;
  _anaMap->minch1 = 0;
  _anaMap->maxch1 = 0;
  _anaMap->usedch2 = 1;
  _anaMap->minSensch2 = 0;
  _anaMap->maxSensch2 = 0;
  _anaMap->minch2 = 0;
  _anaMap->maxch2 = 0;
  _anaMap->usedch3 = 0;
  _anaMap->minSensch3 = 0;
  _anaMap->maxSensch3 = 0;
  _anaMap->minch3 = 0;
  _anaMap->maxch3 = 0;
  _anaMap->usedch4 = 0;
  _anaMap->minSensch4 = 0;
  _anaMap->maxSensch4 = 0;
  _anaMap->minch4 = 0;
  _anaMap->maxch4 = 0;

#ifdef SSM_005
  _anaMap->usedch5 = 1;
  _anaMap->minSensch5 = 0;
  _anaMap->maxSensch5 = 0;
  _anaMap->minch5 = 0;
  _anaMap->maxch5 = 0;
  _anaMap->usedch6 = 1;
  _anaMap->minSensch6 = 0;
  _anaMap->maxSensch6 = 0;
  _anaMap->minch6 = 0;
  _anaMap->maxch6 = 0;
  _anaMap->usedch7 = 0;
  _anaMap->minSensch7 = 0;
  _anaMap->maxSensch7 = 0;
  _anaMap->minch7 = 0;
  _anaMap->maxch7 = 0;
  _anaMap->usedch8 = 0;
  _anaMap->minSensch8 = 0;
  _anaMap->maxSensch8 = 0;
  _anaMap->minch8 = 0;
  _anaMap->maxch8 = 0;
#endif

  String c = "";
  char c_aux;

  myLocalFile.readStringUntil(' ');
  _config->APN += myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  _config->APNuser += myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  _config->APNpass += myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  _config->ssid += myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  _config->password += myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  _config->server += myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  _config->port = myLocalFile.readStringUntil('\r').toInt();
  myLocalFile.readStringUntil(' ');
  _config->f_ssl = ToBoolean(myLocalFile.readStringUntil('\r'));
  myLocalFile.readStringUntil(' ');
  _config->path = myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  //ToCharArray( _config->sensorID, myLocalFile);
  _config->sensorID += myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  _config->measureFreq = myLocalFile.readStringUntil('\r').toInt();
  myLocalFile.readStringUntil(' ');
  _config->sleepTime = myLocalFile.readStringUntil('\r').toInt();
  myLocalFile.readStringUntil(' ');
  _config->waitTime = myLocalFile.readStringUntil('\r').toInt();
  myLocalFile.readStringUntil(' ');
  _config->minSpaceSD = myLocalFile.readStringUntil('\r').toInt();
  myLocalFile.readStringUntil(' ');
  _config->programa += myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  _config->desviacion = myLocalFile.readStringUntil('\r').toInt();
  myLocalFile.readStringUntil(' ');
  _config->portRJ45 = myLocalFile.readStringUntil('\r').toInt();

  myLocalFile.readStringUntil(' ');
  //ToCharArray( _configMQTT->brokerID, myLocalFile);
  _configMQTT->brokerID += myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  _configMQTT->port = myLocalFile.readStringUntil('\r').toInt();
  myLocalFile.readStringUntil(' ');
  //ToCharArray( _configMQTT->user, myLocalFile);
  _configMQTT->user += myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  //ToCharArray( _configMQTT->pass, myLocalFile);
  _configMQTT->pass += myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  //ToCharArray( _configMQTT->topic1, myLocalFile);
  _configMQTT->topic1 += myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  //ToCharArray( _configMQTT->topic2, myLocalFile);
  _configMQTT->topic2 += myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  //ToCharArray( _configMQTT->topic3, myLocalFile);
  _configMQTT->topic3 += myLocalFile.readStringUntil('\r');
  myLocalFile.readStringUntil(' ');
  //ToCharArray( _configMQTT->topic4, myLocalFile);
  _configMQTT->topic4 += myLocalFile.readStringUntil('\r');

  myLocalFile.readStringUntil(' ');
  c = myLocalFile.readStringUntil('\r');
  if (c[0] == 'Y')_anaMap->usedch1 = 1;
  else _anaMap->usedch1 = 0;
  myLocalFile.readStringUntil(' ');
  _anaMap->minSensch1 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxSensch1 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->minch1 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxch1 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  c = myLocalFile.readStringUntil('\r');
  if (c[0] == 'Y')_anaMap->usedch2 = 1;
  else _anaMap->usedch2 = 0;
  //_anaMap->usedch2 = myLocalFile.readStringUntil('\r').toInt();
  myLocalFile.readStringUntil(' ');
  _anaMap->minSensch2 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxSensch2 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->minch2 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxch2 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  c = myLocalFile.readStringUntil('\r');
  if (c[0] == 'Y')_anaMap->usedch3 = 1;
  else _anaMap->usedch3 = 0;
  //_anaMap->usedch3 = myLocalFile.readStringUntil('\r').toInt();
  myLocalFile.readStringUntil(' ');
  _anaMap->minSensch3 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxSensch3 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->minch3 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxch3 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  c = myLocalFile.readStringUntil('\r');
  if (c[0] == 'Y')_anaMap->usedch4 = 1;
  else _anaMap->usedch4 = 0;
  //_anaMap->usedch4 = myLocalFile.readStringUntil('\r').toInt();
  myLocalFile.readStringUntil(' ');
  _anaMap->minSensch4 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxSensch4 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->minch4 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxch4 = myLocalFile.readStringUntil('\r').toFloat();

#ifdef SSM_005
  myLocalFile.readStringUntil(' ');
  c = myLocalFile.readStringUntil('\r');
  if (c[0] == 'Y')_anaMap->usedch5 = 1;
  else _anaMap->usedch5 = 0;
  myLocalFile.readStringUntil(' ');
  _anaMap->minSensch5 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxSensch5 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->minch5 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxch5 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  c = myLocalFile.readStringUntil('\r');
  if (c[0] == 'Y')_anaMap->usedch6 = 1;
  else _anaMap->usedch6 = 0;
  myLocalFile.readStringUntil(' ');
  _anaMap->minSensch6 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxSensch6 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->minch6 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxch6 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  c = myLocalFile.readStringUntil('\r');
  if (c[0] == 'Y')_anaMap->usedch7 = 1;
  else _anaMap->usedch7 = 0;
  myLocalFile.readStringUntil(' ');
  _anaMap->minSensch7 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxSensch7 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->minch7 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxch7 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  c = myLocalFile.readStringUntil('\r');
  if (c[0] == 'Y')_anaMap->usedch8 = 1;
  else _anaMap->usedch8 = 0;
  myLocalFile.readStringUntil(' ');
  _anaMap->minSensch8 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxSensch8 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->minch8 = myLocalFile.readStringUntil('\r').toFloat();
  myLocalFile.readStringUntil(' ');
  _anaMap->maxch8 = myLocalFile.readStringUntil('\r').toFloat();
#endif

  myLocalFile.close();
  return 0;
}


bool ToBoolean(String valueRead)
{
  uint8_t indx = 0;
  valueRead.toLowerCase();

  if ((valueRead.indexOf("true") != -1) || (valueRead.toInt() == 1)) {
    indx = 1;
  }
  else  if ((valueRead.indexOf("false") != -1) || (valueRead.toInt() == 0)) {
    indx = 2;
  }

  switch (indx)
  {
    case 1:
      return true;
    case 2:
      return false;
    default:
      return false;
  }
}

//
////Log File
//String logName = "CSVLOG.TXT";
//String logNameDir = "/LOGDATA";
////String logFileText = "Time,BoardTemp,Input V,Input mA,Sensor ID,SensCh1 (4-20mA),SensCh2 (4-20mA),SensCh3 (4-20mA),SensCh4 (4-20mA),SensCh5 (4-20mA),SensCh6 (4-20mA),SensCh7 (4-20mA),SensCh8 (4-20mA),FailConnection,Envio Dato,Reset,Error,Programa";
////
////void writeLogFile(File myLocalFile,String _time,float _pcbTemp,float _volt,float _current,String _idSens, float _ch1,float _ch2,float _ch3,float _ch4,float _ch5,float _ch6, float _ch7,float _ch8,bool *b_failConnect,bool* b_sendData,bool*b_reset,bool* b_error,String b_prog){
////  myLocalFile.print(_time);myLocalFile.print(",");
////  myLocalFile.print(_pcbTemp);myLocalFile.print(",");
////  myLocalFile.print(_volt);myLocalFile.print(",");
////  myLocalFile.print(_current);myLocalFile.print(",");
////  myLocalFile.print(_idSens);myLocalFile.print(",");
////  myLocalFile.print(_ch1);myLocalFile.print(",");
////  myLocalFile.print(_ch2);myLocalFile.print(",");
////  myLocalFile.print(_ch3);myLocalFile.print(",");
////  myLocalFile.print(_ch4);myLocalFile.print(",");
////
////  myLocalFile.print(_ch5);myLocalFile.print(",");
////  myLocalFile.print(_ch6);myLocalFile.print(",");
////  myLocalFile.print(_ch7);myLocalFile.print(",");
////  myLocalFile.print(_ch8);myLocalFile.print(",");
////
////  if (*b_failConnect){
////    myLocalFile.print("Si");myLocalFile.print(",");
////  }else{
////    myLocalFile.print("No");myLocalFile.print(",");
////    *b_failConnect  = false;
////  }
////  if (*b_sendData){
////    myLocalFile.print("Si");myLocalFile.print(",");
////    *b_sendData  = false;
////  }else{
////    myLocalFile.print("No");myLocalFile.print(",");
////  }
////  if (*b_reset){
////    myLocalFile.print("Si");myLocalFile.print(",");
////    *b_reset     = false;
////  }else{
////    myLocalFile.print("No");myLocalFile.print(",");
////  }
////  if (* b_error){
////    myLocalFile.print("Si");myLocalFile.print(",");
////    * b_error     = false;
////  }else{
////    myLocalFile.print("No");myLocalFile.print(",");
////  }
////  myLocalFile.println(b_prog);
////  myLocalFile.println();
////  myLocalFile.close();
////}
////
////String logFileText = "Time,BoardTemp,Input V,Input mA,Sensor ID,SensCh1 (4-20mA),SensCh2 (4-20mA),SensCh3 (4-20mA),SensCh4 (4-20mA),FailConnection,Envio Dato,Reset,Error,Programa";
////
////void writeLogFile(File myLocalFile,String _time,float _pcbTemp,float _volt,float _current,String _idSens, float _ch1,float _ch2,float _ch3,float _ch4,bool *b_failConnect,bool* b_sendData,bool*b_reset,bool* b_error,String b_prog){
////  myLocalFile.print(_time);myLocalFile.print(",");
////  myLocalFile.print(_pcbTemp);myLocalFile.print(",");
////  myLocalFile.print(_volt);myLocalFile.print(",");
////  myLocalFile.print(_current);myLocalFile.print(",");
////  myLocalFile.print(_idSens);myLocalFile.print(",");
////  myLocalFile.print(_ch1);myLocalFile.print(",");
////  myLocalFile.print(_ch2);myLocalFile.print(",");
////  myLocalFile.print(_ch3);myLocalFile.print(",");
////  myLocalFile.print(_ch4);myLocalFile.print(",");
////  if (*b_failConnect){
////    myLocalFile.print("Si");myLocalFile.print(",");
////  }else{
////    myLocalFile.print("No");myLocalFile.print(",");
////    *b_failConnect  = false;
////  }
////  if (*b_sendData){
////    myLocalFile.print("Si");myLocalFile.print(",");
////    *b_sendData  = false;
////  }else{
////    myLocalFile.print("No");myLocalFile.print(",");
////  }
////  if (*b_reset){
////    myLocalFile.print("Si");myLocalFile.print(",");
////    *b_reset     = false;
////  }else{
////    myLocalFile.print("No");myLocalFile.print(",");
////  }
////  if (* b_error){
////    myLocalFile.print("Si");myLocalFile.print(",");
////    * b_error     = false;
////  }else{
////    myLocalFile.print("No");myLocalFile.print(",");
////  }
////  myLocalFile.println(b_prog);
////  myLocalFile.close();
////}
//
//
//String logFileText = "Time,JSON,Reset,Error,Códigos de error,Programa";
//
//void writeLogFile(File myLocalFile, String _time, String LogJSON, bool*b_reset, bool* b_error, String* err_code, String b_prog) {
//  bool b_aux;
//  b_aux = b_error;
//
//  // if (b_aux == true) //DESCOMENTAR PARA GUARDAR SOLO CUANDO HAY ERROR
//  // {
//  myLocalFile.print(_time); myLocalFile.print(",");
//  myLocalFile.print(LogJSON); myLocalFile.print(",");
//
//  if (*b_reset) {
//    myLocalFile.print("Si"); myLocalFile.print(",");
//    *b_reset     = false;
//  } else {
//    myLocalFile.print("No"); myLocalFile.print(",");
//  }
//  if (* b_error) {
//    myLocalFile.print("ERROR!!!"); myLocalFile.print(",");
//    * b_error     = false;
//  } else {
//    myLocalFile.print("No"); myLocalFile.print(",");
//  }
//  myLocalFile.print(*err_code);
//  myLocalFile.print(",");
//
//  *err_code = "000  | ";
//  myLocalFile.println(b_prog);
//  // }
//}
//
//String getNameFile() {  //Genera un nombre de achivo a partir de la fecha en formato : YYYYMMDD_HH:MM:SS
//  current_timestamp = rtc.getEpoch();
//  return intToDigits(year(current_timestamp), 4) + intToDigits(month(current_timestamp), 2) + intToDigits(day(current_timestamp), 2) + "_" + intToDigits(hour(current_timestamp), 2) + ":" + intToDigits(minute(current_timestamp), 2) + ":" + intToDigits(second(current_timestamp), 2); //+EXTENSION; (EXTENSION DEL ARCHIVO .csv .txt)
//}
//
//
//void borrarArchivo() {
//
//  String nameFile;
//  Serial.print("Escriba el nombre del archivo a borrar: ");
//  while (!Serial.available());
//
//  nameFile = Serial.readString();
//  nameFile = nameFile.substring(0, nameFile.length());
//  Serial.println(nameFile);
//  pinMode(SD_CS, OUTPUT);
//  digitalWrite(SD_CS, HIGH);
//  SPI.begin();
//  delay(10);
//  // if (card.init(SPI_HALF_SPEED, SD_CS)) {
//  if (SD.begin(SD_CS)) {
//    //volume.init(card);
//    Serial.println("Wiring is correct and a card is present.");
//    if (!SD.exists(nameFile)) Serial.println("File does not exist");
//    else {
//      SD.remove(nameFile);
//      if (!SD.exists(nameFile)) Serial.println("File removed");
//      else Serial.println("Fail removing file");
//    }
//  }
//  else {
//    Serial.println(F("SD Card Error!")); delay(1000);
//    err_code += "| x4000 |";
//    _error = true;
//  }
//  SPI.end();
//  pinMode(SD_CS, INPUT_PULLDOWN);
//}
//
//void leerArchivo() {
//  String nameFile;
//  File localFile;
//  Serial.print("Escriba el nombre del archivo: ");
//  while (!Serial.available());
//  nameFile = Serial.readString();
//  nameFile = nameFile.substring(0, nameFile.length());
//  Serial.println(nameFile);
//  pinMode(SD_CS, OUTPUT);
//  digitalWrite(SD_CS, HIGH);
//  SPI.begin();
//  delay(10);
//  //if (card.init(SPI_HALF_SPEED, SD_CS)) {
//  if (SD.begin(SD_CS)) {
//    //volume.init(card);
//    Serial.println("Wiring is correct and a card is present.");
//    localFile = SD.open(nameFile);
//    if (!localFile) {
//      Serial.println("Error abriendo el archivo: " + String(nameFile));
//      err_code += "| x4100 |";
//      _error = true;
//    }
//    if (localFile) {
//      while (localFile.available()) {
//        Serial.write(localFile.read());
//      }
//      localFile.close();
//    }
//  }
//  else {
//    Serial.println(F("SD Card Error!")); delay(1000);
//    err_code += "| x4000 |";
//    _error = true;
//  }
//  SPI.end();
//  pinMode(SD_CS, INPUT_PULLDOWN);
//}
//
//void listaArchivos() {
//  File localFile;
//  pinMode(SD_CS, OUTPUT);
//  digitalWrite(SD_CS, HIGH);
//  SPI.begin();
//  delay(10);
//  //if (card.init(SPI_HALF_SPEED, SD_CS)) {
//  if (SD.begin(SD_CS)) {
//    //volume.init(card);
//    localFile = SD.open("/");
//    while (true) {
//      File entry =  localFile.openNextFile();
//      if (! entry) {
//        // no more files
//        break;
//      }
//      // Serial.print(entry.name());
//      entry.printName(&Serial);
//      Serial.println();
//      entry.close();
//    }
//    localFile.close();
//  }
//  else {
//    Serial.println(F("SD Card Error!")); delay(1000);
//    err_code += "| x4000 |";
//    _error = true;
//  }
//  SPI.end();
//  pinMode(SD_CS, INPUT_PULLDOWN);
//}
//
//
//
//uint8_t _SaveDataLog_(File outFile, String programa) {  //almacena los datalogs en directorios divididos por dias
//
//  String dayDir;
//  String f_log;
//
//  current_timestamp = rtc.getEpoch();
//  if (year(current_timestamp) < 2021) {
//    Serial.println("Error en la fecha, iniciando actualización");
//    //  insertarComando("OtSV", longitud + 1, programa);
//    return 1;
//  }
//
//  localTime = String(year(current_timestamp));
//  if (month(current_timestamp) > 9) {
//    localTime += ("/" + String(month(current_timestamp)));
//  }
//  else
//    localTime += ("/0" + String(month(current_timestamp)));
//
//  if (day(current_timestamp) > 9) {
//    localTime += ("/" + String(day(current_timestamp)));
//  }
//  else
//    localTime += ("/0" + String(day(current_timestamp)));
//
//  localTime += (" " + String(hour(current_timestamp)));
//  localTime += (":" + String(minute(current_timestamp)));
//  localTime += (":" + String(second(current_timestamp)));
//
//  //NO MODIFICAR ESTE FORMATO SI SE QUIERE UTILIZAR LA FUNCION _findFirstFile_() de SdFat.h
//  dayDir = ("/" + String(year(current_timestamp)));  //la barra corresponde al path
//  if (month(current_timestamp) > 9) {
//    dayDir +=  String(month(current_timestamp));
//  }
//  else
//    dayDir += "0" + String(month(current_timestamp));
//  if (day(current_timestamp) > 9) {
//    dayDir +=  String(day(current_timestamp));
//  }
//  else
//    dayDir += "0" + String(day(current_timestamp));
//
//  placa.RecStatus();
//  placa.EnableConmutado3v();
//
//  Serial.println();
//  Serial.print("Time: "); Serial.print(localTime);
//  Serial.print("\t BoardTemp: "); Serial.print(MeasureTemperature());
//  Serial.print("\t Input V: "); Serial.print(powerIn.busvoltage);
//  Serial.print("\t Input mA: "); Serial.print(powerIn.current_mA);
//  Serial.print("\t Sensor ID: "); Serial.print(localConfig.sensorID);
//  Serial.print("\t SensCh1 (4-20mA): "); Serial.print(sensCh1);
//  Serial.print("\t SensCh2 (4-20mA): "); Serial.print(sensCh2);
//  Serial.print("\t SensCh3 (4-20mA): "); Serial.print(sensCh3);
//  Serial.print("\t SensCh4 (4-20mA): "); Serial.println(sensCh4);
//  Serial.println();
//
//  pinMode(SD_CS, OUTPUT);
//  digitalWrite(SD_CS, HIGH);
//  SPI.begin();
//
//  delay(10);
//
//  if (!SD.begin(SD_CS)) {
//    Serial.println(F("SD Card Error!")); delay(1000);
//    err_code += "| x4000 |";
//    _error = true;
//    SPI.end();
//    pinMode(SD_CS, INPUT_PULLDOWN);
//    placa.SetPrevStatus();
//    return 2;
//  }
//
//  if (!SD.exists(logNameDir)) {  // Si no existe el directorio principal
//    Serial.println("Generando directorio principal de datalog");
//    if (!SD.mkdir(logNameDir)) {
//      Serial.println("Error generando directorio principal del datalog: " + String(logNameDir));
//      err_code += "| x4110 |";
//      _error = true;
//      return 2;
//    }
//    else  //Si se genera
//      Serial.println("Se ha generado el directorio principal del datalog: " + String(logNameDir));
//  }
//
//  //Si ya existia el directorio principal
//  if (!SD.exists(logNameDir + dayDir)) { // Si no existe el directorio diario
//    Serial.println("Generando directorio diario de datalog");
//    if (!SD.mkdir(logNameDir + dayDir)) {
//      Serial.println("Error generando directorio diario de datalog: " + String(logNameDir + dayDir));
//      err_code += "| x4111 |";
//      _error = true;
//      return 2;
//    }
//    else  //Si se genera
//      Serial.println("Se ha generado el directorio diario: " + String(logNameDir + dayDir));
//  }
//
//  //MODIFICAR para indicar el tramo de tiempo que cubre cada archivo
//  f_log = dayDir;
//  //  f_log += String(hour(current_timestamp));
//  //  f_log += String(minute(current_timestamp));
//  //  f_log += String(second(current_timestamp));
//  f_log += (".csv");
//
//  f_log.trim();
//
//  if (!SD.exists(logNameDir + dayDir + "/" + f_log)) { //Si no existe el fichero del log
//    Serial.println("Generando archivo diario");
//    outFile = SD.open(logNameDir + dayDir + "/" + f_log, FILE_WRITE);
//    if (!outFile) {
//      Serial.println("Error abriendo el archivo para escritura: " + String(logNameDir + dayDir + "/" + f_log));
//      err_code += "| x4102 |";
//      _error = true;
//      return 2;
//    } else { //Si se genera
//      outFile.println(logFileText); //Mete los nombres de las columnas
//      outFile.println();
//      Serial.println("Se ha generado el fichero: " + String(f_log) + " | En el directorio: " + String(logNameDir + dayDir));
//      outFile.close();
//    }
//  }
//
//  //Si ya existia
//  outFile = SD.open(logNameDir + dayDir + "/" + f_log, FILE_WRITE);
//  if (!outFile) {
//    Serial.println("Error abriendo el archivo para escritura: " + String(f_log));
//    err_code += "| x4102 |";
//    _error = true;
//    return 2;
//  }
//  else
//  {
//    Serial.println("El localtime de V case 22 es: " + String(localTime));
//
//    //FUNCION QUE ESCRIBE EN EL ARCHIVO GENERADO
//    writeLogFile(outFile, localTime, LogJSON, &_reset, &_error, &err_code, programa);
//    outFile.close();
//    Serial.println("GUARDADO DESDE _SAVEDATALOG_()");
//  }
//
//
//  SPI.end();
//  pinMode(SD_CS, INPUT_PULLDOWN);
//  placa.SetPrevStatus();
//  return 0;
//}
//
//
//
//uint8_t replaceFile(File outFile, const char nameFile[], String body) { //REEMPLAZA EL ARCHIVO SI EXISTIA ANTERIORMENTE, SINO LO GENERA
//
//  //Si ya existia el archivo -> ELIMINAR
//
//  if (SD.exists(nameFile)) {
//    Serial.println("ELIMINANDO ARCHIVO: " + String(nameFile) + " ANTERIOR");
//    outFile = SD.open(nameFile, FILE_WRITE);
//
//    if (outFile.remove())
//      Serial.println("Archivo eliminado correctamente");
//    else {
//      Serial.println("Error al eliminar el archivo");
//      delay(1000);
//      err_code += "| x4103 |";
//      _error = true;
//      SPI.end();
//      pinMode(SD_CS, INPUT_PULLDOWN);
//      placa.SetPrevStatus();
//      return 1;
//    }
//  }
//  else {
//    Serial.println("Generando nuevo archivo: " + String(nameFile));
//    outFile = SD.open(nameFile, FILE_WRITE);
//    if (!outFile) {
//      Serial.println("Error generando el archivo para escritura:" + String(nameFile));
//      err_code += "| x4104 |";
//      _error = true;
//      return 2;
//    } else { //Si se genera
//      outFile.println(body);
//      outFile.println();
//      Serial.println("Se ha generado el fichero:" + String(nameFile));
//      outFile.close();
//
//      return 0;
//    }
//  }
//}
//
//
//
//uint8_t writeUpdateFile(File outFile, String body) {   //genera el archivo binario de actualización =)
//  placa.RecStatus();
//  placa.EnableConmutado3v();
//
//  const char fileUpdate[] = "UPDATE.txt";
//  const char newFileUpdate[] = "newUPDATE.txt";
//
//  pinMode(SD_CS, OUTPUT);
//  digitalWrite(SD_CS, HIGH);
//  SPI.begin();
//
//  delay(10);
//
//  if (!SD.begin(SD_CS)) {
//    Serial.println("SD Card Error!");
//    delay(1000);
//    err_code += "| x4000 |";
//    _error = true;
//    SPI.end();
//    pinMode(SD_CS, INPUT_PULLDOWN);
//    placa.SetPrevStatus();
//    return 1;
//  }
//
//  //Si ya existia el archivo UPDATE -> ELIMINAR
//  uint8_t err_SD = replaceFile(outFile, fileUpdate, body);
//  if (err_SD == 0)
//    err_SD = replaceFile(outFile, newFileUpdate, body);
//
//  if (err_SD != 0 )
//  {
//    return 2;
//  }
//
//  SPI.end();
//  pinMode(SD_CS, INPUT_PULLDOWN);
//  placa.SetPrevStatus();
//  return 0;
//}
//
//void startFTPFile() { //ftp1
//  IPAddress servidorFTP (3, 209, 73, 20);     //(35, 163, 228, 146) //(212,227,247,203)
//  const char *userFTP = "X1test";         //"dlpuser" //"u47750976-ssm-tech "
//  const char *passFTP = "prueba.tech.1";     //"rNrKYTX9g7z3RgJRmxWuGHbeu" //"Qwertyui.2022"
//  const char *fileNameFTP = "ArchivoDesdeSD(28-01-2022).txt";
//  const uint16_t portFTP = 21;
//  delay(1000);
//
//  File outFile;
//
//  String filename_ = "textX1FTP.txt";
//
//  if (!SD.begin(SD_CS)) {
//    Serial.println("Error al iniciar SD, saliendo de busqueda de actualización");
//    delay(1000);
//    err_code += "| x4000 |";
//    _error = true;
//    SPI.end();
//    pinMode(SD_CS, INPUT_PULLDOWN);
//    placa.SetPrevStatus();
//  }
//  else {
//    outFile = SD.open(filename_, FILE_READ );
//
//    int fileSize = outFile.fileSize();
//
//    if (!ftp.connect(servidorFTP, portFTP, userFTP, passFTP)) {
//      Serial.println("Error connecting to FTP server");
//      while (true);
//    }
//    delay(1000);
//
//    Serial.println("Preparando envío");
//
//    int sizeFileSend = ftp.store(fileNameFTP, outFile);
//
//    ftp.stop();
//    outFile.close();
//
//    Serial.println("FileSize: " + String(fileSize));
//    Serial.println("SizeFileSent: " + String(sizeFileSend));
//
//    if (sizeFileSend >= fileSize) {
//      Serial.println("Envio de archivo completado");
//    }
//    else if (sizeFileSend == 0) {
//      Serial.println("!!Error al subir el archivo");
//    }
//    else if (sizeFileSend < fileSize) {
//      Serial.println("!!Error, subida de archivo no completada");
//    }
//
//    Serial.println("FIN");
//
//    while (1);
//  }
//}



//uint8_t findFile(const char nameToFind[], File &fileToReturn) { //busca y devuelve un archivo de la SD
//#ifdef DEBUGCODE
//  Serial.println("Iniciando busqueda del archivo: ");
//  Serial.print(nameToFind);
//#endif
//
//  pinMode(SD_CS, OUTPUT);
//  digitalWrite(SD_CS, HIGH);
//  SPI.begin();
//  delay(10);
//
//  if (SD.begin(SD_CS)) {
//#ifdef DEBUGCODE
//    Serial.println("SD INICIALIZADA");
//#endif
//    if (!SD.exists(nameToFind)) {
//#ifdef DEBUGCODE
//      Serial.println("ERROR!!! ARCHIVO NO ENCONTRADO")
//#endif
//      return 1;
//    }
//    else {
//#ifdef DEBUGCODE
//      Serial.println("Archivo encontrado")
//#endif
//      fileToReturn = SD.open(nameToFind, FILE_READ);       // Open file for reading
//      if (!fileToReturn) {
//        Serial.println("08.000.- Error abriendo el archivo para lectura: " + String(nameToFind));
//        err_code += "| x4101 |";
//        _error = true;
//        return 2;
//      }
//      else {
//        return 0;
//      }
//    }
//  }
//  else {
//#ifdef DEBUGCODE
//    Serial.println(F("ERROR INICIALIZANDO SD!"));
//#endif
//    err_code += "| x4000 |";
//    _error = true;
//    return 3;
//  }
//  //   myDataFile.close();
//  // SPI.end();
//  // pinMode(SD_CS, INPUT_PULLDOWN);
//}
