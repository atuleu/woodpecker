#pragma once

#include <stdbool.h>
#include <stdint.h>

void ADC_init();

void ADC_start();

#define ADC_RESULT_NOT_READY 0xffff

uint16_t ADC_result(uint8_t index);
