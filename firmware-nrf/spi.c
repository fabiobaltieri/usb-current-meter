/*
 * Copyright 2011 Fabio Baltieri <fabio.baltieri@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#include <avr/io.h>
#include <util/delay.h>

#include "board.h"

#if 0
#define USICR_BASE ( (0 << USIWM1) | (1 << USIWM0) | \
		     (0 << USICS1) | (0 << USICS0) | (1 << USITC) )
#endif
void spi_init(void)
{
	SPI_DDR  |= _BV(SPI_SCK) | _BV(SPI_MOSI) | _BV(SPI_SS);
	SPI_PORT |= _BV(SPI_SS);

#if 0
	USICR = USICR_BASE;
#endif
}

void spi_o(uint8_t data)
{
	__asm__ __volatile__ (
			"sbrc %[data], 7"        "\n\t"
			"sbi  %[port], %[mosi]"  "\n\t"
			"sbrs %[data], 7"        "\n\t"
			"cbi  %[port], %[mosi]"  "\n\t"
			"sbi  %[port], %[sck]"   "\n\t"
			"cbi  %[port], %[sck]"   "\n\t"

			"sbrc %[data], 6"        "\n\t"
			"sbi  %[port], %[mosi]"  "\n\t"
			"sbrs %[data], 6"        "\n\t"
			"cbi  %[port], %[mosi]"  "\n\t"
			"sbi  %[port], %[sck]"   "\n\t"
			"cbi  %[port], %[sck]"   "\n\t"

			"sbrc %[data], 5"        "\n\t"
			"sbi  %[port], %[mosi]"  "\n\t"
			"sbrs %[data], 5"        "\n\t"
			"cbi  %[port], %[mosi]"  "\n\t"
			"sbi  %[port], %[sck]"   "\n\t"
			"cbi  %[port], %[sck]"   "\n\t"

			"sbrc %[data], 4"        "\n\t"
			"sbi  %[port], %[mosi]"  "\n\t"
			"sbrs %[data], 4"        "\n\t"
			"cbi  %[port], %[mosi]"  "\n\t"
			"sbi  %[port], %[sck]"   "\n\t"
			"cbi  %[port], %[sck]"   "\n\t"

			"sbrc %[data], 3"        "\n\t"
			"sbi  %[port], %[mosi]"  "\n\t"
			"sbrs %[data], 3"        "\n\t"
			"cbi  %[port], %[mosi]"  "\n\t"
			"sbi  %[port], %[sck]"   "\n\t"
			"cbi  %[port], %[sck]"   "\n\t"

			"sbrc %[data], 2"        "\n\t"
			"sbi  %[port], %[mosi]"  "\n\t"
			"sbrs %[data], 2"        "\n\t"
			"cbi  %[port], %[mosi]"  "\n\t"
			"sbi  %[port], %[sck]"   "\n\t"
			"cbi  %[port], %[sck]"   "\n\t"

			"sbrc %[data], 1"        "\n\t"
			"sbi  %[port], %[mosi]"  "\n\t"
			"sbrs %[data], 1"        "\n\t"
			"cbi  %[port], %[mosi]"  "\n\t"
			"sbi  %[port], %[sck]"   "\n\t"
			"cbi  %[port], %[sck]"   "\n\t"

			"sbrc %[data], 0"        "\n\t"
			"sbi  %[port], %[mosi]"  "\n\t"
			"sbrs %[data], 0"        "\n\t"
			"cbi  %[port], %[mosi]"  "\n\t"
			"sbi  %[port], %[sck]"   "\n\t"
			"cbi  %[port], %[sck]"   "\n\t"

			: /* no output */
			: [port] "I" (_SFR_IO_ADDR(SPI_PORT)),
			  [mosi] "I" (SPI_MOSI),
			  [sck]  "I" (SPI_SCK),
			  [data] "r" (data));
}

uint8_t spi_io(uint8_t data)
{
	uint8_t ret = 0;
	uint8_t i;

	for (i = 0; i < 8; i++) {
		if (data & 0x80)
			SPI_PORT |=  _BV(SPI_MOSI);
		else
			SPI_PORT &= ~_BV(SPI_MOSI);

		SPI_PORT |= _BV(SPI_SCK);

		ret <<= 1;

		if (SPI_PIN & _BV(SPI_MISO))
			ret |= 0x01;

		data <<= 1;

		SPI_PORT &= ~_BV(SPI_SCK);
	}

	return ret;
#if 0
	USIDR = data;

	USICR = USICR_BASE;                 /* MSB */
	USICR = USICR_BASE | (1 << USICLK);
	USICR = USICR_BASE;                 /* 7 */
	USICR = USICR_BASE | (1 << USICLK);
	USICR = USICR_BASE;                 /* 6 */
	USICR = USICR_BASE | (1 << USICLK);
	USICR = USICR_BASE;                 /* 5 */
	USICR = USICR_BASE | (1 << USICLK);
	USICR = USICR_BASE;                 /* 4 */
	USICR = USICR_BASE | (1 << USICLK);
	USICR = USICR_BASE;                 /* 3 */
	USICR = USICR_BASE | (1 << USICLK);
	USICR = USICR_BASE;                 /* 2 */
	USICR = USICR_BASE | (1 << USICLK);
	USICR = USICR_BASE;                 /* 1 */
	USICR = USICR_BASE | (1 << USICLK);
	USICR = USICR_BASE;                 /* LSB */
	USICR = USICR_BASE | (1 << USICLK);

	ret = USIDR;

	return ret;
#endif
}
