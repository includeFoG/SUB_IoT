#include "../include/ConfigBoard_DLG001.hpp"

#include "DLG001_def.hpp"
#include "TCA6408A_lib.hpp"
#include "driver/i2c.h"
#include "driver/uart.h"

#include "esp_err.h"
#include "freertos/projdefs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hal/gpio_types.h"

#include "esp_log.h"
#include <sys/types.h>

static const char *TAG = "ConfigBoard";

void programStart() {
  printf("\n\n*********************************************\n");
  printf("        Program start        \n");
  // printf(FILENAME);
  printf("%s\n", DEVICE_NAME);
  printf("*********************************************\n");
}


static esp_err_t set_i2c(void) {  //ANTES ESTABA DECLARADA COMO STATIC Y ESO DA FALLO AL CONFIGURAR UN RECURSO GLOBAL
  i2c_config_t i2c_config = {};

  i2c_config.mode = I2C_MODE_MASTER;
  i2c_config.sda_io_num = SDA;
  i2c_config.scl_io_num = SCL;
  i2c_config.sda_pullup_en = false;
  i2c_config.scl_pullup_en = false;
  i2c_config.master.clk_speed =
      100000; // compatible con todos los dispositivos de la placa
  i2c_config.clk_flags =
      I2C_SCLK_SRC_FLAG_FOR_NOMAL; // 0: cualquier reloj disponible con la freq
                                   // indicada
 
  esp_err_t error;
  error = i2c_param_config(I2C_NUM_0, &i2c_config);
  if(error != ESP_OK) return 1;
  error = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0,
                                     ESP_INTR_FLAG_LEVEL1);
  if(error != ESP_OK) return 2;

  return ESP_OK;
}

void scanI2CDevices() {
  uint8_t address;
  int nDevices = 0;

  printf("\nScanning for I2C devices ...\n");
  for (address = 0x01; address < 0x7f; address++) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); 	// permite construir secuencias de comandos de
                               						// i2c (generar tramas, escrituras, lecturas...)
    i2c_master_start(cmd);     												// comando de inicio de secuencia (trama)
    i2c_master_write_byte( cmd,
    	(address << 1) | I2C_MASTER_WRITE,
        true); 																	// Escribe la dirección del dispositivo (con el bit de escritura)
    i2c_master_stop(cmd); 													// Añadimos fin de transmisión

    esp_err_t error = i2c_master_cmd_begin(
        I2C_NUM_0, cmd,
        1000 / portTICK_PERIOD_MS); 										// Envía el comando al bus I2C, espera
                                    													// maximo 1000ms en ticks
    i2c_cmd_link_delete(cmd);

    if (error == ESP_OK) {
      ESP_LOGD(TAG, "I2C device found at address 0x%02X", address);
      nDevices++;
    } else if (error == ESP_ERR_TIMEOUT) {
      ESP_LOGE(TAG, "Error %d at address 0x%02X", error, address);
    }
  }
  if (nDevices == 0) {
    ESP_LOGE(TAG, "\nNo I2C devices found\n");
  }
}

static esp_err_t init_pin_config(void) {
  esp_err_t error;
  // Configuración de los pines de selección del multiplexor de UART
  error = gpio_reset_pin(SEL0);
  if(error != ESP_OK) return 1;
  error = gpio_reset_pin(SEL1);
  if(error != ESP_OK) return 2;

  error = gpio_set_direction(SEL0, GPIO_MODE_OUTPUT);
  if(error != ESP_OK) return 3;
  error = gpio_set_direction(SEL1, GPIO_MODE_OUTPUT);
  if(error != ESP_OK) return 4;

  // inicialización del multiplexor con salida 0 -> UART A Mikroe2
  error = gpio_set_level(SEL0, 0);
  if(error != ESP_OK) return 5;
  error = gpio_set_level(SEL1, 0);
  if(error != ESP_OK) return 6;

  return ESP_OK;
}

/*static esp_err_t config_uart1(uart_port_t uartDefined, int baudRate) {
  uart_config_t uart_config = {
      .baud_rate = baudRate,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
  };
  
  esp_err_t error;

  error = uart_param_config(uartDefined, &uart_config);
  if(error != ESP_OK) return 1;
  error = uart_set_pin(uartDefined, UMTX, UMRX, UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE);
  if(error != ESP_OK) return 2;
  // 0 en tx_buff los datos se envían sin almacenar en buffer, sin cola de
  // eventos UART
  error = uart_driver_install(uartDefined, 1024, 0, 0, NULL, 0);
  if(error != ESP_OK) return 3;

  return ESP_OK;
}*/



void initBoard() {

  set_i2c();
  init_pin_config();

#if USE_MIKROE_1
  config_uart1(UART_NUM_1, MIKROE1_BAUDRATE);
#endif
  mux_uart starter_uart =
      MIKROE_2; // definimos una uart de inicio para selectUart()
  selectUart(starter_uart);

  vTaskDelay(0.5*configTICK_RATE_HZ);
  scanI2CDevices();
  programStart();
}

esp_err_t selectUart(uint8_t dir) {
	esp_err_t error = 1;
  switch (dir) {
  case MIKROE_2: // MIKROE2                                    UART1   LOW-LOW
    error = gpio_set_level(SEL0, 0);
    error = gpio_set_level(SEL1, 0);
    ESP_LOGD(TAG, "UART_MIKROE2_ENABLED");
    break;
  case RS_232_ch1: // RS232_channel1 U2Rx(R1Out) U2Tx(T1In)    UART2   LOW-HIGH
    error = gpio_set_level(SEL0, 0);
    error = gpio_set_level(SEL1, 1);
    ESP_LOGD(TAG, "UART_RS_232_ch1_ENABLED");
    break;
  case RS_232_ch2: // RS232_channel2 U3Rx(R2Out) U3Tx(T2In)    UART3   HIGH-LOW
    error = gpio_set_level(SEL0, 1);
    error = gpio_set_level(SEL1, 0);
    ESP_LOGD(TAG, "UART_RS_232_ch2_ENABLED");
    break;
  case RS_485: // RS485                                   	   UART4
               // HIGH-HIGH
    error = gpio_set_level(SEL0, 1);
    error = gpio_set_level(SEL1, 1);
    ESP_LOGD(TAG, "UART_RS_485_ENABLED");
    break;
  }
  
  return error;
}

// ***********************************************************************
//  ESPACIO DE CLASE: [PowerManager]
// ***********************************************************************

bool PowerManager::initialized = false;  // Inicializamos en false
bool PowerManager::initZeroFlag = false; // Inicializamos en false



PowerManager::PowerManager(uint16_t powersDir, uint16_t initStates)
    : powersDir(powersDir), powerStates(initStates),
      expGPIOS(TCA6408A::getInstance(EXP_GPIO_ADDR, I2C_NUM_0))
       {
  if (!initZeroFlag) {
    initialize(
        powersDir, initStates,
        true); // efecto de "softreset" al TCA con los valores establecidos
  }
}




uint8_t PowerManager::reverseByte(uint8_t byteToReverse) {
  uint8_t reversedByte = 0;
  for (int i = 0; i < 8; i++) {
    reversedByte |= (((byteToReverse >> i) & 1) << (7 - i));
  }
  ESP_LOGD(TAG,"ReversedByte: %#X if >>4: %#X ", reversedByte, reversedByte>>4);
  return reversedByte;
}

uint8_t PowerManager::reverseIndex(uint8_t indexToReverse){ // VALIDO SOLO PARA INDICES DE 0 a 3
	//EN LA DLG_001_ESP las salidas del TCA6408A están al revés respecto a la nomenclatura de la PCB (ver .hpp)
	uint8_t binaryIndex = 1 << indexToReverse; //pasamos a binario el índice: 4 = b1000
	uint8_t reversedBinaryIndex = (reverseByte(binaryIndex) >> 4); // 4 = b0001
	
	if(reversedBinaryIndex==0){
		ESP_LOGE(TAG,"ERROR, NO SE PUEDE OBTENER UN INDICE DE 0x0");
		return 255;
	}
	
	//ahora necesitamos saber el indice al que corresponde con el orden inverso:
	for(int i = 0 ; i<=8 ; i++)
	{
		if((reversedBinaryIndex >> (i+1)) == 0)
		{
			indexToReverse = (i);
			break;
		}
	}
	ESP_LOGD(TAG,"ReversedIndex: %d", indexToReverse);
	return indexToReverse;
}

PowerManager &PowerManager::getInstance() {
  static PowerManager instance(0, 0); // Instancia estática
  return instance;
}

/*
   Esta función se utiliza para inicializar el expansor a un estado deseado.
   initZero = true; -> realiza una configuración CERO , como si se quitase la
   alimentación al TCA, no modifica el valor de initialized initZero = false; ->
   configura el TCA con los valores enviados, establece initialized a true
*/
void PowerManager::initialize(uint16_t powersDir, uint16_t initStates,
                              bool initZero) {
ESP_LOGD(TAG,"\ninitialized: %d, initzero: %d", this->initialized, initZero);
  if (!this->initialized || initZero) {
	ESP_LOGD(TAG,"INICIALIZANDO POWERMANAGER\n");
    //this->powers = powers;
    this->powerStates = initStates;

    if (0x8000 & powersDir) // habilitado bit Vsys si está habilitado bit 7
    {
      ESP_ERROR_CHECK(gpio_reset_pin(EN_VSys));
      ESP_ERROR_CHECK(gpio_set_direction( EN_VSys, GPIO_MODE_OUTPUT)); // CONFIGURAMOS COMO SALIDA
      if (0x8000 & initStates) {
        ESP_ERROR_CHECK(gpio_set_level(EN_VSys, 1));
        ESP_LOGI(TAG,"EN_VSys ENABLED\n");
      }
    }
     if (0x4000 & powersDir) // habilitado bit Vsys si está habilitado bit 7
    {
      ESP_ERROR_CHECK(gpio_reset_pin(EN_VCC_MIKROE));
      ESP_ERROR_CHECK(gpio_set_direction( EN_VCC_MIKROE, GPIO_MODE_OUTPUT)); // CONFIGURAMOS COMO SALIDA
      if (0x4000 & initStates) {
        ESP_ERROR_CHECK(gpio_set_level(EN_VCC_MIKROE, 1));
        ESP_LOGI(TAG,"EN_VCC_MIKROE ENABLED\n");
      }
    }
    
    // en primer lugar inicializamos la comunicación con el expansor de GPIOS:
    this->expGPIOS.begin();

    uint8_t TPS4H160Powers = (powersDir & 0xFF); // conservamos solo el primer byte
    uint8_t TPS4H160InitState = (initStates & 0xFF);

    // nos aseguramos de configurar según nos informan de los powers
    // ya que solo hay 4 salidas, eliminamos de powers los posibles bytes
    // enviados fuera del rango
    //ESP_LOGD(TAG,"powersDirout: %#X CONFIG_POWERS_BOARD: %#X", powersDir, CONFIG_POWERS_BOARD); //DELETE
    this->powersDir = (powersDir & CONFIG_POWERS_BOARD);
    //ESP_LOGD(TAG,"this->powersDir %#X", this->powersDir); //DELETE

    ESP_LOGI(TAG, "TPS4H160Powers: %#X\tTPS4H160InitState: %#X\n", TPS4H160Powers,TPS4H160InitState);

    TPS4H160InitState = (reverseByte(TPS4H160InitState)) >> 4;
    // el >>4 es DEPENDIENTE DE HARDWARE por como se ha ordenado para que cuadre la nomenclatura en placa

    uint8_t maskStart = 0b1; //va desplazandose bit a bit para comprobar los estados

    for (int i = 0; i < 4; i++) 
    {
      if (TPS4H160Powers & maskStart) 
      {
        uint8_t auxBinResult = 0;
        this->expGPIOS.pin_mode(i, 1); // definimos el DOut como salida power
        ESP_LOGI(TAG, "Inicializando como power DIGOut: %d", i);

        auxBinResult = TPS4H160InitState & maskStart;

        this->expGPIOS.pin_write(i,
                           bool(auxBinResult)); // configuramos el estadoinicial 

      	ESP_LOGI(TAG, "Configurando DIDOut a nivel: %d \t mediante maskStart: %#X  TPS4H160InitState & maskStart: %#X",
                 bool(auxBinResult), maskStart, auxBinResult);

     }
      maskStart <<= 1;
      
    }
    
	this->showStatus();
	
    if (!initZero) {
      initialized = true;
    }
    else {
      this->initZeroFlag = true;
    }
  }
  else {
    ESP_LOGE(TAG," SINGLETON CLASS NOT INITIALIZED");
  }
}


//WORKING //habilita la salida indicada (solo 1 modo seguro)
/*		Se ha definido como uint16_t porque realmente TCA6408A podría controlar 8 salidas
*													 
*
*					[indexToEnable]:
*  BIT   | 			power Related		|	Other

*	0	 |	_DIG_OUT1 -> (S1) CN1 P6  	|	TCA6408A P3 -> TPS4H160 DO_01 -> IN1  								
*	1	 |	_DIG_OUT2 -> (S2) CN3 P6	|   TCA6408A P2 -> TPS4H160 DO_02 -> IN2							 
*	2	 |	_DIG_OUT3 -> (S3) CN5 P6	|	TCA6408A P1 -> TPS4H160 DO_03 -> IN3  													
*	3	 |	_DIG_OUT4 -> (S4) CN6 P6	|	TCA6408A P0 -> TPS4H160 DO_04 -> IN4 					
*   ...
*   14   |		EN_VCC_MIKROE			|   
*	15	 |      EN_Vsys	   			    |
*
* 
* la función espera un ENTERO de 0 a 15
* que simbolice el índice del bit en un uint16_t por ejemplo si se le pasa un 9 equivaldrá a BIT 9 = 0b0010 0000 0000
*/
uint8_t PowerManager::enablePower(uint8_t indexToEnable)
{  	
		ESP_LOGI(TAG,"ENABLING OUTPUT: %d",indexToEnable);
		if(indexToEnable <= 15)
		{   
			if(this->configOutput( indexToEnable, 1) == 0) 	// si todo ha ido bien
				this -> powerStates |= (1<<indexToEnable); 				//actualiza valor de powerState
			else{return 2;}
		}
		else
		{
			ESP_LOGE(TAG, "OUT OF RANGE enablePower");
			return 1;
		}
		this->showStatus();
		return 0;
} 

uint8_t PowerManager::configOutput(uint8_t indexToEnable, bool status) // único capaz de acceder a pin_write
{
	if(this->powersDir & (1<<indexToEnable))
	{
		if(indexToEnable < 4) //si es <4 está controlado por el TCA6408A
		{
			this->expGPIOS.pin_write( reverseIndex(indexToEnable),  status);
		}
		else{ //OTRAS ALIMENTACIONES
			esp_err_t ret ;
			
			switch(indexToEnable)
			{
				case 14: //EN_VCC_MIKROE
					ret = gpio_set_level(EN_VCC_MIKROE, uint32_t(status));
					if(ret != ESP_OK) return 2;
					ESP_LOGI(TAG,"EN_VCC_MIKROE %s\n",status? "ENABLED":"DISABLED" );
					break;
				case 15:	//EN_VSys
					ret = gpio_set_level(EN_VSys, uint32_t(status));
					if(ret != ESP_OK) return 2;
					ESP_LOGI(TAG,"EN_VSys %s\n",status? "ENABLED":"DISABLED");
					break;
					
				default: //BITS NO DEFINIDOS
					ESP_LOGE(TAG,"power-bit not in use: %#X ", (1<<indexToEnable));
					return 1;
			}
		}
		return 0;
	}	
	else{
		ESP_LOGE(TAG,"%d NO ESTA DECLARADO COMO SALIDA: %#X",indexToEnable,this->powersDir);
	}
	return 255;
}


uint8_t PowerManager::enableMultiPower(uint16_t bitsToEnable)     //habilita las salidas indicadas, las que estén a 0 las deja como esten
{
	ESP_LOGI(TAG,"ENABLING OUTPUTS: %#X",bitsToEnable);
	for(int i=0; i<=15; i++)
	{
		if(bitsToEnable & (1<<i)) //si está a nivel alto
		{
			ESP_LOGI(TAG,"ENABLING: %d",i);
			esp_err_t error =(this->configOutput( i, 1));
			switch(error){
				case 0: //la manda a nivel alto
					this-> powerStates |= (1<<i); //Si ha ido bien actualiza powerStates, lo hacemos por partes por si falla
					break;
				case 2: //error
					return 1;
				default: // ===1 bit not in use || not enabled cause not OUTPUT
					break;
			}
		}
	}
	this->showStatus();
	return 0;
}

/*establece el estado de TODAS las salidas como el indicado (PORT)
* En caso de que alguno de los bits que se quiera habilitar NO SEA UNA SALIDA informará y lo omitirá activando el resto
* Si se intenta poner a nivel alto una entrada cambiará el bit a 0 e informará con el bit X de Y no es una salida
* Si se intenta poner a nivel bajo una entrada informará con NO ESTA DECLARADO COMO SALIDA
*/
uint8_t PowerManager::setPortPowerStates(uint16_t powerStatus)          
{
	for(int i=0; i<15; i++){
		if((powerStatus & (1 << i)) && !(this->powersDir & (1<<i))) //si el que es 1 el "bitToEnable" y el "powersDir" es 0 -> NO ES SALIDA
		{
			ESP_LOGE(TAG, "El bit %d de: %#X NO ES UNA SALIDA -> %#X", i, powerStatus, this->powersDir);
			//return 255; EN LUGAR DE SALTAR UN ERROR SE VA A INFORMAR SE VA A QUITAR Y SE VA A CONTINUAR
			powerStatus = (powerStatus & ~ (1<<i));
		}
	}
	
	ESP_LOGI(TAG,"SETING POWER STATUS: %#X",powerStatus);
	//if(powerStatus & 0xFF) 
	//{
		//si hay señales activas en el primer byte, las gestiona el TCA6408A
		this->expGPIOS.port_write((reverseByte(powerStatus & 0xFF) >> 4)); //configuramos todo el port del TCA6408A
		this-> powerStates = (powerStates & 0xFF00) | (powerStatus & 0x00FF) ; //nos quedamos con los bits más altos borrando los inferiores y le añadimos los nuevos valores inferiores
	//}
	//if(powerStatus & 0xFF00)	//la parte alta OTRAS ALIMENTACIONES NO gestionadas por TCA6408A
	//{  
		for(int i = 8 ; i<= 15 ; i++) //configuramos todos los bits de la parte alta
		{
			esp_err_t error =this->configOutput( i, bool(powerStatus & (1 << i)) );
			
			switch(error){
				case 0: 
					if(bool(powerStatus & (1 << i))) //si se pone a 1
					{
						this -> powerStates |= (1<<i);
					}
					else //si se pone a 0
					{
						this -> powerStates &= ~ (1<<i);
					}
					break;
				case 2: //error
					return 1;
				default: // ===1 bit not in use || not enabled cause not OUTPUT
					break;
			}
		}
	//}
	this->showStatus();
	return 0;
}


uint8_t PowerManager::disablePower(uint8_t indexToDisable)    //deshabilita la salida indicada (solo1 modo seguro)
{
	ESP_LOGI(TAG,"DISABLING OUTPUT: %d",indexToDisable);
	if(indexToDisable <=15)
	{   
		if((this->configOutput( indexToDisable, 0)) == 0){
			this -> powerStates &= ~ (1<<indexToDisable);
		}
		else{return 2;}
	}
	else
	{
		ESP_LOGE(TAG, "OUT OF RANGE disablePower");
		return 1;
	}
	this->showStatus();
	return 0;
}


uint8_t PowerManager::disableMultiPower(uint16_t bitsToDisable)  //WORKING //deshabilita las salidas indicadas
{
	ESP_LOGI(TAG,"DISABLING OUTPUTS: %#X",bitsToDisable);
	for(int i=0; i<=15; i++)
	{
		if(bitsToDisable & (1<<i)) //si está indicada
		{
			ESP_LOGI(TAG,"DISABLING: %d",i );
			esp_err_t error =this->configOutput( i, 0);
			
			switch(error){
				case 0: //la manda a nivel alto
					this -> powerStates &= ~ (1<<i); //Si ha ido bien actualiza powerStates, lo hacemos por partes por si falla
					break;
				case 2: //error
					return 1;
				default: // ===1 bit not in use || not enabled cause not OUTPUT
					break;
			}
		}
	}
	this->showStatus();
	return 0;
}


void PowerManager::showStatus(){
	ESP_LOGD(TAG,"powersDir: %#x\t|\tpowerState: %#x",powersDir, powerStates);
}

