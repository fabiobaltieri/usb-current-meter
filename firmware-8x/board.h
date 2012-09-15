/* LEDs */

#define LED_A_PORT    PORTB
#define LED_A_DDR     DDRB
#define LED_A         PB3

#define led_a_on()     LED_A_PORT |=  _BV(LED_A);
#define led_a_off()    LED_A_PORT &= ~_BV(LED_A);
#define led_a_toggle() LED_A_PORT ^=  _BV(LED_A);

#define led_init() do {				\
		LED_A_DDR |= _BV(LED_A);	\
	} while (0);

/* ADC */
#define AMP_TH_H 550
#define AMP_TH_L 500
#define ADC_VREF_mV (1100 / 4)
#define ADC_VREF_BITS (1024 / 4)

/* 6500W version */
#define CAL_POWER	6500
#define CAL_VOLTAGE_L	1026l
#define CAL_VOLTAGE_H	12074l

/* Gains */
#define set_high_gain_ch0()	(DDRD |=  _BV(PD5))
#define set_low_gain_ch0()	(DDRD &= ~_BV(PD5))

#define set_high_gain_ch1()	(DDRD |=  _BV(PD6))
#define set_low_gain_ch1()	(DDRD &= ~_BV(PD6))

#define set_high_gain_ch2()	(DDRD |=  _BV(PD7))
#define set_low_gain_ch2()	(DDRD &= ~_BV(PD7))

#define set_high_gain_ch3()	(DDRB |=  _BV(PB0))
#define set_low_gain_ch3()	(DDRB &= ~_BV(PB0))

#define set_high_gain_ch4()	(DDRB |=  _BV(PB5))
#define set_low_gain_ch4()	(DDRB &= ~_BV(PB5))

#define set_high_gain_ch5()	(DDRB |=  _BV(PB4))
#define set_low_gain_ch5()	(DDRB &= ~_BV(PB4))

#define set_high_gain_ch6()	(DDRB |=  _BV(PB2))
#define set_low_gain_ch6()	(DDRB &= ~_BV(PB2))

#define set_high_gain_ch7()	(DDRB |=  _BV(PB1))
#define set_low_gain_ch7()	(DDRB &= ~_BV(PB1))
