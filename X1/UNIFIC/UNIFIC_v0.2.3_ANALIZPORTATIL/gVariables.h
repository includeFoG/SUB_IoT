
//TEST DE HTTPS caso 'h' puede quitarse
String httpreq1="PUT /pruebas/mytest-003/pruebaDESDEX1.txt HTTP/1.1\r\nHost: f8i1bdejsc.execute-api.us-east-1.amazonaws.com\r\nContent-Type: text/plain\r\nContent-Length: 22\r\n\n";

//TEST I2C IO 16 CH PCF8575 PLACA ACTU
uint16_t x;
uint8_t outnum=0;


//long tiempopasado=0;//PRUEBA PARA XAVI TIEMPOS RS485 TESTXAVI
String directorios;

int num_dir = 8;
String localTime = "NoInit_0";
String LogJSON = "{NO INIT}";
String err_code = "000  | ";

bool _sendData, _sendPhoto, _mvTrap, _tkGPS, _reset = true;
bool _failConnect, _configLoaded = false;
bool _error = false;

int retry = 0;

int contFallos = 0;

int n_Analiz = 1; 

//#define WDT_TIME WDT_SOFTCYCLE16M
#define WDT_TIME WDT_SOFTCYCLE8M
//#define WDT_TIME WDT_SOFTCYCLE4M
//#define WDT_TIME WDT_SOFTCYCLE16S


//AUX MEDIDA SENSORES PREV (DESVIACION)
int numDesv = 0;

float prev_Ch1 = 0;
float prev_Ch2 = 0;
float prev_Ch3 = 0;
float prev_Ch4 = 0;

#ifdef SSM_005
float prev_Ch5 = 0;
float prev_Ch6 = 0;
float prev_Ch7 = 0;
float prev_Ch8 = 0;
#endif

float WMaxPrev = 0;
bool  F_BrokerMQTT = false;


//Datos de alarma AWAKE
uint8_t al_hour;
uint8_t al_minute;
uint8_t al_second;
uint8_t _h = 0;
uint8_t _m = 0;
uint8_t _s = 0;
int sum_al = 0; //Sumatorio de la hora de la alarma en segundos
int sum_act = 0; //Sumatorio de la hora actual en segundos
long  sum_SecAct = 0; //Sumatorio de lahora actual en segundos
bool f_FTsleep = true;

long SendTime = -2147483648 ; //Tiempo trascurrido desde el ultimo envio
unsigned long current_timestamp;    // Actual timestamp read from NTPtime_t now;

///RELACIONADOS CON DESCARGA DE ARCHIVOS HTTP//
String buff_ = "";
long tam_paq = 0;
char c_c = ' ';
int content_L = 0;    //EN USO
int ava_ = 0;
char c_2[12];
int ind_ = 0;
unsigned long t_test = 0;
int t_ = 0;     //EN USO
String dwld_fN = "";
String save_fN = "";
int time_ = 0;    //EN USO 'g'  y 'E' caso DISEN


//uint32_t known_CRC32 = 0;
String known_CRC32 = "";       //EN USO
uint32_t checkSumCRC32 = 0;    //EN USO

String str_aux=""; //EN USO



bool a_sendTime = true; //flag de alarma de tiempo de envio
uint8_t maxFiles = 10; //NUMERO MÁXIMO DE ARCHIVOS DE ACTUALIZACION

char buffer_[100];//pruebaftp




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////STRUCT LISTAS
struct nodo {

  char* tipo;
  int chnl;
  char* topic = "Nan";
  char* param_name;
  char* param_value;

  struct nodo *p_sig; //puntero a bloque siguiente (inicializado null)
};

struct nodo *p_raiz = NULL;
struct nodo *p_lastnodo = NULL;

int voidlist() {
  if (p_raiz == NULL)
    return 1;
  else
    return 0;
}


void nuevo_nodo(char* _comms, int _chnl, char* _name, char* _topic, char* _value) //INTRODUCE NUEVO NODO AL FINAL DE LA LISTA
{
  //GENERACION DE NODO
  struct nodo *p_nodonuevo;
  p_nodonuevo = (nodo *)malloc(sizeof(struct nodo));//malloc devuelve una dirección donde se inicia el espacio reservado de tamaño nodo,(nodo *) indica que esa dirección pertenece a un puntero de tipo elemento
  //
  //  if (!p_nodonuevo) { //si malloc devuelve 0 no se ha podido reservar el espacio de memoria{
  //    Serial.println("No se ha reservado memoria para el nuevo nodo");
  //    return false;
  //  }
  Serial.print("marca0");
  String *p;
  String test;
  test = "holaqueso";
  p = &test;
  //INFORMACION DE NODO
  p_nodonuevo->chnl = _chnl;
  Serial.print("marca00");
  p_nodonuevo->topic = _topic;
  p_nodonuevo->param_name = _name;
  p_nodonuevo->param_value =  _value;
  Serial.print("marca01");

  p_nodonuevo->p_sig = NULL;

  Serial.print("marca02");

  //ENLAZAMIENTO DE NODO
  if (voidlist()) { //si la lista esta vacia
    p_raiz = p_nodonuevo;    //el nuevo nodo es el primer elemento de la lista
    p_lastnodo = p_nodonuevo; //el nuevo nodo se convierte en el ultimo de la lista
    Serial.print("marca03");
  }
  else { //si no está vacía
    p_lastnodo->p_sig = p_nodonuevo; //el que era el ultimo nodo, ahora termina apuntando al nuevo nodo
    p_lastnodo = p_nodonuevo;      //el nuevo nodo ahora es el último
    Serial.print("marca04");
    //   return true;
  }
}

void printlist() {
  struct nodo *p_aux = p_raiz;
  int cnt = 1;
  Serial.println("marca2");

  while (p_aux != NULL)
  {
    Serial.println();
    Serial.print("Nodo: ");
    Serial.println(cnt);
    Serial.println("Comms: " + String(p_aux->tipo));
    Serial.println("Chnl: " + String(p_aux->chnl));
    Serial.println("Topic: " + String(p_aux->topic));
    Serial.println("Name: " + String(p_aux->param_name ));
    Serial.println("Value: " + String(p_aux->param_value ));
    Serial.println("_______________________________________");
    Serial.println();

    p_aux = p_aux->p_sig;
    cnt++;
  }
}

int num_nodos() {
  struct nodo *p_aux = p_raiz;
  int cnt = 0;

  while (p_aux != NULL)
  {
    cnt++;
    p_aux = p_aux->p_sig;
  }
  return cnt;
}

char** topics_nodos(uint8_t n_readlines = 10) //se le puede pasar el número de líneas leidas
{
  struct nodo *p_aux = p_raiz;
  int cnt_topics = 0;
  char* v_topics[n_readlines];
  bool exist = false;
  //char* c_nan="Nan";

  while (p_aux != NULL)
  {
    if (p_aux->topic != "Nan") //si hay topic en el nodo
    {
      for (int i = 0; i < n_readlines; i++)
      {
        if (v_topics[i] == p_aux->topic) //comprueba si esta en v_topics
        {
          bool exist = true;
          i = n_readlines;
        }
      }
      if (!exist) //si no existe
      {
        v_topics[cnt_topics] = p_aux->topic;
        cnt_topics++;
      }
    }
    p_aux = p_aux->p_sig;
  }
  Serial.print("Topics encontrados: ");
  Serial.println(cnt_topics);

  return v_topics;
}
