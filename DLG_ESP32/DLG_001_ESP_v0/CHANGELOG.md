

[21/11/2024] Terminada primera versión de todas las funciones PowerManager[V0]
    	- Meter filtro de powersDir como filtro para evitar poner a nivel alto entradas de TCA como Otras alimentaciones u otros pines[done]


ERRORES:
	·Parece que después de modificar los archivos de arduino a c/c++ en la creación del objeto power hay un desbordamiento de pila
		En ocasiones arroja este error aunque no llegue a pararse el programa
			Break at address "0x4037889e" with no debug information available, or outside of program code.
		Puede que esté relacionado con la declaración de la variable superpower y este error:
			Multiple errors reported.
			
			1) Failed to execute MI command:
			-var-create - * superPower
			Error message from debugger back end:
			value has been optimized out
			
			2) Unable to create variable object
			
			3) Failed to execute MI command:
			-var-create - * superPower
			Error message from debugger back end:
			value has been optimized out
			
		-Puede deberse a la estructura de la memoria
			
			
	·[SOLVED]Después de configurar el i2c con set_i2c() dentro del mismo cpp que hace la configuración funciona bien a la hora de hacer el scanner_i2c
		pero desde otros archivos es como si no estuviese configurado.
		[SOLUCION] NO HABIA CONFIGURADO EL PIN_MODE.
	
	