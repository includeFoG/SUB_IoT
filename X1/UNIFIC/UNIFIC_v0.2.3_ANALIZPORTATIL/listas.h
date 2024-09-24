/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////STRUCT LISTAS
struct nodo {

  char* tipo;
  int chnl;
  char* topic="Nan";
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

void topics_nodos()
{
  struct nodo *p_aux = p_raiz;
  int cnt = 0;
  int numtopics = 10; //numero máximo de topics 10
  String v_topics[numtopics];
  bool exist = false;

  while (p_aux != NULL)
  {
    if (p_aux->topic != "Nan") //si hay topic en el nodo
    {
      if (v_topics.indexOf(p_aux->topic)==-1) //comprueba si no esta en v_topics
      {
        Serial.println("El topic no está en el vector de topics");
        int nmr=v_topics.indexOf('a');
        v_topics[nmr] = p_aux->topic; //si no está introduce el topic en la posición en que no hay ningún topic
      }
    }
    p_aux = p_aux->p_sig;
  }

  Serial.print("Topics encontrados: ");
  for (int i = 0; i < numtopics; i++)
  {
    if (v_topics[i] != "Nan")
      Serial.print(v_topics[i] + " ; ");
    else
      i = numtopics;
  }
}
