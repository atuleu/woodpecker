#pragma once

#include <stdbool.h>
#include <stdint.h>

void init_UART(bool alternate);

void UART_putc(char c);

void UART_wait_free();

uint8_t UART_available();

struct Frame;

void UART_push_frame(const struct Frame *f);
