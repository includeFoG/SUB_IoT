/**
 * @file       TinyGsmClientSaraR4.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmClientSaraR5_h
#define TinyGsmClientSaraR5_h
//#pragma message("TinyGSM:  TinyGsmClientSaraR5")

//#define TINY_GSM_DEBUG Serial

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 512 //original: 64
#endif

#define TINY_GSM_MUX_COUNT 7

#include <TinyGsmCommon.h>

#define GSM_NL "\r\n"
static const char GSM_OK[] TINY_GSM_PROGMEM = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;
static const char GSM_CME_ERROR[] TINY_GSM_PROGMEM = GSM_NL "+CME ERROR:";

enum SimStatus {
  SIM_ERROR = 0,
  SIM_READY = 1,
  SIM_LOCKED = 2,
};

enum RegStatus {
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};


class TinyGsmSaraR5
{

public:

class GsmClient : public Client
{
  friend class TinyGsmSaraR5;
  typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

public:
  GsmClient() {}

  GsmClient(TinyGsmSaraR5& modem, uint8_t mux = 0) {
    init(&modem, mux);
  }

  bool init(TinyGsmSaraR5* modem, uint8_t mux = 0) {
    this->at = modem;
    this->mux = mux;
    sock_available = 0;
    prev_check = 0;
    sock_connected = false;
    got_data = false;

    at->sockets[mux] = this;

    return true;
  }

public:

  virtual int connect(const char *host, uint16_t port, int timeout_s) { 
    stop();
    TINY_GSM_YIELD();
    rx.clear();

    uint8_t oldMux = mux;
    sock_connected = at->modemConnect(host, port, &mux, false, timeout_s); 
    if (mux != oldMux) {
        DBG("WARNING--:  Mux number changed from", oldMux, "to", mux);
        at->sockets[oldMux] = NULL;
    }
    at->sockets[mux] = this;
    at->maintain();

    return sock_connected;
  }

TINY_GSM_CLIENT_CONNECT_OVERLOADS()

  virtual void stop() {
   unsigned long timeOutStop = 0;
    TINY_GSM_YIELD();
    // Read and dump anything remaining in the modem's internal buffer.
    // The socket will appear open in response to connected() even after it
    // closes until all data is read from the buffer.
    // Doing it this way allows the external mcu to find and get all of the data
    // that it wants from the socket even if it was closed externally.
    rx.clear();
    at->maintain();
	timeOutStop = millis();
    while (sock_connected && sock_available > 0 && ((millis()-timeOutStop) < 120000)) { //while (sock_connected && sock_available > 0 ){
      at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
      rx.clear();
      at->maintain();
    }
    at->modemDisconnect(mux);
  }

TINY_GSM_CLIENT_WRITE()

TINY_GSM_CLIENT_AVAILABLE_WITH_BUFFER_CHECK()
TINY_GSM_CLIENT_READ_WITH_BUFFER_CHECK()

//TINY_GSM_CLIENT_AVAILABLE_NO_BUFFER_CHECK()
//TINY_GSM_CLIENT_READ_NO_BUFFER_CHECK()

TINY_GSM_CLIENT_PEEK_FLUSH_CONNECTED()

  /*
   * Extended API
   */

  String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;

private:
  TinyGsmSaraR5*   at;
  uint8_t         mux;
  uint16_t        sock_available;
  uint32_t        prev_check;
  bool            sock_connected;
  bool            got_data;
  RxFifo          rx;
};


class GsmClientSecure : public GsmClient
{
public:
  GsmClientSecure() {}

  GsmClientSecure(TinyGsmSaraR5& modem, uint8_t mux = 1)
    : GsmClient(modem, mux)
  {}

public:

  virtual int connect(const char *host, uint16_t port, int timeout_s) {
    stop();
    TINY_GSM_YIELD();
    rx.clear();
	
    uint8_t oldMux = mux;
    sock_connected = at->modemConnect(host, port, &mux, true, timeout_s);
    if (mux != oldMux) {
        DBG("WARNING!!:  Mux number changed from", oldMux, "to", mux);
        at->sockets[oldMux] = NULL;
    }
    at->sockets[mux] = this;
    at->maintain();
	
    return sock_connected;
  }
};


public:

  TinyGsmSaraR5(Stream& stream)
    : stream(stream)
  {
    memset(sockets, 0, sizeof(sockets));
  }
  
  
  /*
   * GPS location functions
   */
   
   
     // GPS ON
  uint8_t enableGPS(uint8_t mode=1, uint8_t aidMode=0, uint8_t GSNSSSystem=67, bool uggga=true, bool uggll=true) {   //GSNSSSystem=1 
	String resp="";
	uint8_t ind=0;
	
	waitResponse();
	
	sendAT(GF("+UGPS?"));
	waitResponse(30000L,resp,GF("+UGPS: "));
	resp = stream.readStringUntil('\n');
	DBG(resp);
	
	// if(resp.indexOf("Unhandled:")!=-1){
		// resp = stream.readStringUntil('\n');
		// DBG(resp);
	// }
	waitResponse();
	
	if (resp.indexOf(String(mode)+","+String(aidMode)+","+String(GSNSSSystem))==-1)
	{
		sendAT(GF("+UGPS="),mode,',',aidMode,',',GSNSSSystem);  //SI SE CONFIGURA EL AID HAY QUE AÑADIR UGIND (ver mas comandos atcommands)
		ind = waitResponse(30000L,resp,GF("OK"),GF("+CME ERROR:"));
	
		if ((ind != 1) && (ind !=2)) {
			waitResponse();
			resp = stream.readStringUntil('\n');
		    return 2; //ERROR 2: UGPS NO RESPONDE OK
		}
	
		if(ind == 2){
			resp = stream.readStringUntil('\n');
			if (resp.indexOf("GPS aiding mode already set")==-1){
				return 3; //ERROR 3: CME ERROR
			}
		}
	}

	 sendAT(GF("+UGGGA="),uggga);  //HABILITA MENSAJES NMEA
	 if (waitResponse()!=1){
		 return 4; //ERROR 4: UGGGA NO RESPONDE OK
	 }
	 sendAT(GF("+UGGLL="),uggll); //HABILITA MANTENER EL ULTIMO MENSAJE NMEA, (NMEA es volatil)
	 if(waitResponse()!=1){
		 return 5; //ERROR 5: UGGLL NO RESPONDE OK
	 }
	 
    return 1;
  }

  
  
//GPS OFF
  bool disableGPS() {
	  sendAT(GF("+UGGGA=0")); 
	 if (waitResponse()!=1){
		 return false;
	 }
	 
    sendAT(GF("+UGPS=0"));
    if (waitResponse() != 1) {
      return false;
    }
    return true;
  }

 // Get GNSS fix data
  String getGPS_NMEA() {
	String resp="";
	uint8_t ind=0;
	
    sendAT(GF("+UGGGA?"));
	ind = waitResponse(10000L,resp,GF("ERROR"),GF("+UGGGA:")); //desplaza la respuesta hasta "String1" ó "String2" y la almacena en resp Responde con (int) según el valor por el que salga
	if(ind!=2) { 
		waitResponse();  
		DBG("CARE HERE2");
		return "Err1"; //ERROR 1: La respuesta no contiene +UGGGA:
	}
	resp = stream.readStringUntil('\n'); //obtiene la trama respuesta
	DBG("////////");
	DBG(resp);
	DBG("////////");
	if(waitResponse(15000L)!=1){ // espera el OK
		DBG("ErrOK");//ERROR 2: EL MODULO NO DEVUELVE OK
	}
    if(resp.indexOf("1,Not available")!=-1){
		enableGPS();
		return "Err3"; //ERROR 3: UGPS Disabled
	}
	if(resp == " 0,NULL"){
		enableGPS();
	    return "Err4"; //ERROR 4: NMEA Disabled
	}

	return resp;
  }
  

  // get GPS informations
  String getGPS() {
    String resp="";
    uint8_t ind=0;

    sendAT(GF("+UGGLL?"));
	ind = waitResponse(10000L,resp,GF("ERROR"),GF("+UGGLL:"));
	if(ind!=2) {
		waitResponse();
		return "false";
	}
	resp = stream.readStringUntil('\n');
	waitResponse();
	return resp;
	
   // if(waitResponse(GF(GSM_NL "+UGGLL: 0,NULL"))!=1){
         // return "UGGLL disabled";
	// }
  }
   
   
   //WORKING
 // String cellLocate(){ //ULOC //ULOCIND //ULOCAID //ULOCGNSS //ULOCCELL //UGPS  //REQUIRES ACTIVATION OF PSD CONTEX (UPSD, UPSDA, UPSND)   pag401-402
 // } 

  
  String takeTimeGPS(){
	  String resp ="";
	  uint8_t ind=0;
	  
	sendAT(GF("+UGZDA=1"));
	if(waitResponse(10000L)!=1){
	   return "1";
	}
	  
    sendAT(GF("+UGZDA?"));
	ind = waitResponse(10000L,resp,GF("ERROR"),GF("+UGZDA:"));
	if(ind!=2) {
		waitResponse();
		return "2";
	}
	
	resp = stream.readStringUntil('\n');
	if(waitResponse()!=1){
		return "3";
	}
	if(resp ==" 1,Not available"){
		return "4";
	}

	return resp;
  }
  

  
  //25.3.11 Get number of GNSS satellites in view +UGGSV
  String satellitesViewGPS(){
   String resp="";
   uint8_t ind=0;
   
   	sendAT(GF("+UGGSV=1"));
	if(waitResponse(10000L)!=1){
	   return "1"; //ERROR 1:
	}
	
	sendAT(GF("+UGGSV?"));
	ind = waitResponse(10000L,resp,GF("ERROR"),GF("+UGGSV:"));
	if(ind!=2){
		waitResponse();
		return "2"; //ERROR 2:
	}
	resp = stream.readStringUntil('K'); //K de OK
	if(waitResponse()!=1){
		return "3"; //ERROR 3:
    }
	
	return resp;
  }
  
  //deshabilita los mensajes de UGGSV (satellites in view)
  bool disableSatViewMessage(){
    sendAT(GF("+UGGSV=0"));
	if(waitResponse()!=1){
		return false;
	}
	
	return true;
  }
  
  String satelliteInfo(){
   String resp = "";
   uint8_t ind=0;
   
   sendAT("+UGGSA=1");
   if(waitResponse(10000L)!=1){
	   return "1"; //ERROR 1:
	}
	
	sendAT(GF("+UGGSA?"));
	ind = waitResponse(10000L,resp,GF("ERROR"),GF("+UGGSA:"));
	if(ind!=2){
		waitResponse();
		return "2"; //ERROR 2:
	}
	resp = stream.readStringUntil('K');//K DE OK
	if(waitResponse()!=1){
		return "3"; //ERROR 3:
    }
	
	return resp;
  }
  
  bool disableSatelliteInfo(){
	  sendAT(GF("+UGGSA=0"));
	  if(waitResponse(10000L)!=1){
		  return false;
	  }
   return true;
  }
   
   //habilita el almacenamiento de mensajes NMEA (RMC)
   //Get recommended minimum GNSS data +UGRMC
   String minimumGPSdata(){
	String resp="";
	uint8_t ind=0;
	
	sendAT(GF("+UGRMC=1"));
	if(waitResponse(10000L)!=1){
	   return "1"; //ERROR 1:
	}
	
	sendAT(GF("+UGRMC?"));
	ind = waitResponse(10000L,resp,GF("ERROR"),GF("+UGRMC:"));
	if(ind!=2){
		waitResponse();
		return "2"; //ERROR 2:
	}
	resp = stream.readStringUntil('\n');
	if(waitResponse()!=1){
		return "3"; //ERROR 3:
    }
	//ERROR 4: resp == "Not available"
	return resp;
   }
   
   bool disableMinimumGPSFrame(){
	sendAT(GF("+UGRMC=0"));
	if(waitResponse()!=1){
		return false;
	}
	return true;	
   }
   
   
   //Track made and ground speed
   String trackGroundSpeed(){
	   String resp = "";
	   uint8_t ind=0;
	   
	   sendAT(GF("+UGVTG=1"));
	   if(waitResponse(10000L)!=1){
	   return "1"; //ERROR 1:
	}
	
	sendAT(GF("+UGVTG?"));
	ind = waitResponse(10000L,resp,GF("ERROR"),GF("+UGVTG:"));
	if(ind!=2){
		waitResponse();
		return "2"; //ERROR 2:
	}
	resp = stream.readStringUntil('\n');
	if(waitResponse()!=1){
		return "3"; //ERROR 3:
    }
	
	return resp;
   }
   
   bool disableTrrackGroundSpeed(){
   	sendAT(GF("+UGVTG=0"));
	if(waitResponse()!=1){
		return false;
	}
	return true;
   }
   
   
   
   
   
 ////////////////////////////////////////////////////////////////////////////

  /*
   * Basic functions
   */
 
		
  bool begin(const char* pin = NULL) {
    return init(pin);
  }

  bool init(const char* pin = NULL) { 
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    if (!testAT()) {
		waitResponse();
      return false;
    }
	waitResponse();
    sendAT(GF("E0"));   // Echo Off
    if (waitResponse() != 1) {
      return false;
    }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();

    getModemName();

    int ret = getSimStatus();
    // if the sim isn't ready and a pin has been provided, try to unlock the sim
    if (ret != SIM_READY && pin != NULL && strlen(pin) > 0) {
      simUnlock(pin);
      return (getSimStatus() == SIM_READY);
    }
    // if the sim is ready, or it's locked but no pin has been provided, return true
    else {
      return (ret == SIM_READY || ret == SIM_LOCKED);
    }
  }

  String getModemName() {
    sendAT(GF("+CGMI"));
    String res1;
    if (waitResponse(5000L, res1) != 1) {
      return "u-blox Cellular Modem";
    }
    res1.replace(GSM_NL "OK" GSM_NL, "");
    res1.trim();

    sendAT(GF("+GMM"));
    String res2;
    if (waitResponse(5000L, res2) != 1) {
      return "u-blox Cellular Modem";
    }
    res2.replace(GSM_NL "OK" GSM_NL, "");
    res2.trim();

    String name = res1 + String(' ') + res2;
    DBG("### Modem:", name);
    if (!name.startsWith("u-blox SARA-R5") && !name.startsWith("u-blox SARA-N4")) {
      DBG("### WARNING:  You are using the wrong TinyGSM modem!");
	  DBG(name);
    }

    return name;
  }

TINY_GSM_MODEM_SET_BAUD_IPR()

TINY_GSM_MODEM_TEST_AT()

TINY_GSM_MODEM_MAINTAIN_CHECK_SOCKS()

  bool factoryDefault() {
    sendAT(GF("&F"));  // Resets the current profile, other NVM not affected
    return waitResponse() == 1;
  }

TINY_GSM_MODEM_GET_INFO_ATI()

  bool hasSSL() {
    return true;
  }

  bool hasWifi() {
    return false;
  }

  bool hasGPRS() {
    return true;
  }

  /*
   * Power functions
   */
   
  bool ATtest() { 
  delay(200);
    DBG(GF("### ATtest"));
    if (!testAT()) {
		waitResponse();
		 DBG(GF("### e"));
      return false;
    }
	waitResponse();
	 DBG(GF("### ok"));
	return true;
  }

  bool restart() {
    if (!testAT()) {
		waitResponse();
      return false;
    }
	waitResponse();
	
    sendAT(GF("+CFUN=16"));
    if (waitResponse(10000L) != 1) {
        if (waitResponse(10000L) != 1) {  //añadido doble filtrado 23/02 (en ocasiones se queda aquí enclavado al no obtener respuesta)
			return false;
		}
    }
    delay(7000);  // TO DO:  Verify delay timing here ORIGINAL:3000 //antes:10s
    return init();
  }
  
  bool cleanRestart(){
	if (!testAT()) {
		waitResponse();
      return false;
    }
	waitResponse();
	
    sendAT(GF("+CFUN=0"));
    if (waitResponse(10000L,GF("OK"))!=1) {
		waitResponse();
		return false;
	}
	else{
		delay(6000);  // TO DO:  Verify delay timing here ORIGINAL:3000 //antes:10s
	    sendAT(GF("+CFUN=1"));
		if (waitResponse(10000L) != 1) {
			return false;
		}
	}
	 delay(6000);  // TO DO:  Verify delay timing here ORIGINAL:3000 //antes:10s
    return init();
  }

  bool poweroff() {
	  if(ATtest()){
			sendAT(GF("+CPWROFF"));
			return waitResponse(40000L) == 1;
	  }
	  else
	  {
		  DBG("EL MODEM NO RESPONDE");
		  return false;
	  }
  }

  bool radioOff() {
	  
  String resp="";
  
 delay(3000); 
	 sendAT(GF("+CFUN=0"));
	int res1 = waitResponse(10000L,resp,GF("OK"));
		if (res1 != 1) {
			waitResponse();
			return false;
		}
		else
			DBG("SaraR5 en funcionalidad mínima");
		
    return true;
  }

  bool sleepEnable(bool enable = true) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  
  
  
  
   
String _TakeTime(){  //MODIF!!!   "21/08/31,15:42:50+04"  -> 2021/08/31 15:42:50
	 String _date="";
	 String _day="";
	 String _month="";
	 int _year=0;
	 String _Time="";
	 String resp="";
	 
	 waitResponse();
	
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
	
	if(_year < 2022){
		return "Error2!";
	}
    return resp;
   }
   
     ////// String getGsmLocation() {
    ////// sendAT(GF("+ULOC=2,3,0,120,1"));
    ////// if (waitResponse(30000L, GF(GSM_NL "+UULOC:")) != 1) {
      ////// return "";
    ////// }
    ////// String res = stream.readStringUntil('\n');
    ////// waitResponse();
    ////// res.trim();
    ////// return res;
  ////// }
  
   
  // bool mqttConnect(const char* _broker,int _port, const char* clientID, const char* user, const char* pass,const char* apn, int timeOut=60) {  
// delay(5000);
    // TINY_GSM_YIELD();

// bool F_name=false;

  	// sendAT(GF("+CGATT?"));  
	// waitResponse(10000L);	
	// sendAT(GF("+CREG?"));  
	// waitResponse(10000L);		
	// sendAT(GF("+CEREG?"));   
	// waitResponse(10000L);
	// sendAT(GF("+CGREG?"));   
	// waitResponse(10000L);
	// sendAT(GF("+COPS?"));    
	// waitResponse(10000L); 
	// sendAT(GF("+CGPADDR=1"));   
	// waitResponse(10000L);
	
	
	//////COMPROBACION DE PRECONEXION
	// sendAT(GF("+UMQTTC=4,0,\"ASTICAN/IoT/TEST\""));      
	// if (waitResponse(10000,GF(GSM_NL "+UMQTTC: 4,1")) == 1) {
		// waitResponse(10000);
		// sendAT(GF("+UMQTTC=5,\"ASTICAN/IoT/TEST\""));  
		// if(waitResponse(10000L)==1)
			// return true;
    // }
	// delay(1000);
	
    // sendAT(GF("+CGATT=0"));  // attach to GPRS
    // if (waitResponse(210000L) != 1) { //360000L
      // return false;
    // }
	// delay(1000);
	
	  // sendAT(GF("+CFUN=0"));
    // if (waitResponse(10000L) != 1) {
      // return false;
    // }
	
		  // sendAT(GF("+CFUN=16"));
    // if (waitResponse(10000L) != 1) {
      // return false;
    // }	
	// delay(6000);
	
		  // sendAT(GF("+CFUN=1"));
    // if (waitResponse(10000L) != 1) {
      // return false;
    // }		
	
	
	// delay(6000);//antes 6000
	
	// sendAT(GF("+UMQTTNV=0"));
    // if (waitResponse(10000L) != 1) {
      // return false;
    // }		
	

	
	// sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');  // Define PDP context 1
    // if(waitResponse(10000L) != 1){
		// return false;
	// }
	
	
	////// sendAT(GF("+UMNOPROF=0"));  
    ////// if (waitResponse(10000L) != 1) { 
      ////// return false;
    ////// }
	
	
	 // sendAT(GF("+CFUN=1"));
    // if (waitResponse(10000L) != 1) {
      // return false;
    // }
	// delay(6000);//antes 6000
		// sendAT(GF("+COPS=1,2,21407"));        // Manual-register to the network // Manual-register to the network  sendAT(GF("+COPS=4,2,21407")); AT+COPS=4,0,"Movistar",7
    // if (waitResponse(210000L) != 1) {
      // return false;
    // }
	
	 ////// sendAT(GF("+CGATT=1"));  // attach to GPRS
    ////// if (waitResponse(210000L) != 1) { //360000L
      //////return false;
    ////// }
	
	
	 ////// sendAT(GF("+CEREG=1"));  // attach to GPRS
    ////// if (waitResponse(210000L) != 1) { //360000L
      ////// return false;
    ////// }
	 // delay(1000);
	
  	// sendAT(GF("+CGATT?"));  
	// waitResponse(10000L);	
	// sendAT(GF("+CREG?"));  
	// waitResponse(10000L);		
	// sendAT(GF("+CEREG?"));   
	// waitResponse(10000L);
	// sendAT(GF("+CGREG?"));   
	// waitResponse(10000L);
	// sendAT(GF("+COPS?"));   
	// waitResponse(10000L); 
	// sendAT(GF("+UMNOPROF?"));   
	// waitResponse(10000L); 
	// sendAT(GF("+CGPADDR=1"));   
	// waitResponse(10000L);
	
	
	// sendAT(GF("+CPSMS=0"));  
    // if (waitResponse(10000L) != 1) { 
      // return false;
    // }
	
	// sendAT(GF("+CEDRXS=0"));  
    // if (waitResponse(10000L) != 1) { 
      // return false;
    // }
	
	// if(F_name==false){
	// F_name=true;
	// sendAT(GF("+UMQTT=0,\""),clientID,GF("\""));      //cliente      
    // if (waitResponse(10000L) != 1) {
      // return false;
    // }
	// }
	// else{
		// F_name=false;
		// sendAT(GF("+UMQTT=0,\"100000002\""));          
    // if (waitResponse(10000L) != 1) {
      // return false;
    // }
	// }
	
	
	// sendAT(GF("+UMQTT=1,"),_port);        //puerto
    // if (waitResponse(10000L) != 1) {
      // return false;
    // }
	
	
	//////sendAT(GF("+UMQTT=2,"),_broker,GF(","),_port);      //servidor
    ////// if (waitResponse(10000L) != 1) {
      ////// return false;
    ////// }
	
	// sendAT(GF("+UMQTT=3,\"161.22.47.43\","),_port);      //servidor
    // if (waitResponse(10000L) != 1) {
      // return false;
    // }
	
	// sendAT(GF("+UMQTT=4,\""),user,GF("\",\""), pass, '"');     //username + password
    // if (waitResponse(10000L) != 1) {
      // return false;
    // }
	
	// sendAT(GF("+UMQTT=10,"),timeOut);    //10, <timeout>
    // if (waitResponse(10000L) != 1) {
      // return false;
    // }
	
    ////// sendAT(GF("+UMQTT=11,0")); //10, <securemode>  0 no secure , 1 secure
    ////// if (waitResponse(10000L) != 1) {
      ////// return false;
    ////// }
	
	
	// sendAT(GF("+UMQTT=12,1")); //12,1 clean sesion //12,0 no clean sesion
    // if (waitResponse(10000L) != 1) {
      // return false;
    // }
	
  	// sendAT(GF("+CGATT?"));  
	// waitResponse(10000L);	
	// sendAT(GF("+CREG?"));  
	// waitResponse(10000L);		
	// sendAT(GF("+CEREG?"));   
	// waitResponse(10000L);
	// sendAT(GF("+CGREG?"));   
	// waitResponse(10000L);
	// sendAT(GF("+COPS?"));   
	// waitResponse(10000L); 
	// sendAT(GF("+CGPADDR=1"));   
	// waitResponse(10000L);
	
	
	// delay(5000);//antes 5000

	
	// sendAT(GF("+UMQTTC=1"));
	// waitResponse();
	// if(waitResponse(150000,GF(GSM_NL "+UMQTTC: 1,1")) != 1) {
		// sendAT(GF("+UMQTTER?"));      
	    // waitResponse(10000L);
		// return false;
	// }
	// else if (waitResponse(120000,GF(GSM_NL "+UUMQTTC: 1,0")) != 1) {
		// sendAT(GF("+UMQTTER?"));      
		// waitResponse(10000L);
			// return false;
    // }
	
	// sendAT(GF("+UMQTTER?"));      
	// waitResponse(10000L);
	
	// delay(1000);
	
	//////////////////////////////////////////////////////////////////////////
	
	////// sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');  // Define PDP context 1
    ////// waitResponse();
	
   ////// sendAT(GF("+CGATT=1"));  // attach to GPRS
    /////// if (waitResponse(210000L) != 1) { //360000L
      ////// return false;
    ////// }
				
	////// sendAT(GF("+CPSMS=0"));  
    ////// if (waitResponse(10000L) != 1) { 
      ////// return false;
    //////}
	////// sendAT(GF("+CEDRXS=0"));  
    ////// if (waitResponse(10000L) != 1) { 
      ////// return false;
    ////// }
			
				
      ////// sendAT(GF("+CGATT=0"));  // detach from GPRS
    ////// if (waitResponse(200000L) != 1) { //ORIGINAL:360000L
      ////// return false;
    ////// }
	////// delay(5000);
	
	  ////// sendAT(GF("+CFUN=1"));
    ////// if (waitResponse(10000L) != 1) {
      ////// return false;
    ////// }
	
	
   ////// sendAT(GF("+CFUN=16"));
    ////// if (waitResponse(10000L) != 1) {
      ////// return false;
     ////// }
	
   ////// delay(6000);
   ////// sendAT(GF("+UMQTT=0,\""),clientID,GF("\""));      //cliente      
    ////// if (waitResponse(10000L) != 1) {
      ////// return false;
    ////// }
	
	
	////// sendAT(GF("+UMQTT=1,"),_port);        //puerto
    ////// if (waitResponse(10000L) != 1) {
      ////// return false;
    ////// }
	
	
	////// sendAT(GF("+UMQTT=2,"),_broker,GF(","),_port);      //servidor
    ////// if (waitResponse(10000L) != 1) {
      ////// return false;
    ////// }
	
	////// sendAT(GF("+UMQTT=4,\""),user,GF("\",\""), pass, '"');     //username + password
    ////// if (waitResponse(10000L) != 1) {
      ////// return false;
    ////// }
	
	////// sendAT(GF("+UMQTTNV=2"));     
    ////// if (waitResponse() != 1) {
      ////// return false;
    ////// }
		////// sendAT(GF("+UMQTTNV=1"));     
    ////// if (waitResponse() != 1) {
      ////// return false;
   ////// }
	////// delay(5000);
	////// sendAT(GF("+UMQTTC=1"));      
	////// if (waitResponse(120000,GF(GSM_NL "+UUMQTTC: 1,")) != 1) {
			////// return false;
   ////// }
	
	
	// return true;
  // }
  
  // bool mqttDisc(){
	  // sendAT(GF("+UMQTTC=0"));      
    // if (waitResponse(10000L) != 1) {
      // return false;
    // }
	
	// return true;
  // }
  
  // bool mqttSend(const char* _topic, const char* _message, const int _QoS=1, bool Perm=1) {  
// delay(1000);
	// sendAT(GF("+UMQTTC=2,"),_QoS,GF(","),Perm,GF(",\""),_topic,GF("\",\""),_message,GF("\""));   
    // if (waitResponse(20000,",1") != 1) {
	  // DBG("TINYGSM: ERROR AL ENVIAR MENSAJE");
      // return false;
    // }
	// waitResponse();
	// DBG("TINYGSM: MENSAJE ENVIADO");
	
	//// sendAT(GF("+UMQTTC=0"));
    //// if (waitResponse(10000L) != 1) {
      //// DBG("TINYGSM: MENSAJE ENVIADO -> ERROR AL DESCONECTAR DEL BROKER");
      //// return false;
    //// }
	
	// return true;
  // }

   

  /*
   * SIM card functions
   */

TINY_GSM_MODEM_SIM_UNLOCK_CPIN()

TINY_GSM_MODEM_GET_SIMCCID_CCID()

  String getIMEI() {
    sendAT(GF("+CGSN"));
    if (waitResponse(GF(GSM_NL)) != 1) {
      return "";
    }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  SimStatus getSimStatus(unsigned long timeout_ms = 10000L) {
	  uint8_t cntr=0;
	  
    for (unsigned long start = millis(); millis() - start < timeout_ms; ) {
      sendAT(GF("+CPIN?"));
      if (waitResponse(GF(GSM_NL "+CPIN:")) != 1) {
        delay(1000);
		cntr++;
		
		if(cntr>=3){
			return SIM_ERROR;
		}
		
        continue;
      }
      int status = waitResponse(GF("READY"), GF("SIM PIN"), GF("SIM PUK"), GF("SIM not inserted"));
      waitResponse();
      switch (status) {
        case 2:  
        case 3:  return SIM_LOCKED;
        case 1:  return SIM_READY;
        default: 

			return SIM_ERROR;
      }
    }
    return SIM_ERROR;
  }


TINY_GSM_MODEM_GET_REGISTRATION_XREG(CEREG)

TINY_GSM_MODEM_GET_OPERATOR_COPS()

  /*
   * Generic network functions
   */

TINY_GSM_MODEM_GET_CSQ()

 bool isNetworkConnected() {
	DBG("TINYGSM: SARAR5");
    delay(3000);
	
	waitResponse();
    RegStatus s = getRegistrationStatus();
	waitResponse();
	
    if (s == REG_OK_HOME || s == REG_OK_ROAMING)
	{
		  sendAT(GF("+CGATT?"));  //NUEVO VERSION UMNOPROF
		  if(waitResponse(10000L,GF("+CGATT: 1"))==1){
				DBG("TINYGSM: MODEM CONECTADO");
				waitResponse();
				
				// uint8_t mux_;
		
				// sendAT(GF("+USOCR=6"));  // create a socket 
				// if (waitResponse(GF(GSM_NL "+USOCR:")) == 1) {  // reply is +USOCR: ## of socket created
					// mux_ = stream.readStringUntil('\n').toInt();
					// waitResponse();
					// delay(500);
					// sendAT(GF("+USOCL="), mux_);
					// waitResponse(120000L);  // can take up to 120s to get a response
					// return true;
				// }
				
				sendAT(GF("+CGACT=1,1"));  
				if (waitResponse(150000L) != 1) {
					DBG("TINY GSM: ER0");
					return false;
				}
				
				// bool b_upsda=true;
				
				// delay(2000);
				// sendAT(GF("+UPSDA=0,3")); 
				// if (waitResponse(150000L) != 1) {
					// delay(2000);
					// sendAT(GF("+UPSDA=0,3")); 
					// if (waitResponse(150000L) != 1) {
						// DBG("TINY GSM: 0F01");
						// b_upsda=false;
						////return false;
						
					// }
				// }
				// if(b_upsda){
					// String resp0="";
					// uint8_t ind0=0;
			
					// ind0 = waitResponse(150000L,resp0,GF("ERROR"),GF("+UUPSDA: 0,"));
					// if(ind0 != 2){
						// DBG("TINY GSM: 0F02");
						// waitResponse();
						////return false;
					// }
					// resp0 = stream.readStringUntil('\n');
					// if (resp0.length() < 14)
					// {
						// DBG("TINY GSM: 0F03");
						////return false;
					// }
				// }
				return true;
			}
			else 
				return false;
	}
    else if (s == REG_UNKNOWN)  // for some reason, it can hang at unknown..
      return isGprsConnected();
    else return false;
  }

 bool isNetworkConnected1() {
	DBG("TINYGSM: SARAR5");
    delay(3000);
	
    RegStatus s = getRegistrationStatus();
    if (s == REG_OK_HOME || s == REG_OK_ROAMING)
	{
		  sendAT(GF("+CGATT?"));  //NUEVO VERSION UMNOPROF
		  if(waitResponse(10000L,GF("+CGATT: 1"))==1){
				DBG("TINYGSM: MODEM CONECTADO");
				waitResponse();
				
				// uint8_t mux_;
		
				// sendAT(GF("+USOCR=6"));  // create a socket 
				// if (waitResponse(GF(GSM_NL "+USOCR:")) == 1) {  // reply is +USOCR: ## of socket created
					// mux_ = stream.readStringUntil('\n').toInt();
					// waitResponse();
					// delay(500);
					// sendAT(GF("+USOCL="), mux_);
					// waitResponse(120000L);  // can take up to 120s to get a response
					// return true;
				// }
				
				sendAT(GF("+CGACT=1,1"));  
				if (waitResponse(150000L) != 1) {
					DBG("TINY GSM: ER0");
					return false;
				}
				
				delay(2000);
				sendAT(GF("+UPSDA=0,3")); 
				if (waitResponse(150000L) != 1) {
					delay(2000);
					sendAT(GF("+UPSDA=0,3")); 
					if (waitResponse(150000L) != 1) {
						DBG("TINY GSM: ER1");
						return false;
					}
				}
				
				String resp0="";
				uint8_t ind0=0;
			
				ind0 = waitResponse(150000L,resp0,GF("ERROR"),GF("+UUPSDA: 0,"));
				if(ind0 != 2){
					DBG("TINY GSM: ER2");
					waitResponse();
					return false;
				}
				resp0 = stream.readStringUntil('\n');
				if (resp0.length() < 14)
				{
					DBG("TINY GSM: ER3");
					return false;
				}
				return true;
			}
			else 
				return false;
	}
    else if (s == REG_UNKNOWN)  // for some reason, it can hang at unknown..
      return isGprsConnected();
    else return false;
  }

  bool isNetworkConnected0() {
	String resp="";
	uint8_t ind = 0;
	DBG("TINYGSM: SARAR5");
    delay(3000);
	
	waitResponse();
    RegStatus s = getRegistrationStatus();
	waitResponse();
    if (s == REG_OK_HOME || s == REG_OK_ROAMING)
	{
		  sendAT(GF("+CGATT?"));  //NUEVO VERSION UMNOPROF
		  if(waitResponse(10000L,GF("+CGATT: 1"))==1){
				DBG("TINYGSM: MODEM CONECTADO");
				waitResponse();
				   
				sendAT(GF("+CGACT=1,1"));  
				if (waitResponse(150000L) != 1) {
					DBG("TINY GSM: ER0");
					return false;
				}
				
				delay(2000);
				sendAT(GF("+UPSDA=0,3")); 
				if (waitResponse(150000L) != 1) {
					delay(2000);
					sendAT(GF("+UPSDA=0,3")); 
					if (waitResponse(150000L) != 1) {
						DBG("TINY GSM: ER1");
						return false;
					}
				}
			
				ind = waitResponse(150000L,resp,GF("ERROR"),GF("+UUPSDA: 0,"));
				if(ind != 2){
					DBG("TINY GSM: ER2");
					waitResponse();
					return false;
				}
				resp = stream.readStringUntil('\n');
				if (resp.length() < 11) //antes 14  toma valor de IP
				{
					DBG("TINY GSM: ER3");
					return false;
				}
			return true;
			}
			else 
				return false;
	}
    else if (s == REG_UNKNOWN)  // for some reason, it can hang at unknown..
      return isGprsConnected();
    else return false;
  }

TINY_GSM_MODEM_WAIT_FOR_NETWORK()

  bool setURAT( uint8_t urat ) {
    // AT+URAT=<SelectedAcT>[,<PreferredAct>[,<2ndPreferredAct>]]

    sendAT(GF("+COPS=2"));        // Deregister from network
    if (waitResponse() != 1) {
      return false;
    }
    sendAT(GF("+URAT="), urat);  // Radio Access Technology (RAT) selection
    if (waitResponse() != 1) {
      return false;
    }
    sendAT(GF("+COPS=0"));        // Auto-register to the network
    if (waitResponse() != 1) {
      return false;
    }
    return restart();
  }

  /*
   * GPRS functions
   */
   
   bool gprsConnect(const char* apn, const char* user = NULL, const char* pwd = NULL) {
	String resp="";
	//uint8_t ind=0;
	 

    if (gprsDisconnect()!=true){
		DBG("TINYGSM: ERROR AL DESCONECTAR EL MODULO GPRS");
	}
	else{
		sendAT(GF("+CFUN=0"));   //NUEVA VERSION GPRSCON
		if (waitResponse(10000L) != 1) { 
			DBG("TINY GSM: ERR_0");
			return false;
		}
	}
	 delay(6000);  
	 delay(6000);  
	sendAT(GF("+UMNOPROF=100"));   //NUEVA VERSION GPRSCON
    if (waitResponse(10000L) != 1) { 
	  DBG("TINY GSM: ERR_1");
      return false;
    }
	delay(2000);
	
	sendAT(GF("+CFUN=16"));   //NUEVA VERSION GPRSCON
    if (waitResponse(10000L) != 1) { 
	  DBG("TINY GSM: ERR_2");
      return false;
    }
	delay(7000);
	
	sendAT(GF("+CGATT=0"));   //NUEVA VERSION GPRSCON
	if(waitResponse()!=1){
		if (waitResponse(210000L) != 1) { 
		  DBG("TINY GSM: ERR_3");
		  return false;
		}
    }
	
	waitResponse();
	waitResponse();
	waitResponse();
	sendAT(GF("+CFUN?"));   
    if (waitResponse(10000L, GF(GSM_NL "+CFUN: 1")) != 1) {
    
	sendAT(GF("+CFUN=0"));   //NUEVA VERSION GPRSCON
    if (waitResponse(10000L) != 1) { 
	  DBG("TINY GSM: ERR_4");
      return false;
    }
	delay(6000);
	}
	sendAT(GF("+CGATT=0"));   //NUEVA VERSION GPRSCON
    if (waitResponse(210000L) != 1) { 
	  DBG("TINY GSM: ERR_5");
      return false;
    }
	delay(6000);
	waitResponse();
	
	sendAT(GF("+UAUTHREQ=1,3,\""), user, GF("\",\""), pwd, '"');
    if (waitResponse() != 1) { 
	  DBG("TINY GSM: ERR1");
      return false;
    }
	 
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');  // Define PDP context 1
    if(waitResponse()!=1){
		DBG("TINY GSM: ERR2");
		return false;
	}
	
	sendAT(GF("+UPSD=0,0,0"));  //AÑADIDO 03-02-23
    if(waitResponse()!=1){
		DBG("TINY GSM: ERR3.0");
		return false;
	}
	
	sendAT(GF("+UPSD=0,1,\""), apn, '"');  // Define PDP context 1 #MODIF 03-02-23 antes 1,1,"apn"
    if(waitResponse()!=1){
		DBG("TINY GSM: ERR3");
		return false;
	}
	
		
	sendAT(GF("+UPSD=0,100,1"));  
    if(waitResponse()!=1){
		DBG("TINY GSM: ERR4");
		return false;
	}
	//sendAT(GF("+UPSD=1,100,1"));   #COMENTADO 03-02-23
    //if(waitResponse()!=1){
	//	DBG("TINY GSM: ERR4.2");
	//	return false;
	//}
		
	sendAT(GF("+CFUN=1"));  
    if(waitResponse(60000L)!=1){
		DBG("TINY GSM: ERR5");
		return false;
	} 
	delay(6000);
	////////////////////////MODIFICADO 06/07///////////////////////
		// String resp="";
	// uint8_t ind=0;
	
	// waitResponse();
	
	// sendAT(GF("+UGPS?"));
	// waitResponse(30000L,resp,GF("+UGPS: "));
	// resp = stream.readStringUntil('\n');
	// DBG(resp);
	
	// waitResponse();
	
	// if (resp.indexOf(String(mode)+","+String(aidMode)+","+String(GSNSSSystem))==-1)
	// {
		// sendAT(GF("+UGPS="),mode,',',aidMode,',',GSNSSSystem);  //SI SE CONFIGURA EL AID HAY QUE AÑADIR UGIND (ver mas comandos atcommands)
		// ind = waitResponse(30000L,resp,GF("OK"),GF("+CME ERROR:"));
	
		// if ((ind != 1) && (ind !=2)) {
			// waitResponse();
			// resp = stream.readStringUntil('\n');
		    // return false; //ERROR 2: UGPS NO RESPONDE OK
		// }
	
		// if(ind == 2){
			// resp = stream.readStringUntil('\n');
			// if (resp.indexOf("GPS aiding mode already set")==-1){
				// return false; //ERROR 3: CME ERROR
			// }
		// }
	// }

	 // sendAT(GF("+UGGGA="),uggga);  //HABILITA MENSAJES NMEA
	 // if (waitResponse()!=1){
		 // return false; //ERROR 4: UGGGA NO RESPONDE OK
	 // }
	 // sendAT(GF("+UGGLL="),uggll); //HABILITA MANTENER EL ULTIMO MENSAJE NMEA, (NMEA es volatil)
	 // if(waitResponse()!=1){
		 // return false; //ERROR 5: UGGLL NO RESPONDE OK
	 // }
	 
    // return true;
	
	enableGPS();
	delay(2000);
	///////////////////////////////////////////////////////////////
	
	sendAT(GF("+CGATT?"));  
	if(waitResponse(10000L,GF("+CGATT: 0"))==1){
		DBG("TINYGSM: MODEM DESCONECTADO, CONECTANDO");
		waitResponse();
		delay(3000);
		sendAT(GF("+CGATT=1"));  // attach to GPRS
		if (waitResponse() != 1) {   
			if(waitResponse() != 1){
				if (waitResponse(210000L) != 1) { //original:360000L  
					DBG("TINY GSM: ERR6");
					waitResponse();
					return false;
				}
			}
		}
		waitResponse();
	delay(6000);
	}

    sendAT(GF("+CGACT=1,1"));  // activate PDP profile/context 1
    if (waitResponse(150000L) != 1) {
		DBG("TINY GSM: ERR7");
      return false;
    }
	
	// sendAT(GF("+UPSDA=0,3")); 
    // if (waitResponse(150000L) != 1) {
		// DBG("TINY GSM: ERR8");
      // return false;
    // }
	
	// ind = waitResponse(150000L,resp,GF("ERROR"),GF("+UUPSDA: 0,"));
	// if(ind != 2){
		// DBG("TINY GSM: ERR9");
		// waitResponse();
		// return false;
	// }
	// resp = stream.readStringUntil('\n');
	// if (resp.length() < 14)
	// {
		// DBG("TINY GSM: ERR10");
		// return false;
	// }
	
	sendAT(GF("+UPSDA=1,1")); 
    if (waitResponse(150000L) != 1) {
		DBG("TINY GSM: ERR11.2");
      //return false;
    }
		sendAT(GF("+UPSDA=0,1")); 
    if (waitResponse(150000L) != 1) {
		DBG("TINY GSM: ERR11.2");
      //return false;
    }
	
    return true;
  }
   
 //NUEVA VERSION GPRSCON CON UMNOPROF
 // bool gprsConnect1(const char* apn, const char* user = NULL, const char* pwd = NULL) {
	// String resp="";
	// uint8_t ind=0;
	 

    // if (gprsDisconnect()!=true){
		// DBG("TINYGSM: ERROR AL DESCONECTAR EL MODULO GPRS");
	// }
	 // delay(6000);  
	 
	// sendAT(GF("+CFUN=0"));   //NUEVA VERSION GPRSCON
    // if (waitResponse(10000L) != 1) { 
	  // DBG("TINY GSM: ERR_0");
      // return false;
    // }
	 
	// sendAT(GF("+UMNOPROF=100"));   //NUEVA VERSION GPRSCON
    // if (waitResponse(10000L) != 1) { 
	  // DBG("TINY GSM: ERR_1");
      // return false;
    // }
	
	// sendAT(GF("+CFUN=16"));   //NUEVA VERSION GPRSCON
    // if (waitResponse(10000L) != 1) { 
	  // DBG("TINY GSM: ERR_2");
      // return false;
    // }
	// delay(6000);
	// sendAT(GF("+CGATT=0"));   //NUEVA VERSION GPRSCON
    // if (waitResponse(210000L) != 1) { 
	  // DBG("TINY GSM: ERR_3");
      // return false;
    // }
	// delay(6000);
	// waitResponse();
	// sendAT(GF("+CFUN=0"));   //NUEVA VERSION GPRSCON
    // if (waitResponse(10000L) != 1) { 
	  // DBG("TINY GSM: ERR_4");
      // return false;
    // }
	
	// sendAT(GF("+CGATT=0"));   //NUEVA VERSION GPRSCON
    // if (waitResponse(210000L) != 1) { 
	  // DBG("TINY GSM: ERR_5");
      // return false;
    // }
	
	
	//////sendAT(GF("+USECPRF=0,0,0"));  
    //////if (waitResponse() != 1) { 
	//////  DBG("TINY GSM: ERR0");
    //////  return false;
    //////}
	
	// sendAT(GF("+UAUTHREQ=1,3,\""), user, GF("\",\""), pwd, '"');
    // if (waitResponse() != 1) { 
	  // DBG("TINY GSM: ERR1");
      // return false;
    // }
	 
    // sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');  // Define PDP context 1
    // if(waitResponse()!=1){
		// DBG("TINY GSM: ERR2");
		// return false;
	// }
	

	
	// sendAT(GF("+UPSD=1,1,\""), apn, '"');  // Define PDP context 1
    // if(waitResponse()!=1){
		// DBG("TINY GSM: ERR3");
		// return false;
	// }
	
		
	// sendAT(GF("+UPSD=0,100,1"));  
    // if(waitResponse()!=1){
		// DBG("TINY GSM: ERR4");
		// return false;
	// }
		
	// sendAT(GF("+CFUN=1"));  
    // if(waitResponse(60000L)!=1){
		// DBG("TINY GSM: ERR5");
		// return false;
	// }
	
	// sendAT(GF("+CGATT=1"));  // attach to GPRS
    // if (waitResponse(210000L) != 1) { //original:360000L  
	  // DBG("TINY GSM: ERR6");
	// waitResponse();
      // return false;
    // }

    // sendAT(GF("+CGACT=1,1"));  // activate PDP profile/context 1
    // if (waitResponse(150000L) != 1) {
		// DBG("TINY GSM: ERR7");
      // return false;
    // }
	
	// sendAT(GF("+UPSDA=0,3")); 
    // if (waitResponse(150000L) != 1) {
		// DBG("TINY GSM: ERR8");
      // return false;
    // }
	
	// ind = waitResponse(150000L,resp,GF("ERROR"),GF("+UUPSDA: 0,"));
	// if(ind != 2){
		// DBG("TINY GSM: ERR9");
		// waitResponse();
		// return false;
	// }
	// resp = stream.readStringUntil('\n');
	// if (resp.length() < 14)
	// {
		// DBG("TINY GSM: ERR10");
		// return false;
	// }
	
	// sendAT(GF("+UPSDA=0,1")); 
    // if (waitResponse(150000L) != 1) {
		// DBG("TINY GSM: ERR11");
      // return false;
    // }
	
    // return true;
  // }
  

  
  

 // bool gprsConnect0(const char* apn, const char* user = NULL, const char* pwd = NULL) {
 //   if (gprsDisconnect()!=true)
//		DBG("TINYGSM: ERROR AL DESCONECTAR EL MODULO GPRS");
	
  //  delay(6000); //antes 6000
	
    ////// sendAT(GF("+CGATT=1"));  // attach to GPRS
    ////// if (waitResponse(210000L) != 1) { //360000L
      ////// return false;
   ////// }

    ////// Using CGDCONT sets up an "external" PCP context, i.e. a data connection
    //////using the external IP stack (e.g. Windows dial up) and PPP link over the
    ////// serial interface.  This is the only command set supported by the LTE-M
    ////// and LTE NB-IoT modules (SARA-R5xx, SARA-N4xx)

     // sendAT(GF("+UAUTHREQ=1,1,\""), user, GF("\",\""), pwd, '"');
     // waitResponse();	  
	 ////// sendAT(GF("+CGAUTH=1,0,\""), user, GF("\",\""), pwd, '"');  //ORIGINAL DESCOMENTADO
     ////// waitResponse();
    

  //  sendAT(GF("+CGDCONT=1,\"IPV4V6\",\""), apn, '"');  // Define PDP context 1
  //  waitResponse();
	
   //sendAT(GF("+CGATT=1"));  // attach to GPRS
   // if (waitResponse(210000L) != 1) { //original:360000L  
   //   return false;
   // }
	


   // sendAT(GF("+CGACT=1,1"));  // activate PDP profile/context 1
   // if (waitResponse(150000L) != 1) {
   //   return false;
   // }

   // return true;
  //}
 
 bool gprsDisconnect() {
	  
  String resp="";
  
  sendAT(GF("+CGATT?"));  
	if(waitResponse(10000L,GF("+CGATT: 0"))!=1){
	DBG("TINYGSM: MODEM CONECTADO, DESCONECTANDO");
	waitResponse();
	
  
   delay(5000); //añadido
 
	sendAT(GF("+CGATT=0"));
	int res1 = waitResponse(60000L,resp,GF("OK"), GF("+CME")); 
		if (res1 != 1) {
			waitResponse();
			if(res1==2){
				DBG("TINYGSM: ERROR, REINTENTANDO");
				sendAT(GF("+CGATT=0"));  // detach from GPRS
                if (waitResponse(60000L) != 1)  //ORIGINAL:360000L
                return false;
			}
			else
			return false;
		}
		else
			DBG("SaraR5 desconectado del servicio GPRS");
		
		sendAT(GF("+CFUN=0"));
		if(waitResponse(60000L)!=1){
		return false;
		}
 
	 delay(5000); //antes 5000
	 }
	 else{
		 DBG("TINYGSM: CLIENTE DESCONECTADO");
	 }
    return true;
  }

  //bool gprsDisconnect0() {
	  
 ////// sendAT(GF("+CGACT=1,0"));  // Deactivate PDP context 1
 ////// if (waitResponse(40000L) != 1) {
 //////   return false;
 ////// }
    ////// sendAT(GF("+CGATT=0"));  // detach from GPRS
    ////// if (waitResponse(200000L) != 1) { //ORIGINAL:360000L
      ////// return false;
    ////// }
  // String resp="";
  
 // delay(3000); //añadido
 
	// sendAT(GF("+CGATT=0"));
	
	// int res1 = waitResponse(60000L,resp,GF("OK"), GF("+CME")); 
		// if (res1 != 1) {
			// waitResponse();
			// if(res1==2){
				// DBG("TINYGSM: ERROR, REINTENTANDO");
				// sendAT(GF("+CGATT=0"));  // detach from GPRS
                // if (waitResponse(60000L) != 1)  //ORIGINAL:360000L
                // return false;
			// }
			// else
			// return false;
		// }
		// else
			// DBG("SaraR5 desconectado del servicio GPRS");
	
 
	 // delay(6000); //antes 5000


    // return true;
  // }

TINY_GSM_MODEM_GET_GPRS_IP_CONNECTED()

  /*
   * IP Address functions
   */

  String getLocalIP() {
    sendAT(GF("+CGPADDR"));
    if (waitResponse(GF(GSM_NL "+CGPADDR:")) != 1) {
      return "";
    }
    streamSkipUntil(',');  // Skip context id
    String res = stream.readStringUntil('\r');
    if (waitResponse() != 1) {
      return "";
    }
    return res;
  }

  IPAddress localIP() {
    return TinyGsmIpFromString(getLocalIP());
  }

  /*
   * Phone Call functions
   */

  bool setGsmBusy(bool busy = true) TINY_GSM_ATTR_NOT_AVAILABLE;

  bool callAnswer() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool callNumber(const String& number) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool callHangup() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Messaging functions
   */

  String sendUSSD(const String& code) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool sendSMS(const String& number, const String& text) {
    sendAT(GF("+CSCS=\"GSM\""));  // Set GSM default alphabet
    waitResponse();
    sendAT(GF("+CMGF=1"));  // Set preferred message format to text mode
    waitResponse();
    sendAT(GF("+CMGS=\""), number, GF("\""));  // set the phone number
    if (waitResponse(GF(">")) != 1) {
      return false;
    }
    stream.print(text);  // Actually send the message
    stream.write((char)0x1A);
    stream.flush();
    return waitResponse(60000L) == 1;
  }

  bool sendSMS_UTF16(const String& number, const void* text, size_t len) TINY_GSM_ATTR_NOT_IMPLEMENTED;


  /*
   * Location functions
   */

  String getGsmLocation() {
    sendAT(GF("+ULOC=2,3,0,120,1"));
    if (waitResponse(30000L, GF(GSM_NL "+UULOC:")) != 1) {
      return "";
    }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Battery & temperature functions
   */

  uint16_t getBattVoltage() TINY_GSM_ATTR_NOT_AVAILABLE;

  int8_t getBattPercent() {
    sendAT(GF("+CIND?"));
    if (waitResponse(GF(GSM_NL "+CIND:")) != 1) {
      return 0;
    }

    int8_t res = stream.readStringUntil(',').toInt();
    int8_t percent = res*20;  // return is 0-5
    // Wait for final OK
    waitResponse();
    return percent;
  }

  uint8_t getBattChargeState() TINY_GSM_ATTR_NOT_AVAILABLE;

  /*bool getBattStats(uint8_t &chargeState, int8_t &percent, uint16_t &milliVolts) {
    percent = getBattPercent();
    return true;
  }*/

  float getTemperature() {
    // First make sure the temperature is set to be in celsius
    sendAT(GF("+UTEMP=0"));  // Would use 1 for Fahrenheit
    if (waitResponse() != 1) {
      return (float)-9999;
    }
    sendAT(GF("+UTEMP?"));
    if (waitResponse(GF(GSM_NL "+UTEMP:")) != 1) {
      return (float)-9999;
    }
    streamSkipUntil(','); // Skip units (C/F)
    int res = stream.readStringUntil('\n').toInt();
    float temp = -9999;
    if (res != 655355) {
      temp = ((float)res)/10;
    }
    return temp;
  }

  /*
   * Client related functions
   */

protected:

 // bool modemConnect_v046b(const char* host, uint16_t port, uint8_t* mux,
                    // bool ssl = false, int timeout_s = 120)
  // {
	  
	// String resp0="";
	// uint8_t ind0=0;
	// int rsp=0;
	
	  // delay(500);
    // uint32_t timeout_ms = ((uint32_t)timeout_s)*1000;
	
	// sendAT(GF("+UPSDA=1,4")); 
	// waitResponse(150000L);
	// waitResponse();
	// delay(3000);
	// sendAT(GF("+UPSDA=1,3")); 
	// if (waitResponse(150000L) != 1) {
		// waitResponse();
		// DBG("TINY GSM: ER1");
		// return false;
	// }
	
	////// bool b_upsda=true;
	
	////// sendAT(GF("+UPSDA=1,3")); 
	////// if (waitResponse(150000L) != 1) {
		////// delay(2000);
		////// sendAT(GF("+UPSDA=1,3")); 
		////// if (waitResponse(150000L) != 1) {
			////// DBG("TINY GSM: ER1");
			////////////return false;
			////// b_upsda=false;
		////// }
	////// }

   ////// if(b_upsda)
   ////// {
		////// String resp0="";
		////// uint8_t ind0=0;
		
		////// ind0 = waitResponse(150000L,resp0,GF("ERROR"),GF("+UUPSDA: 0,"));
		////// if(ind0 != 2){
			////// DBG("TINY GSM: ER2");
			////// waitResponse();
			////////////return false;
		////// }
		////// resp0 = stream.readStringUntil('\n');
		////// if (resp0.length() < 14)
		////// {
			////// DBG("TINY GSM: ER3");
			////////////return false;
		////// }
   ////// }
	
		
	// ind0 = waitResponse(150000L,resp0,GF("ERROR"),GF("+UUPSDA: 0,"));
	// if(ind0 != 2){
		// DBG("TINY GSM: ER2");
		// waitResponse();
		// return false;
	// }
	// resp0 = stream.readStringUntil('\n');
	// if (resp0.length() < 10) //ANTES 14
	// {
		// DBG("TINY GSM: ER3");
		// return false;
	// }
   
	
	////// sendAT(GF("+UPSDA=0,1")); 
    ////// if (waitResponse(150000L) != 1) {
		////// DBG("TINY GSM: ERR11");
      ////// return false;
    ////// }
	// delay(1000);
	// sendAT(GF("+USECPRF=0,0,0"));  //NUEVA VERSION UMNOPROF
    // if (waitResponse() != 1) { 
	  // DBG("TINY GSM: ERR0");
      // return false;
    // }
	// delay(500);
    // sendAT(GF("+USOCR=6"));  // create a socket
    // if (waitResponse(GF(GSM_NL "+USOCR:")) != 1) {  // reply is +USOCR: ## of socket created
      // return false;
    // }
    // *mux = stream.readStringUntil('\n').toInt();
    // waitResponse();
	// delay(500);

    // if (ssl) {
      // sendAT(GF("+USOSEC="), *mux, ",1,0");  //ORIGINAL: sendAT(GF("+USOSEC="), *mux, ",1"); // sendAT(GF("+USOSEC="), *mux, ",1,0");
      // waitResponse();
    // }
	// delay(500);
	
    ////// Enable NOdelay
    // sendAT(GF("+USOSO="), *mux, GF(",6,1,1"));
    // waitResponse();

    ////// Enable KEEPALIVE, 30 sec
    //////sendAT(GF("+USOSO="), *mux, GF(",6,2,30000"));
    //////waitResponse();
	
	//////AÑADIDO DE MKRNB   (TLS)
  //////  sendAT(GF("+USECPRF="), *mux, GF(",0,1"));  
  //////  waitResponse();


    ////// connect on the allocated socket
	// delay(1000);
    // sendAT(GF("+USOCO="), *mux, ",\"", host, "\",", port); 
	
    // rsp = waitResponse(timeout_ms);
	// DBG("Valor devuelto de USOCO: ",rsp);
    // return (1 == rsp);
  // }




 bool modemConnect(const char* host, uint16_t port, uint8_t* mux,          //modemConnect_v046
                    bool ssl = false, int timeout_s = 120)
  {
	  delay(500);
    uint32_t timeout_ms = ((uint32_t)timeout_s)*1000;
	
	bool b_upsda=true;
	
	sendAT(GF("+UPSDA=0,3")); //ANTES 0,3 #MODIF 03-02-23 de 1,3 a 0,3
	if (waitResponse(150000L) != 1) {  
		delay(2000);
		sendAT(GF("+UPSDA=0,3")); //ANTES 0,3 #MODIF 03-02-23 de 1,3 a 0,3
		if (waitResponse(150000L) != 1) {
			DBG("TINY GSM: ER1");
			//return false;
			b_upsda=false;
		}
	}

   if(b_upsda)
   {
		String resp0="";
		uint8_t ind0=0;
		
		ind0 = waitResponse(150000L,resp0,GF("ERROR"),GF("+UUPSDA: 0,"));
		if(ind0 != 2){
			DBG("TINY GSM: ER2");
			waitResponse();
			//return false;
		}
		resp0 = stream.readStringUntil('\n');
		if (resp0.length() < 14)
		{
			DBG("TINY GSM: ER3");
			//return false;
		}
   }
	
	// sendAT(GF("+UPSDA=0,1")); 
    // if (waitResponse(150000L) != 1) {
		// DBG("TINY GSM: ERR11");
      // return false;
    // }
	delay(100);
	sendAT(GF("+USECPRF=0,0,0"));  //NUEVA VERSION UMNOPROF
    if (waitResponse() != 1) { 
	  DBG("TINY GSM: ERR0");
      return false;
    }
	
	delay(100);
    sendAT(GF("+USOCR=6"));  // create a socket
    if (waitResponse(GF(GSM_NL "+USOCR:")) != 1) {  // reply is +USOCR: ## of socket created
      return false;
    }
	
    *mux = stream.readStringUntil('\n').toInt();
    waitResponse();
    delay(100);
    if (ssl) {
      sendAT(GF("+USOSEC="), *mux, ",1,0");  //ORIGINAL: sendAT(GF("+USOSEC="), *mux, ",1"); // sendAT(GF("+USOSEC="), *mux, ",1,0");
      waitResponse();
    }
	delay(100);
	
    // Enable NOdelay
    sendAT(GF("+USOSO="), *mux, GF(",6,1,1"));
    waitResponse();

    // Enable KEEPALIVE, 30 sec
    //sendAT(GF("+USOSO="), *mux, GF(",6,2,30000"));
    //waitResponse();
	
	//AÑADIDO DE MKRNB   (TLS)
  //  sendAT(GF("+USECPRF="), *mux, GF(",0,1"));  
  //  waitResponse();


    // connect on the allocated socket
	delay(1000);
	waitResponse();
    sendAT(GF("+USOCO="), *mux, ",\"", host, "\",", port); 
	
    int rsp = waitResponse(timeout_ms);
	DBG("Valor devuelto de USOCO: ",rsp);
    return (1 == rsp);
  }

  // bool modemConnect0(const char* host, uint16_t port, uint8_t* mux,
                    // bool ssl = false, int timeout_s = 120)
  // {
	  // delay(500);
    // uint32_t timeout_ms = ((uint32_t)timeout_s)*1000;
	
	// sendAT(GF("+USECPRF=0,0,0"));  //NUEVA VERSION UMNOPROF
    // if (waitResponse() != 1) { 
	  // DBG("TINY GSM: ERR0");
      // return false;
    // }
	
    // sendAT(GF("+USOCR=6"));  // create a socket
    // if (waitResponse(GF(GSM_NL "+USOCR:")) != 1) {  // reply is +USOCR: ## of socket created
      // return false;
    // }
    // *mux = stream.readStringUntil('\n').toInt();
    // waitResponse();

    // if (ssl) {
      // sendAT(GF("+USOSEC="), *mux, ",1,0");  //ORIGINAL: sendAT(GF("+USOSEC="), *mux, ",1"); // sendAT(GF("+USOSEC="), *mux, ",1,0");
      // waitResponse();
    // }
	
	
    ////// Enable NOdelay
    // sendAT(GF("+USOSO="), *mux, GF(",6,1,1"));
    // waitResponse();

    ////// Enable KEEPALIVE, 30 sec
    //////sendAT(GF("+USOSO="), *mux, GF(",6,2,30000"));
    //////waitResponse();
	
	//////AÑADIDO DE MKRNB   (TLS)
 //////  sendAT(GF("+USECPRF="), *mux, GF(",0,1"));  
  //////  waitResponse();


    // connect on the allocated socket
    // sendAT(GF("+USOCO="), *mux, ",\"", host, "\",", port); 
    // int rsp = waitResponse(timeout_ms);
	// DBG("Valor devuelto de USOCO: ",rsp);
    // return (1 == rsp);
  // }

  bool modemDisconnect(uint8_t mux) {
    TINY_GSM_YIELD();
    if (!modemGetConnected(mux)) {
      sockets[mux]->sock_connected = false;
      return true;
    }
    sendAT(GF("+USOCL="), mux);
    return 1 == waitResponse(120000L);  // can take up to 120s to get a response
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+USOWR="), mux, ',', len);
	delay(150);
    if (waitResponse(GF("@")) != 1) {
      return 0;
    }
    // 50ms delay, see AT manual section 25.10.4
    delay(150); //ORIGINAL:50
    stream.write((uint8_t*)buff, len);
    stream.flush();
    if (waitResponse(GF(GSM_NL "+USOWR:")) != 1) {
      return 0;
    }
    streamSkipUntil(','); // Skip mux
    int sent = stream.readStringUntil('\n').toInt();
    waitResponse();  // sends back OK after the confirmation of number sent
    return sent;
  }

  size_t modemRead(size_t size, uint8_t mux) {
	  delay(100);
    DBG("### FLAG0 mux:",mux," size: ", size);
    sendAT(GF("+USORD="), mux, ',', size);
    if (waitResponse(GF(GSM_NL "+USORD:")) != 1) {
      return 0;
    }
    streamSkipUntil(','); // Skip mux
    size_t len = stream.readStringUntil(',').toInt();
    streamSkipUntil('\"');

    for (size_t i=0; i<len; i++) {
      TINY_GSM_MODEM_STREAM_TO_MUX_FIFO_WITH_DOUBLE_TIMEOUT
    }
    streamSkipUntil('\"');
    waitResponse();
    DBG("### READ:", len, "from", mux);
    sockets[mux]->sock_available = modemGetAvailable(mux);
    return len;
  }
  

  size_t modemGetAvailable(uint8_t mux) {
	  delay(100);
    // NOTE:  Querying a closed socket gives an error "operation not allowed"
    sendAT(GF("+USORD="), mux, ",0");
    size_t result = 0;
    uint8_t res = waitResponse(GF(GSM_NL "+USORD:"));
    // Will give error "operation not allowed" when attempting to read a socket
    // that you have already told to close
    if (res == 1) {
      streamSkipUntil(','); // Skip mux
      result = stream.readStringUntil('\n').toInt();
      // if (result) DBG("### DATA AVAILABLE:", result, "on", mux);
      waitResponse();
    } else if (res == 3) {
      streamSkipUntil('\n'); // Skip the error text
    }
    if (!result) {
      sockets[mux]->sock_connected = modemGetConnected(mux);
    }
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    // NOTE:  Querying a closed socket gives an error "operation not allowed"
	String resp="";
	delay(100);
    sendAT(GF("+USOCTL="), mux, ",10");
    uint8_t ind = waitResponse(10000L,resp,GF(GSM_NL "+USOCTL:"),GF("+CME "));
	

    if (ind != 1){
		if(ind ==2){
			resp = stream.readStringUntil('\n');
			if(resp.indexOf("operation not allowed")==-1){//antes sin -1
				DBG("TINYGSM: the socket: "+String(mux)+" is closed");
			}
		}
      return false;
	}

    streamSkipUntil(','); // Skip mux
    streamSkipUntil(','); // Skip type
    int result = stream.readStringUntil('\n').toInt();
    // 0: the socket is in INACTIVE status (it corresponds to CLOSED status
    // defined in RFC793 "TCP Protocol Specification" [112])
    // 1: the socket is in LISTEN status
    // 2: the socket is in SYN_SENT status
    // 3: the socket is in SYN_RCVD status
    // 4: the socket is in ESTABILISHED status
    // 5: the socket is in FIN_WAIT_1 status
    // 6: the socket is in FIN_WAIT_2 status
    // 7: the sokcet is in CLOSE_WAIT status
    // 8: the socket is in CLOSING status
    // 9: the socket is in LAST_ACK status
    // 10: the socket is in TIME_WAIT status
    waitResponse();
    return (result != 0);
  }

public:

  /*
   Utilities
   */

TINY_GSM_MODEM_STREAM_UTILITIES()

  // TODO: Optimize this!
  uint8_t waitResponse(uint32_t timeout_ms, String& data,
                       GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=GFP(GSM_CME_ERROR), GsmConstStr r4=NULL, GsmConstStr r5=NULL)
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
        if (a <= 0) continue; // Skip 0x00 bytes, just in case
        data += (char)a;
        if (r1 && data.endsWith(r1)) {
          index = 1;
          goto finish;
        } else if (r2 && data.endsWith(r2)) {
			// while(stream.available()>0){
				// a = stream.read();
				// data+=char(b);
			// }
			// DBG(data);
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
        } else if (data.endsWith(GF(GSM_NL "+UUSORD:"))) {
          int mux = stream.readStringUntil(',').toInt();
          int len = stream.readStringUntil('\n').toInt();
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->got_data = true;
            sockets[mux]->sock_available = len;
          }
          data = "";
          DBG("### URC Data Received:", len, "on", mux);
        } else if (data.endsWith(GF(GSM_NL "+UUSOCL:"))) {
          int mux = stream.readStringUntil('\n').toInt();
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->sock_connected = false;
          }
          data = "";
          DBG("### URC Sock Closed:", mux);
        }
      }
    } while (millis() - startMillis < timeout_ms);
finish:
    if (!index) {
     // data.trim();
      if (data.length()) {
        DBG("### Unhandled:", data);
      }
      data = "";
    }
      DBG("TINY GSM:",'<', index, '>',data);
    return index;
  }

  uint8_t waitResponse(uint32_t timeout_ms,
                       GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=GFP(GSM_CME_ERROR), GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5);
  }

  uint8_t waitResponse(GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=GFP(GSM_CME_ERROR), GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    return waitResponse(1000, r1, r2, r3, r4, r5);
  }

public:
  Stream&       stream;

protected:
  GsmClient*    sockets[TINY_GSM_MUX_COUNT];
};

#endif
