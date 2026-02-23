#include "i2c_bus.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>

#define I2C_PORT I2C_NUM_0
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9

i2c_master_bus_handle_t i2c_bus_init(void) {
  i2c_master_bus_config_t cfg = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port = I2C_PORT,
      .sda_io_num = I2C_SDA_PIN,
      .scl_io_num = I2C_SCL_PIN,
      .flags.enable_internal_pullup = true,
  };
  i2c_master_bus_handle_t bus;
  ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &bus));
  return bus;
}

i2c_master_dev_handle_t i2c_bus_add_device(i2c_master_bus_handle_t bus,
                                           uint8_t addr, uint32_t freq_hz) {
  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = addr,
      .scl_speed_hz = freq_hz,
  };
  i2c_master_dev_handle_t dev;
  ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_cfg, &dev));
  return dev;
}

esp_err_t write_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t val) {
  uint8_t buf[2] = {reg, val};
  return i2c_master_transmit(dev, buf, 2, pdMS_TO_TICKS(100));
}

esp_err_t read_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t *buf,
                   size_t len) {
  return i2c_master_transmit_receive(dev, &reg, 1, buf, len,
                                     pdMS_TO_TICKS(100));
}
