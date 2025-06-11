#include <FreeRTOS_SAMD21.h>
#include <FlashStorage.h>
#include <DFRobot_HX711.h>

DFRobot_HX711 MyScale1(9, 8); //par de galgas1
float peso1 = 0.0;
DFRobot_HX711 MyScale2(5, 2); //par de galgas2
float peso2 = 0.0;

#include "Params.h"
#include "F_aux.h"
#include "events.h"


//**************************************************************************
// Type Defines and Constants
//**************************************************************************
#define LED
//#define  ERROR_LED_PIN  13 //Led Pin: Typical Arduino Board
//#define  ERROR_LED_PIN  2 //Led Pin: samd21 xplained board

//#define ERROR_LED_LIGHTUP_STATE  HIGH // the state that makes the led light up on your board, either low or high


SemaphoreHandle_t xMutex_SG1;
SemaphoreHandle_t xMutex_SG2;

//*****************************************************************
//Thread that set the calibration saved in paramStrain1 and after that measure every 100ms and save the value in "peso1"
//*****************************************************************
TaskHandle_t Handle_SG1Task;
static void threadSG1( void *pvParameters )
{
  //SERIAL.println("\n\nThread A: Started");
  TickType_t lastWakeTime1 = xTaskGetTickCount();
  if (paramStrain1.valid == true) {
    MyScale1.setOffset(paramStrain1.offset);
    MyScale1.setCalibration(paramStrain1.scale);
  }
  while (1) {
    if ( xMutex_SG1 != NULL )
    {
      if ( xSemaphoreTake( xMutex_SG1, 0) == pdTRUE )
      {
        taskENTER_CRITICAL();
        peso1 = MyScale1.readWeight();
        taskEXIT_CRITICAL();
        
        xSemaphoreGive( xMutex_SG1 );

        xEventGroupSetBits(xEventSend, BIT_0_SG1_READY);//marcamos la lectura de SG1 como hecha
        myDelayUsUntil(&lastWakeTime1, 12500);   // myDelayUs(12500);
      }
      else {
        SERIAL.println("SG1 sem timeout");
      }
    }

  }
}



//*****************************************************************
//Thread that set the calibration saved in paramStrain2 and after that measure every 100ms and save the value in "peso2"
//*****************************************************************
TaskHandle_t Handle_SG2Task;
static void threadSG2( void *pvParameters )
{
  //SERIAL.println("\n\nThread C: Started");
  TickType_t lastWakeTime2 = xTaskGetTickCount();
  if (paramStrain2.valid == true) {
    MyScale2.setOffset(paramStrain2.offset);
    MyScale2.setCalibration(paramStrain2.scale);
  }
  while (1) {
    if ( xMutex_SG2 != NULL )
    {
      
      if ( xSemaphoreTake( xMutex_SG2, 0) == pdTRUE )
      {
        taskENTER_CRITICAL();
        peso2 = MyScale2.readWeight();
        taskEXIT_CRITICAL();

        xSemaphoreGive( xMutex_SG2 );

        xEventGroupSetBits(xEventSend, BIT_1_SG2_READY);//marcamos la lectura de SG2 como hecha
        myDelayUsUntil(&lastWakeTime2, 12500); // myDelayUs(12500);
      }
    }
    else {
      SERIAL.println("SG2 sem timeout");
    }
  }
}



//*****************************************************************
// Thread that print in terminal and send by RS485 the result of measures with 3 decimals every 100ms:  peso1, peso2
//*****************************************************************
TaskHandle_t Handle_RS485TxMeasureTask;
static void threadRS485TxMeasure( void *pvParameters )
{
  //SERIAL.println("\n\nThread B: Started");

  unsigned long tiempoMedida = 0;
  char bufferAUX[128];  // Buffer para almacenar el valor convertido a string
  const EventBits_t uxBitsToWaitFor = BIT_0_SG1_READY | BIT_1_SG2_READY;
  
  //Se puede hacer pruebas en las que no se haga cambio de puerto serie (quitar debug) y ver si podemos quitar el control de flujo y dejarlo fuera del bucle
  //ya que no estoy seguro de que esto afecte a otras funcionalidades que no son el almacenamiento de la sesion
  digitalWrite(RS485_FLOW, HIGH);
  myDelayMs(5);


  TickType_t lastWakeTime = xTaskGetTickCount();
  
  while (1)
  {
    xEventGroupWaitBits(xEventSend, uxBitsToWaitFor, pdTRUE, pdTRUE, portMAX_DELAY); //limpia los bits una vez los recibe
    
    if ( xSemaphoreTake( xMutex_SG1,  pdMS_TO_TICKS(12) ) == pdTRUE ) {
      if ( xSemaphoreTake( xMutex_SG2,  pdMS_TO_TICKS(12) ) == pdTRUE ) { //si se han tomado ambos semaforos

        tiempoMedida = millis();
        SERIAL.print(tiempoMedida);
        SERIAL.write(',');
        SERIAL.print(peso1, 3);
        SERIAL.write(',');
        SERIAL.print(peso2, 3);
        SERIAL.println();

        SERIAL1.print(tiempoMedida);
        SERIAL1.write(',');
        SERIAL1.print(peso1, 3);
        SERIAL1.write(',');
        SERIAL1.print(peso2, 3);
        SERIAL1.write('\n'); //mejor que println == \r\n

        xSemaphoreGive( xMutex_SG1 );
        xSemaphoreGive( xMutex_SG2 );

        myDelayUsUntil(&lastWakeTime, 12500); //cedemos el tiempo de ejecución para que las otras tareas puedan tomar el semaforo
      }
      else{ // si no se ha tomado el semaforo de sg2
         xSemaphoreGive(xMutex_SG1);
      }
    }

  } 
}



//*****************************************************************
// Thread that read Serial1 (RS485) to remote config
//*****************************************************************
/*TaskHandle_t Handle_dTask;
  static void threadD(void *pvParameters) {
  while (1) {
    if (SERIAL1.available()) {
      SERIAL.println("Message received");
      const char* rxRS485 = SERIAL1.readStringUntil('\n').c_str();
      SERIAL.print(">>>"); SERIAL.println(rxRS485);
      if (((String)rxRS485).equals(COMMAND_CONFIG)) {
        SERIAL.println("COMMANDCONFIG");
      }
    }
  }
  }*/

//*****************************************************************

void setup()
{
  SERIAL.begin(SERIAL_BAUD_RATE);
  SERIAL1.begin(SERIAL1_BAUD_RATE);

  pinMode(RS485_FLOW, OUTPUT);
  digitalWrite(RS485_FLOW, LOW);

  pinMode(RATE_SEL, OUTPUT);
  digitalWrite(RATE_SEL, RATE_SEL_STATUS);

  delay(7000); // prevents usb driver crash on startup, do not omit this 5000
  //while (!SERIAL) ;  // Wait for serial terminal to open port before starting program

  FlashStorage(myFlashST1, paramST);
  FlashStorage(myFlashST2, paramST);

  //REINICIO DE VALORES NVM
  //________________________________________
  /*struct paramST paramST_RESET;
    paramST_RESET.valid = false;
    paramST_RESET.offset = 0;
    paramST_RESET.scale = 0.0;
    paramST_RESET.ID = 0;
    myFlashST1.write(paramST_RESET);
    myFlashST2.write(paramST_RESET);
    while (1);*/
  //________________________________________

  if ((myFlashST1.read().ID != 0 ) || (myFlashST2.read().ID != 0) ) { //Si ID1 y ID2 == 0 nunca se ha configurado, NO ENTRA
    SERIAL.println("reading values");
    paramStrain1 = myFlashST1.read(); //tomamos valores almacenados en "nvm" y los asignamos a las estructuras de datos de las galgas
    paramStrain2 = myFlashST2.read();
  }
  else {
    SERIAL.println("ID not seted first Time");
  }
  //Serial.println( paramStrain1.ID);
  //Serial.println( paramStrain2.ID);

  paramStrain1.ID = 1; //NO MODIFICAR INDICES: 1 y 2, SE EMPLEAN EN DATALOGGER PARA IDENTIFICAR PROCESOS
  paramStrain2.ID = 2;

  SERIAL.println("");
  SERIAL.println("******************************");
  SERIAL.println("        Program start         ");
  SERIAL.print(FILENAME);
  SERIAL.print("\t|\t");
  SERIAL.println(VERSION);
  SERIAL.println(DEVICE_NAME);
  SERIAL.println("******************************");
  SERIAL.flush();
  SERIAL.println("Stored values:");
  SERIAL.print("offset1:"); SERIAL.println(paramStrain1.offset);
  SERIAL.print("scale1:"); SERIAL.println(paramStrain1.scale);
  SERIAL.print("valid1:"); SERIAL.println(paramStrain1.valid);
  SERIAL.print("ID1:"); SERIAL.println(paramStrain1.ID);
  SERIAL.print("\noffset2:"); SERIAL.println(paramStrain2.offset);
  SERIAL.print("scale2:"); SERIAL.println(paramStrain2.scale);
  SERIAL.print("valid2:"); SERIAL.println(paramStrain2.valid);
  SERIAL.print("ID2:"); SERIAL.println(paramStrain2.ID);


  if (( paramStrain1.valid == false) || ( paramStrain2.valid == false)) { //si no se ha calibrado nunca o ha fallado en la última calibración
    SERIAL.println("calibration validation flag on gauge is FALSE, NEED FIRST CALIBRATION");
  }
  else {
    SERIAL.println("\n\nInit Meassuring, press any key to cancel and calibrate");
  }

  commandResponseRS485("ready for commands"); //envia mediante command response por si datalogger quiere hacer configuración para avisarle que está listo

  unsigned long timenowConfig = millis();
  //uint8_t counter = 0; //contador para enviar mensaje ready RS485
  int res = -1; //response de el calibration gauge 0: correcto, -1:error
  while ((millis() - timenowConfig) < CONFIG_TIME) //MIENTRAS SE ENCUENTRE DENTRO DE CONFIG_TIME ESPERA QUE EL USUARIO INTRODUZCA UN COMANDO POR PUERTO SERIE O RS485
  {
    Serial.print(".");

    if (SERIAL1.available() != 0) { //Si ha llegado algo por RS485
      SERIAL.println(">>COMM RS485 received");
      char c_ = 0;
      String rxRS485 = "";

      while (c_ != '\n') {
        c_ = SERIAL1.read(); //espera leer comando de config
        rxRS485 += c_;
      }
      SERIAL.print("[RS485_Rx]: "); SERIAL.println(rxRS485);

      if (check_cmd(rxRS485.c_str(), res) == ADV_ERROR) { //si el return es default devuelve error
        SERIAL.println("Unknown command");
        commandResponseRS485("Unknown command, try again");
        timenowConfig = millis(); //reiniciamos el contador para dar tiempo
      }
    }

    else if (SERIAL.available()) //Si se envia alguna tecla se borran los validadores de calibracion
    {
      SERIAL.readStringUntil('\n');
      paramStrain1.valid = false;
      paramStrain2.valid = false;
    }
    /*else if (counter == 10) { //cada segundo envía ready for commands
      counter == 0;
      commandResponseRS485("ready for commands");
      }*/

    //aquí NO ENTRA si se hace con comando
    if (( paramStrain1.valid == false) || ( paramStrain2.valid == false)) //En el caso de que la galga no esté calibrada previamente o se haya llamado al comando, llama a la función de calibración
    {
      SERIAL.println("Starting gauge calibration");
      res = gaugeCalibration();

      break; //sale del while de configuración
    }
    delay(100); //para que no imprima puntos infinitos
    //counter++;
  }

  if (res == -1)
  {
    SERIAL.println("\nCalibration Proccess Fail");
  }
  else
  {
    SERIAL.println("Calibration Proccess Succesfull");
    if ( paramStrain1.valid == true)
    {
      SERIAL.println("\nST1 stored");
      myFlashST1.write(paramStrain1); //guardamos los valores en nvm
    }
    if ( paramStrain2.valid == true)
    {
      SERIAL.println("ST2 stored\n");
      myFlashST2.write(paramStrain2);//guardamos los valores en nvm
    }
  }

  commandResponseRS485("Out of config mode");

  SERIAL.print("\n[SG] Out of config mode\n");

  MyScale1.setOffset(paramStrain1.offset);
  MyScale2.setOffset(paramStrain2.offset);
  MyScale1.setCalibration(paramStrain1.scale);
  MyScale2.setCalibration(paramStrain2.scale);

  vSetErrorSerial(&SERIAL);

  //TEST
  struct paramST testStruct1, testStruct2;

  testStruct1 = myFlashST1.read();
  testStruct2 =  myFlashST2.read();

  SERIAL.print(testStruct1.valid); SERIAL.print("\t"); SERIAL.print(testStruct1.offset); SERIAL.print("\t"); SERIAL.print(testStruct1.scale); SERIAL.print("\t"); SERIAL.println(testStruct1.ID);
  SERIAL.print(testStruct2.valid); SERIAL.print("\t"); SERIAL.print(testStruct2.offset); SERIAL.print("\t"); SERIAL.print(testStruct2.scale); SERIAL.print("\t"); SERIAL.println(testStruct2.ID);


  xMutex_SG1 = xSemaphoreCreateMutex();
  xMutex_SG2 = xSemaphoreCreateMutex();

  if (xMutex_SG1 == NULL || xMutex_SG2 == NULL) {
    SERIAL.println("Error creando mutex");
    while (1);
  }
  
  createEvents();

  // Create the threads that will be managed by the rtos
  // Sets the stack size and priority of each task
  // Also initializes a handler pointer to each task, which are important to communicate with and retrieve info from tasks
  xTaskCreate(threadSG1,     "Task SG1",       320, NULL, tskIDLE_PRIORITY + 2, &Handle_SG1Task); //calibration saved in paramStrain1 and after that measure every 100ms
  xTaskCreate(threadSG2,     "Task SG2",       256, NULL, tskIDLE_PRIORITY + 2, &Handle_SG2Task); //calibration saved in paramStrain1 and after that measure every 100ms
  xTaskCreate(threadRS485TxMeasure,     "Task RS485TxMeasure",       512, NULL, tskIDLE_PRIORITY + 3, &Handle_RS485TxMeasureTask); //print in terminal and send by RS485 the result of measures every 100ms
  //xTaskCreate(threadD,     "Task D",       256, NULL, tskIDLE_PRIORITY, &Handle_dTask); //listening rs485 to config command
  //xTaskCreate(threadCalib,     "Task Calib",       256, NULL, tskIDLE_PRIORITY + 3, &Handle_calibTask);
  // Start the RTOS, this function will never return and will schedule the tasks.
  vTaskStartScheduler();

  // error scheduler failed to start should never get here
  while (1)
  {
    SERIAL.println("Scheduler Failed! \n");
    SERIAL.flush();
    delay(1000);
  }

}

//*****************************************************************
// This is now the rtos idle loop
// No rtos blocking functions allowed!
//*****************************************************************
void loop()
{
  // Optional commands, can comment/uncomment below

}
