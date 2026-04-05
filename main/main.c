#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_bus.h"
#include "mpu6050.h"
#include "nvs_flash.h"
#include "udp.h"

static const char *TAG = "MPU";
#define MPU_ADDR 0x68
#define I2C_FREQ_HZ 100000

static void wifi_connect(void) {
  // Initialize WiFi
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  // Configure WiFi as station
  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = "dlink-E930",
              .password = "maohf79648",
          },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  // Connect
  ESP_ERROR_CHECK(esp_wifi_connect());

  ESP_LOGI(TAG, "WiFi connecting...");
}
void app_main(void) {

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_create_default_wifi_sta();
  wifi_connect();

  xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);

  i2c_master_bus_handle_t bus = i2c_bus_init();
  i2c_master_dev_handle_t mpu = i2c_bus_add_device(bus, MPU_ADDR, I2C_FREQ_HZ);

  mpu6050_init(mpu);

  mpu_vec3_t accel;
  mpu_vec3_t gyro;

  TickType_t last_wake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(1000); // 100 Hz

  while (1) {
    int64_t start = esp_timer_get_time();

    mpu6050_read_accel(mpu, &accel);
    mpu6050_read_gyro(mpu, &gyro);

    ESP_LOGI(TAG, "A: %.2f %.2f %.2f | G: %.2f %.2f %.2f", accel.x, accel.y,
             accel.z, gyro.x, gyro.y, gyro.z);

    vTaskDelayUntil(&last_wake, period);
    mpu6050_check_and_wake(mpu);

    int64_t end = esp_timer_get_time();
    ESP_LOGI(TAG, "Loop: %lld us", end - start);
  }
}
