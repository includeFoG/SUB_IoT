//#define  NUMITEMS NUM_VALORES


//#define SIZE_BUFFER 512
//#define SIZE_BUFFER_CreateQueue 512  //ADDED CONFIG NECESARIO PARA GENERAR LAS COLAS vCreateQueues()

QueueHandle_t xTxBuffer = NULL;


struct qMessage
{
  float qData[ NUM_VALORES + 2 ];//float qData[ NUMITEMS + 2]; 
  unsigned long qTimeStamp;
};

struct qPosition
{
  uint8_t qID;
  float latitude;
  float longitude;
  uint8_t fix;
  //float HDOP;
};

QueueHandle_t xDataQueue = NULL;
QueueHandle_t xFileFlagQueue = NULL;
QueueHandle_t xCommFlagQueue = NULL;
QueueHandle_t xCommQueue = NULL;
QueueHandle_t xGpsQueue = NULL;
QueueHandle_t xGpsLogQueue = NULL;

#ifdef DEBUG_LOGEVENTS //ADDED
QueueHandle_t xLogEventsQueue = NULL; //ADDED LOG Cola para log de eventos
#endif
//QueueHandle_t xConfigQueue = NULL; //ADDED CONFIG Cola para descarga archivo config AWS

void vCreateQueues( void )
{

  xDataQueue = xQueueCreate( 8, sizeof( qMessage ) );
  xCommQueue = xQueueCreate( 1, sizeof( qMessage ) );
  xCommFlagQueue = xQueueCreate( 4, sizeof( int ) );
  xFileFlagQueue = xQueueCreate( 4, sizeof( int ) );
  xGpsQueue = xQueueCreate( 1, sizeof( qPosition ) );
  xGpsLogQueue = xQueueCreate( 1, sizeof( qPosition ) );
  xTxBuffer = xQueueCreate (SIZE_BUFFER / Q_DIV, sizeof(uint8_t));
#ifdef DEBUG_LOGEVENTS
  xLogEventsQueue = xQueueCreate(10, sizeof(byte)); //ADDED LOG tamaño de la cola 10bytes -> 10 estados
#endif
  //xConfigQueue = xQueueCreate( SIZE_BUFFER, sizeof( uint8_t ) ); //ADDED CONFIG  tamaño de la cola = tamaño del buffer
  //xConfigQueue = xQueueCreate( SIZE_BUFFER_CreateQueue, sizeof( uint8_t ) ); //ADDED CONFIG  tamaño de la cola = tamaño del buffer

  if (xTxBuffer == NULL)
  {
    SERIAL.println("Fail Buffer");
  }
  if ( ( xDataQueue == NULL )  )
  {
    SERIAL.println("Fail creating queue 1");
  }
  if ( ( xCommQueue == NULL )  )
  {
    SERIAL.println("Fail creating queue 2");
  }
  if ( ( xCommFlagQueue == NULL )  )
  {
    SERIAL.println("Fail creating queue 3");
  }
  if ( ( xFileFlagQueue == NULL )  )
  {
    SERIAL.println("Fail creating queue 4");
  }
  if ( ( xGpsQueue == NULL )  )
  {
    SERIAL.println("Fail creating queue 5");
  }
  if ( ( xGpsLogQueue == NULL )  )
  {
    SERIAL.println("Fail creating queue 6");
  }
#ifdef DEBUG_LOGEVENTS
  if ( ( xLogEventsQueue == NULL )  ) //ADDED LOG
  {
    SERIAL.println("Fail creating queue 7");
  }
#endif
  //   if ( ( xConfigQueue == NULL )  )  //ADDED CONFIG
  //  {
  //    SERIAL.println("Fail creating queues 8");
  //  }
}


#ifdef DEBUG_LOGEVENTS
void sendToEventsLogQueue(byte b_infoLog) { //ADDED LOG
  if (uxQueueSpacesAvailable(xLogEventsQueue) != 0)
    xQueueSend( xLogEventsQueue, &b_infoLog, ( TickType_t ) 0);
  else
    DEBUG("LA COLA DE LOGEVENTS ESTA LLENA");
}
#endif
