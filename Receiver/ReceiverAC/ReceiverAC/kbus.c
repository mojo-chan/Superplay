/*
 * kbus.c
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>
#include "hw_misc.h"
#include "report.h"
#include "atari.h"
#include "kbus.h"

#define LE_CHR(a,b,c,d)		( ((uint32_t)(a)<<24) | ((uint32_t)(b)<<16) | ((c)<<8) | (d) )

#define BUFFER_SIZE		(sizeof(KBUS_PACKET_t) + 2)

volatile uint8_t		rx_buffer_DMA[BUFFER_SIZE];
volatile KBUS_PACKET_t	*rx_packet_DMA = (KBUS_PACKET_t *)rx_buffer_DMA;
volatile uint8_t		packet_ready_SIG = 0;
volatile uint8_t		timeout_SIG = 0;

KBUS_PACKET_t			packet;

#pragma region Transport Layer

/**************************************************************************************************
* Transmit a byte
*/
static inline void kbus_tx(uint8_t byte)
{
	while (KBUS_USART.STATUS & USART_DREIF_bm);
	KBUS_USART.DATA = byte;
}

/**************************************************************************************************
* Reset DMA to start of input buffer. DMA is NOT enabled.
*/
void kbus_reset_dma(void)
{
	KBUS_DMA_CH.CTRLA &= ~EDMA_CH_ENABLE_bm;
	while(KBUS_DMA_CH.CTRLA & EDMA_CH_ENABLE_bm);

	KBUS_DMA_CH.TRFCNTL = sizeof(KBUS_PACKET_t);
	KBUS_DMA_CH.ADDRL = (( (uint16_t)&rx_buffer_DMA) >> 0) & 0xFF;
	KBUS_DMA_CH.ADDRH = (( (uint16_t)&rx_buffer_DMA) >> 8) & 0xFF;
}

/**************************************************************************************************
* Set up K-BUS interface. RX DMA configured but not enabled.
*/
void KBUS_init(void)
{
	// USART
	KBUS_PORT.DIRCLR = KBUS_RX_PIN_bm;
	KBUS_PORT.DIRSET = KBUS_TX_PIN_bm;
	KBUS_PORT.KBUS_RX_PINCTRL = (KBUS_PORT.KBUS_RX_PINCTRL & ~PORT_ISC_gm) | PORT_ISC_FALLING_gc;	// event triggered on falling edge

	KBUS_USART.CTRLA = 0;
	KBUS_USART.BAUDCTRLB = (uint8_t)((uint8_t)BSCALE << 4) | ((uint16_t)BSEL >> 8);
	KBUS_USART.BAUDCTRLA = (uint8_t)BSEL;
	KBUS_USART.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
	KBUS_USART.CTRLB = USART_RXEN_bm | USART_TXEN_bm | USART_CLK2X_bm;

	// DMA from USART
	KBUS_DMA_CH.CTRLA = EDMA_CH_RESET_bm;
	NOP();
	KBUS_DMA_CH.CTRLA = 0;
	KBUS_DMA_CH.CTRLB = 0;
	KBUS_DMA_CH.ADDRCTRL = EDMA_CH_RELOAD_BLOCK_gc | EDMA_CH_DIR_INC_gc;
	KBUS_DMA_CH.TRIGSRC = KBUS_DMA_TRIGGER_SRC;
	kbus_reset_dma();

	// incoming character detection event
	KBUS_EVENT_CTRL = KBUS_EVENT_MUX;

	// timeout
	KBUS_TC.CTRLA = 0;
	KBUS_TC.CTRLB = 0;
	KBUS_TC.CTRLC = 0;
	KBUS_TC.CTRLD = TC45_EVACT_RESTART_gc | KBUS_EVSEL;
	KBUS_TC.CTRLE = 0;
	KBUS_TC.INTCTRLA = TC45_OVFINTLVL_LO_gc;
	KBUS_TC.INTCTRLB = 0;
	KBUS_TC.CNT = 0;
	KBUS_TC.PER = 0x9C3F;	// timeout, 10mS @ 16MHz/4
	KBUS_TC.CCA = 0x004F;	// end of packet, 20uS @ 16MHz/4
	KBUS_TC.CTRLA = TC45_CLKSEL_DIV4_gc;
}

/**************************************************************************************************
* Restart timeout timer
*/
inline void kbus_restart_timeout(void)
{
	KBUS_TC.CNT = KBUS_TC.CCA + 1;	// reset to just past end of packet timeout
									// RX pin events will reset to zero if a new packet comes in
	timeout_SIG = 0;
}

/**************************************************************************************************
* Handle timer overflows (controller not sending reports)
*/
ISR(KBUS_TC_OVF_vect)
{
	timeout_SIG = 0xFF;
}

/**************************************************************************************************
* Handle timer CCA matches (end of packet)
*/
ISR(KBUS_TC_CCA_vect)
{
	packet_ready_SIG = 0xFF;
}

/**************************************************************************************************
* Send data out over K-BUS
*/
void kbus_send(const void *buffer, uint8_t length)
{
	kbus_tx(0xFF);	// preamble to aid UART sync/auto-baud
	kbus_tx(0xFF);

	CRC.CTRL = CRC_RESET1_bm;
	NOP();
	CRC.CTRL = CRC_SOURCE_IO_gc;

	while(length--)
	{
		kbus_tx(*(uint8_t *)buffer);
		CRC.DATAIN = *(uint8_t *)buffer++;
	}

	kbus_tx(CRC.CHECKSUM0);		// CRC, little endian
	kbus_tx(CRC.CHECKSUM1);

	kbus_reset_dma();
	KBUS_DMA_CH.CTRLA |= EDMA_CH_ENABLE_bm;
	kbus_restart_timeout();
	packet_ready_SIG = 0;
}

/**************************************************************************************************
* Find a KBUS device to talk to
*/
void kbus_find_device(void)
{
	for(;;)
	{
		// send sync signal
		while ((KBUS_USART.STATUS & USART_DREIF_bm) == 0);
		KBUS_USART.DATA = 0xFF;

		// check for responses from device
		if (KBUS_USART.STATUS & USART_RXCIF_bm)
		{
			if (KBUS_USART.DATA == 0x0F)	// OK response
				break;
		}
	}

	// wait for device to stop sending
	do
	{
		KBUS_USART.DATA;	// clear buffer and RX flag
		_delay_ms(10);		// KBUS must be quiet for at least 10ms
	} while ((KBUS_USART.STATUS & USART_RXCIF_bm) == 0);
}

/**************************************************************************************************
* Validate the packet in the input buffer. Command in the packet must be command + response bit
*/
bool kbus_validate_packet(uint8_t command)
{
	if (rx_packet_DMA->length > 63)
		return false;
	if (*(uint16_t *)&rx_buffer_DMA[sizeof(KBUS_PACKET_t)] != HW_crc16((void *)rx_buffer_DMA, sizeof(KBUS_PACKET_t)))
		return false;

	if (rx_packet_DMA->command != (command | RESPONSE_BIT_bm))
		return false;

	return true;
}

#pragma endregion

/**************************************************************************************************
* State machine STATE_PING_TEST. Does an echo test to check that the link is working.
*/
bool kbus_state_ping_test(void)
{
	uint8_t retries = 0;

	do
	{
		KBUS_PACKET_t cmd;
		cmd.command = KCMD_LOOPBACK;
		cmd.length = 63;
		for (uint8_t i = 0; i < KBUS_PACKET_DATA_SIZE; i++)
			cmd.data[i] = i;
		kbus_send(&cmd, 2 + 63 + 2);

		// get response
		uint8_t rx_count = 0;
		do
		{
			if (timeout_SIG)
				break;

			if (packet_ready_SIG)
			{
				if (!kbus_validate_packet(cmd.command) ||
					memcmp((void *)rx_packet_DMA->data, &cmd.data, KBUS_PACKET_DATA_SIZE) != 0)
				{
					rx_count++;
					break;
				}
				return true;
			}
		} while (rx_count < 5);

		retries++;
		_delay_ms(1);
	} while(retries < 3);

	return false;
}

/**************************************************************************************************
* Main loop, continually reads updates from K-BUS
*/
void KBUS_run(void)
{
	uint8_t retries = 0;

	AC_init();

	// ask for a report
	KBUS_PACKET_t report_cmd;
	report_cmd.command = KCMD_READ_REPORT;
	report_cmd.length = 0;
	*(uint16_t *)&report_cmd.data[0] = HW_crc16(&report_cmd, 2);

	for(;;)
	{
		// find device to talk to
		for(;;)
		{
			kbus_find_device();				// wait for device to be connected
			if (kbus_state_ping_test())		// test link is working
				break;
		}

		// TODO: read configs

		// updates come continuously
		retries = 0;
		do
		{
			if (timeout_SIG)
				retries++;

			if (packet_ready_SIG)
			{
				if (!kbus_validate_packet(KCMD_READ_REPORT) ||
					packet.length != 16)	// fixed report size
				{
					retries++;
				}
				else
				{
					RPT_decode_kbus_matrix((uint8_t *)&packet.data);
					RPT_refresh_input_matrix();
					AC_update();
					retries = 0;
				}
				break;
			}
		} while (retries < 3);

		_delay_ms(1);
	}
}
