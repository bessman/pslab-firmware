/**
 * @file oscilloscope.c
 * @author Alexander Bessman (alexander.bessman@gmail.com)
 * @brief High-level driver for the PSLab's Oscilloscope (OSC) instrument
 * @details
 */

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../bus/spi/spi.h"
#include "../bus/uart/uart.h"
#include "helpers/buffer.h"
#include "helpers/delay.h"
#include "registers_ng/adc.h"
#include "registers_ng/dma.h"
#include "registers_ng/tmr.h"
#include  "commands.h"

enum Gain {
    GAIN_X1,
    GAIN_X2,
    GAIN_X4,
    GAIN_X5,
    GAIN_X8,
    GAIN_X10,
    GAIN_X16,
    GAIN_X32,
    GAIN_INVALID,
};

struct OscilloscopeState {
    uint8_t num_channels;
    ADC_Channel0Input ch0map;
    uint16_t resolution;
    enum Gain gains[CHANNEL_NUMEL];
};

struct TriggerState {
    Channel channel;
    uint16_t level;
    uint16_t waiting;
    uint16_t timeout;
    bool ready;
    bool polarity;
};

static struct OscilloscopeState g_state;
static struct TriggerState g_trigger_state;

TMR_Timer const g_TIMER = TMR_TIMER_5;

static void setup(
    uint8_t const num_channels,
    uint16_t const samples,
    float const timegap,
    uint16_t const ch1_gain,
    uint16_t const ch2_gain
)
{
    g_state.gains[CHANNEL_1] = ch1_gain;
    g_state.gains[CHANNEL_2] = ch2_gain;
    g_state.gains[CHANNEL_3] = 1;
    g_state.gains[CHANNEL_4] = 1;

    uint16_t const min_12bit_timegap = 8;
    bool const slow_sampling = timegap >= min_12bit_timegap;
    bool const single_channel = num_channels == 1;
    bool const use_12bit = slow_sampling && single_channel;
    ADC_reset();
    ADC_setup(
        num_channels,
        ADC_CHANNEL_0_INPUT_CH1,
        ADC_SAMPLING_MODE_SIMULTANEOUS,
        ADC_SAMPLE_TRIGGER_TMR5,
        use_12bit ? ADC_RESOLUTION_12BIT : ADC_RESOLUTION_10BIT
    );
    ADC_start();

    TMR_Prescaler prescaler = TMR_PRESCALER_1;
    uint32_t delay = (uint32_t)(timegap * FCY);

    while (delay > UINT16_MAX) {
        ++prescaler;
        delay /= 8;
    }

    TMR_reset(g_TIMER);
    TMR_set_prescaler(g_TIMER, prescaler);
    TMR_set_period(g_TIMER, (uint16_t)delay);

    uint16_t const resolution = 1 << (use_12bit ? 12 : 10);

    for (uint8_t i = 0; i < num_channels; ++i) {
        DMA_reset(i);
        size_t const address =
            (size_t)(BUFFER + i * BUFFER_SIZE / num_channels);
        DMA_setup(i, samples, address, DMA_SOURCE_ADC);
        uint16_t const gain = g_state.gains[i];

        float const scaling = ADC_pin_ranges[i] / resolution / gain;
        float const offset = ADC_pin_offsets[i] / gain;
        UART1_write_u32(*(uint32_t *)&scaling);
        UART1_write_u32(*(uint32_t *)&offset);
        UART1_write_str("V");
    }
}

static void arm(
    Channel const trigger_channel,
    float const trigger_voltage,
    Edge const trigger_dir,
    uint16_t const trigger_timeout
)
{
    g_trigger_state.channel = trigger_channel;
    g_trigger_state.waiting = 0;
    g_trigger_state.timeout = trigger_timeout;
    g_trigger_state.ready = false;

    uint16_t const gain = g_state.gains[trigger_channel];
    float const range = ADC_pin_ranges[trigger_channel] / gain;
    float const offset = ADC_pin_offsets[trigger_channel] / gain;
    uint16_t const resolution = g_state.resolution;
    int16_t const level = (trigger_voltage - offset) / range * resolution;
    // Clamp trigger level to the resolution range, excluding zero. Sample
    // polarity is true at or above trigger level, which means zero must be
    // excluded or trigger condition will never be met.
    g_trigger_state.level = (uint16_t)clamp(level, 1, resolution - 1);

    // NB: Here, "polarity" refers to either side of the trigger voltage.
    // polarity == true means that capture starts when the voltage crosses the
    // trigger voltage from below.
    if (trigger_dir == EDGE_ANY) {
        // Set trigger polarity to the opposite of the current sample polarity.
        g_trigger_state.polarity =
            *ADC_p_BUFFERS[trigger_channel] < g_trigger_state.level;
        // Ready to trigger when the voltage crosses the trigger level.
        g_trigger_state.ready = true;
    } else {
        g_trigger_state.polarity = trigger == EDGE_RISING;
    }

    ADC_enable_interrupt(trigger);
}

static int32_t clamp(int32_t const v, int32_t const lo, int32_t const hi)
{
    return max(lo, min(hi, v));
}

static void trigger(Channel const channel)
{
    struct TriggerState ts = g_trigger_state;
    bool triggered = false;

    // Trigger timeout?
    triggered |= ++ts.waiting > ts.timeout;

    // Don't trigger immediately if the voltage starts at or above the trigger
    // value.
    bool const sample_polarity = *ADC_p_BUFFERS[ts.channel] >= ts.level;
    bool const polarity_equal = ts.polarity == sample_polarity;
    ts.ready |= !polarity_equal;

    // Trigger condition met?
    triggered |= ts.ready & polarity_equal;

    if (triggered) {
        switch (g_state.num_channels) {
        case 4:
            DMA3_start_fast();
        case 3:
            DMA2_start_fast();
        case 2:
            DMA1_start_fast();
        case 1:
            DMA0_start_fast();
        default:
            break;
        }

        ADC_disable_interrupt();
    }
}

static void Capture(void) {
    uint8_t config = UART1_Read();
    SetSAMPLES_REQUESTED(UART1_ReadInt());
    SetDELAY(UART1_ReadInt()); // Wait DELAY / 8 us between samples.

    uint8_t ch0sa = config & 0x0F;
    uint8_t ch123sa = config & 0x10;
    uint8_t trigger = config & 0x80;

    ADC1_SetOperationMode(ADC1_10BIT_SIMULTANEOUS_MODE, ch0sa, ch123sa);

    /* Check if the trigger channel is sampled. If not, convert the trigger
     * channel in addition to the sampled channels. */
    if (trigger && GetTRIGGER_CHANNEL() > GetCHANNELS()) {
        ADC1_ConversionChannelsSet(GetTRIGGER_CHANNEL());
        ResetTrigger();
    } else if (trigger) {
        ADC1_ConversionChannelsSet(GetCHANNELS());
        ResetTrigger();
    } else {
        ADC1_ConversionChannelsSet(GetCHANNELS());
        SetTRIGGERED(1);
    }

    int i;
    for (i = 0; i <= GetCHANNELS(); i++) {
        SetBUFFER_IDX(i, &BUFFER[i * GetSAMPLES_REQUESTED()]);
    }

    SetCONVERSION_DONE(0);
    SetSAMPLES_CAPTURED(0);
    SetBUFFER_IDX(0, &BUFFER[0]);
    SetTimeGap();
    ADC1_InterruptFlagClear();
    ADC1_InterruptEnable();
    LED_SetLow();
}

response_t OSCILLOSCOPE_CaptureDMA(void) {
    uint8_t config = UART1_Read();
    SetSAMPLES_REQUESTED(UART1_ReadInt());
    SetDELAY(UART1_ReadInt());  // Wait DELAY / 8 us between samples.

    uint8_t ch0sa = config & 0x0F;
    uint8_t mode = config & 0x80 ? ADC1_12BIT_DMA_MODE : ADC1_10BIT_DMA_MODE;

    SetCHANNELS(0);  // Capture one channel.
    ADC1_SetOperationMode(mode, ch0sa, 0);

    DMA_StartAddressASet(DMA_CHANNEL_0, (uint16_t) &BUFFER[0]);
    DMA_PeripheralAddressSet(DMA_CHANNEL_0, (uint16_t) &ADC1BUF0);
    DMA_TransferCountSet(DMA_CHANNEL_0, GetSAMPLES_REQUESTED() - 1);
    DMA_FlagInterruptClear(DMA_CHANNEL_0);
    DMA_InterruptEnable(DMA_CHANNEL_0);
    DMA_ChannelEnable(DMA_CHANNEL_0);

    SetSAMPLES_CAPTURED(GetSAMPLES_REQUESTED());
    SetCONVERSION_DONE(1); // Assume it's all over already.
    SetTimeGap();
    LED_SetLow();

    return SUCCESS;
}

static void ResetTrigger(void) {
    SetTRIGGER_WAITING(0);
    SetTRIGGER_READY(0);
    SetTRIGGERED(0);
}

static void SetTimeGap(void) {
    TMR5_Initialize();
    TMR5_StopWhenIdle();
    TMR5_Period16BitSet(GetDELAY() - 1);
    TMR5_SetPrescaler(TMR_PRESCALER_8);
    TMR5_InterruptFlagClear();
    TMR5_InterruptDisable();
    TMR5_Start();
}

response_t OSCILLOSCOPE_GetCaptureStatus(void) {
    UART1_Write(GetCONVERSION_DONE());
    UART1_WriteInt(GetSAMPLES_CAPTURED());
    return SUCCESS;
}

response_t OSCILLOSCOPE_ConfigureTrigger(void) {
    uint8_t config = UART1_Read();
    uint8_t channelbits = config & 0x0F;

    int i;
    for (i = 0; i < MAX_CHANNELS; i++) {
        if (channelbits & (1 << i)) {
            SetTRIGGER_CHANNEL(i);
            break;
        }
    }

    SetTRIGGER_PRESCALER(config >> 4);
    SetTRIGGER_LEVEL(UART1_ReadInt());

    return SUCCESS;
}

response_t OSCILLOSCOPE_SetPGAGain(void) {
    tSPI_CS channel;

    switch (UART1_Read()) {
    case 1:
        channel = SPI_CH1;
        break;
    case 2:
        channel = SPI_CH2;
        break;
    default:
        UART1_Read(); // Consume remaining data.
        return FAILED;
    }

    enum Gain const gain = UART1_Read();

    if (gain >= GAIN_INVALID) {
        return FAILED;
    }

    uint16_t const write_register = 0x4000;
    uint16_t cmd = write_register | gain;

    SPI_Config const pga_config = {{{
        .PPRE = SPI_SCLK125000 >> 3,
        .SPRE = SPI_SCLK125000 & 7,
        .MSTEN = 1,
        .CKP = SPI_IDLE_LOW,
        .SSEN = 0,
        .CKE = SPI_SHIFT_TRAILING,
        .SMP = 1,
        .MODE16 = 1,
        .DISSDO = 0,
        .DISSCK = 0
    }}};

    if (SPI_configure(pga_config)) {
        LED_SetHigh();
        return SPI_exchange_int(channel, &cmd);
    }

    return FAILED;
}
