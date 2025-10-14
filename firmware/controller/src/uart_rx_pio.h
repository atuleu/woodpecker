#pragma once

#include <hardware/pio.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UART_SIZE        16
#define UART_MASK        (UART_SIZE - 1)
#define UART_PACKET_SIZE 9

_Static_assert(
    UART_SIZE > 0 && (UART_SIZE & UART_MASK) == 0,
    "UART_SIZE should be a power of two"
);

struct UART_Rx {
	uint8_t         buffer[UART_SIZE * UART_PACKET_SIZE];
	uint32_t        head, tail;
	int             pin;
	PIO             pio;
	int             sm;
	int             offset;
	int             dma;
	uint32_t        tx_errors;
	absolute_time_t deadline;
};
typedef struct UART_Rx UART_Rx_t;

// init an UART_rx on given pin
int  UART_Rx_init(UART_Rx_t *uart, int pin, int baudrate);
// deinit UART_rx
void UART_Rx_deinit(UART_Rx_t *uart);

// Returns the number of available packet
size_t UART_available(UART_Rx_t *uart);

// Copy as many available packet on buffer. return the number of copied packed.
// Returns the number of packet written. Returns
// PICO_ERROR_INSUFFICIENT_BUFFER_SIZE if len < UART_PACKET_SIZE
int UART_Rx_get(UART_Rx_t *uart, uint8_t *buffer, size_t len);

// Checks if an UART is blocked (not sufficient character received before an
// internal deadline, and resets it. return true in that case. If the UART
// is fien, returns false.
bool UART_Rx_check_and_unblock(UART_Rx_t *uart);

#ifdef __cplusplus
}
#endif
