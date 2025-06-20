
#include "myROM.h"

ssm_eeprom::ssm_eeprom(int addr)
{  
  eeprom_address = addr;
}

void ssm_eeprom::writeData(unsigned int eaddress, byte data){
  Wire.beginTransmission(eeprom_address);
  // set the pointer position
  Wire.write((int)(eaddress >> 8));
  Wire.write((int)(eaddress & 0xFF));
  Wire.write(data);
  Wire.endTransmission();
  delay(10);
}

byte ssm_eeprom::readData(unsigned int eaddress){
  byte result;
  Wire.beginTransmission(eeprom_address);
  // set the pointer position
  Wire.write((int)(eaddress >> 8));
  Wire.write((int)(eaddress & 0xFF));
  Wire.endTransmission();
  Wire.requestFrom(eeprom_address,1); // get the byte of data
  result = Wire.read();
  return result;
}
void ssm_eeprom::writeString(unsigned int eaddress, String str){  
  for (int i=0; i<str.length()+1; i++){ 
    writeData(i+eaddress,(byte)str[i]);
  }
  
}
String ssm_eeprom::readString(unsigned int eaddress){
  byte data=0xFF;
  
  String outData = "";
  while (data!=0){
    data=readData(eaddress);
    
    outData += (char)data;
    if (data==0xFF) return outData;
    eaddress++;
  }
  return outData;

}
 
