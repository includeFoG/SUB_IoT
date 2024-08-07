/*NOTES:
 * -El primer envío del dispositivo no es un dato válido, se mantiene el envío para facilitar la comprobación de las comunicaciones, pero los sensores aún no se encuentran preparados para
 * realizar una medición, cuando se haya completado un vector de medidas NUM_MEASURES_V, se enviará el primer valor
 * El primer valor de temperatura y humedad son una primera medida individual (sin media)  antes de encender el calentador
 * Del mismo modo CO2 y TVOC se inicializan en 0 y 0 respectivamente (CO2 <400 nos indica que el dispositivo se ha reiniciado)
 * -El calentador se enciende tras la primera medida al iniciar el dispositivo.
 * 
 * TODO:
 * Ver opciones para introducir calentador cuando la humedad se encuentra por encima del 90% o por debajo del 10%
 * Meter control de offset por diferencia temperatura interna externa
 * -ver sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));
 * Gestion del calentador activar 40 min 1 vez al día (?)
 * Meter Pila
 * Meter pm2.5, pm10
 * meter pwm al ventilador -> NO TIENE PWM ESE PIN (2, 3, 5, 6, 9, 10, 11, 12, 16 / A2, 17 / A3, 19 / A5)
 * 
 * 
 * 14/06/2023 THCO2[V.0.0.3]
 * -Añadido cambio de frecuencia cuando el nivel de CO2 se encuentre por encima del CHANGE_FREQ_CO2_UMBRAL de PRINCIPAL_FREQ a SECONDARY_FREQ
 * -Ahora el ventilador se enciende solo durante las NUM_MEASURES_FAN últimas medidas (NUM_MEASURES_FAN segundos) de un vector de datos (NUM_MEASURES_V datos), 
 * suficiente para renovar el aire en el interior del dispositivo
 * -Corregido error en el que en la lectura de temperatura y humedad si no se había adquirido el dato del sensor y el índice era 0 introducía un 0 en el vector de medidas
 * -Corregido error en el que si el SGP30 no tomaba un dato válido este se introducía igualmente en el vector de mediciones y hacía medias erroneas
 * 
 * 
 * 
 * 
 * 12/06/2023 THCO2[V.0.0.2]
 * -Cambios en heater ahora se activa cada 30 min o cuando la humedad se encuentra por debajo del 10% o por encima del 90%
 * -Añadido vector circular para media de TEMP,HUM,CO2,TVOC
 * -Ahora la activación del calentador no para el programa, cuando este esté activo no se tomarán medidas de temperatura ni humedad, 
 * cuando se apague tras 30 seg de encendido se tomarán medidas de humedad y una vez haya pasado 1 min desde el apagado se tomarán temperaturas
 * 
 * 
 * 02/06/2023 THCO2[V.0.0.1]
 * Añadido uso de ECCX08 para comunicaciones cifradas con AWS cloud ATECC508A
 * Habilitado DHCP
 * La dirección a la que responde el ECCX08 una vez está lockeado es 0x32
 * -> La adición de AWS presenta problemas a la hora de tomar lecturas del sensor de SGP30, se está desbordando la memoria.
 *    NO ERA DESBORDAMIENTO DE MEMORIA, existía algún conflicto con el bus SPI, al hacer un Wire.end() en cada loop se ha solucionado el problema
 * 
 * 
 * 
 * 
 * 
 * 
 * ////////////////////////////////////////////////////////////////////////////////////////////////////////////
 * 17/10/2022 [v01.1.0]
 * Actualizado firmware MEETS para que cuenten con sensor de temperatura/humedad AM2320
 * 
 * [TEST_X1_GP_V0]
 * Programa preparado para placas SAMD21 MINI BREAKOUT y ARDUINO nano33
 * Configurado con MIKROE ETH WIZ CLICK
 * Preparado para sensor AM2320, SGP30(nano33-1.8.11),SHT30(samd21-1.6.21)
 */



 /*
  * PARA PODER USAR ESTE PROGRAMA:
  * Es necesario cambiar de Arduino SAMD Boards de la versión 1.6.21 a la versión 1.8.11
  * PARA MEETS:
  *   -Configurar IP y sensores
  *   -En oficina dejar DHCP
  *   -Mide CO2 y VOC con SGP30 y temp/hum con SHT30
  *   
  *   
  * PARA CPD:
  *   -Configurar MAC
  */



  /*
   * CONEXION LED CATODO COMUN:
   *             1234
   *             ||||
   *              |
   *             R-GB
   *             
   * CONEXION LED ANODO COMUN:
   *             1234
   *             ||||
   *              |
   *             R+BG
   *             
   *           
   */


   /*ERRORES:
    * 
    * El sensor SGP30 empieza a medir el CO2 desbordándose.
    * Aparentemente es un tema de memoria, salta con AWS_CLOUD en la función de mqtt.connect
    * Se ha probado el programa sin AWS_CLOUD y va bien, se ha probado con AWS_CLOUD y sin certificado (para disminuir peso de código) y tampoco se han conseguido resultados
    * Mismo programa poniendo puerto correcto NO FUNCIONA SENSOR, con puerto incorrecto FUNCIONA TODO
    * 
    * */
    */
