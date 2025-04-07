#ifndef GEST_TIME
#define GEST_TIME


int MIN_FREQ = 10;

unsigned long frec_medida = 10000;
unsigned long frec_medidas[4] = {frec_medida,frec_medida/2,frec_medida/4,frec_medida/8};
unsigned long frec_envio = 300;
unsigned long frec_envios[15]; //SE HACE NECESARIO INICIALIZAR EL VECTOR DE ENVIOS ANTES DE SABER SU TAMAÑO. VECTOR LIMITADO A 28h aprx
int n=0;

unsigned long iddleTime = frec_medida;
unsigned long _sendTime = frec_envio;



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




#endif
