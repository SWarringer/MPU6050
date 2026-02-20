#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/i2c_types.h"
#include "soc/clk_tree_defs.h"
#include <stdint.h>

#define I2C_PORT I2C_NUM_0
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define MPU6050_ADDR 0x68
#define I2C_FREQ_HZ 100000
static const char *TAG = "MPU6050";

i2c_master_bus_handle_t i2c_init_bus() {
  i2c_master_bus_config_t i2c_mst_config = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port = I2C_PORT,
      .sda_io_num = I2C_SDA_PIN,
      .scl_io_num = I2C_SCL_PIN,
      .flags.enable_internal_pullup = true,
  };

  i2c_master_bus_handle_t bus_handle;
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
  return bus_handle;
}

i2c_master_dev_handle_t i2c_add_device(i2c_master_bus_handle_t bus) {
  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = 0x68,
      .scl_speed_hz = 100000,
  };

  i2c_master_dev_handle_t dev_handle;
  ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_cfg, &dev_handle));
  return dev_handle;
}

esp_err_t wake_mpu(i2c_master_dev_handle_t dev) {

  uint8_t wake_cmd[2] = {0x6B, 0x00};
  esp_err_t ret;

  ret = i2c_master_transmit(dev, wake_cmd, 2, pdMS_TO_TICKS(100));
  return ret;
}

esp_err_t who_am_i(i2c_master_dev_handle_t dev, uint8_t *whoami) {

  uint8_t reg = 0x75;
  esp_err_t ret;

  ret =
      i2c_master_transmit_receive(dev, &reg, 1, whoami, 1, pdMS_TO_TICKS(100));
  return ret;
}

void app_main(void) {
  i2c_master_bus_handle_t bus = i2c_init_bus();
  i2c_master_dev_handle_t mpu = i2c_add_device(bus);

  esp_err_t ret = wake_mpu(mpu);
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "MPU6050 Wake up succeeded!");
  } else {
    ESP_LOGE(TAG, "MPU6050 Wake up failed!");
  }

  uint8_t whoami;
  ret = who_am_i(mpu, &whoami);
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "WHO_AM_I = 0x%02X", whoami);
  } else {
    ESP_LOGE(TAG, "Failed to read WHO_AM_I, err=%d", ret);
  }
}
