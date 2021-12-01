#pragma once
#include <array>
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <pico/stdlib.h>
#include <pio_spi.h>
#include <stdio.h>
#include <vector>

enum class Colors : uint8_t
{
	white,
	black,
	red
};

struct Pins
{
  public:
	Pins(const uint8_t mosi = PICO_DEFAULT_SPI_TX_PIN,
		 const uint8_t busy = PICO_DEFAULT_SPI_RX_PIN,
		 const uint8_t clock = PICO_DEFAULT_SPI_SCK_PIN,
		 const uint8_t channel_select = PICO_DEFAULT_SPI_CSN_PIN,
		 const uint8_t reset = 8,
		 const uint8_t data_command = 9)
		: mosi(mosi), busy(busy), clock(clock), channel_select(channel_select), reset(reset), data_command(data_command)
	{
	}
	uint8_t mosi;
	uint8_t busy;
	uint8_t clock;
	uint8_t channel_select;
	uint8_t reset;
	uint8_t data_command;
};

namespace Devices
{
	struct Waveshare154V2b
	{
		static constexpr uint8_t width = 200;
		static constexpr uint8_t height = 200;
		static constexpr uint8_t amount_colors = 3;
		static constexpr const std::array<Colors, 3> available_colors = {Colors::white, Colors::black, Colors::red};
	};
} // namespace Devices

struct Command
{
	uint8_t code;
	std::initializer_list<uint8_t> data;
};

namespace EPaperDevice
{
	template <typename T>
	struct Device
	{
		using Buffer = std::array<Colors, static_cast<uint16_t>(T::width * T::height)>;

	  public:
		Device(const Pins &&pins)
			: _pins{std::move(pins)} { _initialize(); }
		void run();
		void sleep();
		void render();
		
	  private:
		Pins _pins;
		Buffer _pixel_buffer{Colors::white};
		pio_spi_inst_t _spi;
		void _initialize();
		void _reset();
		void _wait_if_busy();
		void _init_device_registers();
		void _send_command(const Command &command);
		void _send_command(const Command &&command);		
	};
} // namespace EPaperDevice
  // namespace EPaperDevice

template <typename T>
void EPaperDevice::Device<T>::_send_command(const Command &command)
{
	static constexpr auto transfer_data = [](const auto &_spi, const auto &_pins, const uint8_t dc_voltage, const uint8_t &data, const uint8_t size) -> void {
		gpio_put(_spi.cs_pin, 0);
		gpio_put(_pins.data_command, dc_voltage);
		pio_spi_write8_blocking(&_spi, data, size);
		gpio_put(_spi.cs_pin, 1);
	};

	transfer_data(_spi, _pins, 0, &command.code, 1);
	transfer_data(_spi, _pins, 0, &command.data, command.data.size());

	_wait_if_busy();
}

template <typename T>
void EPaperDevice::Device<T>::_send_command(const Command &&command)
{
	_send_command(command);
}