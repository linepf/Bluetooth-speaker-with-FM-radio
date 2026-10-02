/*
 * I2C.c
 *
 * Created: 2024-07-22 오후 2:44:33
 *  Author: JJH
 */ 
#include "I2C/I2C.h"
#include <avr/io.h>

void i2c_start(uint8_t address, int hz, int RW)
{//이 함수 사용시 보낼 기기의 데이터와 주소 필요
	// RW가 0이면 쓰기 / RW가 1이면 읽기
	TWBR = hz;
	TWCR = (1 << TWINT) | (1<<TWSTA) | (1<<TWEN); //TWINT의 비트를 1로 세팅하면 TWINT의 비트가 0이 된다.
	//TWSTA : start 비트 1로 set, TWEN : TWI 활성화
	while(!(TWCR & (1<<TWINT))); //위에서 1이 되었던 TWINT 비트가 1이 될때 까지 반복문을 돌면서 대기
	//start 비트가 보내지면 TWINT비트가 1이 된다.
	if(RW == 0){ address = address | 0x00;} // RW가 0이면 쓰기
	if(RW == 1){ address = address | 0x01; } // RW가 1이면 읽기
	TWDR = address; //위에서 start비트를 보냈으므로 주소와 읽기/쓰기 비트 전송
	TWCR = (1<<TWINT) | (1<<TWEN); //위에서 start비트를 보냈으므로 start비트는 제외하고 데이터 전송
	while(!(TWCR & (1<<TWINT))); //위에서 1이 되었던 TWINT 비트가 1이 될때 까지 반복문을 돌면서 대기
	//ack펄스가 올때 까지 대기 ack펄스가 오면 TWINT 를 1로 셋
}
void i2c_stop(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	//스탑비트 보내기 위해 TWINT를 1, TWEN TWI활성화, TWSTO stop비트 전송
}
void i2c_init(void)
{
	DDRD = 0x03; //I2C를 사용하기 위해서 SCL, SCA포트 활성화
	PORTD |= (1 << 0) | (1 << 1);  //I2C를 사용하기 위해서 SCL, SCA포트 활성화
}
void i2c_transmit(uint8_t data) //1byte
{
	TWDR = data; //데이전송
	TWCR = (1<<TWINT) | (1<<TWEN); //데이터 전송
	while(!(TWCR & (1<<TWINT))); //데이터 전송후 ack펄스가 올때 까지 대기 ack펄스가 오면 TWINT 를 1로 셋
}

