/**
 * @file    seg7.h
 * @brief   Controlador de bajo nivel para un display de 7 segmentos (cátodo común)
 *          gobernado por GPIO en un SoC ESP32 bajo ESP-IDF v6.0.
 *
 * @details Este módulo expone la tabla de codificación binaria de los dígitos 0..9
 *          y las primitivas necesarias para inicializar los GPIO y presentar un
 *          dígito en el display. Está diseñado para cumplir con un subconjunto de
 *          las guías MISRA-C:2012 aplicables a firmware embebido:
 *            - Tipos de anchura fija (Dir 4.6): se usa @c uint8_t / @c uint32_t.
 *            - Un único punto de retorno por función (Adv. 15.5).
 *            - Ausencia de números mágicos: constantes con nombre (Adv. 2.5 evitado).
 *            - Sin asignaciones dinámicas de memoria (Regla 21.3).
 *            - Parámetros validados antes de su uso (defensa en profundidad).
 *
 * @note    Configuración de hardware: display de CÁTODO COMÚN. El cátodo común se
 *          conecta a GND; cada segmento se enciende poniendo su GPIO en nivel ALTO.
 *          Para un display de ÁNODO COMÚN habría que invertir la lógica (nivel BAJO
 *          enciende) y complementar la tabla @c SEG7_DIGIT_TABLE.
 *
 * @author  Jesús Salvador López Ortega
 * @date    2026
 * @copyright MIT
 */

#ifndef SEG7_H
#define SEG7_H

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Número de segmentos controlables (A..G). El punto decimal (DP) no se usa. */
#define SEG7_SEGMENT_COUNT   (7U)

/** @brief Valor de dígito máximo representable por la tabla. */
#define SEG7_MAX_DIGIT       (9U)

/**
 * @brief Mapa de GPIO de los siete segmentos, en orden A,B,C,D,E,F,G.
 *
 * @details El índice del arreglo corresponde al segmento: [0]=A ... [6]=G.
 *          Estos valores deben coincidir con el conexionado descrito en
 *          @c wokwi/diagram.json. MISRA Regla 8.4: definición con enlace externo
 *          declarada en cabecera.
 */
extern const uint8_t SEG7_GPIO_MAP[SEG7_SEGMENT_COUNT];

/**
 * @brief Tabla de codificación de dígitos para display de CÁTODO COMÚN.
 *
 * @details Cada entrada es una máscara de 7 bits (bit0=A ... bit6=G). Un bit a 1
 *          indica que el segmento correspondiente debe encenderse. La tabla está
 *          indexada por el valor del dígito (0..9).
 *
 * | Dígito | gfedcba | Segmentos encendidos      |
 * |:------:|:-------:|:--------------------------|
 * |   0    | 0111111 | A B C D E F               |
 * |   1    | 0000110 | B C                       |
 * |   2    | 1011011 | A B D E G                 |
 * |   3    | 1001111 | A B C D G                 |
 * |   4    | 1100110 | B C F G                   |
 * |   5    | 1101101 | A C D F G                 |
 * |   6    | 1111101 | A C D E F G               |
 * |   7    | 0000111 | A B C                     |
 * |   8    | 1111111 | A B C D E F G             |
 * |   9    | 1101111 | A B C D F G               |
 */
extern const uint8_t SEG7_DIGIT_TABLE[SEG7_MAX_DIGIT + 1U];

/**
 * @brief Inicializa como salida los GPIO asociados a los siete segmentos.
 *
 * @return @c ESP_OK si todos los GPIO se configuraron correctamente;
 *         código de error de ESP-IDF en caso contrario.
 */
esp_err_t seg7_init(void);

/**
 * @brief Presenta un dígito decimal en el display de 7 segmentos.
 *
 * @param[in] digit Valor a mostrar, en el rango cerrado [0, 9].
 *
 * @return @c ESP_OK si el dígito es válido y se aplicó al hardware;
 *         @c ESP_ERR_INVALID_ARG si @p digit está fuera de rango.
 */
esp_err_t seg7_show_digit(uint8_t digit);

/**
 * @brief Apaga todos los segmentos del display.
 *
 * @return @c ESP_OK en caso de éxito; código de error de ESP-IDF en caso contrario.
 */
esp_err_t seg7_clear(void);

#ifdef __cplusplus
}
#endif

#endif /* SEG7_H */
