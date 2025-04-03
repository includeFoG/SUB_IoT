/*
   Formato de comando
   '$' Caracter de inicio
   stcal / imucal / cfgwifi / stopwifi / syncrtc  / viewdata / datasesion /stopdata /close
   '/n' Caracter de fin
*/
cmd_BLE cmd_decoder (char* buffer)
{
  Serial.print("\n\n>>[BLE] Command received: ");
  Serial.println(buffer);
  if ( buffer[0] != '$')return ADV_ERROR;
  if (strcmp(buffer, "$sgcal\n") == 0)return CMD_SG_CALIB;
  if (strcmp(buffer, "$imucal\n") == 0)return CMD_IMU_CALIB;
  if (strcmp(buffer, "$cfgwifi\n") == 0)return CMD_CFG_WIFI;
  if (strcmp(buffer, "$stopwifi\n") == 0)return CMD_STOP_WIFI;
  if (strcmp(buffer, "$syncrtc\n") == 0)return CMD_CFG_RTC;
  if (strcmp(buffer, "$viewdata\n") == 0)return CMD_VIEW_DATA;
  if (strcmp(buffer, "$datasesion\n") == 0)return CMD_DATA_SESION;
  if (strcmp(buffer, "$stopdata\n") == 0)return CMD_STOP_DATA;
  if (strcmp(buffer, "$close\n") == 0)return CMD_END_BLE;

  if (strcmp(buffer, "$batinfo\n") == 0)return CMD_BAT_INFO;

  return ADV_ERROR;
}
//*****************************************************************
// Thread taskBLE
//*****************************************************************
void TaskBLE( void *pvParameters )
{
  bool f_waitWifiParam = false;       //flag que indica si se está esperando a que el usuario introduzca un parámetro del wifi
  bool f_statusConfigWifi = false;    //flag que indica el estado de configuración inicialmente Wifi false = pendiente de SSID , true = pendiente de SSID_PASS

  bool f_waitSGParam = false;         //flag que indica si se está esperando a que el usuario introduzca un parámetro de la base SG

  bool f_configIMU = false;           //flag que indica si se encuentra en proceso de calibración de IMU
  bool f_choosingModeCalibIMU = false;//flag usado en la configuración de la IMU , determina si el usuario se encuentra eligiendo el modo de recalibración o calibración normal.
  bool f_choosePosition = false;      //flag usado en la configuración de la IMU , determina si el número introducido por BLE debe asignarse al case de estudio o no.

  bool b_modeCalibIMU = false;     //flag que indica el modo de calibración, false = calibración desde parámetro de fábrica, true = calibración desde último valor de calibración.
  uint8_t i_statusConfigIMU = 0;   //flag que indica el estado de configuración 0=calibración eje (-)Z, 1= ,2=,3=,4=,5=,6=
  int8_t i_statusConfigSG = 0;    //flag que indica el estado de configuración 0 = envía valor de peso para ajustar, 1 = confirma la configuración, 2 = error

  EventBits_t uxbitsDatos;

  BLEDevice::init(BLE_ID);
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();
  // Start advertising
  pServer->getAdvertising()->start();

  Serial.println("[BLE] Waiting a client...");

  bool f_start_CD = false; //flag que indica el inicio de la cuenta atras para eliminar el hilo en función de TIME_BLE_WAIT_DEVICE
  long start_time_CD = 999999;     //tiempo de inicio de CountDown
  uint8_t aux_pre = 0; //usado para no imprimir dos veces el mismo valor por pantalla (tiempo en segundos) cuando no se conecta un dispositivo
  std::string rxDataRS485 = "";
  std::string valueSG = ""; //valor introducido por el usuario mediante BT y transmitido a la base SG mediante RS485
  String aux_String = "";

  for (;;)
  {
    uxbitsDatos = xEventGroupGetBits(xEventDatos);

    blinkLed(3, 100);
    if (f_rx_ble_data) //si se ha recibido algo por BLE
    {
      if (!f_waitWifiParam && !f_waitSGParam && !f_configIMU) //si no se está esperando por un parámetro de configuración wifi ni de la base
      {
        cmd_BLE cmd = cmd_decoder((char*)rxBLE.c_str());
        switch (cmd)
        {
          case CMD_SG_CALIB: //COMANDO DE CALIBRACION DE GALGA SOLO FUNCIONA SI SE ACABA DE ENCENDER LA BASE SG
            //PENDIENTE: Comprobar estado evento 3.3v , en caso de estar previamente activo preguntar si se quiere apagar y volver a encender para entrar en modo configuración
            Serial.println("[BLE] Gauge Calibration,Send any character when ready and wait BT");
            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "Gauge Calibration, \n\tSend any character when ready and wait\n";
              xSemaphoreGive(BLE_txSemaphore);
            }
            rxDataRS485 = "";

            f_waitSGParam = true;
            i_statusConfigSG = -2; //vamos a info de modo previo envio de peso

            break;


          case CMD_IMU_CALIB: //COMANDO DE CALIBRACION DE IMU
            Serial.println("[BLE] IMU calibration");

            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "IMU calibration started:\n\t\tSend 0 for factory calibration(first calibration)\n\t\tSend 1 for recalibration\n";//Send 0 to calibrate horizontal (white top)\n\t\tSend 1 to calibrate vertical (connector botton)\n";
              xSemaphoreGive(BLE_txSemaphore);
            }
            f_configIMU = true;
            f_choosingModeCalibIMU = true; //flag que indica que se encuentra eligiendo modo de calibración (recalibración o calibración normal)
            break;


          case CMD_CFG_WIFI://COMANDO DE CONFIGURACION WIFI
            Serial.println("[BLE] WiFi Configuration");

            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "WiFi Configuration\nType SSID:\n";
              xSemaphoreGive(BLE_txSemaphore);
            }

            f_waitWifiParam = true; //habilitamos el flag de espera de parámetro wifi
            f_statusConfigWifi = false; //Ponemos el estado de config wifi a false (pendiente de SSID)
            break;


          case CMD_STOP_WIFI: //COMANDO DE DESACTIVACION WIFI
            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              if (wifi_task_handle != NULL) {
                Serial.println("[BLE] Disconnecting WiFi");
                txBLE = "Disconnecting WiFi\n\n\n";
                xEventGroupSetBits(xEventWifi, BIT_0_DESC_WIFI);
              }
              else {
                Serial.println("[BLE] WiFi not initialized");
                txBLE = "WiFi not initialized\n";
              }
              xSemaphoreGive(BLE_txSemaphore);
            }
            break;


          case CMD_CFG_RTC: //COMANDO DE ACTUALIZACION RTC
            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              if (wifi_task_handle != NULL) {
                Serial.println("[BLE] RTC Configuration\n");
                txBLE = "RTC Configuration\n";
                xEventGroupSetBits(xEventWifi,  BIT_1_ACTU_RTC_EXT );
              }
              else {
                Serial.println("[BLE] Wifi not initialized");
                txBLE = "WiFi not initialized\n";
              }
              xSemaphoreGive(BLE_txSemaphore);
            }
            break;


          case CMD_VIEW_DATA: //COMANDO PARA VISUALIZAR LOS DATOS ENVIADOS POR EL DISPOSITIVO TRANSDUCTOR BASE
            Serial.println("[BLE] Data Visualization");

            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "Data Visualization\n";
              xSemaphoreGive(BLE_txSemaphore);
            }
            xEventGroupSetBits(xEventDatos, BIT_0_DAT_VIEW);
            break;

          case CMD_DATA_SESION: //COMANDO PARA INICIALIZAR LA GRABACION DE UNA SESION
            Serial.println("[BLE] Preparing Sesion");

            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "Preparing sesion\n";
              xSemaphoreGive(BLE_txSemaphore);
            }
            xEventGroupSetBits(xEventDatos, BIT_1_DAT_SESION);

            break;


          case CMD_STOP_DATA: //COMANDO PARA FINALIZAR LA VISUALIZACION Y GRABACION DE UNA SESION
            if ((uxbitsDatos & (BIT_1_DAT_SESION | BIT_0_DAT_VIEW)) != 0) { //si se está trabajando con los datos
              Serial.println("[BLE] Stop Data");
              if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
              {
                txBLE = "Stop data\n";
                xSemaphoreGive(BLE_txSemaphore);
              }

              xEventGroupClearBits(xEventDatos, BIT_0_DAT_VIEW );

              if ((uxbitsDatos & BIT_1_DAT_SESION) != 0) { //si está en medio de una sesión
                xEventGroupClearBits(xEventDatos, BIT_1_DAT_SESION ); //limpia bit de sesión
                //xEventGroupSetBits(xEventDatos, BIT_2_END_SESION );   //levanta bit de cierre de archivo
              }
            }
            else { //si no se está trabajando con datos
              Serial.println("[BLE] Data Visualization NOT init");
              if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
              {
                txBLE = "Data Visualization NOT init\n";
                xSemaphoreGive(BLE_txSemaphore);
              }
            }
            break;


          case CMD_END_BLE: //TERMINA CON LA TAREA DE BLE
            if (wifi_task_handle != NULL) {
              if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
              {
                txBLE = "\nClosing WiFi Thread";
                Serial.println(txBLE.c_str());
                xSemaphoreGive(BLE_txSemaphore);
              }
              xEventGroupSetBits(xEventWifi, BIT_0_DESC_WIFI);

              EventBits_t uxbitsWifi = xEventGroupGetBits(xEventWifi);

              while ((uxbitsWifi & BIT_0_DESC_WIFI) != 0) { //wifi_task_handle != NULL) { //mientras siga habilitado Wifi
                uxbitsWifi = xEventGroupGetBits(xEventWifi);
                if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
                {
                  txBLE = ".";
                  Serial.print(txBLE.c_str());
                  xSemaphoreGive(BLE_txSemaphore);
                }
                myDelayMs(10);
              }
              Serial.println("wifi_task_handle=Null");
            }

            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "Closing BLE conn and task";
              Serial.println(txBLE.c_str());
              Serial.println();
              xSemaphoreGive(BLE_txSemaphore);
            }
            myDelayMs(50);

            STATE = IDDLE; //consideramos que ya estamos configurados

            BLEDevice::deinit(false);
            BLE_task_handle = NULL;
            vTaskDelete(NULL);
            break;

          case CMD_BAT_INFO: //consultar estado de batería
             Serial.println("[BLE] [Battery Info] - ");

            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "[Battery info]:\nChrg[%],Vol[mV],AvgCur[mA],Cap[mAh],FullCap[mAh],Pwr[mW],Health[%]:\n";
              txBLE += getBatteryStatus(true);
              txBLE +="\n\n";
              xSemaphoreGive(BLE_txSemaphore);
            }
            break;


          default:
            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "Command no recognized\n";
              Serial.print(txBLE.c_str());
              Serial.print(": " + String(cmd) + "\n");
              xSemaphoreGive(BLE_txSemaphore);
            }
            break;
        }
      }
      else if (f_waitWifiParam) //si se está esperando un parámetro de configuración wifi
      {
        switch ((int)f_statusConfigWifi)
        {
          case 0: //pendiente de SSID
            Serial.println("SSID recived");
            SSID = (String)rxBLE.c_str(); //asigna valor enviado por BLE para SSID-Wifi
            SSID = SSID.substring(0, SSID.length() - 1); //quita caracter final

            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "Type Pass:\n";
              xSemaphoreGive(BLE_txSemaphore);
            }

            f_statusConfigWifi = true; //cambia estado a pendiente de SSID_PASS
            break;//? se puede quitar el break para que pase directamente al case1

          case 1: //pendiente de SSID_PASS
            SSID_PASS = (String)rxBLE.c_str();//asigna valor enviado por BLE para password-wifi
            SSID_PASS = SSID_PASS.substring(0, SSID_PASS.length() - 1); //quita caracter final

            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "WiFi Config Complete, please wait\n";
              xSemaphoreGive(BLE_txSemaphore);
            }
            Serial.println("[BLE] WiFi Config Complete, please wait");
            Serial.print("[BLE] SSID: "); Serial.print(SSID); Serial.print(" PASS: "); Serial.println(SSID_PASS);
            f_statusConfigWifi = false;  //cambia estado a estado inicial pendiente de SSID
            f_waitWifiParam = false;     //deshabilitamos el flag de espera de parámetro de configuracion wifi
            xTaskCreatePinnedToCore(TaskWIFI,  "Task WIFI" ,  4096,  NULL,  1,  &wifi_task_handle,  ARDUINO_RUNNING_CORE); //INICIAMOS HILO DE WIFI
            break;
          default:
            break;
        }
      }
      else if (f_waitSGParam) { //si se está esperando un parámetro de configuración de la base SG
        Serial.print("[BLE] Value for SG recived by BT: "); //recibido mediante callback de BT

        valueSG = rxBLE; //asigna valor enviado por BLE para Base SG

        aux_String = (String)valueSG.c_str();
        aux_String.replace("\n", ""); //eliminamos posibles caracteres introducidos por el terminal BT para poder trabajar con el dato
        aux_String.replace("\r", "");
        aux_String.replace("\0", "");
        valueSG = (std::string(aux_String.c_str()));

        Serial.println(valueSG.c_str());
        bool comeFromCase0 = false;
        switch (i_statusConfigSG) {
          case -2:
            //PENDIENTE: Comprobar estado evento 3.3v , en caso de estar previamente activo preguntar si se quiere apagar y volver a encender para entrar en modo configuración
            rxDataRS485 = "";
            rxDataRS485 = waitResponseRS485Command(); //comprueba que el dispositivo base SG está en modo configuración [ready for commands]

            if (rxDataRS485 != "") { //si hay respuesta
              //enviamos comando de configuración
              sendTransmisionRS485("$sgcal");

              rxDataRS485 = waitResponseRS485Command();

              if ((String)rxDataRS485.c_str() == "[OK]") //esperamos respuesta de confirmación [OK]
              {
                //ESTO HACE QUE EN BASE SG EMPIECE gaugeCalibration(true)
                Serial.println("[BLE] SG Calibration Started");
                //si ha llegado hasta aquí se ha iniciado gaugeCalibration(true) en base SG
                //en primer lugar envía las medidas de calculo de offset
                Serial.println("[BLE] Calculating offset, please retire all the forces in the body. \tSend any character when ready and wait, this process could take a few secs");
                if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
                {
                  txBLE = "Calculating offset\n\t1.please retire all the forces in the body.\n\t2.Send any character when ready and wait,\n\tthis process could take a few secs\n";
                  xSemaphoreGive(BLE_txSemaphore);
                }
                rxDataRS485 = "";
                i_statusConfigSG = -1; //lo enviamos al siguiente case
              }
              else {
                Serial.print("[BLE] Reception RS485 Error: "); Serial.println(rxDataRS485.c_str());
                if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
                {
                  txBLE = "Error, please check connection and reset the system: ";
                  txBLE += rxDataRS485;
                  xSemaphoreGive(BLE_txSemaphore);
                }
              }

              rxDataRS485 = "";
            }
            else { // si no hay respuesta
              Serial.println("[BLE] No response");
              if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
              {
                txBLE = "no response\n";
                xSemaphoreGive(BLE_txSemaphore);
              }
              //FALTA HACER ALGO PARA FINALIZAR AQUI
            }
            break;
          case -1:
            sendTransmisionRS485("Y"); //enviamos aceptación de condición NO PESO
            while (rxDataRS485 != "[endMeasures]") {
              rxDataRS485 =  waitResponseRS485Command();
            }
            Serial.println();

            rxDataRS485 = "";
            //una vez pasadas las medidas envía el offset calculado [ 1 , 2 ]
            waitResponseRS485Command();
            //después base SG llama a SetScale(,, true)
            //base envía "[Put the weigth to set Slope ID:]" y espera que datalogger envíe un valor (introducido desde BT)
            waitResponseRS485Command();
            Serial.println("[BLE] Place weight and introduce value for SG1 by BT"); //tiene que volver a pasar por el bucle para que refresque valor introducido por BT ir a (f_waitSGParam)
            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "\nPlace weight and introduce value for SG1\n";
              xSemaphoreGive(BLE_txSemaphore);
            }
            i_statusConfigSG = 0; //vamos a envío de peso
            break;
          case 0: //envio de valor de peso
            Serial.println("sending valueSG");
            sendTransmisionRS485(valueSG); //envia el valor de peso a base SG

            //base SG responde con [raw measure ID: valor medido] si el valor es válido, en caso contrario responde [Incorrect Format]
            if (waitResponseRS485Command() == "[Incorrect Format]") {
              comeFromCase0 = true;
              //ESTE CASE CONTINUA AL CASE 2
            }
            else {
              if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
              {
                //base SG responde con [weigth measure ID: valor medido kg]
                txBLE =  waitResponseRS485Command();
                txBLE += "\n\t";
                //seguidamente pide confirmación  [Y: to confirm / N: to modif]
                txBLE +=  waitResponseRS485Command();
                txBLE += "\n";
                xSemaphoreGive(BLE_txSemaphore);
              }
              //hay que salir y volver a entrar para leer lo que el usuario escriba
              i_statusConfigSG = 1; //vamos a confirmación

              break;
            }

          case 1: //una vez asignado el valor del peso se envía confirmación de configuración
            if (!comeFromCase0) { //si no viene de case0
              sendTransmisionRS485(valueSG); //envia el valor  Y ó N

              if ( valueSG == "Y" ) {
                //base SG responde con [END CAL. : ID]
                rxDataRS485 = waitResponseRS485Command();
                if (((String)rxDataRS485.c_str()).indexOf("1") != -1) {
                  //ESPERAMOS A QUE NOS LLEGE [Put the weigth to set Slope ID]
                  waitResponseRS485Command();
                  Serial.println("Please introduce weight value for SG2 by BT");
                  if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
                  {
                    txBLE = "\nPlease introduce weight value for SG2\n";
                    xSemaphoreGive(BLE_txSemaphore);
                  }
                }
                else if (((String)rxDataRS485.c_str()).indexOf("2") != -1) {
                  f_waitSGParam = false; //sale del modo de configuración SGParam
                  //ESPERAMOS A RECIBIR [CALIBRATION COMPLETED]
                  waitResponseRS485Command();
                  //ESPERAMOS A RECIBIR [Out of config mode]
                  waitResponseRS485Command();
                  Serial.println("[BLE] OK SG - END CONFIG"); //LA BASE SG YA SE ENCUENTRA LIBRE Y EN EL LOOP
                  if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
                  {
                    txBLE = "\n\nOK SG - END CONFIG\n\n\n";
                    xSemaphoreGive(BLE_txSemaphore);
                  }
                }
              }
              else { //puede ser "N" o valor erroneo
                //base SG responde con : [|  ReSend weigth to set scale]
                Serial.println("TESTF");
                waitResponseRS485Command();
                //vuelve a el case superior
                Serial.println("Introduce the value again by BT"); //permanece en el modo de configuración SGParam
                if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
                {
                  txBLE = "Introduce the value again by BT\n";
                  xSemaphoreGive(BLE_txSemaphore);
                }
              }
              i_statusConfigSG = 0;//reseteamos en envío de peso
              break;
            }
            else { //si viene de case0 continua a case2
              comeFromCase0 = false;
            }
          case 2:
            Serial.println("[BLE] ERROR in format, please send weight again\n Should be > 0");
            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "\nERROR in format, please send the weight again\n";
              xSemaphoreGive(BLE_txSemaphore);
            }
            i_statusConfigSG = 0;//volvemos a envio de peso
            break;
        }
      }
      else if (f_configIMU) { //si se ha iniciado el proceso de calibración de la IMU esta parte del código enciende y APAGA la alimentación del i2c
        Serial.print("[BLE] Value for IMU recived by BT: "); //recibido mediante callback de BT

        aux_String = (String)rxBLE.c_str();
        aux_String.replace("\n", ""); //eliminamos posibles caracteres introducidos por el terminal BT para poder trabajar con el dato
        aux_String.replace("\r", "");
        aux_String.replace("\0", "");

        Serial.println(aux_String.toInt());

        std::string aux_stringMode = "";

        if (f_choosingModeCalibIMU) { //el flag de mode es el primer valor introducido en la config de la IMU, determina si se va a hacer una recalibración
          b_modeCalibIMU = bool(aux_String.toInt()); //si mete algún parámetro erroneo devolverá 0 entrando en modo factory calibration

          if (b_modeCalibIMU) {
            aux_stringMode = "Recalibration";
          }
          else {
            aux_stringMode = "Factory calibration";
          }
          if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
          {
            txBLE = aux_stringMode;
            txBLE += "\n\t\tSend 0 to calibrate horizontal (white top)\n\t\tSend 1 to calibrate vertical (connector botton)\n";
            xSemaphoreGive(BLE_txSemaphore);
          }
          f_choosingModeCalibIMU = false;
          f_choosePosition = true;
          i_statusConfigIMU = -1; //para que pase por default y continue
        }

        else if (f_choosePosition) { //si es el primer valor introducido
          digitalWrite(PWR_V3_EN, HIGH); //habilita  PULL-UP I2C

          i_statusConfigIMU = aux_String.toInt();
          f_choosePosition = false; //quitamos que el usuario sea el que determine el siguiente case, lo hará el case en sí mismo
        }

        switch (i_statusConfigIMU) {
          case 0: //calibración horizontal tapa blanca hacia arriba
            Serial.println("Horizontal Calibration");
            calibrateIMU(b_modeCalibIMU, true);

            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "IMU Calibrated\n\nSelect Acel Sens:\n\t1:+-2g\n\t2:+-4g\n\t3:+-8g;\n\t4:+-16g\n";
              xSemaphoreGive(BLE_txSemaphore);
            }
            i_statusConfigIMU = 2; //vamos a la selección de sensibilidad
            break;

          case 1: //calibración vertical conectores hacia abajo
            Serial.println("Vertical Calibration");
            calibrateIMU(b_modeCalibIMU, false);

            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "IMU Calibrated\n\nSelect Acel Sens:\n\t1:+-2g\n\t2:+-4g\n\t3:+-8g;\n\t4:+-16g\n"; //he tenido que aumentar en 1 el indice porque toint() devuelve 0 si hay error (ver case 2)
              xSemaphoreGive(BLE_txSemaphore);
            }
            i_statusConfigIMU = 2; //vamos a la selección de sensibilidad
            break;

          case 2: //selección de sensibilidad de Acelerómetro
            if (aux_String.toInt() >= 1 && aux_String.toInt() <= 3) { //nos aseguramos que sea un valor válido
              sensorIMU.setFullScaleAccelRange(aux_String.toInt() - 1); //corregimos el indice sumado
              nvs_val_sens_acel_IMU = aux_String.toInt() - 1;

              write_nvs_16(nvs_key_sens_acel_IMU, nvs_val_sens_acel_IMU); //lo guardamos en nvs

              if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
              {
                txBLE = "Select Gyro Sens:\n\t1:+-250º/s\n\t2:+-500º/s\n\t3:+-1000º/s;\n\t4:+-2000º/s\n";
                xSemaphoreGive(BLE_txSemaphore);
              }
              i_statusConfigIMU = 3; //vamos a la selección de sens de giro
            }
            else { //si no es un valor válido le obligamos a volver a pasar por el case
              if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
              {
                txBLE = "ERROR in format, please:\nSelect Acel Sens:\n\t1:+-2g\n\t2:+-4g\n\t3:+-8g;\n\t4:+-16g\n";//he tenido que aumentar en 1 el indice porque toint() devuelve 0 si hay error (ver case 2)
                xSemaphoreGive(BLE_txSemaphore);
              }
            }
            break;

          case 3: //selección de sensibilidad de Giroscópio
            if (aux_String.toInt() >= 1 && aux_String.toInt() <= 3) {//nos aseguramos que sea un valor válido
              sensorIMU.setFullScaleGyroRange(aux_String.toInt() - 1);
              nvs_val_sens_giro_IMU = aux_String.toInt() - 1;
              write_nvs_16(nvs_key_sens_giro_IMU, nvs_val_sens_giro_IMU); //lo guardamos en nvs

              i_statusConfigIMU = 4; //vamos a fin de calibración, se cambia el valor por si posteriormente hay un error en el case 4 que haga que tenga que volver al case 4
              //FALLTHROUGH
            }
            else { //si no es un valor válido le obligamos a volver a pasar por el case
              if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
              {
                txBLE = "ERROR in format, please:\nSelect Gyro Sens:\n\t1:+-250º/s\n\t2:+-500º/s\n\t3:+-1000º/s;\n\t4:+-2000º/s\n";
                xSemaphoreGive(BLE_txSemaphore);
              }

              break;
            }

          case 4: //fin de calibración
            f_configIMU = false;
            i_statusConfigIMU = 0;

            digitalWrite(PWR_V3_EN, LOW); //deshabilita  PULL-UP I2C

            Serial.println("\n[BLE] End of process of calibration\n");

            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "\n OK - End of process of calibration\n";
              xSemaphoreGive(BLE_txSemaphore);
            }
            break;
          default: //error en el envío o paso sin hacer nada
            break;
        }
        aux_String = "";
      }

      f_rx_ble_data = false; //limpiamos flag de recepcion
      rxBLE = "";
    }


    if (f_BLE_deviceConnected && (txBLE.length() > 0)) { //Si hay algun dispositivo conectado por BLE y estamos pendientes de realizar una transmisión
      //-> debería estar en otro hilo para no depender de llegar aquí para enviar un mensaje
      pTxCharacteristic->setValue((String)rxBLE.c_str()); //establece el nuevo valor de la característica
      pTxCharacteristic->notify();        //notifica el nuevo valor a los dispositivos conectados
      if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE) //? Si tenemos acceso limpia el buffer pero y si no?
      {
        txBLE = "";
        xSemaphoreGive(BLE_txSemaphore);
      }
      //txValue++;
      //delay(1000); // bluetooth stack will go into congestion, if too many packets are sent
    }


    if (!f_BLE_deviceConnected && f_oldDeviceConnected) { //Si no hay un dispositivo conectado por BLE pero había un dispositivo conectado -> DISCONNECTING
      myDelayMs(500); // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
      Serial.println("[BLE] Device disconnected");
      f_oldDeviceConnected = f_BLE_deviceConnected; //actualiza el estado del f_oldDeviceConnected
    }
    else if (f_BLE_deviceConnected && !f_oldDeviceConnected) {//Si hay algun dispositivo conectado por BLE y antes no lo había -> CONNECTING
      // do stuff here on connecting
      Serial.println("[BLE] Device connected");
      f_oldDeviceConnected = f_BLE_deviceConnected; //actualiza el estado del f_oldDeviceConnected
    }


    if ( ( ( xEventGroupGetBits(xEventDatos) & ( BIT_0_DAT_VIEW | BIT_1_DAT_SESION ) ) != 0)  && (data_read_task_handle == NULL))  //Si está activo algun flag de datos y no esta iniciado el hilo de task read data
    {
      xTaskCreatePinnedToCore(TaskDataReading,  "Task Read" ,  4096        ,  NULL,  1  ,  &data_read_task_handle ,  ARDUINO_RUNNING_CORE); //INICIAMOS HILO TASK READ DATA
    }

    if (!f_BLE_deviceConnected ) //si no hay ningún dispositivo conectado
    {
      if (!f_start_CD) //si no está iniciado el flag de CountDown
      {
        f_start_CD = true;
        start_time_CD = millis();
        aux_pre = 0;
        Serial.print("[BLE] time until delete taskBLE (s):");
      }
      else { //Si ya estaba iniciado el flag de CountDown
        if (millis() - start_time_CD > TIME_BLE_WAIT_DEVICE * 1000) { //si se pasa el tiempo para que un dispositivo se conecte

          if (wifi_task_handle != NULL) {
            if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
            {
              txBLE = "\nClosing WiFi Thread ";
              Serial.println(txBLE.c_str());
              xSemaphoreGive(BLE_txSemaphore);
            }
            xEventGroupSetBits(xEventWifi, BIT_0_DESC_WIFI);

            EventBits_t uxbitsWifi = xEventGroupGetBits(xEventWifi);

            while ((uxbitsWifi & BIT_0_DESC_WIFI) != 0) { //wifi_task_handle != NULL) { //mientras siga habilitado Wifi
              uxbitsWifi = xEventGroupGetBits(xEventWifi);
              if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
              {
                txBLE = ".";
                Serial.print(txBLE.c_str());
                xSemaphoreGive(BLE_txSemaphore);
              }
              myDelayMs(10);
            }
            Serial.println("wifi_task_handle=Null");
          }

          if (xSemaphoreTake(BLE_txSemaphore, 0) == pdTRUE)
          {
            txBLE = "Closing BLE Thread";
            Serial.println(txBLE.c_str());
            xSemaphoreGive(BLE_txSemaphore);
          }
          STATE = IDDLE; //consideramos que ya estamos configurados

          Serial.println("STATE = IDDLE\n");

          BLEDevice::deinit(false);
          BLE_task_handle = NULL;
          vTaskDelete(NULL);
        }
        else {
          uint8_t aux_seconds = ((TIME_BLE_WAIT_DEVICE) - ((millis() - start_time_CD) / 1000)); //tiempo que queda para que se cierre el hilo si no se conecta un dispositivo
          if ((aux_seconds % 10 == 0) && (aux_pre != aux_seconds)) {
            Serial.println(aux_seconds);
            aux_pre = aux_seconds;
          }
        }
      }

    }
    else //si hay dispositivo conectado
      f_start_CD = false; //desactiva flag de inicio cuenta atras para cerrar hilo


    myDelayMs(10);
  }
  Serial.println("!!!out TaskBLE");
  BLE_task_handle = NULL;
  vTaskDelete( NULL );
}
