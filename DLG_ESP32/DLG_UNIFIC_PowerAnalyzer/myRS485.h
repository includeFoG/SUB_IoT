#define MYRS485
#ifdef MYRS485

#include <ModbusMaster.h>
#include <QuickMedianLib.h>

#define MAX485_DE      6
#define MAX485_RE_NEG  6

// instantiate ModbusMaster object
ModbusMaster node3;

float HexToFloat(uint32_t x)
{
  return (*(float*)&x);
}

uint32_t FloatToHex(float x)
{
  return (*(uint32_t*)&x);
}

String err_RS = "0";


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


uint8_t err485[8]; //dispositivos: estacion meteo, raingauge, sensor de luz, amperimetros (4), analizadores de red
#endif
