#ifndef TCA6408A_h
#define TCA6408A_h

#include "driver/i2c.h"
#include "hal/i2c_types.h"


// port register names
#define IREG 0x00	// input port location  -	READ ONLY - NIVELES DE ENTRADA (TAMBIEN DA INFO OUTPUT)
#define OREG 0x01	// output port			-	NIVEL DE LOS PINES OUTPUT default 0xFF
#define VREG 0x02	// polarity inversion	-	PERMITE INVERTIR LA POLARIDAD DE LOS PINES DEFINIDOS COMO ENTRADAS
#define CREG 0x03	// configuration reg - IODIR - CONFIGURA LA DIRECCION DE LOS PINES 1-ENTRADA CON ALTA IMPEDANCIA 0 - SALIDA
#define IODIR 0x03	// common alias for config

#define timeout 1000 //timeout i2c ms




class TCA6408A {
public:
	
	static TCA6408A& getInstance(uint8_t address, i2c_port_t i2c_num = I2C_NUM_0); //AÑADIDO	
			   
	TCA6408A(const TCA6408A&) = delete; 				//eliminamos el constructor de copia
	TCA6408A& operator = (const TCA6408A&) = delete;    //eliminamos asignación por defecto del compilador
			   
	// Wire function setup port
	void begin(void);

	void pin_mode(uint8_t pinNum, bool mode);
	void pin_write(uint8_t pinNum, bool level);
	int  pin_read(uint8_t pinNum);
	// read initial value for pinState
	uint8_t pinState_set( );
	// write a whole byte or word (depending on the port expander chip) to i2c
	void port_write( uint8_t i2cportval );  //este puerto se refiere a los registros del dispositivo
	// access functions for IODIR state copy
	uint8_t iodir_read( );
	void iodir_write( uint8_t iodir );
	void reg_write( uint8_t reg, uint8_t i2cportval );
	uint8_t reg_read( uint8_t reg );

private:
	TCA6408A( uint8_t address, i2c_port_t i2c_num = I2C_NUM_0); //AÑADIDO
			   
    // I2C device address
    uint8_t i2caddr;
	i2c_port_t i2cESP32Port;
    
	// I2C pin_write state persistant storage
	// least significant byte is used for 8-bit port expanders
	uint8_t pinState;
	uint8_t iodir_state;    // copy of IODIR register
	void _begin( void );
};


#endif // TCA6408A_h


