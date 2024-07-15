#ifndef TMR_H
#define TMR_H

#include <stdint.h>

/*********/
/* Types */
/*********/

typedef enum TMR_Timer {
    TMR_TIMER_1,
    TMR_TIMER_2,
    TMR_TIMER_3,
    TMR_TIMER_4,
    TMR_TIMER_5,
    TMR_TIMER_NUMEL,
} TMR_Timer;

typedef enum TMR_Prescaler {
    TMR_PRESCALER_1 = 0b00,
    TMR_PRESCALER_8 = 0b01,
    TMR_PRESCALER_64 = 0b10,
    TMR_PRESCALER_256 = 0b11,
} TMR_Prescaler;

/*************/
/* Functions */
/*************/

/**
 * @brief Reset timer to default configuration
 *
 * @param timer
 */
void TMR_reset(TMR_Timer timer);

/**
 * @brief Start timer
 *
 * @param timer
 */
void TMR_start(TMR_Timer timer);

/**
 * @brief Start TMR5
 * @details
 * Prefer `TMR_start` over `TMRx_start_fast`. Use the latter only in
 * timing-critical sections.
 */
static inline void TMR5_start_fast(void)
{
    extern uint16_t volatile T5CON;
    T5CON |= 1 << 15;
}

/**
 * @brief Set timer period
 * @details
 * Timer sync output is asserted high on period match, and a timer interrupt is
 * generated if enabled.
 *
 * @param timer
 * @param period
 */
void TMR_set_period(TMR_Timer timer, uint16_t period);

/**
 * @brief Set timer prescaler
 *
 * @param timer
 * @param prescaler
 */
void TMR_set_prescaler(TMR_Timer timer, TMR_Prescaler prescaler);

#endif // TMR_H
