//#define CONFIG_ESP_INT_WDT_TIMEOUT_MS 10000 //MODIFICACION de watchdog ISR

#define DEVICE_NAME "SSM_DL_GAUGE_0000"//"SSM_DL_GAUGE_A001"
#define BLE_ID "SSM_DL_GAUGE_0000"
#define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0]))) //ADDED

#define VERSION "V.0.0.7.2"

#define CONFIG_LOG_DEFAULT_LEVEL 2 //3:Info, 4:debug->errores de rto  //BAJAR A 2 PARA EJECUTAR NORMAL

String SSID = "";
String SSID_PASS = "";



#define RESET_WITH_RTC //si se comenta al empezar inicia sesión pero después se amolda al vector de horas(siempre que tenga el RTC en hora), si no se comenta espera a que sea el vector el que le de el inicio
#if !defined(RESET_WITH_RTC)
#define FREQ_TIME  240//en caso de no estar definido el vector de horas, esta es la frecuencia entre medidas en minutos
#endif

//cuidado, si se baja el tiempo de medida por debajo de la ventana de tiempo se pueden duplicar archivos
#define MEASURING_TIME 20 //tiempo midiendo (min) 20min
#define TIME_WINDOW 5 //ventana de tiempo por si se reinicia el dispositivo o se incia en los primeros X minutos de una sesión
#define EXTENSION     ".csv"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define BAUD_SERIAL 115200

#define TIME_OUT_RS485 20      //tiempo en segundos que esperan los bucles a recibir mensajes por RS485

//__________________________________________________CHARGE__________________________________________________
bool CHOOSE_SLEEP_DURING_CHARGE = true; //esta variable decide si el dispositivo duerme durante la carga, en la ejecución solo puede pasar a false si el usuario se lo indica, si quiere volver a false tiene que reiniciar equipo
long lastTimeISRWire = 0; //DEBOUNCING esta variable guarda la última vez que se ejecutó una interrupción por conexión o desconexión del cable de alimentación para evitar rebotes
#define BLINK_CHARGE_FREQUENCE 10 //frecuencia de blink (segundos) cuando el dispositivo se encuentra cargando

//____________________________________________________________NVS__________________________________________________________
#define NVS_NAMESPACE  "NVS_VARS"
char nvs_key_filename[] = "nvs_intro";
int16_t  nvs_val_filename = 1; //almacena el valor introducido por el usuario para usarlo como nombre de archivo

//OFFSETS DE ACELERACIONES
char nvs_key_offset_aX[] = "nvs_off_aX";
char nvs_key_offset_aY[] = "nvs_off_aY";
char nvs_key_offset_aZ[] = "nvs_off_aZ";
int16_t nvs_val_offset_aX = 0;
int16_t nvs_val_offset_aY = 0;
int16_t nvs_val_offset_aZ = 0;

//OFFSET DE GIROSCOPIO
char nvs_key_offset_gX[] = "nvs_off_gX";
char nvs_key_offset_gY[] = "nvs_off_gY";
char nvs_key_offset_gZ[] = "nvs_off_gZ";
int16_t nvs_val_offset_gX = 0;
int16_t nvs_val_offset_gY = 0;
int16_t nvs_val_offset_gZ = 0;

//SENSIBILIDAD IMU
char nvs_key_sens_acel_IMU[] = "nvs_sensA";
char nvs_key_sens_giro_IMU[] = "nvs_sensG";
int16_t nvs_val_sens_acel_IMU = 0;  //0:+-2g ,1:+-4g ,2:+-8g ,3:+-16g
int16_t nvs_val_sens_giro_IMU = 0;  //0:+-250º/s ,1:+-500º/s ,2:+-1000º/s ,3:+-2000º/s


/*//FECHA CALIBRACION GIROSCOPIO
  char* nvs_key_lastIMUCal ="nvs_lastIMUCal";
  uint32_t nvs_val_lastIMUCal = 0;*/

/*//FECHA CALIBRACION GALGA
  char* nvs_key_lastSGCal ="nvs_lastSGCal";
  uint32_t nvs_val_lastSGCal = 0;*/


typedef struct { //estructura para guardar los valores ponderados y medidos(raw)
  int16_t measure_aX, measure_aY, measure_aZ;
  int16_t measure_gX, measure_gY, measure_gZ;
  int pond_aX, pond_aY, pond_aZ;
  int pond_gX, pond_gY, pond_gZ;
} IMUStruct;



//____________________________________________________________BT___________________________________________________________
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define TIME_BLE_WAIT_DEVICE 10 //tiempo en segundos que el ble espera a que se conecte un dispositivo antes de eliminar el hilo 



typedef enum
{
  CMD_SG_CALIB,     //0
  CMD_IMU_CALIB,    //1
  CMD_CFG_WIFI,     //2
  CMD_STOP_WIFI,    //3
  CMD_CFG_RTC,      //4
  CMD_VIEW_DATA,    //5
  CMD_DATA_SESION,  //6
  CMD_STOP_DATA,    //7
  CMD_END_BLE,      //8
  CMD_BAT_INFO,     //9
  ADV_ERROR         //10
} cmd_BLE;





//____________________________________________________________CONSTANTS____________________________________________________________
#define S_TO_uS_FACTOR 1000000ULL     /* Conversion factor for  seconds to micro seconds */
#define mS_TO_uS_FACTOR 1000ULL

//// Set BATTERY_CAPACITY to the design capacity of your battery.
const unsigned int BATTERY_CAPACITY = 10400; // e.g. 850mAh battery design capacity  para el fuelgauge



const char host[] = "esp32";
const char ntpServer1[] = "pool.ntp.org";
const char ntpServer2[] = "time.nist.gov";
const long  gmtOffset_sec = 0;         //0 Hora canaria GMT+0 | 3600 -> zona horaria +1
const int   daylightOffset_sec = 3600;    //3600 = uso de horario de verano usado para pasar UTC a GMT
const char time_zone[] = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)



const char name_batteryStatusFile[] = "/dataBattery.csv";
const char directoryBattery[] = "/BatteryInfo";

const char directorySession[] = "/Datalog";


//____________________________________________________________PIN DEFINITION____________________________________________________________
#define LED 36            //L2
#define RLED 1            //L1-R
#define GLED 2            //L1-G
#define BLED 6            //L1-B

#define CS_SD 10          //ChipSlect SD

#define PWR_IN 15         //Detecta si hay tensión de entrada para cargar la batería
#define PWR_IN_MASK 1ULL << PWR_IN  //mascara de PWR_IN

#define IRQ_IMU 16        //Interrupción IMU
#define IRQ_RTC 21        //Interrupción RTC
#define IRQ_FUEL 39       //Interrupción FuelGauge

#define PWR_RS485_EN 42   //Alimentación RS485 EXTERNO 

#define PWR_V3_EN 38      //Alimenta RS485 INTERNO, SD, IMU, PULL-UP I2C
#define DE_485    40      //Driver Output Enable  -> IF RE is HIGH and DE is low the IC go to lowPower shutdown mode
#define _RE_485   41      //Receiver Output Enable
// RO: Receiver Output -> conected in ESP32 to Rx
// DI: Driver Input -> conected in ESP32 to Tx

//!RE: Receiver Output Enabler -> RO enabled if !RE is low, if !RE (1) & DE (0) ->lowpower shutdown
// DE: Driver Output Enable -> Output enable if DE is high

/*  !RE   |   DE    |   STATUS
     0        0          Reception only
     0        1          Transmision                RECEPTION/TRANSMISION
     1        0          lowpower Shutdown
     1        1          ?                          TRANSMISION ONLY
*/


#define RXPIN 4         // GPIO 4 => RX for Serial1 or Serial2
#define TXPIN 5         // GPIO 5 => TX for Serial1 or Serial2
