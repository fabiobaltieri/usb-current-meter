/*
 * Copyright 2011 Fabio Baltieri (fabio.baltieri@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#include "board.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "adc.h"

void adc_init(void)
{
	ADCSRA = ( (1 << ADEN)  | /* enable           */
		   (0 << ADSC)  | /* start conversion */
		   (0 << ADATE) | /* free running     */
		   (1 << ADIF)  | /* clear interrupts */
		   (0 << ADIE)  | /* interrupt enable */
		   (0 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) );

	/* 1Mhz / 8 = 125kHz */
}

void adc_stop(void)
{
	ADCSRA = 0x00; /* ADC disable */
}

uint16_t adc_get (uint8_t cfg)
{
	ADMUX = cfg;

	/* wait signals to stabilize */
	_delay_us(70);

	ADCSRA |= _BV(ADSC);

	loop_until_bit_is_clear(ADCSRA, ADSC);

	return ADCW;
}
