#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "dht.h"

/* Thresholds */
#define TEMPERATURE_THRESHOLD   45
#define HUMIDITY_LOW_THRESHOLD  25
#define HUMIDITY_HIGH_THRESHOLD 60

/* GPIO mapping */
#define GPIO_DHT               GPIO_NUM_5

#define GPIO_TEMP_ALRM         GPIO_NUM_14
#define GPIO_HUM_ALRM          GPIO_NUM_15

#define GPIO_TEMP_RST          GPIO_NUM_26
#define GPIO_HUM_RST           GPIO_NUM_27


static bool temp_alarm = false;
static bool hum_alarm  = false;

/** @brief Configuración de puertos GPIO */
static void gpio_init(void);

/** @brief Lectura de los valores del sensor DHT22 */ 
void sensor_task(void *pvParameters);

/** @brief Sistema de control de los GPIO */
void control_task(void *pvParameters);

void app_main(void)
{    
    gpio_init();

    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
    xTaskCreate(control_task, "control_task", 2048, NULL, 5, NULL);
}




/**
 * @brief Inicialización de los GPIOs del sistema
 *
 * @details
 * Esta función configura los pines GPIO utilizados en el sistema para:
 *
 * - Salidas digitales (LEDs de alarma)
 * - Entradas digitales (botones de reset)
 *
 * Configuración de salidas:
 * Los pines asociados a las alarmas de temperatura y humedad se configuran
 * como salidas digitales. Estos pines controlan directamente los LEDs
 * indicadores de estado.
 *
 * Configuración de entradas:
 * Los pines asociados a los botones se configuran como entradas digitales
 * con resistencias de pull-up internas habilitadas (`GPIO_PULLUP_ENABLE`).
 *
 * Pull-up interno:
 * Al habilitar el pull-up interno, el pin se mantiene en nivel lógico alto (1)
 * cuando el botón no está presionado. Al presionar el botón, el pin se conecta
 * a GND, generando un nivel lógico bajo (0).
 *
 * Esto implica que:
 * - Estado sin presionar → lógica = 1
 * - Estado presionado   → lógica = 0 (active low)
 *
 * Esta configuración elimina la necesidad de resistencias externas de pull-up,
 * simplificando el hardware.
 *
 * Notas de diseño:
 * - Los LEDs deben incluir una resistencia limitadora de corriente en serie
 *   (típicamente 220 Ω a 330 Ω).
 * - Los botones están diseñados para operar en lógica activa en bajo.
 * - Se reutiliza la estructura `gpio_config_t` para configurar diferentes
 *   grupos de pines, modificando únicamente los campos necesarios.
 *
 * @note La función debe ser llamada antes de iniciar cualquier tarea que
 *       acceda a los GPIOs.
 */
static void gpio_init(void)
{
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << GPIO_TEMP_ALRM) | (1ULL << GPIO_HUM_ALRM)
    };
    gpio_config(&io_conf);

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pin_bit_mask = (1ULL << GPIO_TEMP_RST) | (1ULL << GPIO_HUM_RST);
    gpio_config(&io_conf);
}

/**
 * @brief Lectura de los valores del sensor DHT22
 * 
 * @details 
 * DHT_TYPE_AM2301 (DHT21, DHT22, AM2302, AM2321)
 * 
 * esp_err_t dht_read_float_data(dht_sensor_type_t sensor_type, gpio_num_t pin, 
 * float *HUM, float *TEMP)
 * 
 * Read float data from sensor on specified pin.
 * HUM and TEMP are returned as floats.
 * 
 * Parameters:
 *      sensor_type – DHT11 or DHT22
 *      pin – GPIO pin connected to sensor OUT
 *      HUM – [out] HUM, percents, nullable
 *      TEMP – [out] TEMP, degrees Celsius, nullable
 *
 * Returns:
 *      ESP_OK on success
 */
void sensor_task(void *pvParameters)
{
    float temp, hum;

    while (1)
    {
        if (dht_read_float_data(DHT_TYPE_AM2301, GPIO_NUM_5, &hum, &temp) == ESP_OK)
        {
            printf("[SENSOR] T=%.2f H=%.2f\n", temp, hum);

            /* Temperature logic */
            temp_alarm = (temp > TEMPERATURE_THRESHOLD);

            /* Humidity logic */
            hum_alarm = (hum < HUMIDITY_LOW_THRESHOLD ||
                         hum > HUMIDITY_HIGH_THRESHOLD);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief Sistema de control de los GPIO 
 * 
 * @details 
 *      pines de alarma (LED): 14,15 
 *      pines de reset (BUTTON): 26,27
 */
void control_task(void *pvParameters)
{
    while (1)
    {
        /* Reset buttons (active low) */
        if (gpio_get_level(GPIO_TEMP_RST) == 0)
        {
            temp_alarm = false;
        }

        if (gpio_get_level(GPIO_HUM_RST) == 0)
        {
            hum_alarm = false;
        }

        /* Apply outputs */
        gpio_set_level(GPIO_TEMP_ALRM, temp_alarm);
        gpio_set_level(GPIO_HUM_ALRM, hum_alarm);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}