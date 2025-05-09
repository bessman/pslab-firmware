#ifndef PSLAB_ADC
#define PSLAB_ADC

enum ADC_Source {
    ADC_SOURCE_MANUAL,
    ADC_SOURCE_INT0,
    ADC_SOURCE_TMR3,
    ADC_SOURCE_PWM_SET,
    ADC_SOURCE_TMR5,
    ADC_SOURCE_RESERVED,
    ADC_SOURCE_CTMU,
    ADC_SOURCE_AUTO,
};

enum Status ADC_reset(void);

enum Status ADC_setup(uint8_t n_channels, enum ADC_Source source);

enum Status ADC_start(void);

#endif // PSLAB_ADC
