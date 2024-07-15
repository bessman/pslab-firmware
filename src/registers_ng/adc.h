#ifndef ADC_H
#define ADC_H

#include <stdint.h>

typedef enum ADC_AnalogPin {
    ADC_PIN_CH1,
    ADC_PIN_CH2,
    ADC_PIN_CH3,
    ADC_PIN_MIC,
    ADC_PIN_CAP,
    ADC_PIN_RES,
    ADC_PIN_VOL,
    ADC_PINS_NUMEL
} ADC_AnalogPin;

typedef enum ADC_Channel0Input {
    ADC_CHANNEL_0_INPUT_CH2 = 0b00000,
    ADC_CHANNEL_0_INPUT_CH3 = 0b00001,
    ADC_CHANNEL_0_INPUT_MIC = 0b00010,
    ADC_CHANNEL_0_INPUT_CH1 = 0b00011,
    ADC_CHANNEL_0_INPUT_CAP = 0b00101,
    ADC_CHANNEL_0_INPUT_RES = 0b00111,
    ADC_CHANNEL_0_INPUT_VOL = 0b01000,
    ADC_CHANNEL_0_INPUT_CTMU_TEMP = 0b11110,
    ADC_CHANNEL_0_INPUT_CTMU = 0b11111,
    ADC_CHANNEL_0_INPUTS_NUMEL
} ADC_Channel0Input;

typedef enum ADC_SamplingMode {
    ADC_SAMPLING_MODE_SEQUENTIAL = 0b0,
    ADC_SAMPLING_MODE_SIMULTANEOUS = 0b1,
} ADC_SamplingMode;

typedef enum ADC_SampleTrigger {
    ADC_SAMPLE_TRIGGER_MANUAL = 0b0000,
    ADC_SAMPLE_TRIGGER_TMR5 = 0b0100,
    ADC_SAMPLE_TRIGGER_CTMU = 0b0110,
    ADC_SAMPLE_INTERNAL_CTR = 0b0111,
} ADC_SampleTrigger;

typedef struct ADC_Config {
    ADC_Channel0Input channel_0_select;
    bool simultaneous_sampling;
    ADC_SampleTrigger sample_trigger;
} ADC_Config;

typedef enum ADC_Resolution {
    ADC_RESOLUTION_10BIT = 0b0,
    ADC_RESOLUTION_12BIT = 0b1,
} ADC_Resolution;

enum {ADC_BUFFERS_NUMEL = 16};
extern uint16_t const volatile *const ADC_p_BUFFERS[ADC_BUFFERS_NUMEL];

extern ADC_AnalogPin const ADC_analog_pins[ADC_PINS_NUMEL];
extern float const ADC_pin_ranges[ADC_PINS_NUMEL];
extern float const ADC_pin_offsets[ADC_PINS_NUMEL];

#endif // ADC_H
