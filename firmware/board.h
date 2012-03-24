/* LEDs */

#define LED_A_PORT    PORTB
#define LED_A_DDR     DDRB
#define LED_A         PB0

#define led_a_on()     LED_A_PORT |=  _BV(LED_A);
#define led_a_off()    LED_A_PORT &= ~_BV(LED_A);
#define led_a_toggle() LED_A_PORT ^=  _BV(LED_A);

#define led_init() do {				\
		LED_A_DDR |= _BV(LED_A);	\
	} while (0);

/* ADC */

#define ADC_COIL       (_BV(REFS2) | _BV(REFS1) | 0x02)
#define ADC_OFFSET_1X  (_BV(REFS2) | _BV(REFS1) | 0x04)
#define ADC_OFFSET_20X (_BV(REFS2) | _BV(REFS1) | 0x05)
#define ADC_COIL_1X    (_BV(REFS2) | _BV(REFS1) | 0x06)
#define ADC_COIL_20X   (_BV(REFS2) | _BV(REFS1) | 0x07)

#define AMP_TH 45 /* ~ 1024 / 20 */
#define SATURATION_TH 1020
#define ADC_VREF_mV 2560
#define V_TO_W(x) (x * 4000 / 2130) /* from schematics */
