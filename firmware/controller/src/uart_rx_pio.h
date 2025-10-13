#pragma once

#include <hardware/pio.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#define UART_SIZE        16
#define UART_PACKET_SIZE 9
_Static_assert(
    UART_SIZE > 0 && (UART_SIZE & (UART_SIZE - 1)) == 0,
    "UART_SIZE should be a power of two"
);

struct UART_Rx {
	uint8_t  buffer[UART_SIZE * UART_PACKET_SIZE];
	uint32_t head, tail;
	int      pin;
	PIO      pio;
	int      sm;
	int      offset;
	int      dma;
};
typedef struct UART_Rx UART_Rx_t;

int  UART_Rx_init(UART_Rx_t *uart, int pin, int baudrate);
void UART_Rx_deinit(UART_Rx_t *uart);

size_t  UART_Rx_available(UART_Rx_t *uart);
uint8_t UART_Rx_getc(UART_Rx_t *uart);
uint8_t UART_Rx_unsafe_getc(UART_Rx_t *uart);

#ifdef __cplusplus
}
#endif
