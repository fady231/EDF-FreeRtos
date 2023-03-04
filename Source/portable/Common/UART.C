




void uart_init(int baud_rate)
{	
UCSRA=0b00000010;
UCSRB=0b00011000;
UCSRC=0b10000110;
int temp= (F_CPU/2*baud_rate)-1;
UBRRH=(temp>>8)&0B01111111;
UBRRL=(char)temp;

}

void uart_send_char(char c)
{
	
	while(UCSRC>>UDRE&1);
	 UDR=C;
}
char uart_receive_char(void)
{
	while(UCSRC>>RXC&1);
	return UDR;
}
void uart_send_string(char* c)
{
	do
	{
		uart_send_char(*c);
		c++;
	}
	while(*c!='\0'&&*c!='\n'&&*c!='\r');
	uart_send_char(*c);
	
}
void uart_receive_string(char* c)
{
	do
	{
		*c=uart_receive_char();
		c++;
	}
	while(*c!='\0'&&*c!='\n'&&*c!='\r');
	*c=uart_receive_char();
}