#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "adc.h"
#include "dma.h"
#include "tmr.h"
#include "types.h"

#include "../helpers/delay.h"

static uint16_t *g_buffer;
static uint16_t g_buffer_n_items;
static uint8_t g_n_channels;

static TMR_Timer const g_CLOCK = TMR_TIMER_5;

static void trigger(void)
{
    /* TODO: Write InterruptCallback to call this from CMP interrupt.
     *
     * CMP4 can compare AN3 (input of the CH1 pin) or AN0 (input of the CH2 pin)
     * with CV_REFIN, which is settable via a resistor ladder.
     *
     * The resistor ladder divides a source voltage, V_RSRC, by 24 or 32 steps.
     * V_RSRC can be set to either AV_DD (3.3 V) or V_REF+ (CH1).
     *
     * When V_RSRC is 3.3 V, CV_REFIN has the following possible values:
     *
     *     CV_REFIN = x / 24 * 3.3 V
     *     CV_REFIN = (1 / 4 + x / 32) * 3.3 V
     *     0 <= x <= 15
     *
     * This gives the following possible unique values:
     *
     * 0.0, 0.137, 0.275, 0.412, 0.55, 0.688, 0.825, 0.928, 0.963, 1.031, 1.1,
     * 1.134, 1.237, 1.341, 1.375, 1.444, 1.512, 1.547, 1.65, 1.753, 1.787,
     * 1.856, 1.925, 1.959, 2.062, 2.166, 2.269, 2.372
     *
     * CH1 and CH2 are bipolar, i.e. their input range spans both negative and
     * positive voltages. They are also inverted, i.e. the maximum positive
     * input voltage corresponds to an ADC output of zero, while the maximum
     * negative value corresponds to the maximum raw value of the ADC.
     *
     * By mapping the above possible CV_REFIN values to a normalized input
     * range, we get the percentage values of the input range at which
     * triggering is possible, in the positive and negative parts of the
     * input range:
     *
     * Positive range (incl. zero):
     * 0,  6,  8,  12,  17,  19,  25,  31, 33, 38, 42, 44, 50, 58, 67, 75, 83, 92
     * Negative range:
     *    -6, -8, -12, -17, -19, -25, -31,    -38,    -44
     *
     * For example, using the full input range of -16.5 V - 16.5 V, the possible
     * trigger values are:
     *
     * 15.12, 13.75, 12.38, 11.0, 9.62, 8.25, 7.22, 6.88, 6.19, 5.5, 5.16,
     * 4.13, 3.09, 2.75, 2.06, 1.37, 1.03, 0.0, -1.03, -1.38, -2.06, -2.75,
     * -3.09, -4.12, -5.16, -6.19, -7.22
     *
     * This means that hardware trigger is unavailable in the lower end of the
     * negative input range.
     *
     * Additionally, since V_RSRC can be set to V_REF+ (the value of CH1), and the
     * resistor ladder bypassed, it is also possible to trigger when the signals
     * at CH1 and CH2 cross each other.
     */
    TMR_start(g_CLOCK);
}

static void reset(void)
{
    TMR_reset(g_CLOCK);
    ADC_reset();
    for (Channel c = CHANNEL_1; c <= g_n_channels; ++c) {
        DMA_reset(c);
    }
    free(g_buffer);
    g_buffer = NULL;
    g_buffer_n_items = 0;
    g_n_channels = 0;
}

/**
 * @brief Clean up resources
 */
static void callback(Channel const channel)
{
    DMA_reset(channel);
    --g_n_channels;

    // If this was the last active channel, clean up ADC and TMR.
    if (!g_n_channels) {
        TMR_reset(g_CLOCK);
        ADC_reset();
    }
}

static enum Status configure_trigger(Channel const trigger_channel)
{
    // TODO: Set up CMP interrupt if trigger is enabled.
    // Trigger immediately.
    trigger();
    return E_OK;
}

static enum Status setup_channels(
    uint8_t const n_channels,
    uint16_t const samples
) {
    enum Status status;

    for (Channel i = CHANNEL_1; i <= n_channels; ++i) {
        uint16_t *const address = g_buffer + i * samples;
        status = DMA_setup(i, samples, (size_t)address, DMA_SOURCE_ADC);
        if (status) { return status; }

        status = DMA_start(i);
        if (status) { return status; }

        status = DMA_interrupt_enable(i, callback);
        if (status) { return status; }
    }

    return E_OK;
}

enum Status OSC_capture(
    uint8_t const n_channels,
    uint16_t const samples,
    uint32_t const delay
) {
    if (g_buffer) { return E_RESOURCE_BUSY; }

    enum Status status = E_OK;
    status = CHANNEL_check(n_channels);
    if (status) { return E_BAD_ARGUMENT; }

    uint16_t const n_items = n_channels * samples;
    g_buffer = malloc(n_items * sizeof(*g_buffer));
    if (!g_buffer) { return E_MEMORY_INSUFFICIENT; }

    g_buffer_n_items = n_items;
    g_n_channels = n_channels;

    uint16_t clock_period;
    enum TMR_Prescaler clock_prescaler;
    status = TMR_get_period_prescaler(delay, &clock_period, &clock_prescaler);
    if (status) { goto error; }

    status = TMR_set_period(g_CLOCK, clock_period);
    if (status) { goto error; }

    status = TMR_set_prescaler(g_CLOCK, clock_prescaler);
    if (status) { goto error; }

    status = setup_channels(n_channels, samples);
    if (status) { goto error; }

    status = ADC_setup(n_channels, g_CLOCK);
    if (status) { goto error; }

    status = ADC_start();
    if (status) { goto error; }

    // Timer isn't running yet. Conversion and DMA do not start until TMR is
    // started in trigger function.
    status = configure_trigger(CHANNEL_NONE);
    if (status) { goto error; }

    return status;

error:
    reset();
    return status;
}

enum Status OSC_fetch(uint16_t **buffer, uint16_t *n_items)
{
    *buffer = g_buffer;
    *n_items = g_buffer_n_items;
    g_buffer = NULL;
    g_buffer_n_items = 0;
    g_n_channels = 0;
    return E_OK;
}
