#pragma once
#include <array>
#include <concepts>
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <pico/stdlib.h>
#include <pio_spi.h>
#include <stdio.h>
#include <vector>
#include <memory>

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
  public:
	uint8_t code;
	std::initializer_list<uint8_t> data;
};

namespace EPaperDevice
{
	template <typename T>
	struct Device
	{
		using Buffer = std::array<Colors, static_cast<u_int16_t>(T::width *T::height)>;

	  public:
		Device(const Pins &&pins)
			: _pins{std::move(pins)} {}
		void run();
		void sleep();
		void render();

	  private:
		Pins _pins;
		Buffer _pixel_buffer;
		pio_spi_inst_t _spi;
		void _initialize();
		void _reset();
		void _wait_if_busy();
		void _send_command(const Command &command);
		void _send_command(const Command &&command);
	};	
} // namespace EPaperDevice
  // namespace EPaperDevice

template<typename T> void EPaperDevice::Device<T>::_send_command(const Command &command)
{
	gpio_put(_spi.cs_pin, 0);
	gpio_put(_pins.data_command, 0);
	pio_spi_write8_blocking(&_spi, &command.code, 1);
	gpio_put(_spi.cs_pin, 1);
	_wait_if_busy();
}
template <typename T>
void EPaperDevice::Device<T>::_send_command(const Command &&command)
{
	_send_command(command);
}