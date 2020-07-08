/*
 * UART Protocol Test for Master.c
 *
 * Created: 2019-12-12 오후 3:26:47
 * Author : TyeolRik
 */ 

/*
ATmega128         ATmega128
Master (NOW)       Slave
////////////////////////////
PB4---------------PB0(/SS)		PB4: Red    Line
PB1(SCK)----------PB1(SCK)		PB1: Brown  Line
PB2(MOSI)---------PB2(MOSI)		PB2: Yellow Line
PB3(MISO)---------PB3(MISO)		PB3: Orange Line
////////////////////////////
*/

/* 스위치 키패드 구현
ATmega128         
Master (NOW)       
//////////////////
PD0---------------Orange    Line
PD1---------------Green		Line
PD2---------------Red		Line
PD3---------------Brown		Line
////////////////////////////
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "FND_counter.h"
// #include "Notes.h"

#define F_CPU 16000000
#define SS_1     PORTB |= 0x10	//	0x10 = 0b00010000
#define SS_0     PORTB &=~0x10	//	0x10 = 0b00010000 -> ~0x10 = 0b11101111

volatile unsigned char rx_buf;
unsigned short count = 1;

unsigned char total_number_of_game = 8;

unsigned char master_mode = 1;	// 1: Menu Mode / 2: Game Mode

// 비행기 노래의 버튼 순서와 눌러야하는 타이밍에 대한 전역변수
unsigned char correct_button[50] = {1, 2, 3, 2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 2, 3, 2, 1, 1, 1, 2, 2, 1, 2, 3, 1, 2, 3, 2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 2, 3, 2, 1, 1, 1, 2, 2, 1, 2, 3};
unsigned int correct_msec_time[50];
unsigned int msec = 0;

unsigned short score = 0;

unsigned short _notes_index_count = 0;
unsigned int _msec_memory = 0;

// Debugging
unsigned int test_FND;

volatile unsigned int _menu_memory_msec = 0;
volatile unsigned int _menu_real_msec = 0;

unsigned int tolerance_ms = 250;

// 떳다떳다 비행기를 만드는 함수
void make_airplane_correct_time_array() {
	
	int idx = 0;
	int flow_time = 1130;	// 처음 딜레이를 줄 수 있음. Master와 Slave 간의 시간격차(SPI통신 후 반응속도)가 약 1130ms 임.
	
	// 떳다떳다 비행기는 2/4박자 이므로 4분음표가 550ms를 취함.
	correct_msec_time[idx++] = flow_time = flow_time + 2000 + 4 * 1100;
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 3 / 4; // 412 : 점8분음표
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 1 / 4; // 137 : 16분음표
	correct_msec_time[idx++] = flow_time = flow_time + 550 * 2 / 4; // 275 : 8분음표
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

void initialize_variables() {
	msec = 0;
	score = 0;
	_msec_memory = 0;
	_notes_index_count = 0;
	test_FND = 0;
	
	_menu_memory_msec = 0;
	_menu_real_msec = 0;
	
	make_airplane_correct_time_array();
	/*
	// Initialize Note
	int i;
	int temp = 0;
	for(i = 0; i < 25; i++) {
		temp = temp + 1000;
		correct_msec_time[i] = temp;
	}
	*/
}

unsigned int _abs_minus(int a, int b) {
	if(a > b) {
		return (a - b);
	} else {
		return (b - a);
	}
}

/* SPI 통신을 위한 초기화과정 */
void SPI_Masterinit(void){
	DDRB|=0x17;	//	0x17 = 0b00010111
	SPCR|=0xD0;	//	0xD0 = 0b11010000
	SS_1;
	
}

/* 실제로 SPI 전송을 하는 함수 */
void SPI_TX(unsigned char data){
	SS_0;
	SPCR&=~0x80;	// 0x80 = 0b10000000 -> ~0x80 = 0b01111111
	SPDR=data;
	while(!(SPSR&0x80));	// 0x80 = 0b10000000
	SPCR|=0x80;	// 0x80 = 0b10000000 -> ~0x10 = 0b11101111
}

/* 현재 시간과 노트의 시간 중 근접한 시간을 찾음. */
short find_index_in_array(unsigned int now_time) {
	for(short idx = 0; idx < 50; idx++) {
		if(_abs_minus(now_time, correct_msec_time[idx]) <= tolerance_ms) {
			return idx;
		}
	}
	return -1;		// 너무 빠르거나 늦게 누르면 -1을 리턴함.
}

/* 버튼 입력을 통해서 점수를 리턴함. */
unsigned short getScore(unsigned char button_number, unsigned int time_msec) {
	short now_index = 0;
	// 2번 연속으로 눌렸는지 체크
	if(_abs_minus(time_msec, (int)_msec_memory) < 50) {
		// 연속으로 눌려짐
		return 0;
	} else {
		now_index = find_index_in_array(time_msec);
		if(now_index == -1) {
			// tolerance 타이밍에 누르지 않으면 -1을 리턴함.
			return 0;
		} else {
			if((button_number == correct_button[now_index])) {
				correct_msec_time[now_index] = 0;
				return 5;
			} else {
				correct_msec_time[now_index] = 0;
				return 0;
			}
		}
	}
}

SIGNAL(SPI_STC_vect) {
	rx_buf=SPDR;
	//SS_1;
}

/* 인터럽트 버튼 0번 */
ISR(INT0_vect) {
	int time_difference = _abs_minus(_menu_memory_msec, _menu_real_msec);
	if(time_difference > 100) {
		_menu_memory_msec = _menu_real_msec;
		if(master_mode == 2) {		// Game Mode
			score = score + getScore(1, msec);		// 점수를 증가
		}
		// 버튼이 잘 작동하는지 확인하기 위한 함수
		if(master_mode == 1) { count = 1111; }
	}
}

/* 인터럽트 버튼 1번 */
ISR(INT1_vect) {
	if((int)_abs_minus(_menu_memory_msec, _menu_real_msec) > 100) {
		_menu_memory_msec = _menu_real_msec;
		if(master_mode == 2) {		// Game Mode
			score = score + getScore(2, msec);
		}
		// 버튼이 잘 작동하는지 확인하기 위한 함수
		if(master_mode == 1) { count = 2222; }
	}
}

/* 인터럽트 버튼 2번 */
ISR(INT2_vect) {
	if((int)_abs_minus(_menu_memory_msec, _menu_real_msec) > 100) {
		_menu_memory_msec = _menu_real_msec;
		if(master_mode == 2) {		// Game Mode
		score = score + getScore(3, msec);
		}
		// 버튼이 잘 작동하는지 확인하기 위한 함수
		if(master_mode == 1) { count = 3333; }
	}
}

/* 인터럽트 버튼 3번 */
ISR(INT3_vect) {
	if((int)_abs_minus(_menu_memory_msec, _menu_real_msec) > 100) {
		_menu_memory_msec = _menu_real_msec;
		if(master_mode == 2) {		// Game Mode
			score = score + getScore(4, msec);
		}
		// 버튼이 잘 작동하는지 확인하기 위한 함수
		if(master_mode == 1) { count = 4444; }
	}
}

/* Master ATmega128 우상단 버튼 인터럽트 */
/* Menu Mode에서 곡번호를 선택하는 기능을 구현 */
// 오른쪽 아래버튼 (SW1) 누르면 숫자 올라감
SIGNAL(INT4_vect) {
	count++;
	if(count > total_number_of_game) {
		count = 1;
	}
	_delay_ms(2500);
}


/* Master ATmega128 우하단 버튼 인터럽트 */
/* Menu Mode에서 Game Mode로, 게임을 실행하는 기능 구현 */
// 오른쪽 아래버튼 (SW2) 누르면 데이터 보냄
SIGNAL(INT5_vect) {
	if(_abs_minus(msec, _msec_memory) < 400) {
		// 2번 눌러진거임.
	} else {
		// Change Master Mode
		switch(master_mode) {
			case 1:	// Game Start
				master_mode = 2;
				initialize_variables();
				SPI_TX(count);	// 게임 번호를 Slave한테 보냄. (게임 시작하라고)
				break;
			case 2:	// Game End;
				master_mode = 1;
				initialize_variables();
				SPI_TX(404);	// End game
				break;
		}
	}
}

int main(){
	
	// Atmega 오른쪽 버튼
	DDRE = 0b11001111; // INT4, 5 
	
	// 스위치 4개
	DDRD = 0b11110000;
	
	EICRA = 0xaa; //falling edge
	EICRB = 0xaa; //falling edge
	EIMSK = 0b00111111; //interrupt en
	
	DDRA = 0xFF;	// 0x80 = 0b11111111
	SPI_Masterinit();
	sei();
	
	// Initialize : Seung-Rok Baek
	initialize_FND();
	initialize_variables();
	
	while(1) {
		switch(master_mode) {
			case 1:		// Menu Mode
				FND_show_number(count);
				break;
			case 2:		// Game Mode
				FND_show_number((int)score);
				break;
		}
		msec = msec + 8;
		_menu_real_msec = msec + 8;
	}
}