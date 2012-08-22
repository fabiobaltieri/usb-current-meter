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

#include "spi.h"
#include "nrf24l01p.h"

static uint8_t buf[16];

static void hello(void)
{
	uint8_t i;

	for (i = 0; i < 9; i++) {
		led_a_toggle();
		_delay_ms(50);
	}
}

/* RF_IRQ */
ISR(INT0_vect)
{
	nrf_irq();

	led_a_off();
}

ISR(WDT_vect)
{
	led_a_on();
	nrf_standby();
	led_a_toggle();
	nrf_tx(buf, sizeof(buf));
	led_a_toggle();

	buf[15]++;
}

int main(void)
{
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
