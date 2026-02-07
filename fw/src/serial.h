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
#ifndef SERIAL_H_
#define SERIAL_H_

enum { PAR_NONE, PAR_ODD, PAR_EVEN };

void init_serial(int uidx, long baud, int bits, int par, int stop);
int have_input(int uidx);

int uart_write(int uidx, unsigned char c);
int uart_read(int uidx);


#endif	/* SERIAL_H_ */
