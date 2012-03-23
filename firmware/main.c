#include "board.h"

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv.h"

#include "requests.h"
#include "adc.h"

uint32_t value;

static void reset_cpu(void)
{
	wdt_disable();

	wdt_enable(WDTO_15MS);

	for (;;);
}

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
	struct usbRequest *rq = (void *)data;
	uint16_t offset;
	uint8_t gain;

	if (rq->bRequest == CUSTOM_RQ_RESET)
		reset_cpu();

	if (rq->bRequest != CUSTOM_RQ_GET_VALUE)
		return 0;

	led_a_on();

	adc_init();

	offset = 0;
	value  = adc_get(ADC_COIL);

	if (value < AMP_TH) {
		offset = adc_get(ADC_OFFSET_20X);
		value  = adc_get(ADC_COIL_20X);
		gain = 20;
	} else {
		gain = 1;
	}

	adc_stop();

	if (value > SATURATION_TH)
		value = -1;
	else if (value < offset)
		value = 0;
	else
		value = 2560 * (value - offset) / 1024 / gain;

	led_a_off();

	usbMsgPtr = (uint8_t *)&value;
	return 2;
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

	led_init();
	led_a_off();

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
	}
}
