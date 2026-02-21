#include "i2c_bus.h"
#include "driver/i2c_types.h"
#include "esp_err.h"

i2c_master_bus_handle_t i2c_bus_init(void) {
  i2c_master_bus_config_t cfg = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port = I2C_NUM_0,
      .sda_io_num = 8,
      .scl_io_num = 9,
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
