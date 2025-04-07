//#include "myRS485.h"
#include "configRS485.h"

#include <ArduinoJson.h>



bool F_lectura = LOW;
int c_error_read = 0;
int c_alarm_comms = 0;
int F_Freq = 0;


uint8_t GET_METER_SYNCRO();
void SERIAL_PRINT();
int Gest_Alarm_Comms();
void GEST_FREQ();

float HexToFloat(uint32_t x);
uint32_t FloatToHex(float x);

//LLAMADAS GEN_RS485
uint8_t read_gen_RS485(float* datosDisp, int addr, uint16_t REG, uint8_t numReg, uint8_t type);
//uint8_t read_gen_RS485(float *datosDisp, int addr, uint16_t REG, uint8_t numReg, uint8_t tipo);


String err_analiz = "0";

uint8_t GET_METER_SYNCRO(uint8_t ID_dispositivo)
{
  Serial.println(F("Obteniendo valores...\n"));
  uint8_t numErrRead = 0;

  for (int i = 0 ; i < Total_Reg ; i++)
  {
    err485[ID_dispositivo] = read_gen_RS485(&DATA_METER_PM2200[i], ID_dispositivo, Reg_addr[i], 2, 2); //uint8_t read_gen_RS485(float* datosDisp, int addr, uint16_t REG, uint8_t numReg, uint8_t tipo)   1:tipo float

    if (err485[ID_dispositivo] == 1) //ERROR EN LA LECTURA DEL DISPOSITIVO
    {
      numErrRead++;
      if (numErrRead == Total_Reg) {
        Serial.println("ERROR EN LA LECTURA DEL DISPOSITIVO CON DIRECCION: " + String(ID_dispositivo));
        return err485[ID_dispositivo]; //añadido 0.2
      }
    }
    if (err485[ID_dispositivo] != 0) {
      Serial.println("ERROR, No se ha podido obtener el dato del registro: " + String(Reg_addr[i]) + " del Analizador: " + String(ID_dispositivo));
    }
    else {
#ifdef JSON_RED//TOMA DE VALORES PARA JSONRED  SI DEVUELVE -1 ES QUE HAY ERROR EN LECTURA
      switch (i) {
        case 0:
          s_ValJSON[takeParam("Vff")] = DATA_METER_PM2200[0];
          break;
        case 1:
          s_ValJSON[takeParam("f1")] = DATA_METER_PM2200[1];
          break;
        case 2:
          s_ValJSON[takeParam("Al1")] = DATA_METER_PM2200[2];
          break;
        case 3:
          s_ValJSON[takeParam("Al2")] = DATA_METER_PM2200[3];
          break;
        case 4:
          s_ValJSON[takeParam("Al3")] = DATA_METER_PM2200[4];
          break;
        case 5:
          s_ValJSON[takeParam("An")] = DATA_METER_PM2200[5];
          break;
        case 6:
          s_ValJSON[takeParam("Agnd")] = DATA_METER_PM2200[6];
          break;
        case 7:
          s_ValJSON[takeParam("Aavg")] = DATA_METER_PM2200[7];
          break;
        case 8:
          s_ValJSON[takeParam("Eac")] = DATA_METER_PM2200[8];
          break;
        case 9:
          s_ValJSON[takeParam("Ere")] = DATA_METER_PM2200[9];
          break;
        case 10:
          s_ValJSON[takeParam("Eap")] = DATA_METER_PM2200[10];
          break;
        case 11:
          s_ValJSON[takeParam("Wac")] = DATA_METER_PM2200[11];
          break;
        case 12:
          s_ValJSON[takeParam("Wre")] = DATA_METER_PM2200[12];
          break;
        case 13:
          s_ValJSON[takeParam("Wap")] = DATA_METER_PM2200[13];
          break;
        case 14:
          s_ValJSON[takeParam("QFP")] = DATA_METER_PM2200[14];
          break;
      }
#endif
    }
  }
  //SERIAL_PRINT();
  //#ifdef JSON_RED
  //  s_ValJSON[takeParam("Vff")] = DATA_METER_PM2200[0];
  //  s_ValJSON[takeParam("f1")] = DATA_METER_PM2200[1];
  //  s_ValJSON[takeParam("Al1")] = DATA_METER_PM2200[2];
  //  s_ValJSON[takeParam("Al2")] = DATA_METER_PM2200[3];
  //  s_ValJSON[takeParam("Al3")] = DATA_METER_PM2200[4];
  //  s_ValJSON[takeParam("An")] = DATA_METER_PM2200[5];
  //  s_ValJSON[takeParam("Agnd")] = DATA_METER_PM2200[6];
  //  s_ValJSON[takeParam("Aavg")] = DATA_METER_PM2200[7];
  //  s_ValJSON[takeParam("Wt")] = DATA_METER_PM2200[8];
  //#endif

  return err485[ID_dispositivo]; //si es 0 todo ok
}


//---------- estacion meteo RK900 -------------------
////-----------  procesa Json ---------------

  DynamicJsonDocument doc(1024);
  
uint8_t procesaJson(uint8_t ID_dispositivo)
{
  Serial.println("estoy en procesaJson");
  // Crear un objeto JSON

  String JSONTEST;

  DeserializationError error = deserializeJson(doc, jsonConfig);

  if (error) {
    Serial.print("Error al analizar JSON: ");
    Serial.println(error.c_str());
    //    return;
  }

  // Acceder a los dispositivos
  JsonArray devices = doc["system"]["devices"];

  Serial.println("Dispositivos conectados:");
  Serial.print("devices.size() = "); Serial.println(devices.size());

  uint8_t numErrRead = 0;
  int i = 0;
  for (JsonObject device : devices) {
    JsonArray registers = device["registers"];
    for (JsonObject reg : registers) {

      err485[ID_dispositivo] = read_gen_RS485(&DATA_METEO_RK900[i], device["ID"].as<int>(), reg["address"].as<int>(), reg["size"].as<int>(), reg["type"].as<int>());
      reg["valor"] = DATA_METEO_RK900[i];
      i = i + 1;
    }
  }

  Serial.print("nº iteraciones = "); Serial.println(i);


  // imprimir los datos de los registros con los valores obtenidos, aqui hay que meter comprobaciones y demas por si el dato no es correcto o se produce un error
  for (JsonObject device : devices) {
    JsonArray registers = device["registers"];
    for (JsonObject reg : registers) {

      Serial.print("[");
      Serial.print(reg["Name"].as<const char*>());
      Serial.print(" = ");
      Serial.print(reg["valor"].as<float>());
      Serial.println(" ] ");
    }

  }

  /*
    jsonConfig = "";
    serializeJson(doc, jsonConfig);
    // Mostrar el JSON actualizado en el monitor serie
    Serial.println("JSON modificado:");
    //  Serial.println(jsonConfig);
  */

  return err485[ID_dispositivo]; //si es 0 todo ok
}

////-----------------------------------------

uint8_t GET_METEO_RK900(uint8_t ID_dispositivo)
{
  //  Serial.println(F("Obteniendo valores...del RK900\n"));
  uint8_t numErrRead = 0;

  //  Serial.print("Total_Reg_RK900 = ");
  //  Serial.println(Total_Reg_RK900);


  //err485[ID_dispositivo] = read_gen_RS485(&DATA_METEO_RK900[i], ID_dispositivo, Reg_addr_RK900[i], 0); //uint8_t read_gen_RS485(float* datosDisp, int addr, uint16_t REG, uint8_t numReg, uint8_t tipo)   1:tipo float

  /*
    node3.begin(2, SerialMux);
     // leer toda la memoria de una vez, bueno para debug.......
      node3.readHoldingRegisters(0, 100);//lectura de registros desde(NUMERO DE REGISTRO, registros a leer)
  */


  for (int i = 0 ; i < Total_Reg_RK900 ; i++)
  {

    err485[ID_dispositivo] = read_gen_RS485(&DATA_METEO_RK900[i], ID_dispositivo, Reg_addr_type_RK900[i].address, Reg_addr_type_RK900[i].num_reg, Reg_addr_type_RK900[i].type);

    Reg_addr_type_RK900[i].valor = round(DATA_METEO_RK900[i] * 100) / 100.0; // DATA_METEO_RK900[i];


    /*
        if (err485[ID_dispositivo] == 1) //ERROR EN LA LECTURA DEL DISPOSITIVO
        {
          numErrRead++;
          if (numErrRead == Total_Reg) {
            Serial.println("ERROR EN LA LECTURA DEL DISPOSITIVO CON DIRECCION: " + String(ID_dispositivo));
            return err485[ID_dispositivo]; //añadido 0.2
          }
        }
        if (err485[ID_dispositivo] != 0) {
          Serial.println("ERROR, No se ha podido obtener el dato del registro: " + String(Reg_addr[i]) + " del Analizador: " + String(ID_dispositivo));
        }
        else {
      #ifdef JSON_RED//TOMA DE VALORES PARA JSONRED  SI DEVUELVE -1 ES QUE HAY ERROR EN LECTURA
          switch (i) {
            case 0:
              s_ValJSON[takeParam("Vff")] = DATA_METER_PM2200[0];
              break;
            case 1:
              s_ValJSON[takeParam("f1")] = DATA_METER_PM2200[1];
              break;
            case 2:
              s_ValJSON[takeParam("Al1")] = DATA_METER_PM2200[2];
              break;
            case 3:
              s_ValJSON[takeParam("Al2")] = DATA_METER_PM2200[3];
              break;
            case 4:
              s_ValJSON[takeParam("Al3")] = DATA_METER_PM2200[4];
              break;
            case 5:
              s_ValJSON[takeParam("An")] = DATA_METER_PM2200[5];
              break;
            case 6:
              s_ValJSON[takeParam("Agnd")] = DATA_METER_PM2200[6];
              break;
            case 7:
              s_ValJSON[takeParam("Aavg")] = DATA_METER_PM2200[7];
              break;
            case 8:
              s_ValJSON[takeParam("Eac")] = DATA_METER_PM2200[8];
              break;

          }
      #endif
        }
    */
    /*
      #ifdef JSON_METEO
        s_ValJSON_METEO[i] = DATA_METEO_RK900[i];
        Serial.print("takeParam(Vff) = "); Serial.println(takeParam("Vff"));
        Serial.println(s_ValJSON_METEO[takeParam("Vff")]);
      #endif
    */
  }

  //  for (int i = 0; i < Total_Reg_RK900 ; i++)
  //  {
  //    doc[Reg_addr_type_RK900[i].nombre] =   round(DATA_METEO_RK900[i] * 100) / 100.0; // redondeamos el valor para evitar cosas como este valor 7.346867719e-41
  //    //      doc["a"] = DATA_METEO_RK900[i];
  //  }
  //
  //  serializeJson(doc, Serial);
  //

  //    serializeJson(doc, JSONTEST);
  //Serial.print("Json ="); Serial.println(JSONTEST);

  Serial.println("--------- Resultados RK900 ----------");
  for (int i = 0; i < Total_Reg_RK900 ; i++) {
    Serial.print(Reg_addr_type_RK900[i].nombre); Serial.print(" = "); Serial.println(Reg_addr_type_RK900[i].valor); //Serial.println(DATA_METEO_RK900[i]);
  }
  Serial.println("-------------------------------------");

  //SERIAL_PRINT();
  //#ifdef JSON_RED
  //  s_ValJSON[takeParam("Vff")] = DATA_METER_PM2200[0];
  //  s_ValJSON[takeParam("f1")] = DATA_METER_PM2200[1];
  //  s_ValJSON[takeParam("Al1")] = DATA_METER_PM2200[2];
  //  s_ValJSON[takeParam("Al2")] = DATA_METER_PM2200[3];
  //  s_ValJSON[takeParam("Al3")] = DATA_METER_PM2200[4];
  //  s_ValJSON[takeParam("An")] = DATA_METER_PM2200[5];
  //  s_ValJSON[takeParam("Agnd")] = DATA_METER_PM2200[6];
  //  s_ValJSON[takeParam("Aavg")] = DATA_METER_PM2200[7];
  //  s_ValJSON[takeParam("Wt")] = DATA_METER_PM2200[8];
  //#endif

  return err485[ID_dispositivo]; //si es 0 todo ok
}

//--------------------------------------------




//---------- estacion meteo RK300 -------------------
uint8_t GET_METEO_RK300(uint8_t ID_dispositivo)
{
  //  Serial.println(F("Obteniendo valores...del RK300\n"));
  uint8_t numErrRead = 0;

  //  Serial.print("Total_Reg_RK300 = ");
  //  Serial.println(Total_Reg_RK300);


  //err485[ID_dispositivo] = read_gen_RS485(&DATA_METEO_RK900[i], ID_dispositivo, Reg_addr_RK900[i], 0); //uint8_t read_gen_RS485(float* datosDisp, int addr, uint16_t REG, uint8_t numReg, uint8_t tipo)   1:tipo float

  /*
    node3.begin(2, SerialMux);
     // leer toda la memoria de una vez, bueno para debug.......
      node3.readHoldingRegisters(0, 100);//lectura de registros desde(NUMERO DE REGISTRO, registros a leer)
  */


  for (int i = 0 ; i < Total_Reg_RK300 ; i++)
  {

    err485[ID_dispositivo] = read_gen_RS485(&DATA_METEO_RK300[i], ID_dispositivo, Reg_addr_type_RK300[i].address, Reg_addr_type_RK300[i].num_reg, Reg_addr_type_RK300[i].type);

    Reg_addr_type_RK300[i].valor = round(DATA_METEO_RK300[i] * 100) / 100.0;  // DATA_METEO_RK300[i];

    /*
        if (err485[ID_dispositivo] == 1) //ERROR EN LA LECTURA DEL DISPOSITIVO
        {
          numErrRead++;
          if (numErrRead == Total_Reg) {
            Serial.println("ERROR EN LA LECTURA DEL DISPOSITIVO CON DIRECCION: " + String(ID_dispositivo));
            return err485[ID_dispositivo]; //añadido 0.2
          }
        }
        if (err485[ID_dispositivo] != 0) {
          Serial.println("ERROR, No se ha podido obtener el dato del registro: " + String(Reg_addr[i]) + " del Analizador: " + String(ID_dispositivo));
        }
        else {
      #ifdef JSON_RED//TOMA DE VALORES PARA JSONRED  SI DEVUELVE -1 ES QUE HAY ERROR EN LECTURA
          switch (i) {
            case 0:
              s_ValJSON[takeParam("Vff")] = DATA_METER_PM2200[0];
              break;
            case 1:
              s_ValJSON[takeParam("f1")] = DATA_METER_PM2200[1];
              break;
            case 2:
              s_ValJSON[takeParam("Al1")] = DATA_METER_PM2200[2];
              break;
            case 3:
              s_ValJSON[takeParam("Al2")] = DATA_METER_PM2200[3];
              break;
            case 4:
              s_ValJSON[takeParam("Al3")] = DATA_METER_PM2200[4];
              break;
            case 5:
              s_ValJSON[takeParam("An")] = DATA_METER_PM2200[5];
              break;
            case 6:
              s_ValJSON[takeParam("Agnd")] = DATA_METER_PM2200[6];
              break;
            case 7:
              s_ValJSON[takeParam("Aavg")] = DATA_METER_PM2200[7];
              break;
            case 8:
              s_ValJSON[takeParam("Eac")] = DATA_METER_PM2200[8];
              break;
            case 9:
              s_ValJSON[takeParam("Ere")] = DATA_METER_PM2200[9];
              break;
            case 10:
              s_ValJSON[takeParam("Eap")] = DATA_METER_PM2200[10];
              break;
            case 11:
              s_ValJSON[takeParam("Wac")] = DATA_METER_PM2200[11];
              break;
            case 12:
              s_ValJSON[takeParam("Wre")] = DATA_METER_PM2200[12];
              break;
            case 13:
              s_ValJSON[takeParam("Wap")] = DATA_METER_PM2200[13];
              break;
            case 14:
              s_ValJSON[takeParam("QFP")] = DATA_METER_PM2200[14];
              break;

          }
      #endif
        }
    */
  }



  DATA_METEO_RK300[1] = DATA_METEO_RK300[1] * 0.001;  // convertirmos el valor a escala de 0.001, segun el supplier hay que hacer esto

  //  for (int i = 0; i < Total_Reg_RK300 ; i++)
  //  {
  //    doc[Reg_addr_type_RK300[i].nombre] = round(DATA_METEO_RK300[i] * 100) / 100.0;; // redondeamos el valor para evitar cosas como este valor 7.346867719e-41
  //  }
  //
  //  serializeJson(doc, Serial);

  Serial.println("--------- Resultados RK300 ----------");
  for (int i = 0; i < Total_Reg_RK300 ; i++) {
    Serial.print(Reg_addr_type_RK300[i].nombre); Serial.print(" = "); Serial.println(Reg_addr_type_RK300[i].valor);//Serial.println(DATA_METEO_RK300[i]);
  }
  Serial.println("-------------------------------------");
  //SERIAL_PRINT();
  //#ifdef JSON_RED
  //  s_ValJSON[takeParam("Vff")] = DATA_METER_PM2200[0];
  //  s_ValJSON[takeParam("f1")] = DATA_METER_PM2200[1];
  //  s_ValJSON[takeParam("Al1")] = DATA_METER_PM2200[2];
  //  s_ValJSON[takeParam("Al2")] = DATA_METER_PM2200[3];
  //  s_ValJSON[takeParam("Al3")] = DATA_METER_PM2200[4];
  //  s_ValJSON[takeParam("An")] = DATA_METER_PM2200[5];
  //  s_ValJSON[takeParam("Agnd")] = DATA_METER_PM2200[6];
  //  s_ValJSON[takeParam("Aavg")] = DATA_METER_PM2200[7];
  //  s_ValJSON[takeParam("Wt")] = DATA_METER_PM2200[8];
  //#endif

  return err485[ID_dispositivo]; //si es 0 todo ok
}

//--------------------------------------------


bool read_reg_RS485 (uint16_t* datos,  int addr, uint16_t reg_base, uint16_t num_reg) //lee registros de RS485
{

  uint8_t result = 0;
  uint8_t j;

  node3.begin(addr, SerialMux);

  //  Serial.println("Iniciando consulta RS485 a ID:" + String(addr) + "   registro base:" + String(reg_base) + " numero de registros: " + String(num_reg));


  result = node3.readHoldingRegisters(reg_base, num_reg);//lectura de registros desde(NUMERO DE REGISTRO, registros a leer)


  delay(50); //antes 1000
  //  Serial.println("INDICADOR RESULT = " + String(result)); //ha de ser 0

  if (result == node3.ku8MBSuccess)
  {
    //    Serial.print("datos[] = ");
    for (j = 0; j < num_reg; j++)
    {
      //    Serial.print("aki2");

      datos[j] = node3.getResponseBuffer(j); //Obtiene las respuestas procedentes de los 'n' registros leidos
      //      Serial.print(datos[j], HEX);
    }
    //    Serial.println("");

    F_lectura = HIGH; //Flag de lectura 1 (lectura correcta)
    c_error_read = 0; //Contador de errores a 0

    //REDUCE EL ESTADO EN 1 SI NO ESTA EN NORMAL
    //    if (F_Freq == 0)
    //      F_Freq = 0;
    //    else
    //      F_Freq--;
    //node3.stop();
    return true;
  } else { //ERROR AL COMUNICAR CON DISP 485
    //node3.stop();
    Serial.println("Error en la comunicación modbus (ku8MBSuccess). Reg base:" + String(reg_base));
    delay(1000);
    c_error_read++; //Aumenta el contador de errores


    if (c_error_read == Total_Reg)
    {
      F_lectura = LOW;
      contFallos++;
      Serial.println(F("05.000.- >>>>WARNING>>>> CANT READ ANY VALUE. COMMUNICATION ERROR!!!\n\n"));
      c_error_read = 0;
    }
  }
  return false;
}


//int plus = 0; //testeo
//Obtiene valores mediante RS485
//uint8_t read_gen_RS485(float *datosDisp, int addr, uint16_t REG, uint8_t numReg, uint8_t tipo) //obtiene el trato tratado de los registros RS485 indicados segun el tipo (sólo válido para parámetros individuales, no enviar registros de parámetros distintos)
uint8_t read_gen_RS485(uint32_t *datosDisp, int addr, uint16_t REG, uint8_t numReg, uint8_t tipo) //obtiene el trato tratado de los registros RS485 indicados segun el tipo (sólo válido para parámetros individuales, no enviar registros de parámetros distintos)
{

  //  Serial.println("----read_gen_RS485 ----- 2 ----------");
  //  Serial.print("REG = "); Serial.println(REG);
  //  Serial.print("numReg = "); Serial.println(numReg);
  //  Serial.print("tipo = "); Serial.println(tipo);

  uint16_t datosReg[numReg]; //contiene el contenido de los distintos registros, datosDisp contiene el valor final del dato consultado (composición de registros)
  uint32_t value = 0;
  bool F_read = false;

  float *datosFloat;
  datosFloat = (float*)datosDisp; //datosFloat apunta a datosDisp

  *datosDisp = 0; //contenido de datosDisp = 0 -> contenido de datosFloat = 0

  //  Serial.println("Iniciando lectura dirección:" + String(addr) + "  | Registro: " + String(REG) + "  | Cantidad: " + String(numReg) + "  | Tipo: " + String(tipo));

  F_read = read_reg_RS485(datosReg, addr, REG, numReg); //mete en datosReg el valor de todos los registros continuos a leer

  //  numReg = 100;
  //F_read = read_reg_RS485(datosReg, 0x02, 0x00, numReg);  //0x02 0x03 0x00 0x00 0x00 0x64 0x44 0x12  AaronMR

  /*
    Serial.print("Valores: ");
    for (int i = 0; i < numReg; i++) {
      Serial.print(datosReg[i]);  // Imprime cada valor
      Serial.print(" ");          // Espacio entre valores
    }
    Serial.println();  // Salto de línea final
  */

  if (F_read != true) //ERROR EN LECTURA (ku8MB)
  {
    return 1; //error en lecturaku8mbsuccess
  }
  else { //LECTURA CORRECTA
    int j = 0;
    switch (tipo) {
      case 0:  //retorna el valor del registro sin tratarlo, tal cual se obtiene del readholding register (CONDICION: numReg=1)
        //        Serial.println("Case 0: retorna valor de registro sin tratamiento");
        for (j = 0; j < numReg; j++)//ESTO HAY QUE REVISARLO, LOS FOR SOLO TIENEN SENTIDO SI LO QUE SE LE PASA A LA FUNCION read_gen_RS485() en uint32_t *datosDisp es un vector
        {
          *datosDisp =  datosReg[j]; //contenido de datosDisp = datosReg[j]
          *datosFloat =  datosReg[j]; //contenido de datosDisp = datosReg[j]

          //          Serial.println("Valor previo: " + String(datosReg[j]));
          //          Serial.println("Valor obtenido de hextofloat: " + String(*datosDisp));
        }
        return 0;

      case 1: //retorna el valor del registro pasandolo de hex a float
        //        Serial.println("Case 1: retorna valor de registro pasando por hextofloat");
        for (j = 0; j < numReg; j++)//ESTO HAY QUE REVISARLO, LOS FOR SOLO TIENEN SENTIDO SI LO QUE SE LE PASA A LA FUNCION read_gen_RS485() en uint32_t *datosDisp es un vector
        {
          //datosFloat[j] = HexToFloat(datosReg[j]);
          *datosFloat = HexToFloat(datosReg[j]); //contenido de datosFloat = HexToFloat(datosReg[j])
          //          Serial.println("Valor previo: " + String(datosReg[j]));
          //          Serial.println("Valor obtenido de hextofloat: " + String(*datosFloat));
        }
        return 0;

      case 2:  //retorna un float (CONDICION: numReg=2)

        //        Serial.println("Case 2: retorna valor de dos registros (float) pasando por hextofloat");
        for (j = 0; j < numReg; j = j + 2)//ESTO HAY QUE REVISARLO, LOS FOR SOLO TIENEN SENTIDO SI LO QUE SE LE PASA A LA FUNCION read_gen_RS485() en uint32_t *datosDisp es un vector
        {
          value = datosReg[j];
          value = value << 16;   //desplaza el dato hacia la izk 16bits para introducirle en los 16 primeros el siguiente registro
          value = value + datosReg[j + 1]; //Suma los valores formados por 2 reg formando un float completo de 32bits

          *datosFloat = HexToFloat(value);
          //*datosFloat = HexToFloat(value)+plus; //contenido de datosFloat = HexToFloat(value)
          //plus++;

          //          Serial.println("Valor previo (union de 2 registros: " + String(value));
          //          Serial.println("Valor obtenido de hextofloat: " + String(*datosFloat));
        }
        return 0;

      default:
        return 2;
    }
  }
}


//SOBRECARGAS READ_GEN_RS485
uint8_t read_gen_RS485(float *datosDisp, int addr, uint16_t REG, uint8_t numReg, uint8_t type ) {
  //  Serial.println("-----------------read_gen_RS485--------1--------");
  uint32_t* _u32 = (uint32_t*)datosDisp;
  return read_gen_RS485(_u32,  addr,  REG, numReg, type);
}


void SERIAL_PRINT() //IMPRIME VALORES EN SERIAL
{
  Serial.println("\n06.1.- Tensión FF AVG = " + String(s_ValJSON[takeParam("Vff")]) + " V.");
  Serial.println("06.2.- Frecuencia = " + String( s_ValJSON[takeParam("f1")]) + " Hz.");
  Serial.println("06.3.- Corriente fase A = " + String(s_ValJSON[takeParam("Al1")]) + " A.");
  Serial.println("06.4.- Corriente fase B = " + String(s_ValJSON[takeParam("Al2")]) + " A.");
  Serial.println("06.5.- Corriente fase C = " + String(s_ValJSON[takeParam("Al3")]) + " A.");
  Serial.println("06.7.- Corriente Neutro = " + String(s_ValJSON[takeParam("An")]) + " A.");
  Serial.println("06.8.- Corriente fuga Tierra = " + String(s_ValJSON[takeParam("Agnd")]) + " A.");
  Serial.println("06.9.- Corriente Promedio = " + String(s_ValJSON[takeParam("Aavg")]) + " A.");
  Serial.println("06.10.- Energia Activa entregada a la carga = " + String(s_ValJSON[takeParam("Eac")]) + " kWh.");
  Serial.println("06.11.- Energia Reactiva entregada a la carga = " + String(s_ValJSON[takeParam("Ere")]) + " kVARh.");
  Serial.println("06.12.- Energia Aparente entregada a la carga = " + String(s_ValJSON[takeParam("Eap")]) + " kVAh.");
  Serial.println("06.13.- Potencia Activa total = " + String(s_ValJSON[takeParam("Wac")]) + " kWh.");
  Serial.println("06.14.- Potencia Reactiva total  = " + String(s_ValJSON[takeParam("Wre")]) + " kVARh.");
  Serial.println("06.15.- Potencia Aparente total  = " + String(s_ValJSON[takeParam("Wap")]) + " kVAh.");
  Serial.println("06.16.- Factor de potencia total 4Q_FP_PF  = " + String(s_ValJSON[takeParam("QFP")]) + " ");

}


//GESTION ALARMAS COMUNICACIONES PARA POSTERIOR ENVIO DE TEXTO DE ALARMA A MUUTECH
int Gest_Alarm_Comms()
{
  if (F_lectura == HIGH) //No hay error de comunicaciones
  {
    if (c_error_read != 0) //Error en la lectura de un valor
    {
      Serial.println(F(".- >>AD>>> SOME PARAMETER GOT ERROR ON READ"));
      c_alarm_comms = 1;
    }
    else
      c_alarm_comms = 0;


  }
  else //Error de comunicaciones
    c_alarm_comms = 2;
  //  SERIAL_PRINT();

  return c_alarm_comms;
}
