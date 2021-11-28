#pragma once
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <pio_spi.h>
#include <array>
#include <concepts>

enum class colors : uint8_t
{
	white,
	black,
	red
};

namespace Devices
{
	struct WaveshareE154V2b		
	{
		static constexpr uint8_t width = 200;
		static constexpr uint8_t height = 200;		
		static constexpr std::array<colors, 3> available_colors = { colors::white, colors::black, colors::red };		
	};
}

namespace EPaperDevice
{				
	template <typename T>
	struct Device		
	{					
		using Buffer = std::array<colors, static_cast<u_int16_t>(T::width * T::height)>;
		using AvailableColors = std::array<colors, T::amount_colors>;
	public:
		Device(
			const uint8_t mosi = PICO_DEFAULT_SPI_TX_PIN, 
			const uint8_t busy = PICO_DEFAULT_SPI_RX_PIN, 
			const uint8_t clock = PICO_DEFAULT_SPI_SCK_PIN,
			const uint8_t channel_select = PICO_DEFAULT_SPI_CSN_PIN,
			const uint8_t reset = 8,
			const uint8_t data_command = 9
			) 
			: 
			pins{mosi, busy, clock, reset, data_command, channel_select}
	{ 
		initialize(); 
	};					
		struct Pins
		{			
			uint8_t mosi;
			uint8_t busy;			
			uint8_t clock;
			uint8_t channel_select;
			uint8_t reset;
			uint8_t data_command;			
		} pins;
	private:
		Buffer pixel_buffer;		
		pio_spi_inst_t spi;								
		void initialize();		
	};
}