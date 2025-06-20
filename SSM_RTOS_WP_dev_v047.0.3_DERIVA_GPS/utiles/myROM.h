#ifndef _MY_ROM_
#include "Arduino.h"
#include <Wire.h>
#define _MY_ROM_


class ssm_eeprom
{
  public:
    ssm_eeprom(int addr); //constructor
    void writeData(unsigned int eaddress, byte data);    
    byte readData(unsigned int eaddress);
    void writeString(unsigned int eaddress, String str);
    String readString(unsigned int eaddress);
  private:
    byte eeprom_address = 0x50;
};



#endif
