/*
 * FND_counter.c
 *
 * Created: 2019-12-20 오후 1:47:54
 *  Author: TyeolRik
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "FND_counter.h"

unsigned char digit[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67};
unsigned char fnd_sel[4] = {0x08, 0x04, 0x02, 0x01};
unsigned char fnd[4];
unsigned char dot = 0x80;

void initialize_FND() {
	DDRC = 0xFF;
	DDRG = 0x0F;
	PORTG = 0x0F;
	
	// Turn ON
	PORTC = 0b11111111;		// ON
	PORTG = 0b00001111;		// 모든 FND 선택
	
	// Turn OFF
	PORTC = 0b00000000;		// ON
	PORTG = 0b00001111;		// 모든 FND 선택
}

/* FND로 숫자를 출력함 */
void FND_show_number(unsigned short number) {
	
	int i;
	// unsigned short _count = 0;
	
	fnd[0] = digit[number / 1000];
	fnd[1] = digit[(number % 1000) / 100];
	fnd[2] = digit[(number % 100) / 10];
	fnd[3] = digit[(number % 10)];
	for(i = 0; i < 4; i++) {
		PORTC = fnd[i];
		PORTG = fnd_sel[i];
		_delay_ms(2);	// 2ms * 4회 = 8ms => 8ms 당 count 1 증가 => count == 125 일 때, 1초 증가
	}
}