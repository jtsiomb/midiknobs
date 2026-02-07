/*
midiknobs - simple MIDI controller
Copyright (C) 2026  John Tsiombikas <nuclear@mutantstargoat.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "midi.h"
#include "serial.h"

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

#define TIMER_INTR_HZ		10
#define TIMER_CNT			(F_CPU / 8 / TIMER_INTR_HZ)
static int8_t rotacc;
static uint8_t rot_pending;

#define CC_NUM_BASE		20
#define CC_NUM_ROT		(CC_NUM_BASE + NUM_VAL)
#define CC_NUM_BN0		(CC_NUM_ROT + 1)
#define CC_TOTAL		(NUM_VAL + 3)

#ifdef DEBUG_ENABLE
uint8_t dbgmode;
#endif

static uint8_t ctrlno[CC_TOTAL];
static uint8_t alt_map;
static int alt_curprog;

uint8_t filter_adc(uint8_t idx);

int main(void)
{
	uint8_t i, chan, bnstate, bndiff, prev_bnstate = 0;
	int8_t cur_rot;

	DDRC = 0x20;
	DDRB = 0x80;
	DDRD = 0;
	/* enable pull-ups */
	PORTC = 0xff;
	PORTB = 0xff;
	PORTD = 0xff;

#ifdef DEBUG_ENABLE
	if((PINC & 0x20) == 0) {
		/* debug mode */
		dbgmode = 1;
		init_serial(0, 9600, 8, PAR_NONE, 1);
		printf("midiknobs debug mode...\n");
	} else {
#endif
		/* MIDI mode */
		init_serial(0, 31250, 8, PAR_NONE, 1);
#ifdef DEBUG_ENABLE
	}
#endif

	/* both buttons pressed at start, alt CC mapping */
	if(((PIND >> 4) & 3) == 0) {
		ctrlno[0] = 1;		/* modulation */
		ctrlno[1] = 91;		/* reverb */
		ctrlno[3] = 64;		/* damper/sustain */
		ctrlno[4] = 65;		/* portamento */
		alt_map = 1;
	} else {
		for(i=0; i<CC_TOTAL; i++) {
			ctrlno[i] = CC_NUM_BASE + i;
		}
	}

	PCICR |= 1 << PCIE2;	/* enable pin change interrupt 2 (PCINT16..23) */
	/* unmask pin change intr for D2,D3 */
	PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);

#ifdef USE_ADC
	DIDR0 |= 3;	/* disable digital input for ADC0,ADC1 pins */

	/* enable ADC, interrupts, prescaler=64 (125khz) */
	ADCSRA = (1 << ADEN) | (1 << ADIE) | 5;
	/* left-adjusted, AVCC ref, ADC0 */
	ADMUX = (1 << ADLAR) | (1 << REFS0);

	/* kick off the ADC conversions */
	ADCSRA |= 1 << ADSC;

	/* timer setup: Clear Timer on Compare (CTC) mode, div8 clock */
	TCCR1B |= (1 << WGM12) | (1 << CS11);
	OCR1AH = TIMER_CNT >> 8;
	OCR1AL = TIMER_CNT & 0xff;
	TIMSK1 |= 1 << OCIE1A;

	sei();

	/* wait for the initial ADC conversions to happen, and the rolling average
	 * to stabilize, and send out the initial positions of the pots
	 */
	while(nconv < NUM_VAL * AVGCOUNT);

	chan = ~PINB & 0xf;
	for(i=0; i<NUM_VAL; i++) {
		uint8_t val = filter_adc(i) >> 1;
		adc[i].prev_val = val;
		midi_value(chan, ctrlno[i], val);
	}
#endif

	for(;;) {
		chan = ~PINB & 0xf;

#ifdef USE_ADC
		for(i=0; i<NUM_VAL; i++) {
			uint8_t val = filter_adc(i) >> 1;
			if(val != adc[i].prev_val) {
				midi_value(chan, ctrlno[i], val);
				adc[i].prev_val = val;
			}
		}
#endif

		if(rot_pending) {
			cli();
			cur_rot = rotacc;
			rotacc = 0;
			sei();

			if(cur_rot == 2) cur_rot = 1;
			else if(cur_rot == -2) cur_rot = -1;

			if(alt_map) {
				if(cur_rot) {
					alt_curprog += cur_rot;
					if(alt_curprog < 0) {
						alt_curprog = 0;
					} else if(alt_curprog > 127) {
						alt_curprog = 127;
					}
					midi_progchg(chan, alt_curprog);
				}
				rot_pending = 0;
			} else {
				if(cur_rot) {
					midi_value(chan, CC_NUM_ROT, cur_rot + 64);
				} else {
					midi_value(chan, CC_NUM_ROT, 0);
					rot_pending = 0;
				}
			}
		}

		bnstate = ~PIND >> 4;
		bndiff = bnstate ^ prev_bnstate;
		prev_bnstate = bnstate;

		for(i=0; i<4; i++) {
			if(bndiff & 1) {
				midi_value(chan, ctrlno[CC_NUM_BN0 - CC_NUM_BASE], bnstate & 1 ? 127 : 0);
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

	/* change the input and start the next conversion */
	ADMUX = (ADMUX & 0xf0) | next;
	ADCSRA |= 1 << ADSC;

	nconv++;
}
#endif

/* prev		inc		dec
 * 00		01		10
 * 01		11		00
 * 11		10		01
 * 10		00		11
 */
static const int8_t dir[] = {
	0,		/* 00 -> 00 */
	1,		/* 01 -> 00 */
	-1,		/* 10 -> 00 */
	0,		/* 11 -> 00 (invalid) */
	-1,		/* 00 -> 01 */
	0,		/* 01 -> 01 */
	0,		/* 10 -> 01 (invalid) */
	1,		/* 11 -> 01 */
	1,		/* 00 -> 10 */
	0,		/* 01 -> 10 (invalid) */
	0,		/* 10 -> 10 */
	-1,		/* 11 -> 10 */
	0,		/* 00 -> 11 (invalid) */
	-1,		/* 01 -> 11 */
	1,		/* 10 -> 11 */
	0		/* 11 -> 11 */
};

ISR(PCINT2_vect)
{
	static uint8_t state;
	int8_t delta;

	state = (state >> 2) | (PIND & 0xc);
	delta = dir[state];

	if(delta > 0) {
		if(rotacc < 63) rotacc++;
	} else if(delta < 0) {
		if(rotacc > -63) rotacc--;
	}
}

ISR(TIMER1_COMPA_vect)
{
	/* it's time to check the rotary value and transmit */
	if(rotacc) {
		rot_pending = 1;
	}
}
