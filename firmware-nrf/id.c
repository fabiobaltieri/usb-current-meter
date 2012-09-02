#include <avr/eeprom.h>

#include "id.h"

static uint8_t EEMEM ee_board_id;
static uint8_t board_id;

void read_board_id(void)
{
	board_id = eeprom_read_byte(&ee_board_id);
}

uint8_t get_board_id(void)
{
	return board_id;
}

void set_board_id(uint8_t id)
{
	eeprom_write_byte(&ee_board_id, id);
	board_id = id;
}
