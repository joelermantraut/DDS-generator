#ifndef CE_BT_ADC_CONTROL_H
#define CE_BT_ADC_CONTROL_H

#include "stm32f4xx.h"

// Librerias

typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
	uint32_t perif_port;
	uint32_t perif_adc;
	ADC_TypeDef *adc;
	uint8_t injected_channel;
	uint8_t channel;
} ADC_PIN;

// Estructuras

void ADC_init(ADC_PIN adc);
int32_t ADC_read(ADC_PIN adc);

// Prototipos

#endif
