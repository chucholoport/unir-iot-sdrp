/**
 * @file    dado-electronico.c
 * @brief   Ejercicio 3 — Dado electrónico. Al pulsar un botón se genera un número
 *          aleatorio en el rango [1, 6] y se muestra en el display de 7 segmentos.
 *
 * @details Diseño:
 *            - El botón se conecta entre el GPIO de entrada y GND, con resistencia
 *              de pull-up interna habilitada. En reposo el pin lee nivel ALTO; al
 *              pulsar, lee nivel BAJO (flanco descendente).
 *            - Se aplica anti-rebote (debounce) por software mediante un umbral
 *              temporal, evitando lecturas espurias sin usar temporizadores extra.
 *            - La aleatoriedad procede de @c esp_random(), respaldada por el
 *              generador de hardware (RNG) del ESP32.
 *
 *          Firmware para ESP-IDF v6.0, compilado desde la extensión Wokwi de
 *          VS Code.
 *
 * @author  Jesús Salvador López Ortega
 * @date    2026
 * @copyright MIT
 */

#include "seg7.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_random.h"
#include "esp_log.h"

/** @brief Etiqueta para el subsistema de trazas. */
static const char *const APP_TAG = "dado-electronico";

/** @brief GPIO de entrada al que se conecta el botón (a GND). */
#define APP_BUTTON_GPIO        (GPIO_NUM_23)

/** @brief Periodo de sondeo del botón, en milisegundos. */
#define APP_POLL_PERIOD_MS     (20U)

/** @brief Ventana de anti-rebote expresada en ciclos de sondeo (~60 ms). */
#define APP_DEBOUNCE_CYCLES    (3U)

/** @brief Valor mínimo del dado. */
#define APP_DICE_MIN           (1U)

/** @brief Valor máximo del dado. */
#define APP_DICE_MAX           (6U)

/**
 * @brief Devuelve un valor aleatorio uniforme en el rango [1, 6].
 * @return Número entero sin signo entre @c APP_DICE_MIN y @c APP_DICE_MAX.
 */
static uint8_t app_roll_dice(void)
{
    const uint32_t span  = (uint32_t)(APP_DICE_MAX - APP_DICE_MIN + 1U);
    const uint32_t raw   = esp_random();
    const uint8_t  value = (uint8_t)(APP_DICE_MIN + (raw % span));

    return value;
}

/**
 * @brief Configura el GPIO del botón como entrada con pull-up interno.
 * @return @c ESP_OK en caso de éxito; código de error de ESP-IDF en caso contrario.
 */
static esp_err_t app_button_init(void)
{
    const gpio_config_t btn_conf =
    {
        .pin_bit_mask = (1ULL << (uint64_t)APP_BUTTON_GPIO),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE
    };

    return gpio_config(&btn_conf);
}

/**
 * @brief Punto de entrada de la aplicación ESP-IDF.
 * @return void — no retorna; el sondeo del botón es un bucle infinito.
 */
void app_main(void)
{
    esp_err_t status = seg7_init();

    if (status == ESP_OK)
    {
        status = app_button_init();
    }

    if (status != ESP_OK)
    {
        ESP_LOGE(APP_TAG, "Inicializacion fallida (err=%d). Abortando.",
                 (int)status);
    }
    else
    {
        uint8_t stable_low = 0U;
        uint8_t handled    = 0U;

        (void)seg7_show_digit(0U);
        ESP_LOGI(APP_TAG, "Dado listo. Pulse el boton para lanzar.");

        for (;;)
        {
            const int raw_level = gpio_get_level(APP_BUTTON_GPIO);

            if (raw_level == 0) /* botón presionado (activo a nivel bajo) */
            {
                if (stable_low < APP_DEBOUNCE_CYCLES)
                {
                    stable_low++;
                }

                if ((stable_low >= APP_DEBOUNCE_CYCLES) && (handled == 0U))
                {
                    const uint8_t roll = app_roll_dice();
                    (void)seg7_show_digit(roll);
                    ESP_LOGI(APP_TAG, "Dado lanzado: %u", (unsigned)roll);
                    handled = 1U;
                }
            }
            else /* botón liberado: rearma la detección */
            {
                stable_low = 0U;
                handled    = 0U;
            }

            vTaskDelay(pdMS_TO_TICKS(APP_POLL_PERIOD_MS));
        }
    }
}