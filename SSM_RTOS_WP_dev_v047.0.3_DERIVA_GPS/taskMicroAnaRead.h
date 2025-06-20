TaskHandle_t Handle_microAnaReadTask;


void adcConfig()
{

  ADC->CTRLA.bit.ENABLE = 0;                     // Disable ADC
  while ( ADC->STATUS.bit.SYNCBUSY == 1 );       // Wait for synchronization

  ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1_Val;
  ADC->AVGCTRL.reg =  ADC_AVGCTRL_SAMPLENUM_64 |  ADC_AVGCTRL_ADJRES(4);
  ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV16 |
                   ADC_CTRLB_RESSEL_12BIT;
  ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_DIV2_Val;
  ADC->CTRLA.bit.ENABLE = 1;                     // Enable ADC
  while ( ADC->STATUS.bit.SYNCBUSY == 1 );       // Wait for synchronization
}

float map4_20(long dato)
{
  long temp = dato * 3300 / 4096;
  return (float)temp / 100.00;

}

float mapVolt ( long dato)
{
  float _v2 = dato * 3300 / 4096;

  const float _r1 = RES_BOARD;
  const float _r2 = RES_INLINE;
  float temp = (_r1 + _r2) / _r1;
  //  temp = _r1/temp;
  return _v2 * temp / 1000;

}


#define PIN_ANA_MUX CAM_SS  //recibe lectura placa analog
#define PIN_CTRL_MUX SERVO_PWM //deberia ser DIGI3 

const int pinArray[4] = { DIGI0, DIGI1, DIGI2, PIN_CTRL_MUX }; //ADDED

void init_Mux() { //Inicia mux placa analog a 0
  pinMode(PIN_ANA_MUX, INPUT);
  // digitalWrite(PIN_CTRL_MUX, LOW); //MODIF
  for (int i = 0; i < 4; i++) //MODIF
  {
    pinMode(pinArray[i], OUTPUT);
    digitalWrite(pinArray[i], LOW);
  }
  //  Serial.println("init_M2");
}

void SetMuxChannel(byte canal) //Selecciona el canal indicado a partir de 4 bits: DIGI0, DIGI1, DIGI2, PIN_CTRL_MUX
{
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(pinArray[i], bitRead(canal, i));
  }
}




void powerOnMux()
{
  uxBits_PCF = xEventGroupGetBits( eventPwr5volts );

  EventBits_t uxBits_STATE = xEventGroupGetBits( myEvents );//AÑADIDO POR WAIT4

  for (int i = 0; i < 4; i++)
  {
    pinMode(pinArray[i], OUTPUT);
  }
  xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr | BIT_4_analogReading );
  // digitalWrite(PIN_CTRL_MUX, HIGH);  //MODIF
  SetMuxChannel(0); //fija el canal 0

  SERIAL.println("ANA:START");
  DEBUG("ANA: POWER SENSORS");
  uxBits_PCF |= BIT_22_S15_G; //ENCIENDE LS + TD
  uxBits_PCF |= BIT_21_S14_H; //ENCIENDE SENSORES VELA 1
  uxBits_PCF |= BIT_8_S1_E;   //ENCIENDE SENSORES VELA 2

  if ( (uxBits_STATE & BIT_10_saveData) != 0)
  {
    /*uxBits_PCF |= BIT_22_S15_G; //ENCIENDE LS + TD
      uxBits_PCF |= BIT_21_S14_H; //ENCIENDE SENSORES VELA 1
      uxBits_PCF |= BIT_8_S1_E;   //ENCIENDE SENSORES VELA 2 */

    uint8_t t_night = rtc.getHours();
    SERIAL.print("Check time: ");
    SERIAL.println(t_night);

    if ((t_night < 22) && (t_night > 4))
    {
      DEBUG("ANA: ON DVR Y CAMS");
      uxBits_PCF |= BIT_17_S10_L;    //ENCIENDE DVR
      uxBits_PCF |= BIT_16_S9_RS2;   //ENCIENDE CAMARAS VELA 1
      uxBits_PCF |= BIT_13_S6_RS1;   //ENCIENDE CAMARAS VELA 2
    }
  }


  DEBUG("PwrOnMx accionPCF = " + String(uxBits_PCF, BIN));

  taskYIELD();                 //0b000XXX00XX0XXXX0XCCCCCCC
  accionPCF(true, uxBits_PCF); //0b011000110010000100000000


  myDelayMs(POWER_SENSOR_DELAY);
}

void powerOffMux()
{

  //ADDED MODIF
  //uxBits_PCF = NULL;
  EventBits_t uxBits_OFF_PCF = NULL;

  SERIAL.println("\nANA: SENS OFF");
  uxBits_OFF_PCF |= BIT_21_S14_H; //APAGA Sensores  Vela 1
  uxBits_OFF_PCF |= BIT_8_S1_E; //APAGA Sensores  Vela 2
  uxBits_OFF_PCF |= BIT_22_S15_G;  //APAGA LS + TD
  DEBUG("ANA: DVR OFF");
  uxBits_OFF_PCF |= BIT_17_S10_L;//APAGA DVR
  uxBits_OFF_PCF |= BIT_16_S9_RS2; //APAGA CAMARAS VELA1
  uxBits_OFF_PCF |= BIT_13_S6_RS1; //APAGA CAMARAS VELA2
                                    //0b000XXX00XX0XXXX0XCCCCCCC
  accionPCF(false, uxBits_OFF_PCF); //0b011000110010000100000000
  //ENDADDED

  for (int i = 0; i < 4; i++)
  {
    pinMode(pinArray[i], INPUT);
  }


  xEventGroupClearBits( eventPwr5volts, BIT_4_analogReading );
  xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr  );
}

#ifdef DEBUG_MEASURE
int _cont = 0;
#endif


bool validData(float data, int maxData, int minData)
{
  return ((maxData >= data) && (data >= minData));
}


static void threadMicroAnaRead( void *pvParameters ) {

  DEBUG("ANA: Start AnalogRead");

  adcConfig();
  init_Mux();

  struct qMessage localData;

  powerOnMux(); //QUITADO POR WAIT4

  TickType_t lastWakeTime = xTaskGetTickCount(); //ADDED

  static const int auxBinStatus = BIT_20_SendUbi | BIT_10_saveData; //( (B00000100 * 256) + B00000100 ); //ADDED  //WAIT4 B0000010000000100

  while (1)
  {
    xEventGroupSetBits( eventTaskState,  BIT_2_taskAna ); //ADDED WORKING

    EventBits_t uxBits = xEventGroupGetBits( myEvents );//AÑADIDO POR WAIT4

    if ((uxBits & auxBinStatus) == 0)//si no hay evento activo
    {
      powerOffMux();  //<<<-----------------------------------------------------------------------------Si no estoy midiendo me apago y espero a que el controlador me llame
      DEBUG("ANA: WAITING");

      xEventGroupWaitBits(//WAIT4 comprueba si se ha activado alguna de las funciones
        myEvents,
        BIT_20_SendUbi | BIT_10_saveData,
        false,
        false,
        portMAX_DELAY ) ;

      SERIAL.println("ANA: WAKE UP state: " + String((xEventGroupGetBits(myEvents) & auxBinStatus) , BIN)); //WAIT4

      //SERIAL.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>TESTA");
      powerOnMux();
      //SERIAL.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>TESTB");
    }
    //SERIAL.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>TESTC");
    lastWakeTime = xTaskGetTickCount(); //ADDED


    localData.qTimeStamp = millis();

    //MODIFICADO PARA MONTAJE POD 14/06
    for (uint8_t k = 0; k < 4; k++)
    {
      SetMuxChannel(k + 2); //+2 porque por temas de conexionado se han dejado libres las primeras 2 entradas
      localData.qData[k] = map4_20(analogRead(PIN_ANA_MUX));
    }
    for (uint8_t p = 0; p < 6; p++)
    {
      SetMuxChannel(p + 8); //+8 porque por temas de conexionado
      localData.qData[p + 4] = map4_20(analogRead(PIN_ANA_MUX));
    }

    SetMuxChannel(14);
    localData.qData[NUM_VALORES] = mapVolt(analogRead(PIN_ANA_MUX));
    SetMuxChannel(15);
    localData.qData[NUM_VALORES + 1] = mapVolt(analogRead(PIN_ANA_MUX));
    //Serial.println();

#ifdef DEBUG_MEASURE
    if (_cont == 20 )
    {
      SERIAL.print(localData.qTimeStamp);
      for (byte i = 0 ; i < NUM_VALORES ; i++)
      {
        SERIAL.print(','); SERIAL.print(localData.qData[i]);
      }
      SERIAL.print(','); SERIAL.print(localData.qData[NUM_VALORES]);
      SERIAL.print(','); SERIAL.print(localData.qData[NUM_VALORES + 1]);
      SERIAL.println();
      _cont = 0 ;
    }
    else
    {
      _cont++ ;
    }
#endif

    if ((uxBits & BIT_20_SendUbi) != 0 )//<<<----------------------------------------------Si tengo que enviar mando los ultimos datos a la cola de envío hasta que se setee el bit de envío
    {
      // xQueueSend( xCommQueue, ( void * ) &localData, ( TickType_t ) 0 );
      if (xQueueOverwrite( xCommQueue, ( void * ) &localData ) == pdTRUE) { //ver tambien xQueuePeek
        SERIAL.println("env.");
        DEBUG("CLear BIT_20_SendUbi"); //TESTEO
        xEventGroupClearBits( myEvents,  BIT_20_SendUbi );
      }
    }
    if ((uxBits & BIT_10_saveData) != 0 ) //<<<---------------------------------------------------Si estoy midiendo envío los datos a la cola de datos a loguear en los archivos y ejecuto pasados 100ms
    {
      xQueueSend( xDataQueue, ( void * ) &localData, ( TickType_t ) 0 );
    }

    taskYIELD(); //ADDED TEST
    myDelayMsUntil(&lastWakeTime, 100);
  }
  SERIAL.println("Deleting AnalogReading");

  xEventGroupClearBits( eventTaskState,  BIT_2_taskAna ); //ADDED WORKING

  vTaskDelete( NULL );
}
