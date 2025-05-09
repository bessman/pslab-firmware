#ifndef PSLAB_HARDWARE
#define PSLAB_HARDWARE

// IO

#define CNENB (*(uint16_t volatile *)0x0E18)

enum CNENB_Bits {
    CNENB_BITS_LA1 = 1 << 10,
    CNENB_BITS_LA2 = 1 << 11,
    CNENB_BITS_LA3 = 1 << 12,
    CNENB_BITS_LA4 = 1 << 13,
};

// Interrupt control

#define IEC0 (*(uint16_t volatile *)0x0820)
#define IEC1 (*(uint16_t volatile *)0x0822)
#define IEC2 (*(uint16_t volatile *)0x0824)

enum IEC1_Bits {
    IEC1_BITS_CMIE = 1 << 2,
    IEC1_BITS_CNIE = 1 << 3,
};

#define IFS0 (*(uint16_t volatile *)0x0800)
#define IFS1 (*(uint16_t volatile *)0x0802)
#define IFS2 (*(uint16_t volatile *)0x0804)

enum IFS1_Bits {
    IFS1_BITS_CMIF = 1 << 2,
    IFS1_BITS_CNIF = 1 << 3,
};

// Comparator

#define CMSTAT_ADDR ((uint16_t volatile *)0x0A80)
#define CMSTAT (*CMSTAT_ADDR)
struct CMSTATBits {
    uint16_t C1OUT:1;
    uint16_t C2OUT:1;
    uint16_t C3OUT:1;
    uint16_t C4OUT:1;
    uint16_t :4;
    uint16_t C1EVT:1;
    uint16_t C2EVT:1;
    uint16_t C3EVT:1;
    uint16_t C4EVT:1;
    uint16_t :3;
    uint16_t PSIDL:1;
} volatile *const CMSTATbits = (struct CMSTATBits volatile *)CMSTAT_ADDR;

#define CVRCON_ADDR ((uint16_t volatile *)0x0A82)
#define CVRCON (*CVRCON_ADDR)
struct CVRCONBits {
    // Resistor ladder tap point
    uint16_t CVR:4;
    // Resistor ladder source
    uint16_t CVRSS:1;
    // Resistor ladder range
    uint16_t CVRR:1;
    // CV_REF1O output enable
    uint16_t CVROE:1;
    // Resistor ladder enable
    uint16_t CVREN:1;
    uint16_t :2;
    // CV_REFIN select
    uint16_t VREFSEL:1;
    uint16_t :3;
    // CV_REF2O output enable
    uint16_t CVR2OE:1;
} volatile *const CVRCONbits = (struct CVRCONBits volatile *)CVRCON_ADDR;

struct CMCONBits {
    // Channel select
    uint16_t CCH:2;
    uint16_t :2;
    // Reference select
    uint16_t CREF:1;
    uint16_t :1;
    // Event polarity
    uint16_t EVPOL:2;
    // Output
    uint16_t COUT:1;
    // Event
    uint16_t CEVT:1;
    // Operation mode (not available in CM4, which cannot be used as an op-amp)
    uint16_t OPMODE:1;
    uint16_t :2;
    // Output polarity
    uint16_t CPOL:1;
    // Output enable
    uint16_t COE:1;
    // Enable
    uint16_t CON:1;
};
#define CM1CON_ADDR ((uint16_t volatile *)0x0A84)
#define CM1CON (*CM1CON_ADDR)
struct CMCONBits volatile *const CM1CONbits = (struct CMCONBits *)CM1CON_ADDR;
#define CM2CON_ADDR ((uint16_t volatile *)0x0A8C)
#define CM2CON (*CM2CON_ADDR)
struct CMCONBits volatile *const CM2CONbits = (struct CMCONBits *)CM2CON_ADDR;
#define CM3CON_ADDR ((uint16_t volatile *)0x0A94)
#define CM3CON (*CM3CON_ADDR)
struct CMCONBits volatile *const CM3CONbits = (struct CMCONBits *)CM3CON_ADDR;
#define CM4CON_ADDR ((uint16_t volatile *)0x0A9C)
#define CM4CON (*CM4CON_ADDR)
struct CMCONBits volatile *const CM4CONbits = (struct CMCONBits *)CM4CON_ADDR;

#endif // PSLAB_HARDWARE
