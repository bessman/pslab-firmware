/**
 * @file adc.c
 * @author Alexander Bessman (alexander.bessman@gmail.com)
 * @brief Analog to Digital Converter (ADC) driver
 */

#include <stdbool.h>
#include <stdint.h>

#include "adc.h"

/*********/
/* Types */
/*********/

struct ADCxCON1Bits {
    uint16_t DONE :1;
    uint16_t SAMP :1;
    uint16_t ASAM :1;
    uint16_t SIMSAM :1;
    uint16_t SSRCG :1;
    uint16_t SSRC :3;
    uint16_t FORM :2;
    uint16_t AD12B :1;
    uint16_t :1;
    uint16_t ADDMABM :1;
    uint16_t ADSIL :1;
    uint16_t :1;
    uint16_t ADON :1;
};

struct ADCxCON2Bits {
    uint16_t ALTS :1;
    uint16_t BUFM :1;
    uint16_t SMPI :5;
    uint16_t BUFS :1;
    uint16_t CHPS :2;
    uint16_t CSCNA :1;
    uint16_t :2;
    uint16_t VCFG :3;
};

struct ADCxCON3Bits {
    uint16_t ADCS :8;
    uint16_t SAMC :5;
    uint16_t :2;
    uint16_t ADRC :1;
};

struct ADCxCON4Bits {
    uint16_t DMABL :3;
    uint16_t :5;
    uint16_t ADDMAEN :1;
};

struct ADCxCHS123Bits {
    uint16_t CH123SA0 :1;
    uint16_t CH123NA :2;
    uint16_t CH123SA :2;
    uint16_t :1;
    uint16_t CH123SB0 :1;
    uint16_t CH123NB :2;
    uint16_t CH123SB :2;
};

struct ADCxCHS0Bits {
    uint16_t CH0SA :6;
    uint16_t :1;
    uint16_t CH0NA :1;
    uint16_t CH0SB :6;
    uint16_t :1;
    uint16_t CH0NB :1;
};

/***********/
/* Externs */
/***********/

extern struct ADCxCON1Bits volatile AD1CON1bits;
extern struct ADCxCON2Bits volatile AD1CON2bits;
extern struct ADCxCON3Bits volatile AD1CON3bits;
extern struct ADCxCON4Bits volatile AD1CON4bits;
extern struct ADCxCHS0Bits volatile AD1CH0Sbits;

extern uint16_t const volatile ADC1BUF0, ADC1BUF1, ADC1BUF2, ADC1BUF3,
                               ADC1BUF4, ADC1BUF5, ADC1BUF6, ADC1BUF7,
                               ADC1BUF8, ADC1BUF9, ADC1BUFA, ADC1BUFB,
                               ADC1BUFC, ADC1BUFD, ADC1BUFE, ADC1BUFF;


uint16_t const volatile *const ADC_p_BUFFERS[16] = {
    &ADC1BUF0, &ADC1BUF1, &ADC1BUF2, &ADC1BUF3, &ADC1BUF4, &ADC1BUF5, &ADC1BUF6, &ADC1BUF7,
    &ADC1BUF8, &ADC1BUF9, &ADC1BUFA, &ADC1BUFB, &ADC1BUFC, &ADC1BUFD, &ADC1BUFE, &ADC1BUFF
};

/********************/
/* Public functions */
/********************/

void ADC_setup(
    uint8_t const num_channels,
    ADC_Channel0Input const ch0_select,
    bool const simultaneous_sampling,
    ADC_SampleTrigger const conversion_trigger,
    ADC_Resolution const resolution
)
{
    AD1CON2bits.CHPS = num_channels - 1;
    AD1CON1bits.SIMSAM = simultaneous_sampling;
    AD1CH0Sbits.CH0SA = ch0_select;
    AD1CON1bits.SSRC = conversion_trigger;
    AD1CON1bits.AD12B = resolution;
}

ADC_AnalogPin const ADC_analog_pins[ADC_PINS_NUMEL] = {
    [ADC_PIN_CH1] = ADC_PIN_CH1,
    [ADC_PIN_CH2] = ADC_PIN_CH2,
    [ADC_PIN_CH3] = ADC_PIN_CH3,
    [ADC_PIN_MIC] = ADC_PIN_MIC,
    [ADC_PIN_CAP] = ADC_PIN_CAP,
    [ADC_PIN_RES] = ADC_PIN_RES,
    [ADC_PIN_VOL] = ADC_PIN_VOL,
};

float const ADC_pin_ranges[ADC_PINS_NUMEL] = {
    [ADC_PIN_CH1] = -33.F,
    [ADC_PIN_CH2] = -33.F,
    [ADC_PIN_CH3] = 6.6F,
    [ADC_PIN_MIC] = 6.6F,
    [ADC_PIN_CAP] = 3.3F,
    [ADC_PIN_RES] = 3.3F,
    [ADC_PIN_VOL] = 3.3F,
};

float const ADC_pin_offsets[ADC_PINS_NUMEL] = {
    [ADC_PIN_CH1] = -16.5F,
    [ADC_PIN_CH2] = -16.5F,
    [ADC_PIN_CH3] = 3.3F,
    [ADC_PIN_MIC] = 3.3F,
    [ADC_PIN_CAP] = 0.F,
    [ADC_PIN_RES] = 0.F,
    [ADC_PIN_VOL] = 0.F,
};
