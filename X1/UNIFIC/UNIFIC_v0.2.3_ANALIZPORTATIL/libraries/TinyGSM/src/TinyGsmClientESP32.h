/**
 * @file       TinyGsmClientESP32.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */
 
 //SE HA MODIFICADO EL SERIAL POR SERIALUSB PARA USAR LA LIBRERIA CON SAMD21 MINI

#ifndef TinyGsmClientESP32_h
#define TinyGsmClientESP32_h
//#pragma message("TinyGSM:  TinyGsmClientESP32")

//#define TINY_GSM_DEBUG Serial

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 1024  //ANTES 512 
#endif

#define TINY_GSM_MUX_COUNT 5

#include <TinyGsmCommon.h>


#define GSM_NL "\r\n"
static const char GSM_OK[] TINY_GSM_PROGMEM = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;
static unsigned TINY_GSM_TCP_KEEP_ALIVE = 150;

// <stat> status of ESP32 station interface
// 2 : ESP32 station connected to an AP and has obtained IP
// 3 : ESP32 station created a TCP or UDP transmission
// 4 : the TCP or UDP transmission of ESP32 station disconnected
// 5 : ESP32 station did NOT connect to an AP
enum RegStatus {
  REG_OK_IP        = 2,
  REG_OK_TCP       = 3,
  REG_UNREGISTERED = 4,
  REG_DENIED       = 5,
  REG_UNKNOWN      = 6,
};



class TinyGsmESP32
{

public:

class GsmClient : public Client
{
  friend class TinyGsmESP32;
  typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

public:
  GsmClient() {}

  GsmClient(TinyGsmESP32& modem, uint8_t mux = 1) {
    init(&modem, mux);
  }

  bool init(TinyGsmESP32* modem, uint8_t mux = 1) {
    this->at = modem;
    this->mux = mux;
    sock_connected = false;

    at->sockets[mux] = this;

    return true;
  }

public:
  virtual int connect(const char *host, uint16_t port, int timeout_s) {
    stop();
    TINY_GSM_YIELD();
    rx.clear();
	//Serial.println("[tinygsm]: Prueba ubi 1");
    sock_connected = at->modemConnect(host, port, mux, false, timeout_s);
	Serial.println("[tinygsm]: RESPUESTA:"+String(sock_connected));
    return sock_connected;
  }

TINY_GSM_CLIENT_CONNECT_OVERLOADS()

  virtual void stop() {
    TINY_GSM_YIELD();
    at->sendAT(GF("+CIPCLOSE="), mux);
    sock_connected = false;
    at->waitResponse();
    rx.clear();
  }

TINY_GSM_CLIENT_WRITE()

TINY_GSM_CLIENT_AVAILABLE_NO_MODEM_FIFO()

TINY_GSM_CLIENT_READ_NO_MODEM_FIFO()

TINY_GSM_CLIENT_PEEK_FLUSH_CONNECTED()

  /*
   * Extended API
   */

  String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;

private:
  TinyGsmESP32* at;
  uint8_t         mux;
  bool            sock_connected;
  RxFifo          rx;
};


class GsmClientSecure : public GsmClient
{
public:
  GsmClientSecure() {}

  GsmClientSecure(TinyGsmESP32& modem, uint8_t mux = 1)
    : GsmClient(modem, mux)
  {}

public:
  virtual int connect(const char *host, uint16_t port, int timeout_s) {
    stop();
    TINY_GSM_YIELD();
    rx.clear();
    sock_connected = at->modemConnect(host, port, mux, true, timeout_s);
	
    return sock_connected;
  }
};
class GsmClientUDP : public GsmClient 
{
public:
  GsmClientUDP() {}

  GsmClientUDP(TinyGsmESP32& modem, uint8_t mux = 1)
    : GsmClient(modem, mux)
  {}
protected:
  uint8_t _sock;  // socket ID for Wiz5100
  uint16_t _remaining; // remaining bytes of incoming packet yet to be processed
  
public:

  virtual int connect(const char *host, uint16_t port, int timeout_s) {
    stop();
    TINY_GSM_YIELD();
    rx.clear();
    sock_connected = at->modemConnectUDP(host, port, mux, false, timeout_s);
	
    return sock_connected;
  }
  int available() {
	  return _remaining;
	}

};  

public:

  TinyGsmESP32(Stream& stream)
    : stream(stream)
  {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */

  bool begin(const char* pin = NULL) {
    return init(pin);
  }

  bool init(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    if (!testAT()) {
      return false;
    }
    sendAT(GF("E1"));   // Echo ON //off=0
    if (waitResponse() != 1) {
      return false;
    }
    sendAT(GF("+CIPMUX=1"));  // Enable Multiple Connections
    if (waitResponse() != 1) {
      return false;
    }
    sendAT(GF("+CWMODE=1"));  // Put into "station" mode
    if (waitResponse() != 1) {
      return false;
    }
	sendAT(GF("+CWAUTOCONN=0"));  // Put into "station" mode
    if (waitResponse() != 1) {
      return false;
    }
	sendAT(GF("+CWDHCP=1,1"));  // Put into "station" mode
    if (waitResponse() != 1) {
      return false;
    }
    DBG(GF("### Modem:"), getModemName());
    return true;
  }

  String getModemName() {
    return "ESP32";
  }

  void setBaud(unsigned long baud) {
    sendAT(GF("+UART="), baud, "8,1,0,0");
  }

TINY_GSM_MODEM_TEST_AT()

TINY_GSM_MODEM_MAINTAIN_LISTEN()

  bool factoryDefault() {
    sendAT(GF("+RESTORE"));
    return waitResponse() == 1;
  }

  String getModemInfo() {
    sendAT(GF("+GMR"));
    String res;
    if (waitResponse(1000L, res) != 1) {
      return "";
    }
    res.replace(GSM_NL "OK" GSM_NL, "");
    res.replace(GSM_NL, " ");
    res.trim();
    return res;
  }

  bool hasSSL() {
    return true;
  }

  bool hasWifi() {
    return true;
  }

  bool hasGPRS() {
    return false;
  }

  /*
   * Power functions
   */

  bool restart() {
    if (!testAT()) {
      return false;
    }
    sendAT(GF("+RST"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    if (waitResponse(10000L, GF(GSM_NL "ready" GSM_NL)) != 1) {
      return false;
    }
    delay(500);
    return init();
  }

  bool poweroff() {
    sendAT(GF("+GSLP=0"));  // Power down indefinitely - until manually reset!
    return waitResponse() == 1;
  }

  bool radioOff() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool sleepEnable(bool enable = true) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * SIM card functions
   */

  RegStatus getRegistrationStatus() {
    sendAT(GF("+CIPSTATUS"));
    if (waitResponse(3000, GF("STATUS:")) != 1) return REG_UNKNOWN;
    int status = waitResponse(GFP(GSM_ERROR), GF("2"), GF("3"), GF("4"), GF("5"));
	//Serial.println(status);
    waitResponse();  // Returns an OK after the status
    return (RegStatus)status;
  }

  /*
   * Generic network functions
   */
   
   int16_t _ActuTime(){   //MODIF!!!
	sendAT(GF("+CIPSNTPCFG=1,-1"));
	int res1 = waitResponse(GF("OK"));
	if (res1 != 1) {
      waitResponse();
      return 0;
    }
    
    return res1;
   }
   
   
    String _TakeTime(){  //MODIF!!!   "Fri Sep  3 13:33:48 2021"  -> 2021/09/03 13:33:48
	 String resp="";
	 String _date="";
	 String _day="";
	 String _month="";
	 int _year=0;
	 String _Time="";
	
    //unsigned long t_timeout = millis();
	 
	// do{
		sendAT(GF("+CIPSNTPTIME?"));

		int res1 = waitResponse(1000L,resp,GF("ERROR"), GF("+CIPSNTPTIME:"));
	
		if (res1 != 2) {
			waitResponse();
			return "Error1!";
		}
	
		if (waitResponse(1000L, resp) != 1) {
			return "Error2!";
		}
		
		resp.trim();
		resp.replace("AT+CIPSNTPTIME?\r\n",""); //AÑADIDO PARA PRUEBA ELMASA -> relacionado con nivel debug
		resp.replace("+CIPSNTPTIME:","");
		resp = resp.substring(4);
		Serial.println("PASO 1: "+String(resp));
		//	return resp;
		
		_year =((resp.substring(16,20)).toInt());
		_month = resp.substring(0,3);
		_day = resp.substring(4,6);
		if((_day.toInt())<10)
		{
		//	_day = "0"+String(_day.substring(1));
			_day.trim();
			_day = "0"+String(_day);
		}
		_Time = resp.substring(7,15);
		

    //while((_year < 2000) && (millis()-t_timeout<30000) );
	
	
	if (_month=="Jan")
			_month="01";
	else if (_month=="Feb")
			_month="02";
	else if (_month=="Mar")
			_month="03";
	else if (_month=="Apr")
			_month="04";
	else if (_month=="May")
			_month="05";
	else if (_month=="Jun")
			_month="06";
	else if (_month=="Jul")
			_month="07";
	else if (_month=="Aug")
			_month="08";
    else if (_month=="Sep")
			_month="09";
	else if (_month=="Oct")
			_month="10";
	else if (_month=="Nov")
			_month="11";
	else if (_month=="Dec")
			_month="12";
	
	resp =String(_year);
	resp +="/"+String(_month);
	resp +="/"+String(_day);
	resp +=" "+String(_Time);
	

    return resp;
   }
   

   
   

  int16_t getSignalQuality() {
    sendAT(GF("+CWJAP?"));
    int res1 = waitResponse(GF("No AP"), GF("+CWJAP:"));
    if (res1 != 2) {
      waitResponse();
      return 0;
    }
    streamSkipUntil(',');  // Skip SSID
    streamSkipUntil(',');  // Skip BSSID/MAC address
    streamSkipUntil(',');  // Skip Chanel number
    int res2 = stream.parseInt();  // Read RSSI
    waitResponse();  // Returns an OK after the value
    return res2;
  }

 int16_t getListAp() {
    sendAT(GF("+CWLAP"));
    int res1 = waitResponse();
    // if (res1 != 2) {
      // waitResponse();
      // return 0;
    // }
    // streamSkipUntil(',');  // Skip SSID
    // streamSkipUntil(',');  // Skip BSSID/MAC address
    // streamSkipUntil(',');  // Skip Chanel number
    // int res2 = stream.parseInt();  // Read RSSI
    // waitResponse();  // Returns an OK after the value
    return res1;
  }
  int16_t setTxPwr(int pwr) { //min 11(14 dBm), max 0 (19.5 dBm)
    sendAT(GF("+RFPOWER="),pwr);
    int res1 = waitResponse();
    // if (res1 != 2) {
      // waitResponse();
      // return 0;
    // }
    // streamSkipUntil(',');  // Skip SSID
    // streamSkipUntil(',');  // Skip BSSID/MAC address
    // streamSkipUntil(',');  // Skip Chanel number
    // int res2 = stream.parseInt();  // Read RSSI
    // waitResponse();  // Returns an OK after the value
    return res1;
  }
  
  bool isNetworkConnected()  {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_IP || s == REG_OK_TCP || s == REG_UNREGISTERED);
  }

  bool waitForNetwork(unsigned long timeout_ms = 60000L) {
    for (unsigned long start = millis(); millis() - start < timeout_ms; ) {
      sendAT(GF("+CIPSTATUS"));
      int res1 = waitResponse(3000, GF("busy p..."), GF("STATUS:"));
      if (res1 == 2) {
        int res2 = waitResponse(GFP(GSM_ERROR), GF("2"), GF("3"), GF("4"), GF("5"));
        if (res2 == 2 || res2 == 3) {
            waitResponse();
            return true;
         }
        }
      delay(250);
    }
    return false;
  }

  /*
   * WiFi functions
   */

  bool networkConnect(const char* ssid, const char* pwd) {
    sendAT(GF("+CWJAP=\""), ssid, GF("\",\""), pwd, GF("\""));
    if (waitResponse(60000L, GFP(GSM_OK), GF(GSM_NL "FAIL" GSM_NL)) != 1) {
      return false;
    }

    return true;
  }
  bool networkConnect(const char* ssid, const char* pwd,const char* bssid) {
    sendAT(GF("+CWJAP=\""), ssid, GF("\",\""), pwd,GF("\",\""), bssid, GF("\""));
    if (waitResponse(60000L, GFP(GSM_OK), GF(GSM_NL "FAIL" GSM_NL)) != 1) {
      return false;
    }

    return true;
  }
  String searchNearestAp(const char* ssid, unsigned long timeout = 10000){
	sendAT(GF("+CWLAP"));
	String data= "";
 //   int index = 0;
	String response = "";
	String mac = "";
	int pwr = 0;
	bool findSSID = false;
	//Serial.println ("Searching : " + String(ssid));
	unsigned long startMillis = millis();
	do {
		// while (stream.available() > 0) {
			// char c = stream.read();
			// Serial.write(c);
			// response += c;		
			// Serial.println();
// Serial.println("A");			
			// if ( response.endsWith(String(ssid)+ "\",")) {
				// Serial.println("B");
				// if( !findSSID ){
					// Serial.println("FINDSSID!!! DEBUG RTO");
					// pwr = stream.readStringUntil(',').toInt();
					// streamSkipUntil('"');
					// mac= stream.readStringUntil('"');
					// findSSID = true;
				// }else{
					// if (pwr < stream.readStringUntil(',').toInt()){
						// streamSkipUntil('"');
						// mac= stream.readStringUntil('"');
						// Serial.println("FINDSSID222222!!! DEBUG RTO");
					// }
// Serial.println("C");					
				// }		
	  // Serial.println("D");
			// }if ( response.endsWith(GFP(GSM_OK)))
				// Serial.println("FINDSSID33333333333!!! DEBUG RTO");
			// goto fin;
		// }
	// }while (millis()-startMillis < timeout);
			while (stream.available() > 0) {
			char c = stream.read();
			//Serial.write(c);
			response += c;			
			if ( response.endsWith(String(ssid)+ "\",")) {
				if( !findSSID ){
					pwr = stream.readStringUntil(',').toInt();
					streamSkipUntil('"');
					mac= stream.readStringUntil('"');
					findSSID = true;
				}else{
					if (pwr < stream.readStringUntil(',').toInt()){
						streamSkipUntil('"');
						mac= stream.readStringUntil('"');
					}						
				}		
	  
			}if ( response.endsWith(GFP(GSM_OK)))
			goto fin;
		}
	}while (millis()-startMillis < timeout);
fin:
Serial.println();
	Serial.print("pot: ");
	Serial.print(pwr);
	Serial.print(", ");
	Serial.print("mac: ");
	Serial.print(mac); 
	Serial.println();	
	return mac;	
  }

  bool networkDisconnect() {
    sendAT(GF("+CWQAP"));
    bool retVal = waitResponse(10000L) == 1;
    waitResponse(GF("WIFI DISCONNECT"));
    return retVal;
  }

  /*
   * IP Address functions
   */

  String getLocalIP() {
    sendAT(GF("+CIPSTA?")); //ANTES:   sendAT(GF("+CIPSTA??"));
    int res1 = waitResponse(GF("ERROR"), GF("+CWJAP:"));
    if (res1 != 2) {
      return "";
    }
    String res2 = stream.readStringUntil('"');
    waitResponse();
    return res2;
  }

  IPAddress localIP() {
    return TinyGsmIpFromString(getLocalIP());
  }
  
  int ping(String localIP) {
    sendAT(GF("+PING="),GF("\""),GF(localIP),GF("\""));
    int res1 = waitResponse(GF("ERROR"), GF("+PING:"));
    int res2 = stream.parseInt();
    waitResponse();
    return res2;
  }

  /*
   * Battery & temperature functions
   */

  // Use: float vBatt = modem.getBattVoltage() / 1000.0;
  uint16_t getBattVoltage() TINY_GSM_ATTR_NOT_AVAILABLE;
  int8_t getBattPercent() TINY_GSM_ATTR_NOT_AVAILABLE;
  uint8_t getBattChargeState() TINY_GSM_ATTR_NOT_AVAILABLE;
  bool getBattStats(uint8_t &chargeState, int8_t &percent, uint16_t &milliVolts) TINY_GSM_ATTR_NOT_AVAILABLE;
  float getTemperature() TINY_GSM_ATTR_NOT_AVAILABLE;

  /*
   * Client related functions
   */

protected:

  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75)
 {
    uint32_t timeout_ms = ((uint32_t)timeout_s)*3000;
/*     if (ssl) {
      sendAT(GF("+CIPSSLSIZE=4096"));
      waitResponse();
    } */
    sendAT(GF("+CIPSTART="), mux, ',', ssl ? GF("\"SSL") : GF("\"TCP"),
           GF("\",\""), host, GF("\","), port, GF(","), TINY_GSM_TCP_KEEP_ALIVE);
    // TODO: Check mux
    int rsp = waitResponse(timeout_ms,
                           GFP(GSM_OK),
                           GFP(GSM_ERROR),
                           GF("ALREADY CONNECT"));
    // if (rsp == 3) waitResponse();  // May return "ERROR" after the "ALREADY CONNECT"
	DBG("[tinygsm] RSP: "+String(rsp));
    return (1 == rsp);
  }
  bool modemConnectUDP(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75)
 {
    uint32_t timeout_ms = ((uint32_t)timeout_s)*2000;
    if (ssl) {
      sendAT(GF("+CIPSSLSIZE=4096"));
      waitResponse();
    }
    sendAT(GF("+CIPSTART="), mux, ',', ssl ? GF("\"SSL") : GF("\"UDP"),
           GF("\",\""), host, GF("\","), port, GF(","), TINY_GSM_TCP_KEEP_ALIVE);
    // TODO: Check mux
    int rsp = waitResponse(timeout_ms,
                           GFP(GSM_OK),
                           GFP(GSM_ERROR),
                           GF("ALREADY CONNECT"));
    // if (rsp == 3) waitResponse();  // May return "ERROR" after the "ALREADY CONNECT"
    return (1 == rsp);
  }


  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CIPSEND="), mux, ',', len);
    if (waitResponse(GF(">")) != 1) {
      return 0;
    }
    stream.write((uint8_t*)buff, len);
    stream.flush();
    if (waitResponse(10000L, GF(GSM_NL "SEND OK" GSM_NL)) != 1) {
      return 0;
    }
    return len;
  }

  bool modemGetConnected(uint8_t mux) {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_IP || s == REG_OK_TCP);
  }

public:

  /*
   Utilities
   */

TINY_GSM_MODEM_STREAM_UTILITIES()

  // TODO: Optimize this!
  uint8_t waitResponse(uint32_t timeout_ms, String& data,
                       GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    /*String r1s(r1); r1s.trim();
    String r2s(r2); r2s.trim();
    String r3s(r3); r3s.trim();
    String r4s(r4); r4s.trim();
    String r5s(r5); r5s.trim();
    DBG("### ..:", r1s, ",", r2s, ",", r3s, ",", r4s, ",", r5s);*/
    data.reserve(64);
    int index = 0;
    unsigned long startMillis = millis();
    do {
      TINY_GSM_YIELD();
      while (stream.available() > 0) {
        int a = stream.read();
		Serial.write(a);
        if (a <= 0) continue; // Skip 0x00 bytes, just in case
        data += (char)a;
        if (r1 && data.endsWith(r1)) {
          index = 1;
          goto finish;
        } else if (r2 && data.endsWith(r2)) {
          index = 2;
          goto finish;
        } else if (r3 && data.endsWith(r3)) {
          index = 3;
          goto finish;
        } else if (r4 && data.endsWith(r4)) {
          index = 4;
          goto finish;
        } else if (r5 && data.endsWith(r5)) {
          index = 5;
          goto finish;
        } else if (data.endsWith(GF(GSM_NL "+IPD,"))) {
          int mux = stream.readStringUntil(',').toInt();
          int len = stream.readStringUntil(':').toInt();
          int len_orig = len;
          if (len > sockets[mux]->rx.free()) {
            DBG("### Buffer overflow: ", len, "->", sockets[mux]->rx.free());
          } else {
            DBG("### Got: ", len, "->", sockets[mux]->rx.free());
          }
          while (len--) {
            TINY_GSM_MODEM_STREAM_TO_MUX_FIFO_WITH_DOUBLE_TIMEOUT
          }
          if (len_orig > sockets[mux]->available()) { // TODO
            DBG("### Fewer characters received than expected: ", sockets[mux]->available(), " vs ", len_orig);
          }
          data = "";
        } else if (data.endsWith(GF("CLOSED"))) {
          int muxStart = max(0,data.lastIndexOf(GSM_NL, data.length()-8));
          int coma = data.indexOf(',', muxStart);
          int mux = data.substring(muxStart, coma).toInt();
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->sock_connected = false;
          }
          data = "";
          DBG("### Closed: ", mux);
        }
      }
    } while (millis() - startMillis < timeout_ms);
finish:
    if (!index) {
      data.trim();
      if (data.length()) {
        DBG("### Unhandled:", data);
      }
      data = "";
    }
    //DBG('<', index, '>');
    return index;
  }

  uint8_t waitResponse(uint32_t timeout_ms,
                       GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5);
  }

  uint8_t waitResponse(GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    return waitResponse(3000, r1, r2, r3, r4, r5);
  }

public:
  Stream&       stream;

protected:
  GsmClient*    sockets[TINY_GSM_MUX_COUNT];
};

#endif
