#include <arduino.h>
#include <Encoder.h>
#include <WDTZero.h>

#include "DEF_PINS_GP.h"
#include "Config.h"

#define ENCODER_OPTIMIZE_INTERRUPTS // ENCODER_OPTIMIZE_INTERRUPTS puede causar problemas si se usan interrupciones con otra librería alternativa: ENCODER_USE_INTERRUPTS
Encoder myEnc(A_WHITE, B_GREEN);

WDTZero wtd;


//#define TEST_LUIS //////////////////////////////////////////////////////////////////////////////////////////////    [DELETE]
#define EN_DEBUG //EN_DEBUG , EN_DEBUG_2

void setup() {

  Serial.begin(115200);
  Serial1.begin(38400); //comunicación con CI-RS232


  pinMode(BUT_RESET, INPUT_PULLUP);

  wtd.setup(WDT_TIME);
  wtd.attachShutdown(ISR_WDT);

  Serial.println("\n******************************");
  Serial.println("       GP0_EncoderReel_V2       ");
  Serial.println(FILENAME);
  Serial.println("********************************");

  Serial1.print(createPacket(0));
}

void loop() {
  wtd.clear();
  String command = "";

#ifdef TEST_LUIS //////////////////////////////////////////////////////////////////////////////////////////////    [DELETE]
  long newPosition = random(0, 600000); //////////////////////////////////////////////////////////////////////////////////////////////    [DELETE]
#else //////////////////////////////////////////////////////////////////////////////////////////////    [DELETE]
  long newPosition = myEnc.read(); //devuelve ticks relativos a primera posición
#endif //////////////////////////////////////////////////////////////////////////////////////////////    [DELETE]
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    distance = ticksToMeters(newPosition);
#ifdef EN_DEBUG_2:
    Serial.println("Ticks: " + String(newPosition) + "\t meters: " + String(distance, 4));
#endif
  }

  newBUT = digitalRead(BUT_RESET);

  if (newBUT != oldBUT) {
    if (newBUT == false) {
#if defined(EN_DEBUG) || defined(EN_DEBUG_2)
      Serial.println("RESET");
#endif
      myEnc.readAndReset();
      delay(100);
    }
    oldBUT = newBUT;
  }

  if ((millis() - pretime) >= (1000.0 / send_period)) {
    Serial1.print(createPacket(distance));
#if defined(EN_DEBUG) || defined(EN_DEBUG_2)
    Serial.println("createpacket payload: " + String(distance, 4));
    Serial.print(createPacket(distance));
#endif
    pretime = millis();
  }

  if (Serial1.available()) {
    command = Serial1.readString();
#if defined(EN_DEBUG) || defined(EN_DEBUG_2)
    Serial.println("RECIBIDO COMANDO: " + String(command));
#endif


    if (command.indexOf("F") != -1) { //comando de frecuencia de envío
      send_period = command.substring(1).toInt();
      Serial1.println("OK-F");
    }
    else if (command.indexOf("D") != -1) { //comando de diámetro rueda dentada eje encoder
      radioMain = command.substring(1).toFloat();
      perRuedaMain = 2 * PI * radioMain;
      resMain = perRuedaMain / ticksLibrary;
      Serial1.println("OK-D");
    }
    else if (command.indexOf("R") != -1) { //comando de reset encoder
      myEnc.readAndReset();
      Serial1.println("OK-R");
    }
    else if (command.indexOf("S") != -1) { //comando de set encoder
      myEnc.write(metersToTicks(command.substring(1).toInt()));
      Serial1.println("OK-S");
    }
    else {
      Serial1.println("ERR_COMMAND");
    }
  }
}


float ticksToMeters(long newPosition) {
  return newPosition * resMain;
}

long metersToTicks(float meters) {
  return meters / resMain;
}



String createPacket(float data) {
  //$PITCH±DD.D,ROLL±DDD.D,DIST±DDDD.DD<CR><LF>  37 bytes
  String payload;
  char distFormat[7]; //0000.00 (m)
  char sign;

  if (data >= 0) {
    sign = '+';
  }
  else
    sign = '-';

  payload += "$PITCH";
  payload += "+00.0";
  payload += ",ROLL";
  payload += "+000.0";
  payload += ",DIST";
  payload += (sign);
  
  sprintf(distFormat,"%07.2f",data); 
  
  payload += distFormat;
  payload += "\r\n";

  return payload;
  //return String(payload, 4) + "M\r\n";
}





void ISR_WDT() {
  Serial.println("WTD RESET");
}
