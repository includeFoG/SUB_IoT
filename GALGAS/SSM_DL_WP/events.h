#include "event_groups.h"


//___________________________________________________EVENTS___________________________________________________
#define BIT_0_DESC_WIFI     ( 1 << 0 ) //bit que indica si el wifi se tiene que desconectar
#define BIT_1_ACTU_RTC_EXT  ( 1 << 1 )  //bit que indica si se tiene que actualizar el RTC EXT en función del interno
EventGroupHandle_t xEventWifi; //Grupo de eventos relacionado con actuacion de taskWiFi sobre distintos elementos


#define BIT_0_DAT_VIEW      ( 1 << 0 ) //Indica si se ha habilitado la visualización de datos con el comando CMD_VIEW_DATA = $viewdata
#define BIT_1_DAT_SESION    ( 1 << 1 ) //Indica si se ha habilitado una sesión de datos con el comando CMD_DATA_SESION = $datasesion o por tiempo
//#define BIT_2_DAT_START     ( 1 << 2 ) //Indica si ha comenzado la sesión de datos (solo para sesiones temporizadas (de momento solo las de vector RTC))
//#define BIT_3_END_SESION  ( 1 << 3 ) //Indica si se ha habilitado el cierre de una sesión con el comando CMD_STOP_DATA = $stopdata o por tiempo NECESARIO PORQUE: se puede tener el bit BIT_1_DAT_SESION a 0 y estar viendo los datos, si eso te cierra un archivo que no existe da problemas
//se puede solucionar si funciona el tema de ver si está abierto el file
EventGroupHandle_t xEventDatos; //Grupo de eventos relacionado con la toma y visualización de datos


#define BIT_0_LED_DATA_SAVE (1 << 0 ) //indica si se están almacenando datos en la SD
EventGroupHandle_t xEventLeds; //Grupo de eventos relacionado con los leds indicadores del sistema



bool createEvents() {
  xEventWifi = xEventGroupCreate();
  xEventDatos = xEventGroupCreate();
  xEventLeds = xEventGroupCreate();

  if ( xEventWifi == NULL )
  {
    Serial.println("Fail creating Events0");
  }
  if ( xEventDatos == NULL )
  {
    Serial.println("Fail creating Events1");
  }
  if ( xEventLeds == NULL )
  {
    Serial.println("Fail creating Events2");
  }

  return true;
}



//________________________________________________SEMAPHORES________________________________________________
volatile SemaphoreHandle_t BLE_txSemaphore; //Semaforo usado para impedir que otros hilos escriban de forma simultanea en txBLE sin que otro termine
volatile SemaphoreHandle_t manager_semaphore; //Semaforo usado en en loop() (manager) para esperar a que se cumplan ciertas acciones sin sobrecargas

void createSemaphores() {
  BLE_txSemaphore = xSemaphoreCreateBinary();
  manager_semaphore = xSemaphoreCreateBinary();
}
