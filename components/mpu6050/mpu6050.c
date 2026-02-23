#include "mpu6050.h"
#include "i2c_bus.h"
#include "esp_log.h"

#define MPU_REG_WHO_AM_I     0x75
#define MPU_REG_PWR_MGMT_1   0x6B
#define MPU_REG_ACCEL_XOUT_H 0x3B
#define MPU_REG_GYRO_XOUT_H  0x43

static const char *TAG = "MPU6050";

void mpu6050_init(i2c_master_dev_handle_t dev) {
  uint8_t whoami;
  if (read_reg(dev, MPU_REG_WHO_AM_I, &whoami, 1) == ESP_OK) {
    ESP_LOGI(TAG, "WHO_AM_I = 0x%02X", whoami);
  }

  write_reg(dev, MPU_REG_PWR_MGMT_1, 0x00); // wake up
  write_reg(dev, 0x19, 7);                  // gyro sample rate
  write_reg(dev, 0x1C, 0);                  // accel range ±2g
}

esp_err_t mpu6050_read_accel(i2c_master_dev_handle_t dev, mpu_vec3_t *accel) {
  uint8_t raw[6];
  if (read_reg(dev, MPU_REG_ACCEL_XOUT_H, raw, 6) != ESP_OK)
    return ESP_FAIL;

  int16_t x = (raw[0] << 8) | raw[1];
  int16_t y = (raw[2] << 8) | raw[3];
  int16_t z = (raw[4] << 8) | raw[5];

  accel->x = x / 16384.0f;
  accel->y = y / 16384.0f;
  accel->z = z / 16384.0f;
  return ESP_OK;
}

esp_err_t mpu6050_read_gyro(i2c_master_dev_handle_t dev, mpu_vec3_t *gyro) {
  uint8_t raw[6];
  if (read_reg(dev, MPU_REG_GYRO_XOUT_H, raw, 6) != ESP_OK)
    return ESP_FAIL;

  int16_t x = (raw[0] << 8) | raw[1];
  int16_t y = (raw[2] << 8) | raw[3];
  int16_t z = (raw[4] << 8) | raw[5];

  gyro->x = x / 131.0f;
  gyro->y = y / 131.0f;
  gyro->z = z / 131.0f;
  return ESP_OK;
}

esp_err_t mpu6050_check_and_wake(i2c_master_dev_handle_t dev) {
  uint8_t pwr;
  if (read_reg(dev, MPU_REG_PWR_MGMT_1, &pwr, 1) != ESP_OK)
    return ESP_FAIL;

  if (pwr & 0x40)
    return write_reg(dev, MPU_REG_PWR_MGMT_1, 0x00);

  return ESP_OK;
}
