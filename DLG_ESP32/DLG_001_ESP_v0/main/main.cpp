
#include <stdio.h>

#include "../include/ConfigBoard_DLG001.hpp"
#include "freertos/FreeRTOS.h" 
#include "freertos/task.h"

#include "driver/gpio.h"

//#include "esp_partition.h"
#include "esp_log.h"
#include "include/DLG001_def.hpp"


#include "portmacro.h"
#include "sdkconfig.h"
//#include "led_strip.h"
//#include "driver/gpio.h"

//#include "Sensor.h"

//#include "freertos/semphr.h"


//semáforos de recursos compartidos
//SemaphoreHandle_t xSemaphore_i2c_0;  //mutex i2c_NUM_0



static const char *TAG = "main";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO (gpio_num_t)3

static uint8_t s_led_state = 0;


static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

/*static void initSemaphores(){
	xSemaphore_i2c_0 = xSemaphoreCreateMutex();
	if(xSemaphore_i2c_0 == NULL){
		ESP_LOGE(TAG, "ERROR CREANDO SEMAFORO, memoria insuficiente");
		while(1);
	}
}*/

extern "C" void app_main(void) 
{
	/*const esp_partition_t* part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "ffat");
    if (part) {
        ESP_LOGI("Partition", "FFAT Offset: %#lX, size: %#lX", (unsigned long)part->address, (unsigned long)part->size);
    }*/
    // initSemaphores();
    // if (xSemaphoreTake(xSemaphore_i2c_0, portMAX_DELAY) == pdTRUE) { //No
    // debería ser necesario al no estar iniciadas aún las otras tareas

    //    xSemaphoreGive(xSemaphore_i2c_0);
    //}
    initBoard();

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    PowerManager& superPower = PowerManager::getInstance(); // Obtén la única instancia
    superPower.initialize(0b1100000000001111, 0b1000000000000001);
   

    /*
   vTaskDelay(10000 / portTICK_PERIOD_MS);
   PowerManager& superPower2 = PowerManager::getInstance(); // Obtén la única
   instancia printf("INITIALIZED, NOW CHECK STATE SHOULD KEEP SAME");
   vTaskDelay(pdMS_TO_TICKS(1000));
   superPower2.initialize(0b100000000001111, 0b1000000000000101);*/

    /* Configure the peripheral according to the LED type */
    configure_led();

    while (1) {
      ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
      blink_led();
      /* Toggle the LED state */
      s_led_state = !s_led_state;
      vTaskDelay(
          2 * configTICK_RATE_HZ); // CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}

void vApplicationStackOverflowHook( TaskHandle_t xTask,char *pcTaskName ){
	
	for(;;){}									
}




/*
*******************************************************************TEST REGION*******************************************************************


    //----------------------------------------------- PRUEBA DE OBJETO POWER -----------------------------------------------
    printf("\nLATEST STATUS CHECK NOW\n");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    PowerManager& superPower = PowerManager::getInstance(); // Obtén la única instancia
    printf("\nGET INSTANCE 1, NOW CHECK STATE SHOULD BE OFF\n");
    vTaskDelay(2 * configTICK_RATE_HZ);
    superPower.initialize(0b1100000000001111, 0b1000000000000001);
    
    for(int i = 0; i<=15 ; i++)
    {
		superPower.enablePower( i); //0-15 debería saltar error entre [4 y 13]
		printf("TEST\n");
		vTaskDelay(2);
	}
	printf("TEST2\n");
	for(int i = 15; i>=0 ; i--)
    {
		superPower.disablePower(i); //debería saltar error entre [4 y 13]
		vTaskDelay(2);
	}
	printf("TEST3\n");
	superPower.enableMultiPower(0xFFFF); //HABILITA ALL		debería saltar error en sus respectivos no declarados como salida
	vTaskDelay(2);
	superPower.disableMultiPower(0xFFFF); //DESHABILITA ALL  debería saltar error en sus respectivos no declarados como salida
	vTaskDelay(2);
	printf("TEST4\n");
	
	superPower.enablePower(0); //habilita 0 
	vTaskDelay(2);
	printf("TEST5\n");
	superPower.enableMultiPower(0b0100000000001010); //HABILITA PARCIAL 
	vTaskDelay(2);
	superPower.disableMultiPower(0b0100000000001010); //DESHABILITA PARCIAL (0 sigue habilitado)
	vTaskDelay(2);
	printf("TEST6\n");
	superPower.setPortPowerStates(0b1000000000000101); //establece niveles
	vTaskDelay(2);
	superPower.enablePower(1); //habilita 1 
	vTaskDelay(2);
	printf("TEST7\n");
	superPower.disableMultiPower(0x8000); //DESHABILITA en_vsys
	vTaskDelay(2);
	printf("TEST8\n");
	superPower.setPortPowerStates(0xFF); //establece niveles debería saltar errores y solo establecer los que sí puede 
	vTaskDelay(2);
	printf("TEST9\n");
	superPower.setPortPowerStates(0x0); //establece niveles 
	vTaskDelay(2);
	//--------------------------------------------- FIN PRUEBA DE OBJETO POWER---------------------------------------------
	
	
	

*/