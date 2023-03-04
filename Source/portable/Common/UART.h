#ifndef UART_H
#define UART_H

void uart_init(int baud_rate);


void uart_send_char(char c);

char uart_receive_char(void);

void uart_send_string(char* c);

void uart_receive_string(char* c);






#endif