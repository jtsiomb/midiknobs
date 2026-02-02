#include <stdio.h>
#include <stdint.h>
#include "midi.h"
#include "serial.h"

#define UART	0

extern uint8_t dbgmode;

void midi_all_off(void)
{
	uart_write(UART, MIDI_CMD_CHANMSG);
	uart_write(UART, MIDI_CHANMSG_NOTES_OFF);
}

void midi_note(int chan, int note, int vel)
{
	if(vel > 0) {
		uart_write(UART, MIDI_CMD_NOTEON | chan);
	} else {
		uart_write(UART, MIDI_CMD_NOTEOFF | chan);
	}
	uart_write(UART, note);
	uart_write(UART, vel & 0x7f);
}

void midi_value(int chan, int ctlnum, int val)
{
	if(dbgmode) {
		val &= 0x7f;
		printf("%2x %2x %2x\t[%d] CC(%d): %d\n", MIDI_CMD_CC | chan, ctlnum, val,
				chan, ctlnum, val);
	} else {
		uart_write(UART, MIDI_CMD_CC | chan);
		uart_write(UART, ctlnum);
		uart_write(UART, val & 0x7f);
	}
}
