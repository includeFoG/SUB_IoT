#ifndef Sensor_h
#define Sensor_h

#include <vector>
#include <string>
#include <variant>  //usado para que la medición pueda ser de distinto tipo
#include <tuple>

/*
   Esta clase contiene los datos de una única medida:
      timestamp
      nombre/s de medida
      valores de la medida
*/
using Valor = std::variant<float, int, bool, std::string>;   //Definir el tipo de valor como una variante de float, int, bool, y string
using tuple_Medida = std::tuple<std::string,Valor>;

class Measure
{
  protected:
    unsigned long timeStamp;                                 //Momento de la medida
    /*std::vector<std::string> keys;                           //Nombres de las medidas, se puede utilizar para hacer estrategias de medidas parciales
    std::vector<Valor> values;                               //Valores de la medición que pueden ser float, int, bool, o string*/
    std::vector<tuple_Medida> mediciones;                    //Vector de tuplas con nombre de medida y su valor correspondiente

  public:
    //CONSTRUCTORES
    Measure(unsigned long timeStamp, std::vector<tuple_Medida> mediciones);
    Measure(unsigned long timestamp);

    //FUNCIONES
    void printMeasure() const; 
    void add_vMeasure(const std::vector<tuple_Medida> medicion); //añade un vector de tuplas completo con las medidas //puede ser paso por ref porque dentro se genera copia
    void add_Value(const tuple_Medida& medida);                  //añade una tupla al vector de tuplas de medidas      //puede ser paso por ref porque dentro se genera copia
    void clearMeasure();                                         //elimina los datos almacenados en la measure (tanto timestamp como medidas)
};

class sensor {
  public:
    std::string nombre;                        //ID del sensor

    std::string topic;                         //Topic al que se envía la información -> OJO!!! quizá este debería ser un elemento de otro objeto de envío y componerlos, junto con: frecuencia de envío, dirección, protocolo...
    std::vector<Measure> mediciones;           //Medidas contenidas en el sensor
 
    unsigned int measureFrequency;             //Frecuencia de muestreo

    bool powered;                              //informa si está alimentado por la placa

    sensor(std::string nombre, std::string topic, int measureFrequency);
};

class sensRS485Modbus : public sensor {
  private:
    std::vector<uint16_t> registers;
    unsigned int baudrate; 
    
   
};


#endif //Sensor_h
