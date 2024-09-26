const char* FILENAME = (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__);

#include "DLG_def.h"
#include "Config.h"
#include "Sensor.h"



void setup() {
  initBoard();

  delay(1000);
  unsigned long valorDeTS = millis();
  /*std::vector<std::string> keyName = {"T", "H", "Fire", "status"};
  std::vector<Valor> val = {23.32f, 100, false, std::string("Normal")};
  measure primeraMedida(valorDeTS, keyName, val);*/

  std::vector<tuple_Medida> mediciones = {
    {"temperatura",23.12f},
    {"humedad",69},
    {"estado",true},
    {"tipo","tipo1"}
  };
    std::vector<tuple_Medida> mediciones2 = {
    {"testeo",11.11f},
    {"tester",22},
    {"test",false},
    {"testing","tested"}
  };

  Measure primeraMedida(valorDeTS, mediciones);
  
  primeraMedida.printMeasure();
  Serial.println("\n");
  primeraMedida.clearMeasure();
  primeraMedida.printMeasure();
  Serial.println("\n");
  primeraMedida.add_vMeasure(mediciones2);
  primeraMedida.printMeasure();
  Serial.println("\n");
  tuple_Medida nuevoValor = std::make_tuple("ADDED", 69); // Usando make_tuple
  primeraMedida.add_Value(nuevoValor);
  primeraMedida.printMeasure();
  Serial.println("\n");
  

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("lego");
  delay(2000);
}
