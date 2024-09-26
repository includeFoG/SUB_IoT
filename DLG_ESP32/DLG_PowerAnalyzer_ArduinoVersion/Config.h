#ifndef Config_h
#define Config_h

#include "DLG_def.h"

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



//void programStart();
void initBoard();
void selectUart(uint8_t);

#endif //Config_h
