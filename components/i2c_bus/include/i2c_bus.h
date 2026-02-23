#pragma once
#include "driver/i2c_master.h"
#include "esp_err.h"
#include <stdint.h>

i2c_master_bus_handle_t i2c_bus_init(void);
i2c_master_dev_handle_t i2c_bus_add_device(i2c_master_bus_handle_t bus,
                                           uint8_t addr, uint32_t freq_hz);
esp_err_t write_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t val);
esp_err_t read_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t *buf,
                   size_t len);
