#pragma once

#include <stdbool.h>

typedef struct ip4_addr ip4_addr_t;

bool netusb_init();

const ip4_addr_t *netusb_own_ip();
const ip4_addr_t *netusb_broadcast_ip();

void netusb_task();

void netusb_deinit();
