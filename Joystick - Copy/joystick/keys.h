/*
 * keys.h
 *
 * Created: 14/07/2015 16:40:39
 *  Author: Paul Qureshi
 */ 


#ifndef KEYS_H_
#define KEYS_H_


#define KEY_TC					TCD0
// 20Hz @ 16MHz
#define KEY_TC_DIV				TC_TC0_CLKSEL_DIV64_gc
#define KEY_TC_PER				0x30D3


extern uint8_t KEY_read(void);
extern void KEY_clear(void);


#endif /* KEYS_H_ */