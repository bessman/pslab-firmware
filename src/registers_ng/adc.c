#include <stdint.h>

#include "types.h"

#include "adc.h"

struct ADCON1Bits {
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
    uint16_t ADSIDL :1;
    uint16_t :1;
    uint16_t ADON :1;
  };

struct ADCON2Bits {
    uint16_t ALTS :1;
    uint16_t BUFM :1;
    uint16_t SMPI :5;
    uint16_t BUFS :1;
    uint16_t CHPS :2;
    uint16_t CSCNA :1;
    uint16_t :2;
    uint16_t VCFG :3;
};

struct ADCON3Bits {
    uint16_t ADCS:8;
    uint16_t SAMC:5;
    uint16_t :2;
    uint16_t ADRC:1;
};

struct ADCON4Bits {
    uint16_t CH123SA:1;
    uint16_t CH123NA:2;
    uint16_t :5;
    uint16_t CH123SB:1;
    uint16_t CH123NB:2;
};

struct ADCRegisters {
    struct ADCON1Bits volatile *const p_con1bits;
    struct ADCON2Bits volatile *const p_con2bits;
    struct ADCON3Bits volatile *const p_con3bits;
    struct ADCON4Bits volatile *const p_con4bits;
};

extern struct ADCON1Bits volatile AD1CON1bits;
extern struct ADCON2Bits volatile AD1CON2bits;
extern struct ADCON3Bits volatile AD1CON3bits;
extern struct ADCON4Bits volatile AD1CON4bits;

static struct ADCRegisters const g_ADC_REGS = {
    .p_con1bits = &AD1CON1bits,
    .p_con2bits = &AD1CON2bits,
    .p_con3bits = &AD1CON3bits,
    .p_con4bits = &AD1CON4bits,
};

enum Status ADC_reset(void)
{
    static struct ADCConf {
        struct ADCON1Bits const con1bits;
        struct ADCON2Bits const con2bits;
        struct ADCON3Bits const con3bits;
        struct ADCON4Bits const con4bits;
    } const default_conf = {
        .con1bits = { 0 },
        .con2bits = { 0 },
        .con3bits = { 0 },
        .con4bits = { 0 },
    };
    struct ADCRegisters const *const regs = &g_ADC_REGS;
    *regs->p_con1bits = default_conf.con1bits;
    *regs->p_con2bits = default_conf.con2bits;
    *regs->p_con3bits = default_conf.con3bits;
    *regs->p_con4bits = default_conf.con4bits;
    return E_OK;
}

enum Status ADC_setup(uint8_t const n_channels, enum ADC_Source const source)
{
    enum Status status = E_OK;
    if ( (status = CHANNEL_check(n_channels)) ) { return E_BAD_ARGUMENT; }

    struct ADCRegisters const *const regs = &g_ADC_REGS;
    regs->p_con1bits->SSRC = source;

    enum CHPS {
        SINGLE_CHANNEL,
        DUAL_CHANNEL,
        QUAD_CHANNEL,
    };
    static enum CHPS const simultaneous_channels[CHANNEL_NUMEL] = {
        [CHANNEL_1] = SINGLE_CHANNEL,
        [CHANNEL_2] = DUAL_CHANNEL,
        [CHANNEL_3] = QUAD_CHANNEL,
        [CHANNEL_4] = QUAD_CHANNEL,
    };
    regs->p_con2bits->CHPS = simultaneous_channels[n_channels];

    enum SMPI {
        INCREMENT_DMA_ADDRESS_EVERY_1_CONVERSIONS,
        INCREMENT_DMA_ADDRESS_EVERY_2_CONVERSIONS,
        INCREMENT_DMA_ADDRESS_EVERY_3_CONVERSIONS,
        INCREMENT_DMA_ADDRESS_EVERY_4_CONVERSIONS,
    };
    static enum SMPI const dma_address_increment_rate[CHANNEL_NUMEL] = {
        [CHANNEL_1] = INCREMENT_DMA_ADDRESS_EVERY_1_CONVERSIONS,
        [CHANNEL_2] = INCREMENT_DMA_ADDRESS_EVERY_2_CONVERSIONS,
        [CHANNEL_3] = INCREMENT_DMA_ADDRESS_EVERY_3_CONVERSIONS,
        [CHANNEL_4] = INCREMENT_DMA_ADDRESS_EVERY_4_CONVERSIONS,
    };
    regs->p_con2bits->SMPI = dma_address_increment_rate[n_channels];

    return status;
}

enum Status ADC_start(void)
{
    struct ADCRegisters const *const regs = &g_ADC_REGS;
    regs->p_con1bits->ASAM = 1;
    regs->p_con1bits->SIMSAM = 1;
    regs->p_con3bits->ADCS = 1;
    regs->p_con1bits->ADON = 1;
    return E_OK;
}
