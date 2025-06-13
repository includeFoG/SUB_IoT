
//*****************************************************************
// Thread TaskWiFi
//*****************************************************************

//#define DATE_STRING_SHORT           3

// Month names in flash
//const char monthNames_P[] PROGMEM = "JanFebMarAprMayJunJulAugSepOctNovDec";
// Day of the week names in flash
//const char dayNames_P[] PROGMEM = "SunMonTueWedThuFriSat";
TaskHandle_t BLE_task_handle;
TaskHandle_t wifi_task_handle;
void TaskWIFI( void *pvParameters ) {
  printf("[TaskWIFI Started]\n");
  myDelayMs(1000);
  WiFi.mode(WIFI_STA); //indicamos conexión a punto de acceso al iniciar el WiFi
  //sincroniza NTP con el RTC_INT
  sntp_set_time_sync_notification_cb( timeavailable ); //configuracion del ntp, setea un callback timeavailable() que actualiza el RTC interno de forma automática. Se ejecuta timeavailable en cada sincronizacion
  esp_sntp_servermode_dhcp(1);//sntp_servermode_dhcp(1);    // (optional) Enable sntp from dhcp
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);  //initialice ntp with GMT config //!!!!HAY QUE PROBAR A TRABAJAR CON UTC HACIENDO CAMBIOS DE CONFIGURACIONES

  printf("Init WiFi\n");
  WiFi.begin(SSID, SSID_PASS);
  Serial.print("CONNECTING TO: ");
  Serial.print(SSID);

  long timerConnection = millis(); //valor tomado para alertar en caso de no poder conectar
  EventBits_t preUxBits;

  while (WiFi.status() != WL_CONNECTED) {
    myDelayMs(500);
    Serial.print(".");
    preUxBits = xEventGroupGetBits(xEventWifi);

    if ( ( preUxBits & BIT_0_DESC_WIFI ) != 0 ) //Si está habilitado el bit de desconectar wifi
    {
      WiFi.disconnect();
      xEventGroupClearBits(xEventWifi, BIT_0_DESC_WIFI);
      wifi_task_handle = NULL;
      vTaskDelete( NULL );
    }

    if (millis() - timerConnection > 15000) {

      if (xSemaphoreTake(BLE_txSemaphore, pdMS_TO_TICKS(100)) == pdTRUE)
      {
        txBLE = "Cant connect to WiFi, please stop WiFi and reconfig\n"; //para que esto se imprima tiene que llegar al final del loop de BT
        xSemaphoreGive(BLE_txSemaphore);
      }
      timerConnection = millis();
    }
  }

  Serial.println();
  myDelayMs(50);
  if (xSemaphoreTake(BLE_txSemaphore, pdMS_TO_TICKS(100)) == pdTRUE)
  {
    txBLE = "Connected to ";
    txBLE += String(SSID).c_str();
    txBLE += "\n";
    txBLE += "IP address: ";
    txBLE += String(WiFi.localIP().toString()).c_str();
    txBLE += "\n\n\n";
    xSemaphoreGive(BLE_txSemaphore);
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    while (1) {
      Serial.println("Error setting up MDNS responder!");
      myDelayMs(1000);
    }
  }

  Serial.println("mDNS responder started");

  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();

  EventBits_t uxBits;
  for (;;)
  {
    server.handleClient();

    uxBits = xEventGroupWaitBits(
               xEventWifi,   // The event group being checked.   The event group in which the bits are being checked
               BIT_0_DESC_WIFI | BIT_1_ACTU_RTC_EXT, /* The bits within the event group to wait for. */
               pdTRUE,        /* BIT_0_DESC_WIFI & BIT_1_ACTU_RTC_EXT should be cleared before returning. */  //Solo pasa una vez por lo que configuramos que se limpien solos al salir, no es necesario a posteriori hacer un clear
               pdFALSE,       /* Don't wait for both bits, either bit will do. */
               10 );/* Wait a maximum of 100ms for either bit to be set. */

    if ( ( uxBits & BIT_0_DESC_WIFI ) != 0 ) //Si está habilitado el bit de desconectar wifi
    {
      /* xEventGroupWaitBits() returned because just BIT_0_DESC_WIFI was set. */
      server.stop(); //para el servidor
      myDelayMs(100);

      if (BLE_task_handle == NULL) // SI NO EXISTE HILO DE BT
      {
        Serial.println("[WiFi] CHANGING STATE TO IDDLE");
        if (xSemaphoreTake(manager_semaphore,portMAX_DELAY) == pdTRUE) {
          STATE = IDDLE; //consideramos que ya estamos configurados
          xSemaphoreGive(manager_semaphore); //ADDED 0.7.6
        }
      }
      WiFi.disconnect();
      wifi_task_handle = NULL;
      vTaskDelete( NULL );

    }
    if ( ( uxBits & BIT_1_ACTU_RTC_EXT ) != 0 ) //Si está habilitado el bit de actualizar el RTC_EXTERNO en función del RTC_INTERNO
    {
      struct tm t2;
      digitalWrite(PWR_V3_EN, HIGH); ////Alimenta RS485 INTERNO, SD, IMU, PULL-UP I2C
      /* xEventGroupWaitBits() returned because just BIT_0_DESC_WIFI was set. */
      //Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));

      //Serial.print("\nTime-NTP: "); //!!! hay que ver aquí si usamos printlocaltime o gettimestruct
      //printLocalTime();

      Serial.print("[WIFI] RTC-EXT: ");
      rtcEXT.read(&t2);
      Serial.println(&t2, "%A, %B %d %Y %H:%M:%S");

      struct tm t1 = rtc.getTimeStruct(); //hay que llamar a getTime para poder actualizar el valor de T1

      Serial.print("[WIFI] RTC-INT: ");
      Serial.println(&t1, "%A, %B %d %Y %H:%M:%S");


      double seconds = difftime(mktime(&t1), mktime(&t2));
      Serial.print("[WIFI] Difference: ");
      Serial.println(seconds);

      t2 = rtc.getTimeStruct();
      rtcEXT.write(&t2);

      Serial.print("[WIFI] RTCEXT actualized: ");
      rtcEXT.read(&t2);
      Serial.println(&t2, "%A, %B %d %Y %H:%M:%S");

      t2.tm_year = t2.tm_year - 100 + 2000; //transformación para ver el año en tiempo real (t2.tm_year = años desde 1900)
      t2.tm_mon++; //enero=0

      if (xSemaphoreTake(BLE_txSemaphore, pdMS_TO_TICKS(100)) == pdTRUE)
      {
        txBLE = (String(t2.tm_year) + "/" + String(t2.tm_mon) + "/" + String(t2.tm_mday) + " " + String(t2.tm_hour) + ":" + String(t2.tm_min) + ":" + String(t2.tm_sec)).c_str();
        //txBLE = String(&t2, "%A, %B %d %Y %H:%M:%S").c_str();
        txBLE += "\n\n\n";
        xSemaphoreGive(BLE_txSemaphore);
      }

    }
  }
  Serial.println("!!!out TaskWifi");
  wifi_task_handle = NULL;
  vTaskDelete( NULL );
}
