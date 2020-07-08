/*
 * FND_counter.h
 *
 * Created: 2019-12-20 오후 1:44:52
 *  Author: TyeolRik
 */ 
/*
unsigned char digit[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67};
unsigned char fnd_sel[4] = {0x08, 0x04, 0x02, 0x01};
unsigned char fnd[4];
unsigned char dot = 0x80;
*/

void initialize_FND();
void FND_show_number(unsigned short number);