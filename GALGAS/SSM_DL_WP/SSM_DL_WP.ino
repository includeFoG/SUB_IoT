#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

//#include <basicMPU6050.h>
#include <MPU6050.h>
#include "Params.h"
#include "htmlCode.h"

//#include "stream_buffer.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include "time.h"
#include "timers.h"
#include "esp_sntp.h" //deprecated, use: esp_sntp.h

#include <ESP32Time.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>     //mDNS para identificar el dispositivo en una red a partir de su nombre y no su IP
#include <Update.h>
#include <WebServer.h>

#include <ErriezDS1339.h>
#include <SparkFunBQ27441.h>

#include "events.h"

#include "esp_log.h" //para ver los logs habilitar core debug level 
#include "nvs.h" //NON VOLATILE STORAGE -> tipo int principalmnente, no float 
#include "nvs_flash.h"


MPU6050 sensorIMU(0x69);

nvs_handle_t handle_nvs_vars;

BLECharacteristic * pTxCharacteristic;
WebServer server(80);
ESP32Time rtc; //RTC interno
ErriezDS1339 rtcEXT; //RTC externo


 enum states { //posibles estados del dispositivo (SOLO USADOS PARA ESTADOS FUERA DE HILOS DE CONFIG BT y WIFI, NO GENERAR ESTADOS PARA ESTOS HILOS)
  IDDLE,        //0
  CONFIG,       //1 en hilo de BT/WIFI
  INTIME,       //2 en hora de sesión
  MEASURING,   //3 midiendo
  SLEEP,        //4
  CHARGING,     //5
  FULLCHARGED   //6
};
RTC_DATA_ATTR states STATE = CONFIG; //estado inicial almacenado en RTC
states PRESTATE = MEASURING; //preestado inicial -> para hacer que salte


#include "taskISRWire.h"
#include "F_aux.h"


//___________________________________________________________________________________________THREADS___________________________________________________________________________________________

#include "taskWiFi.h"
#include "taskDataReading.h"
#include "taskBLE.h"
#include "taskLedStatus.h"

//__________________________________________________________________________________________MAIN CODE__________________________________________________________________________________________

bool checkISRInFirstTime = true; //esta variable permite comprobar el estado del pin de interrupción de carga ISR_connectPowerCable haciendo que solo se produzca 1 vez, la primera que arranca el dispositivo
void setup()
{
  startPinConfig();

  //______________________________________________________________ISR______________________________________________________________________________
  //              OJO LAS INTERRUPCIONES SE DESACTIVAN AL ENTRAR EN DEEPSLEEP
  if (digitalRead(PWR_IN)) {
    attachInterrupt(digitalPinToInterrupt(PWR_IN), ISR_disconnectPowerCable, FALLING); //habilitamos interrupción desconexion de PWR_IN
  }
  else {
    attachInterrupt(digitalPinToInterrupt(PWR_IN), ISR_connectPowerCable, RISING); //habilitamos interrupción conexion de PWR_IN (CARGA DE BATERIA)
  }

  if (checkISRInFirstTime) { //necesario por si se encontraba cargando antes de arrancar para que salte la interrupción
    if (digitalRead(PWR_IN)) {
      ISR_connectPowerCable();
    }
  }
  checkISRInFirstTime = false; //lo ponemos a false para que después del deepsleep no vuelva a entrar en el check
  //_______________________________________________________________________________________________________________________________________________



  //***********************************************FREQS*********************************************
  //cpufreqs[6] = {240, 160, 80, 40, 20, 10};
  setCpuFrequencyMhz(160); // 40,20,10 se bloquea el programa después de setup_RTC_EXT() "Task watchdog got triggered", ver #include "soc/rtc_wdt.h"
  //Serial.updateBaudRate(BAUD_SERIAL);
  //Serial1.updateBaudRate(BAUD_SERIAL);
  checkFreqs();
  //*******************************************************************************************************

  Serial.begin(BAUD_SERIAL);
  Serial1.begin(BAUD_SERIAL, SERIAL_8N1, RXPIN, TXPIN);
  WiFi.setSleep(true); //control modem sleep when only in STA mode [WiFiGeneric.cpp]


  switch (STATE) {
    case CONFIG:
      Serial.println("\n\n***********************************************************");
      Serial.println("\t\t\tProgram start");
      Serial.print(FILENAME);
      Serial.print("\t|\t");
      Serial.println(VERSION);
      Serial.print(DEVICE_NAME);
      Serial.print("\t|\tCONFIG_LOG_DEFAULT_LEVEL:");
      Serial.println(CONFIG_LOG_DEFAULT_LEVEL);
      Serial.println("***********************************************************");
      break;
    case CHARGING: //CREADO PARA QUE EL DEFAULT NO NOS MODIFIQUE EL ESTADO A INTIME Y PARA ACTUAR EN WAKEUP MIENTRAS CARGA
      Serial.println("\n\n******************************");
      Serial.println("        CHARGING         ");
      Serial.println(FILENAME);
      Serial.println(DEVICE_NAME);
      Serial.println("******************************");

      //hacemos un blink con el led RGB rojo
      digitalWrite(RLED, LOW);
      myDelayMs(400);
      digitalWrite(RLED, HIGH);

      if (checkIfWantToSleepCharging()) { //preguntamos al usuario si quiere abortar el sleepmode

        checkStateOfChargeToSleep(true, false); //comprobamos estado de carga y si está cargando volvemos a dormir
        if (STATE == CHARGING) {
          break;
        }
      }
      else { //si quiere abortar el sleep mode pasamos a state config
        Serial.println(" CHANGED TO CONFIG MODE");
        STATE = CONFIG;
        break;
      }
    //en caso de que el estado cambie a FULLCHARGED en checkStateOfChargeToSleep continuará al siguiente case
    case FULLCHARGED: //CREADO PARA QUE EL DEFAULT NO NOS MODIFIQUE EL ESTADO A INTIME
      Serial.println("\n\n******************************");
      Serial.println("        FULL CHARGED         ");
      Serial.println(FILENAME);
      Serial.println(DEVICE_NAME);
      Serial.println("******************************");

      if (checkIfWantToSleepCharging()) { //preguntamos al usuario si quiere abortar el sleepmode
        checkStateOfChargeToSleep(true, true); //comprobamos estado de carga y volvemos a dormir
      }
      else { //si quiere abortar el sleep mode pasamos a state config
        Serial.println(" CHANGED TO CONFIG MODE");
        STATE = CONFIG;
      }

      break;
    default:
       if( sToNextSesion(getNextSesion(rtc.getTimeStruct()))< TIME_WINDOW *60){
          STATE = INTIME;
          Serial.println("\n\n******************************");
          Serial.println("       Time to Sesion         ");
          Serial.println("******************************");
      }
      else{
          Serial.println("\n\n******************************");
          Serial.print("       State:"); Serial.println(STATE);
          Serial.println("******************************");
      }
      break;
  }

  detachInterrupt(digitalPinToInterrupt(PWR_IN));
  //nvs_flash_erase(); //SOLO USAR UNA VEZ CUANDO SE AÑADAN NUEVAS VARIABLES A LA PARTICION
  init_nvs();//inicializa NVS
  read_nvs_16(nvs_key_filename, &nvs_val_filename); //lee valor y lo asigna a nvs_val_filename puede dar error la primera vez hasta que se haga el write

  digitalWrite(PWR_V3_EN, HIGH); //habilita  PULL-UP I2C
  i2cScann();

  //COMPROBAMOS Y AVISAMOS SI LA IMU SE ENCUENTRA CALIBRADA
  read_nvs_16(nvs_key_offset_aX, &nvs_val_offset_aX); //lee valores offset aceleraciones y lo asigna a nvs_val_offset_a
  read_nvs_16(nvs_key_offset_aY, &nvs_val_offset_aY);
  read_nvs_16(nvs_key_offset_aZ, &nvs_val_offset_aZ);

  read_nvs_16(nvs_key_offset_gX, &nvs_val_offset_gX);  //lee valores offset osciloscopio y lo asigna a nvs_val_offset_g
  read_nvs_16(nvs_key_offset_gY, &nvs_val_offset_gY);
  read_nvs_16(nvs_key_offset_gZ, &nvs_val_offset_gZ);

  read_nvs_16(nvs_key_sens_acel_IMU, &nvs_val_sens_acel_IMU);  //lee los valores de sensibilidad almacenados en flash (solo para confirmar que se puede leer)
  read_nvs_16(nvs_key_sens_giro_IMU, &nvs_val_sens_giro_IMU);

  //______________________________________________________________ISR______________________________________________________________________________
  //              OJO LAS INTERRUPCIONES SE DESACTIVAN AL ENTRAR EN DEEPSLEEP
  if (digitalRead(PWR_IN)) {
    attachInterrupt(digitalPinToInterrupt(PWR_IN), ISR_disconnectPowerCable, FALLING); //habilitamos interrupción desconexion de PWR_IN
  }
  else {
    attachInterrupt(digitalPinToInterrupt(PWR_IN), ISR_connectPowerCable, RISING); //habilitamos interrupción conexion de PWR_IN (CARGA DE BATERIA)
  }
  //_______________________________________________________________________________________________________________________________________________


  if (nvs_val_offset_aX == 0) { //para comprobar si está calibrado nos vale con mirar este offset
    Serial.println("\nIMU HAS NOT BEEN PREVIOUSLY CALIBRATED\n");
  }

  if (STATE == CONFIG) { //NECESARIO para que cuando despierte del deepsleep no reconfigure el RTC y pierda segundos
    setup_RTC_EXT(); //configuramos el RTC_EXT
  }

  Serial.println("TIME OF RTC_EXT:");
  struct tm tInicio;
  rtcEXT.read(&tInicio);
  Serial.println(&tInicio, "%A, %B %d %Y %H:%M:%S");

  wakeup_reason(); //comprueba si se ha despertado después de estar en lowpower, si no es así configura RTC_INT a partir de RTC_EXT si RTC_EXT no tiene fecha fija 01/01/2000

  getBatteryStatus(true); //informa del estado del sistema de seguridad de batería y sus parámetros

  digitalWrite(PWR_V3_EN, LOW);//deshabilita  PULL-UP I2C
  Serial.flush();
  check_nvs_index_value(); //comprueba si el valor que se le va a poner al archivo en caso de falla de RTC es correcto

  createEvents();
  createSemaphores();
  myDelayMs(10); //en alguna ocasion ha aceptado el dispositivo antes de terminar de hacer algunas cosas

  xTaskCreatePinnedToCore(TaskLedStatus,  "Task LedStatus" ,  2048,  NULL,  1,  &LedStatus_task_handle,  ARDUINO_RUNNING_CORE);

  //esperar aqui hasta que cambie de estado un pin que indique que ha corrido todo el hilo de taskledstatus?

  //AQUI VA LA NUEVA PARTE DE BATERIA
  /*  if (digitalRead(PWR_IN)) { //miramos si tiene cable de carga conectado
      switch (checkIfCharging()) { //
        case 4: //cargando
          STATE = CHARGING;
          break;
        case 5: //100% cargado
          STATE = FULLCHARGED;
          break;
        default:
          break;
      }
    }

    if (STATE == CHARGING) {
      //si está cargando:
      //activa blink led rojo y desactiva todo lo demás
      //activa interrupción que reinicie el sistema si el pin PWR_IN tiene flanco de bajada (FIN DE CARGA)
      //entra en deepsleep y cada 10 minutos se despierta para volver a comprobar el estado de carga
    }
    if (STATE == FULLCHARGED) {
      //si esta 100% cargado
      //activa led rojo FIJO
      //activa interrupción que reinicie el sistema si el pin PWR_IN tiene flanco de bajada (FIN DE CARGA)
      //entra en deepsleep
    }*/




  if (STATE == CONFIG) { //la tarea del BLE solo se inicia si el dispositivo ha sido reiniciado
    xSemaphoreGive(BLE_txSemaphore); //libera semaforo BLE
    xTaskCreatePinnedToCore(TaskBLE,  "Task BLE" ,  4096,  NULL,  1,  &BLE_task_handle,  ARDUINO_RUNNING_CORE);
    long sSleepTime = 0;

    while (true) { //SI ESTAMOS EN CONFIG NO PASA AL LOOP HASTA QUE TERMINE EL ESTADO DE CONFIG Y CONFIGURE LA PRIMERA ALARMA
      myDelayMs(500);
      if ((STATE != CONFIG)) { //Si se ha terminado de configurar (cierre de hilo BLE y WIFI)SALE CON STATE IDDLE

        sSleepTime = sToNextSesion(getNextSesion(rtc.getTimeStruct())); //mira cuándo es la próxima sesióncargado


        Serial.println("s Sleep Time: " + String(sSleepTime));

        break; //sale del while
      }
    }

    // ENTRA EN MODO DEEP_SLEEP
    Serial.println("\nLowering Frequency_");
    setCpuFrequencyMhz(10);
    Serial.updateBaudRate(BAUD_SERIAL);
    Serial1.updateBaudRate(BAUD_SERIAL);
    checkFreqs();

    //SETEA ALARMA
#ifdef RESET_WITH_RTC
    Serial.println("Sleeping for: " + String(sSleepTime) + " s");
    esp_sleep_enable_timer_wakeup(sSleepTime * S_TO_uS_FACTOR); //uS
#else
    Serial.println("Sleeping for: 1 s");
    esp_sleep_enable_timer_wakeup(1 * S_TO_uS_FACTOR); //uS
#endif
    STATE = SLEEP;

    detachInterrupt(digitalPinToInterrupt(PWR_IN));
    if (digitalRead(PWR_IN)) { //si esta a nivel alto
      esp_sleep_enable_ext1_wakeup(PWR_IN_MASK, ESP_EXT1_WAKEUP_ANY_LOW); // ISR_DeepSleep para cuando se desconecta el cable de PWR_IN (carga)
    }
    else { //si esta a nivel bajo
      esp_sleep_enable_ext1_wakeup(PWR_IN_MASK, ESP_EXT1_WAKEUP_ANY_HIGH); // ISR_DeepSleep para cuando se desconecta el cable de PWR_IN (carga)
    }

    esp_deep_sleep_start();

    //LO SIGUIENTE NUNCA TIENE LUGAR
    Serial.println("ERROR0");
  }
  else {
    xSemaphoreGive(manager_semaphore); //libera semaforo manager
  }
}



long lastLoop = millis();
void loop ()//funciona como un hilo más cuando pase el setup
{
  if (((millis() - lastLoop) >= 60000) || (STATE != PRESTATE)) {
    PRESTATE = STATE;
    lastLoop = millis();

    Serial.println("\n----------------------------");
    Serial.print("STATE: ");
    switch (STATE) {
      case IDDLE: Serial.println("IDDLE"); break;
      case CONFIG: Serial.println("CONFIG"); break;
      case INTIME: Serial.println("INTIME"); break;
      case MEASURING: Serial.println("MEASURING"); break;
      case SLEEP: Serial.println("SLEEP"); break;
      case CHARGING: Serial.println("CHARGING"); break;
      case FULLCHARGED: Serial.println("FULL CHARGED"); break;
    }
    Serial.println("----------------------------");


    if (STATE == MEASURING) {
      long timeToFinish = 0;
      //Serial.print(xTimerGetExpiryTime(sesionFreqTimer)); Serial.print("   |   "); Serial.print(xTimerGetPeriod(sesionFreqTimer)); Serial.print("   |   "); Serial.println(xTaskGetTickCount());
      timeToFinish = (long)pdTICKS_TO_MS(xTaskGetTickCount());                 //TIEMPO ACTUAL REAL(ms)
      timeToFinish += (long)pdTICKS_TO_MS(xTimerGetPeriod(sesionFreqTimer));    //CUANDO SE CUMPLE EL TIEMPO DEL TIMER EN TIEMPO REAL(ms)
      timeToFinish -= (long)pdTICKS_TO_MS(xTimerGetExpiryTime(sesionFreqTimer)); // MEASURING_TIME (ms)
      timeToFinish = timeToFinish / 1000; //pasamos a seg
      Serial.print(timeToFinish / 60); /*Serial.print(" min  "); Serial.print(timeToFinish % 60); Serial.print(" sec ");*/Serial.print(" / ");
      Serial.print(pdTICKS_TO_MS(xTimerGetPeriod(sesionFreqTimer)) / 60000); //tiempo total MEASURING_TIME
      Serial.println("  min\n");
    }
    else if (STATE == INTIME) {
      Serial.println("PREPARING MEASUREMENT");
      xEventGroupSetBits(xEventDatos, BIT_1_DAT_SESION); //habilita bit sesión
      xTaskCreatePinnedToCore(TaskDataReading,  "Task Read" ,  4096        ,  NULL,  1  ,  &data_read_task_handle ,  ARDUINO_RUNNING_CORE); //lanza hilo de medidas
    }
    xSemaphoreGive(manager_semaphore);
  }

  if (STATE == IDDLE) { //entra después de haber cerrado el archivo de la sesión
    long sToSleep = 0;
    sToSleep = sToNextSesion(getNextSesion(rtc.getTimeStruct())); //mira cuándo es la próxima sesión
    Serial.println("s Sleep Time: " + String(sToSleep));
    Serial.println("\nLowing Frequency");
    setCpuFrequencyMhz(10);
    Serial.updateBaudRate(BAUD_SERIAL);
    checkFreqs();
    Serial.print("going to sleep for: ");
#ifdef RESET_WITH_RTC
    Serial.print(sToSleep); Serial.println(" s\n\n");
    esp_sleep_enable_timer_wakeup(sToSleep * S_TO_uS_FACTOR); //uS
#else
    unsigned long sleepingTime = ((FREQ_TIME * 60 * 1000) - millis()); //FREQ_TIME en min y sleepingTime en ms tiene en cuenta el tiempo que ha pasado desde que se despertó
    esp_sleep_enable_timer_wakeup(sleepingTime * mS_TO_uS_FACTOR); //4 * 3600 * S_TO_uS_FACTOR); //uS
    sleepingTime = sleepingTime / 1000; //pasado a segundos
    Serial.print(sleepingTime / 60); Serial.print(" min "); Serial.print(sleepingTime % 60); Serial.println(" sec\n\n");
#endif
    STATE = SLEEP;
    xSemaphoreGive(manager_semaphore);

    detachInterrupt(digitalPinToInterrupt(PWR_IN));
    if (digitalRead(PWR_IN)) { //si esta a nivel alto
      esp_sleep_enable_ext1_wakeup(PWR_IN_MASK, ESP_EXT1_WAKEUP_ANY_LOW); // ISR_DeepSleep para cuando se desconecta el cable de PWR_IN (carga)
    }
    else { //si esta a nivel bajo
      esp_sleep_enable_ext1_wakeup(PWR_IN_MASK, ESP_EXT1_WAKEUP_ANY_HIGH); // ISR_DeepSleep para cuando se desconecta el cable de PWR_IN (carga)
    }

    esp_deep_sleep_start();
    //LO SIGUIENTE NUNCA TIENE LUGAR
    Serial.println("ERROR1");
  }
}
