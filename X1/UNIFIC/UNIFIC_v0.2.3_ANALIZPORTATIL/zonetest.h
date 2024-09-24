//FUNCIONES DE PRUEBA PARA TESTZONE

///////////////////////TESTZONE//////////////////////

///////////I2C IO 16 channel///////////////////=]

//Serial.println("Alimentando placa (5V y 3.3V)");
//placa.EnableARDUCAM();
//placa.EnableConmutado3v();
//
//PCF.begin(); //SDA,SCL pin, initial value
//
//x = PCF.read16();
//Serial.print("Read ");
//printHex(x);
//delay(1000);
//
//while (1) {
//  wtd.clear();
//  Serial.println("HLT");
//  while (Serial.available() == 0);
//  Serial.println("Introduzca valor para :" + String(outnum));
//  switch (Serial.read())
//  {
//    case 'H': doHigh(outnum); break;
//    case 'L': doLow(outnum); break;
//    case 'T': doToggle(outnum); break;
//    case '+':
//      if (outnum < 16)
//        outnum++;
//      else
//        outnum = 0;
//      break;
//  }
//}

void printHex(uint16_t x)
{
  if (x < 0x1000) Serial.print('0');
  if (x < 0x100)  Serial.print('0');
  if (x < 0x10)   Serial.print('0');
  Serial.println(x, HEX);
}


void doHigh(uint8_t outnum)
{

  Serial.println("HIGH: " + String(outnum));
  PCF.write(outnum, HIGH);
  //int x = PCF.read16();
  // Serial.print("Read ");
  // printHex(x);
}


void doLow(uint8_t outnum)
{
  Serial.println("LOW: " + String(outnum));
  PCF.write(outnum, LOW);
  // int x = PCF.read16();
  //  Serial.print("Read ");
  //printHex(x);
}




void doToggle(uint8_t outnum)
{

  Serial.println("Toggle: " + String(outnum));
  PCF.toggle(outnum);
  int x = PCF.read16();
  Serial.print("Read ");
  printHex(x);
}
