/*
 * config.c
 *
 * Load and save configuration data from EEPROM.
 */

#include <avr/io.h>
#include <string.h>
#include "global.h"
#include "eeprom.h"
#include "config.h"


const SETTINGS_CONFIG_t default_settings = {
	.length = sizeof(SETTINGS_CONFIG_t),
	.id = SETTINGS_CONFIG_ID,

	.af_high_hz = 15,
	.af_high_duty_pc = 50,
	.af_low_hz = 5,
	.af_low_duty_pc = 25,
	.af_remember_state = 0
};
const SETTINGS_CONFIG_t *settings = &default_settings;
/*
MAPPING_CONFIG_t default_mapping = {
	.id = MAPPING_CONFIG_ID,
	.mapping = {	{	LJOY_UP,	PJOY_UP		},
					{	LJOY_DN,	PJOY_DN		},
					{	LJOY_LF,	PJOY_LF		},
					{	LJOY_RT,	PJOY_RT		},
					{	LMETA,		PMETA		},
					{	LBUTTON1,	PB1			},
					{	LBUTTON2,	PB2			},
					{	LBUTTON3,	PB3			},
					{	LBUTTON4,	PB4			},
					{	LBUTTON5,	PB5			},
					{	LBUTTON6,	PB6			},
					{	LBUTTON7,	PB7			},
					{	LBUTTON8,	PB8			},
					{	LBUTTON9,	PB9			},
					{	LBUTTON10,	PB10		},
					{	LBUTTON11,	PB11		},
					{	LBUTTON12,	PB12		},
					{	LBUTTON13,	PB13		},
					{	LBUTTON14,	PB14		},
					{	LBUTTON15,	PB15		},
					{	LBUTTON16,	PB16		},
				},
};*/
MAPPING_CONFIG_t default_mapping = {	// Hori Fighting Stick Famicom
	.id = MAPPING_CONFIG_ID,
	.mapping = {	{	LJOY_UP,	PJOY_UP		},
					{	LJOY_DN,	PJOY_DN		},
					{	LJOY_LF,	PJOY_LF		},
					{	LJOY_RT,	PJOY_RT		},
					{	LMETA,		PMETA		},
					{	LBUTTON1,	PB1			},
					{	LBUTTON2,	PB2			},
					{	LBUTTON3,	PB3			},
					{	LBUTTON4,	PB4			},
					{	LBUTTON5,	PB8			},
					{	LBUTTON6,	PB7			},
					{	LBUTTON15,	PA1			},	// select
					{	LBUTTON16,	PA4			},	// start
					{	LAF_LOW_1,	PR7			},
					{	LAF_LOW_2,	PR2			},
					{	LAF_LOW_3,	PR5			},
					{	LAF_LOW_4,	PR4			},
					{	LAF_LOW_5,	PA7			},
					{	LAF_LOW_6,	PA6			},
					{	LAF_HIGH_1,	PR8			},
					{	LAF_HIGH_2,	PR3			},
					{	LAF_HIGH_3,	PR6			},
					{	LAF_HIGH_4,	PR5			},
					{	LAF_HIGH_5,	PA8			},
					{	LAF_HIGH_6,	PA5			},
				},
};
const MAPPING_CONFIG_t *map = &default_mapping;


/**************************************************************************************************
** Calculate CRC for an arbitrary buffer
*/
uint32_t cfg_calc_crc(const void *buffer, uint16_t size)
{
	CRC.CTRL = CRC_RESET_RESET1_gc;
	asm("nop");
	CRC.CTRL = CRC_CRC32_bm | CRC_SOURCE_IO_gc;
	uint8_t *ptr = (uint8_t *)buffer;
	while (size--)
		CRC.DATAIN = *ptr++;
	CRC.CTRL |= CRC_BUSY_bm;
	return CRC.CHECKSUM0 | ((uint32_t)CRC.CHECKSUM0 << 8) | ((uint32_t)CRC.CHECKSUM0 << 16) | ((uint32_t)CRC.CHECKSUM0 << 24);
}

/**************************************************************************************************
** Find config in EEPROM by ID number. Returns 0 if not found or bad CRC.
*/
void * CFG_find_config(uint8_t id)
{
	CONFIG_HEADER_t *header = (CONFIG_HEADER_t *)EEP_MAPPED_ADDR(0, 0);

	do
	{
		if ((header->length == 0xFFFF) || (header->length == 0x0000) ||
			(header->id == 0x00) || (header->id == 0xFF))
			break;

		if (header->id == id)
		{
			// check CRC
			uint32_t *crc = (uint32_t *)header + header->length;
			if (*crc != cfg_calc_crc(header, header->length))
				break;
			return header;
		}
	} while ((uint16_t)header < (MAPPED_EEPROM_START + EEPROM_SIZE));
	return 0;
}

/**************************************************************************************************
** Prepare config for use
*/
void CFG_init(void)
{
	EEP_EnableMapping();

	void *ptr;
	if ((ptr = CFG_find_config(SETTINGS_CONFIG_ID)))
	{
		settings = ptr;
		if (settings->length != sizeof(SETTINGS_CONFIG_t))
			settings = &default_settings;
	}

	// size of flexible arrays can't be computed inside initializer
	default_mapping.length = __builtin_object_size(&default_mapping, 0);
	default_mapping.count = (default_mapping.length - 4) / 2;

	if ((ptr = CFG_find_config(MAPPING_CONFIG_ID)))
		map = ptr;
}
