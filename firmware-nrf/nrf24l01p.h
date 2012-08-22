/*
 * Copyright 2011 Fabio Baltieri <fabio.baltieri@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

/* SPI interface commands */
#define CMD_R_REGISTER         0x00
#define CMD_W_REGISTER         0x20
#define CMD_R_RX_PAYLOAD       0x61
#define CMD_W_TX_PAYLOAD       0xa0
#define CMD_FLUSH_TX           0xe1
#define CMD_FLUSH_RX           0xe2
#define CMD_REUSE_TX_PL        0xe3
#define CMD_R_RX_PL_WID        0x60
#define CMD_W_ACK_PAYLOAD      0xa8
#define CMD_W_TX_PAYLOAD_NOACK 0xb0
#define CMD_NOP                0xff

/* nRF24L01+ registers */
#define CONFIG          0x00
#define     MASK_RX_DR      0x40
#define     MASK_TX_DS      0x20
#define     MASK_MAX_RT     0x10
#define     EN_CRC          0x08
#define     CRCO            0x04
#define     PWR_UP          0x02
#define     PRIM_RX         0x01
#define     PRIM_TX         0x00
#define EN_AA           0x01
#define     ENAA_P5         0x20
#define     ENAA_P4         0x10
#define     ENAA_P3         0x08
#define     ENAA_P2         0x04
#define     ENAA_P1         0x02
#define     ENAA_P0         0x01
#define EN_RXADDR       0x02
#define     ERX_P5          0x20
#define     ERX_P4          0x10
#define     ERX_P3          0x08
#define     ERX_P2          0x04
#define     ERX_P1          0x02
#define     ERX_P0          0x01
#define SETUP_AW        0x03
#define     AW_3_BYTES      0x01
#define     AW_4_BYTES      0x02
#define     AW_5_BYTES      0x03
#define SETUP_RETR      0x04
#define     ARD_MASK        0xf0
#define     ARD_SHIFT       4
#define     ARC_MASK        0x0f
#define     ARC_SHIFT       0
#define RF_CH           0x05
#define     RF_CH_MASK      0x7f
#define RF_SETUP        0x06
#define     CONT_WAVE       0x80
#define     RF_DR_LOW       0x20
#define     RF_DR_HIGH      0x08
#define     RF_DR_250K      0x20
#define     RF_DR_1M        0x00
#define     RF_DR_2M        0x08
#define     RF_PWR_0        0x00 /* -18 dBm */
#define     RF_PWR_1        0x02 /* -12 dBm */
#define     RF_PWR_2        0x04 /*  -6 dBm */
#define     RF_PWR_3        0x06 /*   0 dBm */
#define STATUS          0x07
#define     RX_DR           0x40
#define     TX_DS           0x20
#define     MAX_RT          0x10
#define     RX_P_NO_MASK    0x0e
#define     RX_P_NO_SHIFT   1
#define     TX_FULL         0x01
#define OBSERVE_TX      0x08
#define     PLOS_CNT_MASK   0xf0
#define     PLOS_CNT_SHIFT  4
#define     ARC_CNT_MASK    0x0f
#define     ARC_CNT_SHIFT   0
#define RPD             0x09
#define RX_ADDR_P0      0x0A
#define RX_ADDR_P1      0x0B
#define RX_ADDR_P2      0x0C
#define RX_ADDR_P3      0x0D
#define RX_ADDR_P4      0x0E
#define RX_ADDR_P5      0x0F
#define TX_ADDR         0x10
#define RX_PW_P0        0x11
#define RX_PW_P1        0x12
#define RX_PW_P2        0x13
#define RX_PW_P3        0x14
#define RX_PW_P4        0x15
#define RX_PW_P5        0x16
#define FIFO_STATUS     0x17
#define     FIFO_TX_REUSE   0x40
#define     FIFO_TX_FULL    0x20
#define     FIFO_TX_EMPTY   0x10
#define     FIFO_RX_FULL    0x02
#define     FIFO_RX_EMPTY   0x01
#define DYNPD           0x1C
#define FEATURE         0x1D
#define     EN_DPL          0x04
#define     EN_ACK_PAY      0x02
#define     EN_DYN_ACK      0x01

/* function definitions */
void nrf_powerdown(void);
void nrf_standby(void);
void nrf_rx_mode(void);
uint8_t nrf_has_data(void);
uint8_t nrf_rx(uint8_t *data, uint8_t size);
void nrf_tx(uint8_t *data, uint8_t size);
void nrf_wake_queue(void);
void nrf_irq(void);
void nrf_init(void);

void nrf_rx_enable(void);
void nrf_rx_disable(void);

uint8_t nrf_read_reg(uint8_t addr);
void nrf_write_reg(uint8_t addr, uint8_t data);
