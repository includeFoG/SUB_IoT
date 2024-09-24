#define MYRS485
#ifdef MYRS485

#include <ModbusMaster.h>
#include <QuickMedianLib.h>

#define MAX485_DE      26
#define MAX485_RE_NEG  26

// instantiate ModbusMaster object
ModbusMaster node1;
ModbusMaster node2;
ModbusMaster node3;

//------------------------------------------------------------------------------------------------------------------------Config Reg. RJ45 SIEMENS PAC 4200 //NIU
float HexToFloat(uint32_t x)
{
  return (*(float*)&x);
}

uint32_t FloatToHex(float x)
{
  return (*(uint32_t*)&x);
}

String err_ME = "0";
String err_RS = "0";
uint8_t Mac_Master_RJ45[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xE0 };
IPAddress IP_Master_RJ45(192, 168, 0, 50);
//  IPAddress IP_Slave_RJ45(192, 168, 0, 49);

//IPs SIEMENS CT, NO BORRAR!
IPAddress IP_Slave1_RJ45(192, 168, 0, 1);
IPAddress IP_Slave2_RJ45(192, 168, 0, 2);
IPAddress IP_Slave3_RJ45(192, 168, 0, 3);

IPAddress RJ45IP_addr[NUMSLAVES_RJ45] =
{
    IP_Slave1_RJ45,
   //IP_Slave2_RJ45,
   //IP_Slave3_RJ45
};

EthernetClient client_Eth_RJ45;//client_modbus_RJ45
ModbusTCPClient modbusTCPClient(client_Eth_RJ45);




#define Total_RegRJ45 9  //OJO LOS REGISTROS ESTAN EN DATASHEET CON OFFSET 
//registros
#define RJ45Reg_V_FF_AVG 0x003B // (59)0x003B  //SIEMENS PAC4200 reg 59 y 60- 0x03B Media en 3 fases de la tensión L-L (V) Float
#define RJ45Reg_Freq     0x0037 //(55)0x0037  //SIEMENS PAC4200 reg 55 y 56- 0x037 Frecuencia (Hz) Float

#define RJ45Reg_I_L1 0x000D//(13)0x000D    //SIEMENS PAC4200 reg 13 y 14- 0x00D Corrientes de fase (Amp)Float
#define RJ45Reg_I_L2 0x000F//(15)0x000F    //SIEMENS PAC4200 reg 15 y 16- 0x00F
#define RJ45Reg_I_L3 0x0011//(17)0x0011    //SIEMENS PAC4200 reg 17 y 18- 0x011
#define RJ45Reg_I_N  0x0127//(295)0x0127   //SIEMENS PAC4200 reg 295 y 296- 0x127 Corriente neutro (Amp)Float
#define RJ45Reg_P_TOT 0x0165//(357)0x0165  //SIEMENS PAC4200 reg 357 y 358- 0xBC2 Media móvil de potencia activa total(W)
#define RJ45Reg_S_TOT 0x0163//(355)0x0163  //SIEMENS PAC4200 reg 355 y 356- 0xBC2 Media móvil de potencia aparente total(VA)
#define RJ45Reg_Q_TOT 0x0169//(361)0x0169  //SIEMENS PAC4200 reg 361 y 362- 0xBC2 Media móvil de potencia reactiva total (VAr)

uint16_t RJ45Reg_addr[Total_RegRJ45] =
{
  RJ45Reg_V_FF_AVG,
  RJ45Reg_Freq ,
  RJ45Reg_I_L1,
  RJ45Reg_I_L2,
  RJ45Reg_I_L3,
  RJ45Reg_I_N,
  RJ45Reg_P_TOT,
  RJ45Reg_Q_TOT,
  RJ45Reg_S_TOT
};

float DATA_METER_RJ45[Total_RegRJ45];



//Obtiene valores mediante RS485

float Read_RJ45(uint16_t REG) //NIU
{
  float i = 0;
  uint16_t result;
  uint8_t j;
  uint16_t dataRJ45[2];
  uint32_t valor = 0;

  Serial.println("modbus-TCP>> Leyendo registro:" + String(REG) + " de Analizador Siemens-PAC:" + String(n_Analiz + 1));
  //unsigned long tiempo1,tiempo2=0; //TESTXAVI
  for (j = 0; j < 2; j++)
  {
    //tiempo1 = micros();//TESTXAVI
    dataRJ45[j] = modbusTCPClient.inputRegisterRead(REG + j);
    // tiempo2 = micros();//TESTXAVI
    // Serial.println(tiempo2-tiempo1);//TESTXAVI
    // Serial.println("EL TIEMPO EN LEER EL REGISTRO HA SIDO: "+String(millis()-tiempopasado)); //TESTXAVI
    // tiempopasado = millis();//TESTXAVI
    Serial.print("HEX:");
    Serial.println(dataRJ45[j], HEX);
  }

  valor = dataRJ45[0];
  valor = valor << 16; //desplaza el dato hacia la izk 16bits para introducirle en los 16 primeros el siguiente registro
  valor = valor + dataRJ45[1];  //Suma los valores formados por 2 reg formando el registro completo de 32bits

  //Serial.print("valor: ");
  //Serial.println(valor);

  i = HexToFloat(valor); //conversión hexfloat (Modbus devuelve en HEX)
  Serial.println("VALOR DE \"i\": " + String(i));

  if ( String(i).indexOf("nan") != -1)
  {
    Serial.println("Error al tomar valor en Reg: " + String(REG));
    err_code += "| x2111 |";
    _error = true;
  }

  return i;
}

void GET_VALUERJ45() //NIU
{
#ifdef JSON_SIEMENSPAC

  for (int i = 0 ; i < Total_RegRJ45 ; i++)
  {
    DATA_METER_RJ45[i] = Read_RJ45(RJ45Reg_addr[i]);
  }

  for (int i = 0 ; i < Total_RegRJ45 ; i++)
  {
    Serial.print(DATA_METER_RJ45[i]);
    Serial.print("  ;  ");

    //   s_ValSIEMENSPAC[i + 4] = DATA_METER_RJ45[i];
  }
  Serial.println();
  
  s_ValJSON[takeParam("Vff")] = DATA_METER_RJ45[0];
  s_ValJSON[takeParam("f1")] = DATA_METER_RJ45[1];
  s_ValJSON[takeParam("Al1")] = DATA_METER_RJ45[2];
  s_ValJSON[takeParam("Al2")] = DATA_METER_RJ45[3];
  s_ValJSON[takeParam("Al3")] = DATA_METER_RJ45[4];
  s_ValJSON[takeParam("An")] = DATA_METER_RJ45[5];
  s_ValJSON[takeParam("Wt")] = DATA_METER_RJ45[6];
  s_ValJSON[takeParam("Qt")] = DATA_METER_RJ45[7];
  s_ValJSON[takeParam("St")] = DATA_METER_RJ45[8];
  //  Serial.println();
  //  for(int i=0; i<9; i++)
  //  {
  //    Serial.print(s_ValSIEMENSPAC[i+4]);
  //    Serial.print("  ;  ");
  //  }
  Serial.println();
#else
  Serial.println("ERROR no se encuentra definido JSON_SIEMENSPAC");
#endif
}



//-------------------------------------------------------------------------------------------------------------------------------------------------------------------


void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void initCallbacks(ModbusMaster* _node) {
  _node->preTransmission(preTransmission);
  _node->postTransmission(postTransmission);
}

uint8_t resultRS485;

#define CLEAR_DAT         0x005A
#define RETRY_485         3

//#define WEATHER_ADDR      2
#define TOTAL_REG485      10
#define BASE_REG          0x01f4

//#define RAIN_ADDR         3
#define TOTAL_RAIN_REG    1
#define BASE_RAIN_REG     0x0000

//#define LIGHT_ADDR        4
#define TOTAL_LIGHT_REG   1
#define BASE_LIGHT_REG    0x0006

//#define UV_ADDR        5
#define TOTAL_UV_REG   2
#define BASE_UV_REG    0x0000

//#define HC_DISEN_ADDR  1
#define TOTAL_HC_REG 2
#define BASE_HC_REG  0x2600 //0x2600 DIRECCION LOGICA (+1)
#define CLEAN_REG 0x3100



#define AMP1_ADDR          3   //REVISAR 0x0001 en menu de configuración de comunicaciones "Coññ" en el dispositivo esclavo
#define AMP2_ADDR          4
#define AMP3_ADDR          5
#define AMP4_ADDR          6
#define AMP_REG           0x0012  //corriente float (A)
#define FREC_REG          0x001C  //frecuencia float (Hz)

int new_addr = 1;
int retry_rs485 = 0;
int cont_errores = 0;
uint8_t err485[8]; //dispositivos: estacion meteo, raingauge, sensor de luz, amperimetros (4), analizadores de red
float data485[TOTAL_REG485];
float datarain485;
float dataligth485;
float dataAmp485;
float dataUV485[TOTAL_UV_REG ];
float dataHC485[TOTAL_HC_REG];

bool flagFirst = true;
const int NumMed = (DEF_WAIT_TIME / DEF_MEASURE_FREQ);//const int NumMed = (DEF_WAIT_TIME / DEF_MEASURE_FREQ) - 2 -> si sale negativo forzar que sea 1 ; //10;  //Número de medidas 10   HA DE SER EL NUMERO DE VECES QUE SE MIDE ENTRE ENVIOS
const int TimeMed = 1000; // 1000 determina el tiempo entre lecturas de registro (durante 10 seg)
const int numReadReg = (((10 * 1000) / TimeMed) - 1); //(DEF_MEASURE_FREQ / (TimeMed / 1000)) - 1;// numero de lecturas de registro máximo que puede hacerse en una medicion de 10 segundos

/////Variables relacionadas con mediciones continuas de los distintos parámetros
int v_median0[NumMed];//vector que contiene los valores de la velocidad del viento
int v_median2[NumMed];//vector que contiene los valores de la dirección del viento
int v_median3[NumMed];//vector que contiene los valores de los grados del viento
int aux_v_median[NumMed]; //vector auxiliar para poder aplicar GetMedian de la librería QuickMedian, necesario para no modificar los datos

int m_Media[2][NumMed]; //matriz para la lectura continua de velocidad del viento, dirección y grados
int aux_m_Media[2][NumMed]; //matriz auxiliar para poder aplicar GetMedian de la librería QuickMedian, necesario para no modificar los datos
int v_MedPrev[2]; //vector comparativo t-1 para la lectura continua de velocidad del viento, dirección y grados
//float dataMedian[2]; //vector que almacena los valores de las medianas en lectura continua de velocidad del viento, dirección y grados

int _k0 = 0; //indice vector mediana velocidad del viento
int _k1 = 0; //indice vector mediana const simult
int _k2 = 0; //indice vector mediana direccion del viento
int _k3 = 0; //indice vector mediana grados del viento
int _MedPrev0 = 0; //valor comparativo t-1 velocidad del viento
//int _MedPrev2 = 0; //valor comparativo t-1 dirección del viento
//int _MedPrev3 = 0; //valor comparativo t-1 grados del viento



void MedidaConst(float *datas, int deviceID, int total_reg, int base_reg, int reg_med, int *v_Median, int &k, int timeMed_, int numMed_, String NombreMed, int &_MedPrev, bool CorrecLin, int valCorrec = 0)
{
  Serial.println("");

  Serial.print("Iniciando medición continua de " + String(NombreMed));
  Serial.println("  CorrecLin:" + String(CorrecLin));
  Serial.println("Valor de k para " + String(NombreMed) + " = " + String(k));

  SerialM2.begin(M2_BAUD);
  node1.begin(deviceID, SerialM2);
  //delay(5); no necesario gracias a delay(timeMed_)

  int Suma = 0;
  int F_Err_Lin = 0;
  bool F_4Cuad = false;

  if ( NombreMed == "Velocidad") {
    Serial.println("NumMed: " + String(numReadReg)); //NUMMED determina el número de medidas que se van a tomar en el vector v_Med[nummed] (medidas continuas durante 10 segundos)
    Serial.println("Vel max alcanzada: " + String(WMaxPrev));
   
    //Serial.println("Velocidades almacenadas: ");
//    for (int s = 0; s < NumMed ; s++) {
//      Serial.print("[" + String(s) + "]:");
//      Serial.print(v_Median[s]);
//      Serial.print(" , ");
//    }
//    Serial.println();

    int v_Med[numReadReg]; //

    for (int j = 0; j < numReadReg; j++) //Obtiene numReadReg valores en (10) segundos y los guarda en v_Med
    {
      delay(500);

      resultRS485 = node1.readHoldingRegisters(base_reg, total_reg);

      if (resultRS485 == node1.ku8MBSuccess)
      {
        v_Med[j] = node1.getResponseBuffer(reg_med); //lee los valores de los distintos registros

        if (CorrecLin == true)
        {
          if (v_Med[j] > 4)
            F_Err_Lin++;
          if (v_Med[j] > 6) //Flag de 4º cuadrantre
            F_4Cuad = true;
        }


        if ( v_Med[j] > WMaxPrev)
        {
          WMaxPrev = v_Med[j];
          Serial.println("Nuevo Máximo de velocidad : " + String(WMaxPrev));
        }


        Serial.println("Medidas instantaneas " + String(NombreMed) + "[" + String(j) + "] : " + String(v_Med[j]));

      } else Serial.println(resultRS485, HEX);
    }



    if (resultRS485 == node1.ku8MBSuccess)
    {
      if (k > numMed_) //Si se ha completado 1 vector de Medias (v_Median) reiniciamos k y ponemos el flagFirst a 0
      {
        flagFirst = false;
        k = 0;
      }

      for (int j = 0; j < numReadReg; j++)
      {
        Suma += v_Med[j];
      }

      v_Median[k] = Suma / numReadReg; //Obtenemos la media de las medidas


      Suma = 0;

      Serial.println(">>>Media de " + String(NombreMed) + "    v_median:[" + String(k) + "] : " + String(v_Median[k]));

      if (flagFirst == true) //Si se trata de la primera generación de los vectores de MEDIANA
      {
        Serial.println(">>>>>>>>>>>>>>>>>>>FLAG FIRST<<<<<<<<<<<<<<<<<<<<<<");
        for (int h = k + 1; h < numMed_ ; h++)
        {
          //delay(50);
          //Serial.println("Valor de h: " + String(h));
          if (v_Median[h] == 0 || v_Median[h] == _MedPrev) //Si se trata del valor no inicializado o el valor impuesto anterior
          {
            v_Median[h] = v_Median[h - 1]; //rellenamos los campos vacios para que no salga mal la mediana
            // Serial.println("Valores de FLAGFIRST : " + String(v_Median[h]));

            // Serial.println("F_F_ v_median[" + String(h) + "]  :" + String(v_Median[h]));
          }
        }
        _MedPrev = v_Median[numMed_ - 1]; //toma el ultimo valor del vector como medida previa
      }

      for (int c = 0; c < numMed_; c++) {
        aux_v_median[c] = v_Median[c];
      }

      datas[reg_med] = QuickMedian<int>::GetMedian(aux_v_median, numMed_); //almacenamos la mediana en la memoria de datos


      Serial.println(">>>>>>Resultado final de " + String(NombreMed) + " : " + String(datas[reg_med]));

      k++;
      Serial.println();

      for (int i = 0 ; i < total_reg; i++) {
        Serial.print(String(datas[i]) + " ");
      }
    }
  }
  else
  {
    int v_Med[numMed_];
    Serial.println("NumMed: " + String(numMed_)); //NUMMED determina el número de medidas que se van a tomar en el vector v_Med[nummed] (medidas continuas durante 10 segundos)

    for (int j = 0; j < numMed_; j++) //Obtiene numMed_ valores en (timeMed * numMed) segundos y los guarda en v_Med
    {
      delay(timeMed_);

      resultRS485 = node1.readHoldingRegisters(base_reg, total_reg);

      if (resultRS485 == node1.ku8MBSuccess)
      {
        v_Med[j] = node1.getResponseBuffer(reg_med); //lee los valores de los distintos registros

        if (CorrecLin == true)
        {
          if (v_Med[j] > 4)
            F_Err_Lin++;
          if (v_Med[j] > 6) //Flag de 4º cuadrantre
            F_4Cuad = true;
        }

        Serial.println("Medidas instantaneas " + String(NombreMed) + "[" + String(j) + "] : " + String(v_Med[j]));

      } else Serial.println(resultRS485, HEX);
    }


    if (resultRS485 == node1.ku8MBSuccess)
    {
      if (k > numMed_) //Si se ha completado 1 vector de Medias (v_Median) reiniciamos k y ponemos el flagFirst a 0
      {
        flagFirst = false;
        k = 0;
      }

      if (CorrecLin == true)
      {
        if (F_Err_Lin > 0 && F_Err_Lin != numMed_ && F_4Cuad) //Si hay algun valor del 4 cuadrante junto con valores del cuadrante 1 y 2
        {
          Serial.println();
          Serial.println("Corrección de linealidad ");
          Serial.println();
          for (int j = 0; j < numMed_; j++)
          {
            if (v_Med[j] > 4) {
              v_Med[j] = v_Med[j] - valCorrec;
            }
          }
        }
      }

      for (int j = 0; j < numMed_; j++)
      {
        Suma += v_Med[j];
      }

      v_Median[k] = Suma / numMed_; //Obtenemos la media de las medidas

      if (CorrecLin == true)
      {
        if (v_Median[k] < 0)
          v_Median[k] = v_Median[k] + valCorrec;
      }

      Suma = 0;

      Serial.println(">>>Media de " + String(NombreMed) + " : " + String(v_Median[k]));

      if (flagFirst == true) //Si se trata de la primera generación de los vectores de MEDIANA
      {
        Serial.println(">>>>>>>>>>>>>>>>>>>FLAG FIRST<<<<<<<<<<<<<<<<<<<<<<");

        for (int h = k + 1; h < numMed_ ; h++)
        {
          //delay(50);
          Serial.println("Valor de h: " + String(h));
          if (v_Median[h] == 0 || v_Median[h] == _MedPrev) //Si se trata del valor no inicializado o el valor impuesto anterior
          {
            v_Median[h] = v_Median[h - 1]; //rellenamos los campos vacios para que no salga mal la mediana
            Serial.println("Valores de FLAGFIRST : " + String(v_Median[h]));
          }
        }
        _MedPrev = v_Median[numMed_ - 1]; //toma la medida previa del flagfirst como el último valor del vector
      }
      for (int c = 0; c < numMed_; c++) {
        aux_v_median[c] = v_Median[c];
      }


      datas[reg_med] = QuickMedian<int>::GetMedian(aux_v_median, numMed_); //almacenamos la mediana en la memoria de datos

      Serial.println(">>>>>>Resultado final de " + String(NombreMed) + " : " + String(datas[reg_med]));

      k++;
      Serial.println();

      for (int i = 0 ; i < total_reg; i++) {
        Serial.print(String(datas[i]) + " ");
      }
    }
  }
  Serial.println();

  SerialM2.end();
}


void MedidaConstSimult(float *datas, int deviceID, int total_reg, int base_reg, int v_regAddr[], int num_regAddr, int &k, int timeMed_, int numMed_, String NombreMed, bool CorrecLin, int v_valCorrec[] = 0)
{
  Serial.println("");
  Serial.println("Iniciando medición continua de : " + String(NombreMed));
  Serial.println("Valor de k para " + String(NombreMed) + " = " + String(k));
  Serial.println("NumMed: " + String(numMed_)); //NUMMED determina el número de medidas que se van a tomar en el vector v_Med[nummed] (medidas continuas durante 10 segundos)

  //  Serial.println("///////7ragADDR1: " + String(v_regAddr[0]));
  //  Serial.println("///////7ragADDR2: " + String(v_regAddr[1]));


  int m_Med[num_regAddr][numMed_];     //crea una matriz con tantas filas como registros a medir y tantas columnas como número de medidas
  //                                  MED1       MED2        MED3
  //              VEL       //REG1
  //        DIRECCION       //REG2
  //           GRADOS       //REG3

  int Suma[num_regAddr];

  for (int i = 0; i < num_regAddr; i++)
  {
    Suma[i] = 0;
    // Serial.println("///////////////////////////////////// NUMITEMS: "+ String(NUMITEMS(v_regAddr)));
    // Serial.println("///////////////////////////////////// SIZEOF: "+ String(sizeof(v_regAddr)));
  }

  SerialM2.begin(M2_BAUD);
  node1.begin(deviceID, SerialM2);
  delay(5);

  int F_Err_Lin = 0;
  bool F_4Cuad = false;

  for (int j = 0; j < numMed_; j++) //Obtiene numMed_ valores en 10 segundos y los guarda en v_Med
  {
    delay(timeMed_);

    // Serial.println("////////////////El valor de J es: " + String(j));

    resultRS485 = node1.readHoldingRegisters(base_reg, total_reg);

    if (resultRS485 == node1.ku8MBSuccess)
    {
      for (int i = 0; i < num_regAddr; i++)
      {
        m_Med[i][j] = node1.getResponseBuffer(v_regAddr[i]);
        //     Serial.println("/////////////////El valor de  m_Med[" + String(i) + "][" + String(j) + "] es: " + String( m_Med[i][j]));

        if (CorrecLin == true)
        {
          if (m_Med[i][j] > 4)
            F_Err_Lin++;
          if (m_Med[i][j] > 6) //Flag de 4º cuadrantre
            F_4Cuad = true;
        }

      }
      //   Serial.print("Medidas instantaneas " + String(NombreMed) + " : ");

      //      for (int i = 0; i < num_regAddr; i++)
      //      {
      //          Serial.print(m_Med[i][j]);
      //          Serial.print(" ; ");
      //      }
      //      Serial.println("");

    } else Serial.println(resultRS485, HEX);
  }
  if (resultRS485 == node1.ku8MBSuccess)
  {
    if (k > numMed_) //Si se ha completado 1 vector de Medias (v_Median) reiniciamos k y ponemos el flagFirst a 0
    {
      flagFirst = false;
      k = 0;
    }

    if (CorrecLin == true)
    {
      if (F_Err_Lin > 0 && F_Err_Lin != numMed_ && F_4Cuad) //Si hay algun valor del 4 cuadrante junto con valores del cuadrante 1 y 2
      {
        Serial.println();
        Serial.println("Realizando corrección de linealidad");
        Serial.println();

        for (int i = 0; i < num_regAddr; i++)
        {
          for (int j = 0; j < numMed_; j++)
          {
            if (m_Med[i][j] > 4)
              m_Med[i][j] = (m_Med[i][j]) - (v_valCorrec[i]);
          }
        }
      }
    }

    for (int i = 0; i < num_regAddr; i++)
    {
      for (int j = 0; j < numMed_; j++)
      {
        Suma[i] += m_Med[i][j];
      }
      m_Media[i][k] = Suma[i] / numMed_; //Obtenemos la media de las medidas

      if (CorrecLin == true)
      {
        if (m_Media[i][k] < 0)
          m_Media[i][k] = m_Media[i][k] + v_valCorrec[i];
      }
    }

    for (int i = 0; i < num_regAddr; i++)
    {
      Suma[i] = 0;
    }

    //  Serial.print(">>>Media de " + String(NombreMed) + " : ");

    for (int i = 0; i < num_regAddr; i++)
    {
      Serial.print(String(m_Media[i][k]));
      Serial.print(" ; ");
    }
    Serial.println("");

    if (flagFirst == true) //Si se trata de la primera generación de los vectores de MEDIANA  (se refiere a que el espacio a rellenar no se había rellenado con un valor propio anterior)
    {
      //Serial.println(">>>>>>>>>>>>>>>>>>>FLAG FIRST<<<<<<<<<<<<<<<<<<<<<<");

      for (int i = 0; i < num_regAddr; i++)
      {
        //   Serial.println("regAddr: " + String(i));
        for (int h = k + 1; h < numMed_ ; h++)
        {
          // delay(50);
          //     Serial.println("Valor de h: " + String(h));
          if (m_Media[i][h] == 0 || m_Media[i][h] == v_MedPrev[i]) //Si se trata del valor no inicializado o el valor impuesto anterior
          {
            m_Media[i][h] = m_Media[i][h - 1]; //rellenamos los campos vacios para que no salga mal la mediana
            //       Serial.println("Valores de FLAGFIRST : " + String(m_Media[i][h]));
          }
        }
        v_MedPrev[i] = m_Media[i][numMed_ - 1]; //toma la medida previa del flagfirst como el último valor del vector
      }
    }


    for ( int y = 0; y < 2; y ++)
    {
      for ( int v = 0; v < numMed_; v++ )
      {
        aux_m_Media[y][v] = m_Media[y][v];
      }
    }


    for (int i = 0; i < num_regAddr; i++)
    {
      datas[v_regAddr[i]] == QuickMedian<int>::GetMedian(aux_m_Media[i], numMed_); //almacenamos la mediana en la memoria de datos
      //    dataMedian[i]=QuickMedian<int>::GetMedian(m_Media[i],numMed_);  //almacenamos la mediana en la memoria de datos
    }

    Serial.print(">>>>>>Resultado final de " + String(NombreMed) + " : ");

    for (int i = 0; i < num_regAddr; i++)
    {
      //  Serial.print(String(dataMedian[i]));
      Serial.print(String(datas[v_regAddr[i]]));
      Serial.print(" ; ");
    }
    Serial.println();
    k++;

  }
  Serial.println();

  SerialM2.end();
}

uint8_t limpiaDisen(int deviceID) {
  Serial.println("Limpiando sensor...");

  SerialM2.begin(M2_BAUD);
  node1.begin(deviceID, SerialM2);
  delay(5);

  resultRS485 = node1.writeMultipleRegisters(CLEAN_REG, 0);

  if (resultRS485 == node1.ku8MBSuccess) {
    for (int i = 0; i < 20; i++) {
      Serial.print(".");
      delay(1000);
    }
    Serial.println();
    Serial.println("Limpieza completada");
    return 0;
  }
  else {
    Serial.println("Error al enviar el frame de limpieza");
    return 1;
  }
}


int read485HEX(float *datHEX, int deviceID, int total_reg, int base_reg) { //Obtiene datos formados por 2 registros de 16 bits que llegan en BIG_ENDIAN

  uint16_t regs[total_reg * 2]; //LOS DATOS ESTAN FORMADOS POR 2 REGISTROS DE 16 bits
  uint32_t def_value1 = 0;
  uint32_t def_value2 = 0;

  float *datosFloat = (float*)datHEX;
  //  float datosFloat[total_reg];


  SerialM2.begin(M2_BAUD);

  node1.begin(deviceID, SerialM2);
  Serial.println("Iniciando comunicación con ID: " + String(deviceID) + " empleando baudrate: " + String(M2_BAUD));
  delay(5);

  resultRS485 = node1.readHoldingRegisters(base_reg, total_reg);



  // Serial.println("Leyendo a partir del registro base: "+String(base_reg,HEX));
  resultRS485 = node1.readHoldingRegisters(base_reg, 4);
  Serial.println("resultado de lectura: " + String(resultRS485));

  delay(500);

  uint16_t bitHigh = 0;
  uint16_t bitLow = 0;

  uint16_t bigEnd = 0;

  if (resultRS485 == node1.ku8MBSuccess)
  {
    for (int j = 0; j < 4; j++)
    {
      regs[j] = node1.getResponseBuffer(j); //Obtiene la respuesta de las direcciones 0 y 1 procedentes de los 2 registros leidos
      Serial.print("j:" + String(j) + "  -  ");
      Serial.print(regs[j], HEX);
      Serial.print("    |    ");
      Serial.print(regs[j], BIN);


      //REORDENACION A BIG ENDIAN 1 (interior)
      bitHigh = regs[j];
      bitHigh = bitHigh << 8;

      bitLow = regs[j];
      bitLow = bitLow >> 8;

      regs[j] = bitHigh | bitLow;
      Serial.print("    |    ");
      Serial.print(regs[j], HEX);
      Serial.print("    |    ");
      Serial.println(regs[j], BIN);
    }

    //REORDENACION A BIG ENDIAN 2 (exterior)
    def_value1 = regs[1];
    def_value1 = def_value1 << 16; //desplaza el dato hacia la izk 16bits para introducirle en los 16 primeros el siguiente registro
    def_value1 = def_value1 + regs[0];  //Suma los valores formados por 2 reg formando el registro completo de 32bits

    def_value2 = regs[3];
    def_value2 = def_value2 << 16; //desplaza el dato hacia la izk 16bits para introducirle en los 16 primeros el siguiente registro
    def_value2 = def_value2 + regs[2];  //Suma los valores formados por 2 reg formando el registro completo de 32bits



    Serial.print(def_value1, HEX);
    Serial.print(" , ");
    Serial.println(def_value2, HEX);


    datosFloat[0] = HexToFloat(def_value1);
    datosFloat[1] = HexToFloat(def_value2);


    Serial.println("Valor 1 obtenido de hextofloat: " + String(datosFloat[0]));
    Serial.println("Valor 2 obtenido de hextofloat: " + String(datosFloat[1]));

#ifdef HC_DISEN_ADDR
    s_ValJSON[takeParam("T")] = datosFloat[0]; //TEMPERATURA
    s_ValJSON[takeParam("hC")] = datosFloat[1]; //valor de hidrocarburos en PPM
#endif
  }

  SerialM2.end();
  return resultRS485;
}


int read485data(float * datas, int deviceID, int total_reg, int base_reg) {
  SerialM2.begin(M2_BAUD);
  //Estacion meteorologica
  node1.begin(deviceID, SerialM2);
  delay(100);
  resultRS485 = node1.readHoldingRegisters(base_reg, total_reg);
  Serial.println("ku8MB: " + String(resultRS485));

  if (resultRS485 == node1.ku8MBSuccess)
  {
    for (int i = 0 ; i < total_reg; i++) {
      datas[i] = node1.getResponseBuffer(i);
      delay(300);
    }
    for (int i = 0 ; i < total_reg; i++) {
      Serial.print(String(datas[i]) + " ");
    }
    Serial.println();

#ifdef WEATHER_ADDR
    if (deviceID == WEATHER_ADDR)
    {
      MedidaConst(datas, deviceID, total_reg, base_reg, 0, v_median0, _k0, TimeMed, NumMed, "Velocidad", _MedPrev0, false);
      //  MedidaConst(datas, deviceID, total_reg,base_reg,2,v_median2,_k2,TimeMed, NumMed,"Direccion",_MedPrev2,true,8);
      //  MedidaConst(datas, deviceID, total_reg,base_reg,3,v_median3,_k3,TimeMed, NumMed,"Grados",_MedPrev3,true,360);
      //int  v_regAddr[]={2,3,4,5,6,7,8};
      int  v_regAddr[] = {2, 3}; //direcciones de las que se quiere obtener la medida constante simultanea
      int v_valCorrec[2] = {8, 360}; //valores de corrección de continuidad para las medidas de dirección del viento (0-8)(0-360)

      MedidaConstSimult(datas, deviceID, total_reg, base_reg, v_regAddr, NUMITEMS(v_regAddr), _k1, TimeMed, NumMed, "Direccion Grados", true, v_valCorrec);


#if defined(JSON_METEO) || defined(JSON_METEOFCAST)//ALMACENAMIENTO EN VECTORES DE VALORES PARA JSON METEO Y JSON METEOFORECAST
      s_ValJSON[takeParam("wSP")] = datas[0] / 10.0; //WindSpeed
      s_ValJSON[takeParam("wPW")] = datas[1];       //WindPwr
      s_ValJSON[takeParam("wD")] = datas[2];       //WindDir
      s_ValJSON[takeParam("wGD")] = datas[3];       //WindDirGrad
      s_ValJSON[takeParam("N")] = datas[6] / 10.0; //Noise
      s_ValJSON[takeParam("pm2")] = datas[7];      //pm2.5
      s_ValJSON[takeParam("pm10")] = datas[8];      //pm10
      s_ValJSON[takeParam("P1")] =  datas[9] / 10.0; //presion en Kpa
      s_ValJSON[takeParam("wSPM")] =  WMaxPrev / 10.0; //WindSpeedMAX
#endif
    }
#endif
#ifdef UV_ADDR
    if (deviceID == UV_ADDR)
    {
      s_ValJSON[takeParam("UV")] = datas[0] / 10.0; //UV mW/ cm2
      s_ValJSON[takeParam("UVi")] = datas[1]; //UV index 0-15
    }
#endif
  } else Serial.println(resultRS485, HEX);

  SerialM2.end();

  return resultRS485;
}


int clearRegister(uint16_t datas, int deviceID,  int _reg) {
  SerialM2.begin(M2_BAUD);
  node2.begin(deviceID, SerialM2);
  delay(5);
  resultRS485 = node2.writeSingleRegister(_reg, datas);
  SerialM2.end();
  return resultRS485;
}


#endif
