
#include <pico/stdlib.h>

#include <stdio.h>

#include "netusb.h"

int main() {
	stdio_init_all();

	printf("Woodpecker hello-world\n");

	if (netusb_init() == false) {
		printf("could not initialize netusb. Bye!\n");
	}

	while (true) {
		netusb_task();
	}
	netusb_deinit();
}
