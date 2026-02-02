#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "midi.h"
#include "serial.h"

uint8_t dbgmode;

#define USE_ADC

#ifdef USE_ADC
#define NUM_VAL		2
#define AVGCOUNT	64

struct adc {
	uint8_t value, prev_val;
	uint8_t raw[AVGCOUNT];
	uint8_t head;
} adc[NUM_VAL];
uint8_t cur_adc;
volatile uint8_t nconv;
#endif

#define CC_NUM_BASE		20
#define CC_NUM_BN0		(CC_NUM_BASE + NUM_VAL + 1)

uint8_t filter_adc(uint8_t idx);

int main(void)
{
	uint8_t i, chan, bnstate, bndiff, prev_bnstate = 0;

	DDRC = 0;
	DDRB = 0x80;
	DDRD = 0;
	/* enable pull-ups */
	PORTC = 0xff;
	PORTB = 0xff;
	PORTD = 0xff;

	if((PINC & 0x20) == 0) {
		/* debug mode */
		dbgmode = 1;
		init_serial(0, 9600, 8, PAR_NONE, 1);
		printf("midiknobs debug mode...\n");
	} else {
		/* MIDI mode */
		init_serial(0, 31250, 8, PAR_NONE, 1);
	}

#ifdef USE_ADC
	DIDR0 |= 3;	/* disable digital input for ADC0,ADC1 pins */

	/* enable ADC, interrupts, prescaler=64 (125khz) */
	ADCSRA = (1 << ADEN) | (1 << ADIE) | 5;
	/* left-adjusted, AVCC ref, ADC0 */
	ADMUX = (1 << ADLAR) | (1 << REFS0);

	/* kick off the ADC conversions */
	ADCSRA |= 1 << ADSC;

	sei();

	/* wait for the initial ADC conversions to happen, and the rolling average
	 * to stabilize, and send out the initial positions of the pots
	 */
	while(nconv < NUM_VAL * AVGCOUNT);

	chan = ~PINB & 0xf;
	for(i=0; i<NUM_VAL; i++) {
		uint8_t val = filter_adc(i) >> 1;
		adc[i].prev_val = val;
		midi_value(chan, CC_NUM_BASE + i, val);
	}
#endif

	for(;;) {
		chan = ~PINB & 0xf;

#ifdef USE_ADC
		for(i=0; i<NUM_VAL; i++) {
			uint8_t val = filter_adc(i) >> 1;
			if(val != adc[i].prev_val) {
				midi_value(chan, CC_NUM_BASE + i, val);
				adc[i].prev_val = val;
			}
		}
#endif

		/* TODO rotary */

		bnstate = ~PIND >> 4;
		bndiff = bnstate ^ prev_bnstate;
		prev_bnstate = bnstate;

		for(i=0; i<4; i++) {
			if(bndiff & 1) {
				midi_value(chan, CC_NUM_BN0 + i, bnstate & 1 ? 127 : 0);
			}
			bnstate >>= 1;
			bndiff >>= 1;
		}
	}
}

uint8_t filter_adc(uint8_t idx)
{
	uint8_t i;
	unsigned int sum;

	sum = 0;
	cli();
	for(i=0; i<AVGCOUNT; i++) {
		sum += adc[idx].raw[i];
	}
	sei();
	return sum / AVGCOUNT;
}

#ifdef USE_ADC
ISR(ADC_vect)
{
	uint8_t cur_adc, next;
	struct adc *cur;

	cur_adc = nconv & (NUM_VAL - 1);
	cur = adc + cur_adc;

	cur->raw[cur->head] = ADCH;
	cur->head = (cur->head + 1) & (AVGCOUNT - 1);

	next = (cur_adc + 1) & (NUM_VAL - 1);

	/* to measure conversion frequency */
	PORTB = (PORTB & 0x7f) | (next << 7);

	/* change the input and start the next conversion */
	ADMUX = (ADMUX & 0xf0) | next;
	ADCSRA |= 1 << ADSC;

	nconv++;
}
#endif
