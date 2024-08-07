uint8_t err_sens = 0;

uint8_t indx_v_CO2 = 0; //indice de sensor SGP30 (CO2)
uint8_t indx_v_TVOC = 0; //indice de sensor SGP30 (TVOC)
uint8_t indx_v_TEMP = 0; //indice de sensor SHT30 (Temp)
uint8_t indx_v_HUM = 0; //indice de sensor SHT30 (Hum)
float v_measCO2[NUM_MEASURES_V];
float v_measTVOC[NUM_MEASURES_V];
float v_measTemp[NUM_MEASURES_V];
float v_measHum[NUM_MEASURES_V];

bool f_newFreq = false;


void enableI2C()
{
  pinMode(SCL, INPUT_PULLUP);
  pinMode(SDA, INPUT_PULLUP);
}




#ifdef AIR_QUALITY_4
bool startSGP30() {
  if (sensorAirQuality.begin() == false) {
    Serial.println(F("SGP30 not detected"));
    return false;
  }
  else {
    sensorAirQuality.initAirQuality();
    Serial.println(F("SGP30 Connected"));
  }
  return true;
}

char measureSGP30() {
  int measCO2 = -1;
  int measTVOC = -1;
  char stat = 'D';

  while (millis() - lastMeasure < 1000); //espera que haya pasado al menos 1 seg desde la última medición

  if (sensorAirQuality.measureAirQuality() == SGP30_SUCCESS) {
    lastMeasure = millis();
    measCO2 = sensorAirQuality.CO2;
    measTVOC = sensorAirQuality.TVOC;
  }
  else {
    Serial.println(F("Error al comunicarse con SGP30"));
    measCO2 = -2;
    measTVOC = -2;
    stat = 'D';
  }
#ifdef EN_DEBUG
  Serial.print("CO2: ");
  Serial.print(measCO2);
  Serial.print("ppm  ,  TVOC: ");
  Serial.print(measTVOC);
  Serial.print("ppb\t|\t");
#endif

  //VALORES BASADOS EN //https://wiki.inbiot.es/covs/#:~:text=VERDE%20%3C%20220%20ppb,ROJO%20%3E%20660%20ppb


  if (((measCO2 >= 400) && (measCO2 < 850)) && ((measTVOC >= 0) && (measTVOC < 600))) {
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_B, LOW);
    Serial.println(F("GOOD_Q"));
    digitalWrite(LED_G, HIGH);
    stat = 'G';
  }
  if (((measCO2 >= 850) && (measCO2 < 1500)) || ((measTVOC >= 600) && (measTVOC < 1000))) {
    digitalWrite(LED_R, LOW);
    Serial.println(F("MID_Q"));
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_R, HIGH);
    stat = 'M';
  }
  if (((measCO2 >= 1500)) || (measTVOC >= 1000)) {
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, LOW);
    Serial.println(F("BAD_Q/ERROR"));
    digitalWrite(LED_R, HIGH);
    stat = 'B';
  }

  //si se ha tomado valores del sensor correctamente los almacena el en vector
  if ( (measCO2 >= 400)) { //valor mínimo que da el sensor para CO2 = 400ppm
    v_measCO2[indx_v_CO2] = measCO2;
  }
  else if (indx_v_CO2 > 0) { //en el caso de que la lectura no sea correcta y el índice es mayor que 0 guarda el valor anterior en su lugar
    v_measCO2[indx_v_CO2] =  v_measCO2[indx_v_CO2 - 1];
  }
  else if (indx_v_CO2 == 0) //si no se ha tomado medición y el índice es 0 guarda el valor anterior del vector circular en su lugar
  {
    v_measCO2[indx_v_CO2] =  v_measCO2[NUM_MEASURES_V - 1];
  }
  
  indx_v_CO2++;

  if (measTVOC >= 0) { //valor mínimo que da el sensor para TVOC = 0ppb
    v_measTVOC[indx_v_TVOC] = measTVOC;
  }
  else if (indx_v_TVOC > 0) { //en el caso de que la lectura no sea correcta y el índice es mayor que 0 guarda el valor anterior en su lugar
    v_measTVOC[indx_v_TVOC] =  v_measTVOC[indx_v_TVOC - 1];
  }
  else if (indx_v_TVOC == 0) //si no se ha tomado medición y el índice es 0 guarda el valor anterior del vector circular en su lugar
  {
    v_measTVOC[indx_v_TVOC] =  v_measTVOC[NUM_MEASURES_V - 1];
  }

  indx_v_TVOC++;
  /*Serial.println(F("Vector:"));
    for (int idx_print = 0; idx_print < NUM_MEASURES_V; idx_print++) {
    Serial.print(v_measCO2[idx_print]);
    Serial.print(",");
    }
    Serial.println();
    for (int idx_print = 0; idx_print < NUM_MEASURES_V; idx_print++) {
    Serial.print(v_measTVOC[idx_print]);
    Serial.print(",");
    }
    Serial.println();*/


  float sumatorioCO2 = 0.0;
  float sumatorioTVOC = 0.0;
  float mediaCO2 = 0.0;
  float mediaTVOC = 0.0;

  for (int idx_m = 0; idx_m < NUM_MEASURES_V; idx_m++) {
    sumatorioCO2 += v_measCO2[idx_m];
    sumatorioTVOC += v_measTVOC[idx_m];
  }
  mediaCO2 = float(sumatorioCO2) / float(NUM_MEASURES_V);
  mediaTVOC = float(sumatorioTVOC) / float(NUM_MEASURES_V);

  /*Serial.println("MEDIAS:");
    Serial.print(mediaCO2);
    Serial.println(mediaTVOC);*/

  if (indx_v_CO2 > NUM_MEASURES_V-1) { //Vuelve a iniciar el indice del vector
    indx_v_CO2 = 0;
  }

  if (indx_v_TVOC > NUM_MEASURES_V-1) { //Vuelve a iniciar el indice del vector y en caso de encontrarse en el inicio del último vector antes del envío activa el ventilador
    indx_v_TVOC = 0;
    digitalWrite(PIN_FAN, LOW);
  }
  if (indx_v_TVOC >= NUM_MEASURES_V - NUM_MEASURES_FAN) { // en caso de encontrarse en los últimos NUM_MEASURES_FAN valores (NUM_MEASURES_FAN segundos) de medición en el vector de datos enciende el ventilador
    digitalWrite(PIN_FAN, HIGH);
  }

  if (int(round(mediaCO2)) >= CHANGE_FREQ_CO2_UMBRAL) {
    f_newFreq = true;
  }
  else {
    f_newFreq = false;
  }

  if (f_firstTime) {
    mediaCO2 = 0;
    mediaTVOC = 0;
  }


  s_ValJSON[takeParam("CO2")] = int(round(mediaCO2));
  s_ValJSON[takeParam("VOC")] = int(round(mediaTVOC));






  return stat;
}

#endif





#ifdef SHT30
#include "Adafruit_SHT31.h"
bool timeToHeat = true;  //controla cuando debe encenderse el calentador

Adafruit_SHT31 sht30 = Adafruit_SHT31();

bool startSHT30() {
  if (!sht30.begin(0x44)) {
    Serial.println(F("SHT30 not detected"));
    return false;
  }
  else {
    Serial.println(F("SHT30 Connected"));
    return true;
  }
}

bool checkMeasure(float measVal) {
  if (! isnan(measVal)) {  // check if 'is not a number'
    return true;
  } else {
    Serial.println(F("Failed to read value"));
    return false;
  }
}

void measureSHT30() {
  float measTemp = -100.0;
  float measHum = -100.0;

  //después de un heaterEnable espera 90seg para tomar la siguiente medida  (30 de TIME_HEAT y 60 de TIME_COLD)
  //se pone antes del encendido del heater para que este no le afecte en la primera medida y no altere la temperatura

  if ((millis() - lastHeatTime >= (TIME_HEAT + TIME_COLD) * 1000) || f_firstTime) {
    do {
      Serial.println(F("R_TEM"));
      measTemp = sht30.readTemperature();
      cnt_retry++;
    } while ((!checkMeasure(measTemp) && (cnt_retry < MAX_ERROR)));
    cnt_retry = 0;
    do {
      Serial.println(F("R_HUM"));
      measHum = sht30.readHumidity();
      cnt_retry++;
    } while (((!checkMeasure(measHum)  && (cnt_retry < MAX_ERROR))));
    cnt_retry = 0;

    if (f_firstTime) {//si es la primera medida rellena el vector completo con la primera medida
      initVector(v_measTemp, measTemp);
      initVector(v_measHum, measHum);
    }
  }

  if (timeToHeat) { //si es tiempo de calentamiento habilita el heater, cambia el flag a false e inicia su contador
    Serial.println(F("EN_HEATER 30s"));
    sht30.heater(true);
    timeToHeat = false;
    lastHeatTime = millis();
  }
  else {
    if ((sht30.isHeaterEnabled() && (millis() - lastHeatTime >= TIME_HEAT * 1000))) { //se activa cuando esté activo el calentador y hayan pasado al menos 30 seg desde que se activó
      Serial.println(F("DIS_HEAT"));
      sht30.heater(false);
      cnt_retry++;
    }
  }

  if (millis() - lastHeatTime >= (TIME_HEAT + TIME_COLD) * 1000) {
    measTemp = measTemp - 1.7; //CORRECCION DE OFFSET por caja

#ifdef EN_DEBUG
    if (measTemp > -100.0) {
      Serial.print(F("Temp: "));
      Serial.print(measTemp);
    }
    if (measHum > -100.0) {
      Serial.print(F("\t|\tHum: "));
      Serial.println(measHum);
    }
#endif

    if (measTemp > -100.0) { //si se ha tomado medición de temperatura la guarda
      v_measTemp[indx_v_TEMP] = measTemp;
    }
    else if (indx_v_TEMP > 0) //si no se ha tomado medición de temperatura y el índice es mayor que 0 guarda el valor anterior en su lugar
    {
      v_measTemp[indx_v_TEMP] =  v_measTemp[indx_v_TEMP - 1];
    }
    else if (indx_v_TEMP == 0) //si no se ha tomado medición de temperatura y el índice es 0 guarda el valor anterior del vector circular en su lugar
    {
      v_measTemp[indx_v_TEMP] =  v_measTemp[NUM_MEASURES_V - 1];
    }

    indx_v_TEMP++;

    if (measHum > -100.0) { //si se ha tomado medición de humedad la guarda
      v_measHum[indx_v_HUM] = measHum;
    }
    else if (indx_v_HUM > 0)//si no se ha tomado medición de humedad y el índice es mayor que 0 guarda el valor anterior en su lugar
    {
      v_measHum[indx_v_HUM] =  v_measHum[indx_v_HUM - 1];
    }
    else if (indx_v_HUM == 0)//si no se ha tomado medición de humedad y el índice es 0 guarda el valor anterior del vector circular en su lugar
    {
      v_measHum[indx_v_HUM] =  v_measHum[NUM_MEASURES_V - 1];
    }

    indx_v_HUM++;

    /*Serial.println(F("Vector: "));
      for (int idx_print = 0; idx_print < NUM_MEASURES_V; idx_print++) {
      Serial.print(v_measTemp[idx_print]);
      Serial.print(", ");
      }
      Serial.println();

      for (int idx_print = 0; idx_print < NUM_MEASURES_V; idx_print++) {
      Serial.print(v_measHum[idx_print]);
      Serial.print(", ");
      }
      Serial.println();*/


    float sumatorioTemp = 0.0;
    float sumatorioHum = 0.0;
    float mediaTemp = 0.0;
    float mediaHum = 0.0;

    for (int idx_m = 0; idx_m < NUM_MEASURES_V; idx_m++) {
      sumatorioTemp += v_measTemp[idx_m];
      sumatorioHum += v_measHum[idx_m];
    }
    mediaTemp = float(sumatorioTemp) / float(NUM_MEASURES_V);
    mediaHum = float(sumatorioHum) / float(NUM_MEASURES_V);

    /*Serial.println("MEDIAS:");
      Serial.print(mediaTemp);
      Serial.println(mediaHum);*/
      
    if (indx_v_TEMP > NUM_MEASURES_V-1) {
      indx_v_TEMP = 0;
    }
    if (indx_v_HUM > NUM_MEASURES_V-1) {
      indx_v_HUM = 0;
    }

    s_ValJSON[takeParam("T")] = mediaTemp;
    s_ValJSON[takeParam("H")] = mediaHum;
  }

  //activamos el heater cada X min para mejorar toma de dato de humedad relativa
  if (((millis() - lastHeatTime) >= (INTERVAL_HEATER * 60000))) { // || (measHum != 0 && (measHum >= 90 || measHum <= 10))) {
    timeToHeat = true;
    Serial.println(F("TimeToHeat"));
  }

  if (f_firstTime) {
    s_ValJSON[takeParam("T")] = measTemp;
    s_ValJSON[takeParam("H")] = measHum;
  }
}
#endif
