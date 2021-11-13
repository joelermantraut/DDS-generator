/**
  ******************************************************************************
  * @file    main.c
  * @author  Ermantraut Joel - Coletto Gallego Danilo - TDII
  * @Trabajo Trabajo Practico 2 - Punto 1
  * @version
  * @date    2 de Noviembre de 2021
  * @brief   Generador de señales
  * @chars	 El dispositivo puede promocionar tres tipos distintas de señales (sinusoidal,
  * 		 triangular y cuadrada), la frecuencia se selecciona con un potenciometro, un
  * 		 pulsador para cambiar el rango de frecuencia que se varía con el dial, otro
  * 		 para controlar la amplitud de salida (3 estados), y un display para ver info
  * 		 respecto a estas condiciones. El generador maneja frecuencias de 100Hz a 10000Hz.
  *
  * Conexiones
  * LCD:
  * 	- RS: PB8
  * 	- E: PE5
  * 	- D4: PE4
  * 	- D5: PE6
  * 	- D6: PE2
  * 	- D7: PC13
  * Potenciometro - PC2
  * DAC - PC5
  * Control de atenuacion por HW:
  * 	 - Nivel 1: PC1
  * 	 - Nivel 2: PC3
  * 	 - Nivel 3: PC1 y PC3
  * Pulsadores: PA0, PA1, PA2, PA3
  * LEDs: PD12, PD13, PD14, PD15

  ******************************************************************************
 **/
/* Include core modules */
#include "stm32f4xx.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
/* Include my libraries here */
#include "defines.h"
#include "tm_stm32f4_dac_signal.h"
#include "tm_stm32f4_delay.h"
#include "tm_stm32f4_hd44780.h"
// TM libraries
#include "ADC_control.h"
#include "GPIO_control.h"
#include "TIM_control.h"

// Libraries

#define MAX_ADC		3650
#define MIN_ADC		0

#define MIN_FREQ	100
#define MAX_FREQ	10000

#define GREEN	0
#define ORANGE	1
#define RED		2
#define BLUE	3

#define LEDS_LEN	4

#define NIVELES		2

#define T_REFRESH_DISPLAY	250
#define FREQ_UPDATE			1000

#define COLUMNAS 			20
#define FILAS				4

#define ADC_MEDIDAS			10

#define DAC_USED			TM_DAC2

#define FACTOR_MULT			3.12

// Defines

typedef enum {
	De_100_a_1k,
	De_1k_a_10k
} Freq_Band;

typedef enum
{
	LED_GREEN	= 0,
	LED_ORANGE	= 1,
	LED_RED		= 2,
	LED_BLUE	= 3
} LED_NAME_t;

// Enums

// Structures

void ADC_init(ADC_PIN adc);
void refrescoDisplay(void);
void TIM2_IRQHandler(void);
void set_att_level(int16_t att_level);
void float_print(char *buffer, const char* text, float num, const char *simbolo);
void actualizar_menu(char filas[FILAS][COLUMNAS + 1], int16_t button);
double calcular_frecuencia(int32_t valor_ADC);
void toggle_LEDs(int16_t LED_index, Salida LEDs_array[LEDS_LEN]);

// Prototypes

ADC_PIN dial_adc = {
	GPIOC,
	GPIO_Pin_2,
	RCC_AHB1Periph_GPIOC,
	RCC_APB2Periph_ADC2,
	ADC2,
	ADC_InjectedChannel_1,
	ADC_Channel_12
}; // Pin PC2

Salida conmutador_banda = {
	RCC_AHB1Periph_GPIOC,
	GPIOC,
	GPIO_Pin_9,
	GPIO_PinSource9
}; // Pin PC9

Salida pin_niveles[NIVELES] = {
	{
		RCC_AHB1Periph_GPIOC,
		GPIOC,
		GPIO_Pin_1,
		GPIO_PinSource1
	}, // Pin PC1
	{
		RCC_AHB1Periph_GPIOC,
		GPIOC,
		GPIO_Pin_3,
		GPIO_PinSource3
	}, // Pin PC3
};

Salida LEDs_In[LEDS_LEN] = {
	{
		RCC_AHB1Periph_GPIOD,
		GPIOD,
		GPIO_Pin_12,
		GPIO_PinSource12
	},
	{
		RCC_AHB1Periph_GPIOD,
		GPIOD,
		GPIO_Pin_13,
		GPIO_PinSource13
	},
	{
		RCC_AHB1Periph_GPIOD,
		GPIOD,
		GPIO_Pin_14,
		GPIO_PinSource14
	},
	{
		RCC_AHB1Periph_GPIOD,
		GPIOD,
		GPIO_Pin_15,
		GPIO_PinSource15
	}
};
// Arreglos de LEDs de la placa

Entrada puls_line_entrada_1 = {RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_0}; // PA0
Entrada puls_line_entrada_2 = {RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_1}; // PA1
Entrada puls_line_entrada_3 = {RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_2}; // PA2
Entrada puls_line_entrada_4 = {RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_3}; // PA3
// Pulsadores de entrada

double frecuencia = 0; // De 100 a 10000 Hz
TM_DAC_SIGNAL_Signal_t forma = TM_DAC_SIGNAL_Signal_Sinus; // Recuerda la forma de onda
int16_t banda = De_100_a_1k;
int16_t trig_ADC = 1;

// Variables

int main(void) {
	int i;
    int32_t valor_ADC;
    int32_t valor_ADC_prom; // Para tomar varias medidas

    double last_frecuencia = frecuencia;
    double actual_frecuencia;
    TM_DAC_SIGNAL_Signal_t last_forma = forma;
    int16_t last_banda = banda;

    SystemInit();

    TM_DELAY_Init();

    TM_HD44780_Init(COLUMNAS, FILAS);
    TM_HD44780_Puts(0, 0, "Bienvenidos");
    TM_HD44780_Puts(0, 2, "Inicializando...");

	conf_in(puls_line_entrada_1, GPIO_PuPd_DOWN);
	conf_in(puls_line_entrada_2, GPIO_PuPd_DOWN);
	conf_in(puls_line_entrada_3, GPIO_PuPd_DOWN);
	conf_in(puls_line_entrada_4, GPIO_PuPd_DOWN);

    for(i = 0; i < NIVELES; i++) {
    	conf_out(pin_niveles[i]);
    }
    set_att_level(0);

    for(i = 0; i < LEDS_LEN; i++) {
    	conf_out(LEDs_In[i]);
    }

    TIM2_Config();

    ADC_init(dial_adc);

    TIM2_Init(1000); // Interrumpe cada 1ms

    while (1) {
    	if (trig_ADC) {
    		trig_ADC = 0;
    		valor_ADC_prom = 0;

    		for(i = 0; i < ADC_MEDIDAS; i++) {
    			valor_ADC = ADC_read(dial_adc);

    			valor_ADC_prom += valor_ADC;
    		}
    		// Tomo varias muestras para evitar el efecto del ruido

    		valor_ADC = (int32_t) valor_ADC_prom / ADC_MEDIDAS;

    		actual_frecuencia = calcular_frecuencia(valor_ADC);

    		if(last_banda != banda || last_forma != forma || abs(last_frecuencia - actual_frecuencia) > 5) {
    			frecuencia = actual_frecuencia;
    			last_frecuencia = frecuencia;
        		last_banda = banda;
        		last_forma = forma;

    			// Evita cambios innecesarios y cambios de frecuencia con una tolerancia de 5Hz
    		    TM_DAC_SIGNAL_Init(DAC_USED, TIM5);
    		    TM_DAC_SIGNAL_SetSignal(DAC_USED, forma, frecuencia * FACTOR_MULT);
    		    // Se añade un factor de multiplicacion porque por alguna
    		    // razon hay una diferencia entre el parametro que se le pasa
    		    // y la salida que se obtiene
    		}

    	}
    }
}

void float_print(char *buffer, const char* text, float num, const char *simbolo) {
    int num_entero = (int) num;                         // Asigno la parte entera del total
    float frac_num = num - num_entero;                  // Resto el total menos la parte entera y obtengo los decimales
    int entero_frac_num = trunc(frac_num * 10);        // Obtengo los decimales como un entero para mostrarlos en el display
    // Lo multiplico por 10 para tener un decimal

    if (entero_frac_num >= 10) {
        siprintf(buffer, "%s %d.%d %s", text, num_entero, entero_frac_num, simbolo);
    } else {
        siprintf(buffer, "%s %d.%d %s", text, num_entero, entero_frac_num, simbolo);
    }
}

void actualizar_menu(char filas[FILAS][COLUMNAS + 1], int16_t button) {
	/*
	 * Lee el pulsador y recordando el estado actual del menu,
	 * lo modifica y devuelve el contenido de cada fila del display.
	 */

	static int16_t wave_type = 0,
				   freq_band = 0,
				   att_level = 0,
				   pref_unit = 0;

	if (button == BUTTON_1) {
		// Cambia el tipo de onda
		wave_type++;
		if (wave_type > 2) wave_type = 0;
	} else if (button == BUTTON_2) {
		// Cambia la banda de frecuencias
		freq_band++;
		if (freq_band > 1) freq_band = 0;
	} else if (button == BUTTON_3) {
		// Cambia el nivel de atenuacion
		att_level++;
		if (att_level > 2) att_level = 0;
	} else if (button == BUTTON_4) {
		// Cambia el prefijo de la unidad de frecuencia (Hz, kHz, etc.)
		pref_unit++;
		if (pref_unit > 1) pref_unit = 0;
	}

	// Despues de que las variables fueron modificadas, obtengo
	// el contenido del display y lo devuelvo.
	if (wave_type == 0) {
		strcpy(filas[0], "Forma: Seno");
		forma = TM_DAC_SIGNAL_Signal_Sinus;
	}
	else if (wave_type == 1) {
		strcpy(filas[0], "Forma: Triangular");
		forma = TM_DAC_SIGNAL_Signal_Triangle;
	}
	else {
		strcpy(filas[0], "Forma: Rectangular");
		forma = TM_DAC_SIGNAL_Signal_Square;
	}

	if (freq_band == 0) {
		strcpy(filas[1], "Banda: 100 a 1k");
		banda = De_100_a_1k;
		escribir_salida(conmutador_banda, De_100_a_1k);
	}
	else {
		strcpy(filas[1], "Banda: 1k a 10k");
		banda = De_1k_a_10k;
		escribir_salida(conmutador_banda, De_1k_a_10k);
	}

	if (att_level == 0) strcpy(filas[2], "Att: Nivel 1");
	else if (att_level == 1) strcpy(filas[2], "Att: Nivel 2");
	else strcpy(filas[2], "Att: Nivel 3");

	set_att_level(att_level);

	if (pref_unit == 0) float_print(filas[3], "f: ", frecuencia, "Hz");
	else float_print(filas[3], "f: ", frecuencia / 1000.0, "kHz");
}

void refrescoDisplay(void) {
	int i;
	char filas[FILAS][COLUMNAS + 1];
	// +1 por el \0

	int16_t button = pulsador_presionado(
			puls_line_entrada_1,
			puls_line_entrada_2,
			puls_line_entrada_3,
			puls_line_entrada_4
	);
	actualizar_menu(filas, button);

	TM_HD44780_Clear();
	for(i = 0; i < FILAS; i++) {
		TM_HD44780_Puts(0, i, filas[i]);
	}
}

/*
 * FUNCIONES DE ACTUALIZACION
 */

double calcular_frecuencia(int32_t valor_ADC) {
	/*
	 * Calcula la frecuencia tomando los valores maximos
	 * y minimos del ADC, y de las frecuencias maximas
	 * y minimas permitidas por el generador.
	 *
	 * Los minimos y maximos de frecuencia dependen de la banda
	 * elegida.
	 */
	double max_freq = banda ? 10000.0 : 1000.0;
	double min_freq = banda ? 1000.0 : 100.0;
	// Calcula las frecuencias extremo segun la banda elegida

	double aux_freq = (valor_ADC * (max_freq - min_freq)) / (MAX_ADC - MIN_ADC);
	if (aux_freq > max_freq) aux_freq = max_freq;
	else if (aux_freq < min_freq) aux_freq = min_freq;

	return aux_freq;
}

void set_att_level(int16_t att_level) {
	/*
	 * Setea las salidas correspondiente para conseguir diferentes
	 * atenuaciones en la señal de salida.
	 */

	if (att_level == 0) {
		escribir_salida(pin_niveles[0], 1);
		escribir_salida(pin_niveles[1], 0);
	} else if (att_level == 1) {
		escribir_salida(pin_niveles[0], 0);
		escribir_salida(pin_niveles[1], 1);
	} else {
		escribir_salida(pin_niveles[0], 1);
		escribir_salida(pin_niveles[1], 1);
	}
}

void toggle_LEDs(int16_t LED_index, Salida LEDs_array[LEDS_LEN]) {
	/*
	 * Conmuta un led de la lista
	 */
	static int16_t LEDs_state[LEDS_LEN] = {0};

	if (LEDs_state[LED_index]) {
		escribir_salida(LEDs_array[LED_index], 1);
	} else {
		escribir_salida(LEDs_array[LED_index], 0);
	}

	LEDs_state[LED_index] = !LEDs_state[LED_index];
}

/*
 * FUNCIONES DE TEMPORIZACION
 */
void TIM2_IRQHandler(void)
{
	/*
	 * Timer para reemplazar el systick, porque la generacion de señal con el DAC
	 * lo requiere.
	 *
	 * Se dispara cada 1ms.
	 */

	static int contSystick = 0;

	if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC1); // Se limpia la bandera de interrupcion.

		++contSystick;

		if (!(contSystick % T_REFRESH_DISPLAY)) {
			refrescoDisplay();
		}
		if (!(contSystick % FREQ_UPDATE)) {
			toggle_LEDs(LED_GREEN, LEDs_In);

			trig_ADC = 1;

			contSystick = 0;
		}
	}
}
