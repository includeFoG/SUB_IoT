/*
   ESTA ERA LA INTERRUPCION ORIGINAL:
   ====================================

   void IRAM_ATTR ISR_connectPowerCable() { //IRAM para que vaya a la RAM y no a la flash y sea más rapida
  if ((millis() - lastTimeISRWire) > 200) { //DEBOUNCING
    detachInterrupt(digitalPinToInterrupt(PWR_IN));
    Serial.println("\n>>>ISR_connectPowerCable");

    //comprobar que no esté midiendo en una sesión
    if ((STATE != INTIME) && (STATE != MEASURING)) {
      //ver si está cargando o cargado
      checkStateOfChargeToSleep(true, true);


    }
    attachInterrupt(digitalPinToInterrupt(PWR_IN), ISR_disconnectPowerCable, FALLING); //habilitamos interrupción desconexion de PWR_IN
    Serial.println(">>>FIN DE INTERRUPCION2");
  }
  lastTimeISRWire = millis();
  }*/


//*****************************************************************
// Thread taskISRWire
//*****************************************************************

void checkStateOfChargeToSleep(bool sleepIfCharging , bool sleepIfFullCharged ); //PROTOTIPADO procede de F_aux.h

TaskHandle_t ISRWire_task_handle;
void TaskISRWire( void *pvParameters ) {

    //myDelayMs(1000);//delay para alejarnos de posible interrupción previa
    //sincroniza NTP con el RTC_INT
    Serial.println("\nInit [taskIRWIRE]");
    for (;;) {
      Serial.println("A");
      
      //no nos interesa que se ejecute nada mientras, recordemos que esto es porque en la interrupción no podíamos ejecutar todo el código que necesitabamos NO HEMOS ENCONTRADO LA FORMA DE PARAR EL RESTO DE HILOS

      checkStateOfChargeToSleep(true, true);//ver si está cargando o cargado

      Serial.println("B");
      //xTaskResumeAll(); //no va a llegar aquí

      ISRWire_task_handle = NULL;
      vTaskDelete( NULL );
    }
}
