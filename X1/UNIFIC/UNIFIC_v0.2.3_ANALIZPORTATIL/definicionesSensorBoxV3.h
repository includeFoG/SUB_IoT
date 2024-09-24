 
int min_(int a,int b){
  if(a>=b)return b;
  return a;
}




//#ifndef FLOW_CONTROL
////Without Flow control
Uart SerialM1(&sercom2, M1_RX, M1_TX, SERCOM_RX_PAD_1, UART_TX_PAD_0);
//#else
////With Flow control
//Uart SerialM1(&sercom2, M1_RX, M1_TX, SERCOM_RX_PAD_1, UART_TX_RTS_CTS_PAD_0_2_3, 2, 5);
//#endif

void SERCOM2_Handler()
{
  SerialM1.IrqHandler();
}


Uart SerialM2 (&sercom5, M2_RX, M2_TX, SERCOM_RX_PAD_3, UART_TX_PAD_2);

void SERCOM5_Handler()
{
  SerialM2.IrqHandler();
}


void init_IO (){
 placa.Initialize();
 pinMode(EN_SERVO,INPUT);
  pinMode(M1_AN,INPUT);
  pinMode(M1_RST,OUTPUT);
  placa.Initialize5V();  
  pinMode(MUX_SEL_A,OUTPUT);
  pinMode(MUX_SEL_B,OUTPUT);
  pinMode(FLOW_RS485, OUTPUT);

  // Init in receive mode
  digitalWrite(FLOW_RS485, LOW);  
  digitalWrite(M1_RST,LOW);
  digitalWrite(MUX_SEL_A,LOW);
  digitalWrite(MUX_SEL_B,LOW);
}


float MeasureTemperature(){
  float RawTemperature = ((3100 * analogRead(TEMP)/4096.0)-500)/10.000 ;
  return RawTemperature;
}

void insertarComando(String _cmd,unsigned int _index,String &_programa){
  if ( _index< _programa.length()){
    _programa=_programa.substring(0,_index)+_cmd+_programa.substring(_index);   // dato
  }else{
    _programa+= _cmd;
  }
  Serial.println(_programa);
}
