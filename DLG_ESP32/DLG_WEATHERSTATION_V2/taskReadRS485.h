
//*****************************************************************
// Thread TaskReadRS485
//*****************************************************************
#include <ModbusMaster.h> //"libraries/ModbusMaster/src/ModbusMaster.h"  

#include <vector>
#include <tuple>

TaskHandle_t readRS485_task_handle;

ModbusMaster node;

void preTransmission()
{
  digitalWrite(FC_485, HIGH);
  digitalWrite(FC_485, HIGH);
}

void postTransmission()
{
  digitalWrite(FC_485, LOW);
  digitalWrite(FC_485, LOW);
}

void initCallbacks(ModbusMaster* _node) {
  _node->preTransmission(preTransmission);
  _node->postTransmission(postTransmission);
}


#define RS485_BAUDRATE     9600    //estacion meteo Rika: baudrate:9600bps
#define ADDR_WS_RIKA       0x010   //dirección modbus estación meteo rika
#define ADDR_MP_RIKA       0x011



struct sensorRS485 {
  uint32_t baudrate;
  uint8_t address;
  std::vector<std::tuple<uint32_t, uint8_t>> registers; //tupla <numero de registro, cantidad de registros>
};


// Inicializar un objeto sensorRS485 con la lista de tuplas
sensorRS485 sens_WS_Rika{
  RS485_BAUDRATE,
  ADDR_WS_RIKA,
  { 
    {1, 1},        // reg:1    |   Device Status     |   16bit , return INT    (0xA000,0xA03F)
    {2, 1},        // reg:2    |   Wind direction    |   16bit , return INT    (0,359º)
    {3, 2},        // reg:3,4  |   Wind Speed        |   32bit , return FLOAT  (0,60m/s)
    {5, 2},        // reg:5,6  |   Air Temp          |   32bit , return FLOAT  (-40,+80ºC)
    {7, 2},        // reg:7,8  |   Air Hum           |   32bit , return FLOAT  (0,100%)
    {9, 2},        // reg:9,10 |   Air pressure      |   32bit , return FLOAT  (150-1100hPa)
    {11, 1},       // reg:11   |   Electronic Compas |   16bit , return INT    (0-359º)
    {12, 1},       // reg:12   |   Rain/Snow         |   16bit , return INT    (0x0000,0x000F)
    {13, 2},       // reg:13,14|   Rainfall          |   32bit , return FLOAT  (100mm/h)
    {15, 2},       // reg:15,16|   Rainfall acc      |   32bit , return FLOAT  
    {17, 1},       // reg:17   |   Rainfall unit     |   16bit , return INT    
    {18, 1},       // reg:18   |   Positioning status|   16bit , return INT    (0,1)
    {19, 2},       // reg:19,20|   Speed of ship     |   32bit , return FLOAT  (km/h)
    {21, 1},       // reg:21   |   Course            |   16bit , return INT    (0-359º)
    {22, 2},       // reg:22,23|   Longitude         |   32bit , return FLOAT  
    {24, 2},       // reg:24,25|   Latitude          |   32bit , return FLOAT  
    {26, 2},       // reg:26,27|   Dust concentration|   32bit , return FLOAT  (ug/m3)
    {28, 2},       // reg:28,29|   Visibility        |   32bit , return FLOAT  (m)
    {30, 2},       // reg:30,31|   Ilumninance       |   32bit , return FLOAT  (Lux)
    {32, 2},       // reg:32,33|   Radiation accu    |   32bit , return FLOAT  (KJ)
    {34, 2},       // reg:34,35|   Radiation         |   32bit , return FLOAT  (W)
    {36, 2},       // reg:36,37|   Real wind direct  |   32bit , return FLOAT  (0,369º)
    {38, 2},       // reg:38,39|   Altitude          |   32bit , return FLOAT  (m)
    {40, 2}        // reg:40,41|   Real wind speed   |   32bit , return FLOAT  (m/s)
  }
};

sensorRS485 sens_MP_Rika{
  RS485_BAUDRATE,
  ADDR_MP_RIKA,
  { 
    {0, 1},        // reg:0    |   Temp              |   16bit , return INT    (ºC)
    {2, 1},        // reg:2    |   Wind direction    |   16bit , return UINT   (%RH)
    {3, 1},        // reg:3,4  |   Wind Speed        |   16bit , return UINT   (hPa)
    {5, 1},        // reg:5,6  |   Air Temp          |   16bit , return UINT   (ppm)
    {7, 1},        // reg:7,8  |   Air Hum           |   16bit , return UINT   (ug/m3)
    {9, 1}         // reg:9,10 |   Air pressure      |   8bit , return FLOATº  (0,0xFF)
  }
};




void initWeatherStation() {
  //enciende estación meteo
  digitalWrite(EN_VSys, HIGH);

  //configura comunicación serie
  SERIAL_MUX.begin(RS485_BAUDRATE, SERIAL_8N1, URX0, UTX0);
  delayMs(100);
}

void changeAddress(uint16_t currentAddr, uint16_t newAddr, uint16_t register){
  Serial.printf("modificando dirección de: %s por: %s\n", currentAddr, newAddr );
  node.begin(currentAddr, SERIAL_MUX);
  uint8_t result = node.writeSingleRegister(_reg, datas);
  Serial.println(result);
}

void readWeatherStation(void) {

}


void taskReadRS485 (void *pvParameters) {
  initCallbacks(&node);
  initWeatherStation();

  //CALLBACKSMODBUS
  //initCallbacks(&node);

  for (;;)
  {
    readWeatherStation();

  }
}
