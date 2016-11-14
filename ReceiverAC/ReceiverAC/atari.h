/*
 * atari.h
 *
 * Created: 04/05/2016 15:39:20
 *  Author: kuro68k
 */ 


#ifndef ATARI_H_
#define ATARI_H_


#define AC_PORT					PORTA
#define AC_JOY_UP_PIN_bm		PIN0_bm
#define AC_JOY_DOWN_PIN_bm		PIN1_bm
#define AC_JOY_LEFT_PIN_bm		PIN2_bm
#define AC_JOY_RIGHT_PIN_bm		PIN3_bm
#define AC_JOY_B1_PIN_bm		PIN4_bm
#define AC_JOY_B2_PIN_bm		PIN5_bm

#define MODE_PORT				PORTC
#define MODE_PIN_bm				PIN2_bm

#define DIR_PORT				PORTD
#define DIR_PIN_bm				PIN4_bm


extern void AC_init(void);
extern void AC_update(REPORT_t *report);


#endif /* ATARI_H_ */