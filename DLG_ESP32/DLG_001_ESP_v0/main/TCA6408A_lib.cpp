#include <stdio.h>
#include <sys/_stdint.h>

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/projdefs.h"
#include "include/TCA6408A_lib.hpp"
#include "portmacro.h"
#include "ConfigBoard_DLG001.hpp"

static const char *TAG = "TCA6408_lib";

TCA6408A::TCA6408A(uint8_t address, i2c_port_t i2c_num)
    : i2caddr(address), i2cESP32Port(i2c_num), iodir_state(0xff) {}

// Obtiene la instancia actual de shrPrta
TCA6408A &TCA6408A::getInstance(uint8_t address, i2c_port_t i2c_num) {
  static TCA6408A instance(address, i2c_num); // Instancia estática
  return instance;
}

// Initialize I2C
void TCA6408A::begin(void) { _begin(); }


// configure port registers as if just power-on reset
void TCA6408A::_begin() {          
  ESP_LOGD(TAG, "TCA6408 BEGINING");
  reg_write(OREG, 0xff);		//OREG  REG : 0x01  OUTPUT PORT
  reg_write(VREG, 0x00);		//VREG  REG : 0x02  POLARITY INVERSION - NO INVERTIR POLARIDAD
  reg_write(IODIR, 0xff);		//IODIR REG : 0x03  CONFIGURATION - ALL CONFIGURADO COMO ENTRADA
  iodir_state = 0xff; //-> ALL ENTRADAS
  pinState = pinState_set();
} // _begin( )

//CONFIGURA DIRECCION DEL PIN LOGICA INVERSA 0 SALIDA 1 ENTRADA
void TCA6408A::pin_mode(uint8_t pinNum, bool mode) { 
  uint8_t mask = 1 << pinNum;
  if (mode == 1) { // OUTPUT
    iodir_state &= ~mask; //pone a 0 el bit indicado
  } else {
    iodir_state |= mask; 
  }
  reg_write(IODIR, iodir_state);
} // pin_mode( )



void TCA6408A::pin_write(uint8_t pinNum, bool level) {
  uint8_t mask = 1 << pinNum;
  if (level == 1) { // HIGH
    pinState |= mask;
  } else {
    pinState &= ~mask;
  }
  reg_write(OREG, pinState);
} // I2CxWrite( )

int TCA6408A::pin_read(uint8_t pinNum) {
  uint8_t mask = 0x1 << pinNum;
  uint8_t pinVal = reg_read(IREG);
  pinVal &= mask;
  if (pinVal == mask) {
    return 1;
  } else {
    return 0;
  }
} // pin_read( )

void TCA6408A::port_write(uint8_t i2cportval) {
  reg_write(OREG, i2cportval);
} // port_write( ) - public access

void TCA6408A::reg_write(uint8_t reg, uint8_t i2cportval) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  
  ESP_LOGD(TAG, "WRITING: %#X in reg: %d", i2cportval, reg);

  i2c_master_start(cmd); // comando de inicio de secuencia
  i2c_master_write_byte(
      cmd, ((i2caddr << 1) | I2C_MASTER_WRITE),
      true); // envio de dirección 7b con desplz para bit escritura/lectura
  i2c_master_write_byte(
      cmd, reg,
      true); // envio de registro en ancho soportado por dispositivo
  i2c_master_write_byte(cmd, i2cportval, true);
  i2c_master_stop(cmd);

  esp_err_t error = i2c_master_cmd_begin(
      i2cESP32Port, cmd,
      pdMS_TO_TICKS(timeout)); // espera cantidad equivalente en ticks a timeout
          
  i2c_cmd_link_delete(cmd);

  if (error != ESP_OK) {
    ESP_LOGE(TAG, "Error transmiting reg_write: %s", esp_err_to_name(error));
  }

  if (reg == OREG)				//ACTUALIZA EL ESTADO DE PINSTATE PARA PODER APLICARLE LA MASCARA CUANDO SE HAGAN CAMBIOS DE UN UNICO PIN
    pinState = pinState_set();
} // reg_write( ) - private


uint8_t TCA6408A::pinState_set() { return reg_read(IREG); } // pinState_set( )

uint8_t TCA6408A::reg_read(uint8_t reg) {
  
  ESP_LOGD(TAG, "READING reg: %d", reg);

  i2c_cmd_handle_t cmdRecept = i2c_cmd_link_create();

  i2c_master_start(cmdRecept); // comando de inicio de secuencia
  i2c_master_write_byte(cmdRecept, ((i2caddr << 1) | I2C_MASTER_WRITE), true);
  i2c_master_write_byte(cmdRecept, reg, true);
  i2c_master_stop(cmdRecept);
  
  esp_err_t error = i2c_master_cmd_begin(
  i2cESP32Port, cmdRecept, pdMS_TO_TICKS(timeout));
  
  i2c_cmd_link_delete(cmdRecept);
  
  if( error != ESP_OK){
	  ESP_LOGE(TAG, "Error transmiting reg_read: %s", esp_err_to_name(error));
  }
  
  cmdRecept = i2c_cmd_link_create();
  i2c_master_start(cmdRecept);
  i2c_master_write_byte(cmdRecept, ((i2caddr << 1) | I2C_MASTER_READ), true ); //solicitamos el dato
  
  uint8_t regState =0;
 
  i2c_master_read_byte(cmdRecept, &regState, I2C_MASTER_NACK );
  i2c_master_stop(cmdRecept);
  
  error = i2c_master_cmd_begin(
	  i2cESP32Port, cmdRecept, pdMS_TO_TICKS(timeout)); //timeout/portTICK_PERIOD_MS);
  
  i2c_cmd_link_delete(cmdRecept);
  
  if (error != ESP_OK){
	  ESP_LOGE(TAG, "Error reading reg_read: %s", esp_err_to_name(error));
  }
  else{
	    
	  ESP_LOGD(TAG, "REG STATE: %#X", regState);
  }
  

  return regState;
} // reg_read( )

uint8_t TCA6408A::iodir_read() { return iodir_state; } // iodir_read( )

void TCA6408A::iodir_write(uint8_t iodir) {
  reg_write(IODIR, iodir);
  iodir_state = iodir;
} // iodir_write( )

