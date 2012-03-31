#include "board.h"

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv.h"

#include "requests.h"
#include "adc.h"

static uint16_t return_value;

static enum mode {
	M_NORMAL = 0,
	M_AUTO   = 1,
} mode;

static uint32_t buffer_value;
static uint16_t buffer_counter;

static void reset_cpu(void)
{
	wdt_disable();

	wdt_enable(WDTO_15MS);

	for (;;);
}

static uint16_t get_power(void)
{
	uint32_t value;
	uint16_t offset;
	uint8_t gain;

	led_a_on();

	offset = 0;
	value  = adc_get(ADC_COIL);

	if (value < AMP_TH) {
		offset = adc_get(ADC_OFFSET_20X);
		value  = adc_get(ADC_COIL_20X);
		gain = 20;
	} else {
		gain = 1;
	}

	if (value > SATURATION_TH)
		value = -1;
	else if (value < offset)
		value = 0;
	else
		value = V_TO_W(ADC_VREF_mV * (value - offset) / 1024 / gain);

	led_a_off();

	return value;
}

static void buffer_poll(void)
{
	if (mode == M_AUTO && TIFR & _BV(OCIE0A)) {
		if (buffer_counter < UINT16_MAX) {
			buffer_value += get_power();
			buffer_counter++;
		} else {
			buffer_value = get_power();
			buffer_counter = 1;
		}

		TIFR |= _BV(OCIE0A);
	}
}

static uint16_t get_buffer(void)
{
	uint16_t ret;

	if (buffer_counter == 0)
		return -1;

	ret = buffer_value / buffer_counter;

	buffer_value = 0;
	buffer_counter = 0;

	return ret;
}

static void change_mode(uint8_t automatic)
{
	if (automatic) {
		mode = M_AUTO;

		/* start with one valid value */
		buffer_value = get_power();
		buffer_counter = 1;

		/* start timer */

		OCR0A = 251; /* about 64 Hz at 16.5 / 1024 MHz */

		TIFR |= _BV(OCIE0A);

		TCCR0A = ( (1 << WGM01) | (0 << WGM00) );
		TCCR0B = ( (0 << WGM02) |
			   (1 << CS02) | (0 << CS01) | (1 << CS00) );
	} else {
		/* stop timer */

		TCCR0B = 0x00;
		TCCR0A = 0x00;

		mode = M_NORMAL;
	}
}

static uint16_t get_raw(void)
{
	uint16_t value;

	value  = adc_get(ADC_COIL);

	return value;
}

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
	struct usbRequest *rq = (void *)data;

	switch (rq->bRequest) {
	case CUSTOM_RQ_GET_VALUE:
		if (mode == M_NORMAL)
			return_value = get_power();
		else
			return_value = get_buffer();
		usbMsgPtr = (uint8_t *)&return_value;
		return sizeof(return_value);
	case CUSTOM_RQ_GET_RAW:
		return_value = get_raw();
		usbMsgPtr = (uint8_t *)&return_value;
		return sizeof(return_value);
	case CUSTOM_RQ_SET_MODE:
		change_mode(rq->wValue.bytes[0]);
		return 0;
	case CUSTOM_RQ_RESET:
		reset_cpu();
		return 0;
	}

	return 0;
}

void hello(void)
{
	uint8_t i;
	for (i = 0; i < 8; i++) {
		led_a_toggle();
		_delay_ms(33);
	}
}

int __attribute__((noreturn)) main(void)
{
	uint8_t i;

	mode = M_NORMAL;

	led_init();
	led_a_off();

	adc_init();
	DIDR0 |= _BV(ADC2D) | _BV(ADC3D);

	wdt_enable(WDTO_1S);

	hello();

	usbInit();
	usbDeviceDisconnect();

	i = 0;
	while (--i) {
		wdt_reset();
		_delay_ms(1);
	}

	usbDeviceConnect();

	sei();
	for (;;) {
		wdt_reset();
		usbPoll();
		buffer_poll();
	}
}
