//________________________________________________SEMAPHORES________________________________________________
volatile SemaphoreHandle_t BLE_Semaphore;   //Semaforo

void createSemaphores() {
  manager_semaphore = xSemaphoreCreateBinary();
}
