/**
 * @file mpu6050-mqtt.c
 * @brief ESP32 MQTT motor monitoring simulation using MPU6050
 *
 * @details
 * This application demonstrates:
 * - ESP32
 * - MQTT communication
 * - Adafruit IO
 * - MPU6050 initialization
 * - Simulated industrial motor telemetry
 *
 * Four MQTT topics are used:
 * - Temperature
 * - Acceleration X
 * - Acceleration Y
 * - Acceleration Z
 *
 * The MPU6050 is initialized correctly through I2C.
 * However, the published values are intentionally simulated
 * to emulate a motor monitoring scenario.
 *
 * Compatible with:
 * - ESP-IDF v6
 * - Wokwi
 * - Adafruit IO
 *
 * @author Your Name
 * @date 2026
 */

/* =========================================================================
 * INCLUDES
 * ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "esp_wifi.h"
#include "mqtt_client.h"

#include "i2cdev.h"
#include "mpu6050.h"

/* =========================================================================
 * DEFINITIONS
 * ========================================================================= */

/**
 * @brief WiFi credentials for Wokwi
 */
#define WIFI_SSID                         "Wokwi-GUEST"
#define WIFI_PASSWORD                     ""

/**
 * @brief MQTT broker
 */
#define MQTT_BROKER_URI                   "mqtt://io.adafruit.com"

/**
 * @brief Adafruit IO credentials
 */
#define MQTT_USERNAME                     "chucholoport"
#define MQTT_PASSWORD                     ""

/**
 * @brief MQTT topics
 */
#define MQTT_TOPIC_TEMP                   MQTT_USERNAME "/feeds/motor-temp"
#define MQTT_TOPIC_ACCEL_X                MQTT_USERNAME "/feeds/motor-vib-x"
#define MQTT_TOPIC_ACCEL_Y                MQTT_USERNAME "/feeds/motor-vib-y"
#define MQTT_TOPIC_ACCEL_Z                MQTT_USERNAME "/feeds/motor-vib-z"

/**
 * @brief I2C configuration
 */
#define I2C_PORT                          I2C_NUM_0
#define I2C_SDA_PIN                       GPIO_NUM_21
#define I2C_SCL_PIN                       GPIO_NUM_22

/**
 * @brief Publish period
 */
#define MQTT_PUBLISH_PERIOD_MS            (2000U)

/* =========================================================================
 * TYPE DEFINITIONS
 * ========================================================================= */

/**
 * @brief Simulated motor telemetry
 */
typedef struct
{
    float temperature;
    float accel_x;
    float accel_y;
    float accel_z;

} motor_data_t;

/* =========================================================================
 * GLOBAL VARIABLES
 * ========================================================================= */

/**
 * @brief Logging tag
 */
static const char * TAG = "MQTT_MOTOR";

/**
 * @brief MQTT client handle
 */
static esp_mqtt_client_handle_t g_mqtt_client = NULL;

/**
 * @brief MPU6050 device descriptor
 */
static mpu6050_dev_t g_mpu6050_dev;

/* =========================================================================
 * FUNCTION PROTOTYPES
 * ========================================================================= */

static void wifi_init(void);

static void mqtt_init(void);

static void mpu6050_sensor_init(void);

static void mqtt_publish_data(
    const motor_data_t * const p_motor_data);

static void generate_motor_simulation(
    motor_data_t * const p_motor_data);

/* =========================================================================
 * WIFI EVENT HANDLER
 * ========================================================================= */

/**
 * @brief WiFi event callback
 *
 * @param arg User argument
 * @param event_base Event base
 * @param event_id Event identifier
 * @param event_data Event data
 */
static void wifi_event_handler(
    void * arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void * event_data)
{
    (void)arg;
    (void)event_data;

    if ((event_base == WIFI_EVENT) &&
        (event_id == WIFI_EVENT_STA_START))
    {
        esp_wifi_connect();
    }
    else if ((event_base == IP_EVENT) &&
             (event_id == IP_EVENT_STA_GOT_IP))
    {
        ESP_LOGI(TAG, "WiFi connected");
    }
}

/* =========================================================================
 * MQTT EVENT HANDLER
 * ========================================================================= */

/**
 * @brief MQTT event callback
 *
 * @param handler_args User arguments
 * @param base Event base
 * @param event_id Event identifier
 * @param event_data Event data
 */
static void mqtt_event_handler(
    void * handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void * event_data)
{
    (void)handler_args;
    (void)base;
    (void)event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED:
        {
            ESP_LOGI(TAG, "MQTT connected");
            break;
        }

        case MQTT_EVENT_DISCONNECTED:
        {
            ESP_LOGI(TAG, "MQTT disconnected");
            break;
        }

        default:
        {
            break;
        }
    }
}

/* =========================================================================
 * WIFI INITIALIZATION
 * ========================================================================= */

/**
 * @brief Initialize WiFi station mode
 */
static void wifi_init(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(
        esp_netif_init());

    ESP_ERROR_CHECK(
        esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            NULL));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL,
            NULL));

    wifi_config_t wifi_config =
    {
        .sta =
        {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config));

    ESP_ERROR_CHECK(
        esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to WiFi...");
}

/* =========================================================================
 * MQTT INITIALIZATION
 * ========================================================================= */

/**
 * @brief Initialize MQTT client
 */
static void mqtt_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg =
    {
        .broker.address.uri = MQTT_BROKER_URI,

        .credentials.username = MQTT_USERNAME,

        .credentials.authentication.password =
            MQTT_PASSWORD,
    };

    g_mqtt_client =
        esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(
        g_mqtt_client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL);

    esp_mqtt_client_start(g_mqtt_client);

    ESP_LOGI(TAG, "MQTT client started");
}

/* =========================================================================
 * MPU6050 INITIALIZATION
 * ========================================================================= */

/**
 * @brief Initialize MPU6050 sensor
 */
static void mpu6050_sensor_init(void)
{
    memset(
        &g_mpu6050_dev,
        0,
        sizeof(mpu6050_dev_t));

    ESP_ERROR_CHECK(
        i2cdev_init());

    ESP_ERROR_CHECK(
        mpu6050_init_desc(
            &g_mpu6050_dev,
            MPU6050_I2C_ADDRESS_LOW,
            I2C_PORT,
            I2C_SDA_PIN,
            I2C_SCL_PIN));

    ESP_ERROR_CHECK(
        mpu6050_init(
            &g_mpu6050_dev));

    ESP_LOGI(TAG, "MPU6050 initialized");
}

/* =========================================================================
 * MOTOR SIMULATION
 * ========================================================================= */

/**
 * @brief Generate simulated motor telemetry
 *
 * @details
 * Simulates:
 * - Temperature around 50 °C
 * - Vibrations around 0.01 g
 *
 * @param p_motor_data Pointer to telemetry structure
 */
static void generate_motor_simulation(
    motor_data_t * const p_motor_data)
{
    p_motor_data->temperature =
        50.0F +
        (((float)(rand() % 5U)) - 2.0F);

    p_motor_data->accel_x =
        0.01F +
        ((((float)(rand() % 5U)) - 2.0F) * 0.001F);

    p_motor_data->accel_y =
        0.01F +
        ((((float)(rand() % 5U)) - 2.0F) * 0.001F);

    p_motor_data->accel_z =
        0.01F +
        ((((float)(rand() % 5U)) - 2.0F) * 0.001F);
}

/* =========================================================================
 * MQTT PUBLISH
 * ========================================================================= */

/**
 * @brief Publish telemetry to Adafruit IO
 *
 * @param p_motor_data Pointer to telemetry structure
 */
static void mqtt_publish_data(
    const motor_data_t * const p_motor_data)
{
    char tx_buffer[32];

    snprintf(
        tx_buffer,
        sizeof(tx_buffer),
        "%.2f",
        p_motor_data->temperature);

    esp_mqtt_client_publish(
        g_mqtt_client,
        MQTT_TOPIC_TEMP,
        tx_buffer,
        0,
        1,
        0);

    snprintf(
        tx_buffer,
        sizeof(tx_buffer),
        "%.4f",
        p_motor_data->accel_x);

    esp_mqtt_client_publish(
        g_mqtt_client,
        MQTT_TOPIC_ACCEL_X,
        tx_buffer,
        0,
        1,
        0);

    snprintf(
        tx_buffer,
        sizeof(tx_buffer),
        "%.4f",
        p_motor_data->accel_y);

    esp_mqtt_client_publish(
        g_mqtt_client,
        MQTT_TOPIC_ACCEL_Y,
        tx_buffer,
        0,
        1,
        0);

    snprintf(
        tx_buffer,
        sizeof(tx_buffer),
        "%.4f",
        p_motor_data->accel_z);

    esp_mqtt_client_publish(
        g_mqtt_client,
        MQTT_TOPIC_ACCEL_Z,
        tx_buffer,
        0,
        1,
        0);

    ESP_LOGI(
        TAG,
        "Published -> T: %.2f X: %.4f Y: %.4f Z: %.4f",
        p_motor_data->temperature,
        p_motor_data->accel_x,
        p_motor_data->accel_y,
        p_motor_data->accel_z);
}

/* =========================================================================
 * MAIN APPLICATION
 * ========================================================================= */

/**
 * @brief Main application entry point
 */
void app_main(void)
{
    motor_data_t motor_data;

    ESP_ERROR_CHECK(
        nvs_flash_init());

    wifi_init();

    mqtt_init();

    mpu6050_sensor_init();

    while (1)
    {
        generate_motor_simulation(
            &motor_data);

        mqtt_publish_data(
            &motor_data);

        vTaskDelay(
            pdMS_TO_TICKS(
                MQTT_PUBLISH_PERIOD_MS));
    }
}