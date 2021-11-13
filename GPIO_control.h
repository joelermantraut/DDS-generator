#ifndef CE_GPIO_CONTROL_H
#define CE_GPIO_CONTROL_H

#include "stm32f4xx.h"
#include <stm32f4xx_gpio.h>

// Librerias

typedef struct {
	uint32_t periferico;
	GPIO_TypeDef* port;
	uint16_t pin;
} Entrada;

typedef struct {
	uint32_t periferico;
	GPIO_TypeDef* port;
	uint16_t pin;
	uint8_t pin_source;
} Salida;

// Estructuras

enum {
	NO_BUTTON, // 0
	BUTTON_1,  // 1
	BUTTON_2,  // 2
	BUTTON_3,  // 3
	BUTTON_4   // 4
};

// Enumeraciones

void conf_in(Entrada entrada, GPIOPuPd_TypeDef PuPd);
void conf_out(Salida salida);
uint8_t leer_entrada(Entrada entrada);
void escribir_salida(Salida salida, uint8_t state);
uint8_t pulsador_presionado(
		Entrada puls_line_entrada_1,
		Entrada puls_line_entrada_2,
		Entrada puls_line_entrada_3,
		Entrada puls_line_entrada_4
);

// Prototipos

#endif
