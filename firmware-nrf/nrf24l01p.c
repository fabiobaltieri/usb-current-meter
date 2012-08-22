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
#include <avr/interrupt.h>
#include <util/delay.h>

#include "board.h"

#include "spi.h"
#include "nrf24l01p.h"

#define ADDR_AW AW_3_BYTES
static uint8_t broadcast[] = { 0x10, 0xab, 0x0f };

#define CHANNEL 24
#define PAYLOADSZ 16

#define NRF_CONFIG (EN_CRC | CRCO)

#define STATE_TX_ON 0x01
#define STATE_RX_ON 0x02
static volatile uint8_t state;

/*
 * Commands
 */

uint8_t nrf_read_reg(uint8_t addr)
{
	uint8_t ret;

	nrf_cs_l();

	spi_io(CMD_R_REGISTER | addr);
	ret = spi_io(CMD_NOP);

	nrf_cs_h();

	return ret;
}

void nrf_write_reg(uint8_t addr, uint8_t data)
{
	nrf_cs_l();

	spi_o(CMD_W_REGISTER | addr);
	spi_o(data);

	nrf_cs_h();
}

static void nrf_read_addr(uint8_t addr, uint8_t *data, uint8_t size)
{
	uint8_t i;

	nrf_cs_l();

	spi_io(CMD_R_REGISTER | addr);
	for (i = 0; i < size; i++)
		data[i] = spi_io(CMD_NOP);

	nrf_cs_h();
}

static void nrf_write_addr(uint8_t addr, uint8_t *data, uint8_t size)
{
	uint8_t i;

	nrf_cs_l();

	spi_io(CMD_W_REGISTER | addr);
	for (i = 0; i < size; i++)
		spi_io(data[i]);

	nrf_cs_h();
}

static void nrf_read_payload(uint8_t *buf, uint8_t size)
{
	uint8_t i;

	nrf_cs_l();

	spi_io(CMD_R_RX_PAYLOAD);
	for (i = 0; i < size; i++)
		buf[i] = spi_io(CMD_NOP);

	nrf_cs_h();
}

static void nrf_write_payload(uint8_t *buf, uint8_t size)
{
	uint8_t i;

	nrf_cs_l();

	spi_o(CMD_W_TX_PAYLOAD);
	for (i = 0; i < size; i++)
		spi_o(buf[i]);

	nrf_cs_h();
}

static void nrf_flush_tx(void)
{
	nrf_cs_l();

	spi_io(CMD_FLUSH_TX);

	nrf_cs_h();
}

static void nrf_flush_rx(void)
{
	nrf_cs_l();

	spi_io(CMD_FLUSH_RX);

	nrf_cs_h();
}

static uint8_t nrf_get_status(void)
{
	uint8_t ret;

	nrf_cs_l();

	ret = spi_io(CMD_NOP);

	nrf_cs_h();

	return ret;
}

/*
 * Normal functions
 */

void nrf_standby(void)
{
	nrf_ce_l();

	nrf_write_reg(CONFIG, NRF_CONFIG | PWR_UP);

	_delay_ms(4.5);
}

void nrf_rx_mode(void)
{
	nrf_write_reg(CONFIG, NRF_CONFIG | PWR_UP | PRIM_RX);

	nrf_ce_h();
}

void nrf_powerdown(void)
{
	nrf_ce_l();

	nrf_write_reg(CONFIG, 0x00);
}

uint8_t nrf_has_data(void)
{
	if (nrf_get_status() & RX_DR) {
		nrf_write_reg(STATUS, RX_DR);
		return 1;
	} else {
		return 0;
	}
}

uint8_t nrf_rx(uint8_t *data, uint8_t size)
{
	uint8_t pipe;
	uint8_t rx_size;

	pipe = (nrf_get_status() & RX_P_NO_MASK) >> RX_P_NO_SHIFT;

	if (pipe > 5)
		return 0;

	rx_size = nrf_read_reg(RX_PW_P0 + pipe);

	if (rx_size == size) {
		nrf_read_payload(data, rx_size);
		return rx_size;
	} else {
		nrf_flush_rx();
		return 0;
	}
}

void nrf_tx(uint8_t *data, uint8_t size)
{
	nrf_write_reg(CONFIG, NRF_CONFIG | PWR_UP | PRIM_TX);

	nrf_write_payload(data, size);

	/* pulse ce to start transmit */
	nrf_ce_h();
}

void nrf_rx_enable(void)
{
	if (!(state & (STATE_TX_ON | STATE_RX_ON))) {
		nrf_standby();
		nrf_rx_mode();
	}
	state |= STATE_RX_ON;
}

void nrf_rx_disable(void)
{
	if (!(state & STATE_TX_ON) && (state & STATE_RX_ON)) {
		nrf_powerdown();
	}
	state &= ~STATE_RX_ON;
}

void nrf_irq(void)
{
	uint8_t status;

	status = nrf_get_status();

	/* RX data ready */
	if (status & RX_DR) {
		nrf_flush_rx();
	}

	/* TX data sent */
	if (status & TX_DS) {
		nrf_standby();
		nrf_powerdown();
	}

	/* TX max retry */
	if (status & MAX_RT) {
	}

	nrf_write_reg(STATUS, status);
}

/*
 * Initialization
 */

void nrf_init(void)
{
	state = 0;

	/* set TX addr and addr size */
	nrf_write_reg(SETUP_AW, ADDR_AW);
	nrf_write_addr(TX_ADDR, broadcast, sizeof(broadcast));
	nrf_write_reg(SETUP_RETR, (0x0 << ARD_SHIFT) | (0x0 << ARC_SHIFT));

	/* set RX pipe 0 */
	nrf_write_addr(RX_ADDR_P0, broadcast, sizeof(broadcast));
	nrf_write_reg(EN_RXADDR, ERX_P0);
	nrf_write_reg(EN_AA, 0);
	nrf_write_reg(DYNPD, 0);
	nrf_write_reg(RX_PW_P0, PAYLOADSZ); /* pipe 0 size */

	/* set channel */
	nrf_write_reg(RF_CH, CHANNEL);
	nrf_write_reg(RF_SETUP, RF_DR_1M | RF_PWR_3);

	nrf_flush_tx();
	nrf_flush_rx();

	/* clear interrupts */
	nrf_write_reg(STATUS, RX_DR | TX_DS | MAX_RT);
}
