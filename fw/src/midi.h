#ifndef MIDI_H_
#define MIDI_H_

#define MIDI_CMD_NOTEON 0x90
#define MIDI_CMD_NOTEOFF 0x80
#define MIDI_CMD_CHANMSG 0xb0
#define MIDI_CHANMSG_NOTES_OFF 0x7b
#define MIDI_CMD_CC 0xb0

void midi_all_off(void);
void midi_note(int chan, int note, int vel);
void midi_value(int chan, int ctlnum, int val);

#endif /* MIDI_H_ */
