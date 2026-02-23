idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES i2c_bus esp_timer driver log freertos mpu6050
)
