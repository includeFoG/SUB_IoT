/*Se pueden configurar distintos registros en función de los valores que se quieran monitorizar. Ver manual PM2200
 * TODOS los registros seleccionados son tipo Float de 32bits
 * POSIBLE CORRECCIÓN:¿?
 *        D+ -----.---------------RX+
          |
          | D- --.------------------RX-
          | |
          | | 100R
          | -////------TX+
          -----/////-----TX-
 */






 
//Conf WiFi
const char ssid[] = "AstWiFi"; //"ASTICAN_IOT"; "AstWiFi";
const char password[] ="WiFi2013"; //"em7cKNXVTH5KW6U7bpknyZZH"; "WiFi2013";

//CONFIGURACIÓN DISPOSITIVO
String NombreDisp="AnalizRed-1";

long FrecUp=6000;  //900000
long FrecAlarm0=FrecUp/2;
long FrecAlarm1=FrecAlarm0/2;
long FrecCrit=6000;  //60000


#define Total_Reg 15 //registros a leer TOTAL:15

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
#define Reg_Ap_E  2717 //PM2200 reg 2718 y 2719 Energía aparente entregada a la carga (kVAh) 
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


float DATA_SYNCROLIFT;
float DATA_METER_PM2200[Total_Reg];

String INF_ALARM[3]={"", "Error en la lectura de algún parámetro","ERROR DE COMUNICACIONES CON ANALIZADOR"};



String peticion="http://apidemo.muutech.com/astican.php?hostname=";
