#ifndef Config_h
#define Config_h

#include "DLG_def.h"
#include <Wire.h>
#include <shrPrta.h> //para uso del expansor GPIO TCA6408
//extern shrPrta expGPIOS; not accessible out of Config

#define DEBUG

#define USE_MIKROE_1

//#define USE_ESP32_UART
#if defined(USE_ESP32_UART)||defined(DEBUG)
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
void selectUart(uint8_t);




// ***********************************************************************
//  [PowerManager] clase para gestionar las alimentaciones que la placa puede controlar:
// ***********************************************************************
/*
 * #define EN_VCC_MIKROE   3   //Enable Mikroe Ports 5V and 3.3V 
 * #define EN_VSys        16   //Enable Vcc out in ALL the JST Connectors
 * 
 * GPIO EXPANDER        | TCA6408A       | con R12(GND): b0100000, con R12(Vcc): b0100001 |  0x20, 0x21
 * 
 * TCA    |       TPS4H160      |               CN
 * ============================================================
 * P0     |     DO_04 -> IN4    |      _DIG_OUT1 -> CN1 P6
 * P1     |     DO_03 -> IN3    |      _DIG_OUT2 -> CN3 P6
 * P2     |     DO_02 -> IN2    |      _DIG_OUT3 -> CN5 P6
 * P3     |     DO_01 -> IN1    |      _DIG_OUT4 -> CN6 P6
 * P4     |          X          |    DI_01 -> _DIG_IN1 -> CN1 P5
 * P5     |          X          |    DI_02 -> _DIG_IN2 -> CN3 P5
 * P6     |          X          |    DI_03 -> _DIG_IN3 -> CN5 P5 
 * P7     |          X          |    DI_04 -> _DIG_IN4 -> CN6 P5  
 * 
 * 
 * powers los 8 primeros bits corresponden al TPS4H160, lo mismo ocurre con powerStates
 * los siguientes 8 bits son para periféricos externos, en este caso:
 * bit 8: MIKROE
 * bit 15: Vsys
 * 
 */

class PowerManager { //<<singleton>>

  private:
    uint16_t powers;      //indica a la clase que salidas DOut son alimentaciones criterio: MSB donde 0b0001 equivale a la salida 1 y 0b1000 equivale a la salida 4, el bit 15 indica si se habilita Vsys y el 8 si se habilita la alimentación del mikroe
    uint16_t powerStates; //estados de alimentación criterio: MSB
    shrPrta& expGPIOS;
    static bool initialized;   //indica si la clase ha sido inicializado previamente
    static bool initZeroFlag;      //indica si se ha configurado como si se iniciase de 0 -> necesidad por hardware no resetea TCA

    static uint8_t reverseByte(byte byteToReverse);

    //CONSTRUCTOR
    PowerManager(uint16_t powers, uint16_t initStates);

  public:
    //eliminamos el constructor de copia y asignación que produce por defecto el compilador
    PowerManager(const PowerManager&) = delete;
    PowerManager& operator=(const PowerManager&) = delete;

    //METODOS:
   static PowerManager& getInstance();              //obtiene la instancia del objeto
   void initialize(uint16_t powers, uint16_t initStates,bool initZero = false);   //inicializa el objeto
   
   void enablePower(uint16_t indexToEnable);      //habilita la salida indicada (solo1 modo seguro)
   void enablePowers(uint16_t indexToEnable);     //habilita las salidas indicadas
   void disablePower(uint16_t indexToDisable);    //deshabilita la salida indicada (solo1 modo seguro)
   void disablePowers(uint16_t indexToDisable);   //deshabilita las salidas indicadas
   void setPower(uint16_t powerStatus);           //establece el estado de las salidas como el indicado
};

#endif //Config_h
