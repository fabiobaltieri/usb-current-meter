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

/* SPI */
#define SPI_DDR  DDRA
#define SPI_PORT PORTA
#define SPI_PIN  PINA
#define SPI_SS   PA2
#define SPI_SCK  PA4
#define SPI_MISO PA5
#define SPI_MOSI PA6

/* nRF24L01+ SPI */
#define NRF_DDR  DDRA
#define NRF_PORT PORTA
#define NRF_CS   SPI_SS
#define NRF_CE   PA3

#define nrf_cs_h() (NRF_PORT |=  _BV(NRF_CS))
#define nrf_cs_l() (NRF_PORT &= ~_BV(NRF_CS))
#define nrf_ce_h() (NRF_PORT |=  _BV(NRF_CE))
#define nrf_ce_l() (NRF_PORT &= ~_BV(NRF_CE))

/* Analog */
#define ANALOG_PORT    PORTB
#define ANALOG_DDR     DDRB
#define ANALOG         PB1

#define analog_on()     ANALOG_PORT |=  _BV(ANALOG);
#define analog_off()    ANALOG_PORT &= ~_BV(ANALOG);

#define analog_init() do {			\
		ANALOG_DDR |= _BV(ANALOG);	\
	} while (0);

/* ADC */
#define ADC_COIL       (_BV(REFS1) | 0x01)
#define ADC_OFFSET_20X (_BV(REFS1) | 0x23)
#define ADC_COIL_1X    (_BV(REFS1) | 0x28)
#define ADC_COIL_20X   (_BV(REFS1) | 0x29)
#define ADC_VCC        (0x21)

#define AMP_TH 45 /* ~ 1024 / 20 */
#define ADC_VREF_mV (1100 / 4)
#define ADC_VREF_BITS (1024 / 4)

/* 4000W version */
#define CAL_POWER	4000
#define CAL_VOLTAGE	1121l
/* 6500W version */
//#define CAL_POWER	6500
//#define CAL_VOLTAGE	1060l
