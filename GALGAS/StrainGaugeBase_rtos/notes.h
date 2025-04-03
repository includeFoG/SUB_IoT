/*_______________________________________INFORMACION DE LIBRERIAS_______________________________________

      FreeRTOS_SAMD21, version: 2.3.0    -> el nombre ha cambiado respecto a la versión 1.0.0 usada en el mainproyect
      YA NO ES NECESARIA LA LINEA DEL PLATFORM PARA FREERTOS
      DFRobot_HX711,   version: 1.0.0
      
      board:
      FlashStorage,    version: 0.5.0
*/

/*_______________________________________INFORMACION DE RS485_______________________________________
 *            FLUJO:              |        CARACTER DE FIN:
 *     DATALOGGER -> SG (Rx)      |             '\n'
 *  MEASURE SG -> DATALOGER (Tx)  |             '\n'         
 *  CONFIG MESSAGE SG-> DATALOGER |             ']'          -> ya que el dispositivo DL si detecta '\n' piensa que ha empezado a enviar medidas 
 */

/*
 * _______________________________________INFORMACION DE HILOS_______________________________________
 * THREAD SG1 SE ENCARGA DE SETEAR LA CALIBRACION DE LA GALGA 1 Y POSTERIORMENTE TOMAR MEDIDAS CADA 100MS Y LAS ALMACENA EN PESO1
 * THREAD SG2 SE ENCARGA DE SETEAR LA CALIBRACION DE LA GALGA 2 Y POSTERIORMENTE TOMAR MEDIDAS CADA 100MS Y LAS ALMACENA EN PESO2
 * THREAD RS485TxMeasure ENVIA E IMPRIME POR PANTALLA PESO1 y PESO2  
 * 
 * _______________________________________INFORMACION DE PLACA_______________________________________
 * En la version STG-002 de la placa se tiene que hacer un cambio de Rx y Rx en el RS-485 y poner jumper con CLK2
 * 
 * Se esta empleando un integrado RS485 para enviar mensajes de forma tansparente en el modulo
 * 
 * 
 */

/*  [v.0.0.5] 02/04/2025
 *  - Parece que al bajar por debajo de 13ms tenemos probloemas al leer las galgas si utilizamos en las 3 tareas delayuntil. dejando uno solo va bien.
 *  - Se ha metido un mutex para el acceso de los datos entre tareas.
 *   
 *   [v.0.0.4] 02/02/2024
 * - Modificado "Y" en buffer recepción después de "ok" para que el dispositivo espere a que el usuario esté listo antes de calcular el offset
 * 
 *   [v.0.0.3] 01/02/2024
 * - Problema solucionado: No se estban almacenando los valores nvm después de una calibración mediante BLE
 * - Corregidos problemas con el parámetro ID
 * - Ahora la galga cuando se configura desde BLE no tarda tanto en empezar a enviar los valores
 *  
 *  [v.0.0.2] 03/01/2024
 *  - Añadida lectura de comandos mediante RS485 al inicio
 *  - gaugeCalibration modificada para remote config, Calibración de la galga habilitada dede BLE->RS485
 *  - Se ha añadido caracter inicial "[" y final "]" para comunicaciones de configuracion mediante RS485
 *  - Modificado offset y scale para que coincidan con el tipo de dato de la librería
 *  - Ahora al hacer la calibración se asigna setOffset y setCalibration 
 *  - Ahora se tiene en cuenta un offset 0 para tomar la medida que hace el cálculo de la pendiente y posteriormente se asigna el offset para calcular la pendiente
 * 
 * 
 * [v.0.0.1] 12/12/2023 
 * - Toma de contacto con el código, pequeños cambios para facilitar lectura
 * - Modificacion en caracter de fin de dato de println()=\r\n a print('\n')
 * - Añadido tiempoMedida al envío de datos
 */
