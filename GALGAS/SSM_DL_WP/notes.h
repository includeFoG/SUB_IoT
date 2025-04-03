/*
 * //configuracion de ESP32 para programar
 * Modulo ESP32S3 dev module
 * Partition scheme: 16MB flash 3mb APP
 * Flash size = 16MB
 * CDC on boot disabled
 * para ver los logs habilitar core debug level info
 * 
 * Quitar R9 y C10 de placa programador V0
 

IMPORTANTE: MODIFICAR LA CAPACIDAD DE LA PILA BATTERY_CAPACITY

  ____________________________________________DIRECCIONES I2C____________________________________________
 *  RTC-DS1339 :             b1101000 = x68 = 104
 *  IMU-MPU6050:             b1101001 = x69 = 105
 *  FUEL GAUGE-BQ27441DRZR:  b1010101 = x55 = 85 //TIENE SWITCH DE ALIMENTACION
 *  
 * 
 * _______________________________________INFORMACION DE LIBRERIAS_______________________________________
 * 
 * https://github.com/espressif/arduino-esp32
 * board ESP32 driver version: 2.0.14
 * 
 *_________________________________________INFORMACION DE HILOS__________________________________________
 * TaskBLE           SE CREA EN SETUP()
 * TaskLedStatus     SE CREA EN SETUP()
 * TaskWIFI          SE CREA EN TASKBLE
 * TaskDataReading   SE CREA EN TASKBLE
 * 
 * ____________________________________________INDICADORES BATERIA___________________________________________ _
 * LED 5 -> Conectado a pin 9 -> Se pone a GND cuando la batería está cargando (ENDENDIDO), se apaga cuando la batería está cargada
 * LED 4 -> Conectado a pin 7 -> Se pone a GND cuando la tensión de entrada para cargar la batería está dentro de los valores necesarios
 * 
 * _______________________________________INFORMACION DE PLACA_______________________________________
 * OJO: Se confirma que con frecuencias inferiores a 160Hz los archivos de sesión pueden tener datos que no se transmitan bien entre placas
 * En esta version de la SSM-DLSG-002 es necesario poner el pin de dirección I2C de la IMU a lvl alto para que no coincida con la dirección del RTC, para ello retiramos R13 y hacemos pullup con C8.
 * Las palancas de Enable del cargador de baterias se posicionan: 1:OFF, 2:ON
 * 
 * EN LA PRIMERA VERSION HARDWARE SSM-DLSG-002 :
 * Hay que cambiar el pin de dirección a la IMU poniéndolo a nivel alto, rotando R13 para que esté en contacto con los 3.3V de C8
 * Hay que cambiar el condensador de salida del regulador de tensión de 3.3V C3 por uno de 47uF de tantalio y el de entrada del micro C1 por uno de 20uF para que no se venga abajo la alimentación al encender el BT
 * 
 * CONFIGURACION DE SWITCHES DE CARGA:
 * MAXIMUM INPUT CURRENT INTO IN PIN 500 mA. USB500 mode
 * EN1 -> 1
 * EN2 -> 0
 * EN PLACA:
 * 1->0
 * 2->1
 * 
 * Veocidad max bus i2c para IMU  400kHz https://makersportal.com/blog/calibration-of-an-inertial-measurement-unit-imu-with-raspberry-pi-part-ii
 * SE HA HECHO UN TEST A LA IMU SACANDO LAS SIGUIENTES VELOCIDADES: //https://naylampmechatronics.com/blog/45_tutorial-mpu6050-acelerometro-y-giroscopio.html
 * 1000 samples -> 1879ms
 * 100  samples -> 188 ms
 * 50   samples -> 94 ms
 * 25   samples -> 47 ms
 * 
 * Rangos de giroscopio: +-250,+-500,+-1000,+-2000º/sec
 *  * FS_SEL | Full Scale Range   | LSB Sensitivity
 * -------+--------------------+----------------
 * 0      | +/- 250 degrees/s  | 131  LSB/deg/s
 * 1      | +/- 500 degrees/s  | 65.5 LSB/deg/s
 * 2      | +/- 1000 degrees/s | 32.8 LSB/deg/s
 * 3      | +/- 2000 degrees/s | 16.4 LSB/deg/s
 * Rangos de acelerómetro: +-2.+-4,+-8,+-16g
 *  * AFS_SEL | Full Scale Range | LSB Sensitivity
 * --------+------------------+----------------
 * 0       | +/- 2g           | 16384 LSB/g  -> gravity g 9,82m/s2
 * 1       | +/- 4g           | 8192  LSB/g
 * 2       | +/- 8g           | 4096  LSB/g
 * 3       | +/- 16g          | 2048  LSB/g
 * 
 * 
 * _________________________________________COMANDOS BLE_____________________________________________
 * ç   Formato de comando
 * '$' Caracter de inicio
 * 
 * COMANDO:    ||   FUNCION:
 * $stcal          Calibra la galga del dispositivo transductor base (SAMD21)
 * $imucal         Calibra la IMU
 * $cfgwifi        Configura y conecta con la Wifi configurada
 * $stopwifi       Finaliza el hilo de Wifi
 * $syncrtc        Sincroniza el RTC externo a partir del NTP y posteriormente actualiza el RTC interno
 * $viewdata       Permite visualizar las medidas que está enviando el transductor base
 * $datasesion     Permite iniciar una sesión según los tiempos de configuración establecidos
 * $stopdata       Finaliza una sesión o la visualización de los datos
 * $close          Cierra el hilo BT (para volver a abrirlo hay que reiniciar el dispositivo)
 * 
 *  * '/n' Caracter de fin
*/
/*____________________________________________________TODO: ________________________________________________________________________
 * - aprender más sobre los uuid
 * - Habría que ver si al despertar en lugar de hacer un wakeup normal podemos pasar a otro modo de menor consumo antes de decircir si 
 * hacer el wakeup normal o no
 * 
 * Secundarias:
 * - Mejorar tiempo de carga haciendo que el dispositivo entre en modo ultra lowpower si detecta que está cargando
 * - Configurar hora mediante comandos
 * - Consultar RTC  bit7 registro archivo log + info reset 
 * - Ver si es posible modificar la comunicación BLE->RS485 para que sea relativa al comando recibido (hilo dedicado en base)
 * - Configuración de timewindow mediante comando
 * - Configuración de franja horaria mediante comando
 * - Configuración de nombre de dispositivo mediante comando
 * - Que la configWifi sea independiente de iniciar el wifi y sus valores se almacenen en nvs https://www.youtube.com/watch?v=1rHWwjGA8uE
 * - Cambiar vector de horas mediante comando
 * 
 * Terciarias:
 * - trabajar con el segundo nucleo
 * - Eventgroup 3.3V para ver si alguien lo está usando y que no se deshabilite por error
 * - Ver opciones multinucleo
 * - Convertir en función la escritura en los buffers
 * - !!!!HAY QUE PROBAR A TRABAJAR CON UTC HACIENDO CAMBIOS DE CONFIGURACIONES. CONFIGURAR SESIONES EN GMT LOCAL Y ALMACENAR UTC
 * - Mirar si hace falta cambiar delay por msdelay
 */

/*__________________________________________________________________________________________________NOTAS DE VERSION__________________________________________________________________________________________________
 * [v.0.0.7.2] - 03/04/2025
 * - Para poder alcanzar la nueva frecuencia de 80Hz introducida en la lectura de las galgas en el firmware 0.0.5 de la base, se ha tenido que reducir la cantidad de muestras que se toman para obtener las medidas de la imu 
 *    pasando de 25 a 5 en el getIMUFiltered del taskDataReading
 * - Se han corregido algunos errores de compatibilidad con std::string con RxBle que por alguna razón no habían saltado en la versión anterior, quizá se hizo alguna correción a posteriori
 * - Ahora el hilo de taskledstatus no realiza un checkIfCharging() si se encuentra en meassuring intentando evitar la perdida del dato con frecuencia de 3 segundos que coincide con el wire.begin
 * - Se ha cambiado el algoritmo que determina que la pila está cargada, había casos que no contemplaba
 * - Ahora aunque se pierda el dato se posicionan las comas necesarias para que el resto de valores si estén ordenados
 * 
 * PROBLEMAS DETECTADOS:
 * - Al cambiar de pila como cambia la capacidad hay un problema con el cálculo de batería llena -> MUY IMPORTANTE INDICAR LA CAPACIDAD DE LA BATERIA PARA QUE CARGUE CORRECTAMENTE
 * - Cuando entre en modo sesión de forma automática aparecen mensajes de intentos de wire.begin() con una frecuencia de 3 segundos, [lo quitamos evitando que la tarea taskledStatus consulte a la batería
 * si se encuentra en el proceso de medición]
 * 
 * a la misma frecuencia se pierden datos en el archivo 
 * 
 * 
 * 
 * 
 * [v.0.0.7.1] - 06/06/2024
 * Se detecta fallo en SD > 64Gb por formato EXFAT se puede hacer partición de 30GB y pasarlas a FAT32 para que funcione
 * Se corrrige (?) bloqueo cuando se inicia una sesión y no se detecta o no está conectada el dispositivo de la galga.
 * 
 * Añadido posteriormente: rxData +=",,,"; cuando no se conecta con el dispositivo de la galga para que se vea que no se han recibido esos datos
 * 
 * En esta versión se observa que en algunos dispositivos (2 de 10) cuando se genera una sesión sin estar conectado a la base generan 5 archivos (por intentos de conexión y time_window) sin embargo
 * en los que contienen datos se produce en el último un ENDFILE, mientras que hay algunos que esos archivos no contienen nada y coincide con que no se produce ENDFILE.
 * He estado repasando el código y no veo por qué cierra los archivos antes de terminar la sesión, supongo que he configurado una salida si no recibia datos pero no la encuentro
 * 
 * 
 * [v.0.0.7] - 06/02/2024
 * - Añadida funcionalidad monitorización de batería mediante archivo independiente.
 * - Añadido comando BLE para consultar estado de batería.
 * - OJO: Se ha probado el dispositivo a una frecuencia de 80Hz y se ve que al recibir los datos de la base en ocasiones se come algún dato, se van a hacer pruebas con 160Hz a ver si sigue pasando
 * - OJO: si se baja el tiempo de medida por debajo de la ventana de tiempo se pueden duplicar archivos
 * - Añadidos indicadores lumínicos para informar si el dispositivo se encuentra cargando o está cargado
 * - Ahora si el dispositivo se conecta a cargar estando en cualquier estado si el usuario no se lo corta entra en un estado de lowpower hasta que se cargue
 * 
 * 
 * [v.0.0.6] - 25/01/2024
 * - Configuración de IMU añadida
 * - set y get de nvs_16 pasados de unsigned a signed para hacerlo concordar con los posibles valores de offset de la IMU
 * - Detectado bug en el que si inicializas el MPU6050 en algunos puntos es como si no estuviese configurado, se trata del tiempo de inicialización del dispositivo.
 * - Añadida configuración de sensibilidad para acelerómetro y giroscopio
 * - En esta versión el dispositivo siempre se calibra para +-2g, +-250º/s y posteriormente se cambia a la escala deseada.
 * - Añadida transformación de raw a unidades m/s2 y º/s
 * - Ahora se puede seleccionar si la calibración de la IMU parte de un valor de fábrica o del último valor de calibración.
 * - Ahora si se introducen datos erroneos de wifi el sistema alerta al no poder conectar
 * - Corregido error en el que si dispositivo base enviaba [incorrect format] se bloqueaba el sistema
 * 
 * 
 * [v.0.0.5] - 10/01/2024
 * - Se ha añadido el cierre del hilo de WiFi si este se encuentra abierto cuando se cierre la comunicación BLE mediante comando $close CMD_END_BLE
 * 
 * [v.0.0.4] - 10/01/2024
 * - Se descubre que en ocasiones la transmisión RS485 envía un caracter nulo lo que puede estropear las comprobaciones
 * - Ahora se puede configurar los parámetros de la galga desde el BLE
 * 
 * 
 * [v.0.0.3] - 05/01/2024
 * - Encontrado problemas en cálculo de alarma, el dispositivo en ocasiones no sigue el patrón, se ha ajustado la función de getNextSesion, inTimeWindow y sToNextSesion.
 * - Corregido problema por el que al cerrar el hilo de BLE desconectando el dispositivo, no se cerraba el hilo de wifi y no pasaba a estado de iddle bloqueando el sistema.
 * - Corregido error en sToNextSesion() la constante de duración de día estaba en ms en lugar de s haciendo que se durmiese x1000 cuando tenía que despertarse a las 00:00
 * - Ahora las Sesiones se producen correctamente
 * - Avances en configuración Base SG mediante BT
 * - Se observa desviación RTC aproximada de 2min dia
 * 
 * 
 * [v.0.0.2] - 02/01/2024
 *  - Añadida actualizacion de baudrate a Serial1 cuando se realiza cambio de frecuencia en CPU  Serial1.updateBaudRate
 *  - Añadido SLEEP_TIME para tener en cuenta el tiempo que lleva la placa despierta cuando no se usa RESET_WITH_RTC para fijar la frecuencia entre sesiones
 *  - Modificado el temporizador de sesión, ahora no es la tarea del loop la que gestiona el tiempo de medida por minuto sino que es un timer, se ha optimizado
 *    y simplificado el uso del loop.
 *  - Generado semaforo para el nuevo funcionamiento del task manager manager_semaphore , evita que los estados se modifiquen sin que tengan lugar las configuraciones previas
 *  - Encontrado problema al actualizar RTCint con RTCext mediante ntp, los formatos usados para comprobar el año no correspondían con los del RTC_EXT (años desde 1900)
 *  
 *  
 * [v.0.0.1] - 19/12/2023-29/12/2023
 *  -Reordenado todo el código 
 *  -Se han modificado algunos nombres para que sean más autodescriptivos
 *  -Se han movido algunas variables globales para convertirlas en locales
 *  -Se ha añadido un contador con TIME_BLE_WAIT_DEVICE para que el hilo de BT se elimine si se pasa TIME_BLE_WAIT_DEVICE segundos sin conectarse un dispositivo
 *  -Añadidos filtros a comandos que necesitan que otros hilos estén activos. config RTC, desact WiFi
 *  -Modificado comando viewdata , ahora con el solo se visualizan los datos de la galga sin necesidad de generar archivo
 *  -Creado comando datasesion para iniciar una sesión, si se quieren visualiazar hay que lamar a viewdata
 *  -Añadido pulse a función blinkled()
 *  -Añadida consulta a RTCEXT al inicio del programa
 *  -Añadidas funciones NVS
 *  -Añadida funcionalidad en la que si el RTCEXT no se encuentra actualizado se genera un archivo a partir de nvs_val_filename
 *  -Añadido STATE almacenado en RTC mem para determinar estado del dispositivo, configuración de alarmas, inicio de sesión...
 *  -Añadida sesión con inicio por RTC
 *  -Añadido filtro en lectura de RS485 antes de iniciar la sesión para esperar a que el dispositivo transductor esté enviando datos
 *  -Añadida ventana de tiempo para las sesiones
 *  -Ampliado hilo led para que blinkled funcione con eventos
 *  -Añadida cabecera al inicio de los archivos
 *  -Añadidas funciones taskDelay para sustituir por los delay -> DIO ALGUNOS ERRORES Y FINALMENTE SE DESHIZO
 *  -En un momento dado se detecto caracteres extraños en el archivo de datos, al parecer están relacionados con tener la declaración de la variable char c fuera del bucle de la tarea
 *  -Cuando el tiempo de medida se tomaba de la placa del ESP32 en ocasiones cada 6 o 7 medidas había un descuadre en el tiempo, ahora el tiempo de medida lo envía la placa de lectura de galgas
 *  -Surge problema reset aleatorio con retorno de wakeup_reason = 0
 */


/*
 * SE PUEDEN EMPLEAR VARIABLES GLOBALES ENTRE TAREAS SIN CONSIDERARSE MALA PRAXIS SIEMPRE QUE SE CONTROLE BIEN LOS ACCESOS PARA QUE EL ORDEN DE LAS MODIFICACIOENS SEA EL DESEADO, NO ES NECESARIO USAR SIEMPRE EVENBITS
 */
