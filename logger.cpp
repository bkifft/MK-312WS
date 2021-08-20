#include <Arduino.h>
#include "logger.h"
#include <esp_task_wdt.h>

byte log_array[LOG_SIZE];
size_t log_insertion_index = 0;
bool log_new = false;
SemaphoreHandle_t  semaphore_logger;

void log(String msg)
{
  Serial.printf("logging: at %d %s\n",log_insertion_index,   msg.c_str());
  if (xSemaphoreTake(semaphore_logger, portMAX_DELAY) == pdTRUE)
  {
    for (int i = 0; i < msg.length(); i++)
    {
      log_array[log_insertion_index] = msg[i];
      log_insertion_index = (log_insertion_index + 1) % LOG_SIZE;
    }
    log_array[log_insertion_index] = '\n';
    log_insertion_index = (log_insertion_index + 1) % LOG_SIZE;
    log_new = true;
    xSemaphoreGive(semaphore_logger);
  }
}

void dump_log(byte * buffer_pointer, size_t buffer_size)
{
  if (!log_new)
  {
    return;
  }
 /* if (buffer_size < LOG_SIZE)
  {
    memset(buffer_pointer, '\0', buffer_size);
    return;
  }*/
  Serial.printf("dumping:\n");

  if (xSemaphoreTake(semaphore_logger, 0/*portMAX_DELAY*/) == pdTRUE)
  {
    // memcpy(dest src len);
    buffer_pointer[buffer_size - 1] = '\0'; 
    memcpy(buffer_pointer, log_array + log_insertion_index, LOG_SIZE - log_insertion_index);
    Serial.printf("logdump: memcpy 1: dest %p src %p len %d\n", buffer_pointer, log_array + log_insertion_index, LOG_SIZE - log_insertion_index);
    esp_task_wdt_reset();
    Serial.printf("logdump a: %s\n", buffer_pointer);
    memcpy(buffer_pointer + log_insertion_index, log_array, log_insertion_index);
    Serial.printf("logdump: memcpy 1: dest %p src %p len %d\n", buffer_pointer + log_insertion_index, log_array, log_insertion_index);

    Serial.printf("logdump b: %s\n", buffer_pointer + log_insertion_index);
    esp_task_wdt_reset();
    log_new = false;
    xSemaphoreGive(semaphore_logger);
  }
}

void init_logger()
{
  semaphore_logger = xSemaphoreCreateBinary();
  xSemaphoreGive(semaphore_logger);
  log_new = false;
  memset(log_array, '.',LOG_SIZE);

}
