/*
 * Notes.c
 *
 * Created: 2019-12-20 오후 6:32:37
 *  Author: TyeolRik
 */ 

#include "Notes.h"

// unsigned char correct_button[25] = {1, 2, 3, 2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 2, 3, 2, 1, 1, 1, 2, 2, 1, 2, 3};
// unsigned int correct_msec_time[25];

void init() {
	int i;
	int temp = 0;
	for(i = 0; i < 25; i++) {
		temp = temp + 1000;
		// correct_msec_time[i] = temp;
	}
}

void falling_Notes() {
	
}