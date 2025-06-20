#include <TimeLib.h>
#include <RTCZero.h>

RTCZero rtc;

#define GAP_TIME  5 //timeWindow
const byte myAlarms[7] = {2, 6, 10, 14, 18, 22, 24}; //VECTOR DE WAVEPISTON
//const byte myAlarms[7] = {0,4,8,12,16,20,24}; //{2,6,10,14,18,22,24};
//const byte myAlarms[13] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 24}; //EL VECTOR TIENE QUE TERMINAR CON EL VALOR 24 PARA QUE FUNCIONE CORRECTAMENTE getNextHour()
//const byte myAlarms[25] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0]))) //ADDED

bool inTimeWindow(time_t t_window)
{
  byte lhour = hour(t_window);
  byte lmin = minute(t_window);
  byte idx = 0;
  uint8_t numHoras = NUMITEMS(myAlarms);

  while (( idx < numHoras ) && (lhour > myAlarms[idx]))
  {
    idx++;
  }
  return (((lhour == myAlarms[idx]) && (lmin < (GAP_TIME * 3))) || (((lhour + 1) == myAlarms[idx]) && (lmin > 60 - GAP_TIME)));
}

byte getNextHour(time_t t_getNextHour)
{
  byte lhour = hour(t_getNextHour);
  byte lmin = minute(t_getNextHour);
  byte idx = 0;
  uint8_t numHoras = NUMITEMS(myAlarms);

  while (( idx < (numHoras)  ) && (lhour >= myAlarms[idx]))
  {
    idx++;
  }

  if (idx >= numHoras - 1 )
  {
    return myAlarms[0];
  }

  if (((lhour + 1) == myAlarms[idx]) && (lmin > 60 - GAP_TIME)) {
    idx++;
  }

  return myAlarms[idx];
}

int initRTC()
{
  const byte _seconds = 0;
  const byte _minutes = 0;
  const byte _hours = 12;

  /* Change these values to set the current initial date */
  const byte _day = 1;
  const byte _month = 1;
  const byte _year = 25;
  rtc.begin();
  // Set the time
  rtc.setHours(_hours);
  rtc.setMinutes(_minutes);
  rtc.setSeconds(_seconds);

  // Set the date
  rtc.setDay(_day);
  rtc.setMonth(_month);
  rtc.setYear(_year);
  return 0;
}
void alarmMatch();



int setAlarmRTC(time_t t_setAlarm)
{

  // time_t t = (time_t) rtc.getEpoch();
  //  byte nexthour = hour(t) +4;
  //  byte nexthour = hour(t)+4;
  byte nexthour = getNextHour(t_setAlarm);
  byte nextminute =  0; //minute(t)+10; //0
  byte nextsecond =  0;//second(t);
  if ( nextsecond >= 60)
  {
    nextsecond = nextsecond - 60;
    nextminute++;
  }
  if ( nextminute >= 60)
  {
    nextminute = nextminute - 60;
    nexthour++;
  }
  if ( nexthour >= 24) nexthour = nexthour - 24;
  rtc.setAlarmTime(nexthour, nextminute, nextsecond);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);//MATCH_HHMMSS
  rtc.attachInterrupt(alarmMatch);
  SERIAL.print("NEXT RESET: ");
  SERIAL.print(String(nexthour));
  SERIAL.print(":");
  SERIAL.print(String(nextminute));
  SERIAL.print(":");
  SERIAL.println(String(nextsecond));
  return 0;
}
int num = 0;

String getNameFile() //devuelve nombre archivo con extensión ej:"20220420_021330.csv"
{
  // Set the time
  time_t t = (time_t) rtc.getEpoch();
  return (intToDigits(year(t), 4) + intToDigits(month(t), 2) + intToDigits(day(t), 2) + "_" + intToDigits(hour(t), 2) + intToDigits(minute(t), 2) + intToDigits(second(t), 2) + EXTENCION).c_str();
  // return "dato" + String(num) + EXTENCION;
}
String getNameFile(time_t t)
{
  // Set the time
  // time_t t =(time_t) rtc.getEpoch();
  return (intToDigits(year(t), 4) + intToDigits(month(t), 2) + intToDigits(day(t), 2) + "_" + intToDigits(hour(t), 2) + intToDigits(minute(t), 2) + intToDigits(second(t), 2) + EXTENCION).c_str();
  // return "dato" + String(num) + EXTENCION;
}




void alarmMatch()
{
  DEBUG("Reset Alarm [Time Array]");
  NVIC_SystemReset();
}
