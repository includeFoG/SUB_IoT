#include "event_groups.h"


//___________________________________________________EVENTS___________________________________________________
#define BIT_0_SG1_READY     ( 1 << 0 )  //bit que indica que se ha tomado la medida de las galgas sg1
#define BIT_1_SG2_READY     ( 1 << 1 )  //bit que indica que se ha tomado la medida de las galgas sg2

EventGroupHandle_t xEventSend; //Grupo de eventos relacionado con actuacion de taskWiFi sobre distintos elementos


bool createEvents() {
  xEventSend = xEventGroupCreate();

  if ( xEventSend == NULL )
  {
    Serial.println("Fail creating Events0");
  }
  return true;
}
