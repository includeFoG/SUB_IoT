const char* FILENAME = (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__);

#include "DLG_def.h"
#include "Config.h"
#include "Sensor.h"



void setup() {
  initBoard();

  delay(1000);
  //unsigned long valorDeTS = millis();
  
  /*std::vector<std::string> keyName = {"T", "H", "Fire", "status"};
  std::vector<Valor> val = {23.32f, 100, false, std::string("Normal")};
  measure primeraMedida(valorDeTS, keyName, val);*/
  
  /* //PRUEBA DE OBJETO MEASURE
  std::vector<tuple_Measure> mediciones = {
    {"temperatura",23.12f},
    {"humedad",69},
    {"estado",true},
    {"tipo","tipo1"}
  };
    std::vector<tuple_Measure> mediciones2 = {
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
  primeraMedida.add_vValues(mediciones2);
  primeraMedida.printMeasure();
  Serial.println("\n");
  tuple_Measure nuevoValor = std::make_tuple("ADDED", 69); // Usando make_tuple
  primeraMedida.add_Value(nuevoValor);
  primeraMedida.printMeasure();
  Serial.println("\n");
  */

  /*//PRUEBA DE OBJETO POWER
  Serial.println("LATEST STATUS CHECK NOW");
  delay(10000);
  PowerManager& superPower = PowerManager::getInstance(); // Obtén la única instancia 
  Serial.println("GET INSTANCE 1, NOW CHECK STATE SHOULD BE OFF");
  delay(10000);
  superPower.initialize(0b100000000001111, 0b1000000000000001);
  Serial.println("INITIALIZED, NOW CHECK STATE SHOULD BE CONFIGURED");
  delay(10000);
  PowerManager& superPower2 = PowerManager::getInstance(); // Obtén la única instancia 
  Serial.println("INITIALIZED, NOW CHECK STATE SHOULD KEEP SAME");
   delay(10000);
   superPower2.initialize(0b100000000001111, 0b1000000000000101);*/

  //PRUEBA OBJETO SENSOR
  std::vector<tuple_RegNumRead> sensor1Registers = {
    std::make_tuple(100, 2),
    std::make_tuple(102, 2),
    std::make_tuple(104, 2),
    std::make_tuple(106, 2),
    std::make_tuple(108, 2)
};
delay(5000);//BORRAR
  sensRS485Modbus sensor1("sensor1", true, 1,0x1,9650,sensor1Registers);
  
  

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("lego");
  delay(2000);
}
