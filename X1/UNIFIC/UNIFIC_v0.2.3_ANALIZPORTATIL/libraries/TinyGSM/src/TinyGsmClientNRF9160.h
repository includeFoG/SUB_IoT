
#ifndef TinyGsmClientBC95G_h
#define TinyGsmClientBC95G_h
//#pragma message("TinyGSM:  TinyGsmClientBC95G")

#define TINY_GSM_DEBUG Serial

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 64
#endif

#define TINY_GSM_MUX_COUNT 7

#include <TinyGsmCommon.h>

#define GSM_NL "\r\n"  //!!
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


class TinyGsmBC95G
{

public:

class GsmClient : public Client
{
  friend class TinyGsmBC95G;
  typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

public:
  GsmClient() {}

  GsmClient(TinyGsmBC95G& modem, uint8_t mux = 0) {
    init(&modem, mux);
  }

  bool init(TinyGsmBC95G* modem, uint8_t mux = 0) {
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
        DBG("WARNING:  Mux number changed from", oldMux, "to", mux);
        at->sockets[oldMux] = NULL;
    }
    at->sockets[mux] = this;
    at->maintain();

    return sock_connected;
  }

TINY_GSM_CLIENT_CONNECT_OVERLOADS()

  virtual void stop() {
    TINY_GSM_YIELD();
    // Read and dump anything remaining in the modem's internal buffer.
    // The socket will appear open in response to connected() even after it
    // closes until all data is read from the buffer.
    // Doing it this way allows the external mcu to find and get all of the data
    // that it wants from the socket even if it was closed externally.
    rx.clear();
    at->maintain();
    while (sock_connected && sock_available > 0) {
      at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
      rx.clear();
      at->maintain();
    }
    at->modemDisconnect(mux);
  }

TINY_GSM_CLIENT_WRITE()

TINY_GSM_CLIENT_AVAILABLE_WITH_BUFFER_CHECK()

TINY_GSM_CLIENT_READ_WITH_BUFFER_CHECK()

TINY_GSM_CLIENT_PEEK_FLUSH_CONNECTED()

  /*
   * Extended API
   */

  String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;

private:
  TinyGsmBC95G*   at;
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

  GsmClientSecure(TinyGsmBC95G& modem, uint8_t mux = 1)
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
        DBG("WARNING:  Mux number changed from", oldMux, "to", mux);
        at->sockets[oldMux] = NULL;
    }
    at->sockets[mux] = this;
    at->maintain();
    return sock_connected;
  }
};


public:

  TinyGsmBC95G(Stream& stream)
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
    sendAT(GF("E0"));   // Echo Off
    if (waitResponse() != 1) {
      return false;
    }

#ifdef TINY_GSM_DEBUG   ///////////////////////////////////////////////////////////////BC95G
    sendAT(GF("+CMEE=1"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();


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
   * Power functions //////////////////////////////////////////////////////////////
   */
   
    bool EN_BC95() {
    if (!testAT()) {
      return false;
    }
    sendAT(GF("+CFUN=1"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    delay(3000);  // TODO:  Verify delay timing here
    return true;
  }
  
    bool DIS_BC95() {
    if (!testAT()) {
      return false;
    }
    sendAT(GF("+CFUN=0"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    delay(3000);  // TODO:  Verify delay timing here
    return true;
  }
  
      bool CONFIG_BC95() {
    if (!testAT()) {
      return false;
    }
    sendAT(GF("+NCONFIG=CR_0354_0338_SCRAMBLING,TRUE"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    delay(3000);  // TODO:  Verify delay timing here
	sendAT(GF("+NCONFIG=CR_0859_SI_AVOID,TRUE"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    delay(3000);  // TODO:  Verify delay timing here
	sendAT(GF("+NCONFIG=AUTOCONNECT,FALSE"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    delay(3000);  // TODO:  Verify delay timing here
    return true;
  }
  
  
  
  
//////////////////////////////////////////////////////////////////////////FUNCIONES PARA BC95G////////////////////////////////////////////////////////////////////////
  bool restart() {
	  delay(10000); //delay para dar tiempo al módulo a que envíe la configuración
    if (!testAT()) {
      return false;
    }
    sendAT(GF("+CFUN=0"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    delay(3000);  // TODO:  Verify delay timing here
	
	   sendAT(GF("+CFUN=1"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    delay(3000);  // TODO:  Verify delay timing here
    return init();
  }

  bool poweroff() {
    sendAT(GF("+CPWROFF"));
    return waitResponse(40000L) == 1;
  }

  bool radioOff() {
    sendAT(GF("+CFUN=0"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    delay(3000);
    return true;
  }

  bool sleepEnable(bool enable = true) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  
  
  
   
    String _TakeTime(){  //MODIF!!!   "+CCLK:21/10/13,10:14:37+04"  -> 2021/10/13 10:14:37
	 String _date="";
	 String _day="";
	 String _month="";
	 int _year=0;
	 String _Time="";
	 String resp="";
	
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
   
     // String getGsmLocation() {
    // sendAT(GF("+ULOC=2,3,0,120,1"));
    // if (waitResponse(30000L, GF(GSM_NL "+UULOC:")) != 1) {
      // return "";
    // }
    // String res = stream.readStringUntil('\n');
    // waitResponse();
    // res.trim();
    // return res;
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
    for (unsigned long start = millis(); millis() - start < timeout_ms; ) {
      sendAT(GF("+CPIN?"));
      if (waitResponse(GF(GSM_NL "+CPIN:")) != 1) {
        delay(1000);
        continue;
      }
      int status = waitResponse(GF("READY"), GF("SIM PIN"), GF("SIM PUK"), GF("NOT INSERTED"));
      waitResponse();
      switch (status) {
        case 2:
        case 3:  return SIM_LOCKED;
        case 1:  return SIM_READY;
        default: return SIM_ERROR;
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
	waitResponse(10000); //NECESARIO POR EL TIEMPO QUE TARDA EL MODULO EN ENCENDERSE, AL USARSE EN OTRA OCASION PUEDE MOSTRAR UN UNHANDLED
	
    RegStatus s = getRegistrationStatus();
    if (s == REG_OK_HOME || s == REG_OK_ROAMING)
      return true;
    else if (s == REG_UNKNOWN)  // for some reason, it can hang at unknown..
	{
		DBG("TINYGSM: REGUNKNOWN");
      return isGprsConnected();
	}
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

  bool gprsConnect(const char* apn, const char* user = NULL, const char* pwd = NULL) {   /////////////////////BC95G
    gprsDisconnect();
	
	sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');  
    waitResponse();
	delay(100);
	
	if (user && strlen(user) >= 0) {
	sendAT(GF("+CGAUTH=1,2,\""), user, GF("\",\""), pwd, '"');
    waitResponse();
    }
	
	sendAT(GF("+CEREG=2"));  
	waitResponse();
	
	delay(100);

    sendAT(GF("+CGATT=1"));  // attach to GPRS
    if (waitResponse(180000L) != 1) { //360000L
      return false;
    }
	
	if (waitResponse(10000,"+CEREG:2") != 1) 
      return false;
	   
	  
	if (waitResponse(30000,"+CEREG:1") != 1) 
      return false;
  
	
	return true;
  }

  bool gprsDisconnect() {       /////////////////////BC95G
  
    sendAT(GF("+CGATT=0"));  // detach from GPRS
    if (waitResponse(360000L) != 1) {
      return false;
    }

    return true;
  }
  
  bool mqttConnect(const char* host,  uint16_t port, const char* ID = NULL, const char* user = NULL,  const char* pwd = NULL) {   /////////////////////BC95G
  
  int indice1, indice2, indice3;
  	sendAT(GF("+CGATT?;+CREG?;+CEREG?;+CGREG?"));  
waitResponse(10000L);	
		sendAT(GF("+CREG?"));  
waitResponse(10000L);		
			sendAT(GF("+CEREG?"));   
			waitResponse(10000L);
				sendAT(GF("+CGREG?"));   
				waitResponse(10000L);
  
  	sendAT(GF("+QIDNSCFG=8.8.8.8,8.8.4.4"));       
    if(waitResponse()!=1)
	{
		DBG("TINYGSM: ERROR AL ACTUALIZAR DNS");
		return false;
    }
	
	delay(150);

	sendAT(GF("+QMTOPEN=0,\""), host,"\",", port);       
    waitResponse();
	
		if(waitResponse(75500,"0,")!=1)
		{
			return false;
		}
		
		indice1 = waitResponse(1000,"0" GSM_NL,"-1" GSM_NL,"1" GSM_NL,"2" GSM_NL,"3" GSM_NL);
		
		if(indice1!=1)
		{
			switch(indice1)
			{
				case 2:
						DBG("TINYGSM: ERROR AL ABRIR LA CONEXION CON EL BROKER");
						break;
				case 3:
						DBG("TINYGSM: ERROR EN PARAMETROS INDICADOS");
						break;
				case 4:
						DBG("TINYGSM: ERROR BROKER OCUPADO");
						break;
				case 5:
						DBG("TINYGSM: ERROR AL ACTIVAR PDP");
						break;
				default:
						DBG("TINYGSM: ERROR EN +QMTOPEN");
						break;
				
			}
			
			DBG("TINYGSM: ERROR AL ABRIR LA CONEXION CON EL BROKER");
			return false;
		}
		
		else
			DBG("TINYGSM: Broker accesible");
	
   delay(100);
  
	 sendAT(GF("+QMTCONN=0,\""), ID, "\",\"", user, "\",\"", pwd, "\"");     
     waitResponse();
	 		// if(waitResponse(30000,"0,0,0" GSM_NL)!=1)
		// {
			// DBG("TINYGSM: Error al conectar con broker");
			// return false;
		// }
		// else
			// DBG("TINYGSM: Broker conectado");

	 indice2= waitResponse(60500,"0,0","0,1","0,2");
	 if(indice2!=1)
		{
			switch(indice2)
			{
				case 2:
						DBG("TINYGSM: PAQUETE REENVIADO");
						break;
				case 3:
						DBG("TINYGSM: ERROR AL ENVIAR PAQUETE");
						break;
				default:
						DBG("TINYGSM: ERROR EN +QMTCONN 1");
						break;
			}
			return false;
		}
		
	 indice3= waitResponse(1000,",0" GSM_NL,",1" GSM_NL,",2" GSM_NL,",3" GSM_NL,",4" GSM_NL);
	 
	 if(indice3!=1)
		{
			switch(indice2)
			{
				case 2:
						DBG("TINYGSM: ERROR CONEXION RECHAZADA: VERSION DEL PROTOCOLO NO ACEPTADA");
						break;
				case 3:
						DBG("TINYGSM: ERROR CONEXION RECHAZADA: IDENTIFICADOR RECHAZADO");
						break;
				case 4:
						DBG("TINYGSM: ERROR CONEXION RECHAZADA: BROKER NO DISPONIBLE");
						break;
				case 5:
						DBG("TINYGSM: ERROR CONEXION RECHAZADA: USUARIO O CONTRASEÑA");
						break;
				default:
						DBG("TINYGSM: ERROR EN +QMTCONN 2, (5) NO AUTORIZADO");
						break;
			}
			DBG("TINYGSM: ERROR AL CONECTAR CON EL BROKER");
			return false;
		}
	
	else
		DBG("TINYGSM: Broker conectado");
	
   delay(100);
   
	return true;
  
  }
  
   bool mqttSend(String topics, String datos) {
	   return mqttSend(topics.c_str(),datos.c_str());
   }
  
  bool mqttSend(const char* topics, const char* datos) {   /////////////////////BC95G
		int indice1;
     delay(100);
	sendAT(GF("+QMTPUB=0,1,2,1,\""), topics,"\"");        //AT+QMTPUB=<tcpconnectID>,<msgID>,<qos>,<retain>,"<topic>"
   
    if (waitResponse(GF(">")) != 1) {  
       return false;
     }
	delay(100);
	
	 
     stream.print(datos);  
     stream.write((char)0x1A);
     stream.flush();
	
	
	
	   if(waitResponse()!=1)
	{
			DBG("TINYGSM: Error al enviar mensaje");
			return false;
    }
	//////////////////////////////////////////////////////////
//	if(waitResponse(30000,"0,1,0" GSM_NL)!=1)  //if(waitResponse(330000,"0,0,0" GSM_NL)!=1)
//		{
//			DBG("TINYGSM: Error al enviar mensaje2");
//			return false;
//		}
//	else
//		DBG("TINYGSM: Mensaje enviado correctamente");
	
		////////////////////////////////////////////////////////
   
	
		if(waitResponse(30000,"0,1")!=1)    //+QMTPUB: <tcpconnectID>,<msgID>,<result>[,<value>]
		{
			DBG("TINYGSM:CONFLICTO EN EL ID DEL MENSAJE");
			return false;
		}
		
		indice1 = waitResponse(1000,",0" GSM_NL,",1" GSM_NL,",2" GSM_NL);
		
		if(indice1!=1)
		{
			switch(indice1)
			{
				case 2:
						DBG("TINYGSM: ERROR REENVIO DEL PAQUETE");
						break;
				case 3:
						DBG("TINYGSM: ERROR AL ENVIAR EL MENSAJE");
						break;
				default:
						DBG("TINYGSM: ERROR EN +QMTOPUB");
						break;
				
			}
			
			DBG("TINYGSM: ERROR AL PUBLICAR EL MENSAJE");
			return false;
		}
		
		else
			DBG("TINYGSM: Mensaje enviado correctamente");
	
   delay(100);

	return true;
  }
  
  
  
    bool mqttDisc() {   /////////////////////BC95G
  
    delay(100);
  	sendAT(GF("+QMTDISC=0"));       //AT+QMTDISC=<tcpconnectID>
    if(waitResponse()!=1)
	{
		DBG("TINYGSM: ERROR DESCONECTANDO DEL BROKER");
		delay(1000);
		return false;
    }
	
	if(waitResponse(1000,"0,0","0,-1")!=1)    //+QMTDISC: <tcpconnectID>,<result>
		{
			DBG("TINYGSM:ERROR AL CERRAR LA CONEXION");
			return false;
		}	
	else
		DBG("TINYGSM: DESCONEXION DEL BROKER CORRECTA");
	
		
	delay(100);

	// sendAT(GF("+QMTCLOSE=0"));      //AT+QMTCLOSE=<tcpconnectID>  //NO PUEDE HACERSE SI ANTES SE HIZO UN DISC Y VICEVERSA, DISC ES MAS COMPLETO AL CERRAR CONEXION TCP
    // waitResponse();
	
	// if(waitResponse(1000,"0,0","0,-1")!=1) //+QMTCLOSE: <tcpconnectID>,<result>
	// {
		// DBG("TINYGSM: ERROR AL CERRAR COMUNICACION CON EL BROKER");
		// return false;
	// }
	// else
		// DBG("TINYGSM: COMUNICACION CON EL BROKER CERRADA CON EXITO");
	// waitResponse(100,"ERROR");
	
   
	return true;
  
  }
  
  
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

  bool getBattStats(uint8_t &chargeState, int8_t &percent, uint16_t &milliVolts) {
    percent = getBattPercent();
    return true;
  }

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
    int16_t res = stream.readStringUntil('\n').toInt();
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

  bool modemConnect(const char* host, uint16_t port, uint8_t* mux,
                    bool ssl = false, int timeout_s = 120)
  {
    uint32_t timeout_ms = ((uint32_t)timeout_s)*1000;
    sendAT(GF("+USOCR=6"));  // create a socket
    if (waitResponse(GF(GSM_NL "+USOCR:")) != 1) {  // reply is +USOCR: ## of socket created
      return false;
    }
    *mux = stream.readStringUntil('\n').toInt();
    waitResponse();

    if (ssl) {
      sendAT(GF("+USOSEC="), *mux, ",1,0");  //ORIGINAL: sendAT(GF("+USOSEC="), *mux, ",1"); // sendAT(GF("+USOSEC="), *mux, ",1,0");
      waitResponse();
    }
	
	
    // Enable NODELAY
    sendAT(GF("+USOSO="), *mux, GF(",6,1,1"));
    waitResponse();

    // Enable KEEPALIVE, 30 sec
    //sendAT(GF("+USOSO="), *mux, GF(",6,2,30000"));
    //waitResponse();
	
	//AÑADIDO DE MKRNB
  //  sendAT(GF("+USECPRF="), *mux, GF(",0,1"));  
  //  waitResponse();


    // connect on the allocated socket
    sendAT(GF("+USOCO="), *mux, ",\"", host, "\",", port); 
    int rsp = waitResponse(timeout_ms);
	DBG("Valor devuelto de USOCO: ",rsp);
    return (1 == rsp);
  }

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
	delay(100);
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
    sendAT(GF("+USOCTL="), mux, ",10");
    uint8_t res = waitResponse(GF(GSM_NL "+USOCTL:"));
    if (res != 1)
      return false;

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
