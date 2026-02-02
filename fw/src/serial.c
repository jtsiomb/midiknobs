#ifndef F_CPU
#ifdef XTAL
#define F_CPU	XTAL
#else
#warning "compiled for 1mhz internal rc osc. serial comms won't work"
#define F_CPU	1000000
#endif
#endif

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/power.h>

#define BUF_SZ	16
#define BUF_IDX_MASK	(BUF_SZ - 1)
#define NEXT_IDX(x)	(((x) + 1) & BUF_IDX_MASK)

struct uartdata {
	unsigned char inbuf[BUF_SZ];
	volatile unsigned char in_rd, in_wr;
} ubuf;

static int uart_send_char(char c, FILE *fp);
static int uart_get_char(FILE *fp);

static FILE std_stream = FDEV_SETUP_STREAM(uart_send_char, uart_get_char, _FDEV_SETUP_RW);

/* value for USART ctrl reg B: rx/tx enable & rx interrupt */
#define CTLB	((1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0))

void init_serial(int uidx, long baud, int bits, int par, int stop)
{
	static const unsigned int ucszbits[] = {3, 3, 3, 3, 3, 0, 1, 2, 3, 7};
	unsigned int ubrr_val = F_CPU / 16 / baud - 1;
	unsigned int ucsz = ucszbits[bits];
	unsigned int ffmt;

	ffmt = (ucsz & 3) << 1;
	if(par) {
		ffmt |= ((par & 1) | 2) << 4;
	}
	if(stop > 1) {
		ffmt |= 1 << USBS0;	/* 2 stop bits */
	}

	power_usart0_enable();
	UBRR0H = (unsigned char)(ubrr_val >> 8);
	UBRR0L = (unsigned char)ubrr_val;
	UCSR0B = CTLB | (ucsz & 4);
	UCSR0C = ffmt;

	ubuf.in_rd = ubuf.in_wr = 0;

	stdin = stdout = stderr = &std_stream;
}

int have_input(int uidx)
{
	return ubuf.in_wr != ubuf.in_rd;
}

static int uart_send_char(char c, FILE *fp)
{
	if(c == '\n') uart_send_char('\r', fp);

	while((UCSR0A & (1 << UDRE0)) == 0);
	UDR0 = (unsigned char)c;
	return 0;
}

static int uart_get_char(FILE *fp)
{
	char c;

	while(ubuf.in_rd == ubuf.in_wr);

	c = ubuf.inbuf[ubuf.in_rd];
	ubuf.in_rd = NEXT_IDX(ubuf.in_rd);
	return c;
}

int uart_write(int uidx, unsigned char c)
{
	while((UCSR0A & (1 << UDRE0)) == 0);
	UDR0 = (unsigned char)c;
	return 0;
}

int uart_read(int uidx)
{
	int c;

	while(ubuf.in_rd == ubuf.in_wr);

	c = ubuf.inbuf[ubuf.in_rd];
	ubuf.in_rd = NEXT_IDX(ubuf.in_rd);
	return c;
}

ISR(USART_RX_vect)
{
	ubuf.inbuf[ubuf.in_wr] = UDR0;
	ubuf.in_wr = NEXT_IDX(ubuf.in_wr);
}
