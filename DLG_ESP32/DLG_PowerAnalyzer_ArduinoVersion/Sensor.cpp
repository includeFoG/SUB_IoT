#include "Sensor.h"
#include "Arduino.h"

/*USADO CUANDO KEYS Y VALUES ERAN DOS VECTORES Measure::Measure(unsigned long timeStamp, std::vector<std::string> keys,  std::vector<Valor> values)
  :timeStamp(timeStamp), keys(keys), values(values) {}*/

Measure::Measure(unsigned long timeStamp, std::vector<tuple_Medida> mediciones)
  : timeStamp(timeStamp), mediciones(mediciones) {}
Measure::Measure(unsigned long timeStamp)
  : timeStamp(timeStamp), mediciones() {}

void Measure::printMeasure() const{
  Serial.println("TimeStamp: " + String(this->timeStamp));
  //range for
  for (const auto& [key, value] : this->mediciones) { //const indica que no se van a modificar las variablesdurante la iteracion, auto el compilador determina el tipo de dato, & referencia a los elementos del vector,
    Serial.print("Key: ");
    Serial.print(key.c_str());
    Serial.print(", Value: ");
    //printValor(value);   // Imprimir el valor de la tupla
    // Imprimir el valor correspondiente usando std::visit
    std::visit([](auto && arg) {
      if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>) {
        Serial.print(arg.c_str()); // Convertir a const char* para std::string
      } else {
        Serial.print(arg); // Imprimir otros tipos directamente
      }
    }, value);

    Serial.println();
  }
}

void Measure::add_Value(const tuple_Medida& medida){
  this->mediciones.push_back(medida);
}

void Measure::add_vMeasure(const std::vector<tuple_Medida> medicion){
  this->mediciones = medicion;
}

void Measure::clearMeasure(){
  this->timeStamp=0;
  this->mediciones.clear();
}
