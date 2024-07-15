/**
 * @file cm.c
 * @author Alexander Bessman (alexander.bessman@gmail.com)
 * @brief Comparator (CM) driver
 */

#include <stdbool.h>
#include <stdint.h>

#include "types.h"

struct CMSTATBits {
    uint16_t C1OUT :1;
    uint16_t C2OUT :1;
    uint16_t C3OUT :1;
    uint16_t C4OUT :1;
    uint16_t :4;
    uint16_t C1EVT :1;
    uint16_t C2EVT :1;
    uint16_t C3EVT :1;
    uint16_t C4EVT :1;
    uint16_t :3;
    uint16_t PSIDL :1;
};

struct CMxCONBits {
    uint16_t CCH :2;
    uint16_t :2;
    uint16_t CREF :1;
    uint16_t :1;
    uint16_t COUT :1;
    uint16_t CEVT :1;
    uint16_t OPMODE :1; // CM1-3 only.
    uint16_t CPOL :1;
    uint16_t COE :1;
    uint16_t CON :1;
};

struct CVRCONBits {
    uint16_t CVR :4;
    uint16_t :1; // CVRSS
    uint16_t CVRR :1;
    uint16_t :1; // CVR1OE
    uint16_t CVREN :1;
    uint16_t :2;
    uint16_t :1; // VREFSEL
    uint16_t :3;
    uint16_t :1; // CVR2OE
};

extern struct CMSTATBits const volatile CMSTATbits;
extern struct CMxCONBits volatile CM1CONbits;
extern struct CMxCONBits volatile CM2CONbits;
extern struct CMxCONBits volatile CM3CONbits;
extern struct CMxCONBits volatile CM4CONbits;
extern struct CVRCONBits volatile CVRCONbits;
