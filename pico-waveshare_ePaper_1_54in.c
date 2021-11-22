//
// Created by geoco on 27.01.2021.
//
#include <pico-waveshare_ePaper_1_54in.h>

static char event_string[128];

void gpio_event_string(char *buf, uint32_t events);

static inline void cs_select() {	
	gpio_put(PIN_CS, 0); // Active low	
}

static inline void cs_deselect() {
	asm volatile("nop \n nop \n nop");
	gpio_put(PIN_CS, 1);
	asm volatile("nop \n nop \n nop");
}
void startLed() {
	gpio_put(PIN_LED, 1);
}

void stopLed() {
	gpio_put(PIN_LED, 0);
}
void gpio_callback(uint gpio, uint32_t events) {
	char *gpio_descriptor;
	switch (gpio) {
	case PIN_CS:
		gpio_descriptor = "C/S";
		break;
	case PIN_MOSI:
		gpio_descriptor = "DIN";
		break;
	case PIN_CLK:
		gpio_descriptor = "CLK";
		break;
	case PIN_DC:
		gpio_descriptor = "D/C";
		break;
	case PIN_RESET:
		gpio_descriptor = "RST";
		break;
	case PIN_MISO:
		gpio_descriptor = "BUSY";
		break;
	default:
		gpio_descriptor = "???";
	}
	gpio_event_string(event_string, events);

	printf("GPIO: %d=[%s] %s\n", gpio, gpio_descriptor, event_string);
	if (gpio == PIN_MISO) {
		printf("BUSY: %d\n", gpio_get(PIN_MISO));
	}
}


void blink(uint8_t amount, uint32_t time) {
	uint8_t i = 0;
	while (i < amount) {
		gpio_put(PIN_LED, 1);
		sleep_ms(time);
		gpio_put(PIN_LED, 0);
		i++;
	}
}

pio_spi_inst_t initDevice() {
	stdio_init_all();
	pio_spi_inst_t spi = { 
		.pio = pio0,
		.sm = 0,
		.cs_pin = PIN_CS
	};
	
	uint offset = pio_add_program(spi.pio, &spi_cpha0_program);
	printf("Loaded program at %d\n", offset);

	pio_spi_init(spi.pio,
		spi.sm,
		offset,
		8,       // 8 bits per SPI frame
		31.25f,  // 1 MHz @ 125 clk_sys
		false,   // CPHA = 0
		false,   // CPOL = 0
		PIN_CLK,
		PIN_MOSI,
		PIN_MISO);
	
	// Chip select is active-low, so we'll initialise it to a driven-high state
	gpio_init(PIN_CS);
	gpio_set_dir(PIN_CS, GPIO_OUT);
	gpio_put(PIN_CS, 0);
	
	/*
	 *spi_init(SPI_PORT, 1000 * 1000);
	spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

	gpio_set_irq_enabled_with_callback(PIN_MISO, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);	

	gpio_set_function(PIN_CS, GPIO_FUNC_SPI);
	gpio_set_function(PIN_CLK, GPIO_FUNC_SPI);
	gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
	gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
	*/
	
	gpio_init(PIN_DBG);
	gpio_set_dir(PIN_DBG, GPIO_OUT);

	gpio_init(PIN_RESET);
	gpio_set_dir(PIN_RESET, GPIO_OUT);
	gpio_put(PIN_RESET, 1);

	gpio_init(PIN_DC);
	gpio_set_dir(PIN_DC, GPIO_OUT);
	gpio_put(PIN_DC, 0);

	gpio_init(PIN_LED);

	//gpio_set_irq_enabled_with_callback(PIN_MOSI, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
	
	gpio_set_dir(PIN_LED, GPIO_OUT);
	
	printf("SPI initialized.\n");
	blink(1, 250);
	
	return spi;
}

void reset() {
	printf("Resetting Panel\n");
	gpio_put(PIN_RESET, 1);
	sleep_ms(200);
	gpio_put(PIN_RESET, 0);
	sleep_ms(10);
	gpio_put(PIN_RESET, 1);
	sleep_ms(200);

	printf("Reset done.\n");
}
void sendCommand(uint8_t reg) {
	cs_select();
	gpio_put(PIN_DC, 0);
	spi_write_blocking(SPI_PORT, &reg, 1);
	cs_deselect();
}

void spi_send_command(pio_spi_inst_t *spi, const u_int8_t command)
{
	gpio_put(spi->cs_pin, 0);
	pio_spi_write8_blocking(spi, &command, 1);
	gpio_put(spi->cs_pin, 1);
	waitUntilBusy(spi);
}

void sendData(uint8_t reg) {
	cs_select();
	gpio_put(PIN_DC, 1);
	gpio_put(PIN_LED, 1);
	uint8_t bytes = spi_write_blocking(SPI_PORT, &reg, 1);
	gpio_put(PIN_LED, 0);
	cs_deselect();
}

void spi_send_data(pio_spi_inst_t *spi, const u_int8_t data)
{
	gpio_put(spi->cs_pin, 0);
	gpio_put(PIN_DC, 1);
	gpio_put(PIN_LED, 1);
	pio_spi_write8_blocking(spi, &data, 1);
	gpio_put(PIN_LED, 0);
	gpio_put(spi->cs_pin, 1);
}
void waitUntilBusy(pio_spi_inst_t *spi) {
	//while (gpio_get(PIN_MISO) == 1) {
	uint8_t data; 
	pio_spi_read8_blocking(spi, &data, 1);
	while (data != 0x00) {
		printf("E-paper device is busy...\n");
		sleep_ms(100);		
		printf("  ... still waiting ...\n");
		printf("E-paper device is available again\n");
		pio_spi_read8_blocking(spi, &data, 1);
	}
}

void turnOnDisplay(pio_spi_inst_t *spi) {
	spi_send_command(spi, 0x22);
	spi_send_data(spi, 0xf7);
	spi_send_command(spi, 0x20);
	/*sendCommand(0x22);
	sendData(0xf7);
	sendCommand(0x20);*/
	waitUntilBusy(spi);
}

void initDeviceRegisters(pio_spi_inst_t *spi) {

	reset();
	waitUntilBusy(spi);
	printf("Software reset\n");
	//sendCommand(0x12); // swreset
	spi_send_command(spi, 0x12); // swreset
	waitUntilBusy(spi);
	printf("Driver output control\n");
	//sendCommand(0x01); // driver output control
	spi_send_command(spi, 0x01); // driver output control
	gpio_put(PIN_DBG, 1);
	/*sendData(0xc7);	
	sendData(0x00);
	sendData(0x01);*/
	spi_send_data(spi, 0xc7);
	spi_send_data(spi, 0x00);
	spi_send_data(spi, 0x01);
	gpio_put(PIN_DBG, 0);	
	printf("Data entry mode\n");
	//sendCommand(0x11); //data entry mode	
	spi_send_command(spi, 0x11);
	spi_send_data(spi, 0x01); 
	//sendData(0x01);	
	printf("Set x address\n");
	//sendCommand(0x44); // set x address start/end position	
	spi_send_command(spi, 0x44);
	spi_send_data(spi, 0x00);
	spi_send_data(spi, 0x18);
	/*sendData(0x00);
	sendData(0x18);	*/
	printf("Set y address\n");
	spi_send_command(spi, 0x45);
	spi_send_data(spi, 0xc7); 
	spi_send_data(spi, 0x00); 
	spi_send_data(spi, 0x00); 
	spi_send_data(spi, 0x00); 
	//sendCommand(0x45); // y address start/end position	
	//sendData(0xc7);
	//sendData(0x00);		
	//sendData(0x00);	
	//sendData(0x00);		
	printf("Waveform\n");
	spi_send_command(spi, 0x3c);
	spi_send_data(spi, 0x01); 
	//sendCommand(0x3c); // border waveform	
	//sendData(0x01);

	printf("???\n");
	spi_send_command(spi, 0x18);
	spi_send_data(spi, 0x80); 
	//sendCommand(0x018); // ???	
	//sendData(0x80);	

	printf("Load temperature and waveform\n");
	spi_send_command(spi, 0x22);
	spi_send_data(spi, 0xb1); 
	//sendCommand(0x22); // load temperature and waveform setting	
	//sendData(0xb1);	
	spi_send_command(spi, 0x20);
	//sendCommand(0x20);
	
	spi_send_command(spi, 0x4e);
	spi_send_data(spi, 0x00); 
	//sendCommand(0x4e); // ram x address count = 0	
	//sendData(0x00);
	
	spi_send_command(spi, 0x4f);
	spi_send_data(spi, 0xc7); 
	spi_send_data(spi, 0x00); 
	//sendCommand(0x4f); // ram y address count = 0x199
	//sendData(0xc7);
	//sendData(0x00);			
		
	waitUntilBusy(spi);
}
void clearScreen(pio_spi_inst_t *spi) {
	printf("Clearing screen\n");
	uint16_t width, height;
	width = (DEVICE_WIDTH % 8 == 0) ? DEVICE_WIDTH / 8 : DEVICE_WIDTH / 8 + 1;
	height = DEVICE_HEIGHT;
	
	uint16_t i = 0;
	uint8_t x = 0;
	uint8_t y = 0;
	spi_send_command(spi, 0x24);
	//sendCommand(0x24);			
	for (y = 0; y < 200; y++)
	{		
		for (x = 0; x < 25; x++)
		{
			//sendData(y % 20 == 0 && x % 2 == 0 ? 0x00 : 0xff);
			spi_send_data(spi, 0xff); 
			//sendData(0xff);
		}
	}
	spi_send_command(spi, 0x26);
	//sendCommand(0x26);	
	for (y = 0; y < 200; y++)
	{		
		for (x = 0; x < 25; x++)
		{			
			//sendData(x % 2 ==0 ? 0x00 : 0x81);
			//sendData(0x00);
			spi_send_data(spi, 0x00);
		}
	}
	waitUntilBusy(spi);

	turnOnDisplay(spi);
}
void deepSleep(pio_spi_inst_t *spi) {
	startLed();
	//sendCommand(0x10);
	//sendData(0x01);
	spi_send_command(spi, 0x10);
	spi_send_data(spi, 0x01);
	waitUntilBusy(spi);
}

static const char *gpio_irq_str[] = {
	"LEVEL_LOW",
	// 0x1
	"LEVEL_HIGH",
	 // 0x2
	"EDGE_FALL",
	// 0x4
	"EDGE_RISE"  // 0x8
};

void gpio_event_string(char *buf, uint32_t events) {
	for (uint i = 0; i < 4; i++) {
		uint mask = (1 << i);
		if (events & mask) {
			// Copy this event string into the user string
			const char *event_str = gpio_irq_str[i];
			while (*event_str != '\0') {
				*buf++ = *event_str++;
			}
			events &= ~mask;

			// If more events add ", "
			if (events) {
				*buf++ = ',';
				*buf++ = ' ';
			}
		}
	}
	*buf++ = '\0';
}