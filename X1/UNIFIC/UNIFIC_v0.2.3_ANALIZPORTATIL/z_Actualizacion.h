
uint8_t checkActu(String *CRC_files) {  //devuelve 0 si no hay actualización o -1 si hay un error. Si hay actualización devuelve el número de archivos
  //RECIBE UN BUFFER DONDE GUARDAR LOS CRC
  bool b_actu = false;
  uint8_t n_files = 0;
  uint8_t n_CRC = 0;
  String aux_ = "";
  // String CRC_files[maxFiles];

  dwld_fN = "/checkActu.txt"; //archivo a descargar del servidor

  //PETICION
  if (client.connect("mytest-003.s3.amazonaws.com", 80, 20000)) {
    Serial.println("Conectado con: mytest-003.s3.amazonaws.com");
    Serial.println("Preparando para enviar: \r\n" );

    Serial.println("GET " + dwld_fN + " HTTP/1.1"); //GET /prueba07-01.txt HTTP/1.1   //GET / HTTP/1.1
    Serial.println("Host: mytest-003.s3.amazonaws.com");//Host: mytest-003.s3.amazonaws.com  //Host: mytest-003.s3-website-us-east-1.amazonaws.com
    Serial.println("Connection: keep-alive");
    Serial.println();

    client.println("GET " + dwld_fN + " HTTP/1.1"); //GET /prueba07-01.txt HTTP/1.1   //GET / HTTP/1.1
    client.println("Host: mytest-003.s3.amazonaws.com");//Host: mytest-003.s3.amazonaws.com  //Host: mytest-003.s3-website-us-east-1.amazonaws.com
    client.println("Connection: keep-alive");
    client.println();

    content_L = 0;

    //LECTURA DE CABECERA
    Serial.println(">CABECERA:");
    while (true) {
      if (client.available()) {
        Serial.print('>');
        String line = client.readStringUntil('\n');
        line.trim();
        line.toLowerCase();
        Serial.println(line);
        if (line.startsWith("content-length")) {
          content_L = line.substring(line.lastIndexOf(':') + 1).toInt();
          Serial.println(content_L);
        }
        else if (line.length() == 0) {
          Serial.println('<');
          break;
        }
      }
    }

    t_ = 0;
    time_ = millis();

    //LECTURA DE BODY Y GENERACION DE ARCHIVO
    while (t_ < content_L && (millis() - time_) < 20000) {
      if (client.available()) {
        Serial.print('~');
        String line = client.readStringUntil('\n');
        t_ += line.length();
        line.trim();
        line.toLowerCase();
        Serial.println(line);

        if (line.startsWith("actualizacion-disponible")) {
          aux_ = line.substring(line.lastIndexOf(':') + 1);
          if (aux_ == "sí" || aux_ == "si" || aux_ == "yes" || aux_ == "true" || aux_ == "1") b_actu = 1;

          else if (aux_ == "no" || aux_ == "false" || aux_ == "0" || aux_ == "") {
            b_actu = 0;
            client.stop();
            return 0;
          }
        }

        else if (line.startsWith("version")) {
          //AQUI SE PUEDE REALIZAR UNA COMPROBACION PARA VER SI LA VERSION ACTUAL CORRESPONDE CON LA VERSION DE LA DESCARGA
        }


        else if (line.startsWith("numero-archivos")) {
          aux_ = line.substring(line.lastIndexOf(':') + 1).toInt();
          if (aux_ == "" || aux_ == "0" )b_actu = 0;

          else if (aux_.toInt() > 0 && aux_.toInt() <= 10) {
            b_actu = 1;
            n_files = aux_.toInt();
          }
          else
            Serial.println("!Numero de archivos indicados superior al máximo o erroneo: " + String(aux_.toInt()));
        }

        else if (line.startsWith("crc")) {
          aux_ = line.substring(line.lastIndexOf('r') + 2, line.lastIndexOf(':'));  //comprueba el número de archivo al que corresponde el CRC (N)
          if ( aux_.toInt() > 0 && aux_.toInt() <= maxFiles) {
            CRC_files[aux_.toInt() - 1] = line.substring(line.lastIndexOf(':') + 1);

            if (n_CRC < aux_.toInt()) {
              n_CRC = aux_.toInt();
              //Serial.println(">>>>>>>>>>>>>>>>>>VALOR ACTUALIZADO DE n_CRC:"+String(n_CRC)+"\t valor de N: "+String(aux_.toInt()));
            }
          }
          else {
            Serial.println("!Numero de archivos CRC superior al máximo o erroneo: " + String(aux_.toInt()) + "\tString: " + String(aux_));
            b_actu = 0;
          }
        }
        else if (line.length() == 0) {
          Serial.println('<');
          break;
        }
      }
    }

    if (b_actu = true) {
      if (n_CRC == n_files) {
        Serial.println("Se ha guardado el mismo número de CRCs que de archivos se han indicado:");
        for (int i = 0; i < n_CRC; i++) {
          Serial.print("CRC" + String(i + 1) + ": ");
          Serial.println(CRC_files[i]);
        }
        Serial.println("VALOR DE RETORNO: " + String(n_CRC));
        return n_CRC; //retorna el numero de archivos que ha de descargar
      }
      else Serial.println("ERROR ENTRE EL NUMERO DE ARCHIVOS INDICADOS Y EL NUMERO DE CRCS");
    }




  }//END IF CLIENT CONNECT
  else
    Serial.println("ERROR AL CONECTAR CON EL CLIENTE");
  return -1;
}//END FUNCTION checkActu


uint8_t dwnldUpdtFiles(String *CRC_files, uint8_t n_CRC, String extension) { //descarga los distintos archivos que se han encontrado en checkUpdate, RECIBE: vector de Strings con los CRC ordenados de los distintos archivos y número de CRCs encontrados en el checkActu
  CRC32 crc_file;
  File outFile;

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin();

  delay(10);

  if (!SD.begin(SD_CS)) {
    Serial.println("Error al iniciar SD, saliendo de busqueda de actualización");
    delay(1000);
    err_code += "| x4000 |";
    _error = true;
    SPI.end();
    pinMode(SD_CS, INPUT_PULLDOWN);
    placa.SetPrevStatus();
    while (1);
  }

  else {


    for (int i = 0; i < n_CRC; i++) {
      known_CRC32 = CRC_files[i];
      crc_file.reset();

      Serial.println("Iniciando descarga de archivo de actualización: " + String(i + 1) + "/" + String(n_CRC));

      dwld_fN = "/updateFile" + String(i + 1) + "." + extension; //archivo a descargar del servidor
      save_fN = dwld_fN; //archivo donde se guardará inicialmente

      if (SD.exists(save_fN))
        SD.remove(save_fN);

      if (!SD.exists(save_fN)) {  // Si no existe el fichero
        outFile = SD.open(save_fN, FILE_WRITE); //Lo crea
        if (!outFile) { //Si no se genera
          Serial.println("Error abriendo el archivo para escritura: " + String(save_fN));
          err_code += "| x4102 |";
          _error = true;
          while (1);
        }
      }


      //PETICION
      if (client.connect("mytest-003.s3.amazonaws.com", 80, 20000)) {
        Serial.println("Conectado con: mytest-003.s3.amazonaws.com");
        Serial.println("Preparando para enviar: \r\n" );

        Serial.println("GET " + dwld_fN + " HTTP/1.1"); //GET /prueba07-01.txt HTTP/1.1   //GET / HTTP/1.1
        Serial.println("Host: mytest-003.s3.amazonaws.com");//Host: mytest-003.s3.amazonaws.com  //Host: mytest-003.s3-website-us-east-1.amazonaws.com
        Serial.println("Connection: keep-alive");
        Serial.println();

        client.println("GET " + dwld_fN + " HTTP/1.1"); //GET /prueba07-01.txt HTTP/1.1   //GET / HTTP/1.1
        client.println("Host: mytest-003.s3.amazonaws.com");//Host: mytest-003.s3.amazonaws.com  //Host: mytest-003.s3-website-us-east-1.amazonaws.com
        client.println("Connection: keep-alive");
        client.println();

        content_L = 0;

        //LECTURA DE CABECERA
        Serial.println(">CABECERA:");
        while (true) {
          if (client.available()) {
            Serial.print('>');
            String line = client.readStringUntil('\n');
            line.trim();
            line.toLowerCase();
            Serial.println(line);
            if (line.startsWith("content-length")) {
              content_L = line.substring(line.lastIndexOf(':') + 1).toInt();
              Serial.println(content_L);
            }
            else if (line.length() == 0) {
              Serial.println('<');
              break;
            }
          }
        }

        Serial.println("Generando nuevo archivo: " + dwld_fN);

        t_ = 0;
        time_ = millis();

        //LECTURA DE BODY Y GENERACION DE ARCHIVO
        while (t_ < content_L && (millis() - time_) < 20000) {
          if (client.available()) {
            outFile.write(client.read());
            t_++;
          }
          if (t_ % 5000 == 0) {
            Serial.print("/////////////////   ");
            Serial.print(t_);
            Serial.println("   ///////////////");
            time_ = millis();
          }
        }
        outFile.close();

        Serial.println();
        Serial.println("Leido: " + String(t_));
        Serial.println();

        //IMPRESION DE ARCHIVO Y CALCULO DE CRC32
        Serial.println("DESCARGA COMPLETADA, INICIANDO IMPRESION:");
        delay(3000);

        outFile = SD.open(save_fN, FILE_READ);

        t_ = 0;

        while (t_ < content_L) {
          char c = outFile.read();
          Serial.write(c);
          crc_updt.update(c);
          t_++;
        }
        checkSumCRC32 = crc_updt.finalize();
        outFile.close();

        Serial.println("\r\n");

        str_aux = String(checkSumCRC32, HEX);
        str_aux.toUpperCase();

        //COMPROBACIÓN DE CRC32
        Serial.print("El CRC32 calculado es: ");
        Serial.print(str_aux);
        Serial.print("\t\tEl CRC32 esperado es: ");
        Serial.println(known_CRC32);
        known_CRC32.toUpperCase();
        if (str_aux == known_CRC32.c_str()) {
          Serial.println();
          Serial.println("PRUEBA DE CRC SUPERADA");
          Serial.println(checkSumCRC32, HEX);
        }
        else {
          Serial.println("NO HA PASADO EL TEST DE CRC");
          Serial.println(checkSumCRC32, HEX);
          Serial.println(known_CRC32);

          //meter flag de repetición hasta mayor que 3 meter error de checksum
          //preparar recopilación de errores de actualización para enviar por http

          // insertarComando("g", longitud + 1, programa);
          // break;
        }
        while (1);

      }//END clientconnect
      else {
        Serial.println("ERROR NO SE HA CONECTADO CON EL SERVIDOR");
      }

    }//END for (file)
  }//END ELSE SD
}//END FUNCTION dwnldUpdtFile


uint8_t updateFirmware() {
  String CRC_files[maxFiles]; //crea un vector de string que almacenará los CRC de los distintos archivos a descargar
  uint8_t err_update = 0;

  err_update = checkActu(CRC_files);

  if (err_update > 0) { //SI HAY ACTUALIZACION Y SE HAN DESCARGADO BIEN LOS ARCHIVOS
    dwnldUpdtFiles(CRC_files, err_update, "txt");
  }
  else
    return 0;
}//END FUNCTION updateFirmware
