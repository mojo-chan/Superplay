/*
 * io_table.c
 *
 */

#include <avr/io.h>
#include "config.h"
#include "io_table.h"

// logical input to physical IO port mapping
// V2.0 hardware
#define IO_TABLE \
		ENTRY(PJOY_UP,		&PORTA, PIN0_bm) \
		ENTRY(PJOY_DN,		&PORTA, PIN1_bm) \
		ENTRY(PJOY_LF,		&PORTA, PIN2_bm) \
		ENTRY(PJOY_RT,		&PORTA, PIN3_bm) \
		ENTRY(PB16,			&PORTA, PIN4_bm) \
		ENTRY(PB15,			&PORTA, PIN5_bm) \
		ENTRY(PB14,			&PORTA, PIN6_bm) \
		ENTRY(PB13,			&PORTA, PIN7_bm) \
		\
		ENTRY(PB1,			&PORTB, PIN0_bm) \
		ENTRY(PB2,			&PORTB, PIN1_bm) \
		ENTRY(PB3,			&PORTB, PIN2_bm) \
		ENTRY(PB4,			&PORTB, PIN3_bm) \
		ENTRY(PB5,			&PORTB, PIN4_bm) \
		ENTRY(PB6,			&PORTB, PIN5_bm) \
		ENTRY(PB7,			&PORTB, PIN6_bm) \
		ENTRY(PB8,			&PORTB, PIN7_bm) \
		\
		ENTRY(PB9,			&PORTC, PIN4_bm) \
		ENTRY(PB10,			&PORTC, PIN5_bm) \
		ENTRY(PB11,			&PORTC, PIN6_bm) \
		ENTRY(PB12,			&PORTC, PIN7_bm) \
		\
		ENTRY(PA5,			&PORTD, PIN0_bm) \
		ENTRY(PA6,			&PORTD, PIN1_bm) \
		ENTRY(PA7,			&PORTD, PIN2_bm) \
		ENTRY(PA8,			&PORTD, PIN3_bm) \
		ENTRY(PMETA,		&PORTD, PIN4_bm) \
		ENTRY(PLED,			&PORTD, PIN5_bm) \
		\
		ENTRY(PR1,			&PORTE, PIN0_bm) \
		ENTRY(PR2,			&PORTE, PIN1_bm) \
		ENTRY(PR3,			&PORTE, PIN2_bm) \
		ENTRY(PR4,			&PORTE, PIN3_bm) \
		ENTRY(PR5,			&PORTE, PIN4_bm) \
		ENTRY(PR6,			&PORTE, PIN5_bm) \
		ENTRY(PR7,			&PORTE, PIN6_bm) \
		ENTRY(PR8,			&PORTE, PIN7_bm) \
		\
		ENTRY(PR9,			&PORTF, PIN0_bm) \
		ENTRY(PR10,			&PORTF, PIN1_bm) \
		ENTRY(PR11,			&PORTF, PIN2_bm) \
		ENTRY(PR12,			&PORTF, PIN3_bm) \
		ENTRY(PA1,			&PORTF, PIN4_bm) \
		ENTRY(PA2,			&PORTF, PIN5_bm) \
		ENTRY(PA3,			&PORTF, PIN6_bm) \
		ENTRY(PA4,			&PORTF, PIN7_bm)


const __flash  IO_PIN_t	io_pin_table[256] = {
#define	ENTRY(name, port, pin_mask)		[name] = {port, pin_mask},
	IO_TABLE
#undef ENTRY
};
