#pragma once
#include "driver/i2c_master.h"
#include "esp_err.h"

typedef struct {
  float x;
  float y;
  float z;
} mpu_vec3_t;

void mpu6050_init(i2c_master_dev_handle_t dev);
esp_err_t mpu6050_read_accel(i2c_master_dev_handle_t dev, mpu_vec3_t *accel);
esp_err_t mpu6050_read_gyro(i2c_master_dev_handle_t dev, mpu_vec3_t *gyro);
esp_err_t mpu6050_check_and_wake(i2c_master_dev_handle_t dev);
esp_err_t i2c_write_reg(i2c_master_dev_handle_t dev,
                        uint8_t reg,
                        uint8_t value);

esp_err_t i2c_read_reg(i2c_master_dev_handle_t dev,
                       uint8_t reg,
                       uint8_t *data,
                       size_t len);
