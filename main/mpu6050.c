#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#define I2C_PORT I2C_NUM_0
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define MPU6050_ADDR 0x68
#define I2C_FREQ_HZ 100000
static const char *TAG = "MPU6050";

void app_main(void) {
  esp_err_t ret;
  uint8_t who_am_i = 0;

  // Configure I2C
  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = I2C_SDA_PIN,
      .scl_io_num = I2C_SCL_PIN,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = I2C_FREQ_HZ,
  };
  i2c_param_config(I2C_PORT, &conf);
  i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);

  // Wake MPU6050
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, 0x6B, true); // PWR_MGMT_1
  i2c_master_write_byte(cmd, 0x00, true); // wake up
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(100));
  i2c_cmd_link_delete(cmd);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to wake MPU6050: %s", esp_err_to_name(ret));
    return;
  }

  vTaskDelay(pdMS_TO_TICKS(10)); // short delay for wake-up

  // Read WHO_AM_I
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, 0x75, true); // WHO_AM_I register
  i2c_master_start(cmd);                  // repeated start
  i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
  i2c_master_read_byte(cmd, &who_am_i, I2C_MASTER_NACK);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(100));
  i2c_cmd_link_delete(cmd);

  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "WHO_AM_I read failed: %s", esp_err_to_name(ret));
  } else {
    ESP_LOGI(TAG, "WHO_AM_I = 0x%02X", who_am_i);
  }

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
