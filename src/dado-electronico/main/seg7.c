/**
 * @file    seg7.c
 * @brief   Implementación del controlador de display de 7 segmentos (cátodo común).
 * @see     seg7.h
 *
 * @author  Jesús Salvador López Ortega
 * @date    2026
 * @copyright MIT
 */

#include "seg7.h"
#include "driver/gpio.h"
#include "esp_log.h"

/** @brief Etiqueta para el subsistema de trazas (ESP_LOGx). */
static const char *const SEG7_TAG = "seg7";

/*
 * Asignación de GPIO por segmento (orden A,B,C,D,E,F,G).
 * Estos pines deben ser coherentes con wokwi/diagram.json.
 * Se eligen pines libres del ESP32 evitando los strapping pins críticos.
 */
const uint8_t SEG7_GPIO_MAP[SEG7_SEGMENT_COUNT] =
{
    15U, /* A */
    2U,  /* B */
    4U,  /* C */
    5U,  /* D */
    18U, /* E */
    19U, /* F */
    21U  /* G */
};

const uint8_t SEG7_DIGIT_TABLE[SEG7_MAX_DIGIT + 1U] =
{
    0x3FU, /* 0 -> A B C D E F     */
    0x06U, /* 1 -> B C             */
    0x5BU, /* 2 -> A B D E G       */
    0x4FU, /* 3 -> A B C D G       */
    0x66U, /* 4 -> B C F G         */
    0x6DU, /* 5 -> A C D F G       */
    0x7DU, /* 6 -> A C D E F G     */
    0x07U, /* 7 -> A B C           */
    0x7FU, /* 8 -> A B C D E F G   */
    0x6FU  /* 9 -> A B C D F G     */
};

esp_err_t seg7_init(void)
{
    uint64_t pin_mask = 0ULL;
    uint8_t  idx      = 0U;
    esp_err_t result  = ESP_OK;

    /* Construcción de la máscara de pines a partir del mapa de segmentos. */
    for (idx = 0U; idx < SEG7_SEGMENT_COUNT; idx++)
    {
        pin_mask |= (1ULL << SEG7_GPIO_MAP[idx]);
    }

    const gpio_config_t io_conf =
    {
        .pin_bit_mask = pin_mask,
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE
    };

    result = gpio_config(&io_conf);
    if (result == ESP_OK)
    {
        result = seg7_clear();
    }

    if (result != ESP_OK)
    {
        ESP_LOGE(SEG7_TAG, "Fallo al inicializar los GPIO del display (err=%d)",
                 (int)result);
    }

    return result;
}

esp_err_t seg7_show_digit(uint8_t digit)
{
    esp_err_t result = ESP_OK;

    if (digit > SEG7_MAX_DIGIT)
    {
        result = ESP_ERR_INVALID_ARG;
    }
    else
    {
        const uint8_t pattern = SEG7_DIGIT_TABLE[digit];
        uint8_t       idx     = 0U;

        for (idx = 0U; idx < SEG7_SEGMENT_COUNT; idx++)
        {
            /* Cátodo común: nivel ALTO enciende el segmento. */
            const uint32_t level = ((pattern & (uint8_t)(1U << idx)) != 0U) ? 1UL : 0UL;
            (void)gpio_set_level((gpio_num_t)SEG7_GPIO_MAP[idx], level);
        }
    }

    return result;
}

esp_err_t seg7_clear(void)
{
    esp_err_t result = ESP_OK;
    uint8_t   idx    = 0U;

    for (idx = 0U; idx < SEG7_SEGMENT_COUNT; idx++)
    {
        (void)gpio_set_level((gpio_num_t)SEG7_GPIO_MAP[idx], 0UL);
    }

    return result;
}
