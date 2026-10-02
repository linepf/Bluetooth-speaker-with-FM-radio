/*
 * TEA5767.c
 *
 * Created: 2024-07-24 오후 5:03:12
 *  Author: JJH
 */ 
#define F_CPU 16000000UL
#include "TEA5767/TEA5767.h"
#include "I2C/I2C.h"
#include <util/delay.h>
#include <stdio.h>
#include <avr/io.h>
//(76MHzto108MHz)

	 

#define LCD_PCF8574_Address  (0x60 << 1)  //읽기/쓰기 비트를 같이 보내야 하므로 1비트 쉬프트 시킨다
//  R/W : AVR기준 0이면 쓰기, 1이면 읽기
#define TEA5767_speed  12 //400KHz 

 uint8_t TEA5767_write_bytes[5] = {0};
 uint8_t TEA5767_read_data[5] = {0};
	 
	 
void TEA5767_write(unsigned long Hz)
	 {//직점 주파수를 입력받아 보내는 함수
		  TEA5767_write_bytes[0] = 0b00000000;        // NOT MUTE
		  TEA5767_write_bytes[1] = 0b00000000;                 // PLL Area
		  TEA5767_write_bytes[2] = 0b11001000;        // Search Up , Search level 중간, Low Side Injection ,   Mono 출력 , 좌측 오디오 출력, 우측 오디오 출력, --
		  TEA5767_write_bytes[3] = 0b00011110;        // No Standby, US/EU Band, 32768KHz, SoftMuteOn, HighCutControlOn, Stereo Noise Cancelling On, --
		  TEA5767_write_bytes[4] = 0b01000000;        //DTC 시정수 75us 시정수를 75로하면 소리는 조금 작아지지만 명확하게 들리는 느낌
		  
		   unsigned short n;
		   n = (unsigned long)4*(Hz*100000-225000)/32768; //Low Side Injection
		   TEA5767_write_bytes[0] = (TEA5767_write_bytes[0] & 0xC0) | (n >> 8);
		   TEA5767_write_bytes[1] = n & 0xFF;
		   
		 i2c_start(LCD_PCF8574_Address,TEA5767_speed,0);
		 
		 /* Data Write */
		 //데이터를 8비트씩 전송
		 for(int i=0; i<5; i++)
		 {
			 TWDR = TEA5767_write_bytes[i];
			 TWCR = (1<<TWINT)|(1<<TWEN);
			 while (!(TWCR & (1<<TWINT)));
		 }
		 
		i2c_stop();
}
	 
void TEA5767_search(){
	TEA5767_write_bytes[0] = 0b01000000;        // Search!!
	//TEA5767_write_bytes[1] = 0b00000000;                 // PLL Area
	//TEA5767_write_bytes[2] = 0b11001000;        // Search Up , Search level 중간, Low Side Injection ,   Mono 출력 , 좌측 오디오 출력, 우측 오디오 출력, --
	//TEA5767_write_bytes[3] = 0b00011110;        // No Standby, US/EU Band, 32768KHz, SoftMuteOn, HighCutControlOn, Stereo Noise Cancelling On, --
	//TEA5767_write_bytes[4] = 0b01000000;
	
	 i2c_start(LCD_PCF8574_Address,TEA5767_speed,0);
	 
	 /* Data Write */
	 //데이터를 8비트씩 전송
	 for(int i=0; i<5; i++)
	 {
		 TWDR = TEA5767_write_bytes[i];
		 TWCR = (1<<TWINT)|(1<<TWEN);
		 while (!(TWCR & (1<<TWINT)));
	 }
	 
	 i2c_stop();
}
	 
void TEA5767_read()
{//데이터를 읽어서 배열에 저장하는 함수

///////////////////////////////////////////////////////////////////
	/* STEP1 START */
	/*TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	 //STEP2 Start check 
	while(1)
	{
		if((TWCR & 0x80) && ((TWSR & 0xf8) == 0x08))
		break;
	}
	
	//  STEP3 Address send 
	TWDR = LCD_PCF8574_Address | 0x01; // 시프트 시킨 주소에다가 1을 더해준다 읽기
	TWCR = (1<<TWINT)|(1<<TWEN);*/
//////////////////////////////////////////////////////////////////////////////

	 i2c_start(LCD_PCF8574_Address, TEA5767_speed,1);

///////////////////////////////////////
	
	/* STEP4 ACK Check */
	while(1)
	{
		if((TWCR & 0x80) && ((TWSR & 0xf8) == 0x40))
		break;
		if((TWCR & 0x80) && ((TWSR & 0xf8) == 0x48))
		return 0;
	}

	for(int i=0; i<5; i++)
	{
		/* STEP5 send ACK or NACK*/
		if((i+1)!=5)
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA); //ACK
		else
		TWCR = (1<<TWINT)|(1<<TWEN); //NACK
		/* STEP6 Data receive Check */
		while(1)
		{
			if((TWCR & 0x80) && ((TWSR & 0xf8) == 0x50))
			break;
			if((TWCR & 0x80) && ((TWSR & 0xf8) == 0x58))
			break;
		}
		/* STEP7 Receive Data*/
		TEA5767_read_data[i] = TWDR;
	}
	
	/* STEP8 STOP */
	i2c_stop();
}

