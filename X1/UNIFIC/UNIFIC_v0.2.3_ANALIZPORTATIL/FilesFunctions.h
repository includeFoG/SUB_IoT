#include <TimeLib.h>


//Log File
String logName = "CSVLOG.TXT";
String logNameDir = "/LOGDATA";

String logFileText = "Time,JSON,Reset,Error,Códigos de error,Programa";

void writeLogFile(File myLocalFile, String _time, String LogJSON, bool*b_reset, bool* b_error, String* err_code, String b_prog) {
  bool b_aux;
  b_aux = b_error;

  // if (b_aux == true) //DESCOMENTAR PARA GUARDAR SOLO CUANDO HAY ERROR
  // {
  myLocalFile.print(_time); myLocalFile.print(",");
  myLocalFile.print(LogJSON); myLocalFile.print(",");

  if (*b_reset) {
    myLocalFile.print("Si"); myLocalFile.print(",");
    *b_reset     = false;
  } else {
    myLocalFile.print("No"); myLocalFile.print(",");
  }
  if (* b_error) {
    myLocalFile.print("ERROR!!!"); myLocalFile.print(",");
    * b_error     = false;
  } else {
    myLocalFile.print("No"); myLocalFile.print(",");
  }
  myLocalFile.print(*err_code);
  myLocalFile.print(",");

  *err_code = "000  | ";
  myLocalFile.println(b_prog);
  // }
}

String getNameFile() {  //Genera un nombre de achivo a partir de la fecha en formato : YYYYMMDD_HH:MM:SS
  current_timestamp = rtc.getEpoch();
  return intToDigits(year(current_timestamp), 4) + intToDigits(month(current_timestamp), 2) + intToDigits(day(current_timestamp), 2) + "_" + intToDigits(hour(current_timestamp), 2) + ":" + intToDigits(minute(current_timestamp), 2) + ":" + intToDigits(second(current_timestamp), 2); //+EXTENSION; (EXTENSION DEL ARCHIVO .csv .txt)
}


void borrarArchivo() {
  String nameFile;
  Serial.print("Escriba el nombre del archivo a borrar: ");
  while (!Serial.available());

  nameFile = Serial.readString();
  nameFile = nameFile.substring(0, nameFile.length());
  Serial.println(nameFile);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin();
  delay(10);
  // if (card.init(SPI_HALF_SPEED, SD_CS)) {
  if (SD.begin(SD_CS)) {
    //volume.init(card);
    Serial.println("Wiring is correct and a card is present.");
    if (!SD.exists(nameFile)) Serial.println("File does not exist");
    else {
      SD.remove(nameFile);
      if (!SD.exists(nameFile)) Serial.println("File removed");
      else Serial.println("Fail removing file");
    }
  }
  else {
    Serial.println(F("SD Card Error!")); 
    err_code += "| x4000 |";
    _error = true;
  }
  SPI.end();
  pinMode(SD_CS, INPUT_PULLDOWN);
}


void leerArchivo() {
  String nameFile;
  File localFile;
  Serial.print("Escriba el nombre del archivo: ");
  while (!Serial.available());
  nameFile = Serial.readString();
  nameFile = nameFile.substring(0, nameFile.length());
  Serial.println(nameFile);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin();
  delay(10);
  //if (card.init(SPI_HALF_SPEED, SD_CS)) {
  if (SD.begin(SD_CS)) {
    //volume.init(card);
    Serial.println("Wiring is correct and a card is present.");
    localFile = SD.open(nameFile);
    if (!localFile) {
      Serial.println("Error abriendo el archivo: " + String(nameFile));
      err_code += "| x4100 |";
      _error = true;
    }
    if (localFile) {
      while (localFile.available()) {
        Serial.write(localFile.read());
      }
      localFile.close();
    }
  }
  else {
    Serial.println(F("SD Card Error!")); delay(1000);
    err_code += "| x4000 |";
    _error = true;
  }
  SPI.end();
  pinMode(SD_CS, INPUT_PULLDOWN);
}



void listaArchivos() {
  File localFile;
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin();
  delay(10);
  //if (card.init(SPI_HALF_SPEED, SD_CS)) {
  if (SD.begin(SD_CS)) {
    //volume.init(card);
    localFile = SD.open("/");
    while (true) {
      File entry =  localFile.openNextFile();
      if (! entry) {
        // no more files
        break;
      }
      // Serial.print(entry.name());
      entry.printName(&Serial);
      Serial.println();
      entry.close();
    }
    localFile.close();
  }
  else {
    Serial.println(F("SD Card Error!")); delay(1000);
    err_code += "| x4000 |";
    _error = true;
  }
  SPI.end();
  pinMode(SD_CS, INPUT_PULLDOWN);
}



uint8_t _SaveDataLog_(File outFile, String programa) {  //almacena los datalogs en directorios divididos por dias

  String dayDir;
  String f_log;

  current_timestamp = rtc.getEpoch();
  if (year(current_timestamp) < 2021) {
    Serial.println("Error en la fecha, iniciando actualización");
    //  insertarComando("OtSV", longitud + 1, programa);
    return 1;
  }

  localTime = String(year(current_timestamp));
  if (month(current_timestamp) > 9) {
    localTime += ("/" + String(month(current_timestamp)));
  }
  else
    localTime += ("/0" + String(month(current_timestamp)));

  if (day(current_timestamp) > 9) {
    localTime += ("/" + String(day(current_timestamp)));
  }
  else
    localTime += ("/0" + String(day(current_timestamp)));

  localTime += (" " + String(hour(current_timestamp)));
  localTime += (":" + String(minute(current_timestamp)));
  localTime += (":" + String(second(current_timestamp)));

  //NO MODIFICAR ESTE FORMATO SI SE QUIERE UTILIZAR LA FUNCION _findFirstFile_() de SdFat.h
  dayDir = ("/" + String(year(current_timestamp)));  //la barra corresponde al path
  if (month(current_timestamp) > 9) {
    dayDir +=  String(month(current_timestamp));
  }
  else
    dayDir += "0" + String(month(current_timestamp));
  if (day(current_timestamp) > 9) {
    dayDir +=  String(day(current_timestamp));
  }
  else
    dayDir += "0" + String(day(current_timestamp));

  placa.RecStatus();
  placa.EnableConmutado3v();

  Serial.println();
  Serial.print("Time: "); Serial.print(localTime);
  Serial.print("\t BoardTemp: "); Serial.print(MeasureTemperature());
  Serial.print("\t Input V: "); Serial.print(powerIn.busvoltage);
  Serial.print("\t Input mA: "); Serial.print(powerIn.current_mA);
  Serial.print("\t Sensor ID: "); Serial.print(localConfig.sensorID);
  Serial.print("\t SensCh1 (4-20mA): "); Serial.print(sensCh1);
  Serial.print("\t SensCh2 (4-20mA): "); Serial.print(sensCh2);
  Serial.print("\t SensCh3 (4-20mA): "); Serial.print(sensCh3);
  Serial.print("\t SensCh4 (4-20mA): "); Serial.println(sensCh4);
  Serial.println();

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin();

  delay(10);

  if (!SD.begin(SD_CS)) {
    Serial.println(F("SD Card Error!")); delay(1000);
    err_code += "| x4000 |";
    _error = true;
    SPI.end();
    pinMode(SD_CS, INPUT_PULLDOWN);
    placa.SetPrevStatus();
    return 2;
  }

  if (!SD.exists(logNameDir)) {  // Si no existe el directorio principal
    Serial.println("Generando directorio principal de datalog");
    if (!SD.mkdir(logNameDir)) {
      Serial.println("Error generando directorio principal del datalog: " + String(logNameDir));
      err_code += "| x4110 |";
      _error = true;
      return 2;
    }
    else  //Si se genera
      Serial.println("Se ha generado el directorio principal del datalog: " + String(logNameDir));
  }

  //Si ya existia el directorio principal
  if (!SD.exists(logNameDir + dayDir)) { // Si no existe el directorio diario
    Serial.println("Generando directorio diario de datalog");
    if (!SD.mkdir(logNameDir + dayDir)) {
      Serial.println("Error generando directorio diario de datalog: " + String(logNameDir + dayDir));
      err_code += "| x4111 |";
      _error = true;
      return 2;
    }
    else  //Si se genera
      Serial.println("Se ha generado el directorio diario: " + String(logNameDir + dayDir));
  }

  //MODIFICAR para indicar el tramo de tiempo que cubre cada archivo
  f_log = dayDir;
  //  f_log += String(hour(current_timestamp));
  //  f_log += String(minute(current_timestamp));
  //  f_log += String(second(current_timestamp));
  f_log += (".csv");

  f_log.trim();

  if (!SD.exists(logNameDir + dayDir + "/" + f_log)) { //Si no existe el fichero del log
    Serial.println("Generando archivo diario");
    outFile = SD.open(logNameDir + dayDir + "/" + f_log, FILE_WRITE);
    if (!outFile) {
      Serial.println("Error abriendo el archivo para escritura: " + String(logNameDir + dayDir + "/" + f_log));
      err_code += "| x4102 |";
      _error = true;
      return 2;
    } else { //Si se genera
      outFile.println(logFileText); //Mete los nombres de las columnas
      outFile.println();
      Serial.println("Se ha generado el fichero: " + String(f_log) + " | En el directorio: " + String(logNameDir + dayDir));
      outFile.close();
    }
  }

  //Si ya existia
  outFile = SD.open(logNameDir + dayDir + "/" + f_log, FILE_WRITE);
  if (!outFile) {
    Serial.println("Error abriendo el archivo para escritura: " + String(f_log));
    err_code += "| x4102 |";
    _error = true;
    return 2;
  }
  else
  {
    Serial.println("El localtime de V case 22 es: " + String(localTime));

    //FUNCION QUE ESCRIBE EN EL ARCHIVO GENERADO
    writeLogFile(outFile, localTime, LogJSON, &_reset, &_error, &err_code, programa);
    outFile.close();
    Serial.println("GUARDADO DESDE _SAVEDATALOG_()");
  }


  SPI.end();
  pinMode(SD_CS, INPUT_PULLDOWN);
  placa.SetPrevStatus();
  return 0;
}



uint8_t replaceFile(File outFile, const char nameFile[], String body) { //REEMPLAZA EL ARCHIVO SI EXISTIA ANTERIORMENTE, SINO LO GENERA
  //Si ya existia el archivo -> ELIMINAR
  if (SD.exists(nameFile)) {
    Serial.println("ELIMINANDO ARCHIVO: " + String(nameFile) + " ANTERIOR");
    outFile = SD.open(nameFile, FILE_WRITE);

    if (outFile.remove())
      Serial.println("Archivo eliminado correctamente");
    else {
      Serial.println("Error al eliminar el archivo");
      delay(1000);
      err_code += "| x4103 |";
      _error = true;
      SPI.end();
      pinMode(SD_CS, INPUT_PULLDOWN);
      placa.SetPrevStatus();
      return 1;
    }
  }
  else {
    Serial.println("Generando nuevo archivo: " + String(nameFile));
    outFile = SD.open(nameFile, FILE_WRITE);
    if (!outFile) {
      Serial.println("Error generando el archivo para escritura:" + String(nameFile));
      err_code += "| x4104 |";
      _error = true;
      return 2;
    } else { //Si se genera
      outFile.println(body);
      outFile.println();
      Serial.println("Se ha generado el fichero:" + String(nameFile));
      outFile.close();

      return 0;
    }
  }
}



uint8_t writeUpdateFile(File outFile, String body) {   //genera el archivo binario de actualización =)
  placa.RecStatus();
  placa.EnableConmutado3v();

  const char fileUpdate[] = "UPDATE.txt";
  const char newFileUpdate[] = "newUPDATE.txt";

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin();

  delay(10);

  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card Error!");
    delay(1000);
    err_code += "| x4000 |";
    _error = true;
    SPI.end();
    pinMode(SD_CS, INPUT_PULLDOWN);
    placa.SetPrevStatus();
    return 1;
  }

  //Si ya existia el archivo UPDATE -> ELIMINAR
  uint8_t err_SD = replaceFile(outFile, fileUpdate, body);
  if (err_SD == 0)
    err_SD = replaceFile(outFile, newFileUpdate, body);

  if (err_SD != 0 )
  {
    return 2;
  }

  SPI.end();
  pinMode(SD_CS, INPUT_PULLDOWN);
  placa.SetPrevStatus();
  return 0;
}




uint8_t findFile(const char nameToFind[], File &fileToReturn) { //busca y devuelve un archivo de la SD
#ifdef DEBUGCODE
  Serial.print("Iniciando busqueda del archivo: ");
  Serial.println(nameToFind);
#endif

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin();
  delay(10);

  if (SD.begin(SD_CS)) {
#ifdef DEBUGCODE
    Serial.println("SD INICIALIZADA");
#endif
    if (!SD.exists(nameToFind)) {
#ifdef DEBUGCODE
      Serial.println("ERROR!!! ARCHIVO NO ENCONTRADO");
#endif
      return 1;
    }
    else {
#ifdef DEBUGCODE
      Serial.println("Archivo encontrado");
#endif
      fileToReturn = SD.open(nameToFind, FILE_READ);       // Open file for reading
      if (!fileToReturn) {
        Serial.println("08.000.- Error abriendo el archivo para lectura: " + String(nameToFind));
        err_code += "| x4101 |";
        _error = true;
        return 2;
      }
      else {
        return 0;
      }
    }
  }
  else {
#ifdef DEBUGCODE
    Serial.println(F("ERROR INICIALIZANDO SD!"));
#endif
    err_code += "| x4000 |";
    _error = true;
    return 3;
  }
  //   myDataFile.close();
  // SPI.end();
  // pinMode(SD_CS, INPUT_PULLDOWN);
}
