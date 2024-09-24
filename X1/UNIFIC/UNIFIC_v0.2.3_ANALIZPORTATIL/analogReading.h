//#include <Wire.h>
//#include <SPI.h>
#include <MCP342x.h>


uint8_t address = 0x68;
MCP342x adc = MCP342x(address);

#ifdef SSM_005
uint8_t address2 = 0x6C;
MCP342x adc2 = MCP342x(address2);
#endif

long adcVal1 = 0;
float adcV1 = 0;
long adcVal2 = 0;
float adcV2 = 0;
long adcVal3 = 0;
float adcV3 = 0;
long adcVal4 = 0;
float adcV4 = 0;

float currentCh1 = 0;
float currentCh2 = 0;
float currentCh3 = 0;
float currentCh4 = 0;

float sensCh1 = 0;
float sensCh2 = 0;
float sensCh3 = 0;
float sensCh4 = 0;


#ifdef SSM_005
long adcVal5 = 0;
float adcV5 = 0;
long adcVal6 = 0;
float adcV6 = 0;
long adcVal7 = 0;
float adcV7 = 0;
long adcVal8 = 0;
float adcV8 = 0;

float currentCh5 = 0;
float currentCh6 = 0;
float currentCh7 = 0;
float currentCh8 = 0;

float sensCh5 = 0;
float sensCh6 = 0;
float sensCh7 = 0;
float sensCh8 = 0;
#endif


uint8_t err ;
MCP342x::Config status;
#define MAX_ANA_RETRY   5
#define ERROR_MEDIDA 1

int err_analog = 0;
int retry_analog = 0;

float map4_20(float _data, float _minVal, float _maxVal) {
  float temp = (_data - 4) * (_maxVal - _minVal) / (20 - 4) + _minVal;
  Serial.print("ADC: "); Serial.print(_data); Serial.print(", Maped: "); Serial.println(temp);
  return temp;
}

bool isInRange(float _Val, float _minVal, float _maxVal, int _error) {
  float margen = (_maxVal - _minVal) * _error / 100;
  return (((_maxVal + margen) >= _Val) && (_Val >= (_minVal - margen)));
}

int DesviaMedida(float medidaActu, float medidaPrev, int SensCh )
{
  float aux_SUP = 0;
  float aux_INF = 0;
  int ret = 0;

  switch (SensCh)
  {
    case 1:
      aux_SUP = (medidaPrev + (((localAnaMap.maxSensch1 - localAnaMap.minSensch1) * localConfig.desviacion) / 100.0));
      aux_INF = (medidaPrev - (((localAnaMap.maxSensch1 - localAnaMap.minSensch1) * localConfig.desviacion) / 100.0));
      ret = 1;
      break;
    case 2:
      aux_SUP = (medidaPrev + (((localAnaMap.maxSensch2 - localAnaMap.minSensch2) * localConfig.desviacion) / 100.0));
      aux_INF = (medidaPrev - (((localAnaMap.maxSensch2 - localAnaMap.minSensch2) * localConfig.desviacion) / 100.0));
      ret = 2;
      break;
    case 3:
      aux_SUP = (medidaPrev + (((localAnaMap.maxSensch3 - localAnaMap.minSensch3) * localConfig.desviacion) / 100.0));
      aux_INF = (medidaPrev - (((localAnaMap.maxSensch3 - localAnaMap.minSensch3) * localConfig.desviacion) / 100.0));
      ret = 3;
      break;
    case 4:
      aux_SUP = (medidaPrev + (((localAnaMap.maxSensch4 - localAnaMap.minSensch4) * localConfig.desviacion) / 100.0));
      aux_INF = (medidaPrev - (((localAnaMap.maxSensch4 - localAnaMap.minSensch4) * localConfig.desviacion) / 100.0));
      ret = 4;
      break;
    case 5:
      aux_SUP = (medidaPrev + (((localAnaMap.maxSensch5 - localAnaMap.minSensch5) * localConfig.desviacion) / 100.0));
      aux_INF = (medidaPrev - (((localAnaMap.maxSensch5 - localAnaMap.minSensch5) * localConfig.desviacion) / 100.0));
      ret = 5;
      break;
    case 6:
      aux_SUP = (medidaPrev + (((localAnaMap.maxSensch6 - localAnaMap.minSensch6) * localConfig.desviacion) / 100.0));
      aux_INF = (medidaPrev - (((localAnaMap.maxSensch6 - localAnaMap.minSensch6) * localConfig.desviacion) / 100.0));
      ret = 6;
      break;
    case 7:
      aux_SUP = (medidaPrev + (((localAnaMap.maxSensch7 - localAnaMap.minSensch7) * localConfig.desviacion) / 100.0));
      aux_INF = (medidaPrev - (((localAnaMap.maxSensch7 - localAnaMap.minSensch7) * localConfig.desviacion) / 100.0));
      ret = 7;
      break;
    case 8:
      aux_SUP = (medidaPrev + (((localAnaMap.maxSensch8 - localAnaMap.minSensch8) * localConfig.desviacion) / 100.0));
      aux_INF = (medidaPrev - (((localAnaMap.maxSensch8 - localAnaMap.minSensch8) * localConfig.desviacion) / 100.0));
      ret = 8;
      break;
  }
  Serial.println("Sens: " + String(SensCh) + "  MedidaActu: " + String(medidaActu) + " MAX:" + String(aux_SUP) + " MIN: " + String(aux_INF));

  if (medidaActu >= aux_SUP || medidaActu <= aux_INF)
    return ret;
  return 0;
}

void initADCRead() {
  Wire.begin();
  delay(1);
  MCP342x::generalCallReset();
  delay(1);
}

int analogMCPRead() {
  initADCRead();
  int cont_err = 0;

  err = adc.convertAndRead(MCP342x::channel2, MCP342x::oneShot,
                           MCP342x::resolution18, MCP342x::gain1,
                           1000000, adcVal1, status);
  if (err != 0) return 5;
  delay(1); // MC342x needs 300us to settle, wait 1ms
  err = adc.convertAndRead(MCP342x::channel1, MCP342x::oneShot,
                           MCP342x::resolution18, MCP342x::gain1,
                           1000000, adcVal2, status);

  if (err != 0) return 6;
  delay(1);
  err = adc.convertAndRead(MCP342x::channel4, MCP342x::oneShot,
                           MCP342x::resolution18, MCP342x::gain1,
                           1000000, adcVal3, status);

  if (err != 0) return 7;
  delay(1); // MC342x needs 300us to settle, wait 1ms
  err = adc.convertAndRead(MCP342x::channel3, MCP342x::oneShot,
                           MCP342x::resolution18, MCP342x::gain1,
                           1000000, adcVal4, status);

  if (err != 0) return 8;


#ifdef SSM_005
  delay(1);
  err = adc2.convertAndRead(MCP342x::channel2, MCP342x::oneShot,
                            MCP342x::resolution18, MCP342x::gain1,
                            1000000, adcVal5, status);

  if (err != 0) return 9;
  delay(1); // MC342x needs 300us to settle, wait 1ms
  err = adc2.convertAndRead(MCP342x::channel1, MCP342x::oneShot,
                            MCP342x::resolution18, MCP342x::gain1,
                            1000000, adcVal6, status);
  if (err != 0) return 10;
  delay(1);
  err = adc2.convertAndRead(MCP342x::channel4, MCP342x::oneShot,
                            MCP342x::resolution18, MCP342x::gain1,
                            1000000, adcVal7, status);
  if (err != 0) return 11;
  delay(1); // MC342x needs 300us to settle, wait 1ms
  err = adc2.convertAndRead(MCP342x::channel3, MCP342x::oneShot,
                            MCP342x::resolution18, MCP342x::gain1,
                            1000000, adcVal8, status);
  if (err != 0) return 12;
#endif


  adcV1 = adcVal1 * 0.015625;
  adcV2 = adcVal2 * 0.015625;
  adcV3 = adcVal3 * 0.015625;
  adcV4 = adcVal4 * 0.015625;

#ifdef SSM_005
  adcV5 = adcVal5 * 0.015625;
  adcV6 = adcVal6 * 0.015625;
  adcV7 = adcVal7 * 0.015625;
  adcV8 = adcVal8 * 0.015625;
#endif

  currentCh1 = adcV1 / 100.0;
  currentCh2 = adcV2 / 100.0;
  currentCh3 = adcV3 / 100.0;
  currentCh4 = adcV4 / 100.0;

#ifdef SSM_005
  currentCh5 = adcV5 / 100.0;
  currentCh6 = adcV6 / 100.0;
  currentCh7 = adcV7 / 100.0;
  currentCh8 = adcV8 / 100.0;
#endif

  sensCh1 = map4_20(currentCh1, localAnaMap.minSensch1, localAnaMap.maxSensch1);
  sensCh2 = map4_20(currentCh2, localAnaMap.minSensch2, localAnaMap.maxSensch2);
  sensCh3 = map4_20(currentCh3, localAnaMap.minSensch3, localAnaMap.maxSensch3);
  sensCh4 = map4_20(currentCh4, localAnaMap.minSensch4, localAnaMap.maxSensch4);

#if defined(JSON_ANALOG003) || defined(JSON_ANALOG005)
  if (localAnaMap.usedch1)
    s_ValJSON[takeParam("S1")] = sensCh1;
  if (localAnaMap.usedch2)
    s_ValJSON[takeParam("S2")] = sensCh2;
  if (localAnaMap.usedch3)
    s_ValJSON[takeParam("S3")] = sensCh3;
  if (localAnaMap.usedch4)
    s_ValJSON[takeParam("S4")] = sensCh4;
#endif



#ifdef SSM_005
  sensCh5 = map4_20(currentCh5, localAnaMap.minSensch5, localAnaMap.maxSensch5);
  sensCh6 = map4_20(currentCh6, localAnaMap.minSensch6, localAnaMap.maxSensch6);
  sensCh7 = map4_20(currentCh7, localAnaMap.minSensch7, localAnaMap.maxSensch7);
  sensCh8 = map4_20(currentCh8, localAnaMap.minSensch8, localAnaMap.maxSensch8);

#if defined(JSON_ANALOG005)
  if (localAnaMap.usedch5)
    s_ValJSON[takeParam("S5")] = sensCh5;
  if (localAnaMap.usedch6)
    s_ValJSON[takeParam("S6")] = sensCh6;
  if (localAnaMap.usedch7)
    s_ValJSON[takeParam("S7")] = sensCh7;
  if (localAnaMap.usedch7)
    s_ValJSON[takeParam("S8")] = sensCh8;

#endif

#endif


  //#if defined(JSON_ANALOG003) || defined(JSON_ANALOG005)
  //
  //  s_ValAnalog[4] = sensCh1;
  //  s_ValAnalog[5] = sensCh2;
  //  s_ValAnalog[6] = sensCh3;
  //  s_ValAnalog[7] = sensCh4;
  //#endif
  //
  //#ifdef JSON_ANALOG005  //VALORES PARA PLACA SSM-005
  //  s_ValAnalog[8] = sensCh5;
  //  s_ValAnalog[9] = sensCh6;
  //  s_ValAnalog[10] = sensCh7;
  //  s_ValAnalog[11] = sensCh8;
  //#endif

  if (!isInRange(sensCh1, localAnaMap.minSensch1, localAnaMap.maxSensch1, ERROR_MEDIDA) && localAnaMap.usedch1) cont_err ++;
  if (!isInRange(sensCh2, localAnaMap.minSensch2, localAnaMap.maxSensch2, ERROR_MEDIDA) && localAnaMap.usedch2) cont_err ++;
  if (!isInRange(sensCh3, localAnaMap.minSensch3, localAnaMap.maxSensch3, ERROR_MEDIDA) && localAnaMap.usedch3) cont_err ++;
  if (!isInRange(sensCh4, localAnaMap.minSensch4, localAnaMap.maxSensch4, ERROR_MEDIDA) && localAnaMap.usedch4) cont_err ++;

  Serial.print("Valor ADC1: "); Serial.print(adcVal1); Serial.print(", Valor V1: "); Serial.print(adcV1); Serial.print(", Valor ch1 4-20mA: "); Serial.print(currentCh1); Serial.print(", Sensor1: "); Serial.println(sensCh1);
  Serial.print("Valor ADC2: "); Serial.print(adcVal2); Serial.print(", Valor V2: "); Serial.print(adcV2); Serial.print(", Valor ch2 4-20mA: "); Serial.print(currentCh2); Serial.print(", Sensor2: "); Serial.println(sensCh2);
  Serial.print("Valor ADC3: "); Serial.print(adcVal3); Serial.print(", Valor V3: "); Serial.print(adcV3); Serial.print(", Valor ch3 4-20mA: "); Serial.print(currentCh3); Serial.print(", Sensor3: "); Serial.println(sensCh3);
  Serial.print("Valor ADC4: "); Serial.print(adcVal4); Serial.print(", Valor V4: "); Serial.print(adcV4); Serial.print(", Valor ch4 4-20mA: "); Serial.print(currentCh4); Serial.print(", Sensor4: "); Serial.println(sensCh4);


#ifdef SSM_005
  if (!isInRange(sensCh5, localAnaMap.minSensch5, localAnaMap.maxSensch5, ERROR_MEDIDA) && localAnaMap.usedch5) cont_err ++;
  if (!isInRange(sensCh6, localAnaMap.minSensch6, localAnaMap.maxSensch6, ERROR_MEDIDA) && localAnaMap.usedch6) cont_err ++;
  if (!isInRange(sensCh7, localAnaMap.minSensch7, localAnaMap.maxSensch7, ERROR_MEDIDA) && localAnaMap.usedch7) cont_err ++;
  if (!isInRange(sensCh8, localAnaMap.minSensch8, localAnaMap.maxSensch8, ERROR_MEDIDA) && localAnaMap.usedch8) cont_err ++;

  Serial.print("Valor ADC5: "); Serial.print(adcVal5); Serial.print(", Valor V5: "); Serial.print(adcV5); Serial.print(", Valor ch5 4-20mA: "); Serial.print(currentCh5); Serial.print(", Sensor5: "); Serial.println(sensCh5);
  Serial.print("Valor ADC6: "); Serial.print(adcVal6); Serial.print(", Valor V6: "); Serial.print(adcV6); Serial.print(", Valor ch6 4-20mA: "); Serial.print(currentCh6); Serial.print(", Sensor6: "); Serial.println(sensCh6);
  Serial.print("Valor ADC7: "); Serial.print(adcVal7); Serial.print(", Valor V7: "); Serial.print(adcV7); Serial.print(", Valor ch7 4-20mA: "); Serial.print(currentCh7); Serial.print(", Sensor7: "); Serial.println(sensCh7);
  Serial.print("Valor ADC8: "); Serial.print(adcVal8); Serial.print(", Valor V8: "); Serial.print(adcV8); Serial.print(", Valor ch8 4-20mA: "); Serial.print(currentCh8); Serial.print(", Sensor8: "); Serial.println(sensCh8);
#endif



  Wire.end();
  return cont_err;
}







//float readADCChannel_4-20(int _channel){
//  long _adcVal=0;
//  float _adcV=0;
//  float _currentCh = 0;
//  float _sensCh = 0;
//  switch (_channel){
//    case 1:
//    err = adc.convertAndRead(MCP342x::channel3, MCP342x::oneShot,
//    MCP342x::resolution18, MCP342x::gain1,
//    1000000, _adcVal, status);
//    break;
//    case 2:
//    err = adc.convertAndRead(MCP342x::channel4, MCP342x::oneShot,
//    MCP342x::resolution18, MCP342x::gain1,
//    1000000, _adcVal, status);
//    break;
//    case 3:
//    err = adc.convertAndRead(MCP342x::channel1, MCP342x::oneShot,
//    MCP342x::resolution18, MCP342x::gain1,
//    1000000, _adcVal, status);
//    break;
//    case 4:
//    err = adc.convertAndRead(MCP342x::channel2, MCP342x::oneShot,
//    MCP342x::resolution18, MCP342x::gain1,
//    1000000, _adcVal, status);
//    break;
//  }
//  _adcV=adcVal*0.015625;
//  return _adcV/100.0;
//}
