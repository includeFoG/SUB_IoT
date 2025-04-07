#ifndef Sensor_h
#define Sensor_h

#include <vector>
#include <string>
#include <variant>   //usado para que la medición pueda ser de distinto tipo
#include <tuple>
#include <optional>  // Para representar el pin de forma opcional

//#include <ModbusMaster.h> //Modbus RTU
#include "ConfigBoard_DLG001.hpp"   //usado para powerManager


/*
   Esta clase contiene los datos de una única medida:
      timestamp
      nombre/s de medida
      valores de la medida
*/
using Valor = std::variant<float, int, bool, std::string>;   //Definir el tipo de valor como una variante de float, int, bool, y string
using tuple_Measure = std::tuple<std::string, Valor>;
using tuple_RegNumRead = std::tuple<uint16_t, uint8_t>;

class Measure
{
  protected:
    unsigned long timeStamp;                                 //Momento de la medida
    /*std::vector<std::string> keys;                           //Nombres de las medidas, se puede utilizar para hacer estrategias de medidas parciales
      std::vector<Valor> values;                               //Valores de la medición que pueden ser float, int, bool, o string*/
    std::vector<tuple_Measure> mediciones;                    //Vector de tuplas con nombre de medida y su valor correspondiente

  public:
    //CONSTRUCTORES
    Measure(unsigned long timestamp);
    Measure(unsigned long timeStamp, std::vector<tuple_Measure> mediciones);

    //METODOS
    void printMeasure() const;
    void add_vValues(const std::vector<tuple_Measure> v_tuplesValues);  //añade un vector de tuplas completo con las medidas //puede ser paso por ref porque dentro se genera copia
    void add_Value(const tuple_Measure& tupleValue);                  //añade una tupla al vector de tuplas de medidas      //puede ser paso por ref porque dentro se genera copia
    void clearMeasure();                                         //elimina los datos almacenados en la measure (tanto timestamp como medidas)
};

struct configPower{
  bool poweredByBoard;
  std::optional<int> pinRelated; 
  
  std::optional<bool> keepOn;
};




class Sensor { //clase base para todos los tipos de sensor, abstracta

  public:
      const std::string nombre;                  //ID del sensor
  protected:
    const configPower infoPower;                   //informa si está alimentado por la placa y de ser así contiene el EXPGPIO asociado
    PowerManager& sensorPowerController;           //copia de instancia de powerManager
    
    //std::string topic;                         //Topic al que se envía la información -> OJO!!! quizá este debería ser un elemento de otro objeto de envío y componerlos, junto con: frecuencia de envío, dirección, protocolo...
    //unsigned int measureFrequency;             //Frecuencia de muestreo


  public:
    std::vector<Measure> v_mediciones;           //Medidas contenidas en el sensor
    
    //CONSTRUCTORES
    //Sensor(std::string nombre, configPower infoPower);
    Sensor(std::string nombre, bool poweredByBoard, std::optional<int> pinRelated, std::optional<bool> keepOn);

    //METODOS estáticos:
    //encender? //virtual no puro o estático
    //apagar?   //virtual no puro o estático
    void saveMeasure(const Measure medida);                //guardar lectura (unica medida) con su timestamp -> esto es porque en ocasiones no nos interesa almacenarla, por ejemplo si la desviación es inferior al X porciento
    void saveMeasures(const std::vector<Measure> medidas); //guarda vector de medidas cada una con su timestamp

    //METODOS virtuales:
    //guardar lectura -> esto es porque en ocasiones no nos interesa almacenarla, por ejemplo si la desviación es inferior al X porciento
    virtual void takeMeasure(const bool save = true) = 0; // =0 virtual puro , cualquier clase derivada debe implementar el método
};


/*
class sensRS485Modbus : public Sensor {
  private:
     ModbusMaster node; 
     
  protected:
    int addr;                        //dirección slave
    int baudRate;                    //baudrate RS485
    std::vector<tuple_RegNumRead> registers; //tupla de dirección de registros a consultar del slave y numero de registros que conforman 1 dato
    

  public:
    //CONSTRUCTOR
    sensRS485Modbus(std::string nombre, bool poweredByBoard, std::optional<int> pinRelated , int addr, int baudRate, std::vector<tuple_RegNumRead> registers);

    //METODOS virtuales:
    void chekPower() override; 
    void takeMeasure(bool save = true) override; //no es const por node el save hace referencia al guardado en las measures del sensor
};*/


#endif //Sensor_h