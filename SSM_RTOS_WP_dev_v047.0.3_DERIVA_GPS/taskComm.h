TaskHandle_t Handle_CommTask;




int takeGPS(qPosition* localPosition);
// FreeMem: TOT 2328 PWR 40 AnR 56 Fil 134 COM 196 CTR 66 TIME: 540000


static bool powerOnComm()
{
  uint8_t _retry = 0;
  EventBits_t _status = xEventGroupGetBits( eventPwr5volts );

  if ( ( _status & BIT_1_comModule ) != 0 )
  {
    DEBUG("COM: POWER ALLREADY ON COMMUNICATION");
    // myDelayMs(100);
    //  myDelayMs(POWER_SENSOR_DELAY);
    //  myDelayMs(5000);
  }
  else
  {
    DEBUG("COM: POWER ON COMMUNICATION");

    xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr | BIT_1_comModule );

    initComPort();
    myDelayMs(100);
  }
  // while (((startComPort() != 0) && (_retry < RETRY_CONNECTION)) != 0)
  while ((startComPort(APN, APN_USER, APN_PASS) != 0) && (_retry < RETRY_CONNECTION) )
  {
    _retry++;
    SERIAL.print("COM: Reseting Modem: ");
    SERIAL.println(_retry);
    //powerOffComPort();//AÑADIDO 21/11/2023
    initComPort();
    myDelayMs(100);
  }
  DEBUG ( "COM: Start Com Port: " +  String(_retry));
  if (_retry >= RETRY_CONNECTION) return false;
  //  DEBUG("START GPS");
  //  modem.enableGPS();
  return true;
}


void powerOffComm()
{
  if (xEventGroupGetBits( eventPwr5volts ) & BIT_1_comModule) {
    myDelayMs(100);

    powerOffComPort();

    SERIAL.println("COM: COM MODULE OFF");
    xEventGroupClearBits( eventPwr5volts, BIT_1_comModule );
    xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr  );
    //taskYIELD();
    myDelayMs(2000);
  }
  else {
    DEBUG("COM: POWER COM ALLREADY OFF");
  }
}

int mapSignalQuality ( int csq)
{
  return ((csq * 2) - 113);
}
static int clearQandReset(int len, int &tryToSendFile);//newstatic
static int sendFile();//newstatic
static void threadComm( void *pvParameters )
{
  //static const int auxBinStatus = ( (B00001000 * 256) + B10000100 ); //ADDED  //WAIT4 B0000100010000100  //BIT_2_SendData | BIT_7_updateTime | BIT_11_iddle
  DEBUG("COM: Starting communication");
  //ADDED
  EventBits_t _status = xEventGroupGetBits( eventPwr5volts );
  if ( ( _status & BIT_1_comModule ) == 0 )
  {
    DEBUG("COM: POWER ON COMMUNICATION (0)");
    xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr | BIT_1_comModule );
    initComPort();
    myDelayMs(5000);
  }
  //ENDADDED
  powerOffComm(); //SIEMPRE VA A FALLAR PORQUE NO SE HA ENCENDIDO AUN EL MODULO
  //powerOnComm();
  struct qMessage localData;
  struct qPosition localPosition;
  //unsigned long start, finish = 0 ;
  //bool timeout = false;
  //TickType_t lastWakeTime = xTaskGetTickCount();
  //int flag = 0;
  int cont_retry = 0;
  //uint8_t tcp_retry = 0; //ADDED
  myDelayMs(5000);
  //static const String l_host = "qp29wxo1pf.execute-api.us-east-1.amazonaws.com";//"qp29wxo1pf.execute-api.us-east-1.amazonaws.com";//"f8i1bdejsc.execute-api.us-east-1.amazonaws.com";
  //static const String l_path = "/test0/ftp-test-private/WP-test";//"/pruebas/mytest-003";   // "/test1/ftp-test-private/WP-test";
  //String l_path = "/test0/ftp-test-private/PUBLIC";//"/pruebas/mytest-003";   // "/test1/ftp-test-private/WP-test";
  //String l_path = localConfig.APIPATH;//"/pruebas/mytest-003";   // "/test1/ftp-test-private/WP-test";  //!!! modificar cuando se añadan parametros

  uint8_t response = 0;
  //bool exe = true;

  //  uint8_t c[SIZE_BUFFER];
  //  uint8_t *idx = &c[0];
  //int sizeFile = 0;
  //int len = 0;
  EventBits_t uxBits = xEventGroupGetBits( myEvents );
  String fileName = "";
  int tryToSendFile = 0;
  int signalQuality;
  DEBUG ( "COM: SET BIT 13");

  //ADDED
#ifdef DEBUG_LOGEVENTS
  byte b_infoLog = NULL;
#endif
  //ENDADDED



  //ADDED CONFIG
  //  String aux_config = "";
  //  uint8_t stateQueue = 0; //utilizado para bit_19_
  //  int contentLength = 0; //se puede utilizar para leer el archivo descargado y comprobar que se ha guardado el mismo número de caracteres

  while (1) {
    xEventGroupSetBits( eventTaskState,  BIT_4_taskComm ); //ADDED WORKING

    //    lastWakeTime = xTaskGetTickCount();
    uxBits = xEventGroupGetBits( myEvents );


    if ((uxBits & BIT_7_updateTime) != 0)
    {
      DEBUG ( "COM: UPDATE TIME ");
      static bool retrys = false;

      if (powerOnComm())
      {
        myDelayMs(500);
        String t_updtTime = updateTime();
        if ( t_updtTime != "-1") {
          rtc.setEpoch(parseDate(t_updtTime));
          //ADDED
#ifdef DEBUG_LOGEVENTS
          b_infoLog = B00000111; //HORA COGIDA CORRECTAMENTE
          sendToEventsLogQueue(b_infoLog);
#endif
          //ENDADDED

          xEventGroupClearBits( myEvents,  BIT_7_updateTime );
          xEventGroupSetBits( myEvents,  BIT_15_updateRdy );
        }
      }
      if (retrys) {
        SERIAL.println("POWER OFF1");
        powerOffComm();
        myDelayMs(10000);    //myDelayMs(60000);//OCT MODIF
      }
      retrys = !retrys;
    }


    if ((uxBits & BIT_2_SendData) != 0 )
    {
      DEBUG ( "COM: SEND DATA ");
      if (powerOnComm())
      {
        if ( xCommQueue != NULL )
        {
          if ( xQueueReceive( xCommQueue, &( localData ), ( TickType_t ) SLOT_TIME ) == pdPASS )
          {
            xEventGroupSetBits( eventTaskState,  BIT_6_IM_sendData);
            xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr | BIT_5_buzzer);

            DEBUG("COM: SEND DATA TO UBIDOTS");

            SERIAL.println("COM: Taking GPS");
            unsigned long timeout_gps = millis();

            while (!(takeGPS(&localPosition) == 0) && (120000 > millis() - timeout_gps)) { //(120000 > millis() - timeout_gps));
              SERIAL.println("RETRYING GPS");
            }
            SERIAL.println("GPS done");

            DEBUG ("COM: Connecting to the server");

            vTaskPrioritySet( &Handle_CommTask , (tskIDLE_PRIORITY + 4)); //ADDED TEST
            while (!(serverConnect(HOST, USE_SSL, 9812)) && (cont_retry < RETRY_CONNECTION))//                                while ((!(serverConnect("industrial.api.ubidots.com", false, 80)) && (cont_retry < RETRY_CONNECTION)))
            { //                                                                                                              while ((!(serverConnect(USE_SSL)) && (cont_retry < RETRY_CONNECTION)))
              vTaskPrioritySet( &Handle_CommTask , (tskIDLE_PRIORITY + 2)); //ADDED TEST
              cont_retry++;
              SERIAL.print("COM: Retry conn:");
              SERIAL.println(cont_retry);

              if ( USE_SSL == false)   {   //ADDED TEST
                client.stop();         //ADDED TEST
              }
              else {                   //ADDED TEST
                clientSSL.stop();      //ADDED TEST
              }

              vTaskPrioritySet( &Handle_CommTask , (tskIDLE_PRIORITY + 4)); //ADDED TEST
            }

            // vTaskPrioritySet( &Handle_CommTask , (tskIDLE_PRIORITY + 2));

            if (cont_retry < RETRY_CONNECTION)
            {
              SERIAL.println("COM: Connected");

              //                DEBUG( "HTTP RESPONSE: " + String(sendPostToUbidots(localData.qData, localPosition, false)));
              //                client.stop();
              signalQuality =  mapSignalQuality ( modem.getSignalQuality());
              SERIAL.print( "Signal csq: ");
              SERIAL.println(signalQuality);
              int stat ;
              if ((uxBits & BIT_11_iddle) != 0) stat = 0;
              else stat = 1;

              response = 0;
              //taskYIELD(); //ADDED TEST

              do {
                vTaskPrioritySet( &Handle_CommTask , (tskIDLE_PRIORITY + 4)); //ADDED TEST
                response = postTCPToUbidots(localData.qData, localPosition, true, TOKEN, DEVICE, signalQuality, stat);
                vTaskPrioritySet( &Handle_CommTask , (tskIDLE_PRIORITY + 2)); //ADDED
                cont_retry++;

                //ADDED MODIF
                SERIAL.print("COM: TCP RESPONSE: ");
                SERIAL.print(response);
                SERIAL.println("\r\n");
              } while ((response < 100) && (cont_retry < RETRY_CONNECTION)); //&& modem.isNetworkConnected() ); MODIFICADO PARA V046


              vTaskPrioritySet( &Handle_CommTask , (tskIDLE_PRIORITY + 4)); //ADDED TEST

              //añadir if con cond no meassuring

              bool err_send = false;

              switch (getTCPFromUbidots(true)) //ADDED variable label (Switch): testswitchlabel //   getTCPFromUbidots( true, TOKEN, "624b1d570dcd2a30c4112697"); //ADDED variableID (Switch): 624b1d570dcd2a30c4112697
              {
                  vTaskPrioritySet( &Handle_CommTask , (tskIDLE_PRIORITY + 2)); //ADDED TEST
                case 0:
#ifdef DEBUG_LOGEVENTS
                  b_infoLog = B00001011; //DVR apagado desde ubidots
                  sendToEventsLogQueue(b_infoLog);
#endif
                  break;
                case 1:
                  xEventGroupSetBits( myEvents, BIT_18_ubidotsSwitch);

#ifdef DEBUG_LOGEVENTS
                  b_infoLog = B00001010; //DVR encendido desde ubidots
                  sendToEventsLogQueue(b_infoLog);
#endif

                  DEBUG("COM: CHANGING SWITCH STATE TO 0");
                  vTaskPrioritySet( &Handle_CommTask , (tskIDLE_PRIORITY + 4)); //ADDED TEST
                  response = postTCPToUbidots(localData.qData, localPosition, USE_SSL, TOKEN, DEVICE, signalQuality, stat, true);
                  vTaskPrioritySet( &Handle_CommTask , (tskIDLE_PRIORITY + 2)); //ADDED TEST
                  SERIAL.print( "COM: TCP RESPONSE: ");
                  SERIAL.print(response);
                  SERIAL.println("\r\n");
                  response = 0;
                  break;
                default:
                  DEBUG("COM: Error en el valor retornado de ubidots");
                  SERIAL.println("POWER OFF2");

                  err_send = true;

                  // powerOffComPort();//powerOffComm(); 02/06
                  break;
              }

              if ( USE_SSL == false)   {   //ADDED TEST
                client.stop();         //ADDED TEST
              }
              else {                   //ADDED TEST
                clientSSL.stop();      //ADDED TEST
              }

              if (err_send) {
                powerOffComPort();
              }
              //              modem.gprsDisconnect();  //ADDED TEST
              //              myDelayMs(500);         //ADDED TEST

              DEBUG( "COM: STOP CLIENT");
            }
            else
            {
              vTaskPrioritySet( &Handle_CommTask , (tskIDLE_PRIORITY + 2));//AÑADIDO 15/06
              SERIAL.println ("COM: Fail connecting to server");
              // powerOffComPort();//powerOffComm(); 02/06 //QUITADO 15/06
              powerOffComm(); //AÑADIDO 15/06
            }
          }
          else
          {
            SERIAL.println("COM: No data recibe");
          }
        }
        else
        {
          //fallo gordo
          SERIAL.println("COM: Fail exist data queue");

#ifdef DEBUG_LOGEVENTS
          b_infoLog = B10010100; //Error cola CommQueque
          sendToEventsLogQueue(b_infoLog);
#endif
        }

        cont_retry = 0;

        //          }
        //          else
        //          {
        //            DEBUG("Fail recive GPS");
        //          }
        //        }

        if (((uxBits & BIT_9_sendFile) != 0) && ((uxBits & BIT_11_iddle) != 0)) //(exe == false)
        {
          myDelayMs(100);
          //         while(true);


          //          int errlen = sendFile(l_host, l_path);
          int errlen = sendFile();//sendFile(String(AWS_HOST), String(AWS_PATH));
          //SERIAL.println("TEST A");

          if (errlen != 0)
          {
            // tryToSendFile++;
            clearQandReset(errlen, tryToSendFile);
            DEBUG("COM: CLEAR AND RESET FILE SEND STATUS");
            //SERIAL.println("TEST B");
          }
        }
        //          SERIAL.println("DISC0");
        //          modem.gprsDisconnect();

      }
      else {
        powerOffComm();
        SERIAL.println("THE COMMS ARE OF");//TESTEO
      }

      //      else
      //      {
      //        powerOffComm();
      //      }
      DEBUG("CLear BIT_2_SendData"); //TESTEO
      xEventGroupClearBits( myEvents,  BIT_2_SendData );
      DEBUG("CLear BIT_20_SendUbi"); //TESTEO
      xEventGroupClearBits( myEvents,  BIT_20_SendUbi );
    }


    if ((uxBits & BIT_11_iddle) != 0)
    {
      SERIAL.println("POWEROFF 0");
      modem.gprsDisconnect();
      powerOffComm();
    }


    xEventGroupWaitBits(
      myEvents,
      BIT_2_SendData | BIT_7_updateTime ,
      false,
      false,
      portMAX_DELAY ) ;
    DEBUG("COM: WAKE TO SEND");



    /*///////////////////////////////////////////////////////////////////////////////////////////////////ADDED////////////////////////////////////////////////////////////
      //        if (((uxBits & BIT_22_deleteConfigAWS) != 0 ))  //=)
      //    {
      //      unsigned long mytimerwait = 0;
      //      String line = "";
      //      bool f_deleteConfig = false;
      //
      //      DEBUG("DELETING CONFIG FILE (AWS)...");
      //      while ((!(serverConnect(localConfig.BUCKET, true, 443)) && (cont_retry < localConfig.RETRY_CONNECTION)))
      //      {
      //        cont_retry++;
      //        DEBUG ("Retrying to connect:" + String(cont_retry));
      //      }
      //      if (cont_retry < localConfig.RETRY_CONNECTION)
      //      {
      //        String response = "";
      //
      //        Serial.println(buildHTTP(2,""));
      //        clientSSL.println(buildHTTP(2,""));
      //        clientSSL.println();
      //
      //        //        Serial.print(buildHTTP(2, "%2FConfig%2FConfigAWS.txt"));
      //        //        clientSSL.print(buildHTTP(2, "%2FConfig%2FConfigAWS.txt"));
      //        //
      //        DEBUG ( "WAIT RESPONSE");
      //
      //        mytimerwait = millis();
      //
      //        while (( millis() - mytimerwait < 30000))
      //        {
      //          if ( clientSSL.available())
      //          {
      //            line = clientSSL.readStringUntil('\n');
      //            line.trim();
      //            Serial.println(">>> RECIBIDO:" + String(line));
      //            line.toLowerCase();
      //
      //            if ( line.startsWith("http/1.1"))
      //            {
      //              response = line.substring(line.lastIndexOf('.') + 3);
      //              DEBUG("RESPONSE: " + String(response));
      //
      //              if (response.endsWith(" ok"))
      //              {
      //                response.remove(response.length() - 3);
      //                DEBUG ("RESPONSE HTTP: " + response);
      //              }
      //            }
      //            else
      //              Serial.println("Error buscando HTTP/1.1");
      //          }
      //        }
      //
      //        if (response.toInt() != 200) {
      //          Serial.println("ERROR EN LA PETICION HTTP");
      //          clientSSL.stop();
      //          break;
      //        }
      //        else {
      //          Serial.println("PETICION HTTP CORRECTA (DELETE CONFIG AWS), comprobando eliminacion...");
      //
      //          //          Serial.print(buildHTTP(2, "%2FConfig%2FConfigAWS.txt"));
      //          //          clientSSL.println(buildHTTP(2, "%2FConfig%2FConfigAWS.txt"));
      //          //          clientSSL.println();
      //          Serial.println(buildHTTP(2,""));
      //          clientSSL.println(buildHTTP(2,""));
      //          clientSSL.println();
      //
      //
      //          DEBUG ( "WAIT RESPONSE");
      //
      //          mytimerwait = millis();
      //
      //          while (( millis() - mytimerwait < 30000))
      //          {
      //            if ( clientSSL.available())
      //            {
      //              line = clientSSL.readStringUntil('\n');
      //              line.trim();
      //              Serial.println(line);    // Uncomment this to show response header
      //              line.toLowerCase();
      //
      //              if ( line.startsWith("http/1.1"))
      //              {
      //                response = line.substring(line.lastIndexOf('.') + 3);
      //                DEBUG("RESPONSE: " + String(response));
      //
      //                if (response.endsWith(" ok"))
      //                {
      //                  response.remove(response.length() - 3);
      //                  DEBUG ("RESPONSE HTTP: " + response);
      //                }
      //              }
      //              else if (line.startsWith("<error><code>nosuchkey"))
      //                f_deleteConfig = true;
      //              else if ((line.startsWith("content-type:")) || (line.endsWith("text/plain"))) {
      //                if (line.endsWith("text/plain")) {
      //                  f_deleteConfig = false;
      //                  Serial.println(" ERROR NO SE HA ELIMINADO EL ARCHIVO ");
      //                  Serial.println("Resto de respuesta:");
      //                  while (clientSSL.available()) {
      //                    Serial.println(clientSSL.read());
      //                  }
      //                  clientSSL.stop();
      //                }
      //                else
      //                {
      //                  Serial.println("SE HA ELIMINADO EL ARCHIVO DE CONFIGURACION DE AWS");
      //                  f_deleteConfig = true;
      //
      //                  Serial.println("Resto de respuesta:");
      //                  while (clientSSL.available()) {
      //                    Serial.println(clientSSL.read());
      //                  }
      //                  clientSSL.stop();
      //
      //                  xEventGroupClearBits( myEvents, BIT_22_deleteConfigAWS);
      //                  Serial.println("BIT_22_deleteConfigAWS Set to 0");
      //
      //
      //                  xEventGroupSetBits( myEvents, BIT_20_reset);
      //                  Serial.println("BIT_20_reset Set to 1");
      //                }
      //              }
      //            }
      //          }
      //        }
      //      }
      //    }
      //
      //
      //
      //    if (((uxBits & BIT_16_checkConfig) != 0 ) && ((uxBits & BIT_21_readyConfig0) != 0 ))  //=)
      //    {
      //      DEBUG(" CONNECTING TO HOST AWS...");
      //
      //      while ((!(serverConnect(localConfig.BUCKET, true, 443)) && (cont_retry < localConfig.RETRY_CONNECTION)))
      //      {
      //        cont_retry++;
      //        DEBUG ("Retrying to connect:" + String(cont_retry));
      //      }
      //      if (cont_retry < localConfig.RETRY_CONNECTION)
      //      {
      //        String response = "";
      //        String response_status = "-1";
      //        String response_header = "";
      //        String local_body = "";
      //        String value = "";
      //        bool f_newConfig = false;
      //
      //        unsigned long mytimerwait;
      //        Serial.println(buildGet());
      //        clientSSL.println(buildGet());
      //        clientSSL.println();
      //
      //        DEBUG ( "WAIT RESPONSE");
      //
      //        mytimerwait = millis();
      //
      //
      //        while (( millis() - mytimerwait < 30000))
      //        {
      //          if ( clientSSL.available())
      //          {
      //            String line = clientSSL.readStringUntil('\n');
      //            line.trim();
      //            Serial.println(line);    // Uncomment this to show response header
      //            line.toLowerCase();
      //
      //            if ( line.startsWith("http/1.1"))
      //            {
      //              response = line.substring(line.lastIndexOf('.') + 3);
      //              //Serial.println("RESPONSE: " + String(response));
      //
      //              if (response.endsWith(" ok"))
      //              {
      //                response.remove(response.length() - 3);
      //                //  DEBUG ("RESPONSE HTTP: " + response);
      //              }
      //            }
      //            else if (line.startsWith("content-length:"))
      //              contentLength = line.substring(line.lastIndexOf(':') + 2).toInt(); //!!! añadido 1 para posterior lectura correcta en taskfile
      //            else if (line.startsWith("<error><code>nosuchkey"))
      //              f_newConfig = false;
      //            else if ((line.startsWith("content-type:")) || (line.endsWith("text/plain"))) {
      //              if (line.endsWith("text/plain")) {
      //                f_newConfig = true;
      //                Serial.println("--ARCHIVO DE CONFIGURACION ENCONTRADO EN AWS--");
      //                Serial.println("newConfig = " + String(f_newConfig));
      //              }
      //              else
      //              {
      //                Serial.println("--NO HAY NUEVO ARCHIVO DE CONFIGURACION--");
      //                f_newConfig = false;
      //              }
      //            }
      //            else if (line.startsWith("x-amzn-trace-id:")) {
      //              clientSSL.readStringUntil('\n'); //elimina el salto de linea antes del body
      //              break; //hasta aquí llega la cabecera
      //            }
      //          }
      //        }
      //        Serial.println("newConfig = " + String(f_newConfig));
      //        if (f_newConfig == true) {
      //          xEventGroupSetBits( myEvents,  BIT_17_newConfig  );
      //          Serial.println("BIT_17_newConfig Set to 1");
      //          xEventGroupClearBits( myEvents,  BIT_16_checkConfig);
      //          Serial.println("BIT_16_checkConfig Set to 0");
      //        }
      //        else {
      //          clientSSL.stop();
      //          Serial.println("NO HAY NUEVA CONFIGURACION f_newConfig=false");
      //          xEventGroupClearBits( myEvents,  BIT_16_checkConfig);
      //          Serial.println("BIT_16_checkConfig Set to 0");
      //          xEventGroupClearBits( configEvents, BIT_0_);
      //          Serial.println("BIT_0_ Set to 0");
      //          break;
      //        }
      //
      //        if (response.toInt() != 200) {
      //          Serial.println("ERROR EN LA PETICION HTTP");
      //          clientSSL.stop();
      //          break;
      //        }
      //      }
      //    }
      //
      //    if ((uxBits & BIT_19_saveConfigAWS) != 0 )
      //    {
      //      Serial.println("COLEANDO");
      //      uint8_t b; //utilizado para almacenar Byte de cola
      //      uint8_t leng = 0; //controlador de String leido del stream (restante)
      //      uint16_t i = 0;   //controlador de String leido del stream (indice)
      //
      //
      //
      //      UBaseType_t  freeSpace = 0;
      //
      //      switch (stateQueue)
      //      {
      //        case 0:
      //          Serial.println("Tamaño de archivo:" + String(contentLength));
      //          //contentLength es un int y la cola acepta máximo tamaño de paquete 1Byte, por lo que:
      //
      //          if ((uxQueueSpacesAvailable( xConfigQueue ) != 0 ))
      //          {
      //            b = (contentLength & 0x000000FF);
      //            xQueueSend( xConfigQueue, &b, 0 );
      //            b = (contentLength & 0x0000FF00) >> 8;
      //            xQueueSend( xConfigQueue, &b, 0 );
      //            b = (contentLength & 0x00FF0000) >> 16;
      //            xQueueSend( xConfigQueue, &b, 0 );
      //            b = (contentLength & 0xFF000000) >> 24;
      //            xQueueSend( xConfigQueue, &b, 0 );
      //            stateQueue = 1;
      //          }
      //          break;
      //
      //        case 1:
      //          Serial.println("PASANDO ARCHIVO");
      //          freeSpace = uxQueueSpacesAvailable( xConfigQueue );
      //
      //          while (freeSpace != 0)
      //          {
      //            if (aux_send == contentLength) {
      //              Serial.println("traspaso entre colas terminado");
      //              stateQueue = 2;
      //              break;
      //            }
      //
      //            if (clientSSL.available()) //1. LEO UNA LINEA
      //            {
      //              if (leng == 0) {
      //                aux_config = clientSSL.readStringUntil('\n');
      //                aux_config += '\n';
      //                leng = aux_config.length();
      //                i = 0;
      //                aux_read += leng;
      //                //   Serial.println(">>>" + String(aux_config) + "   | leng:" + String(leng));
      //              }
      //            }
      //            else if (!clientSSL.available())
      //            {
      //              Serial.println( "CLIENTE CERRADO");
      //              clientSSL.stop();
      //            }
      //
      //            while (leng != 0)//SI NO HE TERMINADO DE ENVIAR LA LINEA
      //            {
      //              if (freeSpace != 0)// Si tengo espacio en la cola
      //              {
      //                // Serial.print("<<< leng: " + String(leng));
      //
      //                b = aux_config[i];
      //
      //                xQueueSend( xConfigQueue, &b, 0 );
      //
      //                leng--;      //disminuimos tamaño restante del String leido del stream
      //                freeSpace--; //disminuimos tamaño libre en la cola
      //                i++;         //aumentamos el char a leer del String obtenido del stream
      //                aux_send++;
      //              }
      //              else
      //              {
      //                //   Serial.println("Esperando que la cola se libere, faltan: " + String(leng) + "       | Se han enviado: " + String(aux_send));
      //                freeSpace =  uxQueueSpacesAvailable( xConfigQueue );
      //              }
      //            }
      //          }
      //
      //          break;
      //
      //        case 2:
      //          {
      //            Serial.println("TRANSMISION FINALIZADA, suspendiendo hilo hasta fin de tarea");
      //            Serial.println("total leido HTTP: " + String(aux_read) + "       | total enviado a cola: " + String(aux_send));
      //            vTaskSuspend(NULL);
      //          }
      //
      //      }
      //
      //
      //      //      if ( clientSSL.available())
      //      //        aux_config = clientSSL.readStringUntil('\n');
      //      //
      //      //      Serial.println("ENVIANDO DATOS A COLA: " + String(aux_config));
      //      //      xQueueSend( xConfigQueue, ( void * ) &aux_config, ( TickType_t ) 0 );
      //    }
    *////////////////////////////////////////////////////////////////////////////////////////////////////ENDADDED//////////////////////////////////////////

  }
  SERIAL.println("COM: Deleting communication");

  xEventGroupClearBits( eventTaskState,  BIT_4_taskComm ); //ADDED WORKING

  vTaskDelete( NULL );
}




int clearQandReset(int len, int &tryToSendFile)
{
#ifdef DEBUG_LOGEVENTS //ADDED
  static byte b_infoLog = NULL;//newstatic
#endif

  uint8_t c ;
  //  int sizeFile =uxQueueSpacesAvailable( xTxBuffer );
  int sizeFile = len;
  //  if (( == 0 ));
  unsigned long timeOut = millis();
  //
  //  while (((sizeFile > 0) && ((millis() - timeOut) < FILE_TIME_SEND_TIMEOUT )))
  //  {
  //    sizeFile--;
  //  }
  DEBUG("COM: CLEARING DATA FROM BUFFER: " + String (sizeFile));
  while (((sizeFile > 0) && ((millis() - timeOut) < FILE_TIME_SEND_TIMEOUT )))// && (! flgFailSend))) !!! FALTA CONTROLAR QUE PASA SI SE CUMPLE EL TIEMPO
  {
    if (xQueueReceive( xTxBuffer, &c, ( TickType_t ) 500 ) == pdPASS)
    {
      sizeFile--;
    }
  }
  tryToSendFile++;
  if ( tryToSendFile >= RETRY_SEND_FILE)
  {
    tryToSendFile = 0;
    xEventGroupSetBits( myEvents,  BIT_16_sendFileOK);
    //ADDEDLOG
#ifdef DEBUG_LOGEVENTS
    b_infoLog = B10000100; //FAIL SENDING FILE AWS
    sendToEventsLogQueue(b_infoLog);
#endif
    //ENDADDED
  }
  xEventGroupSetBits( myEvents,  BIT_13_doneFileSend);
  DEBUG ( "COM: SET BIT 13");
  DEBUG("COM: CLEARING DATA FROM BUFFER: " + String (sizeFile));
  myDelayMs(5000);
  DEBUG("COM: Space available in: " + String (uxQueueSpacesAvailable( xTxBuffer )));
  if ((uxQueueSpacesAvailable( xTxBuffer ) == 0 ))return 0;
  else return -1;
}

int takeGPS(qPosition* localPosition)
{
#ifdef DEBUG_LOGEVENTS
  byte b_infoLog = NULL;
#endif

  static String val = "";
  val = "";
  static int idx = 0;
  idx = 0;
  static int numGps = 0;
  numGps = 0;
  float localHDOP = -1.0;

  //float latD = 0.0;
  //float lonD = 0.0;
  //float latM = 0.0;
  //float lonM = 0.0;
  float lat = 1000.0;
  float lon = 1000.0;
  myDelayMs(10000);

  localPosition->latitude = 0.0;
  localPosition->longitude = 0.0;
  localPosition->fix = 0;
  //localPosition->HDOP = -1.0;

#ifndef REALGPS
  localPosition->latitude = 28.0131710;
  localPosition->longitude = -15.429534;
  localPosition->fix = 1;
  SERIAL.println(modem.getGPS_NMEA());

  return 0;
#else

  String GLL = modem.getGPS_NMEA(); //newstatic
  SERIAL.println(GLL);


  for  (int i = 0 ; i < GLL.length(); i++)
  {
    if (GLL[i] == ',')
    {
      idx ++;
      DEBUG("idx: " + String(idx) + " " + String(val));
      if ( val.length() > 0)
      {
        switch (idx)
        {
          case 4:
            /*latD = val.substring(0, 2).toFloat(); //deberíamos buscar el punto
              latM = val.substring(2).toFloat();
              lat  = latD + (latM / 60.0);*/

            lat = val.substring(2).toFloat();
            lat = lat / 60.0;
            lat = lat + val.substring(0, 2).toFloat();

            localPosition->latitude = lat;
            //            localPosition->latitude = val.toFloat();
            SERIAL.print("LAT: ");
            SERIAL.println(lat);
            break;
          case 5:
            if ( val == "S")localPosition->latitude = 0.0 - localPosition->latitude;
            break;
          case 6:
            /*lonD = val.substring(0, 3).toFloat();
              lonM = val.substring(3).toFloat();
              lon  = lonD + (lonM / 60.0);*/

            lon = val.substring(3).toFloat();
            lon = lon / 60.0;
            lon = lon + val.substring(0, 3).toFloat();

            localPosition->longitude = lon;
            //            localPosition->longitude = val.toFloat();
            SERIAL.print("LON: ");
            SERIAL.println(lon);
            break;
          case 7:
            if ( val == "W")localPosition->longitude = 0.0 - localPosition->longitude;
            break;
          case 8:
            localPosition->fix = (uint8_t)val.toInt();
            //SERIAL.print("FIX: ");
            //SERIAL.println(val);

            //ADDEDLOG
#ifdef DEBUG_LOGEVENTS
            if (localPosition->fix == 1) {
              b_infoLog = B00000010; //lectura GPS correcta
              sendToEventsLogQueue(b_infoLog);
            }
            else {
              b_infoLog = B10000010; //error en lectura GPS
              sendToEventsLogQueue(b_infoLog);
            }
#endif
            //ENDADDED

            break;
          case 9:
            numGps = (uint8_t)val.toFloat();
            break;
          case 10:
            localHDOP = val.toFloat();//antes (uint8-t)val.toFloat()
            SERIAL.print("HDOP: ");
            SERIAL.println(localHDOP);
            //localPosition->HDOP = localHDOP;
            break;
        }
      }
      val = "";
    }
    else
    {
      val += GLL[i];
    }
  }
  val = "";
  DEBUG(numGps);
  if ( lat == 1000) return -1;
  if (lon == 1000) return -1;
  if ( numGps < 4)return -1;
  if ( localHDOP > 5.5 || localHDOP == 0) return -1; //AÑADIDO 3/11/2023
  //  localPosition->latitude=localPosition->latitude/100;
  //  localPosition->longitude=localPosition->longitude/100;
  SERIAL.println("GPS OK");
  return 0;
#endif
}



int sendFile()//sendFile(String _host, String _path)
{
  ///////////////////////////
  EventBits_t uxBits;
  //  simErr++;
  ////////////////////////////
#ifdef DEBUG_LOGEVENTS
  byte b_infoLog = NULL; //ADDEDLOG //newstatic
#endif

  //static String localHost = _host; //newstatic      PUEDE SER CONST -> NO ES NECESARIO ESTE STRING
  //static String localPath = _path;//newstatic        PUEDE SER CONST -> NO ES NECESARIO ESTE STRING
  //buffer
  static uint8_t c[SIZE_BUFFER];// //newstatic
  //Read Name File
  static String fileName = ""; //newstatic  FILENAME SE UTILIZA COMO fileName y como el antiguo response para ahorrar espacio de variables
  fileName = "";
  static int sizeFile = 0; ////newstatic
  sizeFile = 0;
  //static unsigned long fileTimeSend=0;//newstatic
  //fileTimeSend = millis();
  static unsigned long timeOut = 0; //newstatic
  timeOut = 0;
  static int cont_retry = 0; //newstatic
  cont_retry = 0;
  DEBUG("COM: SEND FILE TO AWS");
  ////////////////READ FILE NAME///////////////<--------------------------------
  memset(c, 0, SIZE_BUFFER);
  uint8_t *idx = &c[0];////newstatic
  int len = 0;
  do
  {
    if (xQueueReceive( xTxBuffer, idx + len, ( TickType_t ) 500 ))
    {
      len++;
    }
  } while ( c[len - 1] != ',');
  c[len - 1] = 0;
  SERIAL.print("COM: FileName: ");
  fileName = String((char*)c);
  SERIAL.println(fileName);
  memset(c, 0, len);
  len = 0;
  ////////////////READ FILE SIZE///////////////<--------------------------------
  xQueueReceive( xTxBuffer, idx, ( TickType_t ) 500 );
  sizeFile = c[0];
  xQueueReceive( xTxBuffer, idx, ( TickType_t ) 500 );
  sizeFile |= c[0] << 8;
  xQueueReceive( xTxBuffer, idx, ( TickType_t ) 500 );
  sizeFile |= c[0] << 16;
  xQueueReceive( xTxBuffer, idx, ( TickType_t ) 500 );
  sizeFile |= c[0] << 24;
  SERIAL.print("FileSize: ");
  SERIAL.println(sizeFile);

  if ( sizeFile > 500000)
  {
    SERIAL.println("COM ERROR: ABORT CONN DATA SIZE ERROR");
    return -1;
  }
  taskYIELD();
  SERIAL.println("ISNETWORKCON SENDFILE");
  if (!modem.isNetworkConnected())  //ADDED TEST
  {
    powerOnComm();
  }

  //  if (modem.isNetworkConnected())  QUITADO 14/06 NUEVA LIBRERIA R5
  //  {
  ////////////////CONNECT TO SERVER///////////////<--------------------------------
  myDelayMs(1000);
  SERIAL.println("COM: CONNECTING TO AWS");
  while ((!(serverConnect( AWS_HOST , USE_SSL, 443)) && (cont_retry < RETRY_CONNECTION)))//!!! modificar cuando se añadan parametros
  {
    cont_retry++;
    myDelayMs(1000);

    SERIAL.print("COM: Retrying to connect:");
    SERIAL.println(String(cont_retry));
  }
  if (cont_retry > RETRY_CONNECTION)
  {
    SERIAL.println("COM: ERROR: CONN TO SERVER");
    return sizeFile;
  }

  SERIAL.println(buildPut(sizeFile, fileName));
  taskYIELD();
  ////////////////SEND HEADER///////////////<--------------------------------
  if ( !clientSSL.println(buildPut(sizeFile, fileName)))
  {
    SERIAL.println("COM: ERROR: HEADER");
    clientSSL.stop();
    //      SERIAL.println("DISC1");
    //      modem.gprsDisconnect();
    fileName = "";
    return sizeFile;
  }
  fileName = "";
  ////////////////SEND BODY///////////////<--------------------------------
  timeOut = millis();
  while (((sizeFile > 0) && ((millis() - timeOut) < FILE_TIME_SEND_TIMEOUT )))// && (! flgFailSend))) !!! FALTA CONTROLAR QUE PASA SI SE CUMPLE EL TIEMPO
  {
    if (xQueueReceive( xTxBuffer, idx + len, ( TickType_t ) 500 ) == pdPASS)
    {
      sizeFile--;
      len++;
      if (( len == SIZE_BUFFER - 1) || (sizeFile == 0))
      {
        taskYIELD();
        //////////////////////////////////////////////////////////////////////////Simulando error en transmisión archivo ( por parte hilo com)
        //        if(simErr ==0){
        //          while(true)myDelayMs(1000);
        //        }
        //
        if (!clientSSL.write(c, len))
        {
          SERIAL.println("COM: ERROR: BODY ");
          clientSSL.stop();
          //            SERIAL.println("DISC2");
          //            modem.gprsDisconnect();  //ADDED TEST
          return sizeFile;
        }
        myDelayMs(1);
        //        if (simErr == 4)
        //      {
        //        DEBUG("FALLO SIMULADO ARCHIVO PETO");
        ////        simErr = 0;
        //        clientSSL.stop();
        //        return sizeFile;
        //      }
        memset(c, 0, SIZE_BUFFER);
        len = 0;
      }
    }
  }
  clientSSL.println();
  if (sizeFile != 0)
  {
    clientSSL.stop();
    //      SERIAL.println("DISC3");
    //      modem.gprsDisconnect();  //ADDED TEST

    return sizeFile;
  }

  SERIAL.println("COM: sizeFile left: " + String(sizeFile));
  ////////////////WAIT RESPONSE///////////////<--------------------------------
  //DEBUG ( "COM: FILE SENDED IN: " + String(millis() - fileTimeSend) + " ms");
  SERIAL.println ( "COM: WAIT RESPONSE");

  timeOut = xTaskGetTickCount(); //ANTES millis() 23/11/2023
  //static String response = ""; //newstatic
  fileName = ""; //AHORA USAMOS FILENAME PARA NO CREAR OTRO STRING response = "";
  //SERIAL.println("TEST A2");
  while (( (xTaskGetTickCount() - timeOut) < 60000))//ANTES millis() 23/11/2023
  {
    if ( clientSSL.available())
    {
      //SERIAL.println("TEST C");
      char rx = clientSSL.read();
      SERIAL.write(rx);
      fileName += rx;//response += rx;
      if ( fileName.endsWith("HTTP/1.1 "))//response.endsWith("HTTP/1.1 "))
      {
        fileName = "";//response = "";
      }
      else if (fileName.endsWith(" OK"))//response.endsWith(" OK"))
      {
        fileName.remove(fileName.length() - 3);//response.remove(response.length() - 3);
        if (fileName.toInt() == 200)// response.toInt() == 200)
        {
          //          tryToSendFile = 0;
          xEventGroupSetBits( myEvents,  BIT_16_sendFileOK);


          //ADDEDLOG
#ifdef DEBUG_LOGEVENTS
          b_infoLog = B00000100; //FILE SENT TO AWS
          sendToEventsLogQueue(b_infoLog);
#endif
          //ENDADDED
        }
        SERIAL.print("COM: FILE RESPONSE HTTP: ");
        SERIAL.println(fileName);//response);
        break;
      }
      if (fileName.length() == 64)//response.length() == 64)
      {
        fileName = "";//response = "";
      }
    }
    //else {
    //SERIAL.println("TEST D");
    //}
  }
  fileName = ""; //response = "";
  clientSSL.stop();
  //    SERIAL.println("DISC4");
  //    modem.gprsDisconnect(); //ADDED TEST
  SERIAL.println ("COM: FILE SENT");

  ////////////////BIT MAGNAMENT///////////////<--------------------------------
  //  if (tryToSendFile > 3 )
  //  {
  //    //
  //    tryToSendFile = 0;
  //    xEventGroupSetBits( myEvents,  BIT_16_sendFileOK);
  //    DEBUG ("FAIL SEND FILE, DELETING FORM FILE TO SEND ");

  //  //ADDEDLOG
  //  b_infoLog = B10000100; //FAIL SENDING FILE AWS
  //  sendToEventsLogQueue(b_infoLog);
  //  //ENDADDED
  //  }

  xEventGroupSetBits( myEvents,  BIT_13_doneFileSend);
  DEBUG ( "COM: SET BIT 13");

  return 0;
  //  }
  //  SERIAL.println("POWER OFF3");
  //  powerOffComPort();//powerOffComm(); 02/06
  //  return 1;//ADDED TEST

  /////////////////////////////////////////////////////////////////////////////////////////////////

}
