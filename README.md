midiknobs - MIDI controller
===========================
midiknobs is a simple open hardware/free firmware MIDI controller.

By the nature of all the hardware designs and firmware being free, it can be
modified to include more knobs, encoders and/or buttons, but as it stands, it
supports 4 potentiometers, 1 rotary encoder, and 4 buttons.

By default it maps all those controls to the undefined controller range: 20-29,
so that they can be mapped by MIDI software to whatever useful operation the
user desires. In this default mode the rotary encoder sends relative events,
less than 64 meaning negative, greater than 64 meaning positive, with velocity
sensing.

If the first two buttons are held down while turning the device on, it starts in
an alternate mapping mode, with the following mappings (can be modified by
changing the source code):

 - pot 1: CC 1 - modulation wheel
 - pot 2: CC 91 - reverb
 - button 1: CC 64 - damper/sustain
 - button 2: CC 65 - portamento
 - rotary encoder: program (instrument) change up/down

Project structure
-----------------
 - hw: KiCAD hardware design files.
 - fw: firmware source code.
 - case: OpenSCAD 3D printable case design.

License
-------
Copyright (C) 2026 John Tsiombikas <nuclear@mutantstargoat.com>

This project is free hardware/software. Feel free to use, modify, and/or
redistribute, under the terms of the GNU General Public License v3, or at your
option, any later version published by the Free Software Foundation. See COPYING
for details.

Firmware
--------
To build and install the firmware you'll need the avr-gcc toolchain, and avrdude
for programming the microcontroller. You'll also need one of the AVR programmers
supported by avrdude. I'm using USB TinyISP and that's what the makefile
expects, so make sure to modify the makefile accordingly (all references to
"usbtiny"), for the programmer you have. See `avrdude -c ?` for a list.

With everything set up and connected, run `make` to compile the firmware and
`make program` to upload it to the microcontroller.
