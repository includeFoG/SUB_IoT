
//#ifndef FLOW_CONTROL
////Without Flow control
//Uart SerialM1(&sercom2, M1_RX, M1_TX, SERCOM_RX_PAD_1, UART_TX_PAD_0);

//Uart SerialM2 (&sercom5, M2_RX, M2_TX, SERCOM_RX_PAD_3, UART_TX_PAD_2);
#include <TimeLib.h>

void init_IO () {
  pinMode(M1_AN, INPUT);
  pinMode(M1_RST, OUTPUT);

  pinMode(MUX_SEL_0, OUTPUT);
  pinMode(MUX_SEL_1, OUTPUT);
  pinMode(FLOW_RS485, OUTPUT);

  pinMode(EN_VCC_MIKROE, OUTPUT);

  // Init in receive mode
  digitalWrite(FLOW_RS485, LOW);
  digitalWrite(M1_RST, LOW);

  //RS485
  digitalWrite(MUX_SEL_0, HIGH);
  digitalWrite(MUX_SEL_1, HIGH);
}


void insertarComando(String _cmd, unsigned int _index, String &_programa) {
  Serial.println("insertarComando");
  if ( _index < _programa.length()) {
    _programa = _programa.substring(0, _index) + _cmd + _programa.substring(_index); // dato
  } else {
    _programa += _cmd;
  }
  Serial.println(_programa);
  Serial.println("a_sendTime: " + String(a_sendTime));
}

void EnableMIKRO()
{
  digitalWrite(EN_VCC_MIKROE, HIGH);
}

void DisableMIKRO()
{
  digitalWrite(EN_VCC_MIKROE, LOW);
}

String printTime() {
  current_timestamp = rtc.getEpoch();
  Serial.print("Current UNIX Timestamp: ");
  Serial.println(current_timestamp);

  //  Serial.print("Time & Date: ");
  //  Serial.print(hour(current_timestamp));
  //  Serial.print(":");
  //  Serial.print(minute(current_timestamp));
  //  Serial.print(":");
  //  Serial.print(second(current_timestamp));
  //  Serial.print("; ");
  //  Serial.print(day(current_timestamp));
  //  Serial.print(".");
  //  Serial.print(month(current_timestamp));
  //  Serial.print(".");
  //  Serial.println(year(current_timestamp));
  return (String(day(current_timestamp)) + "-" + String(month(current_timestamp)) + "-" +
          String(year(current_timestamp)) + "; " + String(hour(current_timestamp)) + ":" +
          String(minute(current_timestamp)) + ":" + String(second(current_timestamp)));

}

void initRTC(String fechaHora) {

  const int year = ((fechaHora.substring(0, 4)).toInt());
  const byte month = ((fechaHora.substring(5, 7)).toInt());
  const byte day = ((fechaHora.substring(8, 10)).toInt());

  const byte hours = ((fechaHora.substring(11, 13)).toInt());
  const byte minutes = ((fechaHora.substring(14, 16)).toInt());
  const byte seconds = ((fechaHora.substring(17, 19)).toInt());

  Serial.println("InitRTC:");
  Serial.print (String(year) + "/");
  Serial.print (String(month) + "/");
  Serial.print (String(day) + " ");
  Serial.print (String(hours) + ":");
  Serial.print (String(minutes) + ":");
  Serial.println (String(seconds));

  rtc.setTime(seconds, minutes, hours, day, month, year); //el rtc del ESP32Time toma año completo (2025)


  printTime();
}
