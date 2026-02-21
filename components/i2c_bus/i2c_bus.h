#pragma once
#include "driver/i2c_master.h"

i2c_master_bus_handle_t i2c_bus_init(void);
i2c_master_dev_handle_t i2c_bus_add_device(i2c_master_bus_handle_t bus,
                                           uint8_t addr, uint32_t clk_speed);
