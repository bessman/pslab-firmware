/**
 * @file cmp.c
 * @author Alexander Bessman (alexander.bessman@gmail.com)
 * @brief Comparator (CM) driver
 *
 * See also cvr.c (comparator voltage reference).
 */

 #include <stdbool.h>
 #include <stdint.h>

#include "hardware.h"
#include "types.h"

#define INTERRUPT_CTRL_REG IEC1
#define INTERRUPT_FLAG_REG IFS1
#define CTRL_BIT IEC1_BITS_CMIE
#define FLAG_BIT IFS1_BITS_CMIF

static InterruptCallback g_callbacks[CHANNEL_NUMEL] = {
    [CHANNEL_1] = default_callback,
    [CHANNEL_2] = default_callback,
    [CHANNEL_3] = default_callback,
    [CHANNEL_4] = default_callback,
};

static struct CMCONBits volatile *const g_CM_REGS[CHANNEL_NUMEL] = {
    [CHANNEL_1] = CM1CONbits,
    [CHANNEL_2] = CM2CONbits,
    [CHANNEL_3] = CM3CONbits,
    [CHANNEL_4] = CM4CONbits,
};

static void default_callback(__attribute__((unused))Channel const channel)
{
    interrupt_disable();
}

/**
 * @brief
 *
 * All comparators share a single interrupt vector. The interrupt triggers
 * when an event occurs on any enabled CMx. This function checks the CxEVT
 * bits of CMSTAT to see which comparator(s) triggered, and calls the
 * appropriate callbacks.
 */
void __attribute__((__interrupt__, no_auto_psv)) _CM1Interrupt(void)
{
    interrupt_clear();

    if (CM1CONbits->CON && CMSTATbits->C1EVT) {
        CMSTATbits->C1EVT = 0;
        g_callbacks[CHANNEL_1](CHANNEL_1);
    }

    if (CM2CONbits->CON && CMSTATbits->C2EVT) {
        CMSTATbits->C2EVT = 0;
        g_callbacks[CHANNEL_2](CHANNEL_2);
    }

    if (CM3CONbits->CON && CMSTATbits->C3EVT) {
        CMSTATbits->C3EVT = 0;
        g_callbacks[CHANNEL_3](CHANNEL_3);
    }

    if (CM4CONbits->CON && CMSTATbits->C4EVT) {
        CMSTATbits->C4EVT = 0;
        g_callbacks[CHANNEL_4](CHANNEL_4);
    }
}

static bool is_busy(Channel const channel)
{
    return g_callbacks[channel] != default_callback;
}

enum Status CM_enable_interrupt(
    Channel const channel,
    InterruptCallback const callback
) {
    enum Status status;

    status = CHANNEL_check(channel);
    if (status) { return status; }

    if (is_busy(channel)) { return E_RESOURCE_BUSY; }

    g_callbacks[channel] = callback;
    g_CM_REGS[channel]->CON = 1;
    INTERRUPT_CTRL_REG |= CTRL_BIT;

    return E_OK;
}

enum Status CM_disable_interrupt(Channel const channel)
{
    enum Status status;

    status = CHANNEL_check(channel);
    if (status) { return status; }

    g_CM_REGS[channel]->CON = 0;
    g_callbacks[channel] = default_callback;

    for (int c = CHANNEL_1; c < CHANNEL_NUMEL; ++c) {
        if (g_callbacks[c] != default_callback) { return E_OK; }
    }

    INTERRUPT_CTRL_REG &= ~CTRL_BIT;
    return E_OK;
}
