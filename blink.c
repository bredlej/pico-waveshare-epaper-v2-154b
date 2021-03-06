#include <stdio.h>
#include <pico/stdlib.h>
#include <pico-waveshare_ePaper_1_54in.h>

int main() {

	pio_spi_inst_t spi = initDevice();
	initDeviceRegisters(&spi);
	clearScreen(&spi);

	bool isRunning = 1;
	uint32_t seconds = 0;
	while (true) {
		gpio_put(PIN_DBG, 0);

		if (isRunning) {
			printf(". ");
			seconds = seconds + 1;
			if (seconds > 10) {
				isRunning = 0;
				printf("Sleeping\n");
				deepSleep(&spi);
			}
		}

	}
}
