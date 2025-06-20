/* 
 *  VERSION DE FREERTOS: Arduino-FreeRTOS-SAMD21-master, version=1.0.0
 *  
 *   *->PROBLEMAS EN VERSION RELEASE DEV_V046_2GPS_F:
 *   Tras el primer día de funcionamiento se observa que en el periodo de medición de las 2:00am el dispositivo envía el último mensaje anterior a la medida a la 1:38am con estatus 0
 *   y retoma los envíos a las 2:17am con valor de status 0, el siguiente mensaje se produce a las 2:24am, lo que hace pensar que el envío de las 17 puede encontrarse retrasado por  
 *   la toma de GPS (a las 2:17am se envía correctamente el GPS) Se observa que en FILEZILLA el primer archivo pertenece a las 6:00am, por lo que la sesión no se ha subido (ni el txt
 *   correspondiente) POSIBLES CAUSAS:
 *            -Tras el reinicio ha tardado mucho en llegar al punto que determina si pasa a meassuring, pasando el timeWindow (10min) lo que puede hacer que no tome la medida.  
 *   el error venía por una mala transformación del dato a float eliminaba el 0 a la izquierda después de la coma
 *  
 *  
 *  
 *  
 *  
 *   * ->CON LA NUEVA VERSION DE UMNOPROF PUEDEN HABER RETRASOS SI SE EJECUTA EL ISNETWORKCONNECTED 2 VECES SEGUIDAS ANTES DE HACER ENVIO
 *  
 *   *      -EL HILO DE COMUNICACIONES CUANDO VIENE DE UN RESET Y ESTA MIDIENDO EN OCASIONES NO ENVIA. NO SE BLOQUEA, SE DEBE A LOS TIMEOUTS DE ALGUNOS COMANDOS AT COMO UPSDA
 *          QUE DURAN CASI 3 MINUTOS Y AL ESTAR EN UN WHILE EL SERVERCONNECT, PASA MÁS DE 9 MINUTOS AHÍ DENTRO RETRASANDO TODO EL HILO Y PUDIENDO LLEGAR AL SIGUIENTE ENVÍO
 *          HACIENDO EL PRINT DEL COMM NO RESPONDE, PERO ESTE HILO (COM) NO SE ENCUENTRA REALMENTE BLOQUEADO. SE OBSERVA ADEMAS QUE SE PUEDE PRODUCIR UN RETRASO EN EL TIEMPO 
 *          DE MEDIDA, YA QUE AFECTA A EL HILO DE CONTROL RETRASANDO EL LOOP DEL HILO QUE PASA DE 1 MINUTO A 2 MINUTOS EN ALGUNAS OCASIONES. PARA RESOLVERLO SE HA DE ACTUAR
 *          SOBRE LA CONFIGURACION DEL MODULO. CREO QUE EL RETRASO SE DEBE A QUE CUANDO EMPIEZA A MEDIR PASA MÁS TIEMPO HASTA QUE SE REALIZA LA CONEXION DEL MODULO DE COM
 *          POR ESO SOLAMENTE TIENE LUGAR EL FALLO DESPUES DE UN REINICIO. SE PLANTEAN LAS SIGUIENTES POSIBLES SOLUCIONES: 
 *                        1.PONER UN POWEROFFCOM ANTES DEL POWEONCOMM DE BIT_2_SENDDATA EN EL HILO DE COMUNICACIONES.  -> NO TIENE EL RESULTADO ESPERADO
 *                        2.ELIMINAR  EL ISNETWORKCONNECT PRINCIPAL DEL STARTCOMPORT PARA QUE SIEMPRE REINICIE EL MODEM. -> NO TIENE EL RESULTADO ESPERADO
 *          EL OBJETIVO DE ESTAS ACCIONES ES QUE EL MÓDULO LLEGUE SIEMPRE EN LAS MISMAS CONDICIONES AL PUNTO DE CONEXIÓN DE UBIDOTS
 *          **********************************************VER REALENTIZACIÓN DE TICK***********************************************
 *          
 *    *     -EL TICK SE REALENTIZA AL ENTRAR EN MEASSURING???
*            Se ha probado a quitar los priority del hilo com a ver si era esto lo que le afectaba -> NO ESTA AQUI EL PROBLEMA
 *           -> SE DEBE A LAS SECCIONES CRITICAS EN EL HILO DE FILE retraso aprox 7 seg si la SD está mal aumenta hasta 2min esto hace que falle totalmente el envío en 
 *             tiempo de medida. Si la tarjeta está bien al tardar únicamente 7 seg puede que no se produzca la sección crítica en medio de la medida o que ese tiempo 
 *             sea lo suficientemente corto como para no afectar al envío, pero la realidad es que el tiempo se ha dilatado y las medidas durante ese periodo no se producen
 *             cada 100ms. Por lo que nos encontramos en el punto en el que podemos eliminar las secciones críticas y asegurar el envío en periodos de medida y las mediciones
 *             cada 100ms pero no el correcto guardado en la SD pudiendo dar lugar a fallos o dejamos las secciones críticas garantizando el guardado en la SD de datos, sabiendo
 *             que estos cuando coincidan con la ejecución de la sección crítica no se están tomando cada 100ms y afectando ocasionalmente al envío.
 *  
 *  
 *   *  TO DO LIST:
 *  -watchdog
 *  -quitar debug librería
 *  -reducir tiempo de conexion
 *  -reducir al maximo tiempo de gps
 *  
 *  
 *  SE OBSERVA ERROR EN GPS :
 *    -Al iniciar el sistema fuera de tiempo de medida, si el dispositivo no es capaz de coger el gps por ejemplo al estar dentro de la oficina, hace un primer intento y después se bloquea (sin repetir el bucle:
 *    while (!(takeGPS(&localPosition) == 0) && (120000 > millis() - timeout_gps));
 *    Se observa que con el mismo programa si al iniciarse se encuentra en un estado de medida (coincidencia de horas de medida) no se produce este error.
 *    
 *    
 *  [v047.0.3_DERIVA_GPS] 15/02/2024
 *  En la version 47.0.2 se detectó un problema cuando se encendían las cámaras de forma manual desde ubidots. Este problema hacía que en los siguientes envíos a ubidots la alimentación de los sensores estuviese desconectada
 *  al estar el PCF:  10000011111001100 INCORRECTO en lugar de 11100011001000010. 
 *  No se ha encontrado el origen del fallo aunque se ve que es entre un ANA:POWER SENSOR y un Set Mikro
 *  El problema se podía deber a que la variable uxBits_PCF era global y en un punto en concreto de taskPWrMagnament se le hacía >>7 lo que podía afectar si se hacía un salto a otro punto del programa al quitarle parte de los bits
 *  se ha solucionado empleando una variable local en taskpwrmgnament
 *    
 *  [v047.0.2_DERIVA_GPS] 03/11/2023
 *  Modificada estructura qPosition para añadir HDOP
 *  Se observa que el sistema se queda sin memoria justo después de terminar de subir un archivo y creo que antes de eliminarlo de la lista.
 *    
 *  [v047.0.1_F_RELEASE_PRETALIARTE] 03/11/2023
 *  Debido a errores en el mar relacionados con deriva de GPS se ha modificado la librería del sara R5 para habilitar más sistemas GNSS pasando de UGPS = 1,0,1 a UGPS = 1,0,67
 *  También se está teniendo en cuenta la calidad del fixeo mediante el valor HDOP de la trama GGGA NMEA, este valor de forma ideal ha de estar entre 1 y 2, actualmente lo tenemos
 *  filtrado por debajo de 4.5
 *  se le han sumado 30 segundos al tiempo de cálculo de GPS
 *  
 *  [v047.0.1_F] 27/08/2022
 *  Corrigegido error en la toma de tiempo, se limpiaba BIT_7_updateTime aunque no se obtuviese la respuesta correcta del módulo
 *  Aumentado el tiempo entre intentos de taketime() y aumentado en 1 el número de reintentos
 *  Añadida variable retrys en  if ((uxBits & BIT_7_updateTime) != 0) para reiniciar el módulo cada 2 reintentos
 *  28/09/2022
 *  Debido a que en ocasiones el buzzer no se desconecta se han añadido algunos   shiffty.clear(EN_SERVO); en el hilo de control de power
 *   
 *  [v047_F] 12/08/2022
 *  Corregido encendido DVR (antes solo omitia la sesión de las 22h ahora no enciende dvr entre las 22 y las 4am
 *    
 *  [v047_F] 07/07/2022
 *  Ahora setAlarmRTC() e intimewindow trabajan con el mismo tiempo (solo se calcula 1 vez, lo que evita conflictos si se reinicia el equipo muy cerca de los puntos criticos (GAP_TIME y timereset))
 *  Encontado error entre setalarm y getnexthour, si se produce un reset y es la 1:56am la alarma se fijaba a las 2:00am pero entraba en periodo de medida. se ha solucionado aumentando en 1 idx del vector de tiempo cuando se encuentra dentro del gaptime
 *  (por debajo de la hora) de este modo se realiza el periodo de medida y la alarma se fija a las 6:00am
 *  Modificados algunos tiempos con la etiqueta OCT MODIF para reducir tiempos de conexión 
 *  Modificado updateTime(); ahora no se repite sin necesidad
 *  Quitado 1 minuto de delay despues de poweroff1 -> bajado a 10seg
 *  Aumentado el GAP_TIME por delante de la hora de 10 a 15 min para asegurar medición si se producen retrasos en la conexión
 *  [LIBRERIA R5]
 *  Aumentado tiempo de respuesta UGGGA de 10 seg a 15 para evitar error 2
 *  Añadido enablegps() a los casos en que el gps respondia que no estaba activado
 *  Modificada librería R5 modemConnect (upsda) ( EN LA VERSION DE LIBRERIA PRUEBAS SUPERSIM)
 *  
 *    
 *  [v046_2_GPS]
 *  Quitadas variables estaticas de takeGPS()
 *  Quitado static de root buildPut()
 *  
 *   * ->CORREGIDO! REVISAR PRINT DE RESPUESTA DE UBIDOTS SOLO FUNCIONA LA PRIMERA VEZ    
 *    cambiado orden de asinación y de impresión
 *    variable char c pasada a static
 *    se le quita static al string de response
 *    se le añade client.flush() antes de enviar el payload de gettcp
 *  
 * 19/06 [v046_2]
 * Modificada libreria R5 UPSD y UPSDA a veces hacían referencia al perfil 0 y otras al perfil 1
 * Añadido ,4 en substring respuesta de gettcpresponse para limitar el substring de respuesta 
 * Modificados parentesis de gestión BIT ubidot en hilo power -> IMPORTANTE
 * Se le quita static al string de response en posttcptoubi
 * Se le añade un clientavailable con timeout por si se reciben más caracteres y client.flush() despues de leer respuesta en posttcptoubi() y gettcpfromubi()
 * Quitado BIT_13_S6_RS1 de comprobación BIT_23_UBIFLAG_ON ¿NO TENIA SENTIDO?
 * Quitada sección crítica en el guardado de archivos etiquetado como // NOCRITICALSEC en taskfile
 * Bajado RETRY_CONNECTION de 3 a 2
 * Modificada acción en  BIT_7_updateTime para que apague el módulo después de tomar la hora
 * 
 *  
 *  15/06 [v046] 
 *  Arreglado vector de valores
 *  Añadido xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr  ); en taskpwr después de apagado placa actu
 *  Modificadas resistencias de cálculo de mapVolt()
 *  Modificado encendido placa analogicos en hilo power (ahora la placa se enciende después de encender la lógica y enviar la palabra a PCF) antes encendia placa
 *  al encender la lógica, antes de enviar la palabra
 *  Modificado encendido de sensores y DVR con filtrado de día y noche
 *  Añadido apagado total si ocurre "COM: Fail connecting to the server" antes solo apagaba el módulo con comando
 *  Corregida máscara de PCF placa actuación en hilo power (faltaba el bit de la salida G)
 *  
 *  
 *  14/06 [v046] CAMBIADA LIBRERIA DE SARA R5
 *  Quitados algunos isnetworkconnected():
 *      ·En el do while() de postTCPToUbidots
 *      ·Dentro de postTCPToUbidots
 *      ·En filesend() junto con las 3 lineas siguientes de power off3
 *  Modificado el bucle for (byte i = 0 ; i < NUM_VALORES ; i++) que determinaba el guardado de las señales de los sensores de la placa de analógicos
 *  
 *  06/06 [v045-2]         -> ULTIMA VERSION CARGADA EN PLACA DE PRUEBAS WP CON LIBRERIA SARAR5 ANTERIOR A LA ACTUAL
 *  Modificados bits de placa de actuación para sensores
 *  Añadido gprsdisconnect(); en startComPort()
 * 
 * 04/06[v045]
 * Camiados delay() de libreria R5 por myDelayMs()
 * añadido bool err_send
 * Modificados client.stop para que solo se produzca una vez en conexion ubidots
 * OJO: Añadido client.stop en caso de que no haga conexion (retrys)
 * Bajado tiempo de respuesta de USOCO de 120s a 60s en serverConnect() (si todo va bien tarda 2s)
 * Camiada forma de activar buzzer a bits de eventTaskState
 * Añadido delay de 5s en antes de creacion hilos para que el modem de comunicaciones esté un tiempo apagado después del reset
 * Quitado res-file por wait4
 * 
 * 03/06[V044-2]
 * Añadido timeout en STOP() TINYGSM
 * Para implementación de buzzer:
 *      añadido delay 4s en el setup del hilo de control para permitir buzzer de inicio sin interferir en funcionamiento
 *      añadida funcion if ((status & (BIT_5_buzzer)) != 0) in [PWR]
 *      añadida activación de BIT_5_buzzer después de  if ( t_measure >= MEASURE_PERIOD) [CTRL] si se encuentra activo BIT_10_saveData
 * 
 * 
 * 02/06[v044-2]
 * Modificados algunos powerOffComm() por  powerOffComPort()
 * Quitado client.stop al conectar serverConnect()
 * Quitados los gprsdisconnect() de sendfile() 
 * Movido gprsdisconnect() de BIT_2_SendData en [COM] a BIT_11_Iddle
 * Cambiado GAP_TIME de 5 min a 10 min al no darle tiempo a llegar a intimewindow() dentro de la ventana de tiempo después de hacer un reinicio
 * Añadido -1 a if(resp.indexOf("operation not allowed")==-1){ en librería R5
 * Modificado getNextHour() se ha añadido la condición de que el vector de horas tiene que terminar en 24 para no acceder a un espacio de memoria fuera del vector
 * 
 * 01/06[v044-2]
 * Se quita gprsdisconnect despues de envio ubidots -> se pasa a después de envio aws
 * Cambiado tamaño de memoria de hilos
 * Añadido en libreria R5 +CGATT? en gprsconnect() antes de hacer gprsDisconnect()
 * en powerOnComm() se ha quitado != en el while
 * Añadida comprobación de BIT_23_UBIFLAG_ON (estaba mal) en taskpwrmanager
 * Modificada funcion powerOffCom() para comprobar que no se encuentre apagado previamente
 * Eliminado powerOnComm() al inicio del hilo COM
 * Añadida función ATtest en libreria R5 para comprobar si el módulo está activo
 * 
 * 31-05[v044-2]
 * Añadido xEventGroupSetBits( eventPwr5volts,  BIT_0_changePwr  ); en lowmemory reseting system 
 * Añadido apagado de módulo comunicaciones en distintos puntos de bit_2_senddata
 * 
 * 30-05[v044-2]
 * Añadido el taskyield() en hilo ANA
 * Añadido aumento de prioridad en todos las comunicaciones con ubidots
 * 
 * 29-05 [v044-2]
 * Se ha rastreado un error de powerOnCom() seguido de PowerOff() en el momento de hacer SET al BIT_2_SendData que nos lleva a pensar que hay veces que no se reciben datos de la cola de xCommQueue
 *            -> pasar modem.gprsDisconnect();  de [COM] BIT_2_sendData a wait4 COM para evitar desconexión de la red entre envio UBI y envio AWS ????  [30-05]
 * Eliminados los vTaskResume del hilo de control al no existir vTaskSuspend mas que en taskFile ( este resume se ha mantenido hasta elimiar el suspend correspondiente)
 * Modifiado envio de datos a la cola de tamaño 1 xCommQueue de : xQueueSend( xCommQueue, a: xQueueOverwrite( xCommQueue,
 *                -> SE HA QUITADO POR REPETITIVIDAD EL HILO SIEMPRE ESTABA DISPONIBLE EN UN PERIODO MUY LARGO -> Opción de hacerlo con otro bit? [30-05]
 *                -> SE HA VUELTO AÑADIR PERO CON UN NUEVO BIT BIT_20_SENDUBI [30-05]
 * Añadido BIT_20_SendUbi para sustituir a BIT_2_SendData en el hilo ANAREAD. [30-05]
 *        -> Ahora el hilo de ANA se desocupa después de enviar 1 dato con BIT_20_SENDUBI, antes permanecía hasta desactivarse BIT_2_SendData [30-05]
 * Añadidos prints de debug para ver dónde está llamando el programa a powerOffCom() tras un powerOnCom() en el momento de conectar para BIT_2_SendData en hilo COMS
 * Se ha vuelto a añadir powerOnMux() en la creación del hilo de taskFile por si alguno de los bits del wait4 se encontrase preSeteado en algún momento
 * Añadido if(isnetworkconnected) en lugar de if(poweroncom()) en send to aws
 *
 * 26-05
 * Añadido desconexion de cliente despues de consulta del get a ubidots
 * Añadidos retrys de envio en tcppost
 *
 * Añadido taskcreate para hilo de SD
 * 
 * 
 * 13-05 [v043-4_F]
 * Añadido waitBits en hilos de file y anaread
 * 
 * 
 * 04-05 [v042-2_F]
 * Añadido gestion de errores en funciones de librería SD
 * Modificaciones en creación archivo logevents
 * Mejorada función accionPCF
 * Mejorada funcion power (apartado accionPCF)
 * 
 * 19-04 no se han quitado los valores de las pruebas
 * Se modifica el sistema de encendido mendiante ubidots
 * 
 * 
 * 
 * Pruebas 18-04
 * se ha modificado el send_period disminuyendolo y se ha fijado el return de intimewindow a true para que siempre piense que está en la ventana de envíos y hacer pruebas de fallo de escritura 
 * el error detectado aumenta el tamaño de los últimos archivos de datos, en las lecturas se observa un retraso de 100ms a 124ms producido en los 3 últimos archivos generados, se ha añadido en 
 * threadMicroAnaRead una función que indica cuando se superan los 105ms
 */
