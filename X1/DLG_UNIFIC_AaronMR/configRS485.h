
/*Se pueden configurar distintos registros en función de los valores que se quieran monitorizar. Ver manual PM2200
  TODOS los registros seleccionados son tipo Float de 32bits
  POSIBLE CORRECCIÓN:¿?
         D+ -----.---------------RX+
         |
         | D- --.------------------RX-
         | |
         | | 100R
         | -////------TX+
         -----/////-----TX-
*/







//Conf WiFi
const char ssid[] = "AstWiFi"; //"ASTICAN_IOT"; "AstWiFi";
const char password[] = "WiFi2013"; //"em7cKNXVTH5KW6U7bpknyZZH"; "WiFi2013";

//CONFIGURACIÓN DISPOSITIVO
String NombreDisp = "AnalizRed-1";

long FrecUp = 6000; //900000
long FrecAlarm0 = FrecUp / 2;
long FrecAlarm1 = FrecAlarm0 / 2;
long FrecCrit = 6000; //60000


#define Total_Reg 15 //registros a leer TOTAL:15

#define Total_Reg_RK900 9 //registros a leer del RK900 son 9  (dir viento, vel viento, temp, hum, rainfall, pres at, pm2.5, pm10, noise)
#define Total_Reg_RK300 2 //registros a leer del RK300 son 2   (CO2 y VOC)


//registros
#define Reg_V_FF_AVG 3025  //PM2200 reg 3026 y 3027- 0xBDC Tensión Compuesta FF promedio (Volt)
#define Reg_Freq     3109  //PM2200 reg 3110 y 3110- 0xC26 Frecuencia (Hz)

#define Reg_I_A 2999   //PM2200 reg 3000 y 3001- 0xBB8 Corrientes de fase (Amp)*
#define Reg_I_B 3001   //PM3002 reg 3002 y 3003- 0xBBA*
#define Reg_I_C 3003   //PM2200 reg 3004 y 3005- 0xBBC*
#define Reg_I_N 3005   //PM2200 reg 3006 y 3007- 0xBBE Corriente neutro*
#define Reg_I_G 3007   //PM2200 reg 3008 y 3009- 0xBC0 Corriente de fuga a tierra? *
#define Reg_I_AVG 3009 //PM2200 reg 3010 y 3011- 0xBC2 Promedio de corriente



//nuevos registros
#define Reg_P_Act 3059   //PM2200 reg 3060 y 3061 Potencia activa total
#define Reg_P_Rea 3067   //PM3002 reg 3068 y 3069 Potencia reactiva total
#define Reg_P_Ap  3075   //PM2200 reg 3076 y 3077 Potencia aparente total
#define Reg_4Q_FP_PF 3083   //PM2200 reg 3084 y 3085 Factor de potencia total

#define Reg_Act_E 2699 //PM2200 reg 2700 y 2701 Energía Activa entregada a la carga (kWh) //NO ES 2676, posibles:2700 y 3204(INT64)
#define Reg_Rea_E 2707 //PM2200 reg 2708 y 2709 Energía Reactiva entregada a la carga (kVARh) 
#define Reg_Ap_E  2715 //PM2200 reg 2718 y 2719 Energía aparente entregada a la carga (kVAh) 
//registros 64bts
//#define Reg_E_Act_Del 3203   //PM2200 reg 3204, 3205, 3206 y 3207 Energia activa entregada
//#define Reg_E_Rea_Del 3219   //PM2200 reg 3220, 3221, 3222 y 3223 Energia reactiva entregada
//#define Reg_E_Ap_Del  3235   //PM2200 reg 3236, 3237, 3238 y 3239 Energia aparente entregada



uint16_t Reg_addr[Total_Reg] =
{
  Reg_V_FF_AVG,
  Reg_Freq,
  Reg_I_A,
  Reg_I_B,
  Reg_I_C,
  Reg_I_N,
  Reg_I_G,
  Reg_I_AVG,

  Reg_Act_E,
  Reg_Rea_E,
  Reg_Ap_E,

  Reg_P_Act,
  Reg_P_Rea,
  Reg_P_Ap,
  Reg_4Q_FP_PF

};





typedef struct {
  String nombre;
  uint16_t address;
  uint8_t num_reg;
  uint8_t type;
  float valor;
} RegType;

RegType Reg_addr_type_RK900[Total_Reg_RK900] = {
  {"WD", 1, 1, 0, 0.1}, // direccion viento
  {"WS", 3, 2, 2, 0.2}, // velocidad viento
  {"T", 5, 2, 2, 0.3}, // temp
  {"H", 7, 2, 2, 0.4}, // humedad
  {"P", 9, 2, 2, 0.5}, // presion atm
  {"R", 13, 2, 2, 0.6}, // Rainfall
  {"pm2", 26, 2, 2, 0.7}, // pm2.5
  {"pm10", 48, 2, 2, 0.8}, // pm10
  {"N", 97, 2, 2, 0.9}  // noise
};

RegType Reg_addr_type_RK300[Total_Reg_RK300] = {
  {"CO2", 6, 1, 0, 0.1},  // multiplier is 1
  {"VOC", 14, 1, 0, 0.2}, // multiplier is 0.001
};

//    "config": ["baud":9600,"Time":10, "config_1":1,"config_2":2],

char jsonConfig[] PROGMEM = R"rawliteral(
{
  "system": {
    "devices": [
      {
        "ID": 2,
        "name": "RK900",
        "registers": [
          {"Name": "WD",    "valor": 0, "address": 1,   "size": 1, "type": 0},
          {"Name": "WS",    "valor": 0, "address": 3,   "size": 2, "type": 2},
          {"Name": "T",     "valor": 0, "address": 5,   "size": 2, "type": 2},
          {"Name": "H",     "valor": 0, "address": 7,   "size": 2, "type": 2},
          {"Name": "P",     "valor": 0, "address": 9,   "size": 2, "type": 2},
          {"Name": "R",     "valor": 0, "address": 13,  "size": 2, "type": 2},
          {"Name": "pm2",   "valor": 0, "address": 26,  "size": 2, "type": 2},
          {"Name": "pm10",  "valor": 0, "address": 48,  "size": 2, "type": 2},
          {"Name": "N",     "valor": 0, "address": 97,  "size": 2, "type": 2}
        ]
      },
      {
        "ID": 1,
        "name": "RK300",
        "registers": [
          {"Name": "CO2",   "valor": 0, "address": 6,   "size": 1, "type": 0},
          {"Name": "VOC",   "valor": 0, "address": 14,  "size": 1, "type": 0}
        ]
      }
    ]
  }
}
)rawliteral";
// lector 4-20ma 8ch  -  Leer canal 1, ID3 -  03 03 00 00 00 01 85 E8  , la respuesta = 03 03 02 17 9D 0F DD  donde 


/*

 ,
      {
        "ID": 3, // 4-20ma 8ch
        "name": "4-20mA",
        "registers": [
          {"Name": "Ch_1",   "valor": 0, "address": 0,   "size": 1, "type": 0}
        ]
      }
 
 */






/*
RegType devices[2] = {
  Reg_addr_type_RK900, 
  Reg_addr_type_RK300
}
*/


uint16_t Reg_addr_RK900[Total_Reg_RK900] =
{
  0,
  2,
  3,
  5,
  7,
  13,
  26,
  48,
  97
};


float DATA_SYNCROLIFT;
float DATA_METEO_RK900[Total_Reg_RK900 + 1];
float DATA_METEO_RK300[Total_Reg_RK300 + 1];
float DATA_METER_PM2200[Total_Reg];
String INF_ALARM[3] = {"", "Error en la lectura de algún parámetro", "ERROR DE COMUNICACIONES CON ANALIZADOR"};



String peticion = "http://apidemo.muutech.com/astican.php?hostname=";
