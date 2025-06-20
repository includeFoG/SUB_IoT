bool com_event = false;
bool f_finMedidas = false;

//hasta 24 bits por evento

#define BIT_0_createFile      ( 1 << 0 )
#define BIT_1_closeFile       ( 1 << 1 )
#define BIT_2_SendData        ( 1 << 2 )
#define BIT_4_takeGps         ( 1 << 4 )
#define BIT_5_startMeasuring  ( 1 << 5 ) //!!! NO SE ESTA USANDO
#define BIT_6_listFile        ( 1 << 6 ) //!!! NO SE ESTA USANDO
#define BIT_7_updateTime      ( 1 << 7 )
#define BIT_8_chSendData      ( 1 << 8 )


//flags for file transfer
#define BIT_9_sendFile        ( 1 << 9 )
#define BIT_12_rdyFile        ( 1 << 12 )
#define BIT_13_doneFileSend   ( 1 << 13 )
#define BIT_14_doneFileRead   ( 1 << 14 )
#define BIT_15_updateRdy      ( 1 << 15 )
#define BIT_16_sendFileOK     ( 1 << 16 )
#define BIT_17_snapFileToSend ( 1 << 17 )

#define BIT_10_saveData      ( 1 << 10 )
#define BIT_11_iddle         ( 1 << 11 )

#define BIT_18_ubidotsSwitch ( 1 << 18 ) //primer flag que se activa cuando detecta siwtch camara ubidots encendido
#define BIT_19_IddleYNoSend  ( 1 << 19 ) //wait4 añadido para waitbits en tasfile
#define BIT_20_SendUbi       ( 1 << 20 )


EventGroupHandle_t myEvents;


//#define BIT_0_analogReading     (1<<0) //MODIFIED
//#define BIT_1_comModule         (1<<1)
//#define BIT_2_fileModule        (1<<2)
//#define BIT_3_gpsModule         (1<<3)



//EVENTGROUP: eventPwr5volts                 //ADDED
#define BIT_0_changePwr         (1<<0)
#define BIT_1_comModule         (1<<1)
#define BIT_2_fileModule        (1<<2)
#define BIT_3_gpsModule         (1<<3)
#define BIT_4_analogReading     (1<<4)
#define BIT_5_buzzer            (1<<5)// #define BIT_5_pwrMdvr           (1<<5) //NO SE ESTA USANDO desde que se quito accionMdvr(), se sustituye por BIT para BUFFER

//placa actuation                            //ADDED
#define BIT_6_actuation      (1<<6)    //ALIMENTACION PLACA ACTUACION

#define BIT_7_S0_F           (1<<7)    //F_out[0]   --SIN USO--
#define BIT_8_S1_E           (1<<8)    //E_out[1]   SENSORES VELA 2 P1V2         [X]
#define BIT_9_S2_D           (1<<9)    //D_out[2]   --SIN USO--
#define BIT_10_S3_C          (1<<10)   //C_out[3]   --SIN USO--    
#define BIT_11_S4_B          (1<<11)   //B_out[4]   --SIN USO--
#define BIT_12_S5_A          (1<<12)   //A_out[5]   --SIN USO--
#define BIT_13_S6_RS1        (1<<13)   //SSR_7[6]   CAMARAS VELA 2 P11V2         [SU]
#define BIT_14_S7_R1         (1<<14)   //RLY1[7]    --SIN USO--
#define BIT_15_S8_R2         (1<<15)   //RLY2[8]    --SIN USO--
#define BIT_16_S9_RS2        (1<<16)   //SSR_10[9]  CAMARAS VELA 1 P11V1         [SU]
#define BIT_17_S10_L         (1<<17)   //L_out[10]  DVR                          [SU]
#define BIT_18_S11_K         (1<<18)   //K_out[11]  --SIN USO--
#define BIT_19_S12_J         (1<<19)   //J_out[12]  --SIN USO--    
#define BIT_20_S13_I         (1<<20)   //I_out[13]  --SIN USO--
#define BIT_21_S14_H         (1<<21)   //H_out[14]  SENSORES VELA 1 P1V1        [X]
#define BIT_22_S15_G         (1<<22)   //G_out[15]  LOADSHACKLE + TODO NADA     [X] [SU]

#define BIT_23_UBIFLAG_ON      (1<<23) //ADDED flag de ubidots switch(ON), se activa a partir de BIT_18_ubidotsSwitch [X]


EventGroupHandle_t eventPwr5volts;

//ADDED WORKING byte para controlar el estado de los hilos
#define BIT_0_taskControl  (1<<0)
#define BIT_1_taskPwr      (1<<1)
#define BIT_2_taskAna      (1<<2)
#define BIT_3_taskFile     (1<<3)
#define BIT_4_taskComm     (1<<4)
////////////COMM_STATUS//////////
#define BIT_5_IM_updateTime 
#define BIT_6_IM_sendData (1<<6)  //USADO PARA BUZZER
#define BIT_7_IM_sendAWS
#define BIT_8_IM_waitCom
////////////FILE_STATUS//////////
#define BIT_9_IM_createFile
#define BIT_10_IM_saveData 
#define BIT_11_IM_sendFile
#define BIT_12_IM_deleteName //sendFileOk
#define BIT_13_IM_iddleYNoSend
#define BIT_14_IM_waitFile
/////////////ANA_STATUS/////////
#define BIT_15_IM_sendData
#define BIT_16_IM_saveData (1<<16)  //USADO PARA BUZZER
#define BIT_17_IM_waitAna

////////////GENERAL_STATUS/////
#define BIT_22_IM_resetCom
#define BIT_23_IM_resetLowMem
EventGroupHandle_t eventTaskState;
//END ADDED



//!!!MODIFICAR NOMBRES PARA QUE CONCUERDEN        //ADDED CONFIG
//#define BIT_0_  ( 1 << 0 )
//#define BIT_16_checkConfig      ( 1 << 1 )     //Comprueba si existe nuevo archivo de configuración en AWS (Ctrl-taskComm)
//#define BIT_17_newConfig        ( 1 << 2 )     //Se activa cuando hay nuevo archivo de configuración en AWS(taskComm-Ctrl)
//#define BIT_18_createConfigAWS  ( 1 << 3 )     //Avisa al hilo de file para crear nuevo archivo en SD para config AWS(Ctrl-taskFile)
//#define BIT_19_saveConfigAWS    ( 1 << 4 )     //Descarga la información de AWS en el archivo generado en la SD (Ctrl-taskComm-taskFile)
//#define BIT_20_reset            ( 1 << 5 )     //Reinicia la placa
//#define BIT_21_readyConfig0     ( 1 << 6 )     //Indica que se ha cargado la configuración desde archivo config.txt SD o en su defecto la Default
//#define BIT_22_deleteConfigAWS  ( 1 << 7 )     //Al activarse indica que la descarga ha sido satisfactoria y se puede eliminar el archivo de configuración subido a AWS

//EventGroupHandle_t configEvents; //antes EventGroupHandle_t myEvents;

/* Attempt to create the event group. */

void createEvents() {
  myEvents = xEventGroupCreate();
  eventPwr5volts = xEventGroupCreate();
  eventTaskState = xEventGroupCreate(); //ADDED
  //configEvents = xEventGroupCreate(); //ADDED CONFIG
  /* Was the event group created successfully? */
  if ( myEvents == NULL )
  {
    SERIAL.println("Fail creating Events0");
  }
  if ( eventPwr5volts == NULL )
  {
    SERIAL.println("Fail creating Events1");
  }
  if ( eventTaskState == NULL)
  {
    SERIAL.println("Fail creating Events2");
  }
  //    if (configEvents == NULL) //ADDED CONFIG
  //    {
  //      SERIAL.println("Fail creating Events2");
  //    }
}
