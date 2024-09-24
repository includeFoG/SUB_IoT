#define LANGUAGE 'SP'
#define ELEVATION (11)             //Enter your elevation in m ASL to calculate rel pressure (ASL/QNH) at your place
//#define FILE_WRITE (O_WRITE | O_READ | O_CREAT)


#include "Translation.h"
#include <TimeLib.h>
String dataFile = "forecastData.txt";
double temperature, pressure;
double humidity;
float altitude, VIn ;
float adjusted_temp, adjusted_humi, HeatIndex;
int rel_pressure_rounded;
double DewpointTemperature;
float DewPointSpread;               // Difference between actual temperature and dewpoint
float SLpressure_hPa;               // needed for rel pressure calculation
float UV;

// FORECAST CALCULATION
unsigned long saved_timestamp;      // Timestamp stored in SD

float pressure_difference[12];      // Array to calculate trend with pressure differences
float pressure_value[12];           // Array for the historical pressure values (6 hours, all 30 mins)
// where as pressure_value[0] is always the most recent value

// FORECAST RESULT
int accuracy_;                       // Counter, if enough values for accurate forecasting
int accuracy_in_percent;
String ZambrettisWords;             // Final statement about weather forecast
String trend_in_words;              // Trend in words
String forecast_in_words;           // Weather forecast in words
String pressure_in_words;           // Air pressure in words
String accuracy_in_words;           // Zambretti's prediction accuracy in words
String PredEnvio; //texto a enviar : zambrettiswords +" Presión: "+trend_in_words

// tab file for functions used by powerSaverPressureAndTemperatureAndHumidity
// BME280 configuration settings
const byte osrs_t = 2; // setting for temperature oversampling
// No. of samples = 2 ^ (osrs_t-1) or 0 for osrs_t==0
const byte osrs_p = 5; // setting for pressure oversampling
// No. of samples = 2 ^ (osrs_p-1) or 0 for osrs_p==0
const byte osrs_h = 5; // setting for humidity oversampling
// No. of samples = 2 ^ (osrs_h-1) or 0 for osrs_h==0

// configuration constants
const bool bme280Debug = 1; // controls serial printing
// set to 1 to enable printing of BME280 or BMP280 transactions
// configure period between reports

const long measurementInterval = 20000;  // measurement interval in ms


float TEMP_CORR = 0;              //Manual correction of temp sensor (mine reads 1 degree too high)





String printTime() {
  current_timestamp = rtc.getEpoch();
  Serial.print("Current UNIX Timestamp: ");
  Serial.println(current_timestamp);

  //  Serial.print("Time & Date: ");
  //  Serial.print(hour(current_timestamp));
  //  Serial.print(":");
  //  Serial.print(minute(current_timestamp));
  //  Serial.print(":");
  //  Serial.print(second(current_timestamp));
  //  Serial.print("; ");
  //  Serial.print(day(current_timestamp));
  //  Serial.print(".");
  //  Serial.print(month(current_timestamp));
  //  Serial.print(".");
  //  Serial.println(year(current_timestamp));
  return (String(day(current_timestamp)) + "-" + String(month(current_timestamp)) + "-" +
          String(year(current_timestamp)) + "; " + String(hour(current_timestamp)) + ":" +
          String(minute(current_timestamp)) + ":" + String(second(current_timestamp)));

}

void initRTC(String fechaHora) {

  const byte year = ((fechaHora.substring(2, 4)).toInt());
  const byte month = ((fechaHora.substring(5, 7)).toInt());
  const byte day = ((fechaHora.substring(8, 10)).toInt());

  const byte hours = ((fechaHora.substring(11, 13)).toInt());
  const byte minutes = ((fechaHora.substring(14, 16)).toInt());
  const byte seconds = ((fechaHora.substring(17, 19)).toInt());

  Serial.println("InitRTC:");
  Serial.print (String(year) + "/");
  Serial.print (String(month) + "/");
  Serial.print (String(day) + " ");
  Serial.print (String(hours) + ":");
  Serial.print (String(minutes) + ":");
  Serial.println (String(seconds));

  rtc.begin();
  delay(50);
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);


  printTime();

}


void init_forecast() {
  placa.RecStatus();
  placa.EnableConmutado3v();

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin();
  delay(10);

  if (SD.begin(SD_CS)) {
    if (!SD.exists(dataFile.c_str())) {
      Serial.println("File forecast data does not exist");
    }
    else {
      Serial.println("Clearing forecast Data");
      SD.remove(dataFile.c_str());
      if (!SD.exists(dataFile.c_str())) Serial.println("File removed");
      else Serial.println("Fail removing file");
    }
  }
  else {
    Serial.println(F("SD Card Error!")); delay(1000);
    err_code += "| x4000 |";
    _error = true;
  }
  SPI.end();
  pinMode(SD_CS, INPUT_PULLDOWN);
  placa.SetPrevStatus();
}


void ReadFromSD();
void WriteToSD(int write_timestamp);
void FirstTimeRun();

/////SD functions//////
void funcionesSD() {
  ReadFromSD();              //read stored values and update data if more recent data is available
  Serial.print("09.1.- Timestamp difference: "); Serial.println(current_timestamp - saved_timestamp);
  if (current_timestamp - saved_timestamp > 7200) {   //Si el ultimo save es anterior a 2h ->7200sec reinicia a firstimerun
    //if (current_timestamp - saved_timestamp > 120){    //Si el ultimo save es anterior a 2h ->7200sec reinicia a firstimerun
    FirstTimeRun();
  }
  else if (current_timestamp - saved_timestamp > 1800) { // it is time for pressure update (1800 sec = 30 min)  //!!!!
    Serial.println("Actualizando valor de P para Forecast");
    for (int i = 11; i >= 1; i = i - 1) {
      pressure_value[i] = pressure_value[i - 1];        // shifting values one to the right
    }

    pressure_value[0] = rel_pressure_rounded;             // updating with acutal rel pressure (newest value)
    if (accuracy_ < 12) {
      accuracy_ = accuracy_ + 1;                            // one value more -> accuracy rises (up to 12 = 100%)
    }
    WriteToSD(current_timestamp);                   // update timestamp on storage
  }
  else {
    WriteToSD(saved_timestamp);                     // do not update timestamp on storage
  }
}






int CalculateTrend() {
  int trend;                                    // -1 falling; 0 steady; 1 raising
  Serial.println("12.1.- ---> Calculating trend");

  //--> giving the most recent pressure reads more weight
  pressure_difference[0] = (pressure_value[0] - pressure_value[1])   * 1.5;
  pressure_difference[1] = (pressure_value[0] - pressure_value[2]);
  pressure_difference[2] = (pressure_value[0] - pressure_value[3])   / 1.5;
  pressure_difference[3] = (pressure_value[0] - pressure_value[4])   / 2;
  pressure_difference[4] = (pressure_value[0] - pressure_value[5])   / 2.5;
  pressure_difference[5] = (pressure_value[0] - pressure_value[6])   / 3;
  pressure_difference[6] = (pressure_value[0] - pressure_value[7])   / 3.5;
  pressure_difference[7] = (pressure_value[0] - pressure_value[8])   / 4;
  pressure_difference[8] = (pressure_value[0] - pressure_value[9])   / 4.5;
  pressure_difference[9] = (pressure_value[0] - pressure_value[10])  / 5;
  pressure_difference[10] = (pressure_value[0] - pressure_value[11]) / 5.5;

  //--> calculating the average and storing it into [11]
  pressure_difference[11] = (  pressure_difference[0]
                               + pressure_difference[1]
                               + pressure_difference[2]
                               + pressure_difference[3]
                               + pressure_difference[4]
                               + pressure_difference[5]
                               + pressure_difference[6]
                               + pressure_difference[7]
                               + pressure_difference[8]
                               + pressure_difference[9]
                               + pressure_difference[10]) / 11;

  Serial.print("12.1.1- Current trend: ");
  Serial.println(pressure_difference[11]);

  if      (pressure_difference[11] > 3.75) {   //original 3.5 ajuste 3.75
    trend_in_words = TEXT_RISING_FAST;
    trend = 1;
  }
  else if (pressure_difference[11] > 1.75 && pressure_difference[11] <= 4)  {//original 1.5 ajuste 1.75
    trend_in_words = TEXT_RISING;
    trend = 1;
  }
  else if (pressure_difference[11] > 0.5  && pressure_difference[11] <= 2)  {//original 0.25 ajuste 0.5
    trend_in_words = TEXT_RISING_SLOW;
    trend = 1;
  }
  else if (pressure_difference[11] > -0.5 && pressure_difference[11] < 0.75)  {//original -0.25 /0.5 ajuste -0.75 /0.75
    trend_in_words = TEXT_STEADY;
    trend = 0;
  }
  else if (pressure_difference[11] >= -1.75 && pressure_difference[11] < -0.5) {//original -1.5/-0.25 ajuste -1.75 /-0.5
    trend_in_words = TEXT_FALLING_SLOW;
    trend = -1;
  }
  else if (pressure_difference[11] >= -3.75 && pressure_difference[11] < -1.75)  {//original -3.5/-1.5 ajuste -3.75/-1.75
    trend_in_words = TEXT_FALLING;
    trend = -1;
  }
  else if (pressure_difference[11] <= -3.75) { //original -3.5 ajuste 3.75
    trend_in_words = TEXT_FALLING_FAST;
    trend = -1;
  }

  Serial.println("12.1.2.- " + trend_in_words);
  return trend;
}


char ZambrettiLetter() {
  Serial.println("12.- ---> Calculating Zambretti letter");
  char z_letter;
  int(z_trend) = CalculateTrend();
  // Case trend is falling
  if (z_trend == -1) {
    float zambretti = 0.0009746 * rel_pressure_rounded * rel_pressure_rounded - 2.1068 * rel_pressure_rounded + 1138.7019;
    //A Winter falling generally results in a Z value lower by 1 unit
    if (month(current_timestamp) < 4 || month(current_timestamp) > 9) zambretti = zambretti + 1;
    if (zambretti > 9) zambretti = 9;
    Serial.print("12.2.(1)- Calculated and rounded Zambretti in numbers: ");
    Serial.println(round(zambretti));
    switch (int(round(zambretti))) {
      case 0:  z_letter = 'A'; break;       //Settled Fine
      case 1:  z_letter = 'A'; break;       //Settled Fine
      case 2:  z_letter = 'B'; break;       //Fine Weather
      case 3:  z_letter = 'D'; break;       //Fine Becoming Less Settled
      case 4:  z_letter = 'H'; break;       //Fairly Fine Showers Later
      case 5:  z_letter = 'O'; break;       //Showery Becoming unsettled
      case 6:  z_letter = 'R'; break;       //Unsettled, Rain later
      case 7:  z_letter = 'U'; break;       //Rain at times, worse later
      case 8:  z_letter = 'V'; break;       //Rain at times, becoming very unsettled
      case 9:  z_letter = 'X'; break;       //Very Unsettled, Rain
    }
  }
  // Case trend is steady
  if (z_trend == 0) {
    float zambretti = 138.24 - 0.133 * rel_pressure_rounded;
    Serial.print("12.2.(2)- Calculated and rounded Zambretti in numbers: ");
    Serial.println(round(zambretti));
    switch (int(round(zambretti))) {
      case 0:  z_letter = 'A'; break;       //Settled Fine
      case 1:  z_letter = 'A'; break;       //Settled Fine
      case 2:  z_letter = 'B'; break;       //Fine Weather
      case 3:  z_letter = 'B'; break;       //Antes (E)= Fine, Possibly showers
      case 4:  z_letter = 'E'; break;       //Antes (K)= Fairly Fine, Showers likely
      case 5:  z_letter = 'K'; break;       //Antes (N)= Showery Bright Intervals
      case 6:  z_letter = 'N'; break;       //Antes (P)= Changeable some rain
      case 7:  z_letter = 'P'; break;       //Antes (S)= Unsettled, rain at times
      case 8:  z_letter = 'W'; break;       //Antes (W)= Rain at Frequent Intervals
      case 9:  z_letter = 'X'; break;       //Antes (X)= Very Unsettled, Rain
      case 10: z_letter = 'Z'; break;       //Antes (Z)= Stormy, much rain
    }
  }
  // Case trend is rising
  if (z_trend == 1) {
    float zambretti = 142.57 - 0.1376 * rel_pressure_rounded;
    //A Summer rising, improves the prospects by 1 unit over a Winter rising
    if (month(current_timestamp) >= 4 && month(current_timestamp) <= 9) zambretti = zambretti - 1;
    if (zambretti < 0) zambretti = 0;
    Serial.print("12.2.(3)- Calculated and rounded Zambretti in numbers: ");
    Serial.println(round(zambretti));
    switch (int(round(zambretti))) {
      case 0:  z_letter = 'A'; break;       //Settled Fine
      case 1:  z_letter = 'A'; break;       //Settled Fine
      case 2:  z_letter = 'B'; break;       //Fine Weather
      case 3:  z_letter = 'C'; break;       //Becoming Fine
      case 4:  z_letter = 'F'; break;       //Fairly Fine, Improving
      case 5:  z_letter = 'G'; break;       //Fairly Fine, Possibly showers, early
      case 6:  z_letter = 'I'; break;       //Showery Early, Improving
      case 7:  z_letter = 'J'; break;       //Changeable, Improving
      case 8:  z_letter = 'L'; break;       //Rather Unsettled Clearing Later
      case 9:  z_letter = 'M'; break;       //Unsettled, Probably Improving
      case 10: z_letter = 'Q'; break;       //Unsettled, short fine Intervals
      case 11: z_letter = 'T'; break;       //Very Unsettled, Finer at times
      case 12: z_letter = 'Y'; break;       //Stormy, possibly improving
      case 13: z_letter = 'Z'; break;;      //Stormy, much rain
    }
  }
  Serial.print("12.2.2.- This is Zambretti's famous letter: ");
  Serial.println(z_letter);
  return z_letter;
}


String ZambrettiSays(char code) {
  String zambrettis_words = "";
  switch (code) {
    case 'A': zambrettis_words = TEXT_ZAMBRETTI_A; break;  //see Tranlation.h
    case 'B': zambrettis_words = TEXT_ZAMBRETTI_B; break;
    case 'C': zambrettis_words = TEXT_ZAMBRETTI_C; break;
    case 'D': zambrettis_words = TEXT_ZAMBRETTI_D; break;
    case 'E': zambrettis_words = TEXT_ZAMBRETTI_E; break;
    case 'F': zambrettis_words = TEXT_ZAMBRETTI_F; break;
    case 'G': zambrettis_words = TEXT_ZAMBRETTI_G; break;
    case 'H': zambrettis_words = TEXT_ZAMBRETTI_H; break;
    case 'I': zambrettis_words = TEXT_ZAMBRETTI_I; break;
    case 'J': zambrettis_words = TEXT_ZAMBRETTI_J; break;
    case 'K': zambrettis_words = TEXT_ZAMBRETTI_K; break;
    case 'L': zambrettis_words = TEXT_ZAMBRETTI_L; break;
    case 'M': zambrettis_words = TEXT_ZAMBRETTI_M; break;
    case 'N': zambrettis_words = TEXT_ZAMBRETTI_N; break;
    case 'O': zambrettis_words = TEXT_ZAMBRETTI_O; break;
    case 'P': zambrettis_words = TEXT_ZAMBRETTI_P; break;
    case 'Q': zambrettis_words = TEXT_ZAMBRETTI_Q; break;
    case 'R': zambrettis_words = TEXT_ZAMBRETTI_R; break;
    case 'S': zambrettis_words = TEXT_ZAMBRETTI_S; break;
    case 'T': zambrettis_words = TEXT_ZAMBRETTI_T; break;
    case 'U': zambrettis_words = TEXT_ZAMBRETTI_U; break;
    case 'V': zambrettis_words = TEXT_ZAMBRETTI_V; break;
    case 'W': zambrettis_words = TEXT_ZAMBRETTI_W; break;
    case 'X': zambrettis_words = TEXT_ZAMBRETTI_X; break;
    case 'Y': zambrettis_words = TEXT_ZAMBRETTI_Y; break;
    case 'Z': zambrettis_words = TEXT_ZAMBRETTI_Z; break;
    case '0': zambrettis_words = TEXT_ZAMBRETTI_0; break;
    default: zambrettis_words = TEXT_ZAMBRETTI_DEFAULT; break;
  }
  return zambrettis_words;
}




void ReadFromSD() {

  if (!SD.exists(dataFile.c_str())) {
    Serial.println("File forecast data does not exist -> Going to F.T.R");
    FirstTimeRun();
  }

  Serial.println("08.- Reading memory");

  File myDataFile = SD.open(dataFile.c_str(), FILE_READ);       // Open file for reading
  if (!myDataFile) {
    Serial.println("08.000.- Error abriendo el archivo para lectura: " + String(dataFile));
    err_code += "| x4101 |";
    _error = true;

    return;
  }

  Serial.println("08.1.- ---> Now reading from SD");

  String temp_data;

  temp_data = myDataFile.readStringUntil('\n');
  saved_timestamp = temp_data.toInt();
  Serial.print("08.2.- Timestamp from SD: ");  Serial.println(saved_timestamp);

  temp_data = myDataFile.readStringUntil('\n');
  Serial.println(" mydatafile.readstringuntil result: " + temp_data);
  accuracy_ = temp_data.toInt();
  Serial.print("08.3.- Accuracy value read from SD: ");  Serial.println(accuracy_);

  Serial.print("Last 12 saved pressure values: ");
  for (int i = 0; i <= 11; i++) {
    temp_data = myDataFile.readStringUntil('\n');
    pressure_value[i] = temp_data.toInt();
    Serial.print(pressure_value[i]);
    Serial.print("; ");
  }
  myDataFile.close();
  Serial.println();
}

void WriteToSD(int write_timestamp) {
  Serial.println("11.- Writting in memory");
  //char filename [] = dataFile.c_str();

  File myDataFile = SD.open(dataFile.c_str(), FILE_WRITE);        // Open file for writing (appending)
  if (!myDataFile) {
    Serial.println("Error abriendo el archivo para escritura: " + String(dataFile));
    err_code += "| x4102 |";
    _error = true;
    return;
  }

  myDataFile.seek(0); //posiciona en el punto 0


  Serial.println("11.1.- ---> Now writing to SD");

  myDataFile.println(write_timestamp);                 // Saving timestamp
  myDataFile.println(accuracy_);                        // Saving accuracy value

  for ( int i = 0; i <= 11; i++) {
    myDataFile.println(pressure_value[i]);             // Filling pressure array with updated values
  }
  myDataFile.close();

  Serial.println("11.2.- File written. Now reading file again.");
  myDataFile = SD.open(dataFile.c_str(), FILE_READ);
  if (!myDataFile) {
    Serial.println("Error abriendo el archivo para lectura: " + String(dataFile));
    err_code += "| x4101 |";
    _error = true;

    return;
  }
  Serial.print("11.3.- Found in data.txt = ");
  while (myDataFile.available()) {
    Serial.print(myDataFile.readStringUntil('\n'));
    Serial.print("; ");
  }
  Serial.println();
  myDataFile.close();
}


void FirstTimeRun() {
  Serial.println("10.- ---> Starting initializing process FirstTimeRun FORECAST.");
  accuracy_ = 1;

  File myDataFile = SD.open(dataFile.c_str(), FILE_WRITE);
  if (!myDataFile) {
    Serial.println("Error abriendo el archivo para escritura: " + String(dataFile));
    err_code += "| x4102 |";
    _error = true;

    return;
  }

  myDataFile.seek(0);

  Serial.println(current_timestamp);
  myDataFile.println(current_timestamp);                   // Saving timestamp
  myDataFile.println(accuracy_);                            // Saving accuracy value

  for ( int i = 0; i < 12; i++) {
    myDataFile.println(rel_pressure_rounded);              // Filling pressure array with current pressure
  }

  Serial.println("10.1.- ** Saved initial pressure data. **");
  myDataFile.close();
  //resetFunc();                                              //call reset
}
void prediccion() {
  accuracy_in_percent = accuracy_ * 94 / 12;        // 94% is the max predicion accuracy of Zambretti
  // if ( VIn > 3.4 ) {                                  // check if batt is still ok
  ZambrettisWords = ZambrettiSays(char(ZambrettiLetter()));
  forecast_in_words = TEXT_ZAMBRETTI_FORECAST;
  pressure_in_words = TEXT_AIR_PRESSURE;
  accuracy_in_words = TEXT_ZAMBRETTI_ACCURACY;
  //   }
  // else {
  //   ZambrettisWords = ZambrettiSays('0');              // send Message that battery is empty
  // }

#ifdef JSON_METEOFCAST
  s_ValJSON[takeParam("Prcn")] = accuracy_in_percent;
#endif

  Serial.println("********************************************************");
  Serial.println("13.- ////////DATOS OBTENIDOS////////");
  Serial.print("13.1.- " + forecast_in_words); // "Pronostico:"
  Serial.print(": ");
  Serial.println(ZambrettisWords);  // estado del clima ej: "Buen clima"
  Serial.print("13.2.- " + pressure_in_words); //"Presión"
  Serial.print(": ");
  Serial.println(trend_in_words);  //estabilidad ej: "rápido aumento"
  Serial.print("13.3.- " + accuracy_in_words); //"Precisión de pronóstico"
  Serial.print(": ");
  Serial.print(accuracy_in_percent); // prob acierto
  Serial.println("%");

#ifdef JSON_METEOFCAST
  s_ValJSON[takeParam("prd")] = (ZambrettisWords.toFloat());
  s_ValJSON[takeParam("cP")] = (trend_in_words.toFloat()); //QUITADO PARA HACER PRUEBAS EN MUUTECH
#endif

  if (accuracy_ < 12) {
    Serial.println("13.3.1.- Reason: Not enough weather data yet.");
    Serial.print("We need ");
    Serial.print((12 - accuracy_) / 2);
    Serial.println(" hours more to get sufficient data.");
  }
  else
  {
    PredEnvio = (ZambrettisWords + "  ||  Presión: " + trend_in_words);
    Serial.println("PredEnvio");
  }

  Serial.println("********************************************************");
  Serial.println();
}
void measurementEvent() {

  //*********** Measures Pressure, Temperature, Humidity, VInage and calculates Altitude
  // then reports all of these to Blynk or Thingspeak
  //Measures absolute Pressure, Temperature, Humidity, VInage, calculate relative pressure,
  //Dewpoint, Dewpoint Spread, Heat Index

  Serial.println("06.3.- Obteniendo mediciones");
  temperature = data485[5] / 10.0;

  // print on serial monitor
  Serial.print("06.3.1.- Temp: ");
  Serial.print(temperature); //temperature = temperature
  Serial.println("°C; ");

  // Get humidity
  humidity = data485[4] / 10.0;
  // print on serial monitor
  Serial.print("06.3.2.- Humidity: ");
  Serial.print(humidity); //humidity = humidity
  Serial.println("%; ");

  // Get pressure
  pressure = data485[9] / 10.0;
  // print on serial monitor
  Serial.print("06.3.3.- Pressure: ");
  Serial.print(pressure); //pressures = pressure
  Serial.println("hPa; ");

  // Calculate and print relative pressure
  SLpressure_hPa = (((pressure * 100.0) / pow((1 - ((float)(ELEVATION)) / 44330), 5.255)) / 100.0);
  rel_pressure_rounded = (int)(SLpressure_hPa + .5);

#ifdef JSON_METEOFCAST
  // s_ValJSON[takeParam("P1")] = rel_pressure_rounded; //presion relativa se toma la presion del sensor en la medición de la estacion meteo
#endif

  // print on serial monitor
  Serial.print("06.3.4.- Pressure rel: ");
  Serial.print(rel_pressure_rounded);
  Serial.println("hPa; ");

  // Calculate dewpoint
  double a = 17.271;
  double b = 237.7;
  double tempcalc = (a * temperature) / (b + temperature) + log(humidity * 0.01);
  DewpointTemperature = (b * tempcalc) / (a - tempcalc);

  Serial.print("06.3.5.- Dewpoint: ");
  Serial.print(DewpointTemperature);
  Serial.println("°C; ");

  // With the dewpoint calculated we can correct temp and automatically calculate humidity
  adjusted_temp = temperature - TEMP_CORR;

  if (adjusted_temp < DewpointTemperature) adjusted_temp = DewpointTemperature; //compensation, if offset too high
  //August-Roche-Magnus approximation (http://bmcnoldy.rsmas.miami.edu/Humidity.html)
  adjusted_humi = 100 * (exp((a * DewpointTemperature) / (b + DewpointTemperature)) / exp((a * adjusted_temp) / (b + adjusted_temp)));

  if (adjusted_humi > 100) adjusted_humi = 100;    // just in case

#if defined(JSON_METEO) || defined(JSON_METEOFCAST)
  s_ValJSON[takeParam("H")] = adjusted_humi;
  s_ValJSON[takeParam("T")] = adjusted_temp;
#endif

  Serial.print("06.3.6.- Temp adjusted: ");
  Serial.print(adjusted_temp);
  Serial.println("°C; ");
  Serial.print("06.3.7.- Humidity adjusted: ");
  Serial.print(adjusted_humi);
  Serial.println("%; ");

  // Calculate dewpoint spread (difference between actual temp and dewpoint -> the smaller the number: rain or fog

  DewPointSpread = adjusted_temp - DewpointTemperature;

#ifdef JSON_METEOFCAST
  s_ValJSON[takeParam("dp")] = DewPointSpread;
#endif

  Serial.print("06.3.8.- Dewpoint Spread: ");
  Serial.print(DewPointSpread);
  Serial.println("°C; ");

  // Calculate HI (heatindex in °C) --> HI starts working above 26,7 °C
  if (adjusted_temp <= 30 && adjusted_temp > 26.7 && adjusted_humi <= 45)
  {
    HeatIndex = adjusted_temp;
  }
  else if (adjusted_temp > 26.7) {
    double c1 = -8.784, c2 = 1.611, c3 = 2.338, c4 = -0.146, c5 = -1.230e-2, c6 = -1.642e-2, c7 = 2.211e-3, c8 = 7.254e-4, c9 = -2.582e-6  ;
    double T = adjusted_temp;
    double R = humidity;

    double A = (( c5 * T) + c2) * T + c1;
    double B = ((c7 * T) + c4) * T + c3;
    double C = ((c9 * T) + c8) * T + c6;
    HeatIndex = (C * R + B) * R + A;
  }
  else if (adjusted_temp < 26.7 && adjusted_temp > 19)
  {
    if (adjusted_humi >= 55 && adjusted_humi < 80)
      HeatIndex = adjusted_temp + 1;
    if (adjusted_humi > 80)
      HeatIndex = adjusted_temp + 2;

    if (adjusted_humi > 10 && adjusted_humi < 55)
      HeatIndex = adjusted_temp - 1;
    if (adjusted_humi < 10)
      HeatIndex = adjusted_temp - 2;
  }

  else if (adjusted_temp <= 19 )
  {
    if (adjusted_humi >= 55 && adjusted_humi < 80)
      HeatIndex = adjusted_temp - 1;
    if (adjusted_humi > 80)
      HeatIndex = adjusted_temp - 2;
  }
  else
    HeatIndex = adjusted_temp;

#ifdef JSON_METEOFCAST
  s_ValJSON[takeParam("sT")] = HeatIndex;
#endif

  Serial.print("06.3.9.- HeatIndex: ");
  Serial.print(HeatIndex);
  Serial.println("°C; ");
} // end of void measurementEvent()
