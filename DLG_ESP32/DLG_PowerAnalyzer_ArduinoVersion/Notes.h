/*Descripción del código:
  *   Este código permite tomar lecturas de analizadores de redes Schneider PM2200 y SIEMEN0 PAC4200
  *   Toma los datos de los registros indicados y prepara un mensaje en formato JSON para subirlo a un broker MQTT
  * 
*/


/*Programa para: SSM-DLG-001
 *    Módulo ESP32-S3-WROOM-1U (dev Module en arduino)
 *        Partition scheme: 16MB flash 3mb APP
 *        Flash size = 16MB
 *        USB CDC on boot disabled
 *        
 *        para ver logs y habilitar core debug level info

 
 _______________________________________INFORMACION DE LIBRERIAS_______________________________________
 * 
 * https://espressif.github.io/arduino-esp32/package_esp32_index.json
 * board ESP32 driver version: 3.0.5  C++20
 * 
 * shrPrta modificada para <<singleton>>
*/



  /* La placa tiene dos pines de selección 19 y 20 que permiten gestionar un multiplexor de una uart: SN74CBTLV
   *  En este selector se encuentra:
   *  UART0 -> ESP32 "UART-IN" SN74CBTLV
   *    UART1 -> MIKROE J2 (MIKROE2)
   *    UART2 -> RS232 channel1 U2Rx(R1Out) U2Tx(T1In) 
   *    UART3 -> RS232 channel2 U3Rx(R2Out) U3Tx(T2In)
   *    UART4 -> RS485
   *  
   *  MIKROE1 tiene como uart:
   *    UMRX pin 24
   *    UMTX pin 25
   *  
   * 
   */

  /*PENDIENTE:
   *  -Gestionar la destrucción de objetos
   * 
   * 
   */
