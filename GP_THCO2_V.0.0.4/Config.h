//#define TINY_GSM_DEBUG Serial
#define EN_DEBUG //determina si se habilitan los prints y el envío de t=0 cuando se resetea

//#define AWS_CLOUD //habilita el uso del ECCX08 para encriptación de mensajes



/////PLACA////
#define NANO33IoT //para THCO2 y AIRQUALITY
//#define MINI_BREAKOUT //para CPD


///////PUERTO MIKROE/////
#ifdef MINI_BREAKOUT
#define ETH_MODEM_W5500
/////SENSORES/////
#define SHT30NANO33IoT
#endif

#ifdef NANO33IoT
#define WIFI_NINA
#define AIR_QUALITY_4
#define SHT30
#define INTERVAL_HEATER 60  //frecuencia en minutos a la que se habilitara el calentador del SHT30
#define TIME_HEAT 30 //en segundos tiempo que estará encendido el calentador
#define TIME_COLD 60 //en segundos tiempo que estará enfriando después de que se apague el calentador
#define NUM_MEASURES_V 60 //cantidad de datos para los vectores de media (V.0.0.2) equivale a segundos de medición
#define NUM_MEASURES_FAN INTERVAL_HEATER/4 //Numero de medidas del vector que se tomarán con ventilación (desde el final del vector)
#define DEF_NINA_DHCP
#endif

/////PROTOCOLO/////
#define TINY_GSM_MQTT




#define WDT_TIME WDT_SOFTCYCLE8M
#define M_BAUD 115200 //bc95G: 9600  //ESP32: 115200 //SIM808: 115200  //saraR410M/u201: 115200 //NRF9160:115200 //ESP32 htppfile: AT+UART_CUR=230423,8,1,0,1 //AT+UART_CUR=115200,8,1,0,1 CUR/DEF

#ifdef AWS_CLOUD
#define ECCX08_ADDR 0x32 //dirección I2C ECCX08
#endif


#define DEF_SSID          "BRESSON"            //"JLP_AP"      //"MarinePark"      //"ASTICAN_IOT"               //"AstWiFi"   //JH: "BRESSON_5G"
#define DEF_PASS          "BARCELONA"       //"0123456789"  //"Marine20_Park16" //"em7cKNXVTH5KW6U7bpknyZZH"  //"WiFi2013"  //JH: "BARCELONA"


#define PRINCIPAL_FREQ  600     //JH: 600//SSM:300 fijo de frecuencia normal de envío en segundos
uint16_t DEF_WAIT_TIME  = PRINCIPAL_FREQ; //TIEMPO ENTRE ENVIOS
#define SECONDARY_FREQ  300     //JH: 300 //SSM:150 valor fijo de frecuencia especial de envío en segundos -> depende de CHANGE_FREQ_CO2_UMBRAL 
#define DEF_TRY_TIME   120     //Tiempo de intento de conexión (wifi)

#define CHANGE_FREQ_CO2_UMBRAL  560     //limite a partir del cual la frecuencia de envío pasa a ser la secundaria


//CONFIG COMUNICACIONES MQTT
#define DEF_DIR_BROKER_MQTT  "mqtt.demo2.muutech.com"//JH:mqtt.demo2.muutech.com->"161.22.42.123"// mqtt.astican.muutech.com -> "161.22.42.123" ;// "172.19.1.200"; asticanmuubox //AWS_CLOUD_SSM: "a2hspccjaozl0m-ats.iot.us-east-1.amazonaws.com"-> 54.236.78.46
#define DEF_MQTT_PORT  12003 //8883 SSL // 1883 NO SSL //12040 MUUTECH //12010 asticanmuubox //AWS_CLOUD_SSM: 8883 //JH: 12003 
#define DEF_USER_BROKER_MQTT  ""
#define DEF_PASS_BROKER_MQTT  ""

#define DEF_TOPIC1_MQTT "LoftdeGracia"  //cliente    //JH: "LoftdeGracia" //"SSM"
#define DEF_TOPIC2_MQTT "Salon1" //ubicacion //JH: "Salon1"    //"OFFICE"

#ifdef NANO33IoT
#define DEF_TOPIC3_MQTT "THCO2"    // "CPD" // "AIRQUALITY" //"TEST"
#elif defined(MINI_BREAKOUT)
#define DEF_TOPIC3_MQTT "CPD"
#endif

#define DEF_SENSOR_ID     "SSM-JH-THCO2-001"  //"SSM-cpd-001" //"SSM-meet-001//SSM-GP-0001//SSM-JH-THCO2-001

#define DEF_TOPIC4_MQTT DEF_SENSOR_ID



#ifdef TINY_GSM_MQTT
#include <PubSubClient.h>
#endif
 
#ifdef AIR_QUALITY_4
#include "SparkFun_SGP30_Arduino_Library.h"
SGP30 sensorAirQuality;
#endif


#ifdef ETH_MODEM_W5500
#include <Ethernet2.h>

EthernetClient clientETH;
PubSubClient mqtt(clientETH); //mqtt(DIR_BROKER_MQTT,MQTT_PORT,mqttCallback,client);

uint8_t macTEST[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xA4 }; //CPD 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xA0 de A0 a A7
IPAddress IP_Master(172, 19, 1, 10); //[192.19.1.6 ... 192.19.1.13] //192, 168, 10, 17.18.19.20.21.22.23.24/25.26.27.28.29.30.31.32

#endif
#ifdef WIFI_NINA
#include <WiFiNINA.h>
int status = WL_IDLE_STATUS;

#ifndef DEF_NINA_DHCP
IPAddress IP_MEET(192, 168, 10, 53); //ASTICAN:172.19.1.14 Meets del 14 al 21//marinePark:192, 168, 10, 56
IPAddress _SUBNET(255, 255, 255, 0);
IPAddress _GATEWAY(192, 168, 10, 1);
IPAddress _DNS(8, 8, 8, 8);
#endif

WiFiClient client;


#ifdef AWS_CLOUD

#include <ArduinoECCX08.h>
#include <ArduinoBearSSL.h> //depende de ECCX08




//device_cert (.pem)       HAY QUE PASARLO A UN SECRETS PARA QUE NO ESTÉ PRESENTE EN LOS CODIGOS QUE NO HAGAN USO DE EL (SEGURIDAD)
const char AWS_CERT[] = R"(
-----BEGIN CERTIFICATE-----
MIIDIjCCAgqgAwIBAgIVAN1Gf0A2y3mfj0ukoEaVctmkVBeSMA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMzA2MjExMjMy
MzRaFw00OTEyMzEyMzU5NTlaMIGwMQswCQYDVQQGEwJFUzERMA8GA1UECBMIQ2Fu
YXJpYXMxIzAhBgNVBAcTGkxhcyBQYWxtYXMgZGUgR3JhbiBDYW5hcmlhMR8wHQYD
VQQKExZTdWJTZWEgTWVjaGF0cm9uaWNzIFNMMRwwGgYDVQQLExNEZXBhcnRhbWVu
dG8gZGUgSW9UMSowKAYDVQQDEyFGcmFuY2lzY28gSmF2aWVyIE9ydGVnYSBHb256
w6FsZXowWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAATvejB3IWma1+qzeopGkuFF
3emzKx24fWqJSixPzzBcGYvpF7K+dNstugBOZNPXoxEpBdw5ZZE9tP+U4xhKUiMd
o2AwXjAfBgNVHSMEGDAWgBTtmaSM4q1vON6ia2TAMywFi0zl8jAdBgNVHQ4EFgQU
C3/TbKMcoubCYJ43QGjze6XdZvowDAYDVR0TAQH/BAIwADAOBgNVHQ8BAf8EBAMC
B4AwDQYJKoZIhvcNAQELBQADggEBAGFjj2ipOvupgDC2M/XT+wHVxXJRujI403OH
Xs7GrRJy5HahUGVn298yrYUe1meUBik78p94g2+omHIobhwnCOAvABxO/x6Yg7VX
AYtN41Om5VsFF7cknAR7uKIgmdnv6ayTWwiNEa9LwhHUr9hgmL9XJzwU81yDWu5E
l7mVAnZa4QzuAfLfceBnPiTKhMsJMlmZyYYSRNKh1v3na+Qit8EZee2DrLyn2CJc
ZnVPpr86Y1OVGbvlENkKfK2Oh0qJ9t5PS6zZih5gWJfTAJTvaqQKKnPnjixPJobT
AMePYp+feFyvfe2z58v1sg/lo9IDxZQel88QNkQh6SC5YBEW0xQ=
-----END CERTIFICATE-----
)";

BearSSLClient bearSSLclient(client);
PubSubClient mqtt(bearSSLclient);
#else
PubSubClient mqtt(client);
#endif
#endif
