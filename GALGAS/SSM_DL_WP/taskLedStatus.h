
//*****************************************************************
// Thread TaskLedStatus
//*****************************************************************
TaskHandle_t LedStatus_task_handle;
void TaskLedStatus( void *pvParameters ) {
  EventBits_t uxBitsLeds;

  for ( int i = 0 ; i < 4; i++)
  {
    blinkLed(i, 1000);
  }
  for (;;)
  {
    uxBitsLeds = xEventGroupGetBits(xEventLeds);

    if ( ( uxBitsLeds & BIT_0_LED_DATA_SAVE ) != 0 ) {
      blinkLed(2, 50);
      xEventGroupClearBits(xEventLeds, BIT_0_LED_DATA_SAVE);
    }
    
    if(STATE != MEASURING){
          if (digitalRead(PWR_IN)) { //si está habilitado el pin de tensión de entrada -> alimentación de batería ya que aquí solo nos interesa ver si está en carga o cargada 100% (con conector)
          switch (checkIfCharging()) { //
            case 4: //cargando
              blinkLed(1, 500);
              break;
            case 5: //100% cargado
              blinkLed(1, 0, 0); //enciende continuo
              break;
            default:
              break;
          }
        }
        else if (!digitalRead(RLED)) { //aseguramos que esté apagado el led si no está conectado el cable de carga//tiene logica inversa
          blinkLed(1, 0, 1); //1 pulso de duración 0 -> apaga el led MODIF
        }
    }

    blinkLed(0, 50);
    myDelayMs(2000);
  }
  Serial.println("!!!out TaskLed");
  LedStatus_task_handle = NULL;
  vTaskDelete( NULL );
}
