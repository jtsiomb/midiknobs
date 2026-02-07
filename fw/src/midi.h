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
#ifndef MIDI_H_
#define MIDI_H_

#define MIDI_CMD_NOTEON 0x90
#define MIDI_CMD_NOTEOFF 0x80
#define MIDI_CMD_CHANMSG 0xb0
#define MIDI_CHANMSG_NOTES_OFF 0x7b
#define MIDI_CMD_CC 0xb0
#define MIDI_CMD_PROGCHG 0xc0

void midi_all_off(void);
void midi_note(int chan, int note, int vel);
void midi_value(int chan, int ctlnum, int val);
void midi_progchg(int chan, int prog);

#endif /* MIDI_H_ */
