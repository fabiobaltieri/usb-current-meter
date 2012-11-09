#include "board.h"

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#include "usbdrv.h"

#include "requests.h"
#include "adc.h"

#define NR_CHANNELS 8

static uint8_t current_channel;

static uint16_t buffer_values[NR_CHANNELS];
static uint32_t instant_values[NR_CHANNELS];
static uint8_t gain_mask;

static struct host_frame {
	uint16_t host_values[NR_CHANNELS];
	uint16_t sample_count;
	uint8_t gain_mask;
} hf;

static uint16_t buffer_counter;

static const uint8_t map[NR_CHANNELS] = {
	5, 4, 3, 2, 1, 0, 7, 6
};

static void reset_cpu(void)
{
	wdt_disable();

	wdt_enable(WDTO_15MS);

	for (;;);
}

#define is_high_gain(mask, ch) (mask & (1 << ch))

#define div_round(a, b) (((a) + ((b)/2)) / (b))
static uint16_t get_power(void)
{
	uint32_t value;

	value  = adc_get_value();

	if (is_high_gain(gain_mask, current_channel))
		value = div_round(value * ADC_VREF_mV * CAL_POWER,
				ADC_VREF_BITS * CAL_VOLTAGE_H);
	else
		value = div_round(value * ADC_VREF_mV * CAL_POWER,
				ADC_VREF_BITS * CAL_VOLTAGE_L);

	return value;
}

static void set_high_gain(uint8_t channel)
{
	switch (channel) {
		case 0:
			set_high_gain_ch0();
			break;
		case 1:
			set_high_gain_ch1();
			break;
		case 2:
			set_high_gain_ch2();
			break;
		case 3:
			set_high_gain_ch3();
			break;
		case 4:
			set_high_gain_ch4();
			break;
		case 5:
			set_high_gain_ch5();
			break;
		case 6:
			set_high_gain_ch6();
			break;
		case 7:
			set_high_gain_ch7();
			break;
	}

	gain_mask |= (1 << channel);
}

static void set_low_gain(uint8_t channel)
{
	switch (channel) {
		case 0:
			set_low_gain_ch0();
			break;
		case 1:
			set_low_gain_ch1();
			break;
		case 2:
			set_low_gain_ch2();
			break;
		case 3:
			set_low_gain_ch3();
			break;
		case 4:
			set_low_gain_ch4();
			break;
		case 5:
			set_low_gain_ch5();
			break;
		case 6:
			set_low_gain_ch6();
			break;
		case 7:
			set_low_gain_ch7();
			break;
	}

	gain_mask &= ~(1 << channel);
}

static void change_channel_gain(uint8_t channel)
{
	if (!is_high_gain(gain_mask, channel) &&
			buffer_values[channel] < AMP_TH_L) {
		set_high_gain(channel);
	} else if (is_high_gain(gain_mask, channel) &&
			buffer_values[channel] > AMP_TH_H) {
		set_low_gain(channel);
	}

	adc_set_channel(map[channel]);
}

static void buffer_poll(void)
{
	uint8_t i;

	if (!(TIFR0 & _BV(OCIE0A)))
		return;

	buffer_values[current_channel] = get_power();

	current_channel = (current_channel + 1) % NR_CHANNELS;
	change_channel_gain(current_channel);

	if (current_channel == 0) {
		buffer_counter++;

		/* overflow, start from 1 */
		if (buffer_counter == UINT16_MAX) {
			buffer_counter = 1;
			memset(instant_values, 0,
					sizeof(instant_values));
		}

		for (i = 0; i < NR_CHANNELS; i++)
			instant_values[i] += buffer_values[i];
	}

	TIFR0 |= _BV(OCIE0A);
}

static void buffer_get(void)
{
	uint8_t i;

	/* return last values if no new dataset */
	if (buffer_counter == 0)
		return;

	for (i = 0; i < NR_CHANNELS; i++)
		hf.host_values[i] = instant_values[i] / buffer_counter;

	hf.sample_count = buffer_counter;
	hf.gain_mask = gain_mask;

	buffer_counter = 0;
	memset(instant_values, 0, sizeof(instant_values));
}

static void timer_setup(void)
{
	OCR0A = F_CPU / 1024 / 100; /* about 100 Hz */

	TIFR0 |= _BV(OCIE0A);

	TCCR0A = ( (1 << WGM01) | (0 << WGM00) );
	TCCR0B = ( (0 << WGM02) |
			(1 << CS02) | (0 << CS01) | (1 << CS00) );
}

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
	struct usbRequest *rq = (void *)data;

	switch (rq->bRequest) {
	case CUSTOM_RQ_GET_VALUES:
		led_a_toggle();
		buffer_get();
		usbMsgPtr = (uint8_t *)&hf;
		return sizeof(hf);
	case CUSTOM_RQ_RESET:
		reset_cpu();
		return 0;
	}

	return 0;
}

void hello(void)
{
	uint8_t i;
	for (i = 0; i < 9; i++) {
		led_a_toggle();
		_delay_ms(33);
	}
}

int __attribute__((noreturn)) main(void)
{
	uint8_t i;

	led_init();
	led_a_off();

	adc_init();
	change_channel_gain(0);
	DIDR0 |= _BV(ADC5D) | _BV(ADC4D) |
		 _BV(ADC3D) | _BV(ADC2D) |
		 _BV(ADC1D) | _BV(ADC0D);

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

	/* clear data */
	current_channel = 0;
	gain_mask = 0;
	memset(buffer_values, 0, sizeof(buffer_values));
	memset(instant_values, 0, sizeof(instant_values));
	memset(&hf, 0, sizeof(hf));

	timer_setup();

	sei();
	for (;;) {
		wdt_reset();
		usbPoll();
		buffer_poll();
	}
}
