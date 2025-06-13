//GENERAL:
#define DEVICE_NAME "SSM_GAUGE_0000"
#define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define VERSION "V.0.0.6"


// USADOS EN gaugeCalibration:
#define CONFIG_TIME 10000          //tiempo que espera el setup para entrar en modo configuración
#define TIMEOUT_CALIBRATION 20000  //tiempo que espera el dispositivo a que el usuario introduzca consignas de configuracion por terminal serie
#define OFFSET_TIMES 100          //la cantidad de medidas de offset que van a usarse para hacer la media con un intervalo de 100ms entre ellas

#define MEASURE_FREQ 80 //frecuencia de medida en Hz

 //COMANDOS RS485:
#define COMMAND_CONFIG "$conf"

#define SERIAL_BAUD_RATE 115200
#define SERIAL1_BAUD_RATE 115200




//Structure to save the config of the strain gauge after a calibration
struct paramST{
  boolean valid;
  long offset;
  float scale;     //valor usado para pendiente de la recta
  uint8_t ID;     //ayuda a identificar (para debug) los objetos paramST al pasarlos a funciones y para saber si se tiene que leer valores de flash,
                  //OJO: A ID NO DAR VALOR 0 , se está usando para comprobar si es la primera vez que se ejecuta el programa para  paramStrain = myFlashST.read();
};

struct paramST paramStrain1 = {true, 3, 1, 0}; //NO MODIFICAR INDICES ID:0
struct paramST paramStrain2 = {true, 3, 1, 0}; 


#define RS485_FLOW 3  //pin flowcontrol RS485 H:Tx, L:Rx

#define RATE_SEL_STATUS true //true: 80Hz, false: 10Hz
#define RATE_SEL 4

#define SERIAL          Serial //Sparkfun Samd21 Boards
#define SERIAL1         Serial1 //Adafruit, other Samd21 Boards to RS485
