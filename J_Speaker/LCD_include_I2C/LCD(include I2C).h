/*
 * LCD_include_I2C_.h
 *
 * Created: 2024-07-22 오후 3:03:24
 *  Author: JJH
 */ 


#ifndef LCD(INCLUDE_I2C)_H_
#define LCD(INCLUDE_I2C)_H_
#include <stdint.h>

void i2c_lcd_init(void);
void i2c_lcd_command_4bit(uint8_t command);
void i2c_lcd_command(uint8_t command);
void i2c_lcd_data(uint8_t data);
void i2c_lcd_write_string(char *string);
void i2c_lcd_goto_XY(uint8_t row, uint8_t col);

#endif /* LCD(INCLUDE I2C)_H_ */