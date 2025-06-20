#define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

//#define WDT_TIME WDT_SOFTCYCLE16M
//#define WDT_TIME WDT_SOFTCYCLE8M
//#define WDT_TIME WDT_SOFTCYCLE4M
#define WDT_TIME WDT_SOFTCYCLE2M
//#define WDT_TIME WDT_SOFTCYCLE16S


#define RES_BOARD     100 //ESTA EN K
#define RES_INLINE    2000 //ESTA EN K


const char *APN     =  "internet.easym2m.eu";                                 //"hologram"//"internet.easym2m.eu"//"airtelnet.es"//
const char *APN_USER = "";
const char *APN_PASS  = "";


static const char * DEVICE =      "ssm-device_02";                               //   "ssm-device_02"                         |   "ssm-device_04"                          ||  "ssm-device_03"                       ||  "ssm-device_05"                       ||  "ssm-device_06"
#define PATH        "/api/v1.6/devices/"                                  //                                           |                                            ||                                        ||                                        ||
#define HOST        "industrial.api.ubidots.com"                          //                                           |                                            ||                                        ||                                        ||
#define TOKEN       "BBFF-ls5FeJFqGnSofHF0zA7hYiPMQ8TXau"                 //   "BBFF-ls5FeJFqGnSofHF0zA7hYiPMQ8TXau"   |   "BBFF-JXelYUpa5a9AwMe8pUVFsb731mJvLG"    ||  "BBFF-d2f6hRO3a4F8QY9RttGnIByrbKpjOm" || "BBUS-8FqATnVt1d33CDkF0z9ddRBOHTdXE8"  || "BBUS-mbIeUhPUCa4Evif6kjt1TTlo0iCzo7"
#define VAR_LABEL   "dvrswitchlabel"//"62ac532abe9c5814ec463aea" //"testswitchlabel" //"dvrswitchlabel" //ADDED usado para obtener datos de ubidots y encender DVR(falta añadir a config)


//CONEXION
#define AWS_PATH  "/test0/ftp-test-private/WP-test"                    //"/pruebas/mytest-003";        // "/test1/ftp-test-private/WP-test";
#define AWS_HOST  "qp29wxo1pf.execute-api.us-east-1.amazonaws.com"    //"qp29wxo1pf.execute-api.us-east-1.amazonaws.com";//"f8i1bdejsc.execute-api.us-east-1.amazonaws.com";


#define DVR_TIME_ON 30 //ADDED (minutos) tiempo que el DVR se mantiene encendido(falta añadir parametro a config)


#define NUM_VALORES 10
//#define ARCHIVO "data"

#define SENDFILE
#define RESET_WITH_RTC  //emplea tabla de horas
#define REALGPS
//#define DEBUG_MEASURE

const TickType_t SLOT_TIME = 60000;//normal: 60000, test: 20000;
#define FILE_TIME_SEND_TIMEOUT 200000

#define MEASURING_TIME 20 //tiempo midiendo (min) 20min
#define MEASURE_PERIOD 270 //NORMAL: 240(4h) tiempo entre medidas
#define SEND_PERIOD 10  //TEST ERROR :2 envio de datos ubidots
#define FILE_PERIOD 3 //3 tiempo de duracion de 1 archivo
#define FILE_TIME2 3 //SOLO PARA CONFIGAWS
//#define GPS_PERIOD 5 //NO SE ESTA UTILIZANDO !!!


#define RETRY_SEND_FILE 3

#define POWER_SENSOR_DELAY 5000

#define EXTENCION     ".csv"

#define RETRY_CONNECTION  2

#define SIZE_BUFFER 384
#define Q_DIV 1

//#define ROOTLOG   "DataLog"

//#define routFileSize  //length ruta archivo total: "/20220420/DataLog/.csv"




#define DATALOG "DataLog"

//char* EVENTSLOG = "EventsLog";




///////////////////////////////////////////////////////////////////////////////////MODIFIED//////////////////////////////////////////////////////////
//
//#define configName    "configRTO.txt"
//#define NUM_VALORES 12
//
//char DEF_APN[64]   = "internet.easym2m.eu";  //"hologram"//"internet.easym2m.eu"//"airtelnet.es"//"internet.easym2m.eu"
//char DEF_APN_USER[32] = "";
//char DEF_APN_PASS[32] = "";
//
//
//
//char DEF_DEVICE[32]  =   "ssm-device_03";
//char DEF_PATH[64]    =      "/api/v1.6/devices/";
//char DEF_HOST[64]  =     "industrial.api.ubidots.com";
//char DEF_TOKEN[64]  =     "BBFF-d2f6hRO3a4F8QY9RttGnIByrbKpjOm";
//
//char DEF_BUCKET[64]  =  "qp29wxo1pf.execute-api.us-east-1.amazonaws.com";
//char DEF_APIPATH[64] = "/test0/ftp-test-private/WP-test";
//
//char DEF_ARCHIVO[16] = "data";
//
////#define REALGPS
////#define DEBUG_MEASURE
//
//const TickType_t DEF_SLOT_TIME = 5000;
//
//
//
//#define DEF_MEASURING_TIME 10
//#define DEF_MEASURE_PERIOD 360
//#define DEF_SEND_PERIOD 5
//#define DEF_GPS_PERIOD 5
//#define DEF_FILE_PERIOD 5
//#define DEF_FILE_TIME2 5
//
//
//
//
//#define DEF_POWER_SENSOR_DELAY 2500
//
//char DEF_EXTENCION[16] = ".csv";
//
//#define DEF_RETRY_CONNECTION  5
//
//#define DEF_SIZE_BUFFER 512
//
//char DEF_ROOTLOG[16] = "DataLog";
//
//
//char DEF_DATALOG[16] = "DataLog";
//
//#define DEF_CHECK_UPDATE_TIME  1  //minutos
//
//typedef struct { //ADDED
//  char* APN;
//  char* APN_USER;
//  char* APN_PASS;
//
//  char* DEVICE;
//  char* PATH;
//  char* HOST;
//  char* TOKEN;
//  char* BUCKET;
//  char* APIPATH;
//
//  char* ARCHIVO;
//
//  TickType_t SLOT_TIME;
//
//  uint16_t MEASURING_TIME;
//  uint16_t MEASURE_PERIOD;
//  uint16_t SEND_PERIOD;
//  uint16_t GPS_PERIOD;
//  uint16_t FILE_PERIOD;
//  uint16_t FILE_TIME2;
//
//  int POWER_SENSOR_DELAY;
//
//  char* EXTENCION;
//
//  uint16_t RETRY_CONNECTION;
//
//  uint16_t SIZE_BUFFER;
//
//  char* ROOTLOG;
//
//  char* DATALOG;
//
//  int CHECK_UPDATE_TIME;
//} configData;
//
////configData defaultConfig{DEF_APN, DEF_APN_USER, DEF_APN_PASS, DEF_DEVICE, DEF_PATH, DEF_HOST, DEF_TOKEN, DEF_BUCKET, DEF_APIPATH, DEF_ARCHIVO, DEF_SLOT_TIME, DEF_MEASURING_TIME, DEF_MEASURE_PERIOD, DEF_SEND_PERIOD, DEF_GPS_PERIOD, DEF_FILE_PERIOD, DEF_FILE_TIME2, DEF_POWER_SENSOR_DELAY, DEF_EXTENCION, DEF_RETRY_CONNECTION, DEF_SIZE_BUFFER, DEF_ROOTLOG, DEF_DATALOG};
//configData defaultConfig{&DEF_APN[0], &DEF_APN_USER[0], &DEF_APN_PASS[0], &DEF_DEVICE[0], &DEF_PATH[0], &DEF_HOST[0], &DEF_TOKEN[0], &DEF_BUCKET[0], &DEF_APIPATH[0], &DEF_ARCHIVO[0], DEF_SLOT_TIME, DEF_MEASURING_TIME, DEF_MEASURE_PERIOD, DEF_SEND_PERIOD, DEF_GPS_PERIOD, DEF_FILE_PERIOD, DEF_FILE_TIME2, DEF_POWER_SENSOR_DELAY, &DEF_EXTENCION[0], DEF_RETRY_CONNECTION, DEF_SIZE_BUFFER, &DEF_ROOTLOG[0], &DEF_DATALOG[0],DEF_CHECK_UPDATE_TIME};
//
//configData localConfig;
//
//
//  void dynAllocCharStructConfig() {  //ADDED
//    localConfig.APN = (char*)malloc((sizeof(char) * 64));
//    localConfig.APN_USER = (char*)malloc((sizeof(char) * 32));
//    localConfig.APN_PASS = (char*)malloc((sizeof(char) * 32));
//    localConfig.DEVICE = (char*)malloc((sizeof(char) * 32));
//    localConfig.PATH = (char*)malloc((sizeof(char) * 64));
//    localConfig.HOST = (char*)malloc((sizeof(char) * 64));
//    localConfig.TOKEN = (char*)malloc((sizeof(char) * 64));
//    localConfig.BUCKET = (char*)malloc((sizeof(char) * 64));
//    localConfig.APIPATH = (char*)malloc((sizeof(char) * 64));
//    localConfig.ARCHIVO = (char*)malloc((sizeof(char) * 16));
//    localConfig.EXTENCION = (char*)malloc((sizeof(char) * 16));
//    localConfig.ROOTLOG = (char*)malloc((sizeof(char) * 16));
//    localConfig.DATALOG = (char*)malloc((sizeof(char) * 16));
//
//  //  localConfig.APN = (char*)calloc(32,(sizeof(char)));
//  //  localConfig.APN_USER = (char*)calloc(32,(sizeof(char)));
//  //  localConfig.APN_PASS = (char*)calloc(32,(sizeof(char)));
//  //  localConfig.DEVICE = (char*)calloc(32,(sizeof(char)));
//  //  localConfig.PATH = (char*)calloc(32,(sizeof(char)));
//  //  localConfig.HOST = (char*)calloc(32,(sizeof(char)));
//  //  localConfig.TOKEN = (char*)calloc(32,(sizeof(char)));
//  //  localConfig.BUCKET = (char*)calloc(32,(sizeof(char)));
//  //  localConfig.APIPATH = (char*)calloc(32,(sizeof(char)));
//  //  localConfig.ARCHIVO = (char*)calloc(32,(sizeof(char)));
//  //  localConfig.EXTENCION = (char*)calloc(32,(sizeof(char)));
//  //  localConfig.ROOTLOG = (char*)calloc(32,(sizeof(char)));
//  //  localConfig.DATALOG = (char*)calloc(32,(sizeof(char)));
//  }
