#include "Sensor.hpp"
#include "DLG001_def.hpp" //definiciones de la placa DLG utilizadas para reutilizar, ver comentario a continuación

#include "esp_log.h"

static const char *TAG = "Sensor";

/* ELEMENTOS EXTERNOS EMPLEADOS
   SERIAL_RS485 UART PARA RS485 //de momento, para no aumentar memoria metiendo
   un objeto serial en cada sensor y no tener que gestionar el tráfico de las
   uarts vamos a hacerlo dependiente a DLG_def.h
*/

/*USADO CUANDO KEYS Y VALUES ERAN DOS VECTORES Measure::Measure(unsigned long
  timeStamp, std::vector<std::string> keys,  std::vector<Valor> values)
  :timeStamp(timeStamp), keys(keys), values(values) {}*/

Measure::Measure(unsigned long timeStamp, std::vector<tuple_Measure> mediciones)
    : timeStamp(timeStamp), mediciones(mediciones) {}
Measure::Measure(unsigned long timeStamp)
    : timeStamp(timeStamp), mediciones() {}


void Measure::printMeasure() const {
  printf("TimeStamp: %lu", this->timeStamp);
  // range for

  for (const auto &[key, value] : this->mediciones) { // const indica que no se van a modificar las variables durante la iteracion, auto el compilador determina el tipo de dato, & referencia a los elementos del vector,
    printf("Key: %s\t,\t Value:", key.c_str());
        // Imprimir el valor correspondiente usando std::visit
        std::visit(
            [](auto &&arg) {
              if constexpr (std::is_same_v<
                                std::decay_t<decltype(arg)>,
                                std::string>) { // decltype obtiene el tipoexacto, is_same_v comprueba los tipos , constexpr verificación en tiempo de compilación
                printf("%s", arg.c_str()); // Convertir a const char* para std::string
              } 
              else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, int>) {
                printf("%d", arg); 
              } 
              else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, float>) {
                printf("%f", arg); 
              }else {
              	ESP_LOGE(TAG,"ERROR TIPO NO SOPORTADO");
              }
            },value);

    printf("\n");
    ;
  }
}

void Measure::add_Value(const tuple_Measure &tupleValue) {
  this->mediciones.push_back(tupleValue);
}

void Measure::add_vValues(const std::vector<tuple_Measure> v_tuplesValues) {
  this->mediciones = v_tuplesValues;
}

void Measure::clearMeasure() {
  this->timeStamp = 0;
  this->mediciones.clear();
}

// ***********************************************************************
//  ESPACIO DE CLASE BASE: [SENSOR]
// ***********************************************************************
/*Sensor::Sensor(std::string nombre, configPower infoPower)
  : nombre(nombre), v_mediciones(), infoPower(infoPower) {}*/

Sensor::Sensor(std::string nombre, bool poweredByBoard,
               std::optional<int> pinRelated, std::optional<bool> keepOn)
    : nombre(nombre), infoPower{poweredByBoard, pinRelated, keepOn},
      sensorPowerController(PowerManager::getInstance()) {}

void Sensor::saveMeasure(const Measure medida) {
  this->v_mediciones.push_back(medida);
}
void Sensor::saveMeasures(const std::vector<Measure> medidas) {
  this->v_mediciones = medidas;
}

/*
// ***********************************************************************
//  ESPACIO DE CLASE DERIVADA DE SENSOR: [sensRS485Modbus]
// ***********************************************************************
sensRS485Modbus::sensRS485Modbus(std::string nombre,  bool poweredByBoard,
std::optional<int> pinRelated, int addr, int baudRate,
std::vector<tuple_RegNumRead> registers) : Sensor(nombre, poweredByBoard,
pinRelated), addr(addr), baudRate(baudRate), registers(registers)
{
  this->sensorPowerController.initialize(0b100000000001111, 0b1000000000000001);
//TEST CONTINUAR POR AQUI
}

void sensRS485Modbus::takeMeasure(bool save) {
  SERIAL_RS485.begin(this->baudRate);
  this->node.begin(this->addr, SERIAL_RS485);
  Serial.println("[" + String(this->nombre.c_str()) + "] - Failed to START
modbus RTU Client");

  uint8_t resultRS485;

  //AQUI HAY QUE HACER UNA GESTION DE LA CORRELACION ENTRE LOS REGISTROS, DE
MOMENTO VAMOS A SIMPLIFICARLO COMO SI TODOS ESTUVIESEN SEGUIDOS Y EL PRIMERO SEA
EL REGISTRO BASE
  //ESTO DEBERIA SER UNA FUNCION QUE SE LLAME CADA VEZ QUE HAYA CORRELATIVOS
  int total_reg = (this->registers).size();
  std::vector<uint16_t> v_Medidas(total_reg);


  resultRS485 = node.readHoldingRegisters(std::get<0>(this->registers[0]),
total_reg); //lee desde el registro indicado la cantidad de registros contenidos
en registers

  if (resultRS485 == node.ku8MBSuccess)
  {
    for (int i = 0; i < total_reg; i++)
    {
      v_Medidas[i] = node.getResponseBuffer(i); //lee los valores de los
distintos registros Serial.println("Medidas instantaneas [" + String(i) + "] : "
+ String(v_Medidas[i]));
    }
  } else
  {
    Serial.print("ku8MB: "); Serial.println(resultRS485, HEX);
  }
}

void takeMeasure(bool save = true)
{
  if(this->infoPower.poweredByBoard){
    Serial.println("Sensor alimentado por placa");
  }

}*/