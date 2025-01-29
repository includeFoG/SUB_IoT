#ifndef Config_h
#define Config_h

//____________________________BOARD PARAMETERS___________________________________
#define DEVICE_NAME "SSM_DLG_PowerAnalyzer_0001"


#define ESP32_BAUDRATE   115200

#define MIKROE1_BAUDRATE   9600   //bc95G: 9600  //ESP32: 115200 //SIM808: 115200  //saraR410M/u201: 115200 //NRF9160:115200 //ESP32 htppfile: AT+UART_CUR=230423,8,1,0,1 //AT+UART_CUR=115200,8,1,0,1 CUR/DEF


#define USE_MIKROE_1

void initBoard();
void delayMs(int ms);
void delayMsUntil(TickType_t *previousWakeTime , int ms);

#endif //Config_h
