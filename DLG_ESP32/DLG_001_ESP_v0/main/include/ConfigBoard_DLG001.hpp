#ifndef ConfigBoard_DLG001_h
#define ConfigBoard_DLG001_h 

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h" 	
#include "DLG001_def.hpp"
#include "TCA6408A_lib.hpp"		//para uso del expansor GPIO TCA6408



#define DEVICE_NAME "SSM_DLG_PowerAnalyzer_0001"



//#define USE_MIKROE_1

//#define USE_ESP32_UART
#if defined(USE_ESP32_UART)
#define ESP32_BAUDRATE 115200 //uart ESP32
#endif

#ifdef USE_MIKROE_1
#define MIKROE1_BAUDRATE 115200
#endif
#ifdef USE_MIKROE_2
#define MIKROE2_BAUDRATE 115200
#endif



//void programStart();   not accessible out of Config
//void scanI2CDevices(); not accessible out of Config
//void initExpGPIOS();   not accessible out of Config
void initBoard();
esp_err_t selectUart(uint8_t);





// ***********************************************************************
//  [PowerManager] clase para gestionar las alimentaciones que la placa puede controlar:
// ***********************************************************************
/*
 * #define EN_VCC_MIKROE   3   //Enable Mikroe Ports 5V and 3.3V 
 * #define EN_VSys        16   //Enable Vcc out in ALL the JST Connectors
 * 
 * GPIO EXPANDER        | TCA6408A       | con R12(GND): b0100000, con R12(Vcc): b0100001 |  0x20, 0x21
 * 
 * |	TCA    |       TPS4H160      |               CN
 * ===============================================================
 * |	P0     |     DO_04 -> IN4    |    _DIG_OUT4 -> (S4) CN6 P6   
 * |	P1     |     DO_03 -> IN3    |    _DIG_OUT3 -> (S3) CN5 P6 
 * |	P2     |     DO_02 -> IN2    |    _DIG_OUT2 -> (S2) CN3 P6
 * |	P3     |     DO_01 -> IN1    |    _DIG_OUT1 -> (S1) CN1 P6  
 * |	P4     |          X          |    DI_01 -> _DIG_IN1 -> CN1 P5
 * |	P5     |          X          |    DI_02 -> _DIG_IN2 -> CN3 P5
 * |	P6     |          X          |    DI_03 -> _DIG_IN3 -> CN5 P5 
 * |	P7     |          X          |    DI_04 -> _DIG_IN4 -> CN6 P5  
 * 
 * 
 * powers los 8 primeros bits corresponden al TPS4H160, lo mismo ocurre con powerStates
 * los siguientes 8 bits son para periféricos externos, en este caso:
 * bit 8: MIKROE
 * bit 15: Vsys
 * 
 */
 
/*typedef union { // Flags
	 uint16_t total; // Variable que representa un byte completo "permite utilizar el mismo espacio de memoria para almacenar tanto el byte completo total como las banderas individuales"
	struct { // :1 representa un único bit , el orden de los bits lo determina el orden de la estructura
	     uint16_t f0  :1; // Flag  0 primer bit	LSB (l-e)
	     uint16_t f1  :1; // Flag  1
	     uint16_t f2  :1; // Flag  2
	     uint16_t f3  :1; // Flag  3
	     uint16_t f4  :1; // Flag  4
	     uint16_t f5  :1; // Flag  5
	     uint16_t f6  :1; // Flag  6
	     uint16_t f7  :1; // Flag  7 
	     uint16_t f8  :1; // Flag  8
	     uint16_t f9  :1; // Flag  9 
	     uint16_t f10 :1; // Flag 10 
	     uint16_t f11 :1; // Flag 11 
	     uint16_t f12 :1; // Flag 12 
	     uint16_t f13 :1; // Flag 13 
	     uint16_t f14 :1; // Flag 14 
	     uint16_t f15 :1; // Flag 15 ultimo bit	MSB (l-e)
	};
} wordFlags;

extern  wordFlags powerFlags; // Flags alimentaciones extern para que pueda ser compartida entre los distintos cpp*/

class PowerManager{ //<<singleton>>

  private:
    uint16_t powersDir;   //indica a la clase que salidas DOut son alimentaciones criterio: MSB donde 0b0001 equivale a la salida 1 y 0b1000 equivale a la salida 4, el bit 15 indica si se habilita Vsys y el 8 si se habilita la alimentación del mikroe
    uint16_t powerStates; //estados de alimentación criterio: MSB
    TCA6408A& expGPIOS;
    static bool initialized;   //indica si la clase ha sido inicializado previamente
    static bool initZeroFlag;  //indica si se ha configurado como si se iniciase de 0 -> necesidad por hardware no resetea TCA
    
    uint8_t configOutput(uint8_t indexToEnable, bool status); //gestiona las alimentaciones

    static uint8_t reverseByte(uint8_t byteToReverse);
    static uint8_t reverseIndex(uint8_t indexToReverse);

    //CONSTRUCTOR
    PowerManager(uint16_t powersDir, uint16_t initStates);

  public:
    //eliminamos el constructor de copia y asignación que produce por defecto el compilador
    PowerManager(const PowerManager&) = delete;
    PowerManager& operator=(const PowerManager&) = delete;


    //METODOS:
   static PowerManager& getInstance();              //obtiene la instancia del objeto
   void initialize(uint16_t powers, uint16_t initStates,bool initZero = false);   //inicializa el objeto
   
   uint8_t enablePower(uint8_t indexToEnable);      		//habilita la salida indicada (solo1 modo seguro) NIVEL ALTO
   uint8_t enableMultiPower(uint16_t bitsToEnable);     	//habilita las salidas indicadas NIVEL ALTO
   uint8_t disablePower(uint8_t indexToDisable);    		//deshabilita la salida indicada (solo1 modo seguro) NIVEL BAJO
  
   uint8_t disableMultiPower(uint16_t bitsToDisable);   	//deshabilita las salidas indicadas NIVEL BAJO
   uint8_t setPortPowerStates(uint16_t bitsStatus);		 	//Establece valores completos para los puertosPower TCA6408A y otras alimentaciones NIVEL ALTO Y NIVEL BAJO
   
   void showStatus(void); 									//Imprime powersDir y powerStates
};

#endif //ConfigBoard_DLG001_h