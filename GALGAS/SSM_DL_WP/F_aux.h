


void myDelayMs(int ms)
{
  vTaskDelay( ms  / portTICK_PERIOD_MS );
}

void myDelayMsUntil(TickType_t *previousWakeTime, int ms)
{
  vTaskDelayUntil( previousWakeTime, ms / portTICK_PERIOD_MS );
}

//FUNCION que escanea los dispositivos i2c conectados a la placa
void i2cScann() {
  byte error, address;
  int nDevices = 0;

  myDelayMs(5000);
  Wire.begin();
  Serial.println("Scanning for I2C devices ...");
  for (address = 0x01; address < 0x7f; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if (error != 2) {
      Serial.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  }
}

void checkFreqs() {
  uint32_t Freq = getCpuFrequencyMhz();
  Serial.print("CPU Freq = " + String(Freq) + " MHz\n"); //INIT: 240 MHz
  /*Freq = getXtalFrequencyMhz();
    Serial.print("XTAL Freq = "); //INIT: 40 MHz
    Serial.print(Freq);
    Serial.println(" MHz");
    Freq = getApbFrequency();
    Serial.print("APB Freq = ");  //INIT: 80000000 Hz
    Serial.print(Freq);
    Serial.println(" Hz\n");*/
}

void startPinConfig() {
  pinMode(LED, OUTPUT);
  pinMode(RLED, OUTPUT);
  pinMode(GLED, OUTPUT);
  pinMode(BLED, OUTPUT);
  pinMode(IRQ_IMU, INPUT_PULLUP);
  pinMode(IRQ_RTC, INPUT_PULLUP);
  pinMode(IRQ_FUEL, INPUT_PULLUP);
  pinMode(PWR_RS485_EN, OUTPUT);
  pinMode(PWR_V3_EN, OUTPUT);
  pinMode(DE_485, OUTPUT);
  pinMode(_RE_485, OUTPUT);
  pinMode(PWR_IN, INPUT);

  digitalWrite(LED, HIGH);
  digitalWrite(RLED, HIGH);
  digitalWrite(GLED, HIGH);
  digitalWrite(BLED, HIGH);

  digitalWrite(DE_485, LOW);
  digitalWrite(_RE_485, LOW);
  digitalWrite(PWR_RS485_EN, LOW); //Alimentación RS485 EXTERNO
  digitalWrite(PWR_V3_EN, LOW); //Alimenta RS485 INTERNO, SD, IMU, PULL-UP I2C
}

/*
   pone todos los pines a nivel bajo menos RLED
   esta función está pensada para el deep sleep en conjuncion con gpio_deep_sleep_hold_en para garantizar que los pines que nos interesan estén en el estado que nos interesa durante el deep sleep
*/
void pinsToLow() {
  digitalWrite(DE_485, LOW);
  digitalWrite(_RE_485, LOW);
  digitalWrite(PWR_RS485_EN, LOW); //Alimentación RS485 EXTERNO
  digitalWrite(PWR_V3_EN, LOW); //Alimenta RS485 INTERNO, SD, IMU, PULL-UP I2C

  //logica negativa
  digitalWrite(LED, HIGH);
  digitalWrite(GLED, HIGH);
  digitalWrite(BLED, HIGH);
}
//________________________________________________NVS________________________________________________
static esp_err_t init_nvs() { //inicializa NVS
  esp_err_t error_ESP;
  esp_err_t error_ESP2;

  error_ESP2 = nvs_flash_init();
  if (error_ESP2 != ESP_OK) {
    ESP_LOGE(NVS_NAMESPACE, "Error init: %d", error_ESP2);
  }

  error_ESP = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle_nvs_vars); //abre con entre var nvs y flash

  if (error_ESP != ESP_OK) {
    ESP_LOGE(NVS_NAMESPACE, "Error opening NVS");
  }
  else {
    ESP_LOGI(NVS_NAMESPACE, "NVS Ready");
  }
  return error_ESP;
}


static esp_err_t read_nvs_16(char* nvs_key, int16_t *nvs_value, bool f_log = true) { //uint16_t *nvs_value) { //lee valores de nvs para uint16_t
  esp_err_t error_ESP;

  //error_ESP = nvs_get_u8(handle_nvs, nvs_key, nvs_value); //getter uint_8
  //error_ESP = nvs_get_u16(handle_nvs, nvs_key, nvs_value);
  error_ESP = nvs_get_i16(handle_nvs_vars, nvs_key, nvs_value);

  if ((error_ESP != ESP_OK) && f_log) {
    ESP_LOGE(nvs_key, "Error read_nvs_16: %d", error_ESP); //4354 clave no encontrada
  }
  else if (f_log) {
    ESP_LOGI(nvs_key, "read_nvs_16 OK");
  }
  return error_ESP;
}

static esp_err_t write_nvs_16(char* nvs_key, int16_t nvs_value, bool f_log = true) { //uint16_t nvs_value) { //guarda valores nvs Nombre de variable y valor uint16_t
  esp_err_t error_ESP;

  //error_ESP = nvs_set_u16(handle_nvs, nvs_key, nvs_value);
  error_ESP = nvs_set_i16(handle_nvs_vars, nvs_key, nvs_value);

  if ((error_ESP != ESP_OK) && f_log) {
    ESP_LOGE(nvs_key, "Error write_nvs_16: %d", error_ESP); //4361 handle o nombre invalido

  }
  else if (f_log) {
    ESP_LOGI(nvs_key, "write_nvs_16 OK");
  }
  return error_ESP;
}


void check_nvs_index_value() { //en caso de RTC_EXT no actualizado comprueba si el valor para generar los archivos es correcto con el usuario
  Serial.println();
  Serial.print(nvs_key_filename); Serial.print(" = ");
  Serial.println(nvs_val_filename);
  Serial.print("OK? [Y/N]");

  long pretime = millis();
  char consig;

  while (!Serial.available() && (millis() - pretime < 5000)) {
    myDelayMs(1000);
    if (Serial.available()) {
      consig = Serial.read();

      if (consig == 'Y') {
        Serial.println("\nOK\n");
        return;
      }
      else if (consig == 'N') {
        Serial.print("\nIntroduce new value");
        int newValue;
        while (!Serial.available() && (millis() - pretime < 20000)) {
          myDelayMs(1000);
          if (Serial.available()) {
            newValue = (Serial.readString()).toInt();
            Serial.println();

            //EL VALOR INTRODUCIDO SE ASIGNA A nvs_val_introduced, SERÁ EL QUE SE UTILIZARÁ EN LOS ARCHIVOS SI RTC_EXT NO ESTÁ ACTUALIZADO
            write_nvs_16(nvs_key_filename, newValue); //escribimos el nuevo valor en la key
            read_nvs_16(nvs_key_filename, &nvs_val_filename); //actualizamos nuestro valor de nvs_val_day

            Serial.print(nvs_key_filename); Serial.print(": ");
            Serial.print(nvs_val_filename);
            Serial.println(" - OK\n");
            return;
          }
          Serial.print('.');
        }
      }
    }
    Serial.print(".");
  }
}
//________________________________________________TIMERS________________________________________________
//Configura los timers
TimerHandle_t sesionFreqTimer;
uint8_t timerId = 1;

void sesionFreqTimerCallBack(TimerHandle_t expiredTimer) {
  Serial.println("ENDING MEASUREMENT");
  xEventGroupClearBits(xEventDatos, BIT_1_DAT_SESION );//limpiamos BIT_1_DAT_SESION para que se cierre archivo y hilo de datos
}


esp_err_t setup_Timer() {
  const char TAG_TIMERSESION[] = "[TimerSesion]";
  ESP_LOGI(TAG_TIMERSESION, "Starting Timer");
  sesionFreqTimer = xTimerCreate("sesionTimer", //just a name
                                 pdMS_TO_TICKS(MEASURING_TIME * 60000), //timer period ticks
                                 pdFALSE, //autoreload when expires
                                 (void *)&timerId, //assign ech timer  auniq id equal to its array index
                                 sesionFreqTimerCallBack);

  if (sesionFreqTimer == NULL) {
    ESP_LOGE(TAG_TIMERSESION, "ERROR! timer was not created");
  }
  else {
    if (xTimerStart(sesionFreqTimer, 0) != pdPASS) {
      ESP_LOGE(TAG_TIMERSESION, "ERROR! timer could not set in active state");
    }
  }

  return ESP_OK;
}
//________________________________________________TIME GENERAL FUNC________________________________________________

void setup_RTC_EXT() //configuraciones iniciales
{
  //antes de hacer el begin vamos a comprobar que exista una diferencia entre ambos RTC

  while (!rtcEXT.begin()) {
    Serial.println("RTC EXT not found");
  }
  // Set square wave out pin
  // SquareWaveDisable, SquareWave1Hz, SquareWave4096Hz, SquareWave8192Hz, SquareWave32768Hz
  //rtcEXT.setSquareWave(SquareWaveDisable); //deshabilita señal de reloj de salida del RTC
}



void sync_RTCs(bool ext_to_int, bool checkAll = false) //función que sincroniza los RTCs true = EXT to INT, false = INT to EXT
{
  digitalWrite(PWR_V3_EN, HIGH); //habilita  PULL-UP I2C

  struct tm extTime;
  if (ext_to_int) { //si se actualiza EXT to INT
    rtcEXT.read(&extTime); //leemos el valor del RTCEXT

    if ((extTime.tm_year < 124) && checkAll) { //en caso de que no esté actualizado o no se desee hacer el checkAll (124 años que han pasado desde 1900 hasta 2024)
      Serial.println("\nRTC EXT NOT ON TIME, seting NVS_FILENAME");
      //check_nvs_index_value(); //comprueba si el valor que se le va a poner al archivo es correcto con el usuario

      extTime.tm_year = 100; //100 años pasados desde 1900 hasta 2000)
      extTime.tm_mon = 00;
      extTime.tm_mday = 01; //nvs_val_day;
      extTime.tm_hour = 00;
      extTime.tm_min = 00;
      extTime.tm_sec = 00;

      rtcEXT.write(&extTime);
      rtcEXT.read(&extTime);
    }
    else if (extTime.tm_year >= 124) { //(124 años que han pasado desde 1900 hasta 2024)
      Serial.println("\nRTC_EXT ON TIME =)");
    }
    else {
      Serial.println("\nRTC NOT ON TIME BUT SETTED");
    }

    rtc.setTimeStruct(extTime);  //ACTUALIZAMOS EL RTC_INT CON EL VALOR DE RTC_EXT
    Serial.print("RTC-INT ACTUALIZED BY RTC_EXT: ");
    Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));
  }
  else { //si se actualiza INT to EXT
    Serial.print("\nRTC-EXT ACTUALIZED BY RTC_INT: ");
    struct tm intTime;
    intTime = rtc.getTimeStruct(); //leemos valor de RTCINT
    myDelayMs(5);
    rtcEXT.write(&intTime); // condicion para actualizar RTCEXT:ENERO = 0, año ha de estar tal cual 2024
    myDelayMs(5);
    rtcEXT.read(&intTime);
  }

  Serial.print("\nRTC_EXT: "); Serial.println(&extTime, "%A, %B %d %Y %H:%M:%S"); //leemos de nuevo el valor para guardar la configuración en initTime
  Serial.print("RTC_INT: "); Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));
}


void printLocalTime() //imprime valor RTCint
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) { //obtiene la hora GMT a partir del rtc interno después de actualizarse por el NTP, función de time.h
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void timeavailable(struct timeval * t) //función para notificar la sincronización del RTC interno con el NTP
{
  Serial.println("NTP syncronized:");
  printLocalTime();

  //ACTUALIZAMOS RTC_EXT CON EL VALOR DEL NTP QUE HA LLEGADO AL RTC_INT
  sync_RTCs(false);
}

//________________________________________________BATTERY________________________________________________
bool setupBQ27441(bool sendPrints = true)
{
  // Use lipo.begin() to initialize the BQ27441-G1A and confirm that it's
  // connected and communicating.
  if (!lipo.begin()) // begin() will return true if communication is successful
  {
    // If communication fails, print an error message and loop forever.
    if (sendPrints) {
      Serial.println("Error: Unable to communicate with BQ27441.");
      Serial.println("  Check wiring and try again.");
      Serial.println("  (Battery must be plugged into Battery Babysitter!)");
    }
    return false;
  }
  if (sendPrints) {
    Serial.println("Wire Connected to BQ27441!");
  }

  // Uset lipo.setCapacity(BATTERY_CAPACITY) to set the design capacity
  // of your battery.
  lipo.setCapacity(BATTERY_CAPACITY);
  return true;
}


/*
   ESTA FUNCION RECIVE UN VECTOR DE ENTEROS DE 7 HUECOS
   DEVUELVE EN UN ARRAY DE ENTEROS LOS VALORES DE INFO DE LA BATERIA EN EL SIGUIENTE ORDEN:
   0: soc           ->Estado de carga(%)
   1: volts         -> Tensión de batería(mV)
   2: volts       -> Corriente de batería(mA)
   3: Capacity      -> Capacidad remanente de batería (mAh)
   4: Full Capacity -> Capacidad total de batería (mAh)
   5: Power         -> Potencia entregada / absorbida (mW)
   6: health        -> Estado de la batería (%)
*/
void getBatInfo(int (&v_infoBat)[7])
{
  // Read battery stats from the BQ27441-G1A
  unsigned int soc = lipo.soc();  // Read state-of-charge (%)
  unsigned int volts = lipo.voltage(); // Read battery voltage (mV)
  int current = lipo.current(AVG); // Read average current (mA)
  unsigned int fullCapacity = lipo.capacity(FULL); // Read full capacity (mAh)
  unsigned int capacity = lipo.capacity(REMAIN); // Read remaining capacity (mAh)
  int power = lipo.power(); // Read average power draw (mW)
  int health = lipo.soh(); // Read state-of-health (%)

  v_infoBat[0] = soc;
  v_infoBat[1] = volts;
  v_infoBat[2] = current;
  v_infoBat[3] = capacity;
  v_infoBat[4] = fullCapacity;
  v_infoBat[5] = power;
  v_infoBat[6] = health;

  for (uint8_t idx = 0; idx < 7; idx++) {
    Serial.print(v_infoBat[idx]);
    if (idx != 6) {
      Serial.print(",");
    }
  }
  Serial.println();
}



/*Esta función comprueba si el dispositivo está cargando a partir del pin de tensión de entrada y la corriente que entra a la batería
   Salidas:
   OJO: hay condiciones que en función de si se hace antes o después del inicio pueden variar las lecturas, no asumir todas las condiones como válidas (ver excel info dispositivo)
   0: Desconectada, Battery System Protect activado, Bat desconectada + Switch ON -> estado carga 0%, AvCurrent 0mA, Capacidad 0mAh
   1: Switch off + protec ON                         -> NO RESPONDE (batería conectada o no, switch off , protección ON)
   2: Switch off + protec OFF                        -> AvCurrent 0mA, Capacity > 0
   3: En descarga                                    -> AvCurrent < 0
   4: Cargando                                       -> AvCurrent > 0, estado carga <100%
   5: Carganda 100%                                  -> AvCurrent >= 0, estado carga 100%
   >5 UNKNOWN
*/
uint8_t checkIfCharging() {
  digitalWrite(PWR_V3_EN, HIGH); //aseguramos que se habilita  PULL-UP I2C
  if (!setupBQ27441(false)) {
    return 1; //Switch off
  }
  int v_infoBat[7];
  //obtenemos los valores de batería  0:Estado de carga(%), 1:Tensión de batería(mV), 2:Corriente(mA), 3:Capacidad remanente(mAh),4:Capacidad total(mAh),5:Potencia(mW), 6:Health(%)
  getBatInfo(v_infoBat);

  if (digitalRead(PWR_IN)) { //cable de carga conectado
    if (v_infoBat[2] >= 0) { //corriente >= 0
      if (v_infoBat[3] >= (0.9 * v_infoBat[4])) { //capacidad = 10000/10400  antes:v_infoBat[0] == 100 carga 100%, esto a veces estando al 97% al conectar cable marca 100%, no lo ponemos en 10400 porque podría tardar muchas horas más
        Serial.println(">>bat full charged");
        return 5; //cargada 100%
      }

      Serial.println("bat in charge");
      return 4; //cargando
    }
  }
  else { //no tiene el cable de carga conectado
    //en descarga
    if (v_infoBat[2] < 0) { //si la corriente es negativa es que está descargando
      return 3;
    }
  }

  if (v_infoBat[2] == 0) { //si no esta consumiendo ni cargandose
    if (v_infoBat[3] > 0) { //si la capacidad es mayor que 0 
      Serial.println("Switch Off + protect Off");
      return 2; //switch off + protect OFF
    }
    else { //capacidad <=0
      if (v_infoBat[0] == 0) { //carga %
        Serial.println("Battery System Protect Enabled");
        return 0; //Desconectada, Battery System Protect activado, Bat desconectada + Switch ON
      }
    }
  }
  Serial.println("->default");
  return 6;
}



/*
   FUNCION USADA PARA COMPROBAR SI EL EQUIPO SE ENCUENTRA CARGANDO O EN CARGA Y CONFIGURAR DEEP SLEEP EN CONSECUENCIA
   toSleep = 1 -> permite a la función poner al equipo en deepsleep
   toSleep = 0 -> no permite a la función dormir al equipo solo cambia el estado de STATE
*/
void checkStateOfChargeToSleep(bool sleepIfCharging = false, bool sleepIfFullCharged = false) {
  if (digitalRead(PWR_IN)) { //miramos si tiene cable de carga conectado

    switch (checkIfCharging()) { //
      case 4: //cargando
        STATE = CHARGING;
        if (sleepIfCharging) {
          Serial.println("\nCHARGING Lowing Frequency");
          setCpuFrequencyMhz(10);
          Serial.updateBaudRate(BAUD_SERIAL);
          //checkFreqs(); QUITADO POR REDUCCION DE TIEMPO

          Serial.println("C-going to sleep for: " + String(BLINK_CHARGE_FREQUENCE) + " s\n\n");

          //Al estar ya encendido habilitamos ISR_DeepSleep a LOW
          detachInterrupt(digitalPinToInterrupt(PWR_IN));
          esp_sleep_enable_ext1_wakeup(PWR_IN_MASK, ESP_EXT1_WAKEUP_ANY_LOW);     //DeepSleep ISR Se despierta por nivel bajo en el pin PWR_IN
          esp_sleep_enable_timer_wakeup(BLINK_CHARGE_FREQUENCE * S_TO_uS_FACTOR); //Se despierta a los BLINK_CHARGE_FREQUENCE segundos

          esp_deep_sleep_start();
          //LO SIGUIENTE NUNCA TIENE LUGAR
          Serial.println("ERROR2");
        }
        break;
      case 5: //100% cargado
        STATE = FULLCHARGED; //cambiamos STATE
        if (sleepIfFullCharged) {
          Serial.println("\nState FULLCHARED Lowing Frequency");
          setCpuFrequencyMhz(10);
          Serial.updateBaudRate(BAUD_SERIAL);
          
          Serial.println("FC-going to sleep for: " + String(BLINK_CHARGE_FREQUENCE*6)+" s\n\n");

          pinsToLow();
          digitalWrite(RLED, LOW);//DEJAMOS FIJO EL RGB ROJO

          //Al estar ya encendido habilitamos ISR_DeepSleep a LOW
          detachInterrupt(digitalPinToInterrupt(PWR_IN));

          esp_sleep_enable_ext1_wakeup(PWR_IN_MASK, ESP_EXT1_WAKEUP_ANY_LOW);     //DeepSleep ISR Se despierta por nivel bajo en el pin PWR_IN
          esp_sleep_enable_timer_wakeup(BLINK_CHARGE_FREQUENCE * 6 * S_TO_uS_FACTOR); //Se despierta a los BLINK_CHARGE_FREQUENCE segundos
          gpio_hold_en((gpio_num_t) RLED);
          gpio_deep_sleep_hold_en(); //aisla los pines haciendo que no cambien de estado a partir de este punto
          esp_deep_sleep_start();

          //LO SIGUIENTE NUNCA TIENE LUGAR
          Serial.println("ERROR3");
        }
        break;
      default:
        break;
    }
  }
}


/*ESTA FUNCION PERMITE DECIDIR AL USUARIO SI EL DISPOSITIVO DEBE DORMIR HASTA QUE SE CARGUE COMPLETAMENTE EL DISPOSITIVO O SE DESCONECTE LA ALIMENTACION
   en caso de decidir que no se duerma la desición se guarda en una variable global y no se puede revertir sin reiniciar el equipo
   0: abort sleep while charging
   1: sleep during charge
*/
bool checkIfWantToSleepCharging() {
  if (CHOOSE_SLEEP_DURING_CHARGE) {
    Serial.println();
    Serial.println("Power wire to charge battery is connected. Want to GO TO SLEEP until disconnect or full charge?");
    Serial.print("OK? [Y/N]");

    long pretime = millis();
    char consig;

    while (!Serial.available() && (millis() - pretime < 5000)) {
      myDelayMs(3000);
      if (Serial.available()) {
        consig = Serial.read();

        if (consig == 'Y') {
          Serial.println("\nOK, preparing to sleep\n");
          return 1;
        }
        else if (consig == 'N') {
          CHOOSE_SLEEP_DURING_CHARGE = false;
          Serial.println("\nOK, disabling deep sleep during charge\n");
          return 0; //abort sleep while charging
        }
      }
      Serial.print(".");
    }
    return 1; //en caso de pasar el timeout se va a dormir
  }
  else { //si se cambió previamente
    return 0; //abort sleep while charging
  }
}



/*
   FUNCION USADA PARA COMPROBAR SI EL EQUIPO SE ENCUENTRA CARGANDO O EN CARGA DESPUES DE UNA INTERRUPCION POR PIN PWR_IN EN SLEEPMODE
   esta función es necesaria porque en deep sleep no funciona ni ISR_disconnectPowerCable(falling) ni ISR_connectPowerCable(rising), por lo que si el cambio de estado se produce mientras duerme no se daría cuenta nunca
   hay que asegurarse de que esta función no intervenga cuando lo hacen las otras. Ambas pueden hacer uso del switch(STATUS) inicial de Setup()
   A esta función solo se debe acceder si venimos de despertanos de la interrupción relacionada:
   esp_sleep_enable_ext1_wakeup(PWR_IN_MASK, ESP_EXT1_WAKEUP_ANY_HIGH);
   esp_sleep_enable_ext1_wakeup(PWR_IN_MASK, ESP_EXT1_WAKEUP_ANY_LOW);

   con el fin de que esta función realice lo que debería realizar:
   ISR_disconnectPowerCable(falling)
   ISR_connectPowerCable(rising)

   Ya que del deepsleep solo podemos detectar que la función fue causada por ext1 pero no si ha sido rising o falling
*/
void checkStateOfChargeIfISRInSleepMode() {
  if (digitalRead(PWR_IN)) { //si está encendido asumimos que ha sido una conexión durante el deep sleep
    Serial.println("\n>>>ISR_DEEPSLEEP_connectPowerCable");

    //comprobar que no esté midiendo en una sesión
    if ((STATE != INTIME) && (STATE != MEASURING)) {
      myDelayMs(500); //delay para dar tiempo a la batería a que empiece a cargar
      //ver si está cargando o cargado
      if (checkIfWantToSleepCharging()) { //preguntamos al usuario si quiere abortar el sleepmode

        checkStateOfChargeToSleep(true, true); //comprobamos estado de carga y si está cargando volvemos a dormir
      }
    }
    else {
      Serial.println(">>>0 - PASO POR AQUI BORARR ESTO"); //PASA POR AQUI 
    }
  }
  else { //si no está encendido asumimos que ha sido una desconexión durante el deep sleep
    Serial.println("\n>>>ISR_DEEPSLEEP_disconnectPowerCable");
    gpio_hold_dis((gpio_num_t) RLED);
    gpio_deep_sleep_hold_dis(); //quita el aislamiento del todos los pines haciendo que puedan cambiar de estado en deep sleep
    digitalWrite(RLED, HIGH);//en caso de que estuviese encendido apaga el led (logica inversa)
    if ((STATE != INTIME) && (STATE != MEASURING)) {
      STATE = CONFIG;
    }
    else {
      Serial.println(">>>1 - PASO POR AQUI BORARR ESTO");
    }
  }
}


//prototipo de funcion
void ISR_connectPowerCable();
/*
   ESTA INTERRUPCION SE ENCARGA DE CUANDO EL EQUIPO ESTA DESPIERTO INDICAR QUE SE HA DESCONECTADO EL CABLE DE ALIMENTACION
   Cambiará el estado de la variable de control STATE a CONFIG
   OJO LAS INTERRUPCIONES SE DESACTIVAN AL ENTRAR EN DEEPSLEEP
*/
void IRAM_ATTR ISR_disconnectPowerCable() {
  if ((millis() - lastTimeISRWire) > 200) { //DEBOUNCING
    detachInterrupt(digitalPinToInterrupt(PWR_IN));
    Serial.println("\n>>>ISR_disconnectPowerCable");

    gpio_deep_sleep_hold_dis(); //quita el aislamiento del pin para que pueda apagarse en el deepsleep
    digitalWrite(RLED, HIGH);//en caso de que estuviese encendido apaga el led (logica inversa)

    if ((STATE != INTIME) && (STATE != MEASURING)) {
      STATE = CONFIG;
    }
    attachInterrupt(digitalPinToInterrupt(PWR_IN), ISR_connectPowerCable, RISING); //habilitamos interrupción conexion de PWR_IN (CARGA DE BATERIA)
    Serial.println("\n>>>FIN DE INTERRUPCION");
  }
  lastTimeISRWire = millis();
}


/*
   ESTA INTERRUPCION SE ENCARGA DE CUANDO EL EQUIPO ESTA DESPIERTO INDICAR QUE SE HA CONECTADO EL CABLE DE ALIMENTACION
   Cambiará el estado de la variable de control STATE afectando a las condiciones impuestas para esta en setup y loop
   OJO LAS INTERRUPCIONES SE DESACTIVAN AL ENTRAR EN DEEPSLEEP
*/
void IRAM_ATTR ISR_connectPowerCable() { //IRAM para que vaya a la RAM y no a la flash y sea más rapida
  if ((millis() - lastTimeISRWire) > 200) { //DEBOUNCING
    detachInterrupt(digitalPinToInterrupt(PWR_IN));
    Serial.println("\n>>>ISR_connectPowerCable");

    //comprobar que no esté midiendo en una sesión
    if ((STATE != INTIME) && (STATE != MEASURING)) {
      xTaskCreate(TaskISRWire,  "Task ISRWire" ,  3072,  NULL,  1,  &ISRWire_task_handle);
    }
    attachInterrupt(digitalPinToInterrupt(PWR_IN), ISR_disconnectPowerCable, FALLING); //habilitamos interrupción desconexion de PWR_IN
    Serial.println(">>>FIN DE INTERRUPCION2");
  }
  lastTimeISRWire = millis();
}



std::string batteryStats(bool checkSecureSystem = false)
{
  // Read battery stats from the BQ27441-G1A
  unsigned int soc = lipo.soc();  // Read state-of-charge (%)
  unsigned int volts = lipo.voltage(); // Read battery voltage (mV)
  int current = lipo.current(AVG); // Read average current (mA)
  unsigned int fullCapacity = lipo.capacity(FULL); // Read full capacity (mAh)
  unsigned int capacity = lipo.capacity(REMAIN); // Read remaining capacity (mAh)
  int power = lipo.power(); // Read average power draw (mW)
  int health = lipo.soh(); // Read state-of-health (%)

  std::string toPrint = "";

  if (checkSecureSystem) {
    if (current == 0) {
      toPrint = "ATTENTION!:  battery not connected or Secure System Enabled:\n\t";
    }
    else if (current > 0) {
      toPrint = "Battery Charging:\n\t";
    }
    else {
      toPrint = "Battery in normal use:\n\t";
    }
  }

  // Now print out those values:
  toPrint += std::to_string(soc);
  toPrint += ",";
  toPrint += std::to_string(volts);
  toPrint += ",";
  toPrint += std::to_string(current);
  toPrint += ",";
  toPrint += std::to_string(capacity);
  toPrint += ",";
  toPrint += std::to_string(fullCapacity);
  toPrint += ",";
  toPrint += std::to_string(power);
  toPrint += ",";
  toPrint += std::to_string(health);

  Serial.print("\n[BATTERY INFO] - ");
  Serial.println(toPrint.c_str());
  return toPrint;
}

std::string getBatteryStatus(bool checkSecureSystem = false) {
  digitalWrite(PWR_V3_EN, HIGH); //aseguramos que se habilita  PULL-UP I2C
  if (!setupBQ27441()) { //si no consigue contactar con el cargador
    return ("Battery not available check switch and battery secure system"); //puede ser porque no esté habilitado el switch
  }
  myDelayMs(2000); //tiempo para estabilizar medida
  return batteryStats(checkSecureSystem);
}



//Función que imprime por pantalla a que se debió el wakeup para filtrar cuando se despierta por haber estado en LowPower
/*
    ESP_SLEEP_WAKEUP_UNDEFINED,    //!< In case of deep sleep, reset was not caused by exit from deep sleep
    ESP_SLEEP_WAKEUP_ALL,          //!< Not a wakeup cause, used to disable all wakeup sources with esp_sleep_disable_wakeup_source
    ESP_SLEEP_WAKEUP_EXT0,         //!< Wakeup caused by external signal using RTC_IO
    ESP_SLEEP_WAKEUP_EXT1,         //!< Wakeup caused by external signal using RTC_CNTL
    ESP_SLEEP_WAKEUP_TIMER,        //!< Wakeup caused by timer
    ESP_SLEEP_WAKEUP_TOUCHPAD,     //!< Wakeup caused by touchpad
    ESP_SLEEP_WAKEUP_ULP,          //!< Wakeup caused by ULP program
    ESP_SLEEP_WAKEUP_GPIO,         //!< Wakeup caused by GPIO (light sleep only)
    ESP_SLEEP_WAKEUP_UART,         //!< Wakeup caused by UART (light sleep only)
    ESP_SLEEP_WAKEUP_WIFI,              //!< Wakeup caused by WIFI (light sleep only)
    ESP_SLEEP_WAKEUP_COCPU,             //!< Wakeup caused by COCPU int
    ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG,   //!< Wakeup caused by COCPU crash
    ESP_SLEEP_WAKEUP_BT,           //!< Wakeup caused by BT (light sleep only)
*/
void wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("\n>>>Wakeup caused by external signal using RTC_IO");  break;
    case ESP_SLEEP_WAKEUP_EXT1 : //Este caso puede pasar por la desconexión del cable de alimentación ISR en deepSleep
      Serial.println("\n>>>Wakeup caused by external signal using RTC_CNTL");
      checkStateOfChargeIfISRInSleepMode();
      //STATUS = CONFIG; //pasamos el STATUS a config
      break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("\n>>>Wakeup caused by timer"); sync_RTCs(true); break; //actualizamos RTCINT con RTCEXT para no perder segundos (no comprueba si es correcto)
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("\n>>>Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("\n>>>Wakeup caused by ULP program"); break;
    default :
      Serial.printf("\n>>>Wakeup was not caused by deep sleep (LP): %d\n", wakeup_reason);
      Serial.printf("reason: %d\n", esp_reset_reason());

      sync_RTCs(true, true); //actualizamos RTCINT A PARTIR DE RTCEXT y hacemos check de RTCEXT por si no estuviese en hora
      break;

      //!!!FALTA COMPROBAR SI EN LOS OTROS CASOS NO SE PIERDE EL DATO DEL RTCINT
  }
}

const byte myAlarmSesion[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};//{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};//{ 2, 6, 10, 14, 18, 22};
//Obtiene la hora de la siguiente sesión en función de un vector de horas -> se posiciona en el vector en la hora que coincide con la actual (para posteriormente comprobar timewindow) o la siguiente
uint8_t getNextSesion(struct tm actual_tm) {
  uint8_t actualHour = actual_tm.tm_hour;
  uint8_t idx = 0;
  uint8_t numHoras = NUMITEMS(myAlarmSesion);

  while ((idx < numHoras) && (actualHour > myAlarmSesion[idx])) {
    idx++;
  }

  if (idx >= numHoras) {
    idx = 0;
  }

  return myAlarmSesion[idx]; //puede coincidir con la hora actual (por timewindow)
}

bool inTimeWindow(tm t_actual, uint8_t alarmTime) //Comprueba si está TIME_WINDOW minutos por encima de la hora de medición y si es así informa con un bool
{
  byte lmin = t_actual.tm_min;
  byte lhour = t_actual.tm_hour;
  byte idx = 0;
  uint8_t numHoras = NUMITEMS(myAlarmSesion);

  while (( idx < numHoras ) && (lhour > myAlarmSesion[idx])) //busca la hora en myAlarmSesion que sea superior a la hora actual
  {
    idx++;
  }
  return ((lhour == myAlarmSesion[idx]) && (lmin < TIME_WINDOW));//SI ES LA MISMA HORA Y SE HA PASADO TIME_WINDOW  //|| (((lhour + 1) == myAlarmSesion[idx]) && (lmin > 60 - TIME_WINDOW))); || SI ES UNA HORA MENOS Y LE FALTAN TIME_WINDOW
}




//calcula los s que quedan para la siguiente sesión a partir de la hora actual
/*
   24h = 00h = 86.400 s
*/
//recive hora actual y hora de alarma calculada por getNextSesion, tiene en cuenta el timewindow
uint32_t sToNextSesion(uint8_t alarmTime) {//TEST
  struct tm actual_tm = rtc.getTimeStruct();
  uint32_t actual_s = 0; //almacena la hora actual en S
  uint32_t next_s = 0;   //almacena la hora de alarma en S y posteriormente el resultado de la operacion para devolverlo

  if (actual_tm.tm_hour == alarmTime) { //si la hora actual es la misma que la de la alarma
    Serial.println("checking inTimeWindow");
    if (inTimeWindow(actual_tm, alarmTime)) { //comprueba si está dentro del timewindow
      Serial.println("inTime");
      return 1; //si es así retorna 1 seg
    }
    else {
      Serial.println("not in TimeWindow, going to next sesion");
      uint8_t idx = 0;
      uint8_t numHoras = NUMITEMS(myAlarmSesion);
      while ((idx < numHoras) && (alarmTime >= myAlarmSesion[idx])) {
        idx++;
      }
      if (idx >= numHoras) {
        idx = 0;
      }
      alarmTime = myAlarmSesion[idx];
    }
  }

  actual_s = (actual_tm.tm_hour * 3600 + actual_tm.tm_min * 60 + actual_tm.tm_sec); //pasamos tiempo actual a segundos
  Serial.print("\nACTUAL HOUR: "); Serial.print(&actual_tm, "%A, %B %d %Y %H:%M:%S"); Serial.println(" |Actual_s: " + String(actual_s));
  next_s = alarmTime * 3600; //pasamos hora de alarma a segundos
  Serial.print("NEXT HOUR: "); Serial.print(alarmTime); Serial.println(" |Next_s: " + String(next_s));

  if (actual_s < next_s) {//                     00|-----A________________N--------------|23:59
    next_s = next_s - actual_s;
  }
  else { //(next_s < actual_s)                   00|_____N----------------A______________|23:59
    next_s = 86400 - actual_s + next_s;
  }

  //Serial.println("s to Sesion: " + String(next_s));

  return next_s;
}

/*void setAlarmRTC() {

  }*/






//________________________________________________LED________________________________________________
/* CODIGOS LED:
    3Colores: Inicio hilo led
    L2(rojo) parpadeo: Funcionamiento hilo led
    Parpadeo azul : TaskBLE habilitada
    Parpadeo verde: En sesion
*/
void blinkLed(uint8_t led, int time, uint8_t pulses = 1) //DEBERIA USARSE UNICAMENTE POR EL HILO DE LEDS PARA EVITAR RETRASOS
{
  uint8_t count = 0;
  switch (led)
  {
    case 0: //led solo rojo
      if (pulses != 0) {
        for (count = 0; count < pulses; count++) {
          digitalWrite(LED, LOW);

          myDelayMs(time);
          digitalWrite(LED, HIGH);
        }
      }
      else {
        digitalWrite(LED, LOW);
      }
      break;
    case 1: //led rojo RGB
      if (pulses != 0) {
        for (count = 0; count < pulses; count++) {
          digitalWrite(RLED, LOW);
          myDelayMs(time);
          digitalWrite(RLED, HIGH);
        }
      }
      else {
        digitalWrite(RLED, LOW);
      }
      break;
    case 2: //led verde RGB
      if (pulses != 0) {
        for (count = 0; count < pulses; count++) {
          digitalWrite(GLED, LOW);

          myDelayMs(time);
          digitalWrite(GLED, HIGH);
        }
      }
      else {
        digitalWrite(GLED, LOW);
      }
      break;
    case 3: //led azul RGB
      if (pulses != 0) {
        for (count = 0; count < pulses; count++) {
          digitalWrite(BLED, LOW);
          myDelayMs(time);
          digitalWrite(BLED, HIGH);
        }
      }
      else {
        digitalWrite(BLED, LOW);
      }
      break;
  }
  myDelayMs(time);
}
//________________________________________________RS485_____________________________________________

//lee el buffer de recepción del RS485 hasta el caracter ']'
std::string waitResponseRS485Command() {
  //const char TAG_RS485_RX[] = "[RS485-Rx]";
  digitalWrite(PWR_V3_EN, HIGH);//INTERNO
  digitalWrite(PWR_RS485_EN, HIGH); //EXTERNO

  //ponemos los pines en modo recepcion:
  digitalWrite(_RE_485, LOW);
  digitalWrite(DE_485, LOW);
  myDelayMs(50);

  std::string rxDataRS485 = "";

  //  unsigned long timeOut = millis();


  Serial.print("waitResponseRS485Command...   |   ");
  char c = 0;
  while (c != ']') { //((millis() - timeOut) < TIME_OUT_RS485 * 1000) && (c != ']')) {
    if (Serial1.available() > 0) {
      c = Serial1.read();
      if (c != '\0') {
        rxDataRS485 += c; //lee todo el mensaje añade también el \n
      }
    }
  }


  if (rxDataRS485.length() > 0) {
    Serial.print("[RS485-Rx]: ");
    for (int i = 0; i < rxDataRS485.length(); i++)
    {
      Serial.write(rxDataRS485[i]);
    }
    Serial.println();
    //ESP_LOGI(TAG_RS485_RX, "%s", rxDataRS485);
  }

  return rxDataRS485;
}


void sendTransmisionRS485(std::string command) {
  //const char TAG_RS485_TX[] = "[RS485-Tx]";
  //NOS ASEGURAMOS DE QUE ESTE ENCENDIDO
  digitalWrite(PWR_RS485_EN, HIGH);
  digitalWrite(PWR_V3_EN, HIGH);
  //ponemos los pines en modo transmision:
  digitalWrite(_RE_485, HIGH);
  digitalWrite(DE_485, HIGH);
  myDelayMs(5);

  Serial1.print(command.c_str());
  Serial1.print("\n");
  //ESP_LOGI(TAG_RS485_TX, "Sended: %s", command);
  Serial.print("[RS485-Tx]: "); Serial.println(command.c_str());

  myDelayMs(5);
  //ponemos los pines en modo recepcion:
  digitalWrite(DE_485, LOW);
  digitalWrite(_RE_485, LOW);
  myDelayMs(5);
}

//________________________________________________BT________________________________________________
BLEServer *pServer = NULL;
bool f_BLE_deviceConnected = false;     //indica si hay algun dispositivo conectado por BLE
bool f_oldDeviceConnected = false;        //informa del estado anterior para saber si hay conexión o desconexión
bool f_rx_ble_data = false;        //indica si se ha recibido algo por BLE

std::string rxBLE = "";
std::string txBLE = "";

//uint8_t txValue = 0;
//std::string txValue = "hola\n";
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      f_BLE_deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      f_BLE_deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      rxBLE = std::string(pCharacteristic->getValue().c_str());
      f_rx_ble_data = true;
    }
};

//________________________________________________IMU________________________________________________

/*Obtiene los valores de la imu a partir de un filtro paso bajo de media movil ponderada
   25samples 47ms
   samples: numero de muestras que se desean tomar para el cálculo
   movil: cantidad de bits de desplazamiento para hacer media movil -> 2^movil datos, movil 2->4samples
   rst reinicia el valor de las variables estáticas usadas para el filtro, debe reiniciarse si se cambia de condiciones iniciales (posición) respecto a la última calibración
*/
void getIMUFiltered(IMUStruct & IMUParams, uint16_t samples = 25, uint8_t movil = 2, bool rst = false) {

   if (samples == 1) {
      sensorIMU.getAcceleration(&IMUParams.measure_aX, &IMUParams.measure_aY, &IMUParams.measure_aZ);
      sensorIMU.getRotation(&IMUParams.measure_gX, &IMUParams.measure_gY, &IMUParams.measure_gZ);
  
      IMUParams.pond_aX = IMUParams.measure_aX;
      IMUParams.pond_aY = IMUParams.measure_aY;
      IMUParams.pond_aZ = IMUParams.measure_aZ;
  
      IMUParams.pond_gX = IMUParams.measure_gX;
      IMUParams.pond_gY = IMUParams.measure_gY;
      IMUParams.pond_gZ = IMUParams.measure_gZ;
      return;
  }
  
  //Variables usadas por el filtro pasa bajos perduran en el tiempo
  static long f_aX, f_aY, f_aZ;
  static long f_gX, f_gY, f_gZ;

  int p_aX = 0; int p_aY = 0; int p_aZ = 0;
  int p_gX = 0; int p_gY = 0; int p_gZ = 0;;

  if (rst) {
    f_aX = 0;
    f_aY = 0;
    f_aZ = 0;
    f_gX = 0;
    f_gY = 0;
    f_gZ = 0;
  }

  for (int i = 0; i < samples; i++) { //filtro media movil ponderada 4 muestras

    sensorIMU.getAcceleration(&IMUParams.measure_aX, &IMUParams.measure_aY, &IMUParams.measure_aZ);
    sensorIMU.getRotation(&IMUParams.measure_gX, &IMUParams.measure_gY, &IMUParams.measure_gZ);
    //Serial.print("\ntest A: \t"); Serial.print(IMUParams.pond_aX); Serial.print("\t"); Serial.print(IMUParams.measure_aX); Serial.print("\tPREstatic-f_ax:\t"); Serial.print(f_aX);

    // Filtrar las lecturas
    f_aX = f_aX - (f_aX >> movil) + IMUParams.measure_aX;
    p_aX = f_aX >> movil;
    f_aY = f_aY - (f_aY >> movil) + IMUParams.measure_aY;
    p_aY = f_aY >> movil;
    f_aZ = f_aZ - (f_aZ >> movil) + IMUParams.measure_aZ;
    p_aZ = f_aZ >> movil;

    f_gX = f_gX - (f_gX >> movil) + IMUParams.measure_gX;
    p_gX = f_gX >> movil;
    f_gY = f_gY - (f_gY >> movil) + IMUParams.measure_gY;
    p_gY = f_gY >> movil;
    f_gZ = f_gZ - (f_gZ >> movil) + IMUParams.measure_gZ;
    p_gZ = f_gZ >> movil;
  }

  IMUParams.pond_aX = p_aX;
  IMUParams.pond_aY = p_aY;
  IMUParams.pond_aZ = p_aZ;

  IMUParams.pond_gX = p_gX;
  IMUParams.pond_gY = p_gY;
  IMUParams.pond_gZ = p_gZ;
}



/*//funcion para CALIBRAR la imu
   IMUParams estructura de valores de imu
   b_horizontal: determina si el dispositivo se encuentra en horizontal (tapa blanca hacia el cielo) o en vertical (conectores hacia el suelo)
   samples: numero de muestras que se desean tomar para cada iteración
   movil: cantidad de bits de desplazamiento para hacer media movil -> 2^movil datos
   iter: numero de veces que se calculará el offset
   b_recalibrate: si es true parte del último valor almacenado en la memoria nvs, si es false, parte del valor default de offset de la imu (constante -5712)
*/
void calibrateIMU(bool b_recalibrate = false, bool b_horizontal = true, uint8_t iter = 100, uint16_t samples = 200, uint8_t movil = 5) {
  int aX_offset, aY_offset, aZ_offset;
  int gX_offset, gY_offset, gZ_offset;

  IMUStruct IMUParams;
  myDelayMs(500);//damos tiempo a que se alimente
  sensorIMU.initialize(); //inicializamos IMU
  myDelayMs(10);//para dar tiempo al MPU6050 a iniciarse

  //variables para almacenar el valor de offset
  if (b_recalibrate) { //si se quiere hacer recalibrado a partir del offset del calibrado anterior
    Serial.println("RECALIBRATE");
    read_nvs_16(nvs_key_offset_aX, &nvs_val_offset_aX); //lee valores offset aceleraciones almacenados en flash
    aX_offset = nvs_val_offset_aX;
    read_nvs_16(nvs_key_offset_aY, &nvs_val_offset_aY);
    aY_offset = nvs_val_offset_aY;
    read_nvs_16(nvs_key_offset_aZ, &nvs_val_offset_aZ);
    aZ_offset = nvs_val_offset_aZ;

    read_nvs_16(nvs_key_offset_gX, &nvs_val_offset_gX);  //lee valores offset osciloscopio almacenados en flash
    gX_offset = nvs_val_offset_gX;
    read_nvs_16(nvs_key_offset_gY, &nvs_val_offset_gY);
    gY_offset = nvs_val_offset_gY;
    read_nvs_16(nvs_key_offset_gZ, &nvs_val_offset_gZ);
    gZ_offset = nvs_val_offset_gZ;
  }
  else { //si se quiere calibrar "de fabrica", leemos los valores constantes almacenados en la IMU
    Serial.println("FACTORY CALIBRATION");
    //leemos el valor del último offset para partir de este los offset base de la IMU son -5712  1903  1048  0 0 0
    aX_offset = sensorIMU.getXAccelOffset();//este valor NO se modifica en la memoria de la IMU, siempre es -5712
    aY_offset = sensorIMU.getYAccelOffset();
    aZ_offset = sensorIMU.getZAccelOffset();

    gX_offset = sensorIMU.getXGyroOffset();
    gY_offset =  sensorIMU.getYGyroOffset();
    gZ_offset = sensorIMU.getZGyroOffset();
  }

  Serial.print("valores previos de offset \t"); Serial.print(aX_offset); Serial.print("\t"); Serial.print(aY_offset); Serial.print("\t"); Serial.print(aZ_offset); Serial.print("\t");
  Serial.print(gX_offset); Serial.print("\t"); Serial.print(gY_offset); Serial.print("\t"); Serial.println(gZ_offset);

  const uint16_t gravity = 16384; //valor de la gravedad en raw para +-2g (porque initialize configura en +-2g +-250º/s)
  int16_t plusX, plusZ;

  if (b_horizontal) {
    plusX = 0;
    plusZ = - gravity;
  }
  else {
    plusX = - gravity;
    plusZ = 0;
  }

  for (uint8_t i = 0; i < iter; i++) {
    //en primer lugar tomamos "samples" lecturas filtradas , devuelve los valores ponderados en la estructura IMUParams
    getIMUFiltered(IMUParams, samples, movil);

    Serial.print(IMUParams.pond_aZ + plusZ); Serial.print("\t"); Serial.println(IMUParams.pond_aZ);

    //con el promedio de las medidas realizadas comprobamos si se cumplen las condiciones iniciales (horizontal o vertical) y si no cumple aumentamos o disminuimos el offset
    if (IMUParams.pond_aX + plusX > 0) aX_offset--;
    else aX_offset++;
    if (IMUParams.pond_aY   > 0) aY_offset--;
    else aY_offset++;
    if (IMUParams.pond_aZ + plusZ  > 0) aZ_offset--;
    else aZ_offset++;

    //Calibrar el giroscopio a 0º/s en todos los ejes (ajustar el offset)
    if (IMUParams.pond_gX > 0) gX_offset--;
    else gX_offset++;
    if (IMUParams.pond_gY > 0) gY_offset--;
    else gY_offset++;
    if (IMUParams.pond_gZ > 0) gZ_offset--;
    else gZ_offset++;

    //seteamos los nuevos offsets en el sensor
    Serial.print("[IMU-CAL] Offsets iter[" + String(i) + String("]:\t"));
    Serial.print(aX_offset); Serial.print("\t"); Serial.print(aY_offset); Serial.print("\t"); Serial.print(aZ_offset); Serial.print("\t\t");
    Serial.print(gX_offset); Serial.print("\t"); Serial.print(gY_offset); Serial.print("\t"); Serial.println(gZ_offset);
    sensorIMU.setXAccelOffset(aX_offset);
    sensorIMU.setYAccelOffset(aY_offset);
    sensorIMU.setZAccelOffset(aZ_offset);

    sensorIMU.setXGyroOffset(gX_offset);
    sensorIMU.setYGyroOffset(gY_offset);
    sensorIMU.setZGyroOffset(gZ_offset);

    /*
        //realizamos esta vez una medición reiniciando los valores statics para que no afecten ahora que tiene nuevos offsets
        getIMUFiltered(IMUParams, samples, movil, false);
        Serial.print("\tValues:\t\t");
        Serial.print("\t"); Serial.print(IMUParams.measure_aX); Serial.print("\t"); Serial.print(IMUParams.measure_aY); Serial.print("\t"); Serial.print(IMUParams.measure_aZ); Serial.print("\t\t");
        Serial.print(IMUParams.measure_gX); Serial.print("\t"); Serial.print(IMUParams.measure_gY); Serial.print("\t"); Serial.println(IMUParams.measure_gZ); Serial.println("");*/
  }

  //al terminar almacenamos los valores de los offsets en las variables NVS para que perduren
  nvs_val_offset_aX = aX_offset;
  nvs_val_offset_aY = aY_offset;
  nvs_val_offset_aZ = aZ_offset;
  write_nvs_16(nvs_key_offset_aX, nvs_val_offset_aX);
  write_nvs_16(nvs_key_offset_aY, nvs_val_offset_aY);
  write_nvs_16(nvs_key_offset_aZ, nvs_val_offset_aZ);

  nvs_val_offset_gX = gX_offset;
  nvs_val_offset_gY = gY_offset;
  nvs_val_offset_gZ = gZ_offset;
  write_nvs_16(nvs_key_offset_gX, nvs_val_offset_gX);
  write_nvs_16(nvs_key_offset_gY, nvs_val_offset_gY);
  write_nvs_16(nvs_key_offset_gZ, nvs_val_offset_gZ);

  read_nvs_16(nvs_key_offset_aX, &nvs_val_offset_aX); //lee valores offset aceleraciones almacenados en flash
  read_nvs_16(nvs_key_offset_aY, &nvs_val_offset_aY);
  read_nvs_16(nvs_key_offset_aZ, &nvs_val_offset_aZ);

  read_nvs_16(nvs_key_offset_gX, &nvs_val_offset_gX);  //lee valores offset osciloscopio almacenados en flash
  read_nvs_16(nvs_key_offset_gY, &nvs_val_offset_gY);
  read_nvs_16(nvs_key_offset_gZ, &nvs_val_offset_gZ);

  //Serial.print("valores almacenados:\t"); Serial.print(nvs_val_offset_aX); Serial.print("\t"); Serial.print(nvs_val_offset_aY); Serial.print("\t"); Serial.print(nvs_val_offset_aZ); Serial.print("\t");
  //Serial.print(nvs_val_offset_gX); Serial.print("\t"); Serial.print(nvs_val_offset_gY); Serial.print("\t"); Serial.println(nvs_val_offset_gZ);
}


/*
   Esta función carga todas las configuraciones de la IMU:
      offsets de aceleración X,Y,Z
      offsets de giroscopio X,Y,Z
      sensibilidad de aceletrometro
      sensibilidad de giroscopio
*/

bool configCalibrationIMU() { //carga la configuración almacenada en NVS en la IMU, retorna false si hay error, true si está correcto

  read_nvs_16(nvs_key_offset_aX, &nvs_val_offset_aX); //lee valores offset aceleraciones almacenados en flash
  read_nvs_16(nvs_key_offset_aY, &nvs_val_offset_aY);
  read_nvs_16(nvs_key_offset_aZ, &nvs_val_offset_aZ);

  read_nvs_16(nvs_key_offset_gX, &nvs_val_offset_gX);  //lee valores offset osciloscopio almacenados en flash
  read_nvs_16(nvs_key_offset_gY, &nvs_val_offset_gY);
  read_nvs_16(nvs_key_offset_gZ, &nvs_val_offset_gZ);

  read_nvs_16(nvs_key_sens_acel_IMU, &nvs_val_sens_acel_IMU);  //lee los valores de sensibilidad almacenados en flash
  read_nvs_16(nvs_key_sens_giro_IMU, &nvs_val_sens_giro_IMU);


  myDelayMs(500); //por si no esta ready IMU
  sensorIMU.initialize(); //inicializamos IMU
  myDelayMs(10);//para dar tiempo al MPU6050 a iniciarse

  if (sensorIMU.testConnection()) {
    Serial.println("IMU iniciada correctamente");
  }
  else {
    Serial.println("Error iniciando IMU!");
    return false;
  }



  sensorIMU.setXAccelOffset(nvs_val_offset_aX);
  sensorIMU.setYAccelOffset(nvs_val_offset_aY);
  sensorIMU.setZAccelOffset(nvs_val_offset_aZ);

  sensorIMU.setXGyroOffset(nvs_val_offset_gX);
  sensorIMU.setYGyroOffset(nvs_val_offset_gZ);
  sensorIMU.setZGyroOffset(nvs_val_offset_gZ);

  //pasamos a los rangos de aceleración y gyroscopio decididos por el usuario
  sensorIMU.setFullScaleAccelRange(nvs_val_sens_acel_IMU);
  sensorIMU.setFullScaleGyroRange(nvs_val_sens_giro_IMU);

  return true;
}

/*
   esta función calcula a partir de los valores almacenados en NVS el valor real en gs y º/s de la medida raw obtenida

*/
void rawToMeasure_IMU(float v_measures[]) { //{0:accelX , 1:accelY , 2:accelZ , 3:gyroX , 4:gyroY , 5:gyroZ , 6:temp}
  //valores de sensibilidad
  const uint16_t v_sens_acel_IMU[] = {16384, 8192, 4096, 2048}; //0:+-2g ,1:+-4g ,2:+-8g ,3:+-16g
  const float v_sens_giro_IMU[] = {131, 65.5, 32.8, 16.4};      //0:+-250º/s ,1:+-500º/s ,2:+-1000º/s ,3:+-2000º/s

  //lecturas de variables de sensibilidad NVS
  read_nvs_16(nvs_key_sens_acel_IMU, &nvs_val_sens_acel_IMU, false);  //lee los valores de sensibilidad almacenados en flash
  read_nvs_16(nvs_key_sens_giro_IMU, &nvs_val_sens_giro_IMU, false);

  for (uint8_t i = 0; i < 3; i++) { //transformación de aceleraciones
    v_measures[i] = (float)v_measures[i] * ((float)9.81 / (float) v_sens_acel_IMU[nvs_val_sens_acel_IMU]); //1g -> X LSB
  }

  for (uint8_t i = 3; i < 6; i++) { //transformación de grados
    v_measures[i] = float(v_measures[i]) * (float(1.0) / v_sens_giro_IMU[nvs_val_sens_giro_IMU]); //1º/s -> X LSB
  }

  v_measures[6] = ((float)v_measures[6] / 360.0) + 36.53;
}


//______________________________________________________________SD_________________________________________________________________

/*A esta función se le pasa un nombre de directorio y un nombre de archivo. Comprueba si el directorio existe y si no lo crea
   devuelve 0 si no lo crea y no existe
   devuelve 1 si lo crea
   devuelve 2 si estaba creado
*/
uint8_t checkDirectoryThenCreate( char* nameMainDirectory) {
  if (!SD.exists(nameMainDirectory)) { //comprobamos si existe el directorio
    if (!SD.mkdir(nameMainDirectory)) { //si no existe lo creamos
      Serial.println("Error mkdir:" + String(nameMainDirectory));
      return 0;
    }
    return 1;
  }
  return 2;
}
