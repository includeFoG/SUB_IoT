

//*****************************************************************
// Thread TaskDataReading
//*****************************************************************

TaskHandle_t data_read_task_handle;
void TaskDataReading( void *pvParameters )
{
  setCpuFrequencyMhz(240);
  Serial.updateBaudRate(BAUD_SERIAL);
  Serial1.updateBaudRate(BAUD_SERIAL);
  checkFreqs();
  
  digitalWrite(PWR_RS485_EN, HIGH); //Alimenta RS485 Externo
  digitalWrite(PWR_V3_EN, HIGH);    //Alimenta RS485 INTERNO, SD, IMU, PULL-UP I2C
  myDelayMs(2000);
  Wire.begin();

  Serial.println("\nStarting data thread");
  myDelayMs(50);

  const char TAG_DATAREADING[] = "[DAT]";

  if (configCalibrationIMU()) {
    Serial.println("CONFIGURACION DE IMU CARGADA");
  }
  else {
    Serial.println("ERROR AL CARGAR VALORES DE IMU");
  }

  EventBits_t uxBitsDatos;

  std::string rxData = ""; //buffer recepción
  //std::string rxToSD = ""; //buffer de almacenamiento 
  //uint8_t countToWrite = 0; //indice usado para decidir cuando se almacena el dato en la SD
  IMUStruct IMUParams; //estructura de datos de IMU

  //ESPERAMOS HASTA QUE EL DISPOSITIVO TRANSDUCTOR ESTE LISTO PARA COMUNICAR
  char c_ = 0;
  long preTime = millis();
  Serial.print("[DAT] Waiting for SG Transductor response...\n");
  while (c_ != '\n') { //filtro para esperar a que esté enviando el RS485 del transductor NO ESTA FUNCIONANDO CORRECTAMENTE, SE LO SALTA AUNQUE NO ENVIE¿?
    if (Serial1.available() > 0) {
      c_ = Serial1.read();
      if (c_ != '\n')
      {
        rxData += c_;
      }
    }
    if (millis() - preTime > 180000) {
      Serial.println("[DaT]Without Response!!!");
      break;
    }
  }
  Serial.print("response: ");
  for (int i = 0; i < rxData.length(); i++)
  {
    Serial.write(rxData[i]);
  }
  Serial.println();
  rxData = "";

  File sessionFile;       //nuevo en cada iteración (de sesión)
  File batteryStatusFile; //único
  char* resultPathBatteryStatusFile= nullptr;
  char* resultPathSessionFile = nullptr;

  uxBitsDatos = xEventGroupGetBits(xEventDatos);

  if ((uxBitsDatos & BIT_1_DAT_SESION) != 0) { //si se ha activado la sesión de datos
    
    SPI.begin();  // Usa pines por defecto del ESP32
    SPI.setFrequency(SPI_SPEED); 
    myDelayMs(2000);

    bool sd_ok = false;
    for (int i = 0; i < 3; i++)
    {
      SD.end();  // Reset interno del driver SD
      myDelayMs(500);
      
      if (SD.begin(CS_SD)) {
        sd_ok = true;
        ESP_LOGI(TAG_DATAREADING, "SD_PRE_INITIALIZED");
        break;
      }
      ESP_LOGW(TAG_DATAREADING, "Intento %d: SD.begin() falló, reintentando...", i+1);
      myDelayMs(300);
    }
    
    if (!sd_ok) 
    {
      ESP_LOGE(TAG_DATAREADING, "No se pudo iniciar la tarjeta SD tras varios intentos.");
    }

    ESP_LOGI(TAG_DATAREADING, "Rechecking-SD");
    
    SD.end();  // Reset interno del driver SD
    myDelayMs(500);

    if (!SD.begin(CS_SD)) {
      ESP_LOGE(TAG_DATAREADING,"Error al iniciar tarjeta SD");
      if (STATE == INTIME) {//Este if es para que el while solo tenga lugar cuando se trata de una sesión automática por tiempo, no manual mediante BT (si es manual se para manualmente)
        ESP_LOGE(TAG_DATAREADING,"Card Mount Failed, deleting task - CHANGING STATE TO IDDLE");
        
        xEventGroupClearBits(xEventDatos, BIT_1_DAT_SESION );

        if (xSemaphoreTake(manager_semaphore, portMAX_DELAY) == pdTRUE) {
          STATE = IDDLE;
          xSemaphoreGive(manager_semaphore); //ADDED 0.7.6
        }

        digitalWrite(PWR_RS485_EN, LOW);
        data_read_task_handle = NULL;
        vTaskDelete(NULL);
      }
    } else {
      ESP_LOGI(TAG_DATAREADING,"[DAT] SD ok");
    }

    digitalWrite(PWR_RS485_EN, HIGH); //PRUEBA no debería ser necesario
    myDelayMs(50);

    //NO MOVER ESTE BLOQUE DE AQUI PARA QUE EL NOMBRE DEL ARCHIVO ESTE LO MAS CERCANO AL TIEMPO DE INICIO POSIBLE
    // ***********************************************************************************************************
    blinkLed(1, 100, 2);
    std::string nameFile = "";
    //_______________________________________________SESIONFILE_____________________________________________________
    Serial.print("creating file: ");
    struct tm tm_INT = rtc.getTimeStruct(); //leemos valor de RTCINT

    nameFile = "/";
    if (tm_INT.tm_year < 124) { //SI NO ESTA EN HORA EL RTC EXT (124 años que han pasado desde 1900 hasta 2024)
      nameFile += String(nvs_val_filename).c_str(); //nameFile += String(nvs_val_day).c_str();
      nameFile += "_";
      nvs_val_filename++;//aumentamos en 1 el valor de nvs_val_filename para el próximo archivo
    }
    nameFile += rtc.getTime("%Y%m%d_%H%M%S").c_str();
    nameFile += EXTENSION;
    write_nvs_16(nvs_key_filename, nvs_val_filename);
    Serial.print("FileName: "); Serial.println(nameFile.c_str());

    //una vez tenemos el nombre de archivo comprobamos que exista la carpeta de archivos y si no la creamos
    resultPathSessionFile = (char*)malloc(strlen(directorySession) + nameFile.length() + 1); //+1 para el endchar
    if (resultPathSessionFile != NULL) {
      strcpy(resultPathSessionFile, directorySession);
      strcat(resultPathSessionFile, nameFile.c_str());
    }
    if (!SD.exists(directorySession)) { //comprobamos si existe /BatteryInfo
      if (!SD.mkdir(directorySession)) { //si no existe lo creamos
        Serial.println("Error mkdir directorySession");
      }
      else {
        Serial.print("directorySesion"); Serial.println("   <created>");
      }
    }

    //_______________________________________________BATTERYFILE_____________________________________________________
    //preparamos directorio y nombre de archivos de bateria
    resultPathBatteryStatusFile = (char*)malloc(strlen(directoryBattery) + strlen(name_batteryStatusFile) + 1); //+1 para el endchar
    if (resultPathBatteryStatusFile != NULL) {
      strcpy(resultPathBatteryStatusFile, directoryBattery);
      strcat(resultPathBatteryStatusFile, name_batteryStatusFile);
    }

    if (!SD.exists(directoryBattery)) { //comprobamos si existe /BatteryInfo
      if (!SD.mkdir(directoryBattery)) { //si no existe lo creamos
        Serial.println("Error mkdir directoryBattery");
      }
      else { //si lo crea
        if (!SD.exists(resultPathBatteryStatusFile)) { //comprobamos si existe el fichero en el directorio y si no existe lo creamos
          batteryStatusFile = SD.open(resultPathBatteryStatusFile, FILE_WRITE);
          batteryStatusFile.println("FileRelated,StateOfCharge[%],Voltage[mV],AvCurrent[mA],Capacity[mAh],FullCapacity[mAg],PowerDraw[mW],Health[%]");
          batteryStatusFile.close();
          myDelayMs(100);
          Serial.print(name_batteryStatusFile); Serial.println("   <created>");
        }
      }
    }
    
    batteryStatusFile = SD.open(resultPathBatteryStatusFile, FILE_APPEND);
    batteryStatusFile.print(nameFile.c_str()); //guardamos archivo relacionado a la medición de batería
    batteryStatusFile.print(",");
    batteryStatusFile.print((getBatteryStatus()).c_str()); //guardamos datos de batería
    batteryStatusFile.println();
    batteryStatusFile.close(); //cerramos archivo
    myDelayMs(100);

    sessionFile = SD.open(resultPathSessionFile, FILE_WRITE); //a partir de aqui sessionFile devuelve true
    myDelayMs(100);
    sessionFile.print("Time[ms],SG1[kg],SG2[kg],AcelX[m/s2],AcelY[m/s2],AcelZ[m/s2],GyroX[deg/s],GyroY[deg/s],GyroZ[deg/s],Temp[");
    sessionFile.write(176); // Código ASCII del símbolo º (grado)
    sessionFile.println("C]");
    
    // **********************************************************************************************************
  }

  uint8_t counterPrint = 0;
  long varMillis;

  //limpiamos el buffer para empezar a tomar nuevos datos en el momento en que decimos que empieza la grabación
  while (Serial1.available() > 0) { //Serial1.flush();
    Serial1.read();
  }

  Serial.print("[DAT] start Sesion");
  ESP_LOGI(TAG_DATAREADING, "*STATE: %d", STATE);

  //nos posicionamos en el punto desde el que queremos almacenar los datos:
  c_ = 0;
  preTime = millis();
  while (c_ != '\n') { //filtro para esperar a que esté enviando el RS485 del transductor
    if (Serial1.available() > 0) {
      c_ = Serial1.read();
    }
    if (millis() - preTime > 180000) {
      Serial.println("[DaT]Without Response CRITIC!!!"); //V.0.0.8
      break;
    }
  }
  Serial.print("[DAT] ready");
  if (STATE == INTIME) {//Este if es para que el while solo tenga lugar cuando se trata de una sesión automática por tiempo, no manual mediante BT (si es manual se para manualmente)
    //habilitamos el timer, el timer en el callback limpiará BIT_1_DAT_SESION finalizando la sesión
    if ( setup_Timer() == ESP_OK) {
      ESP_LOGI(TAG_DATAREADING, "CHANGING STATE TO MEASURING");
      if (xSemaphoreTake(manager_semaphore, portMAX_DELAY) == pdTRUE) {
        STATE = MEASURING;
        xSemaphoreGive(manager_semaphore); //ADDED 0.7.6
      }
    }
    else {
      ESP_LOGE(TAG_DATAREADING, "ERROR STARTING - CHANGING STATE TO IDDLE");
      //FALTA AQUI DESARROLLAR UN POCO QUE PASA SI FALLA
      if (xSemaphoreTake(manager_semaphore, portMAX_DELAY) == pdTRUE) {
        STATE = IDDLE;
        xSemaphoreGive(manager_semaphore); //ADDED 0.7.6
      }

      free(resultPathBatteryStatusFile); //liberamos espacio de resultPath
      free(resultPathSessionFile);
      digitalWrite(PWR_RS485_EN, LOW);
      data_read_task_handle = NULL;
      vTaskDelete( NULL );
    }
  }


  for (;;)
  {
    uxBitsDatos = xEventGroupGetBits(xEventDatos);
    rxData = "";
    varMillis = millis();
    //rxData = String(varMillis).c_str();
    //xData += ",";
    if ( ((uxBitsDatos & BIT_1_DAT_SESION) == 0)) //si NO está en sesión
    {
      if (sessionFile) //si había un archivo abierto lo cierra, sessionfile necesario: se puede tener el bit BIT_1_DAT_SESION a 0 y estar viendo los datos, cerrando un archivo que no existe
      {

        Serial.println("[DAT] closing file");
        sessionFile.close();
        myDelayMs(100);
        blinkLed(1, 100, 3);

        batteryStatusFile = SD.open(resultPathBatteryStatusFile, FILE_APPEND);
        batteryStatusFile.print("ENDFILE,"); //guardamos archivo relacionado a la medición de batería
        batteryStatusFile.println((getBatteryStatus()).c_str()); //guardamos datos de batería
        batteryStatusFile.close(); //cerramos archivo
        myDelayMs(100);
        SD.end();

      }
      if ( (uxBitsDatos & BIT_0_DAT_VIEW) == 0 ) //si tampoco se están visualizando datos se cierra el hilo
      {
        digitalWrite(PWR_RS485_EN, LOW);

        if (STATE == MEASURING) {//Este if solo es TRUE cuando se trata de una sesión automática por tiempo, no manual mediante BT (si es manual STATE = CONFIG)

          ESP_LOGI(TAG_DATAREADING,"CHANGING STATE TO IDDLE");
          if (xSemaphoreTake(manager_semaphore, portMAX_DELAY) == pdTRUE) {
            STATE = IDDLE;
            xSemaphoreGive(manager_semaphore); //ADDED 0.7.6
          }
          
        }
        preTime = millis();
        //terminamos de limpiar lo que queda en puerto serie
        while (Serial1.available() > 0) {
          Serial1.read();
          if (millis() - preTime > 180000) {
            Serial.println("[DaT]Without Response CRITIC 2!!!");
            break;
          }
        }

        Serial.println("[DAT] cerrando hilo datos");
        free(resultPathBatteryStatusFile); //liberamos espacio de resultPath
        free(resultPathSessionFile);
        data_read_task_handle = NULL;
        vTaskDelete(NULL);
      }
    }


    char c = 0; //puede que si lo sacamos fuera de problemas
    uint8_t countComma = 0; //contador de comas
    while ( c != '\n') //TOMAMOS 1 DATO (frase) DE LA GALGA PROVENIENTE DEL RS485
    {
      if (Serial1.available() > 0)
      {
        c = Serial1.read();
        if ((c != '\n'))// && (c != '\r') && (c != ' '))
        {
          rxData += c;
        }
        if(c==','){
          countComma++;
        }
      }
      if (millis() - varMillis >= 200) {
        Serial.println("No response");
        rxData +=",,";
        break;
      }
    }

    if(countComma!=2)
    {
      Serial.println("\nmissData :"+String(countComma));
      for(int i=0;i<(2-countComma);i++){ //nos aseguramos de añadir las comas esperadas para que al menos a partir de aquí sí estén los datos bien ordenados
        rxData +=',';
      }
    }
    //IMU

    getIMUFiltered(IMUParams, 1, 2, false); //getIMUFiltered(IMUParams);

    float v_measures[7];
    v_measures[0] = IMUParams.pond_aX;
    v_measures[1] = IMUParams.pond_aY;
    v_measures[2] = IMUParams.pond_aZ;
    v_measures[3] = IMUParams.pond_gX;
    v_measures[4] = IMUParams.pond_gY;
    v_measures[5] = IMUParams.pond_gZ;
    v_measures[6] = sensorIMU.getTemperature();

    rawToMeasure_IMU(v_measures); //modificamos el valor por referencia

    // Accel
    rxData += "," ;
    rxData += std::to_string(v_measures[0]);
    rxData += "," ;
    rxData += std::to_string(v_measures[1]);
    rxData += "," ;
    rxData += std::to_string(v_measures[2]);
    rxData += "," ;

    // Gyro
    rxData += std::to_string(v_measures[3]);
    rxData += ",";
    rxData += std::to_string(v_measures[4]);
    rxData += "," ;
    rxData += std::to_string(v_measures[5]);
    rxData += "," ;

    // Temp
    rxData += std::to_string(v_measures[6]);

    rxData += '\n';

    if (rxData.length() > 0) //si se ha obtenido algo
    {

      if ( ( uxBitsDatos & BIT_0_DAT_VIEW ) != 0 ) //si queremos visualizar el dato por pantalla y BT en una sesión MANUAL
      {
        if ( counterPrint > 0) //% 10 == 0)
        {
          txBLE = "";
          for (int i = 0; i < rxData.length(); i++)
          {
            Serial.write(rxData[i]);
            if (xSemaphoreTake(BLE_txSemaphore, pdMS_TO_TICKS(100)) == pdTRUE)
            {
              txBLE += rxData[i];
              xSemaphoreGive(BLE_txSemaphore);
            }
          }
        }
      }

      if ( ( uxBitsDatos & BIT_1_DAT_SESION ) != 0 ) //si estamos en una sesión
      {
        xEventGroupSetBits(xEventLeds, BIT_0_LED_DATA_SAVE); //parpadeo VERDE 50ms

        if (sessionFile) { //si se generó el archivo de sesión (necesario para crear el archivo por si primero se acriva el VIEW y posteriormente el DAT_SESION)
            /*if(countToWrite >= NUMBER_OF_MEASURES_TO_WRITE) //si toca escribir
            {
              for (int i = 0; i < rxToSD.length(); i++)
              {
                sessionFile.write(rxToSD[i]);   //Almacena el dato en el archivo
    
                if (CONFIG_LOG_DEFAULT_LEVEL > 2) { //para que solo se vea cuando esta activado el core debug > info
                  if ( counterPrint % 100 == 0) {
                    Serial.write(rxData[i]);
                  }
                }
              }

               rxToSD = "";
               countToWrite=0;
            }
            else{ //si no toca escribir
              rxToSD += rxData;
              countToWrite++;
            }*/

          for (int i = 0; i < rxData.length(); i++)
          {
            sessionFile.write(rxData[i]);   //Almacena el dato en el archivo

            if (CONFIG_LOG_DEFAULT_LEVEL > 3) { //para que solo se vea cuando esta activado el core debug > info
              if ( counterPrint % 100 == 0) {
                Serial.write(rxData[i]);
              }
            }
          } 

          //ESP_LOGI(TAG_DATAREADING, "%s",rxData.c_str()); //Solo funciona con el primer mensaje después no imprime nada de rxData

        }
        else { //EN CASO DE QUE SE ESTUVIESEN VISUALIZANDO DATOS Y POSTERIORMENTE SE DECIDIESE GRABAR LA SESION
          //PENDIENTE!!!
          //Serial.println("Opening file");
           if ( counterPrint % 100 == 0) {
                    ESP_LOGE(TAG_DATAREADING, "ERROR - NO HAY ARCHIVO ABIERTO!");
                    
           }
           SD.end();
           myDelayMs(500);
           if (!SD.begin(CS_SD)) {
             ESP_LOGE(TAG_DATAREADING, "ERROR COMUNICANDO CON SD!");
           }
           else{
             ESP_LOGI(TAG_DATAREADING, "Intentando reabrir...");
             
             sessionFile = SD.open(resultPathSessionFile, FILE_APPEND); //a partir de aqui sessionFile devuelve true
           
             if(sessionFile)
             {
              ESP_LOGI(TAG_DATAREADING, "Archivo reabrierto");
             }
             else{
              ESP_LOGE(TAG_DATAREADING, "NO SE HA PODIDO REABRIR EL ARCHIVO");
             }
           }
        }
      }
      counterPrint++;
    }
    else {
      Serial.println("err");
    }

  }
  Serial.println("!!!out TaskDataRead");
  digitalWrite(PWR_RS485_EN, LOW);
  data_read_task_handle = NULL;
  vTaskDelete( NULL );
}
