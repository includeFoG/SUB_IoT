TaskHandle_t Handle_pwrManagment;
//
//#define BIT_0_changePwr         (1<<0)
//#define BIT_1_comModule         (1<<1)
//#define BIT_2_fileModule        (1<<2)
//#define BIT_3_gpsModule         (1<<3)
//#define BIT_4_analogReading     (1<<4)
//#define BIT_5_pwrMdvr           (1<<5)

//#define EN_MDVR               EN_RS232_0

//void doHigh(uint8_t outnum); //ADDED
//void doLow(uint8_t outnum);  //ADDED

static void resetX1() { //ADDED
  DEBUG("Reiniciando el dispositivo");
  for (int i = 0; i < 5; i++)
  {
    DEBUG(".");
    myDelayMs(1000);
  }

  NVIC_SystemReset();
}



static void pwrManagment( void *pvParameters )
{
  DEBUG("START POWER MANAGMENT");

  shiffty.clear(EN_5VCC);
  for (int k = 0 ; k < 3 ; k++) {
    shiffty.set(EN_SERVO);
    myDelayMs(180);
    shiffty.clear(EN_SERVO);
    myDelayMs(60);
  }
  shiffty.set(EN_5VCC);


  EventBits_t status;
  EventBits_t bitsBuzzer;
  //byte powerStatus;

  int auxBinActuation = ( (B01111111 * 65536) + (B11111111 * 256) + B10000000 ); //ADDED B01111111 11111111 11000000

  PCF.begin();//ADDED !!! REVISAR UBICACION

  while (1)
  {
    xEventGroupSetBits( eventTaskState,  BIT_1_taskPwr ); //ADDED WORKING
    EventBits_t uxBits_PCF_aux; 

    taskYIELD(); //ADDED
    status =  xEventGroupWaitBits(
                eventPwr5volts,
                BIT_0_changePwr,
                true,
                false,
                portMAX_DELAY ) ;

    if ((status & BIT_0_changePwr ) != 0 )
    {
      if ((status & (BIT_4_analogReading | BIT_3_gpsModule | BIT_2_fileModule | BIT_1_comModule | BIT_5_buzzer | BIT_6_actuation)) != 0 )  //ANTES CONTENIA BIT_5_pwrMdvr cambiado por BIT_5_buzzer
      {
        shiffty.clear(EN_5VCC); DEBUG(" Set 5vcc ");

        if ((status & ( BIT_3_gpsModule |  BIT_1_comModule )) != 0 )
        {
          shiffty.set(EN_MIKRO); DEBUG(" Set Mikro ");
        }
        else
        {
          shiffty.clear(EN_MIKRO); DEBUG(" Clear Mikro ");
        }

        if ((status & (BIT_6_actuation)) != 0)
        {
          shiffty.set(EN_ARDUCAM);  //ADDED

          //SERIAL.println("ARDUCAM ON!!!!!!");
          // shiffty.set(EN_ANALOGS); //ULTIMASPRUEBAS QUITADO PARA QUE NO SE ENCIENDAN LOS RELES PASADO DESPUÉS DE PCF.WRITE

          //  DEBUG(" Set Arducam <-----");

          myDelayMs(1); //necesario para que en write16 no se activen todos los bits a 1

          //accionPCF trabaja con los bits: del 6 al 22  de eventPwr5volts ADDED MODIF
          uxBits_PCF_aux = xEventGroupGetBits( eventPwr5volts );
          //if ( ((uxBits_PCF & (BIT_13_S6_RS1 | BIT_23_UBIFLAG_ON)) != 0)) {  //MODIFICADO POR CAMBIOS DVR EN PLACA TEST 14/06
          if ( ((uxBits_PCF_aux & BIT_23_UBIFLAG_ON) != 0)) { //si se habilita el flag de camaras de ubidots
            uxBits_PCF_aux |= BIT_17_S10_L | BIT_16_S9_RS2 | BIT_13_S6_RS1; //enciende alimentación de camaras de ambas velas y cable amarillo dvr, manteniendo el estado de los bits anteriores
            uxBits_PCF_aux = uxBits_PCF_aux & ~(1 << 23); //esto lo hacemos para quitar el bit de BIT_23_UBIFLAG_ON para enviar la palabra a la placa de actuación
            
            /*esto es un parche ya que cuando se encienden las cámaras de forma manual por algún motivo se está apagando los sensores a la hora de hacer la medida dando un BITS PCF: 10000011111001100 (SIN SENTIDO)
                              000XXX00XX0XXXX0X  X= no en uso
               [PWR]BITS PCF: 10000011111001100 INCORRECTO
               [PWR]BITS PCF: 11100011001000010 CORRECTO
                uxBits_PCF |= BIT_8_S1_E | BIT_21_S14_H | BIT_22_S15_G;
            */

            SERIAL.println("PRE PCF: " + String(uxBits_PCF_aux, BIN));
            myDelayMs(1); //SI SE PONE A 5 ENCIENDE LOS RELES
          }
          uxBits_PCF_aux = uxBits_PCF_aux >> 7; //quita los 7 primeros bits de eventPwr5volts  que no pertenecen a la placa de actuación (omite también el 6 el encendido de la propia placa) empieza en bit7

          SERIAL.print("\n[PWR]BITS PCF: ");
          SERIAL.println(String(uxBits_PCF_aux, BIN));


          PCF.write16(uxBits_PCF_aux);
          shiffty.set(EN_ANALOGS); //Añadido 15/06


          if ((xEventGroupGetBits(eventPwr5volts) & auxBinActuation) == 0) { //Si están a 0 todos los bits de actuación (MENOS FLAG UBIDOTS SWITCH CAMARAS)
            //xEventGroupClearBits( eventPwr5volts, BIT_6_actuation ); //MODIFICADO 14/06
            //SERIAL.println("TODOS LOS ACTUADORES APAGADOS!!!!!!!!!!!!!!!");

            if ((status & BIT_23_UBIFLAG_ON) == 0) //si se apaga el flag de ubidots (fin de tiempo)
            {
              shiffty.clear(EN_ARDUCAM);
              shiffty.clear(EN_ANALOGS); //ADDED
              xEventGroupClearBits( eventPwr5volts, BIT_6_actuation ); //MODIFICADO 14/06
              xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr  ); //MODIFICADO 14/06
              DEBUG(" clear arducam <------------------------------");
              SERIAL.println("ANALOG OFF!!!!!!!!!!!!!!!!!!");
            }
          }
        }
        if ((status & (BIT_5_buzzer)) != 0)
        {
          bitsBuzzer = xEventGroupGetBits( eventTaskState );
          myDelayMs(10);

          if ((bitsBuzzer & BIT_6_IM_sendData ) != 0) { //SI ESTA ENCENDIDO EL MODULO DE COMUNICACIONES
            shiffty.set(EN_SERVO);
            myDelayMs(150);
            shiffty.clear(EN_SERVO);
            xEventGroupClearBits( eventTaskState, BIT_6_IM_sendData);
          }
          if ((bitsBuzzer & BIT_16_IM_saveData) != 0) // Si se encienden los sensores pita buzzer
          {
            for (int k = 0; k < 2 ; k++)
            {
              myDelayMs(200);
              shiffty.set(EN_SERVO);
              myDelayMs(200);
              shiffty.clear(EN_SERVO);
            }
            xEventGroupClearBits( eventTaskState, BIT_16_IM_saveData);
          }
          xEventGroupClearBits( eventPwr5volts, BIT_5_buzzer);
          shiffty.clear(EN_SERVO); //asegura limpiar estado de buzzer
        }
        else {
          shiffty.clear(EN_SERVO); //asegura limpiar estado de buzzer
        }
      }
      else
      {
        if ((status & BIT_23_UBIFLAG_ON) == 0) //ADDED
        {
          shiffty.initialize();
          shiffty.set(EN_5VCC);
          DEBUG(" Shiffty initialize <------------------------------");
        }
      }
    }
  }
}


//void doHigh(uint8_t outnum) //ADDED
//{
//  DEBUG("UP: " + String(outnum));
//  PCF.write(outnum, HIGH);
//}
//
//
//void doLow(uint8_t outnum) //ADDED
//{
//  //DEBUG("DOWN: " + String(outnum));
//  PCF.write(outnum, LOW);
//}
