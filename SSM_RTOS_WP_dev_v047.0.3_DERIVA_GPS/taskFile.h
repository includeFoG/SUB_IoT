//SdFat sd;

//String givePrompt(time_t currentTime); //ADDED COMENTADO
//String givePrompt(time_t currentTime, char * typeDir);//ADDED COMENTADO
#ifdef DEBUG_LOGEVENTS
//int count_ = 0; //ADDED LOG
#endif



int powerOnSD()
{
  xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr | BIT_2_fileModule );
  myDelayMs(10);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  SPI.begin();
  myDelayMs(10);
  if ( sd.begin(SD_CS))
  {
    return -1;
  }
  return 0;
}


void powerOffSD()
{
  SPI.end();
  pinMode(SD_CS, INPUT_PULLDOWN);
  xEventGroupClearBits( eventPwr5volts, BIT_2_fileModule );
  xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr  );
  myDelayMs(1);
}


/*//////////////////////////////////////////////////////////////////////////////////////ADDED/////////////////////////////////////////////////
  //
  //
  //void createConfigFile( File configFile, configData* _config) { //ADDED
  //
  //  configFile.print("APN: "); configFile.println(_config->APN);
  //  configFile.print("APN_USER: "); configFile.println(_config->APN_USER);
  //  configFile.print("APN_PASS: "); configFile.println(_config->APN_PASS);
  //
  //  configFile.print("DEVICE: "); configFile.println(_config->DEVICE);
  //  configFile.print("PATH: "); configFile.println(_config->PATH);
  //  configFile.print("HOST: "); configFile.println(_config->HOST);
  //  configFile.print("TOKEN: "); configFile.println(_config->TOKEN);
  //  configFile.print("BUCKET: "); configFile.println(_config->BUCKET);
  //  configFile.print("APIPATH: "); configFile.println(_config->APIPATH);
  //
  //  configFile.print("ARCHIVO: "); configFile.println(_config->ARCHIVO);
  //
  //  configFile.print("SLOT_TIME: "); configFile.println(_config->SLOT_TIME);
  //
  //  configFile.print("MEASURING_TIME: "); configFile.println(_config->MEASURING_TIME);
  //  configFile.print("MEASURE_PERIOD: "); configFile.println(_config->MEASURE_PERIOD);
  //  configFile.print("SEND_PERIOD: "); configFile.println(_config->SEND_PERIOD);
  //  configFile.print("GPS_PERIOD: "); configFile.println(_config->GPS_PERIOD);
  //  configFile.print("FILE_PERIOD: "); configFile.println(_config->FILE_PERIOD);
  //  configFile.print("FILE_TIME2: "); configFile.println(_config->FILE_TIME2);
  //
  //  configFile.print("POWER_SENSOR_DELAY: "); configFile.println(_config->POWER_SENSOR_DELAY);
  //  configFile.print("EXTENCION: "); configFile.println(_config->EXTENCION);
  //  configFile.print("RETRY_CONNECTION: "); configFile.println(_config->RETRY_CONNECTION);
  //  configFile.print("SIZE_BUFFER: "); configFile.println(_config->SIZE_BUFFER);
  //  configFile.print("ROOTLOG: "); configFile.println(_config->ROOTLOG);
  //  configFile.print("DATALOG: "); configFile.println(_config->DATALOG);
  //  configFile.print("CHECK_UPDATE_TIME:"); configFile.println(_config->CHECK_UPDATE_TIME);
  //}
  //
  //
  //
  //void readNoString(File fileToRead, char* p_valor, char c = '\r') { //ADDED
  //  char c_;
  //  bool f_read = false;
  //
  //  //Serial.print("Sale: ");
  //  while (*p_valor != c) {
  //    if (f_read == true)
  //      p_valor++;
  //    c_ = fileToRead.read();
  //    *p_valor = c_;
  //    // Serial.print(p_valor);
  //    f_read = true;
  //  }
  //  *p_valor = '\0';
  //  //Serial.println();
  //}
  //
  //
  //
  //void readConfigFile(File configFile, configData* _localConfig) {
  //
  //  char initializer[] = "";
  //
  //  String S_aux = "";
  //
  //  memset(_localConfig->APN, 0, 64); //NECESARIO PARA FIJAR VALORES DE v_byte a 0  uint8_t v_byte[DEF_SIZE_BUFFER];
  //  memset(_localConfig->APN_USER, 0, 32);
  //  memset(_localConfig->APN_PASS, 0, 32);
  //  memset(_localConfig->DEVICE, 0, 32);
  //  memset(_localConfig->PATH, 0, 64);
  //  memset(_localConfig->HOST, 0, 64);
  //  memset(_localConfig->TOKEN, 0, 64);
  //  memset(_localConfig->BUCKET, 0, 64);
  //  memset(_localConfig->APIPATH, 0, 64);
  //  memset(_localConfig->ARCHIVO, 0, 16);
  //  memset(_localConfig->EXTENCION, 0, 16);
  //  memset(_localConfig->ROOTLOG, 0, 16);
  //  memset(_localConfig->DATALOG, 0, 16);
  //
  //  _localConfig->SLOT_TIME = 0;
  //  _localConfig->MEASURING_TIME = 0;
  //  _localConfig->MEASURE_PERIOD = 0;
  //  _localConfig->SEND_PERIOD = 0;
  //  _localConfig->GPS_PERIOD = 0;
  //  _localConfig->FILE_PERIOD = 0;
  //  _localConfig->FILE_TIME2 = 0;
  //  _localConfig->POWER_SENSOR_DELAY = 0;
  //  _localConfig->RETRY_CONNECTION = 0;
  //  _localConfig->SIZE_BUFFER = 0;
  //  _localConfig->CHECK_UPDATE_TIME = 0;
  //
  //  DEBUG("\\\\\\\\\\\\\\\\\\\\\\\\\\READCONFIGFILE\\\\\\\\\\\\\\\\\\\\\\");
  //  configFile.readStringUntil(' ');
  //  readNoString(configFile, _localConfig->APN);
  //
  //  //DEBUG("APN:" + String(_localConfig->APN)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  readNoString(configFile, _localConfig->APN_USER);
  //  //DEBUG("APN_USER:" + String(_localConfig->APN_USER)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  readNoString(configFile, _localConfig->APN_PASS);
  //  //DEBUG("APN_PASS:" + String(_localConfig->APN_PASS)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  readNoString(configFile, _localConfig->DEVICE);
  //  //DEBUG("DEVICE:" + String(_localConfig->DEVICE)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  readNoString(configFile, _localConfig->PATH);
  //  //DEBUG("PATH:" + String(_localConfig->PATH)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  readNoString(configFile, _localConfig->HOST);
  //  //DEBUG("HOST:" + String(_localConfig->HOST)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  readNoString(configFile, _localConfig->TOKEN);
  //  //DEBUG("TOKEN:" + String(_localConfig->TOKEN)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  readNoString(configFile, _localConfig->BUCKET);
  //  //DEBUG("BUCKET:" + String(_localConfig->BUCKET)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  readNoString(configFile, _localConfig->APIPATH);
  //  //DEBUG("APIPATH:" + String(_localConfig->APIPATH)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  readNoString(configFile, _localConfig->ARCHIVO);
  //  //DEBUG("ARCHIVO:" + String(_localConfig->ARCHIVO)); //configFile.readStringUntil('\r').c_str();
  //
  //  configFile.readStringUntil(' ');
  //  _localConfig->SLOT_TIME = configFile.readStringUntil('\r').toInt();
  //  //DEBUG("SLOT_TIME:" + String(_localConfig->SLOT_TIME)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  _localConfig->MEASURING_TIME = configFile.readStringUntil('\r').toInt();
  //  //DEBUG("MEASURING_TIME:" + String(_localConfig->MEASURING_TIME)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  _localConfig->MEASURE_PERIOD = configFile.readStringUntil('\r').toInt();
  //  //DEBUG("MEASURE_PERIOD:" + String(_localConfig->MEASURE_PERIOD)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  _localConfig->SEND_PERIOD = configFile.readStringUntil('\r').toInt();
  //  //DEBUG("SEND_PERIOD:" + String(_localConfig->SEND_PERIOD)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  _localConfig->GPS_PERIOD = configFile.readStringUntil('\r').toInt();
  //  //DEBUG("GPS_PERIOD:" + String(_localConfig->GPS_PERIOD)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  _localConfig->FILE_PERIOD = configFile.readStringUntil('\r').toInt();
  //  //DEBUG("FILE_PERIOD:" + String(_localConfig->FILE_PERIOD)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  _localConfig->FILE_TIME2 = configFile.readStringUntil('\r').toInt();
  //  //DEBUG("FILE_TIME2:" + String(_localConfig->FILE_TIME2)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  _localConfig->POWER_SENSOR_DELAY = configFile.readStringUntil('\r').toInt();
  //  //DEBUG("POWER_SENSOR_DELAY:" + String(_localConfig->POWER_SENSOR_DELAY)); //configFile.readStringUntil('\r').c_str();
  //
  //  configFile.readStringUntil(' ');
  //  readNoString(configFile, _localConfig->EXTENCION);
  //  //DEBUG("EXTENCION:" + String(_localConfig->EXTENCION)); //configFile.readStringUntil('\r').c_str();
  //
  //  configFile.readStringUntil(' ');
  //  _localConfig->RETRY_CONNECTION = configFile.readStringUntil('\r').toInt();
  //  //DEBUG("RETRY_CONNECTION:" + String(_localConfig->RETRY_CONNECTION)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  _localConfig->SIZE_BUFFER = configFile.readStringUntil('\r').toInt();
  //  //DEBUG("SIZE_BUFFER:" + String(_localConfig->SIZE_BUFFER)); //configFile.readStringUntil('\r').c_str();
  //
  //  configFile.readStringUntil(' ');
  //  readNoString(configFile, _localConfig->ROOTLOG);
  //  //DEBUG("ROOTLOG:" + String(_localConfig->ROOTLOG)); //configFile.readStringUntil('\r').c_str();
  //  configFile.readStringUntil(' ');
  //  readNoString(configFile, _localConfig->DATALOG);
  //  DEBUG("DATALOG:" + String(_localConfig->DATALOG)); //configFile.readStringUntil('\r').c_str();
  //
  //  configFile.readStringUntil(' ');
  //  _localConfig->CHECK_UPDATE_TIME = configFile.readStringUntil('\r').toInt();
  //  DEBUG("CHECK_UPDATE_TIME:" + String(_localConfig->CHECK_UPDATE_TIME)); //configFile.readStringUntil('\r').c_str();
  //
  //  DEBUG("/////////////////////////////////////////");
  //
  //}
  //
  //
  //
  //
  //bool initConfigFile(File configFile) { //Comprobar si existe archivo de configuración en la SD y si no existe lo genera a partir del DEF =)
  //  Serial.println("initconfigfile...");
  //
  //  if (!sd.exists(configName)) {
  //    Serial.println("CREANDO CONFIGFILE");
  //    configFile = sd.open(configName, FILE_WRITE);
  //    if (!configFile)
  //      return false;
  //    createConfigFile(configFile, &defaultConfig);
  //    configFile.close();
  //    Serial.println("CONFIGFILE CREADO");
  //  }
  //  else {
  //    Serial.println("Archivo de configuración encontrado en SD");
  //  }
  //
  //  configFile = sd.open(configName, FILE_READ);
  //
  //  if (!configFile) {
  //    Serial.println("ERROR AL ABRIR CONFIGFILE");
  //    return false; //!!!CREAR EXCEPCION EN RETORNO
  //  }
  //
  //  else {
  //    Serial.println("1: " + String((int) & (*localConfig.APN)));
  //    dynAllocCharStructConfig();
  //    Serial.println("2: " + String((int) & (*localConfig.APN)));
  //
  //    readConfigFile(configFile, &localConfig);
  //    configFile.close();
  //
  //
  //    //////////////////DEBUG/////////////////////////
  //    configFile = sd.open(configName, FILE_READ);
  //    Serial.println();
  //    Serial.println("////////////////////////////DEFCONFIGFILE://///////////////////////////");
  //    if (!configFile)
  //      return false;
  //    while (configFile.available()) {
  //      Serial.write(configFile.read());
  //    }
  //    configFile.close();
  //    Serial.println("////////////////////////////////////////////////////////////////////");
  //    Serial.println();
  //    /////////////////////////////////////////////////
  //  }
  //
  //  return true;
  //}
  //
*///////////////////////////////////////////////////////////////////////////////////////////ENDADDED/////////////////////////////////////////////////////////////



TaskHandle_t Handle_fileTask;
int contFile = 0;

int listFile();
//void addNameToSend(String fileName, String logFile);
void addNameToSend(const char *fileName, const char* logFile, File &ptrFile);
//void readFileToSend (String fileName);
void readFileToSend (const char *fileName, File& ptrFile);
//int deleteFirstName(String logFile);
int deleteFirstName(const char *logFile, File &ptrFile);
//String printRouteFile(String &myNameFile, char *typeDir);
bool printRouteFile(const char* myNameFile, const char* typeDir, String &rootString, bool create);
//String openFileToSend (String fileStore, int  j );
//String openFileToSend (const char *fileStore, int  j , File &ptrFile);
bool openFileToSend (const char *fileStore, int  j , File &ptrFile , String &temp); //NUEVA

uint8_t countCreateFile = 0;

bool emptyFileToSend = false;

static void threadFile( void *pvParameters )
{

  //bool emptyFileToSend = false;
  if (!powerOnSD())
  {
    SERIAL.println("SD card fail, or not present");
    powerOffSD();

    xEventGroupClearBits( eventTaskState,  BIT_3_taskFile ); //ADDED WORKING

    vTaskDelete(NULL);
  }
  DEBUG("SD card initialized.");
  File dataFile;    //escritura de sensores
  File sendFile;    //archivo a enviar al commthread
  File filesToSend; //NUEVO

  //  File configFile; //ADDED CONFIG
  //  File AWSConfigFile; //ADDED CONFIG

#ifdef DEBUG_LOGEVENTS
  File eventsLogFile; //ADDED LOG
  String auxRootEvent = ""; //ADDED string usado para printRouteFile (caso event)
  String fileEventsLogName = ""; //ADDED LOG
  byte b_logEvent = NULL; //recepcion de cola logevents
  char cabeceraLogEvent[31] = "Time, State, Error, ErrorCode";
  byte b_infoLog = NULL;
  bool f_existFileLogEvents = false;
#endif


  String fileName = "";
  String logFileName = "";
  String sendFileName = "";
  String auxRootFile = ""; //ADDED string usado para printRouteFile
  const char* storeFiles = "fToSend.txt"; //ANTES: static String
  String auxRootSend = ""; //ADDED string usado para printRouteFile (caso envío)

  struct qMessage localData;
  //String aux_config = ""; //ADDED CONFIG

  //ADDED CONFIG
  //  if (initConfigFile(configFile) == false) //Añadir al configurar configSD
  //    Serial.println("error al iniciar configfile");
  //  else {
  //    Serial.println("InitConfigFile true");
  //    //    Serial.println("Prueba , localConfig.MEASURING_TIME:" + String(localConfig.MEASURING_TIME));
  //    //    Serial.println("Prueba , localConfig.BUCKET:" + String(localConfig.BUCKET));
  //    //    Serial.println("Prueba , localConfig.TOKEN:" + String(localConfig.TOKEN));
  //    //    Serial.println("Prueba , localConfig.APIPATH:" + String(localConfig.APIPATH));
  //
  //    xEventGroupSetBits( myEvents, BIT_21_readyConfig0);
  //    Serial.println("BIT_21_readyConfig0 Set to 1");
  //  }
  //  Serial.println();
  //ENDADDED

  DEBUG("Starting logging");
  time_t currentTime = rtc.getEpoch();
  //unsigned long fileTime = 0;
  byte stateSend = 0 ;
  String temp = "";

  //ADDED CONFIG
  //  uint8_t v_byte[DEF_SIZE_BUFFER]; //!!! localconfig? usado con BIT_19_
  //  uint8_t *p_to_v_byte = &v_byte[0]; //usado con BIT_19_
  //  int len = 0; //usado con BIT_19_
  //ENDADDED


  static const int auxBinStatus = BIT_0_createFile | BIT_10_saveData | BIT_9_sendFile | BIT_14_doneFileRead | BIT_19_IddleYNoSend; //ADDED  //WAIT4 B000010000100011000000001


  while ( 1 )
  {
    xEventGroupSetBits( eventTaskState,  BIT_3_taskFile ); //ADDED WORKING
    //    TickType_t lastWakeTime = xTaskGetTickCount();
    EventBits_t uxBits = xEventGroupGetBits( myEvents );


    /*  /////////////////////////////////////////////////////////////////////////ADDED/////////////////////////////////////////////////////////////
      //
      //
      //
      //    if ((uxBits & BIT_18_createConfigAWS) != 0 ) //=)
      //    {
      //      fileName = "configAWS.txt";
      //      if (!powerOnSD())
      //      {
      //        DEBUG("SD card failed, or not present");
      //        powerOffSD();
      //        vTaskDelete(NULL);
      //      }
      //
      //      Serial.println("CREATE AWS CONFIG FILE");
      //
      //      if (AWSConfigFile)
      //      {
      //        Serial.println("ARCHIVO DE CONFIGURACION AWS ABIERTO PREVIAMENTE, cerrando");
      //        AWSConfigFile.close();
      //      }
      //
      //
      //      if ( sd.exists(fileName))
      //      {
      //        Serial.println("File exist, removing:" + fileName);
      //        if (AWSConfigFile)
      //          AWSConfigFile.close();
      //
      //        sd.remove(fileName);
      //
      //        if (!sd.exists(fileName)) Serial.println("File removed");
      //        else Serial.println("Fail removing file");
      //      }
      //      //      Serial.println("CORTA<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
      //      //      myDelayMs(3000);
      //      //      vTaskSuspend(NULL);
      //      //      while (1);
      //
      //      Serial.println("Creating new File: " + fileName);
      //
      //      AWSConfigFile = sd.open(fileName, FILE_WRITE);
      //      myDelayMs(5);
      //
      //      if (AWSConfigFile) {
      //        xEventGroupClearBits( myEvents,  BIT_18_createConfigAWS );
      //        Serial.println("BIT_18_createConfigAWS Set to 0");
      //      }
      //    }
      //
      //    if ((uxBits & BIT_19_saveConfigAWS) != 0)
      //    {
      //      Serial.println("TASKFILE_19_");
      //      int sizeConfigFile = 0;
      //
      //      memset(v_byte, 0, DEF_SIZE_BUFFER); //NECESARIO PARA FIJAR VALORES DE v_byte a 0  uint8_t v_byte[DEF_SIZE_BUFFER];
      //      len = 0;
      //
      //      //READ FILE SIZE
      //      xQueueReceive( xConfigQueue, p_to_v_byte, ( TickType_t ) 500 );  //  uint8_t *p_to_v_byte = &v_byte[0];
      //      sizeConfigFile = v_byte[0];
      //      xQueueReceive( xConfigQueue, p_to_v_byte, ( TickType_t ) 500 );
      //      sizeConfigFile |= v_byte[0] << 8;
      //      xQueueReceive( xConfigQueue, p_to_v_byte, ( TickType_t ) 500 );
      //      sizeConfigFile |= v_byte[0] << 16;
      //      xQueueReceive( xConfigQueue, p_to_v_byte, ( TickType_t ) 500 );
      //      sizeConfigFile |= v_byte[0] << 24;
      //      //Serial.println("FileSize READ: "+String(sizeConfigFile)); //añadido +1 en taskcomm
      //      Serial.println(sizeConfigFile);
      //
      //      while (sizeConfigFile != 0)
      //      {
      //        while ((xQueueReceive( xConfigQueue, p_to_v_byte + len, ( TickType_t ) 10 ) == pdPASS) && (sizeConfigFile > 0)) //devuelve pdTRUE si se recibió correctamente un elemento de la cola; de lo contrario, pdFALSE. pdPASS contiene un mensaje
      //        {
      //          //   Serial.println("\r\nsizeConfigFile: "+String(sizeConfigFile));
      //          len++; //aumentamos en 1 el índice de  v_byte
      //          sizeConfigFile--; //disminuimos el tamaño del archivo
      //
      //          //Serial.println("================= el numero de datos que se han recibido es:"+String(len));
      //
      //          if ( len == DEF_SIZE_BUFFER) //cuando se llegue al tamaño del buffer-1
      //          {
      //
      //            if (AWSConfigFile) {
      //              Serial.println("Guardando en archivo : ");
      //              Serial.write(v_byte, len);
      //              Serial.println();
      //
      //              AWSConfigFile.write(v_byte, len);
      //              memset(v_byte, 0, DEF_SIZE_BUFFER);
      //              len = 0;
      //            }
      //            else
      //              DEBUG("EL ARCHIVO CONFIG ESTA CERRADO");
      //
      //            break;
      //          }
      //        }
      //
      //        if ( len > 0) //si aún queda algo en el buffer que no sea un buffer completo, después de hacer la toma de datos de la cola
      //        {
      //          if (AWSConfigFile) {
      //            Serial.println("Guardando en archivo ultimo paquete de tamaño: " + String(len));
      //            Serial.write(v_byte, len);
      //            Serial.println();
      //
      //            //             Serial.write(c, len);
      //            //            Serial.println();
      //            AWSConfigFile.write(v_byte, len);
      //            myDelayMs(5);
      //            len = 0;
      //          }
      //          if (sizeConfigFile != 0) {
      //            Serial.println("Archivo no completado, sizeconfigfile: " + String(sizeConfigFile));
      //            // break;
      //          }
      //        }
      //      }
      //
      //      memset(v_byte, 0, DEF_SIZE_BUFFER);
      //      Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>EL ARCHIVO SE HA COMPLETADO");
      //
      //      if (AWSConfigFile) {
      //        Serial.println("Cerrando archivo AWSConfigFile");
      //        AWSConfigFile.close();
      //      }
      //
      //      Serial.println("Iniciando sustitución de archivos  configAWS.txt -> configRTO.txt");
      //
      //      if (sd.exists(configName)) {
      //        sd.remove(configName);
      //        if (!sd.exists(configName)) {
      //          Serial.println("Antiguo Config eliminado");
      //          xEventGroupClearBits( myEvents, BIT_21_readyConfig0);
      //          Serial.println("BIT_21_readyConfig0 Set to 0");
      //        }
      //        else
      //          Serial.println("ERROR al elimnar antiguo config");  //!!! meter retry?
      //      }
      //
      //      if (AWSConfigFile = sd.open( fileName, O_WRONLY | O_CREAT)) {
      //        if (!AWSConfigFile.rename(configName)) {
      //          Serial.println("ERROR al renombrar el archivo");
      //        }
      //        else
      //          Serial.println("Archivo: " + String(fileName) + "    Renombrado como: " + configName);
      //        AWSConfigFile.close();
      //      }
      //      else
      //        Serial.println("ERROR_F1"); ///!!! necesario controlar la salida
      //
      //      //        if (initConfigFile(configFile) == false) //=)
      //      //    Serial.println("error al iniciar configfile");
      //      //  else {
      //      //    Serial.println("InitConfigFile true");
      //      //    //    Serial.println("Prueba , localConfig.MEASURING_TIME:" + String(localConfig.MEASURING_TIME));
      //      //    //    Serial.println("Prueba , localConfig.BUCKET:" + String(localConfig.BUCKET));
      //      //    //    Serial.println("Prueba , localConfig.TOKEN:" + String(localConfig.TOKEN));
      //      //    //    Serial.println("Prueba , localConfig.APIPATH:" + String(localConfig.APIPATH));
      //      //
      //      //    xEventGroupSetBits( myEvents, BIT_21_readyConfig0);
      //      //    Serial.println("BIT_21_readyConfig0 Set to 1");
      //      //  }
      //      //  Serial.println();
      //
      //      //      if (configFile = sd.open(configName, FILE_READ)) //!!! esto debería ser un readconfigfile ver 486
      //      if (sd.exists(configName)) {
      //      //        char _c;
      //      //        while (configFile.available()) {
      //      //          _c = configFile.read();
      //      //          //Serial.print(_c);
      //      //        }
      //      xEventGroupSetBits( myEvents, BIT_21_readyConfig0);
      //        Serial.println("BIT_21_readyConfig0 Set to 1");
      //
      //      }
      //      else {
      //        Serial.println("ERROR_F2");
      //      }
      //      xEventGroupClearBits( myEvents, BIT_19_saveConfigAWS);
      //      Serial.println("BIT_19_saveConfigAWS Set to 0");
      //    }
    *///

#ifdef DEBUG_LOGEVENTS
    if ( xLogEventsQueue != NULL)  //ADDEDLOG
    {
      if (xQueueReceive( xLogEventsQueue, &b_logEvent, 0) == pdPASS)
      {
        time_t t_eventsLog = (time_t) rtc.getEpoch();
        fileEventsLogName = getNameFile(t_eventsLog).substring(0, 8);
        fileEventsLogName += "_EventsLog.txt"; //Obtiene nombre de archivo ej:"20220420_021330.csv" -> toma :"20220420" -> añade: "20220420_EventsLog.txt"
        auxRootEvent = "";

        if (!printRouteFile(fileEventsLogName.c_str(), EVENTSLOG, auxRootEvent, true)) {
#ifdef DEBUG_LOGEVENTS //solo es valido para (,,true)
          b_infoLog = B10010000; //ERROR EN LA GENERACION DE CARPETA
          sendToEventsLogQueue(b_infoLog);
#endif
        } //!!! FALTA GESTION DE ERROR


        if (sd.begin()) {

          f_existFileLogEvents = sd.exists(auxRootEvent);

          taskYIELD();
          taskENTER_CRITICAL();

          if (eventsLogFile.isOpen()) eventsLogFile.close();
          eventsLogFile = sd.open(auxRootEvent, FILE_WRITE); //BOOL TRUE

          if (eventsLogFile.isOpen()) {

            if (!f_existFileLogEvents) { // SI NO EXISTIA
              DEBUG("Creating File EventsLog: " + auxRootEvent);
              eventsLogFile.println(cabeceraLogEvent);
            }

            byte aux_byte = NULL;
            //            char s_timeEventsLog[11];
            //            strcat(s_timeEventsLog,intToDigits(hour(t_eventsLog), 4).c_str());
            //            strcat(s_timeEventsLog, ":");
            //            strcat(s_timeEventsLog,intToDigits(minute(t_eventsLog), 2).c_str());

            String s_timeEventsLog = intToDigits(hour(t_eventsLog), 4) + ':' + intToDigits(minute(t_eventsLog), 2) + ':' + intToDigits(second(t_eventsLog), 2);

            //eventsLogFile.write(s_timeEventsLog,11);
            eventsLogFile.print(s_timeEventsLog);
            eventsLogFile.print(",");
            DEBUG("File EventsLog");
            DEBUG("          Event: " + String(b_logEvent, BIN));

            aux_byte = b_logEvent >> 7;  //localiza flag de error en el byte

            if (aux_byte != 0) {
              eventsLogFile.print("-,Y,");
              eventsLogFile.print(b_logEvent, (BIN));
              DEBUG("          |ERROR: " + String(aux_byte, BIN));
            }
            else {
              eventsLogFile.print(b_logEvent, (BIN));
              eventsLogFile.print(",N,-");
            }

            eventsLogFile.println();

            eventsLogFile.close();
            taskEXIT_CRITICAL();
          } else {
            DEBUG("ERROR AL ABRIR EVENTLOGFILE");
          }
        }
        else
          DEBUG("ERROR AL INICIAR SD");
      }
    }
#endif


    /////////////////////////////////////////////////////////////////////////ENDADDED/////////////////////////////////////////////////////////
    //    if ((uxBits & BIT_6_listFile) != 0)
    //    {
    //      DEBUG("list");
    //      listFile();
    //      SERIAL.println("done");
    //      xEventGroupClearBits( myEvents,  BIT_6_listFile );
    //    }

    if ((uxBits & BIT_0_createFile) != 0)
    {
      /*// xEventGroupClearBits( myEvents,  BIT_0_createFile ); //ADDED movido a zona de creación de archivos
        //      DEBUG("2===========================================================> " + String(auxRootFile));
        //      //if (dataFile.isOpen())
        //      if (auxRootFile != "")
        //      {
        //        DEBUG("CREATE FILE");
        //        dataFile.close();
        //
        //        //ADDED
        //#ifdef DEBUG_LOGEVENTS
        //        b_infoLog = B00000001;  //ENDFILE
        //        sendToEventsLogQueue(b_infoLog);
        //#endif
        //        //ENDADDED
        //
        //        if ((uxBits & BIT_17_snapFileToSend) != 0)
        //        {
        //          //addNameToSend(storeFiles, storeFiles);
        //          if ((storeFiles.length() >= 10) && (storeFiles.length() <= 20 )) { //tamaños de ftosend.txt y 20220506_090307.csv
        //            addNameToSend(storeFiles.c_str(), storeFiles, filesToSend);//NUEVO            addNameToSend(storeFiles, storeFiles);
        //          }
        //          else {
        //            DEBUG ( "ERROR ADD NAME, error name");
        //          }
        //          xEventGroupClearBits( myEvents,  BIT_17_snapFileToSend );
        //        }
        //        if ((fileName.length() >= 10) && (fileName.length() <= 20 )) { //tamaños de ftosend.txt y 20220506_090307.csv
        //          addNameToSend(fileName.c_str(), storeFiles, filesToSend); //addNameToSend(fileName, storeFiles);
        //        }
        //        else {
        //          DEBUG ( "ERROR ADD NAME, error name");
        //        }
        //        emptyFileToSend = false;
        //
        //        auxRootFile = "";
        //      }*/

      /*    //#ifdef SENDFILE
          //      if ((uxBits & BIT_12_rdyFile) == 0 )
          //      {
          //        stateSend = 0;
          //        xQueueReset( xTxBuffer );
          //
          //        //        sendFileName = fileName;
          //        readFileToSend(storeFiles);
          //        sendFileName = openFileToSend (storeFiles, 1);
          //        DEBUG("SENDFILENAME " + sendFileName + " LENGTH" + String(sendFileName.length()));
          //        if ( sendFileName.length() > 10 )
          //        {
          //          if ( sendFileName.endsWith("txt"))
          //          {
          //            DEBUG ("PREPARE TO SEND: " + sendFileName) ;
          //            DEBUG ("SET BIT 12 ***");
          //            sendFile = sd.open(sendFileName, FILE_READ);
          //          }
          //          else
          //          {
          //            DEBUG ("PREPARE TO SEND: " + printRouteFile(sendFileName, DATALOG));
          //            DEBUG ("SET BIT 12");
          //            sendFile = sd.open(printRouteFile(sendFileName, DATALOG), FILE_READ);
          //          }
          //          if ( sendFile)
          //          {
          //            xEventGroupSetBits( myEvents,  BIT_12_rdyFile );
          //            //DEBUG ("SET BIT_12_rdyFile  <----------");
          //            sendFile.close();
          //          }
          //          else
          //          {
          //            DEBUG ("FAILING READING THIS FILE");
          //            switch (deleteFirstName(storeFiles) != 0) //ADDED MODIF
          //            {
          //              case 0:
          //                break;
          //#ifdef DEBUG_LOGEVENTS
          //              case -1:
          //                b_infoLog = B10001010;  //ERROR en deletefirstname
          //                sendToEventsLogQueue(b_infoLog);
          //                break;
          //              case 2:
          //                b_infoLog = B10001011;  //ERROR tempfile>ptrfile
          //                sendToEventsLogQueue(b_infoLog);
          //                break;
          //              case 3:
          //                b_infoLog = B10001100;  //ERROR al borrar-renombrar archivo
          //                sendToEventsLogQueue(b_infoLog);
          //                break;
          //#endif
          //              default:
          //                break
          //            }
          //          }
          //        }
          //      }
      */    //#endif


      currentTime = rtc.getEpoch();
      fileName = getNameFile(currentTime).c_str(); //Obtiene nombre de archivo ej:"20220420_021330.csv"
      SERIAL.print("FileName: ");
      SERIAL.println(fileName);

      auxRootFile = "";

      if (!printRouteFile(fileName.c_str(), DATALOG, auxRootFile, true)) {
#ifdef DEBUG_LOGEVENTS //solo es valido para (,,true)
        b_infoLog = B10010000; //ERROR EN LA GENERACION DE CARPETA
        sendToEventsLogQueue(b_infoLog);
#endif
      }// no necesita gestion ya que despues lo controla un exists

      SERIAL.print("auxRootF> ");
      SERIAL.println(String(auxRootFile));
      if (auxRootFile.length() == 37) //Seguro de tamaño string (ruta completa)
      {
        //DEBUG("CUMPLE CONDICION == 37 =====================");
        if ( sd.exists(auxRootFile.c_str())) //ADDED MODIF comprueba si existe el archivo  "/20220420/DataLog/20220420_021320.csv"
        {
          //DEBUG("File exist: " + prompt + fileName);
          if (!sd.remove(auxRootFile.c_str())) { //ADDED MODIF si ya existe lo elimina
#ifdef DEBUG_LOGEVENTS
            b_infoLog = B10001100;  //ERROR al borrar archivo
            sendToEventsLogQueue(b_infoLog);
#endif
          }
        }

        taskYIELD();
        taskENTER_CRITICAL();
        SERIAL.print("FILE: NEW File CREATED: ");
        SERIAL.println(auxRootFile); //ADDED MODIF

        if (dataFile.isOpen()) dataFile.close();
        dataFile = sd.open(auxRootFile.c_str(), FILE_WRITE); //ADDED MODIF genera el archivo con ruta "/20220420/DataLog/20220420_021320.csv"  //BOOL TRUE


        if (dataFile.isOpen())//ADDED
        {
          dataFile.print("Time[us],");

          for (int i = 0; i < NUM_VALORES; i++)
          {
            dataFile.print("adc_");
            if ( i < 10 ) dataFile.print("0");
            dataFile.print(String(i) + "[mA]");
            if ( i < NUM_VALORES - 1 )
              dataFile.print(",");
            else
              dataFile.println();
          }


          DEBUG("2===========================================================> " + String(auxRootFile));
          //if (dataFile.isOpen())dataFile.close();

          DEBUG("FILE: CREATE FILE");
          dataFile.close();   //!!!HABRIA QUE AÑADIR UN DATAFILE.CLOSE, DE MOMENTO NO SE LE AÑADE PARA PODER DEBUGUEAR EL NOMBRE 1 UNICA VEZ EN EL ADDNAMETOFILE

          //ADDED
#ifdef DEBUG_LOGEVENTS
          b_infoLog = B00000001;  //ENDFILE
          sendToEventsLogQueue(b_infoLog);
#endif
          //ENDADDED

          if ((uxBits & BIT_17_snapFileToSend) != 0)
          {
            if ((strlen(storeFiles) >= 10) && (strlen(storeFiles) <= 21 )) { //if ((storeFiles.length() >= 10) && (storeFiles.length() <= 20 )) { //tamaños de ftosend.txt y 20220506_090307.csv aunque storeFiles es un const: ftosend.txt
              addNameToSend(storeFiles, storeFiles, filesToSend);//NUEVO            addNameToSend(storeFiles, storeFiles);
            }
            else {
              DEBUG ( "FILE: ERROR ADD NAME, error name");
            }
            xEventGroupClearBits( myEvents,  BIT_17_snapFileToSend );
          }
          if ((fileName.length() >= 10) && (fileName.length() <= 20 )) { //tamaños de ftosend.txt y 20220506_090307.csv
            addNameToSend(fileName.c_str(), storeFiles, filesToSend); //addNameToSend(fileName, storeFiles);
          }
          else {
            DEBUG ( "FILE: ERROR ADD NAME, error name");
          }
          emptyFileToSend = false;

#ifdef DEBUG_LOGEVENTS
          b_infoLog = B00001101;  //NUEVO archivo de datos generado
          sendToEventsLogQueue(b_infoLog);
#endif

          xEventGroupClearBits( myEvents,  BIT_0_createFile ); //ADDED
          countCreateFile = 0;
        }
        else //ADDED MODIF si no se abre el archivo hacer 2 reintentos si no se abre hace clear del bit createfile
        {
          if (countCreateFile < MAX_RETRAY)
            countCreateFile++;
          else {
#ifdef DEBUG_LOGEVENTS
            b_infoLog = B10001101;  //ERROR generando archivo datos
            sendToEventsLogQueue(b_infoLog);
#endif
            xEventGroupClearBits( myEvents,  BIT_0_createFile );  //!!! hay que hacer algo para que no entre en BIT_10_saveData
            countCreateFile = 0;
          }
        }

        taskEXIT_CRITICAL();
      }

      else
      {
        SERIAL.print("axRoFi: ");
        SERIAL.println(auxRootFile);
        SERIAL.println(auxRootFile.length());
        SERIAL.println("ERROR DE MEMORIA NO SE HA FORMADO BIEN EL NOMBRE DE ARCHIVO!");
#ifdef DEBUG_LOGEVENTS
        b_infoLog = B10001110;  //ERROR DE MEMORIA
        sendToEventsLogQueue(b_infoLog);
#endif
        myDelayMs(6000);
        resetX1();
      }
    }


    if ((uxBits & BIT_10_saveData) != 0)
    {
      if ( xDataQueue != NULL )
      {
        if ( xQueueReceive( xDataQueue, &( localData ), ( TickType_t ) 10 ) == pdPASS )
        {
          if (dataFile.isOpen()) dataFile.close(); //ADDED
          //          {
          //            DEBUG("1===========================================================> " + auxRootFile);
          //            dataFile.close();
          //          }

          // NOCRITICALSEC taskYIELD();
          // NOCRITICALSEC taskENTER_CRITICAL();

          // dataFile = sd.open(printRouteFile(fileName, DATALOG), FILE_WRITE);

          dataFile = sd.open(auxRootFile.c_str(), FILE_WRITE);
          dataFile.print(localData.qTimeStamp);
          dataFile.print(',');
          for (int i = 0; i < NUM_VALORES - 1; i++) {
            dataFile.print(localData.qData[i]);
            dataFile.print(',');
          } dataFile.println(localData.qData[NUM_VALORES - 1]);
          dataFile.close();
          // NOCRITICALSEC taskEXIT_CRITICAL();
        }
      }
    }

    if ((uxBits & BIT_9_sendFile) != 0)
    {
      uint8_t c;
      int sizeFile;
      uint16_t idx = 0;
      String auxName = "";
      UBaseType_t  freeSpace = 0;


      switch (stateSend )
      {
        case 0:

          //fileTime = millis();
          if ( sendFileName.endsWith("txt"))
          {
            if (sd.exists(sendFileName)) {
              if (sendFile.isOpen()) sendFile.close();
              sendFile = sd.open(sendFileName, FILE_READ); //BOOL FALSE
            }
            else {////GESTION DE ERRORES no necesaria, después se comprueba si está abierto
#ifdef DEBUG_LOGEVENTS
              b_infoLog = B10010001;  //NO EXISTE EL ARCHIVO A LEER
              sendToEventsLogQueue(b_infoLog);
#endif
            }
          }
          else
          {
            auxRootSend = "";

            if (!printRouteFile(sendFileName.c_str(), DATALOG, auxRootSend, false)) {

#ifdef DEBUG_LOGEVENTS //solo es valido para (,,false)
              b_infoLog = B10001111; //ERROR NO EXISTE LA RUTA DEL ARCHIVO
              sendToEventsLogQueue(b_infoLog);
#endif
            } //no necesita gestión ya que después viene un exists

            if (sd.exists(auxRootSend)) {
              if (sendFile.isOpen()) sendFile.close();
              sendFile = sd.open(auxRootSend, FILE_READ);//BOOL FALSE
            }
            else {////GESTION DE ERRORES no necesaria, después se comprueba si está abierto

#ifdef DEBUG_LOGEVENTS
              b_infoLog = B10010001;  //NO EXISTE EL ARCHIVO A LEER
              sendToEventsLogQueue(b_infoLog);
#endif
            }
          }

          if (sendFile.isOpen())
          {
            if ((uxQueueSpacesAvailable( xTxBuffer ) != 0 ))
            {
              if ( sendFileName.endsWith("txt"))
              {
                DEBUG("nombre del archivo0: " + sendFileName);
                if (openFileToSend(storeFiles, 2, filesToSend, auxName)) { //auxName = openFileToSend (storeFiles.c_str(), 2, filesToSend); //auxName = openFileToSend (storeFiles, 2);
                }
                else {
                  //añadir control de errores
                }
                auxName = auxName.substring(0, auxName.length() - 3) + "txt";
                DEBUG ( "NAME0 : " + auxName);
                idx = 0;
                while ( auxName.c_str()[idx] != 0)
                {
                  xQueueSend( xTxBuffer, &auxName.c_str()[idx], 0 );
                  idx++;
                }
              }
              else
              {
                DEBUG("NAME : " + sendFileName);
                idx = 0;
                while ( sendFileName.c_str()[idx] != 0)
                {
                  xQueueSend( xTxBuffer, &sendFileName.c_str()[idx], 0 );
                  idx++;
                }
              }
              stateSend = 1;
            }
          }
          else
          {
#ifdef DEBUG_LOGEVENTS
            b_infoLog = B10010011; //StateSend reiniciado al no poder abrir el archivo
            sendToEventsLogQueue(b_infoLog);
#endif
            DEBUG(" =============== STATESEND REINICIADO ===============");
            stateSend = 0;

            switch (deleteFirstName(storeFiles, filesToSend)) //ADDED MODIF  // deleteFirstName(storeFiles.c_str(), filesToSend);//deleteFirstName(storeFiles);
            {
              case 0:
                break;
#ifdef DEBUG_LOGEVENTS
              case -1:
                b_infoLog = B10001010; //ERROR en deletefirstname
                sendToEventsLogQueue(b_infoLog);
                break;
              case 2:
                b_infoLog = B10001011; //ERROR tempfile>ptrfile
                sendToEventsLogQueue(b_infoLog);
                break;
              case 3:
                b_infoLog = B10001100; //ERROR en al borrar el archivo
                sendToEventsLogQueue(b_infoLog);
                break;
              case 4:
                b_infoLog = B10010010; //ERROR al renombrar el archivo
                sendToEventsLogQueue(b_infoLog);
                break;
#endif
              default:
                break;
            }
            DEBUG ("FILE: FAILING READING NAME FILE");
            break;
          }
        case 1:
          c = ',';
          xQueueSend( xTxBuffer, &c, 0 );
          sizeFile = sendFile.fileSize();
          DEBUG ( "---------->SIZE: " + String(sizeFile));
          if ((uxQueueSpacesAvailable( xTxBuffer ) != 0 ))
          {
            c = (sizeFile & 0x000000FF);
            xQueueSend( xTxBuffer, &c, 0 );
            c = (sizeFile & 0x0000FF00) >> 8;
            xQueueSend( xTxBuffer, &c, 0 );
            c = (sizeFile & 0x00FF0000) >> 16;
            xQueueSend( xTxBuffer, &c, 0 );
            c = (sizeFile & 0xFF000000) >> 24;
            xQueueSend( xTxBuffer, &c, 0 );
            stateSend = 2;
          }
          break;
        case 2:
          //          DEBUG ( "FILE");
          freeSpace = uxQueueSpacesAvailable( xTxBuffer );
          while (freeSpace != 0)
          {
            if (sendFile.available())
            {
              c = sendFile.read();
              //                Serial.write(c);
              xQueueSend( xTxBuffer, &c, 0 );
              freeSpace--;
            }
            else // if (!sendFile.available())
            {
              //DEBUG ( "CLOSE SEND FILE IN: " + String (millis() - fileTime) + " ms");
              sendFile.close();

              stateSend = 3;
              break;
            }
          }

          break;
        case 3:
          DEBUG ( "CLOSED SEND FILE (END FILE)"); //termina de pasar el archivo de datos a la cola de transmisión
          DEBUG(" Clear BIT_9_sendFile");
          xEventGroupClearBits( myEvents,  BIT_9_sendFile);

          break;
      }
    }



    if (((uxBits & BIT_14_doneFileRead) != 0))
    {
      stateSend = 0;
      DEBUG ( "RESET SEND");
      xEventGroupClearBits( myEvents,  BIT_12_rdyFile);
      DEBUG ("CLEAR BIT 12<--------------");
      xEventGroupClearBits( myEvents,  BIT_14_doneFileRead);
      DEBUG ("CLEAR BIT 14<--------------");
      if (((uxBits & BIT_16_sendFileOK) != 0))
      {
        switch (deleteFirstName(storeFiles, filesToSend)) //ADDED MODIF  // deleteFirstName(storeFiles.c_str(), filesToSend);//deleteFirstName(storeFiles);
        {
          case 0:
            break;
#ifdef DEBUG_LOGEVENTS
          case -1:
            b_infoLog = B10001010; //ERROR en deletefirstname
            sendToEventsLogQueue(b_infoLog);
            break;
          case 2:
            b_infoLog = B10001011; //ERROR tempfile>ptrfile
            sendToEventsLogQueue(b_infoLog);
            break;
          case 3:
            b_infoLog = B10001100; //ERROR en al borrar el archivo
            sendToEventsLogQueue(b_infoLog);
            break;
          case 4:
            b_infoLog = B10010010; //ERROR al renombrar el archivo
            sendToEventsLogQueue(b_infoLog);
            break;
#endif
          default:
            break;
        }
        xEventGroupClearBits( myEvents,  BIT_16_sendFileOK);
        //DEBUG ("CLEAR BIT 16<--------------");
      }
    }
    if (((uxBits & BIT_11_iddle) != 0) && ((uxBits & BIT_9_sendFile) == 0))
    {
      //DEBUG("FILE: IDDLE&NOSENDFILE"); //WAIT4
      if (!emptyFileToSend)
      {
        /*//        DEBUG("0000----NO EMPTY");
          //        if (dataFile.isOpen())
          //        {
          //           DEBUG("0000----IS OPEN");
          //          dataFile.close();
          //#ifdef DEBUG_LOGEVENTS
          //          b_infoLog = B00000001;  //ENDFILE
          //          sendToEventsLogQueue(b_infoLog);
          //#endif
          //          if ((uxBits & BIT_17_snapFileToSend) != 0)
          //          {
          //            DEBUG("0000----SNAPFILE");
          //            if ((storeFiles.length() >= 10) && (storeFiles.length() <= 20 )) { //tamaños de ftosend.txt y 20220506_090307.csv
          //               DEBUG("0000----ADDNAMEEEEEEE");
          //              addNameToSend(storeFiles.c_str(), storeFiles, filesToSend); //addNameToSend(storeFiles, storeFiles);
          //            }
          //            else {
          //              DEBUG ( "ERROR ADD NAME, error name");
          //            }
          //            xEventGroupClearBits( myEvents,  BIT_17_snapFileToSend );
          //          }
          //          addNameToSend(fileName.c_str(), storeFiles, filesToSend); //addNameToSend(fileName, storeFiles);
          //        }
        */ //       DEBUG("0000----WAS CLOSED");
#ifdef SENDFILE
        if ((uxBits & BIT_12_rdyFile) == 0 )
        {
          stateSend = 0;
          xQueueReset( xTxBuffer );

          if (sd.exists(storeFiles)) {
            if (filesToSend.isOpen()) filesToSend.close();
            filesToSend = sd.open(storeFiles, FILE_READ);   //ANTES FILE TEMP generado aqui
            int xx = filesToSend.fileSize(); // no necesitamos control de isOpen porque fileSize devolverá 0 si no está abierto
            if ( xx < 2)
            {
              emptyFileToSend = true;
            }
            filesToSend.close();
            DEBUG("FILE: SIZE OF STOREFILE " + String(xx));

          }
          else {
            DEBUG("FILE: NO EXISTE STOREFILES");
            emptyFileToSend = true;
          }

          if (!emptyFileToSend) {
            readFileToSend(storeFiles, filesToSend);//readFileToSend(storeFiles);

            openFileToSend(storeFiles, 1, filesToSend, sendFileName);// sendFileName = openFileToSend (storeFiles.c_str(), 1, filesToSend); //sendFileName = openFileToSend (storeFiles, 1);

            SERIAL.print("SENDFILENAME: ");
            SERIAL.print(sendFileName);
            DEBUG(" LENGTH: " + String(sendFileName.length()));

            if ((sendFileName.length() >= 10) && (sendFileName.length() <= 20)) // //tamaños de ftosend.txt y 20220506_090307.csv if ( sendFileName.length() >= storeFiles.length() )  //ADDED MODIF antes >10 if ((fileName.length() >= 12) && (fileName.length() <= 20 ))
            {

              if ( sendFileName.endsWith("txt"))
              {
                DEBUG ("FILE: PREPARE TO SEND (txt): " + sendFileName);
                if (sd.exists(sendFileName)) {
                  if (sendFile.isOpen()) sendFile.close();
                  sendFile = sd.open(sendFileName, FILE_READ);
                } else {//GESTION DE ERRORES no necesaria, después se comprueba si está abierto
#ifdef DEBUG_LOGEVENTS
                  b_infoLog = B10010001; //NO EXISTE EL ARCHIVO A LEER
                  sendToEventsLogQueue(b_infoLog);
#endif
                }
              }
              else
              {
                auxRootSend = "";

                if (!printRouteFile(sendFileName.c_str(), DATALOG, auxRootSend, false)) {
#ifdef DEBUG_LOGEVENTS //solo es valido para (,,false)
                  b_infoLog = B10001111; //ERROR NO EXISTE LA RUTA DEL ARCHIVO
                  sendToEventsLogQueue(b_infoLog);
#endif
                } // no necesita gestion , despues se comprueba con exists

                DEBUG ("FILE: PREPARE TO SEND: " + auxRootSend); //ADDED MODIF
                // DEBUG ("SET BIT 12");

                if (sd.exists(auxRootSend)) {
                  if (sendFile.isOpen()) sendFile.close();
                  sendFile = sd.open(auxRootSend, FILE_READ); //ADDED MODIF
                }
                else {//GESTION DE ERRORES no necesaria, después se comprueba si está abierto
#ifdef DEBUG_LOGEVENTS
                  b_infoLog = B10010001; //NO EXISTE EL ARCHIVO A LEER
                  sendToEventsLogQueue(b_infoLog);
#endif
                }
              }

              if ( sendFile.isOpen())
              {
                sendFile.close();
                DEBUG ("FILE: SET BIT 12 ***");
                xEventGroupSetBits( myEvents,  BIT_12_rdyFile );
              }
              else
              {
                DEBUG ("FILE: FAILING READING THIS FILE");
                switch (deleteFirstName(storeFiles, filesToSend) ) //ADDED MODIF  // switch (deleteFirstName(storeFiles) != 0)
                {
                  case 0:
                    break;
#ifdef DEBUG_LOGEVENTS
                  case -1:
                    b_infoLog = B10001010; //ERROR en deletefirstname
                    sendToEventsLogQueue(b_infoLog);
                    break;
                  case 2:
                    b_infoLog = B10001011; //ERROR tempfile>ptrfile
                    sendToEventsLogQueue(b_infoLog);
                    break;
                  case 3:
                    b_infoLog = B10001100; //ERROR en al borrar el archivo
                    sendToEventsLogQueue(b_infoLog);
                    break;
                  case 4:
                    b_infoLog = B10010010; //ERROR al renombrar el archivo
                    sendToEventsLogQueue(b_infoLog);
                    break;
#endif
                  default:
                    break;
                }
              }
            }
            else { //ADDED
              DEBUG("FILE: Nombre de archivo fuera de rango: " + filesToSend);
              switch (deleteFirstName(storeFiles, filesToSend)) //ADDED MODIF  // deleteFirstName(storeFiles.c_str(), filesToSend);//deleteFirstName(storeFiles);
              {
                case 0:
                  break;
#ifdef DEBUG_LOGEVENTS
                case -1:
                  b_infoLog = B10001010; //ERROR en deletefirstname
                  sendToEventsLogQueue(b_infoLog);
                  break;
                case 2:
                  b_infoLog = B10001011; //ERROR tempfile>ptrfile
                  sendToEventsLogQueue(b_infoLog);
                  break;
                case 3:
                  b_infoLog = B10001100; //ERROR en al borrar el archivo
                  sendToEventsLogQueue(b_infoLog);
                  break;
                case 4:
                  b_infoLog = B10010010; //ERROR al renombrar el archivo
                  sendToEventsLogQueue(b_infoLog);
                  break;
#endif
                default:
                  break;
              }
            }
          }
        }
#endif
      }
      taskYIELD();
      ////////////////////////////////////TODO ESTO DEBERIA ESTAR COMENTADO YA SE HACE CON EL WAIT4/////////////
      //       powerOffSD();
      //      DEBUG("S-FILE");
      //      vTaskSuspend(NULL); ///!!!!???
      //      DEBUG("R-FILE");
      //      if (!powerOnSD())
      //      {
      //        DEBUG("SD card failed, or not present");
      //        powerOffSD();
      //
      //        xEventGroupClearBits( eventTaskState,  BIT_3_taskFile ); //ADDED WORKING
      //
      //        vTaskDelete(NULL);
      //      }
      //      DEBUG("FILE: SD card initialized.");
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
    }

    if ((xEventGroupGetBits(myEvents) &  auxBinStatus) == 0)//WAIT4  Si sabemos que va a poder hacer el wait
    {
      DEBUG("FILE: WAITING");

      powerOffSD();
      xEventGroupWaitBits(
        myEvents,
        BIT_0_createFile | BIT_10_saveData | BIT_9_sendFile | BIT_14_doneFileRead | BIT_19_IddleYNoSend ,
        false,
        false,
        portMAX_DELAY ) ;

      DEBUG("FILE: WAKE UP state: " + String((xEventGroupGetBits(myEvents) & auxBinStatus) , BIN)); //WAIT4
      if (!powerOnSD())
      {
        SERIAL.println("FILE: SD card fail, or not present");
        powerOffSD();

        xEventGroupClearBits( eventTaskState,  BIT_3_taskFile ); //ADDED WORKING

        vTaskDelete(NULL);
      }
      DEBUG("FILE: SD card initialized.");
    }
  }
}

//void readFileToSend (String fileName)  //REVISADA
//{
//  File ptrFile = sd.open(fileName, FILE_READ); //BOOL FALSE
//  if (ptrFile)
//  {
//    DEBUG ( "Content of Stored file:");
//
//    while (ptrFile.available())
//    {
//      taskYIELD();
//      taskENTER_CRITICAL();
//
//      Serial.write(ptrFile.read());
//
//      taskEXIT_CRITICAL();
//    }
//    DEBUG("");
//
//    ptrFile.close();
//  }
//  else
//  {
//    DEBUG ("ERROR READING FILE TO SEND");
//  }
//}
void readFileToSend (const char *fileName, File &ptrFile) //NUEVA -> Gestion de errores
{
  if (ptrFile.isOpen()) ptrFile.close();
  ptrFile = sd.open(fileName, FILE_READ); //Solo se utiliza 1 vez con un archivo al que ya se le ha hecho un exists -> no necesario exists
  if (ptrFile.isOpen())
  {
    SERIAL.println( "Content file:");
    while (ptrFile.available())
    {
      taskYIELD();
      taskENTER_CRITICAL();
      SERIAL.write(ptrFile.read());
      taskEXIT_CRITICAL();
    }
    ptrFile.close();
  }
  else
  {
    DEBUG ("ERROR READING FILE TO SEND");
  }
}



//String openFileToSend (String fileStore, int  j ) //REVISADA
//{
//  String temp = "";
//  File ptrFile = sd.open(fileStore, FILE_READ); //BOOL FALSE
//  if ( !ptrFile)
//  {
//    //    DEBUG("FAIL OPENING");
//    return temp;
//  }
//  //  DEBUG("START READING");
//  //  byte i = 0;
//  for (int i = 0; i < j ; i++)
//  {
//    temp = "";
//    while ( ptrFile.available())
//    {
//      taskYIELD(); //ADDED
//      taskENTER_CRITICAL();
//      char c = ptrFile.read();
//      //      Serial.print(c); Serial.print(' '); Serial.println(c, HEX);
//      //      if (( c != '\n') && ( c != '\r')) temp += String(c);
//      taskEXIT_CRITICAL();
//      if (( c != '\n')) temp += String(c);
//      else break;
//    }
//  }
//  ptrFile.close();
//  if (temp.length() < 10) return "";
//  temp = temp.substring(0, temp.length() - 1);
//  //  for (int i = 0; i < temp.length();i++) DEBUG (temp[i]);
//  return temp;
//}
//String openFileToSend (const char *fileStore, int  j , File &ptrFile )//NUEVA
//{
//  //SE UTILIZA EN UNA ZONA SIN CONTROL DE EXISTS Y EN OTRA QUE SI LO TIENE, ES NECESARIO CONTROLAR EL EXISTS DEL ARCHIVO
//  String temp = "";
//  temp.reserve(20);
//  if (sd.exists(fileStore)) {
//    if (ptrFile.isOpen()) ptrFile.close();
//    ptrFile = sd.open(fileStore, FILE_READ);
//  }//NO HACE FALTA CONTROL DE ERRORES SE CONTROLA SI SE HA ABIERTO
//  if ( !ptrFile.isOpen())
//  {
//    //    DEBUG("FAIL OPENING");
//    return temp;
//  }
//  //  DEBUG("START READING");
//  //  byte i = 0;
//  for (int i = 0; i < j ; i++)
//  {
//    temp = "";
//    while ( ptrFile.available())
//    {
//      char c = ptrFile.read();
//      //      Serial.print(c); Serial.print(' '); Serial.println(c, HEX);
//      //      if (( c != '\n') && ( c != '\r')) temp += String(c);
//      if (( c != '\n')) temp += String(c);
//      else break;
//    }
//  }
//  ptrFile.close();
//  if (temp.length() < 10) return "";
//  temp = temp.substring(0, temp.length() - 1);
//  //  for (int i = 0; i < temp.length();i++) DEBUG (temp[i]);
//  return temp;
//}

bool openFileToSend (const char *fileStore, int  j , File &ptrFile , String &temp) //NUEVA2 PUEDE SER VOID
{
  //SE UTILIZA EN UNA ZONA SIN CONTROL DE EXISTS Y EN OTRA QUE SI LO TIENE, ES NECESARIO CONTROLAR EL EXISTS DEL ARCHIVO
  temp = "";
  temp.reserve(20);
  if (sd.exists(fileStore)) {
    if (ptrFile.isOpen()) ptrFile.close();
    ptrFile = sd.open(fileStore, FILE_READ);
  }//NO HACE FALTA CONTROL DE ERRORES SE CONTROLA SI SE HA ABIERTO
  if ( !ptrFile.isOpen())
  {
    //    DEBUG("FAIL OPENING");
    return false;
  }
  //  DEBUG("START READING");
  //  byte i = 0;
  for (int i = 0; i < j ; i++)
  {
    temp = "";
    while ( ptrFile.available())
    {
      char c = ptrFile.read();
      //      Serial.print(c); Serial.print(' '); Serial.println(c, HEX);
      //      if (( c != '\n') && ( c != '\r')) temp += String(c);
      if (( c != '\n')) temp += String(c);
      else break;

      if (temp.length() > 20) {
        temp = "";
        break;
      }
    }
  }
  ptrFile.close();
  if (temp.length() < 10 )
  {
    temp = "";
    return false;
  }
  temp = temp.substring(0, temp.length() - 1);
  //  for (int i = 0; i < temp.length();i++) DEBUG (temp[i]);
  return true;
}


//void addNameToSend(String fileName, String logFile) //REVISADA
//{
//  taskYIELD();
//  taskENTER_CRITICAL();
//
//  File ptrFile = sd.open(logFile, FILE_WRITE); //BOOL TRUE
//  if (ptrFile)
//  {
//    if ((fileName.length() == 19) || (fileName == "fToSend.txt" )) //ADDED
//    {
//      DEBUG ( "FILE: Name Stored: " + fileName);
//      ptrFile.println(fileName.c_str());
//    }
//    else {
//      DEBUG("FILE: ERROR IN FILE NAME TO ADD");
//    }
//    ptrFile.close();
//  }
//  else
//  {
//    DEBUG ( "FILE: ERROR ADDING NAME");
//  }
//
//  taskEXIT_CRITICAL(); //ADDED MODIF
//}
void addNameToSend(const char* fileName, const char* logFile, File &ptrFile)//NUEVO
{
  taskYIELD();
  taskENTER_CRITICAL();

  if (ptrFile.isOpen()) ptrFile.close();

  ptrFile = sd.open(logFile, FILE_WRITE); //BOOL TRUE

  if (ptrFile.isOpen())
  {
    SERIAL.print( "Name Stored: ");
    SERIAL.println(fileName);

    ptrFile.println(fileName);
    ptrFile.close();
  }
  else
  {
    DEBUG ( "ERROR ADD NAME opening file");
  }

  taskEXIT_CRITICAL();
}

//int deleteFirstName(String logFile)  //REVISADA
//{
//  taskYIELD();
//  taskENTER_CRITICAL();
//
//  File tempFile = sd.open("aux", FILE_WRITE);
//
//  if (sd.exists(logFile)) {
//    File ptrFile = sd.open(logFile, FILE_READ);
//  }
//  else {
//    return -1;
//  }
//
//  bool firstLine = false;
//  if ((!tempFile) || (!ptrFile))
//  {
//    DEBUG( "FILE DOESNT OPEN");
//    tempFile.close();
//    ptrFile.close();
//    taskEXIT_CRITICAL();
//    return -1;
//  }
//
//  Serial.print("line to delete: ");
//  while ( ptrFile.available())
//  {
//    uint8_t c = ptrFile.read();
//    if ( firstLine )
//    {
//      tempFile.write(c);
//    }
//    else
//    {
//      Serial.write(c);
//      if ( c == '\n') firstLine = true;
//    }
//  }
//
//  if (tempFile.fileSize() > ptrFile.fileSize()) { //ADDED
//    tempFile.close();
//    ptrFile.close();
//    sd.remove("aux");
//
//    taskEXIT_CRITICAL();
//    return 2;
//  }
//
//  ptrFile.close();
//  sd.remove(logFile);
//
//  if (!sd.exists(logFile)) { //ADDED
//    tempFile.rename(logFile.c_str());
//  }
//  else {
//    tempFile.close();
//
//    taskEXIT_CRITICAL();
//    return 3;
//  }
//
//
//  tempFile.close();
//
//  taskEXIT_CRITICAL();
//  return 0;
//}
int deleteFirstName(const char *logFile, File &ptrFile)//NUEVO
{
  taskYIELD();
  taskENTER_CRITICAL();

  File tempFile = sd.open("aux", FILE_WRITE); //BOOL TRUE

  if (sd.exists(logFile)) {
    if (ptrFile.isOpen()) ptrFile.close();
    ptrFile = sd.open(logFile, FILE_READ); //BOOL FALSE
  }// NO NECESITA CONTROL DE ERRORES SE COMPRUEBA SI SE HA ABIERTO
  bool firstLine = false;
  if ((!tempFile.isOpen()) || (!ptrFile.isOpen()))
  {
    DEBUG( "DELETE WHILE OPEN TEMP");
    if (tempFile.isOpen())  tempFile.close();
    if (ptrFile.isOpen())   ptrFile.close();

    taskEXIT_CRITICAL();
    return -1;
  }
  SERIAL.print("line to delete: ");
  while ( ptrFile.available())
  {
    uint8_t c = ptrFile.read();
    if ( firstLine )
    {
      tempFile.write(c);
    }
    else
    {
      SERIAL.write(c);
      if ( c == '\n') firstLine = true;
    }
  }

  if (tempFile.fileSize() > ptrFile.fileSize()) { //ADDED
    tempFile.close();
    ptrFile.close();

    if (!sd.remove("aux")) {
      taskEXIT_CRITICAL();
      return 3;
    }

    taskEXIT_CRITICAL();
    return 2;
  }

  ptrFile.close();

  if (!sd.remove(logFile)) {
    tempFile.close();

    taskEXIT_CRITICAL();
    return 3;
  }

  if (!tempFile.rename(logFile)) {
    tempFile.close();

    taskEXIT_CRITICAL();
    return 4;
  }

  tempFile.close();

  taskEXIT_CRITICAL();
  return 0;
}

//String printRouteFile(String &myNameFile, char *typeDir)
//{
//  String rootDir = "/" + myNameFile.substring(0, 8) ;//rootdir = "/20220420"
//
//  if ( !sd.exists(rootDir))//comprueba si existe la carpeta del día
//  {
//    sd.mkdir(rootDir); //crea directorio de carpeta del día
//  }
//  if (!sd.exists(rootDir + "/" + String(typeDir)))//comprueba si existe "/20220420/typedir"
//  {
//    sd.mkdir(rootDir + "/" + String(typeDir)); //crea el directorio "/20220420/typedir"
//  }
//  return rootDir + "/" + String(typeDir) + "/" + myNameFile; //retorna : "/20220420/typedir/20220420_021320.csv"
//}

bool printRouteFile(const char* myNameFile, const char* typeDir, String &rootString, bool create) //NUEVO
{
  rootString = "";
  rootString.reserve(36);
  static String auxNameFile = "";
  auxNameFile = String(myNameFile);
  rootString = "/" + auxNameFile.substring(0, 8);

  if ( !sd.exists(rootString))
  {
    if (!create) {
      return false; //NO EXISTE LA RUTA DEL ARCHIVO A LEER (SOLO PARA FALSE)
    }
    if (!sd.mkdir(rootString)) {
      return false; //NO SE HA GENERADO LA CARPETA CORRECTAMENTE (SOLO PARA TRUE)
    }
  }

  rootString += "/";
  rootString += String(typeDir);

  if (!sd.exists(rootString))
  {
    if (!create) {
      return false; //NO EXISTE LA RUTA DEL ARCHIVO A LEER (SOLO PARA FALSE)
    }
    if (!sd.mkdir(rootString )) {
      return false; //NO SE HA GENERADO LA CARPETA CORRECTAMENTE (SOLO PARA TRUE)
    }
  }

  rootString += "/";
  rootString += auxNameFile; //String(myNameFile);
  auxNameFile = "";
  DEBUG(" rtFl: " + rootString);
  return true; //FUNCION TERMINADA CORRECTAMENTE
}


//int printDirectory(File dir, int numTabs)
//{
//  unsigned long timeOut = millis();
//
//  while (true) {
//    File entry =  dir.openNextFile();
//    if (! entry) {
//      // no more files
//      break;
//    }
//    for (uint8_t i = 0; i < numTabs; i++) {
//      Serial.print('\t');
//    }
//    //Serial.print(entry.name());
//    entry.printName(&Serial);
//    if (entry.isDirectory()) {
//      Serial.println("/");
//      printDirectory(entry, numTabs + 1);
//    } else {
//      int  measurement = uxTaskGetStackHighWaterMark( Handle_fileTask );
//      SERIAL.print(" Size: ");
//      SERIAL.print(measurement);
//      // files have sizes, directories do not
//      //      Serial.print("\t\t");
//      //      Serial.println(entry.size(), DEC);
//      Serial.println();
//    }
//    entry.close();
//  }
//  return 0;
//}
//int listFile()
//{
//  File root = sd.open("/");
//  printDirectory(root, 0);
//  return 0;
//}
