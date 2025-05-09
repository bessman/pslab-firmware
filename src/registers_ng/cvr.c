/**
 * @file cvr.c
 * @author Alexander Bessman (alexander.bessman@gmail.com)
 * @brief Comparator voltage reference (CVR) driver
 *
 * The CVR is shared by all comparators. Its output, CV_REFIN, is set by a
 * resistor ladder with 24 or 32 steps and 16 tap points. The tap points are
 * located at the bottom of the ladder (if 24 steps are connected), or in the
 * middle of the ladder (if 32 steps are connected).
 *
 * Resistor ladder diagram:
 *
 * 3.3V (VDD) ───┐
 *               │
 *             [ 8R ] (top eight steps are always connected)
 *             [ 1R ]___Tap point (CVR_R15) -> CV_REFIN
 *             [ 1R ]___Tap point (CVR_R14) -> CV_REFIN
 *               ...
 *             [ 1R ]___Tap point (CVR_R0) -> CV_REFIN
 *             [ 8R ] (if CVRR is CVRR_R8)
 *               |
 * GND ──────────┘
 */

#include <limits.h>
#include <stdint.h>

#include "hardware.h"
#include "types.h"
#include "util.h"

#include "cvr.h"

#define V3_3 0xFFFF
#define BASE_LADDER_STEPS 24

enum CVRR {
    // 0 means connect, 1 means bypass.
    CVRR_8R = 0,
    CVRR_0R = 1,
    CVRR_NUMEL
};

static uint16_t const cvrr_steps[CVRR_NUMEL] = {
    [CVRR_8R] = 8,
    [CVRR_0R] = 0,
};

enum CVR {
    CVR_0R,
    CVR_1R,
    CVR_2R,
    CVR_3R,
    CVR_4R,
    CVR_5R,
    CVR_6R,
    CVR_7R,
    CVR_8R,
    CVR_9R,
    CVR_10R,
    CVR_11R,
    CVR_12R,
    CVR_13R,
    CVR_14R,
    CVR_15R,
    CVR_NUMEL
};

/**
 * @brief Get the CV_REFIN voltage for a combination of CVR and CVRR.
 *
 * @param cvrr
 * CVRR connects (CVRR_8R) or bypasses (CVRR_0R) eight resistor steps at the
 * bottom of the ladder.
 *
 * @param cvr
 * CVR selects the ladder step at which CV_REFIN is connected.
 *
 * @return uint16_t
 * CV_REFIN voltage, linearly scaled between 0 (0.0V) and 0xFFFF (3.3V).
 */
static uint16_t get_cvrefin(enum CVRR const cvrr, enum CVR const cvr)
{
    uint16_t const steps = BASE_LADDER_STEPS + cvrr_steps[cvrr];
    uint16_t const round = steps / 2;
    uint32_t const tmp = (uint32_t)V3_3 * (cvr + cvrr_steps[cvrr] + round);
    return (uint16_t)(tmp / steps);
}

/**
 * @brief Set comparator reference voltage.
 *
 * @param[in] voltage
 * Desired reference voltage, linearly scaled between 0 (0.0V) and
 * 0xFFFF (3.3V).
 *
 * @param[out] voltage_actual
 * Actual reference voltage that was set, linearly scaled between 0 (0.0V) and
 * 0xFFFF (3.3V). Caller may pass NULL if the actual voltage is not of
 * interest.
 *
 * @return enum Status
 */
enum Status CVR_set_ref(uint16_t const voltage, uint16_t *const voltage_actual)
{
    int32_t diff_best = INT32_MAX;
    enum CVRR cvrr_best = CVRR_0R;
    enum CVR cvr_best = CVR_0R;

    for (int cvrr = 0; cvrr < CVRR_NUMEL; ++cvrr) {
        for (int cvr = 0; cvr < CVR_NUMEL; ++cvr) {
            int32_t const diff = (int32_t)voltage - get_cvrefin(cvrr, cvr);
            if (abs32(diff) < abs32(diff_best)) {
                diff_best = diff;
                cvrr_best = cvrr;
                cvr_best = cvr;
            }
        }
    }

    CVRCONbits->CVRR = cvrr_best;
    CVRCONbits->CVR = cvr_best;
    CVRCONbits->CVREN = 1;

    if (voltage_actual) {
        *voltage_actual = get_cvrefin(cvrr_best, cvr_best);
    }

    return E_OK;
}

/**
 * @brief Set comparator reference voltage to VREF+.
 *
 * VREF+ is an external pin, which is connected to CH1. Setting CV_REFIN to
 * this can be used to trigger a comparator interrupt when the comparator's
 * input signal crosses the signal on CH1.
 *
 * CH1 is connected to VREF+ via an op-amp and a programmable gain amplifier.
 * See the oscilloscope module for details.
 *
 * @return enum Status
 */
enum Status CVR_set_ref_external(void)
{
    CVRCONbits->VREFSEL = 1;
    CVRCONbits->CVREN = 0;
    return E_OK;
}
