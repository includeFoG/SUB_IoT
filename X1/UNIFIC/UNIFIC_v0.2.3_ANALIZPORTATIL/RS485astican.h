//#include "myRS485.h"
#include "configRS485.h"

bool F_lectura = LOW;
int c_error_read = 0;
int c_alarm_comms = 0;
int F_Freq = 0;


uint8_t GET_METER_SYNCRO();
void SERIAL_PRINT();
int Gest_Alarm_Comms();
void GEST_FREQ();
String UrlstringMuutech(String _path, String _host, String _sensorID, float Data_Meter [Total_Reg], String Inf_Alarm);
int EnvioMuutech(float DATA_METER[], String textoAlarma);
float HexToFloat(uint32_t x);
uint32_t FloatToHex(float x);
void log_error_485(uint8_t error_);

//LLAMADAS GEN_RS485
uint8_t read_gen_RS485(float* datosDisp, int addr, uint16_t REG, uint8_t numReg);
//uint8_t read_gen_RS485(uint32_t* datosDisp, int addr, uint16_t REG, uint8_t numReg, uint8_t tipo);

String err_analiz = "0";

uint8_t GET_METER_SYNCRO(uint8_t ID_dispositivo)
{
  Serial.println(F("Obteniendo valores...\n"));
  uint8_t numErrRead = 0;

  for (int i = 0 ; i < Total_Reg ; i++)
  {
    err485[ID_dispositivo] = read_gen_RS485(&DATA_METER_PM2200[i], ID_dispositivo, Reg_addr[i], 2); //uint8_t read_gen_RS485(float* datosDisp, int addr, uint16_t REG, uint8_t numReg, uint8_t tipo)   1:tipo float

    if (err485[ID_dispositivo] == 1) //ERROR EN LA LECTURA DEL DISPOSITIVO
    {
      numErrRead++;
      if (numErrRead == Total_Reg) {
        Serial.println("ERROR EN LA LECTURA DEL DISPOSITIVO CON DIRECCION: " + String(ID_dispositivo));
        return err485[ID_dispositivo]; //añadido 0.2
      }
    }
    if (err485[ID_dispositivo] != 0) {
      log_error_485(err485[ID_dispositivo]);
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


void log_error_485(uint8_t error_)
{
  switch (error_) {
    case 1:
      Serial.println("Error en la lectura del dispositivo");
      break;
    case 2:
      Serial.println("Tipo de dato de retorno no declarado");
      err_code += "| x2112 |";
      _error = true;
      break;
    default:
      Serial.println("Error no esperado");
      err_code += "| x2113 |";
      _error = true;
      break;
  }
}

bool read_reg_RS485 (uint16_t* datos,  int addr, uint16_t reg_base, uint16_t num_reg) //lee registros de RS485
{
  uint8_t result = 0;
  uint8_t j;

  node3.begin(addr, SerialM2);

  Serial.println("Iniciando consulta RS485 a ID:" + String(addr) + "   registro base:" + String(reg_base) + " numero de registros: " + String(num_reg));

  result = node3.readHoldingRegisters(reg_base, num_reg);//lectura de registros desde(NUMERO DE REGISTRO, registros a leer)

  delay(50); //antes 1000
  Serial.println("INDICADOR RESULT = " + String(result)); //ha de ser 0

  if (result == node3.ku8MBSuccess)
  {
    for (j = 0; j < num_reg; j++)
    {
      datos[j] = node3.getResponseBuffer(j); //Obtiene las respuestas procedentes de los 'n' registros leidos
    }

    F_lectura = HIGH; //Flag de lectura 1 (lectura correcta)
    c_error_read = 0; //Contador de errores a 0

    //REDUCE EL ESTADO EN 1 SI NO ESTA EN NORMAL
    //    if (F_Freq == 0)
    //      F_Freq = 0;
    //    else
    //      F_Freq--;
    //node3.stop();
    return true;
  }
  else { //ERROR AL COMUNICAR CON DISP 485
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

      err_code += "| x2102 |";
      _error = true;
    }
  }
  return false;
}


//int plus = 0; //testeo
//Obtiene valores mediante RS485
uint8_t read_gen_RS485(uint32_t *datosDisp, int addr, uint16_t REG, uint8_t numReg, uint8_t tipo) //obtiene el trato tratado de los registros RS485 indicados segun el tipo (sólo válido para parámetros individuales, no enviar registros de parámetros distintos)
{
  float transf = 0;
  uint16_t datosReg[numReg]; //contiene el contenido de los distintos registros, datosDisp contiene el valor final del dato consultado (composición de registros)
  uint32_t value = 0;
  bool F_read = false;

  float *datosFloat;
  datosFloat = (float*)datosDisp; //datosFloat apunta a datosDisp

  //DatosDisp actualmente es solo un valor del vector por lo que esto no tiene sentido
  /*for (int i = 0; i < NUMITEMS(datosDisp); i++) //Modificado para ARS antes: i< 8
    {
    datosDisp[i] = 0;
    }
  */

  *datosDisp = 0; //contenido de datosDisp = 0 -> contenido de datosFloat = 0
  
  Serial.println("Iniciando lectura dirección:" + String(addr) + "  | Registro: " + String(REG) + "  | Cantidad: " + String(numReg) + "  | Tipo: " + String(tipo));

  F_read = read_reg_RS485(datosReg, addr, REG, numReg); //mete en datosReg el valor de todos los registros continuos a leer

  /////////////////////////////////////////////////////////////////////////
  //AÑADIDO PARA TEST:
  /*
  F_read = true;
  //1076293874 -> 4026F0F2 -> 100000000100110-1111000011110010 -> 16422 y 61682
  datosReg[0] = 16422;
  datosReg[1] = 61682;
  */
  //DEBE DAR 2,61
  ////////////////////////////////////////////////////////////////////////

  if (F_read != true) //ERROR EN LECTURA (ku8MB)
  {
    return 1; //error en lecturaku8mbsuccess
  }
  else { //LECTURA CORRECTA
    int k_ = 0;
    int j = 0;
    switch (tipo) {
      case 0:  //retorna el valor del registro sin tratarlo, tal cual se obtiene del readholding register (CONDICION: numReg=1)
        Serial.println("Case 0: retorna valor de registro sin tratamiento");
        for (j = 0; j < numReg; j++)//ESTO HAY QUE REVISARLO, LOS FOR SOLO TIENEN SENTIDO SI LO QUE SE LE PASA A LA FUNCION read_gen_RS485() en uint32_t *datosDisp es un vector
        {
          //datosDisp[j] = datosReg[j];
          *datosDisp =  datosReg[j]; //contenido de datosDisp = datosReg[j]
          Serial.println("Valor previo: " + String(datosReg[j]));
          Serial.println("Valor obtenido de hextofloat: " + String(*datosDisp));
        }
        return 0;

      case 1: //retorna el valor del registro pasandolo de hex a float
        Serial.println("Case 1: retorna valor de registro pasando por hextofloat");
        for (j = 0; j < numReg; j++)//ESTO HAY QUE REVISARLO, LOS FOR SOLO TIENEN SENTIDO SI LO QUE SE LE PASA A LA FUNCION read_gen_RS485() en uint32_t *datosDisp es un vector
        {
          //datosFloat[j] = HexToFloat(datosReg[j]);
          *datosFloat = HexToFloat(datosReg[j]); //contenido de datosFloat = HexToFloat(datosReg[j])
          Serial.println("Valor previo: " + String(datosReg[j]));
          Serial.println("Valor obtenido de hextofloat: " + String(*datosFloat));
        }
        return 0;

      case 2:  //retorna un float (CONDICION: numReg=2)

        Serial.println("Case 2: retorna valor de dos registros (float) pasando por hextofloat");
        for (j = 0; j < numReg; j = j + 2)//ESTO HAY QUE REVISARLO, LOS FOR SOLO TIENEN SENTIDO SI LO QUE SE LE PASA A LA FUNCION read_gen_RS485() en uint32_t *datosDisp es un vector
        {
          value = datosReg[j];
          value = value << 16;   //desplaza el dato hacia la izk 16bits para introducirle en los 16 primeros el siguiente registro
          value = value + datosReg[j + 1]; //Suma los valores formados por 2 reg formando un float completo de 32bits

          *datosFloat = HexToFloat(value);
          //*datosFloat = HexToFloat(value)+plus; //contenido de datosFloat = HexToFloat(value)
          //plus++;

          Serial.println("Valor previo (union de 2 registros: " + String(value));
          Serial.println("Valor obtenido de hextofloat: " + String(*datosFloat));
        }
        return 0;

      default:
        return 2;
    }
  }
}

//SOBRECARGAS READ_GEN_RS485
uint8_t read_gen_RS485(float *datosDisp, int addr, uint16_t REG, uint8_t numReg) {

  //  Serial.print("El valor de *datosdisp[4] es: ");
  //  Serial.println(*datosDisp);
  //  Serial.print("Su valor en uint32 es: ");
  //  Serial.println((uint32_t)*datosDisp);

  uint32_t* _u32 = (uint32_t*)datosDisp;


  // Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>HA PASADO POR LA SOBRECARGA");
  return read_gen_RS485(_u32,  addr,  REG, numReg, 2);
}



//LECTURA AMPERIMETROS Circuitor
uint8_t Read_amp_RS485(float *datosAmp_, int addr_)
{
  uint8_t F_try = 0;

  do {
    err485[addr_] = read_gen_RS485(&datosAmp_[0], addr_, AMP_REG, 2 ); //uint8_t read_gen_RS485(float* datos, int addr, uint16_t REG, uint8_t numReg, uint8_t tipo) ; tipo1:float
    retry_rs485++;
    delay(100);
  } while ((err485[addr_] != 0) and (retry_rs485 < RETRY_485));
  if (err485[addr_] != 0) {
    log_error_485(err485[addr_]);
    Serial.println("No se ha podido obtener el dato 1 del Amperimetro " + String(addr_ - 2));
    cont_errores++;
    F_try++;
  }
  else
  {
    switch (addr_) {
      case 3:
        s_ValJSON[takeParam("A1")] = datosAmp_[0]; //Corriente Amp1 (A)
        break;
      case 4:
        s_ValJSON[takeParam("A2")] = datosAmp_[0]; //Corriente Amp1 (A)
        break;
      case 5:
        s_ValJSON[takeParam("A3")] = datosAmp_[0]; //Corriente Amp1 (A)
        break;
      case 6:
        s_ValJSON[takeParam("A4")] = datosAmp_[0]; //Corriente Amp1 (A)
        break;
    }
  }
  retry_rs485 = 0;
  Serial.println("final " + String(addr_ - 2) + ".0: " + String(datosAmp_[0]));

  do {
    err485[addr_] = read_gen_RS485(&datosAmp_[1], addr_, FREC_REG, 2 ); //uint8_t read_gen_RS485(float* datos, int addr, uint16_t REG, uint8_t numReg, uint8_t tipo) ; tipo1:float
    retry_rs485++;
    delay(100);
  } while ((err485[addr_] != 0) and (retry_rs485 < RETRY_485));
  if (err485[addr_] != 0) {
    log_error_485(err485[addr_]);
    Serial.println("No se ha podido obtener el dato 2 del Amperimetro " + String(addr_ - 2));
    cont_errores++;
    F_try++;
  }
  else
  {
    retry_rs485 = 0;
    switch (addr_) {
      case 3:
        s_ValJSON[takeParam("f1")] = datosAmp_[1]; //Corriente Amp1 (A)
        break;
      case 4:
        s_ValJSON[takeParam("f2")] = datosAmp_[1]; //Corriente Amp1 (A)
        break;
      case 5:
        s_ValJSON[takeParam("f3")] = datosAmp_[1]; //Corriente Amp1 (A)
        break;
      case 6:
        s_ValJSON[takeParam("f4")] = datosAmp_[1]; //Corriente Amp1 (A)
        break;
    }
  }
  Serial.println("final " + String(addr_ - 2) + ".1: " + String(datosAmp_[1]));

  if (F_try == 0)
    return 0;
  else if (F_try >= 2)
    return 2;
  else
    return 1;
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
  Serial.println("06.10.- Potencia Activa entregada a la carga = " + String(s_ValJSON[takeParam("Wt")]) + " kWh.\n");
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

    SERIAL_PRINT();
  }
  else //Error de comunicaciones
    c_alarm_comms = 2;

  return c_alarm_comms;
}


//GESTION DE FRECUENCIA EN FUNCION DE ERRORES PRODUCIDOS
void GEST_FREQ()
{
  switch (F_Freq)
  {
    case 0:
      delay(FrecUp);
      break;
    case 1:
      delay(FrecAlarm0);
      break;
    case 2:
      delay(FrecAlarm1);
      break;
    case 3:
      delay(FrecCrit);
      break;
    default:
      delay(FrecUp);
      break;
  }
}
