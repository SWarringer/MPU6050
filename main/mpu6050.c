#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/clk_tree_defs.h"
#include <stdio.h>

#define I2C_PORT I2C_NUM_0
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define MPU6050_ADDR 0x68
#define I2C_FREQ_HZ 100000
static const char *TAG = "MPU6050";

void app_main(void) {
  int i2c_master_port = I2C_PORT;

  i2c_master_bus_config_t i2c_mst_config = {
      .i2c_port = I2C_PORT,
      .sda_io_num = I2C_SDA_PIN,
      .scl_io_num = I2C_SCL_PIN,
      .flags.enable_internal_pullup = true,
  };
}
