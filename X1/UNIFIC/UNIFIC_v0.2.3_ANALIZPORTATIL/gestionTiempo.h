#ifndef GEST_TIME
#define GEST_TIME


int MIN_FREQ = 10;

unsigned long frec_medida = 10000;
unsigned long frec_medidas[4] = {frec_medida,frec_medida/2,frec_medida/4,frec_medida/8};
unsigned long frec_envio = 300;
unsigned long frec_envios[15]; //SE HACE NECESARIO INICIALIZAR EL VECTOR DE ENVIOS ANTES DE SABER SU TAMAÑO. VECTOR LIMITADO A 28h aprx
int n=0;


int frec_meass_index=0;
int frec_send_index=0;
bool frec_ch1 = false;
bool frec_ch2 = false;
bool frec_ch3 = false;
bool frec_ch4 = false;

float lowGap = 6;
float highGap = 17;

unsigned long iddleTime = frec_medida;
unsigned long _sendTime = frec_envio;

bool triggerFrec(float _medida, float _minGap, float _maxGap){
  //Serial.print(" ");Serial.print(_medida);Serial.print(" ");Serial.print(_minGap);Serial.print(" ");Serial.println(_maxGap);
  return (_medida>=_maxGap||_medida<=_minGap);
}
bool alarmChn(bool trigger, bool used){
  if ( used) return trigger;
  return false;
}
void gestionFrecuencias(float _ch1,float _ch2,float _ch3,float _ch4){
//Chequeo guarda superior e inferior

//  if ( alarmChn(triggerFrec(_ch1,localAnaMap.minValch1+localAnaMap.gapch1,localAnaMap.maxValch1-localAnaMap.gapch1),localAnaMap.usedch1)||
//       alarmChn(triggerFrec(_ch2,localAnaMap.minValch2+localAnaMap.gapch2,localAnaMap.maxValch2-localAnaMap.gapch2),localAnaMap.usedch2)||
//       alarmChn(triggerFrec(_ch3,localAnaMap.minValch3+localAnaMap.gapch3,localAnaMap.maxValch3-localAnaMap.gapch3),localAnaMap.usedch3)||
//       alarmChn(triggerFrec(_ch4,localAnaMap.minValch4+localAnaMap.gapch4,localAnaMap.maxValch4-localAnaMap.gapch4),localAnaMap.usedch4)){
//    frec_meass_index++;  

  if ( alarmChn(triggerFrec(_ch1,localAnaMap.minch1,localAnaMap.maxch1),localAnaMap.usedch1)||
       alarmChn(triggerFrec(_ch2,localAnaMap.minch2,localAnaMap.maxch2),localAnaMap.usedch2)||
       alarmChn(triggerFrec(_ch3,localAnaMap.minch3,localAnaMap.maxch3),localAnaMap.usedch3)||
       alarmChn(triggerFrec(_ch4,localAnaMap.minch4,localAnaMap.maxch4),localAnaMap.usedch4)){
    frec_meass_index++;  
    frec_send_index++;
    Serial.println("INDICE DE FRECUENCIA AUMENTADO");
    err_code+="| x1001 ("+String(frec_send_index)+") |"; 
    _error=true;
  }else{
    frec_meass_index--;
    frec_send_index--;
    if(frec_meass_index<0) frec_meass_index=0;
    if(frec_send_index<0) frec_send_index=0;
    if (frec_meass_index==0 && frec_send_index==0)
          Serial.println("SE MANTIENE EL ÍNDICE DE FRECUENCIA A 0");
    else
    {
    Serial.println("INDICE DE FRECUENCIA DISMINUIDO");
   
    }
  }
  if(frec_meass_index>=3)frec_meass_index=3;
  if(frec_meass_index<=0)frec_meass_index=0;
  if(frec_send_index>=n)frec_send_index=n;
  if(frec_send_index<=0)frec_send_index=0;
  if (frec_medidas[frec_meass_index]<MIN_FREQ)iddleTime=MIN_FREQ;  //??? ESTO YA NO DEBERIA PASAR
  else iddleTime= frec_medidas[frec_meass_index];
  _sendTime= frec_envios[frec_send_index];
  
   Serial.print("Ind Frec_meassure:"+String(frec_meass_index));
     Serial.println("  |Valor: "+String(frec_medidas[frec_meass_index]));
    Serial.print("Ind Send_meassure:"+String(frec_send_index));
         Serial.println("  |Valor: "+String(frec_envios[frec_send_index]));
 // Serial.println("al asignar indice: "+ String(frec_send_index)+"   _sendTime=" + String(frec_envios[frec_send_index]));
}


void updatefreq(unsigned long _localMeassureFreq, unsigned long _localSendFreq){   //???? NO ES NECESARIO PASARLE LOS ARGUMENTOS
  frec_medida = _localMeassureFreq;  //_localfreq = localconfig.measurefreq = Def_meassurefrec
  frec_envio = _localSendFreq; 
  
  frec_medidas[0]=frec_medida;  
  for ( int i = 1;i<4;i++){
    frec_medidas[i]=frec_medida/(i*2); 
    if(frec_medidas[i-1] < (i*2))
      frec_medidas[i]=frec_medidas[i-1];

    if (frec_medidas[i]<MIN_FREQ)
    frec_medidas[i]=MIN_FREQ;
  }

    
  iddleTime = frec_medida;
  
  Serial.println("[updatefreq()] el tiempo entre medidas es de: "+String(frec_medida));
  
///////////////////////////////////////////////
 unsigned long auxfreqenvio=frec_envio;

 while(auxfreqenvio>=frec_medidas[3])
 {
  auxfreqenvio=auxfreqenvio/2;
  n++;
 }

 frec_envios[0]=frec_envio;
 for ( int i = 1;i<=n;i++){
    frec_envios[i]=frec_envio/(i*2);   

   if (frec_envios[i]<MIN_FREQ)
    frec_envios[i]=MIN_FREQ;
    
 }
 frec_envios[n]=MIN_FREQ;
 _sendTime = frec_envio;

Serial.println("[updatefreq()] el tiempo entre envíos es de: "+String(_sendTime));
}


/////////////////////////////////////////////////////////////////////////////////////////////
String intToDigits(int dat, int n_digitos){ //a partir de un entero y un número de cífras devuelve un string con el número de cifras deseadas
  String temp = String(dat);
  if (!( temp.length()== n_digitos))
  {
    for ( int i = 0 ; i<n_digitos-temp.length();i++)
    {
      temp = "0"+temp;
    }
  }
  return temp;
}


#endif
