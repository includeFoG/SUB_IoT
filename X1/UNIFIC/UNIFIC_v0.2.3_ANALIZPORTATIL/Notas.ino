/*
 * 
 * 
 * 
 * 
 * 
19/04/2022[v.0.2.1]-> para ARS
-modificado TAKEPARAM() para que cuando se cometa un error en la busqueda del nombre de la key no se acceda a un índice del vector en un espacio de memoria no contemplado, se ha generado la key "Er" en la posición 5 para que se utilice esta en su lugar
-modificado en read_gen_RS485() antes ponía a 0 los valores del vector del índice 0 al 7
--modificado en read_gen_RS485() en el switch se ha sacado la declaración de la i de cada uno de los for y se ha pasado a fuera del switch, ahora es int j=0;
-en read_gen_RS485 datosfloat tomaba la dirección con puntero del valor de un índice de un vector y posteriormente decía que ese valor era otro vector. Se está modificando para que esto no ocurra.

01/07/2022[v.0.2.0]-> BUSQUEDA DE ERRORES ARS (Schneider+Sensores)
-Modificada inserción de valores en case 31: //'e' , de "rsQve" y "rIQve" a "rsOQve" y "rIOQve", no es necesario pasar por O al llamar a Q, pero en O existe un wtd.clear que nos ayuda a que la placa no reinicie si hay algun retraso 
o errores en las lecturas de los analizadores de red
-Se añade en case 31: //'e' una limpieza del json de envio después del último analizador
-Si hay un error de comunicaciones y no se llega a defTopic() no se limpian los flags de topics , se añade en 'A' a la vez que se activa el flag de Analogicos, la desactivación del flag de Analizadores
-c_analiz (codigo de error de analizadores) pasa a llamarse err_analiz para mayor claridad
-Quitado confirmación de envío en 'e' para aumentar número de analizador -> NO TENIA SENTIDO controlar aquí los errores de envío, si la función de envío tras sus reintentos no consigue enviar, se pasa al siguiente analizador.
-Node3 (no hay stop)





27/05/2022 [v.0.2.0]
-Modificada función de lectura analizadores Siemens 's' para lectura optima es necesario realizar una doble conexión para tomar la medida modbusethernet
-Modificada función de lectura analizadores Schneider 'I' se ha optimizado la función, ahora gestiona mejor los errores en cuanto a la conexión con el dispositivo
 ,se ha añadido un control de toma de datos para no agregar al JSON de envio los valores no leidos.
-Modificada la toma de datos de sensores analógicos, ahora no agrega la medición del sensor al JSON si este no se encuentra activado en la configuración mediante USEC 
 
PARAMETROS DE CONFIGURACION [v.0.2.0]
Añadidos nuevos parámetros de configuración al archivo de configuración [Config MQTT]

->>> ENCONTRADO BUG con programa : "GLBsOQeSHVU"
A la hora de asignar el valor de "Al1" con takeParam no es capaz de encontrar el nombre en el vector de Strings para conformar el JSON
-> Se declara el v_String como static const y se añade flush y yield para borrar buffer y dar tiempo al micro 

     ^^^^^^
/////PRUEBAS////(
 -Probar nuevo programa como se encuentra
 -Quitar parametro AL1 a ver si solamente este da el problema
 -Pasar AL1 al final 
 -Modificar JSON y acortarlo para este caso
 -Comprobar que el nuevo s_aux coge el valor en todos los casos
 -Conflicto en posición , rotar uno 

_____________________________________________________________________________________________

Bug encontrado en forecast, el archivo se recrea en la SD, se ha corregido falta hacer pruebas

Mejorada libreria BC95 con CMEE=1 ahora no hace falta hacer preconexión antes de usar el módulo

Corregido BUG envio SSM_003-ESP32-MQTT 
  El bug se debia a la declaración del cliente FTP empleando el mismo socket que el de envío a mqtt.
  
Modificado toma viento [v.0.1.6]

Agregado envío TCP a ubidots con formato [v.0.1.5.A]


 Agregado envio de archivos por FTP (no SFTP) [v.0.1.5.A]
 27/01/2022
 Añadido sensor hidrocarburos DISEN [v.0.1.5.A]
  +limpieza de sensor
 24/01/2022
 Corregido bug de calculo de tiempo entre mediciones[v.0.1.5.A]
 21/01/2022
 AÑADIDO SENSOR UV[v.0.1.5.A]
 19/01/2022
 DESCARGA DE ARCHIVO HTTP (SOLO PARA: SARAR410M)[v.0.1.5.A]
  CONDICIONES:
   
 18/01/2022

 
 INICIO PROGRAMACION DESDE SD [v.0.1.4]
 21/12/2021
 
 TIEMPOS ENTRE MEDICIONES [v.0.1.2.D]
 03/12/2021
 Mejorado el tiempo entre envíos al no realizar tiempo de espera entre mediciones si se ha cumplido el tiempo entre envíos (reducción de tiempo)
 
 DATALOG 2.0 [v.0.1.2.D]
 29/11/2021
 Mejoras realizadas en datalog. Ahora el datalog comprueba la cantidad de espacio libre en la SD y emplea un sistema de directorios para guardar los logs en carpetas diarias, en caso de que no haya espacio suficiente
 elimina el directorio más antiguo siempre que este respete las condiciones de nombre YYMMDD
 03/12/2021
 Arreglado bug que no añadía 0 cuando la cifra de mes y día era inferior a 10 para mantener el formato de 2 cifras.
 
 IMPORTANTE:[v.0.1.2.D]
 26/11/2021
 Se han realizado cambios en la función EnviaMQTT(String MessageToSend) anteriormente se realizaban los envíos y se desconectaba del broker, mqttdisc() 
 se procede a trasladar la desconexión del broker a otra función. La versión más reciente antes de este cambio es la v0.1.2.B o la v0.1.3
 se ha quedado preparado 100% para el SARA R410M
 
 //MEJORAS MQTT SARAR4 [v.0.1.2.D]
 16/11/2021
 Añadido envio por comandos puros de mqtt del sarar4 empleando funciones de nombre generico como el BC95

 INTRODUCCION DE LISTAS [v.0.1.2.D]

 DISPOSITIVO AMPERIMETRO   [v.0.1.2.B]
 22/11/2021 
 El dispositivo de amperímetros únicamente lee uno de los dos registros del dato float
 
 
 //CORREGIDO BUG TAKETIME EN UBLOX SARA U2[v.0.1.2.B]
 10/11/2021
 Al no pasar por taketime() no se inicializaba el RTC, se ha corregido generando la función _taketime en la libreria del saraU2 e iniciando el RTC en el setup
 
 //ENVIO SIEMENS-PAC [v.0.1.2.B]
 08/11/2021
 Añadida funcionalidad de envio a diversos topics para los analizadores, mejoras en código y corrección de errores
 
 //ACTUALIZACION JSON [v.0.1.2.B]
 04/11/2021
 Se actualiza el uso de #defines en los JSON para diferenciación de casos
 
 //GESTION DE ERRORES [v.0.1.2]
 27/10/2021
 Se modifican los errores del Log y se añaden nuevos
 //DesviaMedida
 26/10/2021 
 Mejorada función que detecta la desviación de la medida ahora procedente de cualquier sensor sin necesidad de indicar nada
 //SDFat
 25/10/2021
 Actualizada libreria SDFat 
 //Low Power
 21/10/2021
 Añadida función LowPower
 //LOGDATA
 14/10/2021
 Añadido codigos de error al logdata version 1 decimal
//BC95G
  08/10/2021
  Creada libreria para BC95G dentro de TinyGSM con conexión a broker MQTT y envío de mensajes. NO SE HAN DETECTADO PROBLEMAS DE COMUNICACION
//MEJORA SARAR410
  05/10/2021
Modificada librería tiny GSM ahora la conexión y desconexión del módulo SARAR410M es más rápida. SE ESPERA HABER SOLUCIONADO PROBLEMAS DE CONECTIVIDAD

//MEDIDAS CONTINUAS SIMULTANEAS
 28/09/2021
AÑADIDA FUNCION DE MEDIDAS CONTINUAS SIMULTANEAS (A FALTA DE PRUEBAS)
AÑADIDA LECTURA DE AMPERIMETROS PARA COMPRESORES ASTICAN

//SD
 24/09/2021
ARREGLADOS PROBLEMAS CON SD (RELACIONADOS CON ESCRITURA Y LECTURA CON VALORES DISTINTOS)

//ADC2
  22/09/2021
AÑADIDAS TOMA DE VALORES Y ENVIO DE DATOS RELACIONADOS CON EL SEGUNDO ADC JSON CON DEPENDENCIA UNICA DE LA PLACA INDICADA

 
// ARREGLADO CODIGO RELACIONADO CON 003
   21/09/2021
 ENVIO CORRECTO Y CONTINUO MEDIANTE SARA R4 CON MQTT 
 
//JSON
  10/09/2021
OPTIMIZADA FUNCION buildGenJSON
ELIMINADA LA GENERACION DE JSON PARTICULAR
CREADOS VECTORES DE NOMBRE DE VARIABLE Y VECTORES DE VARIABLES A ENVIAR

//MQTT
  09/09/2021
OPTIMIZACION DEL TAMAÑO DE LOS MENSAJES CON FORECAST MEDIANTE CODIFICACION DE PREDICCION
REDUCCION DE LOS NOMBRES DE LAS VARIABLES DE JSON

//CORRECCION DE PROBLEMAS EVENTUALES AL TOMAR FECHA CON ESP32 (MODIFICACION DE LIBRERIAS TINYGSM)
  09/09/2021
  
//UNIFICACION DE PLACAS
 07/09/2021
INICIO DE PROGRAMACION COMUN PARA LAS DOS PLACAS


//TIMESTAMP
 03/09/2021
CORRECCION DE ERRORES EN TOMA DE FECHAS Y AÑADIDO APAGADO DE MIKROE TRAS ENVIO CON EXITO SI EL TIEMPO DE ENVIO ES SUPERIOR A 60SEG localConfig.waitTime>=60
AÑADIDA ACTUALIZACION DIARIA DE RTC
SIGUIENTE PASO: Emplear las marcas de tiempo para almacenar la información en la tarjeta de memoria.
                Control de tiempo en funciones taketime de librería tinygsm
                Gestión de errores en adquisición de timestamp si llegan datos sin senstido

//TOMA DE VALORES CONTINUA  (medidaConst)
  02/09/2021
OPTIMIZADA LA OBTENCION DE DATOS CONTINUOS CON FUNCIONES GENERALISTAS PARA CUALQUIER REGISTRO RS485 DE CUALQUIER DISPOSITIVO
AÑADIDAS MEDICIONES CONTINUAS PARA: DIRECCION DEL VIENTO / GRADOS DEL VIENTO / VELOCIDAD DEL VIENTO 

//MEDIANAGRADOSVIENTO (medidaConst)
 01/09/2021
AÑADIDAS MEDIANA A LOS GRADOS, SISTEMA DE DIRECCIÓN DE VIENTO MEJORADO
Obs: el sensor de posición de la veleta NO es continuo, es discreto tiene los valores de grados mapeados como los de dirección (0,45,90,135,180,225,270,315)

//TAKETIME 't'
 01/09/2021
AÑADIDAS MARCAS DE TIEMPO REALES PARA: ESP32,SARAR410M, SIM808
LIBRERIA TINYGSM MODIFICADA PARA LA FUNCION _TakeTime()




*//*
 *Version subida al sensor de presiones es la SSM_wifi_command_14 
 *Version subida al sensor de nivel es la SSM_wifi_command_14 
 */


/*
 * 1º
 * PSOOOSVPSOOOSVPSV
 * 2º
 * PPSOOOSVPSV
 * 
 * 
 * 1º
 * PSOOOSVPSOOOSVPSV
 * 2º
 * PSOOOSVPSOOOSVPSV
 * 
 * Opcion con O
 * 1º
 * OOOSV
 * 
 * Optimizando 
 * PSOOOSVPSOOOSVPSV
 * PSOOOSVPSOOOSVPSV

 * 
 */
