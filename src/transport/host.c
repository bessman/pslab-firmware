#include "../commands.h"
#include "../bus/uart/uart.h"

// Primary control bus.
#define HOST_UART0 U1SELECT
#define HOST_UART1 U2SELECT

#ifndef PSLAB_HOST
#define PSLAB_HOST HOST_UART0
#endif // PSLAB_HOST

enum Status HOST_read(
    uint8_t *const buffer,
    uint16_t const size
) {
    enum Status status;
    switch (status = UART_read(PSLAB_HOST, buffer, size)) {
    case E_UART_RX_TIMEOUT:
    case E_UART_RX_PARITY:
    case E_UART_RX_FRAMING:
        return E_HOST_READ;
    case E_UART_RX_OVERRUN:
        return E_HOST_RX_OVERRUN;
    default:
        return status;
    }
}

enum Status HOST_write(
    uint8_t const *const buffer,
    uint16_t const size
) {
    return UART_write(PSLAB_HOST, buffer, size);
}

enum Status HOST_flush_rx(void)
{
    return UART_flush_rx(PSLAB_HOST);
}
