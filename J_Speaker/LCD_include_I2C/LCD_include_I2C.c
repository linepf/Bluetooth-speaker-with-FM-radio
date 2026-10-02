/*
 * LCD_include_I2C_.c
 *
 * Created: 2024-07-22 오후 3:03:32
 *  Author: JJH
 */ 

/*
lcd 1602a를 사용하는 코드이다
이 LCD는 PCF8574이라는 I2C IC를 사용하여 통신 한다.
주소는 아래에 명시되어 있으며 속도는 100KHZ를 사용한다
LCD_include_I2C.h를 사용하려면 "I2C/I2C.h"를 include 해야한다
*/
#define F_CPU 16000000UL
#include "LCD_include_I2C/LCD_include_I2C.h"
#include "I2C/I2C.h"
#include <avr/io.h>
#include <util/delay.h>

#define LCD_PCF8574_Address  (0x27 << 1)  //읽기/쓰기 비트를 같이 보내야 하므로 1비트 쉬프트 시킨다
//R/W : AVR기준 0이면 쓰기, 1이면 읽기, 이번 실습에서는 쓰기만 사용
#define speed 72 //LCD는 100KHz를 사용한다.
//RS1 : 데이터 RS0 : 명령어   /  EN1 : 데이터 전송  EN0 : 데이터 전송 안함
#define RS1_EN1 0x05
#define RS1_EN0 0x01
#define RS0_EN1 0x04
#define RS0_EN0 0x00
#define Backlight 0x08  //Backlight on

void i2c_lcd_init(void) {
	
	_delay_ms(50);
	i2c_lcd_command_4bit(0x30); _delay_ms(15);
	i2c_lcd_command_4bit(0x30); _delay_ms(5);
	i2c_lcd_command_4bit(0x30); _delay_us(100);
	i2c_lcd_command_4bit(0x20); _delay_us(50);
	/*LCD 초기화 포맷(양식)이 위와 같은 4bit 데이터를 4번 전선한 후에 아래와 같은 8비트 명령어를 
	  보내서 LCD를 포맷한다 */

	i2c_lcd_command(0x28); _delay_us(50);
	i2c_lcd_command(0x0c); _delay_us(50); // display on/off control
											//글자보임, 커서 안보임, 커서 안깜빡임
	i2c_lcd_command(0x01); _delay_ms(3); // clear display
	i2c_lcd_command(0x06); _delay_ms(50);
}

void i2c_lcd_command_4bit(uint8_t command) //4bit 명령어 
{
	uint8_t c_buf[2];
	
	c_buf[0] = (command&0xf0) | RS0_EN1 | Backlight;  //명령어 전송, 데이터의 시작을 알리기 위해 EN1, Backlight on
	c_buf[1] = (command&0XF0) | RS0_EN0 | Backlight; //명령어 전송, 데이터의 끝을 알리기 위해 EN0, Backlight on
													//데이터의 시작과 끝을 명확하게 하기 위해서 EN비트를 사용하고 같은 내용을 2번 보낸다.
	i2c_start(LCD_PCF8574_Address,speed,0); //start비트 전송및 주소전송
	for(int i = 0; i < 2; i++)
	{
		i2c_transmit(c_buf[i]);
		_delay_ms(1);
	}
	i2c_stop();  //stop비트 전송
}

void i2c_lcd_command(uint8_t command) //8bit 명령어
{
	uint8_t c_buf[4];
	
	c_buf[0] = (command&0xf0) | RS0_EN1 | Backlight;
	c_buf[1] = (command&0XF0) | RS0_EN0 | Backlight;
	c_buf[2] = ((command<<4)&0XF0) | RS0_EN1 |Backlight;
	c_buf[3] = ((command<<4)&0XF0) | RS0_EN0 |Backlight;
	
	i2c_start(LCD_PCF8574_Address,speed,0);
	for(int i = 0; i < 4; i++)
	{
		i2c_transmit(c_buf[i]);
		_delay_ms(1);
	}
	i2c_stop();
}
void i2c_lcd_data(uint8_t data) {  //8bit 데이터
	uint8_t d_buf[4];

	d_buf[0] = (data & 0xF0) | RS1_EN1 | Backlight;
	d_buf[1] = (data & 0xF0) | RS1_EN0 | Backlight;
	d_buf[2] = ((data << 4) & 0xF0) | RS1_EN1 | Backlight;
	d_buf[3] = ((data << 4) & 0xF0) | RS1_EN0 | Backlight;

	i2c_start(LCD_PCF8574_Address,speed,0);
	for(int i = 0; i < 4; i++)
	{
		i2c_transmit(d_buf[i]);
		_delay_ms(1);
	}
	i2c_stop();
}

void i2c_lcd_write_string(char *string) { // 문자열을 함수 i2c_lcd_data로 전송
	while (*string) {
		i2c_lcd_data(*string++);
	}
}
void i2c_lcd_goto_XY(uint8_t row, uint8_t col)
{
	uint8_t address = (0x40 * row) + col;
	uint8_t command = 0x80 | address;
	
	i2c_lcd_command(command);
}