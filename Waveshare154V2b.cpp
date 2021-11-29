#include <EPaperDevice.h>

namespace Commands
{
	static const Command software_reset{0x12};
	static const Command driver_output = {0x01, {0xc7, 0x00, 0x01}};
}


template<> void EPaperDevice::Device<Devices::Waveshare154V2b>::_initialize()
{	
	_spi = {
		.pio = pio0,
		.sm = 0,
		.cs_pin = _pins.channel_select};	

	uint offset = pio_add_program(_spi.pio, &spi_cpha0_program);
	
	pio_spi_init(_spi.pio,
				 _spi.sm,
				 offset,
				 8,		 // 8 bits per SPI frame
				 31.25f, // 1 MHz @ 125 clk_sys
				 false,  // CPHA = 0
				 false,  // CPOL = 0
				 _pins.clock,
				 _pins.mosi,
				 _pins.busy);
	
	gpio_init(_pins.channel_select);
	gpio_set_dir(_pins.channel_select, GPIO_OUT);
	gpio_put(_pins.channel_select, 0);

	gpio_init(_pins.reset);
	gpio_set_dir(_pins.reset, GPIO_OUT);
	gpio_put(_pins.reset, 1);

	gpio_init(_pins.data_command);
	gpio_set_dir(_pins.data_command, GPIO_OUT);
	gpio_put(_pins.data_command, 0);	
}