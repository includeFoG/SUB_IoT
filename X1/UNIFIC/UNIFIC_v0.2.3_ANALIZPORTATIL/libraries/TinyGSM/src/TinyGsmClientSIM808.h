/**
 * @file     TinyGsmClientSIM808.h
 * @author   Volodymyr Shymanskyy
 * @license  LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date     Nov 2016
 */

#ifndef TinyGsmClientSIM808_h
#define TinyGsmClientSIM808_h
//#pragma message("TinyGSM:  TinyGsmClientSIM808")

#include <TinyGsmClientSIM800.h>


class TinyGsmSim808: public TinyGsmSim800
{

public:

  TinyGsmSim808(Stream& stream)
    : TinyGsmSim800(stream)
  {}

  /*
   * GPS location functions
   */

  // enable GPS
  bool enableGPS() {
    // uint16_t state;

    sendAT(GF("+CGNSPWR=1"));
    if (waitResponse() != 1) {
      return false;
    }

    return true;
  }

  bool disableGPS() {
    // uint16_t state;

    sendAT(GF("+CGNSPWR=0"));
    if (waitResponse() != 1) {
      return false;
    }

    return true;
  }

  // get the RAW GPS output
  // works only with ans SIM808 V2
  String getGPSraw() {
    sendAT(GF("+CGNSINF"));
    if (waitResponse(GF(GSM_NL "+CGNSINF:")) != 1) {
      return "";
    }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  // get GPS informations
  // works only with ans SIM808 V2
  bool getGPS(float *lat, float *lon, float *speed=0, int *alt=0, int *vsat=0, int *usat=0) {
    //String buffer = "";
    // char chr_buffer[12];
    bool fix = false;

    sendAT(GF("+CGNSINF"));
    if (waitResponse(GF(GSM_NL "+CGNSINF:")) != 1) {
      return false;
    }

    stream.readStringUntil(','); // mode
    if ( stream.readStringUntil(',').toInt() == 1 ) fix = true;
    stream.readStringUntil(','); //utctime
    *lat =  stream.readStringUntil(',').toFloat(); //lat
    *lon =  stream.readStringUntil(',').toFloat(); //lon
    if (alt != NULL) *alt =  stream.readStringUntil(',').toFloat(); //lon
    if (speed != NULL) *speed = stream.readStringUntil(',').toFloat(); //speed
    stream.readStringUntil(',');
    stream.readStringUntil(',');
    stream.readStringUntil(',');
    stream.readStringUntil(',');
    stream.readStringUntil(',');
    stream.readStringUntil(',');
    stream.readStringUntil(',');
    if (vsat != NULL) *vsat = stream.readStringUntil(',').toInt(); //viewed satelites
    if (usat != NULL) *usat = stream.readStringUntil(',').toInt(); //used satelites
    stream.readStringUntil('\n');

    waitResponse();

    return fix;
  }

  // get GPS time
  
  String _TakeTime()  //MODIF!!!
  {
	  // FALTA CONTROL DE TIEMPO Y DE FECHA
	  
	 String _date=""; 
	 String _day="";
	 String _month="";
	 int _year=0;
	 String _Time="";
	 String resp="";
	
	sendAT(GF("+CLTS=1"));
	
	 if (waitResponse(1000L, GF(GSM_NL "OK")) != 1) {
		 Serial.println("Err1");
       return "";
     }
	 
	 sendAT(GF("+CNTPCID=1"));
	 if (waitResponse(1000L, GF(GSM_NL "OK")) != 1) {
		 Serial.println("Err2");
       return "";
     }
	 
	 sendAT(GF("+CNTP=\"0.uk.pool.ntp.org\",\"-1\""));
	 if (waitResponse(1000L, GF(GSM_NL "OK")) != 1) {
		 Serial.println("Err3");
       return "";
     }
	 
	  sendAT(GF("+CNTP"));
	  String _res;
	 if (waitResponse(1000L,GF(GSM_NL "OK")) != 1) {   
		 Serial.println("Err4");
       return "";
     }
	 
	 waitResponse();
	 
	  if (waitResponse(1000L,GF(GSM_NL"+CNTP: 1")) != 1) {   
		 Serial.println("Err5");
       return "";
     }
	 
	
	sendAT(GF("+CCLK?"));
	
	int res1 = waitResponse(1000L,resp,GF("ERROR"), GF("+CCLK:"));
	
		if (res1 != 2) {
			waitResponse();
			return "Error1!";
		}
	
		resp = stream.readStringUntil('\n');
        waitResponse();
		resp.trim();
		resp.replace("+CCLK:", "");
		resp.replace("\"", "");
		resp.replace(","," ");
		
		_year = (2000+(resp.substring(0,2).toInt()));
		_month = resp.substring(3,5);
		_day = resp.substring(6,8);
		_Time = resp.substring(9,17);
	
	resp =String(_year);
	resp +="/"+String(_month);
	resp +="/"+String(_day);
	resp +=" "+String(_Time);
	
	
    return resp;
  }
  
  
  
  // works only with SIM808 V2
  bool getGPSTime(int *year, int *month, int *day, int *hour, int *minute, int *second) {
    bool fix = false;
    char chr_buffer[12];
    sendAT(GF("+CGNSINF"));
    if (waitResponse(GF(GSM_NL "+CGNSINF:")) != 1) {
      return false;
    }

    for (int i = 0; i < 3; i++) {
      String buffer = stream.readStringUntil(',');
      buffer.toCharArray(chr_buffer, sizeof(chr_buffer));
      switch (i) {
        case 0:
          //mode
          break;
        case 1:
          //fixstatus
          if ( buffer.toInt() == 1 ) {
            fix = buffer.toInt();
          }
          break;
        case 2:
          *year = buffer.substring(0,4).toInt();
          *month = buffer.substring(4,6).toInt();
          *day = buffer.substring(6,8).toInt();
          *hour = buffer.substring(8,10).toInt();
          *minute = buffer.substring(10,12).toInt();
          *second = buffer.substring(12,14).toInt();
          break;

        default:
          // if nothing else matches, do the default
          // default is optional
          break;
      }
    }
    String res = stream.readStringUntil('\n');
    waitResponse();

    if (fix) {
      return true;
    } else {
      return false;
    }
  }

};

#endif
