/* test
 * "CODIGO RAPIDO" GENERADO A PARTIR DE UNIFIC_v0.2.4_ANALIZPORTATIL
 * 
 *  * //configuracion de ESP32 para programar
 * Modulo ESP32S3 dev module
 * Partition scheme: 16MB flash 3mb APP
 * Flash size = 16MB
 * CDC on boot disabled
 * para ver los logs habilitar core debug level info
 * 
 * 
 * 18/12/2024
 * Corregido error RTC no fijaba bien la fecha, era porque se estba metiendo el año con dos cifras en vez de 4 (error al pasar de samd21 a esp32)
 * SE LE HA QUITADO EL OFFSET DE GMT
 * 
 */
