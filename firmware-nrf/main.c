/*
 * Copyright 2011 Fabio Baltieri <fabio.baltieri@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "board.h"

#include "id.h"
#include "spi.h"
#include "adc.h"
#include "nrf24l01p.h"

#define div_round(a, b) (((a) + ((b)/2)) / (b))

static uint8_t buf[16];

static void hello(void)
{
	uint8_t i;

	for (i = 0; i < 9; i++) {
		led_a_toggle();
		_delay_ms(50);
	}
}

static uint16_t get_power(void)
{
	uint32_t value;
	uint16_t offset;
	uint8_t gain;

	offset = 0;
	value  = adc_get(ADC_COIL);

	if (value < AMP_TH) {
		offset = adc_get(ADC_OFFSET_20X);
		value  = adc_get(ADC_COIL_20X);
		gain = 20;
	} else {
		gain = 1;
	}

	if (value < offset)
		value = 0;
	else
		value = div_round((value - offset) * ADC_VREF_mV * CAL_POWER,
				ADC_VREF_BITS * CAL_VOLTAGE * (uint32_t)gain);

	return value;
}

/* RF_IRQ */
ISR(INT0_vect)
{
	nrf_irq();

	led_a_off();
}

ISR(WDT_vect)
{
	uint16_t val;

	adc_init();
	val = get_power();
	buf[9] = '0' + (val) % 10;
	buf[8] = '0' + (val / 10) % 10;
	buf[7] = '0' + (val / 100) % 10;
	buf[6] = '0' + (val / 1000) % 10;

	val = adc_get(ADC_VCC);
	val = div_round(1100l * 1024, val);
	buf[14] = '0' + (val) % 10;
	buf[13] = '0' + (val / 10) % 10;
	buf[12] = '0' + (val / 100) % 10;
	buf[11] = '0' + (val / 1000) % 10;
	adc_stop();

	led_a_on();
	nrf_standby();
	led_a_toggle();
	nrf_tx(buf, sizeof(buf));
	led_a_toggle();

	buf[15]++;
}

int main(void)
{
	read_board_id();

	led_init();
	analog_init();
	spi_init();

	led_a_on();
	analog_on();

	/* IO init */
	NRF_DDR |= _BV(NRF_CS) | _BV(NRF_CE);
	nrf_cs_h();
	nrf_ce_l();

	nrf_init();

	/* WDT as periodic interrupt */
	WDTCSR |= (1 << WDE) | (1 << WDCE);
	WDTCSR = (0 << WDCE) | (1 << WDIF) | (1 << WDIE) |
		 (1 << WDP2) | (0 << WDP1) | (1 << WDP0);

	/* INT0 init - low level */
	GIMSK |= (1 << INT0);
	MCUCR |= (0 << ISC01) | (0 << ISC00);

	hello();
	sei();

	buf[0] = 'p';
	buf[1] = 'o';
	buf[2] = 'w';
	buf[3] = 'e';
	buf[4] = 'r';

	while (1) {
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_mode();
	}
}
