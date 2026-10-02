/*
 * DFR0781.c
 *
 * Created: 2024-07-21 오후 4:54:03
 *  Author: JJH
 */ 

#include "BLE_DFR0781/DFR0781.h"
#include <avr/io.h>

void DFR0781_init(void){
	UCSR1A=0x00;
	UCSR1B=0x18; //송수신부 동작 on으로 설정
	UCSR1C=0x06; // 통신 비트수 8bit 설정
	UBRR1H=0x00;
	UBRR1L=8;  //115200전송속도 사용
	//UBRR1H = 0;
	//UBRR1L = 103;  //속도 9600으로 설정
}
void tx_1(char data){ //보드로 보내기
	while(!(UCSR1A & 0x20));
	UDR1=data;
}
char rx_1(){
	while(!(UCSR1A & 0x80));
	return UDR1;
}
void DFR_Tx_Data(char *str)
{
	while(*str) {
		tx_1(*str++);
	}
}

void rx_1_string(unsigned char *buffer) {
	for (int i = 0; i < 100; i++) {
		buffer[i] = 0; // 모든 요소를 0으로 초기화
	}
	
	unsigned int i = 0;
	unsigned char c;
	while (i < 100 - 1) {
		c = rx_1();
		if (c == '\n') {
			break; // 라인 피드가 오면 수신 종료
		}
		buffer[i++] = c;
	}
	buffer[i] = '\0'; // 문자열 끝에 널 문자 추가
}
