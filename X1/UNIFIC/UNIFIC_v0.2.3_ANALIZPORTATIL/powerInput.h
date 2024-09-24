#ifndef SSM_POWER_INPUT
#define SSM_POWER_INPUT

#include <Adafruit_INA219.h>

//Adafruit_INA219 ina_input(0x40);
Adafruit_INA219 ina_input(0x41);

typedef struct {
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;
} inaData;

inaData powerIn;

#endif
