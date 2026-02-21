#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/i2c_types.h"
#include "soc/clk_tree_defs.h"
#include <stddef.h>
#include <stdint.h>

#define I2C_PORT I2C_NUM_0
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define MPU6050_ADDR 0x68
#define I2C_FREQ_HZ 100000

#define MPU_REG_PWR_MGMT_1 0x6B
#define MPU_REG_WHO_AM_I 0x75
#define MPU_REG_ACCEL_XOUT_H 0x3B
#define MPU_REG_GYRO_XOUT_H 0x43

static const char *TAG = "MPU6050";

typedef struct {
  float x;
  float y;
  float z;
} mpu_coord_t;

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

esp_err_t write_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t data) {
  uint8_t write_buf[] = {reg, data};
  return i2c_master_transmit(dev, write_buf, 2, pdMS_TO_TICKS(100));
}

esp_err_t read_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t *buffer,
                   size_t len) {
  return i2c_master_transmit_receive(dev, &reg, 1, buffer, len,
                                     pdMS_TO_TICKS(100));
}

void mpu_start_up(i2c_master_dev_handle_t mpu) {

  // Read who_am_i register
  uint8_t whoami;
  esp_err_t ret = read_reg(mpu, MPU_REG_WHO_AM_I, &whoami, 1);
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "WHO_AM_I = 0x%02X", whoami);
  } else {
    ESP_LOGE(TAG, "Failed to read WHO_AM_I, err=%d", ret);
  }
  ret = write_reg(mpu, MPU_REG_PWR_MGMT_1, 0x00);
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "MPU6050 Wake up succeeded!");
  } else {
    ESP_LOGE(TAG, "MPU6050 Wake up failed!");
  }

  // Set gyroscope sample rate to 1kHz
  ret = write_reg(mpu, 0x19, 7);
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "Set the gyro sample rate to 1kHz.");
  } else {
    ESP_LOGE(TAG, "Failed to set gyro sample rate.");
  }

  // Set accelerometer range
  ret = write_reg(mpu, 0x1C, 0);
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "Set the accelerometer to full scale range of ± 2g.");
  } else {
    ESP_LOGE(TAG, "Failed to set the accelerometer scale.");
  }
}

esp_err_t read_accel_data(i2c_master_dev_handle_t mpu, mpu_coord_t *accel) {

  uint8_t raw[6];
  esp_err_t ret;
  ret = read_reg(mpu, MPU_REG_ACCEL_XOUT_H, raw, 6);

  if (ret != ESP_OK) {
    return ret;
  }
  // (high << 8) | low
  int16_t RAWX = (raw[0] << 8) | raw[1];
  int16_t RAWY = (raw[2] << 8) | raw[3];
  int16_t RAWZ = (raw[4] << 8) | raw[5];

  accel->x = (float)RAWX / 16384.0f;
  accel->y = (float)RAWY / 16384.0f;
  accel->z = (float)RAWZ / 16384.0f;

  return ESP_OK;
}

esp_err_t read_gyro_data(i2c_master_dev_handle_t mpu, mpu_coord_t *gyro) {

  uint8_t raw[6];
  esp_err_t ret;

  ret = read_reg(mpu, MPU_REG_GYRO_XOUT_H, raw, 6);
  if (ret != ESP_OK) {
    return ret;
  }

  int16_t RAWX = (raw[0] << 8) | raw[1];
  int16_t RAWY = (raw[2] << 8) | raw[3];
  int16_t RAWZ = (raw[4] << 8) | raw[5];

  gyro->x = (float)RAWX / 131.0f;
  gyro->y = (float)RAWY / 131.0f;
  gyro->z = (float)RAWZ / 131.0f;

  return ESP_OK;
}

esp_err_t check_and_wake(i2c_master_dev_handle_t mpu) {

  uint8_t pwr;
  esp_err_t ret;

  ret = read_reg(mpu, MPU_REG_PWR_MGMT_1, &pwr, 1);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read PWR_MGMT");
    return ret;
  }
  if (pwr & 0x40) {
    ret = write_reg(mpu, MPU_REG_PWR_MGMT_1, 0x00);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "MPU6050 Wake up failed!");
      return ret;
    }
    ESP_LOGI(TAG, "MPU6050 Wake up succeeded!");
  }
  return ret;
}

void app_main(void) {
  i2c_master_bus_handle_t bus = i2c_init_bus();
  i2c_master_dev_handle_t mpu = i2c_add_device(bus);
  mpu_coord_t accel;
  mpu_coord_t gyro;

  vTaskDelay(pdMS_TO_TICKS(100));

  mpu_start_up(mpu);
  vTaskDelay(pdMS_TO_TICKS(100));

  while (1) {

    if (read_accel_data(mpu, &accel) != ESP_OK) {
      ESP_LOGE(TAG, "Failed to read accel data");
    } else {
      ESP_LOGI(TAG, "Accelerometer: x=%.2f, y=%.2f, z=%.2f", accel.x, accel.y,
               accel.z);
    }
    if (read_gyro_data(mpu, &gyro) != ESP_OK) {
      ESP_LOGE(TAG, "Failed to read gyro data");
    } else {
      ESP_LOGI(TAG, "Gyroscope: x=%.2f, y=%.2f, z=%.2f", gyro.x, gyro.y,
               gyro.z);
    }

    check_and_wake(mpu);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
