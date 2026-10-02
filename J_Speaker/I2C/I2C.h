/*
 * I2C.h
 *
 * Created: 2024-07-22 오후 2:44:41
 *  Author: JJH
 */ 


#ifndef I2C_H_
#define I2C_H_
#include <stdint.h>

void i2c_start(uint8_t address, int hz, int RW);
void i2c_stop(void);
void i2c_init(void);
void i2c_transmit(uint8_t data);

#endif /* I2C_H_ */