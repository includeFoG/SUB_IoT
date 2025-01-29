#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp_log.h"

const char* lastSlash = strrchr(__FILE__, '\\');
const char* FILENAME = (lastSlash ? lastSlash + 1 : __FILE__);

#include "DLG_def.h"
#include "Config.h"

#include "taskReadRS485.h"





void setup() {
  initBoard();
  
  xTaskCreate(taskReadRS485,  "Task read RS485" ,  2048,  NULL,  1,  &readRS485_task_handle);
  //xTaskCreate(taskComs,  "Task coms" ,  2048,  NULL,  1,  &coms_task_handle);

}

void loop() { //Control task


}
