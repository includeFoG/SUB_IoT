/*
 * IMPORTANTE!!!:
 * PARA SIMPLIFICAR EL PROGRAMA EL DIAMETRO DE LA CIRCUNFERENCIA PRIMITIVA DE LA RUEDA DENTADA LIGADA A LA POLEA SOBRE LA QUE SE MUEVE EL CABLE HA DE SER IGUAL AL DIAMETRO DE LA POLEA
 */

long oldPosition  = -999;    //determina el valor "anterior" de posición 
long pretime = -999;         //indica el momento millis() en el que se hizo el último envío

uint8_t send_period = 6;    //Frecuencia de envío por RS232 Hz

//UNIDADES DE LONGITUD EN METROS
uint16_t ticksInLoop = 600.0;  //característica del encoder ticks por vuelta
uint16_t ticksLibrary = ticksInLoop * 4.0; //el x4 se debe a que la librería hace conteo de los dos flancos de las dos señales
float radioMain = 0.03375;         //radio de la rueda dentada del eje del encoder (circunferencia primitiva)[metros]
float perRuedaMain = 2.0 * PI * radioMain ; //perimetro de la rueda dentada del eje del encoder 
float resMain = perRuedaMain / ticksLibrary; //calcula lo que avanza 1 tick en relación al perímetro de la rueda dentada del eje de entrada
float distance = 0.0; //variable donde se guardará la distancia recorrida de cable
#define WDT_TIME WDT_SOFTCYCLE16S

bool oldBUT = true; //estado anterior del boton
bool newBUT = true; //estado actual del boton
