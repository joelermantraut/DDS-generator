#include <ADC_control.h>

void ADC_init(ADC_PIN adc) {
    GPIO_InitTypeDef        GPIO_InitStructure;
    ADC_InitTypeDef         ADC_InitStructure;
    ADC_CommonInitTypeDef   ADC_CommonInitStructure;

    /* Puerto C -------------------------------------------------------------*/
    RCC_AHB1PeriphClockCmd(adc.perif_port, ENABLE);

    /* PC2 para entrada analógica */
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin     = adc.pin;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL ;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Activar ADC2 ----------------------------------------------------------*/
    RCC_APB2PeriphClockCmd(adc.perif_adc, ENABLE);

    /* ADC Common Init -------------------------------------------------------*/
    ADC_CommonStructInit(&ADC_CommonInitStructure);
    ADC_CommonInitStructure.ADC_Mode                = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler           = ADC_Prescaler_Div4; // max 36 MHz segun datasheet
    ADC_CommonInitStructure.ADC_DMAAccessMode       = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay    = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    /* ADC Init ---------------------------------------------------------------*/
    ADC_StructInit (&ADC_InitStructure);
    ADC_InitStructure.ADC_Resolution             = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode           = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode     = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge   = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign              = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion        = 1;
    ADC_Init(adc.adc, &ADC_InitStructure);

    /* Establecer la configuración de conversión ------------------------------*/
    ADC_InjectedSequencerLengthConfig(adc.adc, 1);
    ADC_SetInjectedOffset(adc.adc, adc.injected_channel, 0);
    ADC_InjectedChannelConfig(adc.adc, adc.channel, 1, ADC_SampleTime_480Cycles);

    /* Poner en marcha ADC ----------------------------------------------------*/
    ADC_Cmd(adc.adc, ENABLE);
}

int32_t ADC_read(ADC_PIN adc) {

    uint32_t valor_adc;

    ADC_ClearFlag(adc.adc, ADC_FLAG_JEOC);      // borrar flag de fin conversion

    ADC_SoftwareStartInjectedConv(adc.adc);    // iniciar conversion

    while (ADC_GetFlagStatus(adc.adc, ADC_FLAG_JEOC) == RESET); // Espera fin de conversion

    valor_adc = ADC_GetInjectedConversionValue(adc.adc, adc.injected_channel); // obtiene Valor A-D

    return valor_adc;
}
