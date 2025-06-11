//FUNCIONES AUXILIARES RELACIONADAS CON EL FREERTOS:
//**************************************************************************
// Can use these function for RTOS delays
// Takes into account processor speed
// Use these instead of delay(...) in rtos tasks
//**************************************************************************

/*
 * portTICK_PERIOD_MS = 1    ->  tiempo en milisegundos de un tick
 * portTICK_PERIOD_US = 1000 ->  tiempo que dura un tick en microsegundos,
 */
void myDelayUs(int us)
{
  vTaskDelay( us / portTICK_PERIOD_US );
}

void myDelayMs(int ms)
{
  vTaskDelay( (ms * 1000) / portTICK_PERIOD_US );
}

void myDelayMsUntil(TickType_t *previousWakeTime, int ms)
{
  vTaskDelayUntil( previousWakeTime, (ms * 1000) / portTICK_PERIOD_US );
}

void myDelayUsUntil(TickType_t *previousWakeTime, int us) //NO PUEDE SER MENOR A 1000 microsegundos (1 milisegundos) duración de un tick
{
  vTaskDelayUntil( previousWakeTime,(int) us / portTICK_PERIOD_US );
}

//____________________________________________________RS485______________________________________________
typedef enum
{
  CMD_STOP_CONFIG,     //0
  CMD_SG_CALIB,        //1
  ADV_ERROR,           //2
} cmd_RS485;

cmd_RS485 cmd_decoder (const char* buffer)
{
  Serial.print("\n\n>>Command received: ");
  Serial.println(buffer);
  if ( buffer[0] != '$')return ADV_ERROR;
  if (strcmp(buffer, "$sgcal\n") == 0)return CMD_SG_CALIB;
  if (strcmp(buffer, "$stopcfg\n") == 0)return CMD_STOP_CONFIG;

  return ADV_ERROR;
}



void commandResponseRS485(String mesage) { //esta función usa solo delays y no mydelays porque se usa unicamente en modo configuración , es decir dentro de setup() y no en los hilos
  digitalWrite(RS485_FLOW, HIGH);
  delay(5);

  SERIAL1.write('[');
  SERIAL1.print(mesage);
  SERIAL1.write(']');

  delay(5); //necesario para dar tiempo a que haga el envío
  digitalWrite(RS485_FLOW, LOW);
  delay(5);
}
int gaugeCalibration(bool remoteConfig = false);
/*
   devuelve:
   si rxRS485 = CMD_STOP_CONFIG para parar la configuración y pasar a la medida
   si rxRS485 = CMD_SG_CALIB prepara la calibración de la galga
*/
uint8_t check_cmd(const char*  rxRS485, int &response) {
  cmd_RS485 cmd = cmd_decoder(rxRS485);
  int res = 0;
  switch (cmd)
  {
    case CMD_STOP_CONFIG: //COMANDO DE CALIBRACION DE GALGA
      Serial.println("[RS485] End of configuration");
      break;

    case CMD_SG_CALIB: //COMANDO DE CALIBRACION DE GALGA
      delay(200);
      commandResponseRS485("OK"); //informa que ha recivido el comando
      Serial.println("[RS485] Gauge Calibration");
      response = gaugeCalibration(true);
      break;

    default:
      Serial.print("[RS485] Command no recognized: ");
      Serial.print(rxRS485);
      break;
  }
  return cmd;
}



//_________________________________________________GALGAS_______________________________________

//funcion que setea el scale para la galga pasada mediante myScale y paramStrain
// return 0 si todo OK
// return -1 si error en setScale
int setScale(paramST &paramStrain, DFRobot_HX711 myScale, bool remoteConfig = false) {
  SERIAL.println("\nSetting Slope gauge:" + String(paramStrain.ID));
  SERIAL.println("Put the weigth on the measuring body to set Slope, dont retire it until prompted. After that enter the weight value (format: 123.45)");
  //long scale = 0;
  String rxData = "";
  unsigned long  timeNow = millis();
  char c = 0;
  float weight = 0.0;
  paramStrain.valid = false;

  if (remoteConfig) {
    delay(200);
    commandResponseRS485("Put the weigth to set Slope ID:" + String(paramStrain.ID)); //, dont retire it until prompted. After that enter the weight value (format: 123.45)");
  }

  while (true)//((millis() - timeNow ) < TIMEOUT_CALIBRATION) //*10
  {
    if (!paramStrain.valid) { //si no se ha introducido valor
      if (remoteConfig) {
        //timeNow = millis(); //permite que el tiempo no llegue al timeout

        c = 0;
        rxData = "";
        while (( c != '\n')) //&& ((millis() - timeNow ) < TIMEOUT_CALIBRATION)) //TOMAMOS 1 DATO (frase) PROVENIENTE DEL DATALOGER - RS485
        {
          if (SERIAL1.available() > 0)
          {
            c = SERIAL1.read(); //espera leer valor de peso
            if ((c != '\n'))// && (c != '\r') && (c != ' '))
            {
              rxData += c;
            }
          }
        }
        c = 0;
      }
      else {
        if ( SERIAL.available()) //si se escribe el valor del scale (pendiente) se le asigna a la estructura
        {
          rxData = SERIAL.readStringUntil('\n');
        }
      }

      if (rxData.length() != 0 ) {
        weight = atof(rxData.c_str());

        if (weight != 0.0) { //ATENCION ESTO IMPIDE QUE SE ENVIE 0.0
          delay(5000); //este delay es para esperar a que el cuerpo se deforme
          //float scale =  ((myScale.getValue() - paramStrain.offset) / weight);
          myScale.setOffset(0); //quitamos offset para tomar medida en crudo
          long rawMeasure = myScale.getValue();
          myScale.setOffset(paramStrain.offset); //asignamos offset para calcular pendiente
          paramStrain.scale =  ((rawMeasure - paramStrain.offset) / weight);
          paramStrain.valid = true;
        }
        else { //en el caso de que lo que se envíe no sea un numero
          SERIAL.print("Incorrect Format");
          delay(200);
          commandResponseRS485("Incorrect Format");
        }
      }
    }

    else { // if (paramStrain.valid) { //si se ha introducido un valor de peso
      myScale.setCalibration(paramStrain.scale);
      delay(100);

      SERIAL.print("raw measure " + String(paramStrain.ID) + ": "); SERIAL.println(myScale.getValue());
      SERIAL.print("weight measure " + String(paramStrain.ID) + ": " + String( myScale.readWeight()));
      SERIAL.println(String(weight) + "   |   Y: para confirmar / N: para modificar");

      if (!remoteConfig) { //SI NO ESTA EN CONFIGURACION REMOTA LEE Y/N
        //timeNow = millis();
        while (true) { //((millis() - timeNow ) < TIMEOUT_CALIBRATION) {
          if (SERIAL.available()) {
            rxData = SERIAL.readStringUntil('\n');
            break;//sale del while interno
          }
        }
      }
      else { //SI ESTA EN CONFIGURACION REMOTA DATALOGGER LEE Y/N
        delay(200);
        commandResponseRS485("raw measure " + String(paramStrain.ID) + ": " + String( myScale.getValue()));
        delay(200);
        commandResponseRS485("weight measure " + String(paramStrain.ID) + ": " + String( myScale.readWeight()));
        delay(200);
        commandResponseRS485(String(weight) + "   |   Y: to confirm / N: to modif");

        c = 0;
        rxData = "";
        while (true)//(( c != '\n') && ((millis() - timeNow ) < TIMEOUT_CALIBRATION)) //TOMAMOS 1 DATO (frase) PROVENIENTE DEL DATALOGER - RS485
        {
          if (SERIAL1.available() > 0)
          {
            c = SERIAL1.read(); //espera leer Y para validar y salir o N para volver a configurar
            if ((c != '\n'))// && (c != '\r') && (c != ' '))
            {
              rxData += c;
            }
            else { //si llega al caracter final
              break; //sale del while interno
            }
          }
        }
      }

      if (rxData.indexOf("Y") != -1) { //este if con su else deberían mejorarse
        paramStrain.valid = true;
        break; //sale del while externo
      }
      else { //if (rxData[0] == 'N') {
        paramStrain.valid = false; //reinicia el valor de valid para que puedas volver a modificarlo

        if (remoteConfig) {
          delay(200);
          commandResponseRS485(String(rxData) + "  |  ReSend weigth to set scale");
        }
        SERIAL.println("ReSet weigth to set scale\n");
      }
    }
    rxData = "";
  }

  if (!paramStrain.valid) { //si no se introdujo un valor, mantiene el valor anterior de scale y retorna -1 , valid = false
    SERIAL.println("CALIBRATION ABORTED, SCALE " + String(paramStrain.ID) + " NOT SET");
    if (remoteConfig) {
      delay(1000);
      commandResponseRS485("CALIBRATION ABORTED, SCALE " + String(paramStrain.ID) + " NOT SET");
    }
    return -1;
  }
  if (remoteConfig) {
    delay(1000);
    commandResponseRS485("END CAL. : " + String(paramStrain.ID));
  }
  return 0;
}


/*FUNCION QUE SE ENCARGA DE OBTENER EL OFFSET Y LA ESCALA PARA CALIBRAR LAS GALGAS
   return  0 si se completa
   return -1 si salta el timeout de calibración o el error de desbordamiento -> calibration process fail (se queda con la calibracion anterior)
*/
//static void threadCalib( void *pvParameters )
int gaugeCalibration(bool remoteConfig) //remoteConfig determina si el comando de calibración ha llegado por RS485
{
  SERIAL.println("\n\t\t<Gauge Calibration>");
  //TickType_t lastWakeTime = xTaskGetTickCount();
  SERIAL.println("Press (Y) to continue or send command");
  SERIAL.println('>');
  //SERIAL.setTimeout(2000); //?  sets the maximum milliseconds to wait for serial data when using serial.readBytesUntil() or serial.readBytes(). It defaults to 1000 milliseconds.

  String rxData = "";
  unsigned long timeNow = millis();
  char c_;

  //while ((millis() - timeNow ) < TIMEOUT_CALIBRATION) //! este timeout puede hacer que la galga se quede sin calibrar imprimendo datos sin sentido, es necesario realizar una segunda comprobación para que no llegue a tomar datos sin estar calibrado
  while (true)
  { //offset default=0 y escala 1  por lotanto daria valores cercanos a 800930401340
    if (remoteConfig) { //si se ha enviado comando de config por RS485
      //rxData = 'Y';
      rxData = "";
      while (c_ != '\n') {
        if (SERIAL1.available() > 0) {
          c_ = SERIAL1.read(); //espera leer comando de config
          rxData += c_;
        }
      }
      delay(2000); //damos 2 segundos
      break;
    }
    else {
      if ( SERIAL.available())
      {
        rxData = SERIAL.readStringUntil('\n');
        break;
      }
    }
    delay(50);
  }

  if (rxData[0] != 'Y') return -1;

  rxData = "";
  SERIAL.println("Seting offset");
  unsigned long offset1 = 0;
  unsigned long offset2 = 0;

  for (int i = 0; i < OFFSET_TIMES; i++) //obtiene la media de offset
  {
    unsigned long aux1 = MyScale1.getValue();
    unsigned long aux2 = MyScale2.getValue();
    SERIAL.print(aux1); SERIAL.print(' '); SERIAL.println(aux2);

    if (remoteConfig) {
      commandResponseRS485(String(i) + " " + String(aux1) + " " + String(aux2));
    }

    aux1 = aux1 / OFFSET_TIMES;
    aux2 = aux2 / OFFSET_TIMES;

    delay(1000/MEASURE_FREQ);

    offset1 += aux1;
    offset2 += aux2;
  }
  if (remoteConfig) {
    delay(200);
    commandResponseRS485("endMeasures");
  }

  paramStrain1.offset = offset1;
  paramStrain2.offset = offset2;

  SERIAL.print("gauge1 offset: ");
  SERIAL.println(offset1);
  SERIAL.print("gauge2 offset: ");
  SERIAL.println(offset2);

  //MyScale1.setOffset(paramStrain1.offset);
  //MyScale2.setOffset(paramStrain2.offset);

  if (remoteConfig) {
    delay(200);
    commandResponseRS485(String(offset1) + "," + String(offset2));
  }

  if (setScale(paramStrain1, MyScale1, remoteConfig) == -1) {
    return -1;
  }
  if (setScale(paramStrain2, MyScale2, remoteConfig) == -1) {
    return -1;
  }

  delay(200);
  commandResponseRS485("CALIBRATION COMPLETED");
  return 0;
}
