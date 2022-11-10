#include <EPaperDevice.hpp>

namespace Codes
{
	static constexpr uint8_t sw_reset = 0x12;
	static constexpr uint8_t driver_output_control = 0x01;
	static constexpr uint8_t data_entry_mode = 0x11;
	static constexpr uint8_t set_x_address = 0x44;
	static constexpr uint8_t set_y_address = 0x45;
	static constexpr uint8_t ram_x_address_count = 0x4e;
	static constexpr uint8_t ram_y_address_count = 0x4f;
	static constexpr uint8_t set_border_wave = 0x3c;
	static constexpr uint8_t unknown = 0x18;
	static constexpr uint8_t unknown2 = 0x20;
} // namespace Codes

namespace Commands
{
	static const Command software_reset = {Codes::sw_reset};
	static const Command driver_output_control = {Codes::driver_output_control, {0xc7, 0x00, 0x01}};
	static const Command data_entry_mode = {Codes::data_entry_mode, {0x01}};
	static const Command set_x_addressing = {Codes::set_x_address, {0x00, 0x18}};
	static const Command set_y_addressing = {Codes::set_y_address, {0xc7, 0x00, 0x00, 0x00}};
	static const Command set_border_waveform = {Codes::set_border_wave, {0x01}};
	static const Command unknown_command = {Codes::unknown, {0x80}}; // TODO find out what this does
	static const Command load_temperature_and_waveform = {Codes::unknown2};
	static const Command set_ram_x_address_count = {Codes::ram_x_address_count, {0x00}};
	static const Command set_ram_y_address_count = {Codes::ram_y_address_count, {0xc7, 0x00}};
} // namespace Commands

template <>
void EPaperDevice::Device<Devices::Waveshare154V2b>::_initialize()
{
	printf("_initialize()\n");
	stdio_init_all();
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

template <>
void EPaperDevice::Device<Devices::Waveshare154V2b>::_reset()
{
	printf("Performing reset.\n");
	gpio_put(_pins.reset, 1);
	sleep_ms(200);
	gpio_put(_pins.reset, 0);
	sleep_ms(10);
	gpio_put(_pins.reset, 1);
	sleep_ms(200);
}

template <>
void EPaperDevice::Device<Devices::Waveshare154V2b>::_wait_if_busy()
{
	printf("... waiting ...\n");
	uint8_t data;
	pio_spi_read8_blocking(&_spi, &data, 1);
	while (data != 0x00)
	{
		sleep_ms(1);
		pio_spi_read8_blocking(&_spi, &data, 1);
	}
}

template <>
void EPaperDevice::Device<Devices::Waveshare154V2b>::_init_device_registers()
{
	using namespace Commands;
	printf("Initializing device registers.\n");
	_reset();
	_wait_if_busy();
	printf("Software reset.\n");
	_send_command(software_reset);
	printf("Driver output control..\n");
	_send_command(driver_output_control);
	printf("Data entry mode.\n");
	_send_command(data_entry_mode);
	printf("X Addressing.\n");
	_send_command(set_x_addressing);
	printf("Y Adressing.\n");
	_send_command(set_y_addressing);
	printf("Border waveform.\n");
	_send_command(set_border_waveform);
	printf(". ??? .\n");
	_send_command(unknown_command);
	printf("Load temperature and waveform.\n");
	_send_command(load_temperature_and_waveform);
	printf("RAM X address count.\n");
	_send_command(set_ram_x_address_count);
	printf("RAM Y address count.\n");
	_send_command(set_ram_y_address_count);
}

template <>
void EPaperDevice::Device<Devices::Waveshare154V2b>::run()
{
	_init_device_registers();
}