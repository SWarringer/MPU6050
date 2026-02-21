#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_bus.h"
#include "mpu6050.h"

static const char *TAG = "APP";

void app_main(void) {
  i2c_master_bus_handle_t bus = i2c_bus_init();
  i2c_master_dev_handle_t mpu = i2c_bus_add_device(bus, 0x68, 100000);

  mpu6050_init(mpu);

  mpu_vec3_t accel;
  mpu_vec3_t gyro;

  TickType_t last_wake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(10); // 100 Hz

  while (1) {
    int64_t start = esp_timer_get_time();

    mpu6050_read_accel(mpu, &accel);
    mpu6050_read_gyro(mpu, &gyro);

    ESP_LOGI(TAG, "A: %.2f %.2f %.2f | G: %.2f %.2f %.2f", accel.x, accel.y,
             accel.z, gyro.x, gyro.y, gyro.z);

    vTaskDelayUntil(&last_wake, period);

    int64_t end = esp_timer_get_time();
    ESP_LOGI(TAG, "Loop: %lld us", end - start);
  }
}
