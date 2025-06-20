//#define SSM_DEBUG
#define TINY_GSM_DEBUG Serial
//#define DEBUG_LOGEVENTS

#include "utiles/macros.h"
#include "utiles/myROM.h"
#include "utiles/myShifft.h"
#include "utiles/definicionesSensorBoxV3.h"

#include <Arduino.h>

#include "wiring_private.h"
#include <FreeRTOS_SAMD21.h>
#include <queue.h>
#include <event_groups.h>
#include "parametros.h"


#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
SdFat sd;

#define PCF8575_INITIAL_VALUE 0x0000
#include <PCF8575.h> // I2C 16chnl
PCF8575 PCF(0x20);

//#include <WDTZero.h>
//WDTZero wtd;
float frec_analogRead, frec_SDWrite = 0;



//*****************************************************************

const int chipSelect = SD_CS;
//String myTimeStamp = "";


//**************************************************************************
// Type Defines and Constants
//**************************************************************************

#define  ERROR_LED_PIN  25 //Led Pin: Typical Arduino Board
//#define  ERROR_LED_PIN  2 //Led Pin: samd21 xplained board

#define ERROR_LED_LIGHTUP_STATE  HIGH // the state that makes the led light up on your board, either low or high

// Select the serial port the project should use and communicate over
// Some boards use SerialUSB, some use Serial
//#define SERIAL          SerialUSB //Sparkfun Samd51 Boards
#define SERIAL          Serial //Adafruit, other Samd51 Boards

//**************************************************************************
// global variables
//**************************************************************************

TaskHandle_t Handle_controlTask;
TaskHandle_t Handle_controlTask2;

int accionPCF(bool power, uint8_t outnum); //ADDED

unsigned long timeOffset = 0;
void myDelayUs(int us)
{
  vTaskDelay( us / portTICK_PERIOD_US );
}

void myDelayMs(int ms)
{
  vTaskDelay( (ms * 1000) / portTICK_PERIOD_US );
}

void myDelayMsUntil(TickType_t *previousWakeTime, int ms)
{
  vTaskDelayUntil( previousWakeTime, (ms * 1000) / portTICK_PERIOD_US );
}
enum states {
  IDDLE,// = 0,
  CONFIG,// = 1, //ADDED
  NEWFILE,// = 2, //NO USADO
  MEASSURING,// = 3,
  POWERING,// = 4, //NO USADO
  COMMAND,// = 5,
  SETUP,// = 6,
  NEWCONFIG,// = 7, //ADDED
  RESETING,// = 9,
  TESTER,// = 10   //ADDED
};
states localState = SETUP;
states aux_state; //ADDED

EventBits_t uxBits_PCF; //ADDED mejor que sea global

#include "events.h"
#include "queuesDefinitions.h"

#include "communications.h"
#include "gestionTiempo.h"
#include "taskPwrMagnament.h"
#include "taskMicroAnaRead.h"
#include "taskComm.h"
#include "taskFile.h"
//#include "taskGPS.h"
//




void accionPCF(bool power, EventBits_t BITS_statusPCF) //ADDED MODIF
{
  if (power == true)
  {
    EventBits_t uxBits_PCF_2 = xEventGroupGetBits( eventPwr5volts );
    //SERIAL.print("EL ESTADO DE LOS BITS ES: " + String(uxBits_PCF_2, BIN));
    //SERIAL.println("\tSE QUIERE AÑADIR:" + String(BITS_statusPCF, BIN));
    xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr |  BIT_6_actuation | BITS_statusPCF );
  }
  else
  {
    //SERIAL.println("ACCIONPCF_OFF"); 
    //SERIAL.println("SE QUIERE QUITAR:" + String(BITS_statusPCF, BIN));
    xEventGroupClearBits( eventPwr5volts, BITS_statusPCF );
    xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr  );
  }
}


//*****************************************************************
// Create a thread that prints out A to the screen every two seconds
// this task will delete its self after printing out afew messages
//*****************************************************************



static void threadControl( void *pvParameters )
{
  TickType_t lastWakeTime = xTaskGetTickCount();
  EventBits_t uxBits = 0;

  myDelayMs(5000); //añadido para que el modem de comunicaciones esté más tiempo apagado después de un reset

  xTaskCreate(pwrManagment,         " Task Power ",    85, NULL, tskIDLE_PRIORITY + 7, &Handle_pwrManagment); //ANTES 70 //ANTES 100
  xTaskCreate(threadMicroAnaRead,   " Task AnRead",   115, NULL, tskIDLE_PRIORITY + 2, &Handle_microAnaReadTask); //ANTES 100 //ANTES 130
  xTaskCreate(threadFile,           " Task File",    425, NULL, tskIDLE_PRIORITY + 2, &Handle_fileTask); //ANTES 400
  //  xTaskCreate(threadgps,            " Task gps",      115, NULL, tskIDLE_PRIORITY + 2, &Handle_gpsTask); //ANTES 128
  xTaskCreate(threadComm,           " Task Comm",     420, NULL, tskIDLE_PRIORITY + 2, &Handle_CommTask); //ANTES 600

  int t_measure = MEASURE_PERIOD;
  int t_measuring = 0;
  int t_file    = FILE_PERIOD;
  int t_send    = SEND_PERIOD;
  //int t_gps     = GPS_PERIOD;

  int com_err = 0;
  int sys_cont = 0;
  uint8_t maxNumThread = uxTaskGetNumberOfTasks();
  String localBuffer = "";

  uint8_t countUBI = 0;
  uint8_t maxCountUBI = ((DVR_TIME_ON * 60 * 1000) / SLOT_TIME );

  //xEventGroupSetBits( myEvents,  BIT_9_sendFile);
  states _state = SETUP;


  //ADDED
#ifdef DEBUG_LOGEVENTS
  byte b_infoLog = NULL;
#endif
  //ENDADDED

  myDelayMs(4000); //AÑADIDO PARA DAR TIEMPO AL BUZZER AL INICIALIZAR HILO PWR

  while (1)
  {
    lastWakeTime = xTaskGetTickCount();
    xEventGroupSetBits( eventTaskState,  BIT_0_taskControl ); //ADDED WORKING

    EventBits_t uxBitsStatus = xEventGroupGetBits( myEvents );
    //
    //    if (_state == SETUP) Serial.println("Estado en SETUP");
    //    if (_state == MEASSURING) Serial.println("Estado en MEASSURING");
    //    if (_state == IDDLE) Serial.println("Estado en IDDLE");
    //    Serial.print( "STATE BITS: " ); Serial.println( uxBitsStatus, BIN );
    uxBits = 0;

    //ADDED
    if ((uxBitsStatus & BIT_18_ubidotsSwitch) != 0) { //SI SE HA RECIBIDO UN 1 EN EL SWITCH DE UBIDOTS
      if (countUBI == 0) { //SI ES LA PRIMERA ITERACION
        countUBI = 0;
        xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr | BIT_6_actuation | BIT_23_UBIFLAG_ON );
        DEBUG("DVR ACTIVADO POR UBIDOTS ");
      }

      countUBI++;
      SERIAL.print("-> count UBI: ");
      SERIAL.print(countUBI);
      SERIAL.print("/");
      SERIAL.println(maxCountUBI);

      if (countUBI > maxCountUBI) { // SI SE HA CUMPLIDO EL TIEMPO
        xEventGroupClearBits( myEvents, BIT_18_ubidotsSwitch);
        xEventGroupClearBits( eventPwr5volts, BIT_23_UBIFLAG_ON );
        xEventGroupSetBits( eventPwr5volts, BIT_0_changePwr);
        countUBI = 0;
        DEBUG("---> DVR DESACTIVADO POR TIMER ");
      }
    }
    //ENDADDED

    if (_state != SETUP)
    {
      if ( t_measure >= MEASURE_PERIOD)
      {
        uxBits |= BIT_10_saveData;
        uxBits |= BIT_17_snapFileToSend;
        t_measure = 0;
        _state = MEASSURING;
        DEBUG(">Clear BIT_11_Iddle"); //TESTEO
        xEventGroupClearBits( myEvents, BIT_11_iddle );
        xEventGroupSetBits( eventTaskState,  BIT_16_IM_saveData);
        xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr | BIT_5_buzzer );
        myDelayMs(5);
      }
      if  (_state == MEASSURING)
      {
        if ( t_measuring >= MEASURING_TIME)
        {
          uxBits |= BIT_11_iddle;
          DEBUG("BIT 11 IDDLE SET<--------");
          sys_cont++;
          t_measuring = 0;
          _state = IDDLE;

          DEBUG("Clear BIT_10_saveData"); //TESTEO
          xEventGroupClearBits( myEvents, BIT_10_saveData);


        }
        else if (  t_file  >= FILE_PERIOD)
        {
          uxBits |= BIT_0_createFile;
          DEBUG("CTRL: >>>SET BIT_0_createFile<<<");
          t_file = 0;
        }
        t_file++;
        t_measuring ++;
        DEBUG("t_measuring: " + String(t_measuring) + "<------");
      }

      SERIAL.print("\nt_send: ");
      SERIAL.println(String(t_send));

      if (t_send >= SEND_PERIOD) {
        if ((uxBitsStatus & BIT_2_SendData) != 0)
        {
          com_err ++;
          SERIAL.println("-----COMM NO RESPONDE-----");
        }
        else {
          com_err = 0;
          //DEBUG("Flag de error de envio reseteado");
        }
        if (com_err >= 2)
        {
          com_err = 0;
          SERIAL.println("-----RESET TASK COM-----");

          modem.gprsDisconnect();//NUEVO 23/11/2023
          powerOffComm();//NUEVO 23/11/2023

          DEBUG("Clear BIT_9_sendFile, Clear BIT_13_doneFileSend"); //TEST
          xEventGroupClearBits( myEvents,  BIT_9_sendFile );
          xEventGroupClearBits( myEvents,  BIT_12_rdyFile );
          xEventGroupClearBits( myEvents,  BIT_13_doneFileSend );
          //xEventGroupClearBits( myEvents,  BIT_4_takeGps );
          xEventGroupClearBits( myEvents,  BIT_14_doneFileRead );

          // uxBits |=  BIT_14_doneFileRead; //quitado 05052022

          taskYIELD(); //¿?¿?

          xEventGroupClearBits( eventTaskState,  BIT_4_taskComm ); //ADDED WORKING

          vTaskDelete(Handle_CommTask);

        }
        t_send = 0;
        //        acciones += "Send ";
        DEBUG(">SET BIT_2_SendData");
        uxBits |= BIT_2_SendData; //activa senddata
        uxBits |= BIT_20_SendUbi;
        //uxBits |= BIT_4_takeGps;
#ifdef DEBUG_LOGEVENTS
        b_infoLog = B00000110; //TIME TO SEND GPS
        sendToEventsLogQueue(b_infoLog);
#endif

      }
      t_send++;
      t_measure++;
      DEBUG("T_measure: " + String( t_measure));


      if ((uxBitsStatus & BIT_12_rdyFile) != 0)
      {
        if (((uxBitsStatus & BIT_13_doneFileSend) == 0) && ((uxBitsStatus & BIT_14_doneFileRead) == 0))
        {
          uxBits |= BIT_9_sendFile;
          DEBUG ("SET BIT_9_sendFile<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
        }
        if ((uxBitsStatus & BIT_13_doneFileSend) != 0)
        {
          xEventGroupClearBits( myEvents,  BIT_9_sendFile );
          xEventGroupClearBits( myEvents,  BIT_13_doneFileSend );
          uxBits |=  BIT_14_doneFileRead;
          DEBUG ("CLEAR BIT_9_sendFile<------------");
          DEBUG ("CLEAR BIT_13_doneFileSend<--------");
          DEBUG ("SET BIT 14<-----------");
        }
      }
    }
    else
    {
      if ((uxBitsStatus & BIT_15_updateRdy) != 0)
      {
#ifdef RESET_WITH_RTC
        // setAlarmRTC();
        time_t t_actual = (time_t) rtc.getEpoch();
        setAlarmRTC(t_actual);

        _state = IDDLE;

        DEBUG(">Set BIT_11_iddle"); //TESTEO
        uxBits |= BIT_11_iddle;

        // if (!inTimeWindow(rtc.getEpoch()))
        if (!inTimeWindow(t_actual))
        {
          t_measure = 0;
          DEBUG("-------------------------------------------------------------->SET IDDLE STATE");
        }
#else
#ifdef DEBUG_LOGEVENTS
        DEBUG("-------------------------------------------------------------->TIMETOSEND"); //DELETE
        b_infoLog = B00000101; //TIME TO SEND (VECTOR HORAS)
        sendToEventsLogQueue(b_infoLog);
#endif
        _state = IDDLE;

        DEBUG("SEt BIT_11_iddle"); //TESTEO
        xEventGroupSetBits( myEvents, BIT_11_iddle );

#endif
      }
      else {
        uxBits |= BIT_7_updateTime;
      }
    }

    //        lastWakeTime = xTaskGetTickCount();
    SERIAL.println();
    SERIAL.print("FreeMem:");
    SERIAL.print(" TOT ");
    SERIAL.print(xPortGetFreeHeapSize());
    SERIAL.print(" PWR ");
    SERIAL.print(uxTaskGetStackHighWaterMark( Handle_pwrManagment ));
    SERIAL.print(" AnR ");
    SERIAL.print(uxTaskGetStackHighWaterMark( Handle_microAnaReadTask ));
    SERIAL.print(" Fil ");
    SERIAL.print(uxTaskGetStackHighWaterMark( Handle_fileTask ));
    //    SERIAL.print(" GPS ");
    //    SERIAL.print(uxTaskGetStackHighWaterMark( Handle_gpsTask ));
    SERIAL.print(" COM ");
    SERIAL.print(uxTaskGetStackHighWaterMark( Handle_CommTask ));
    SERIAL.print(" CTR ");
    SERIAL.print(uxTaskGetStackHighWaterMark( Handle_controlTask ));
    SERIAL.print(" TIME: ");
    SERIAL.print(lastWakeTime);
    SERIAL.print(" THREADS: "); //ADDED
    SERIAL.println(xEventGroupGetBits( eventTaskState ), (BIN)); //ADDED
    SERIAL.flush();

    if ((maxNumThread > uxTaskGetNumberOfTasks()) && ((xEventGroupGetBits( eventTaskState ) & BIT_4_taskComm ) == 0)) // ADDED MODIF WORKING
    {
      DEBUG("RE-Iniciando Com task");

      xTaskCreate(threadComm,           " Task Comm",     425, NULL, tskIDLE_PRIORITY + 2, &Handle_CommTask);
    }
    if ((maxNumThread > uxTaskGetNumberOfTasks()) && ((xEventGroupGetBits( eventTaskState ) & BIT_3_taskFile ) == 0)) // ADDED MODIF WORKING
    {
      DEBUG("RE-Iniciando File task");
      xTaskCreate(threadFile,           " Task File",    420, NULL, tskIDLE_PRIORITY + 2, &Handle_fileTask);
    }

    //    if ((eTaskGetState( Handle_fileTask ) == eSuspended) && ((uxBits & BIT_10_saveData) != 0) || ((uxBits & BIT_0_createFile) != 0))
    //    {
    // SERIAL.println("Res-File");
    // vTaskResume(Handle_fileTask);   ///!!!???
    //    }
    //    if ((eTaskGetState( Handle_microAnaReadTask ) == eSuspended) && (((uxBits & BIT_2_SendData) != 0) || ((uxBits & BIT_10_saveData) != 0)))
    //    {
    //      SERIAL.println("Res-Ana");
    //      vTaskResume(Handle_microAnaReadTask);
    //    }
    //    if ((eTaskGetState( Handle_CommTask ) == eSuspended) && (((uxBits & BIT_2_SendData) != 0) || ((uxBits & BIT_7_updateTime) != 0) || ((uxBits & BIT_9_sendFile) != 0)))
    //    {
    //      SERIAL.println("Res-Comm: ");
    //      //  Serial.println(eTaskGetState( Handle_CommTask));
    //      vTaskResume(Handle_CommTask);
    //    }
    //    if ((eTaskGetState( Handle_gpsTask ) == eSuspended) && ((uxBits & BIT_4_takeGps) != 0))
    //    {
    //      //DEBUG("Res-GPS");
    //      vTaskResume(Handle_gpsTask);
    //    }
    xEventGroupSetBits( myEvents,  uxBits);

    ////////////////////////////////////////////WAIT4//////////////////////////////////////
    if (((xEventGroupGetBits(myEvents) & BIT_11_iddle) != 0) && ((xEventGroupGetBits(myEvents) & BIT_9_sendFile) == 0)) {
      SERIAL.println(">ACTIVADO BIT_19_IddleYNoSend");
      xEventGroupSetBits(myEvents, BIT_19_IddleYNoSend);
    }
    else
      xEventGroupClearBits(myEvents, BIT_19_IddleYNoSend);
    ///////////////////////////////////////////////////////////////////////////////////////

    if (_state == SETUP) SERIAL.println("State SETUP");
    if (_state == MEASSURING) SERIAL.println("State MEASSURING");
    if (_state == IDDLE) SERIAL.println("State IDDLE");
    SERIAL.print( "STATE BITS: " ); SERIAL.println( uxBitsStatus, BIN );
    SERIAL.println("*******************");
    // wtd.clear();
    myDelayMsUntil(&lastWakeTime, SLOT_TIME);
    if (xPortGetFreeHeapSize() < 1800)
    {
      SERIAL.println("LOW MEMORY RESETING SYSTEM");
      powerOffComm(); //ADDED
      xEventGroupClearBits( eventPwr5volts, BIT_1_comModule | BIT_3_gpsModule); //ADDED
      xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr  );
      myDelayMs(3000);
      NVIC_SystemReset();
    }
  }

}





void setup()
{
  init_IO ();
  initRTC();
  SERIAL.begin(115200);
  // wtd.setup(WDT_TIME);
  // wtd.attachShutdown(ISR_WDT);
  vNopDelayMS(5000); // prevents usb driver crash on startup, do not omit this
  // while (!SERIAL) ;  // Wait for serial terminal to open port before starting program
  vCreateQueues( );
  createEvents();

  SERIAL.println("");
  SERIAL.println("******************************");
  SERIAL.println("        Program start         ");
  SERIAL.println(FILENAME);
  SERIAL.println(DEVICE);
  SERIAL.println("******************************");


  //while(true);
  vSetErrorLed(ERROR_LED_PIN, ERROR_LED_LIGHTUP_STATE);

  xTaskCreate(threadControl,             "Task Control",               120, NULL, tskIDLE_PRIORITY + 8, &Handle_controlTask);  //ADDED MODIF: ANTES 90   //ANTES 150
  // xTaskCreate(threadControl3,             "Task Control",               90, NULL, tskIDLE_PRIORITY + 8, &Handle_controlTask); //ADDED
  vTaskStartScheduler();
}

void loop()
{
}


void ISR_WDT() {

}
