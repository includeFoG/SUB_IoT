#ifndef CONFIG_DATA_DEFAULT
#include "config.h"
#endif


//IOQU
String commandMenu = "*******************************************************************************************************\n"
                     "** NP -> Medida Analog 1     NP -> AlimentoAna 2      NP -> ApagoAna   3         NP -> Alimento RS485 4  **\n"
                     "** NP -> Mido Estacion 5     NP -> Apago estacion 6    G -> PwrEntrada 7          H -> Apago todo 8      **\n"
                     "** I  -> leeAnalizSch 9      NP -> ListaArchivos 10   NP -> LecturaArchivo 11     L -> TemperaturaPCB 12 **\n"
                     "** NP -> Forecast 13         NP -> BorrarArchivo 14   O  -> connexion Inet 15    NP -> Send analog 16    **\n"
                     "** Q  -> Send 485 17         NP -> EnviaAnalizPM 18   S  -> conexion close 19    T  -> Gestion tiempo 20 **\n"
                     "** U  -> Wait(iddle) 21      NP -> Log data 22        W  -> PrintMenu 23         X -> ReadConfig  24    **\n"
                     "** NP ->  25                 Z  -> Reset 26           a  ->     27               NP -> search AP 28      **\n"
                     "** NP -> ecomode             NP -> leeAmpRs485 30     NP -> Gestión RS485-ETH    f  -> MQTT Disc         **\n"
                     "** NP -> Firmwr updt Http 33 r  -> Reset SendTime 44  NP -> LecturaSiemens 45    t  -> TakeTime 46       **\n"
                     "** NP -> UploadFile  47      NP -> LogData(2.0) 48    NP -> TESTEOS 50           -  -> LowPower 60       **\n"
                     "*******************************************************************************************************\n";
bool ToBoolean(String valueRead);


bool ToBoolean(String valueRead)
{
  uint8_t indx = 0;
  valueRead.toLowerCase();

  if ((valueRead.indexOf("true") != -1) || (valueRead.toInt() == 1)) {
    indx = 1;
  }
  else  if ((valueRead.indexOf("false") != -1) || (valueRead.toInt() == 0)) {
    indx = 2;
  }

  switch (indx)
  {
    case 1:
      return true;
    case 2:
      return false;
    default:
      return false;
  }
}
