/**
 * @file    nombre-display.c
 * @brief   Ejercicio 2 — Muestra en un display de 7 segmentos los dígitos
 *          correspondientes a las letras del nombre "JESÚS" según su posición
 *          en el alfabeto castellano, cambiando cada segundo.
 *
 * @details Cálculo de posiciones (alfabeto castellano de 27 letras, la tilde no
 *          altera la posición):
 *            J = 10, E = 5, S = 20, U = 22, S = 20.
 *          Descomponiendo en dígitos individuales, la secuencia mostrada es:
 *            1, 0, 5, 2, 0, 2, 2, 2, 0.
 *
 *          El firmware está pensado para compilarse con ESP-IDF v6.0 desde la
 *          extensión de Wokwi para VS Code (no desde el IDE web de Wokwi).
 *
 * @author  Jesús Salvador López Ortega
 * @date    2026
 * @copyright MIT
 */

#include "seg7.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/** @brief Etiqueta para el subsistema de trazas. */
static const char *const APP_TAG = "nombre-display";

/** @brief Periodo de visualización de cada dígito, en milisegundos. */
#define APP_DIGIT_PERIOD_MS   (1000U)

/** @brief Longitud de la secuencia de dígitos a mostrar. */
#define APP_SEQ_LEN           (9U)

/**
 * @brief Secuencia de dígitos derivada del nombre "JESÚS".
 * @details J=10 -> {1,0}, E=5 -> {5}, S=20 -> {2,0}, U=22 -> {2,2}, S=20 -> {2,0}.
 */
static const uint8_t APP_DIGIT_SEQUENCE[APP_SEQ_LEN] =
{
    1U, 0U, 5U, 2U, 0U, 2U, 2U, 2U, 0U
};

/**
 * @brief Punto de entrada de la aplicación ESP-IDF.
 * @return void — no retorna; el bucle de visualización es infinito.
 */
void app_main(void)
{
    const esp_err_t init_status = seg7_init();

    if (init_status != ESP_OK)
    {
        ESP_LOGE(APP_TAG, "No se pudo inicializar el display (err=%d). Abortando.",
                 (int)init_status);
    }
    else
    {
        ESP_LOGI(APP_TAG, "Display inicializado. Secuencia para 'JESUS' lista.");

        for (;;)
        {
            uint8_t i = 0U;

            for (i = 0U; i < APP_SEQ_LEN; i++)
            {
                const uint8_t digit = APP_DIGIT_SEQUENCE[i];
                (void)seg7_show_digit(digit);
                ESP_LOGI(APP_TAG, "Mostrando digito: %u", (unsigned)digit);
                vTaskDelay(pdMS_TO_TICKS(APP_DIGIT_PERIOD_MS));
            }
        }
    }
}