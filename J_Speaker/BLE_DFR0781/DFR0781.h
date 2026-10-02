/*
 * DFR0781.h
 *
 * Created: 2024-07-21 오후 4:54:28
 *  Author: JJH
 */ 


#ifndef DFR0781_H_
#define DFR0781_H_

void DFR0781_init(void);
void tx_1(char data);
char rx_1();
void DFR_Tx_Data(char *str);
void rx_1_string(unsigned char *buffer);



#endif /* DFR0781_H_ */