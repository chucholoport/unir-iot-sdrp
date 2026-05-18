idf_component_register(
    SRCS 
        "mpu6050-mqtt.c"
    
    INCLUDE_DIRS 
        "."
    
    REQUIRES
        driver
        esp_driver_i2c
        esp_wifi
        esp_event
        nvs_flash
        mqtt
)
