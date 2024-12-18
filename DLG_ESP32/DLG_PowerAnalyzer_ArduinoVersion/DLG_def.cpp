#include "DLG_def.h"
#include "Arduino.h"


virtualPort vport0 = {PORT_J4, {BTN0, PIOE1, PIOE2}}; 
virtualPort vport1 = {PORT_PROG, {UESP32RX, UESP32TX}}; 
virtualPort vport2 = {PORT_MK1, {MK1_AN, MK1_RST, MK1_CS, MK1_PWM, MK1_INT, UMRX, UMTX}}; 
virtualPort vport3 = {PORT_MK2, {MK2_AN, MK2_RST, MK2_CS, MK2_PWM, MK2_INT}}; 
virtualPort vport4 = {PORT_EXP,  {EXPGPIO_O1,EXPGPIO_O2,EXPGPIO_O3,EXPGPIO_O4,EXPGPIO_I1,EXPGPIO_I2,EXPGPIO_I3,EXPGPIO_I4}}; 

virtualPort v_virtualPort[5] = {vport0,vport1,vport2,vport3,vport4}; // puntero usado en DigitalComm para poder acceder a los pines digitales de los distintos puertos virtuales
