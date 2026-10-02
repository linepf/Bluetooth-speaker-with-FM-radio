/*
 * J_Speaker.c
 *
 * Created: 2024-07-21 오후 4:52:45
 * Author : JJH
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "BLE_DFR0781/DFR0781.h"
#include "LCD_include_I2C/LCD_include_I2C.h"
#include "I2C/I2C.h"
#include "TEA5767/TEA5767.h"

#define name_size 20
unsigned long hz = 931;  //93.1MHz 초기 주파수  ex : 981 = 98.1Mhz
char add_hz = 1;
char Play_mode = 0;  //USB모드에서만 사용 가능 / 0 : 순서대로, 1 : 한곡 반복, 2 : 무작위 재생
char Mode = 0 ;  //기기의 모드 설정 기본 BLE모드로 시작 0 : BLE,/ 1 : USB,/ 2 : FM radio
int counter = 0;  //엔코더 카운터를 위한 변수 1000 == 1초
bool button_press = 0; //엔코더 버튼 on/off확인을 위한 변수
unsigned char song_number1[name_size]={0};
unsigned char song_number2[name_size]={0};
unsigned char song_name[name_size]={0}; //USB모드에서 노래 이름을 저장할 배열
unsigned char hz_arr[name_size]={0};
////////////////////////디버깅용////////////////////////
//컴퓨터와의 통신
/*
void UART0_init(void){
	UCSR0A=0x00;
	UCSR0B=0x18; //송수신부 동작 on으로 설정
	UCSR0C=0x06; // 통신 비트수 8bit 설정
	UBRR0H=0x00;
	UBRR0L=8;
}
void tx_0(char data){
	while(!(UCSR0A & 0x20));
	UDR0=data;
}
void tx_0_str(unsigned char *str){
	while(*str) tx_0(*str++);
}
char rx_0(void){
	while(!(UCSR0A & 0x80));
	return UDR0;
}*/
////////////////////////디버깅용////////////////////////
////////////////////////인터럽트, 타이머////////////////////////
void interrupt_init(void){
	EIMSK = 0x30;  //INT4, INT5 사용
	EICRA = 0x00;  //
	EICRB = 0x01;  //INT4는 논리적 변경 발생시 인터럽트 사용 즉 HIGH, LOW 둘다 인터럽트 발생
	sei();
}
void timer_counter_init(){
		TCCR0 = 0x04;   //Normal모드로 동작, OCn핀 출력 연결을 끊음, 프리스케일러값  64분주
		TIMSK = 0x01;   //TCNT값이 255이면 오버플로 인터럽트 사용
		//TCNT0 = 0x06;   //정확한 1ms를 카운터 하기 위해서 TCNT0의 시작 값을 6부터 시작, 250까지 카운트 될시에 1ms
}
ISR(TIMER0_OVF_vect){  //TCNT0을 아래서 설정한 6 ~ 255까지 카운터 후 256이 되는 순간 인터럽트 발생, 인터럽트가 1번 발생할때 마다 1ms 즉 1000번 발생하면 1초 / 인터럽트 문법은 암기
	//if(counter>100) counter = 0; //counter변수의 오버플로 방지
	if(button_press) counter++;
}
ISR(INT4_vect){   //"INT4_vect" 인터럽트 4번 핀 / 엔코더 푸쉬 버튼 / PE4
	if((PINE & 0x10 )==0){ //엔코더를 눌렀을 때
		TCNT0 = 0x06;
		button_press = 1;
		counter = 0;
	}else{
			button_press = 0;
			if(counter>=4000){	//4초 이상 누를시 모드 변경
				++Mode;
				if(Mode>2) {Mode=0;} //3가지 모드
				Mode_change(Mode);
				counter = 0;
			}else{	//일시정지
				if(Mode==0) {DFR_Tx_Data("AT+CB\r\n");} //BLE모드일때 일시정지
				if(Mode==1) DFR_Tx_Data("AT+AA03\r\n"); //USB모드일때 일시정지
				if(Mode==2){	//FM모드일때 누르면 체널 이동값 *10
					if(add_hz==1){
						 add_hz=10;
						 i2c_lcd_goto_XY(1,13);//커서 위치 설정
						 i2c_lcd_write_string("x10");//이름 출력
						 }
					else {
						add_hz=1;
						i2c_lcd_goto_XY(1,13);//커서 위치 설정
						i2c_lcd_write_string(" x1");//이름 출력
						}
					}
			}
	}
}
ISR(INT5_vect){//엔코더 회전  /PE5
	if(button_press==0){
		if((PINE & 0x40)==0){	//PE6
			if(Mode==0) DFR_Tx_Data("AT+CC\r\n");  //BLE모드일때 다음노래 재생
			if(Mode==1){//USB모드일때 다음노래 재생
						DFR_Tx_Data("AT+CC\r\n"); //다음노래 재생
						Mode_State_LCD_set(); //노래 이름이 바뀔 때마다 화면 초기화
						DFR_Tx_Data("AT+MF\r\n"); //????
						read_Songname(song_name); //????
						DFR_Tx_Data("AT+MF\r\n"); //현재 노래 이름 반환 요청
						read_Songname(song_name); //노래이름을 받아서 song_name배열에 저장
						Cleanup_name(); //처음 받아온 이름 정리
						i2c_lcd_goto_XY(1,0);//커서 위치 설정
						i2c_lcd_write_string(song_name);//이름 출력
				 } 
			if(Mode==2){
				hz=hz+add_hz;
				if(hz>1080) hz = 760;
				TEA5767_write(hz);	//
				sprintf(hz_arr, "%4d", hz);
				changg_hz_arr(hz_arr);
				i2c_lcd_goto_XY(1,0); //커서 위치 설정
				i2c_lcd_write_string(hz_arr); //이름 출력
				i2c_lcd_write_string("MHz");
				clearn_hz_arr();
			}
		}else {
			if(Mode==0) DFR_Tx_Data("AT+CD\r\n");  //BLE모드일때 이전노래 재생
			if(Mode==1){	//USB모드일때 이전노래 재생
				 DFR_Tx_Data("AT+CD\r\n"); //이전노래 재생
				 Mode_State_LCD_set();	   //노래 이름이 바뀔 때마다 화면 초기화
				 DFR_Tx_Data("AT+MF\r\n"); //????
				 read_Songname(song_name); //????
				 DFR_Tx_Data("AT+MF\r\n"); //현재 노래 이름 반환 요청
				 read_Songname(song_name); //노래이름을 받아서 song_name배열에 저장
				 Cleanup_name();//처음 받아온 이름 정리
				 i2c_lcd_goto_XY(1,0); //커서 위치 설정
				 i2c_lcd_write_string(song_name); //이름 출력
				 } 
			if(Mode==2){
				hz=hz-add_hz;
				if(hz<760) hz = 1080;
				TEA5767_write(hz);	//
				sprintf(hz_arr, "%4d", hz);
				changg_hz_arr(hz_arr);
				i2c_lcd_goto_XY(1,0); //커서 위치 설정
				i2c_lcd_write_string(hz_arr); //이름 출력
				i2c_lcd_write_string("MHz");
				clearn_hz_arr();
			}
		}
	}
	else {
		if((PINE & 0x40)==0){
			if(Mode==1){//USB모드에서만 작동
					Play_mode++;
					if(Play_mode>2) Play_mode=0;
					USBPlay_mode_change(Play_mode);
			}
		}
	}
		_delay_ms(500);
}  
////////////////////////인터럽트, 타이머////////////////////////

void Mode_State_LCD_set(){
	if(Mode == 0){	//BLE모드
		i2c_lcd_command(0x01); //화면 초기화
		i2c_lcd_write_string("MODE: ");
		i2c_lcd_write_string("BLE");
	}
	
	if(Mode == 1){	//USB모드 일때
		i2c_lcd_command(0x01); //화면 초기화
		i2c_lcd_write_string("MODE: ");
		i2c_lcd_write_string("USB");
		USBPlay_mode_change(Play_mode);
	}
	
	if(Mode == 2){ //DFR은 대기 모드, 라디오 모드
		i2c_lcd_command(0x01); //화면 초기화
		i2c_lcd_write_string("MODE: ");
		i2c_lcd_write_string("FM Radio");
	}
}
void Mode_change(char mode){
	if(mode == 0){	//BLE모드
			i2c_lcd_command(0x01); //화면 초기화
			i2c_lcd_write_string("MODE: ");
			i2c_lcd_write_string("BLE");
			_delay_ms(500); //릴레이가 물리적으로 움직이는 시간
			PORTF = 0x00;
			DFR_Tx_Data("AT+CM01\r\n"); //BLE모드 변경
		 }
		   
	if(mode == 1){	//USB모드 일때 
			i2c_lcd_command(0x01); //화면 초기화
			i2c_lcd_write_string("MODE: ");
			i2c_lcd_write_string("USB");
			DFR_Tx_Data("AT+CM02\r\n"); //USB모드 변경
			USBPlay_mode_change(Play_mode);
			PORTF = 0x00;
		 } 
		 
	if(mode == 2){ //DFR은 대기 모드, 라디오 모드
			i2c_lcd_command(0x01); //화면 초기화
			i2c_lcd_write_string("MODE: ");
			i2c_lcd_write_string("FM Radio");
			DFR_Tx_Data("AT+CM08\r\n"); //DFR대기 모드
			PORTF = 0x01;
			_delay_ms(500); //릴레이가 물리적으로 움직이는 시간
			TEA5767_write(hz);	//초기 주파수  MHz 
			sprintf(hz_arr, "%4d", hz);
			changg_hz_arr(hz_arr);
			i2c_lcd_goto_XY(1,0); //커서 위치 설정
			i2c_lcd_write_string(hz_arr); //이름 출력
			i2c_lcd_write_string("MHz");
			i2c_lcd_goto_XY(1,13);//커서 위치 설정
			i2c_lcd_write_string(" x1");//이름 출력
			clearn_hz_arr();
		 } 
}
void USBPlay_mode_change(char mode){//USB모드에서만 작동
	if(mode==0){
		DFR_Tx_Data("AT+AC00\r\n"); //일반 재생
		i2c_lcd_goto_XY(0,13);
		i2c_lcd_write_string("Re");
	}
	if(mode==1){
		DFR_Tx_Data("AT+AC02\r\n");//한곡재생
		i2c_lcd_goto_XY(0,13);
		i2c_lcd_write_string("On");
	}
	if(mode==2){
		DFR_Tx_Data("AT+AC03\r\n");//랜던 재생
		i2c_lcd_goto_XY(0,13);
		i2c_lcd_write_string("Ra");
	}
}
void read_Songname(unsigned char *buffer){ //노래이름을 받아서 song_name배열에 저장
	for (int i = 0; i < name_size; i++) {
		buffer[i] = 0; // 모든 요소를 0으로 초기화
	}
	
	unsigned int i = 0;
	unsigned char c;
	while (i < name_size - 1) {
		c = rx_1();
		if (c == '\n') {
			break; // 라인 피드가 오면 수신 종료
		}
		buffer[i++] = c;
	}
	buffer[i] = '\0'; // 문자열 끝에 널 문자 추가
}
void Cleanup_name(void){
	//이름 앞에 있는 불필요한 3글자 삭제
	int a=3;
	for(int i=0; i<name_size-a; i++){
		song_name[i] = song_name[i+a];
	}
	for(int i=0; i<name_size; i++){
		if(song_name[i]==' '){
			int j=i;
			while(j<name_size){
				song_name[j]=' ';
				j++;
			}break;
		}
	}
	if(song_name[0]=='/'){ //  '/'지우기
		for(int i=1; i<name_size; i++){// 만약 /가 앞에 있으면 배열을 앞으로 한칸 씩 이동
			song_name[i-1]=song_name[i];
		}
	}
}
void J_Speaker_init(){ //DFR, 인터럽트, 타이머카운터, 기본소리, 입력포트, I2C, 시작 표시
	DFR0781_init();
	//UART0_init(); /디버깅용
	interrupt_init();
	timer_counter_init();
	DFR_Tx_Data("AT+CA20\r\n"); //기본 소리 20
	_delay_ms(40);
	DDRE = 0x00;
	i2c_init(); //I2C초기화 및 관련 포트 활성화
	i2c_lcd_init(); //LCD 초기화 
	i2c_lcd_goto_XY(0,3);
	i2c_lcd_write_string("*J_Speaker*");
	i2c_lcd_goto_XY(1,0);
	i2c_lcd_write_string("begin...");
	_delay_ms(1500);//시작화면 딜레이
	i2c_lcd_command(0x01); //화면 초기화
	Mode_change(Mode);//기본 모드 BLE모드로 시작
	DDRF = 0x01;  //릴레이 핀 초기화
}
void changg_hz_arr(unsigned char *hz_arr){
	for(int i=0; i<name_size; i++){
		if(hz_arr[i]==0){
			hz_arr[i] = hz_arr[i-1];
			hz_arr[i-1]='.';
			break;
		} 
	}
}
void clearn_hz_arr(void){
	for(int i=0; i<name_size; i++){
		hz_arr[i]=0;
		}
}

int main(void)
{
	J_Speaker_init();
	  // PORTF = 0x01;

	//DFR_Tx_Data("AT+CG00\r\n");  //USB모드 일때 BLE사용 못하게 도록 설정 
	//한번만 설정하면 저장됨 모듈을 초기화 시키지 않으면 다시 사용할 필요 없음
	//DFR_Tx_Data("AT+BDJSpeaker\r\n");//BLE 오디오 이름 변경
	//DFR_Tx_Data("AT+BMJSpeaker\r\n");//BLE 이름 변경
	//DFR_Tx_Data("AT+MF\r\n"); //????
	//read_Songname(song_name); //????

    while (1) 
    {
		
		while(Mode==1){//USB모드 일때
					DFR_Tx_Data("AT+MF\r\n"); //????
					read_Songname(song_name); //????
					DFR_Tx_Data("AT+MF\r\n"); //현재 노래 이름 반환 요청
					read_Songname(song_name); //노래이름을 받아서 song_name배열에 저장
					Cleanup_name(); //처음 받아온 이름 정리
					i2c_lcd_goto_XY(1,0);//커서 위치 설정
					i2c_lcd_write_string(song_name);//이름 출력
					_delay_ms(500);
			}
    }
}

