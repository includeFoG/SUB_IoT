#include <Arduino.h>

#define SerialDBG Serial
//#define SSM_DEBUG
#ifdef SSM_DEBUG

namespace {
  template<typename T>
  static void DEBUG(T last) {
    SerialDBG.print("[");
    SerialDBG.print(millis());
    SerialDBG.print("] ");
    SerialDBG.println(last);
    SerialDBG.flush();
  }
  


//  template<typename T, typename... Args>
//  static void DEBUG(T head, Args... tail) {
//    SerialDBG.print(head);
//    SerialDBG.print(' ');
//    DEBUG_PLAIN(tail...);
//  }
//
//  template<typename... Args>
//  static void DEBUG(Args... args) {
//    SerialDBG.print("[");
//    SerialDBG.print(millis());
//    SerialDBG.print("] ");
//    DEBUG_PLAIN(args...);
//  }
}
#else
  #define DEBUG_PLAIN(...)
  #define DEBUG(...)
#endif

String intToDigits(int dat, int n_digitos) {
  String temp = String(dat);
  if (!( temp.length() == n_digitos))
  {
    for ( int i = 0 ; i < n_digitos - temp.length(); i++)
    {
      temp = "0" + temp;
    }
  }
  return temp;
}