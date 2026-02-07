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
#include <stdint.h>
#include "midi.h"
#include "serial.h"

#define UART	0

#ifdef DEBUG_ENABLE
extern uint8_t dbgmode;
#endif

static uint8_t prev_st;

void midi_all_off(void)
{
	uart_write(UART, MIDI_CMD_CHANMSG);
	uart_write(UART, MIDI_CHANMSG_NOTES_OFF);
}

void midi_note(int chan, int note, int vel)
{
	uint8_t st = MIDI_CMD_NOTEON | chan;
	if(prev_st != st) {
		prev_st = st;
		uart_write(UART, MIDI_CMD_NOTEON | chan);
	}
	uart_write(UART, note);
	uart_write(UART, vel & 0x7f);
}

void midi_value(int chan, int ctlnum, int val)
{
#ifdef DEBUG_ENABLE
	if(dbgmode) {
		val &= 0x7f;
		printf("%2x %2x %2x\t[%d] CC(%d): %d\n", MIDI_CMD_CC | chan, ctlnum, val,
				chan, ctlnum, val);
	} else {
#endif
		uint8_t st = MIDI_CMD_CC | chan;
		if(prev_st != st) {
			prev_st = st;
			uart_write(UART, st);
		}
		uart_write(UART, ctlnum);
		uart_write(UART, val & 0x7f);
#ifdef DEBUG_ENABLE
	}
#endif
}

void midi_progchg(int chan, int prog)
{
	uint8_t st = MIDI_CMD_PROGCHG | chan;
	if(prev_st != st) {
		prev_st = st;
		uart_write(UART, st);
	}
	uart_write(UART, prog & 0x7f);
}
