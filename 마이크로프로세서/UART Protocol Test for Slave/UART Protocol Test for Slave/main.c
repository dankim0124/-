/*
 * UART Protocol Test for Slave.c
 *
 * Created: 2019-12-12 오후 4:15:52
 * Author : TyeolRik
 */

/*
ATmega128         ATmega128
Master            Slave(NOW)
////////////////////////////
PB4---------------PB0(/SS)
PB1(SCK)----------PB1(SCK)
PB2(MOSI)---------PB2(MOSI)
PB3(MISO)---------PB3(MISO)
////////////////////////////
*/

#define WDR asm("WDR")

#include <avr/io.h>
#include <math.h>
#include <avr/interrupt.h>

// _delay_ms() 함수 등이 정의되어 있음
#include <util/delay.h>

#define F_CPU 16000000	      // Sets up the default speed for delay.h

volatile unsigned char rx_buf;

volatile unsigned char temp;

unsigned char mode = 1;		// 1 : 게임 안함 / 2 : 게임 함

volatile int falling_timing = 126;	// 몇 초만에 떨어질 것인가? 2초(2000ms) 만에!

unsigned char correct_button[50] = {1, 2, 3, 2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 2, 3, 2, 1, 1, 1, 2, 2, 1, 2, 3, 1, 2, 3, 2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 2, 3, 2, 1, 1, 1, 2, 2, 1, 2, 3};	// 50개의 노트에 대해서 눌러야하는 버튼의 번호
unsigned int correct_msec_time[50];		// 노래 박자에 대한 배열

unsigned char COLS[] = {
	0B00000000,
	0B00000000,
	0B00000000,
	0B00000000,
	0B00000000,
	0B00000000,
	0B00000000,
0B00000000};

unsigned char COLS2[] = {
	0B00000000,
	0B00000000,
	0B00000000,
	0B00000000,
	0B00000000,
	0B00000000,
	0B00000000,
0B00000000};

unsigned int _abs_minus(unsigned int a, unsigned int b) {
	if(a > b) {
		return a - b;
		} else {
		return b - a;
	}
}

// 떳다떳다 비행기를 만드는 함수
void make_airplane_correct_time_array() {
	
	int idx = 0;
	int flow_time = 0;
	
	correct_msec_time[idx++] = flow_time = flow_time + 2000 + 4 * 1100;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 3 / 4; // 412
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 1 / 4; // 137
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4; // 275
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 3 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 1 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 + 4 * 1100;		// 전주
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 3 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 1 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 3 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 1 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4;
	
}

/* 1 Row 씩 다음 Row로 넘기면서 Frame을 구현 */
void move_one_frame() {
	char idx = 0;
	for (idx = 7; idx >= 1; idx--) {
		COLS2[idx] = COLS2[idx - 1];
	}
	COLS2[0] = COLS[7];
	for (idx = 7; idx >= 1; idx--) {
		COLS[idx] = COLS[idx - 1];
	}
	COLS[0] = 0b00000000;
}

/* 특정 열에 떨어질 노트(음표)를 구현 */
void add_note(unsigned char column) {
	// 하드코딩한 이유가 있는데,
	// 컴퓨터는 십진법을 잘 알아먹는데 ATmega는 못 알아먹음.
	switch(column) {
		case 0:
		COLS[0] = 0b00000001;
		break;
		case 1:
		COLS[0] = 0b00000010;
		break;
		case 2:
		COLS[0] = 0b00000100;
		break;
		case 3:
		COLS[0] = 0b00001000;
		break;
		case 4:
		COLS[0] = 0b00010000;
		break;
		case 5:
		COLS[0] = 0b00100000;
		break;
		case 6:
		COLS[0] = 0b01000000;
		break;
		case 7:
		COLS[0] = 0b10000000;
		break;
	}
}

/* 상단 매트릭스를 보여주는 함수 */
void show_dot_matrix1_one_image_frame_cost_8ms() {
	unsigned char idx = 0;
	for(idx = 0; idx < 8; idx++){
		PORTF = COLS[idx];
		PORTD = 0b11111111 - (0b10000000>>(idx));
		
		// PORTD = 0b11111111 - (1<<(idx));
		_delay_ms(1);    // 2ms 동안 대기
	}
	PORTD = 0b11111111;
}

/* 하단 매트릭스를 보여주는 함수 */
void show_dot_matrix2_one_image_frame_cost_8ms() {
	unsigned char idx = 0;
	for(idx = 0; idx < 8; idx++){
		PORTA = COLS2[idx];
		PORTC = 0b11111111 - (0b10000000>>(idx));
		// PORTC = 0b11111111 - (1<<(idx));
		_delay_ms(1);    // 2ms 동안 대기
	}
	PORTC = 0b11111111;
}

/* LED를 실행하는 함수 */
void START_LED(unsigned char game_number) {
	
	DDRA = 0b11111111;
	DDRC = 0b11111111;
	DDRF = 0b11111111;
	DDRD = 0b11111111;
	
	unsigned char i,j;        // 8비트의 변수 선언
	
	unsigned int the_number_of_falling = 0;
	volatile unsigned int count = 0;
	unsigned short idx = 0;
	
	volatile unsigned int real_millisecond = 0;
	
	unsigned int one_loop_time_cost = 66;
	
	// while 블록 안의 문장을 무한 반복
	while(real_millisecond < 50000){
		volatile unsigned int msec;
		
		move_one_frame();
		for (msec = 0; msec < falling_timing / 2; msec++) {
			show_dot_matrix1_one_image_frame_cost_8ms();		// COLS 배열을 보여줌
		}
		for (msec = 0; msec < falling_timing / 2; msec++) {
			show_dot_matrix2_one_image_frame_cost_8ms();		// COLS2 배열을 보여줌
		}
		
		if(_abs_minus(real_millisecond, correct_msec_time[idx] + 500) < one_loop_time_cost) {	// 노트를 500ms 늦게 출력함.
			add_note(correct_button[idx] * 2);
			idx++;
		}
		
		real_millisecond = real_millisecond + one_loop_time_cost;				// 위의 코드 실행하는데 실제로 (one_loop_time_cost) ms 정도 걸림.
	}
	
	switch(game_number) {
		case 1:
		
		break;
	}
	
	// 함수의 형태와 같이 정수형(int)의 값을 반환함
	return 1;
}

/* Slave SPI 통신 전 초기화 과정 */
void SPI_Slaveinit(void){
	DDRB|=0x08;
	SPCR|=0xC0;
	
	// 서지훈
	DDRA = 0b11111111;
	DDRC = 0b11111111;
	make_airplane_correct_time_array();
}

/* Slave SPI 송신 함수 */
void SPI_TX(unsigned char data){
	SPCR&=~0x80;
	SPDR=data;
	while(!(SPSR&0x80));
	SPCR|=0x80;
	PORTA=rx_buf;
}

// 김도현
void START_MUSIC(unsigned char game_number) {
	DDRG = 0b11111111;
	PORTG = 0b11111111;
	/*
	while(1){
		// _delay_ms(100000);
		PORTA=0b11111111;
	}
	*/
}

// 실제로 게임을 시작하는 함수
int GAME_START(unsigned char game_number) {
	START_MUSIC(game_number);
	START_LED(game_number);
	mode = 1;
}

/* SPI 통신으로 데이터를 입력받는 인터럽트 */
ISR(SPI_STC_vect){
	// Data 받아옴 Master로 부터 -> SPDR에 저장됨
	if(SPDR == 404) {
		WDR;		// Soft 리셋 함수
	} else if(mode == 1) {
		mode = 2;
		GAME_START(SPDR);
	}
}

int main(){
	DDRG = 0b11111111;
	// DDRA = 0xFF;
	SPI_Slaveinit();
	//	SREG = 0x80;	// Old Version
	sei();
	while(1){
		//PORTA=rx_buf;
	}
}