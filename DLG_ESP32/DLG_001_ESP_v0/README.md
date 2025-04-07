Programa para: SSM-DLG-001
     Módulo ESP32-S3-WROOM-1U (dev Module en arduino)
         Partition scheme: 16MB flash 3mb APP 9.9mb fatfs
         Flash size = 16MB
         USB CDC on boot disabled
         
         # Name,   Type, SubType,  Offset,   Size,     Flags
nvs,      data, nvs,        0x9000,   0x5000,
otadata,  data, ota,        0xe000,   0x2000,
app0,     app,  ota_0,     0x10000, 0x300000,
app1,     app,  ota_1,    0x310000, 0x300000,
ffat,     data, fat,      0x610000, 0x960000,
factory,  app,  factory,  0xF70000,  0x80000,
coredump, data, coredump, 0xFF0000,  0x10000,
# to create/use ffat, see https://github.com/marcmerlin/esp32_fatfsimage
         
         
         
 La placa tiene dos pines de selección 19 y 20 que permiten gestionar un multiplexor de una uart: SN74CBTLV
     En este selector se encuentra:
     UART0 -> ESP32 "UART-IN" SN74CBTLV
       UART1 -> MIKROE J2 (MIKROE2)
       UART2 -> RS232 channel1 U2Rx(R1Out) U2Tx(T1In) 
       UART3 -> RS232 channel2 U3Rx(R2Out) U3Tx(T2In)
       UART4 -> RS485
     
     MIKROE1 tiene como uart:
       UMRX pin 24
       UMTX pin 25
     
    
  WORKING:

    - Modificar initialize de powerManager para usar las nuevas funciones

    - Meter clase Sensor



    -Configurar las TCA de entrada y sus funciones relacionadas
    
  	-Esquema global (sensor.cpp)
  	-Lecturas RS485 
   

  PENDIENTE:
    -Gestionar la destrucción de objetos
   
