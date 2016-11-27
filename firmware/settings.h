#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <stdint.h>

#define SETTINGS_CCW	0x02
#define SETTINGS_CW 	0x03
#define SETTINGS_BTN	0x04

#define TYPE_KEYBOARD	0x00
#define TYPE_MM			0x01

// Multimedia Keys
#define MMKEY_KB_VOL_UP			0x80 // do not use
#define MMKEY_KB_VOL_DOWN		0x81 // do not use
#define MMKEY_VOL_UP			0xE9
#define MMKEY_VOL_DOWN			0xEA
#define MMKEY_SCAN_NEXT_TRACK	0xB5
#define MMKEY_SCAN_PREV_TRACK	0xB6
#define MMKEY_STOP				0xB7
#define MMKEY_PLAYPAUSE			0xCD
#define MMKEY_MUTE				0xE2
#define MMKEY_BASSBOOST			0xE5
#define MMKEY_LOUDNESS			0xE7
#define MMKEY_KB_EXECUTE		0x74
#define MMKEY_KB_HELP			0x75
#define MMKEY_KB_MENU			0x76
#define MMKEY_KB_SELECT			0x77
#define MMKEY_KB_STOP			0x78
#define MMKEY_KB_AGAIN			0x79
#define MMKEY_KB_UNDO			0x7A
#define MMKEY_KB_CUT			0x7B
#define MMKEY_KB_COPY			0x7C
#define MMKEY_KB_PASTE			0x7D
#define MMKEY_KB_FIND			0x7E
#define MMKEY_KB_MUTE			0x7F // do not use

void settingsInit(void);

uint8_t settingsGetKeycode(uint8_t type);
void    settingsSetKeycode(uint8_t type, uint8_t keycode);

uint8_t settingsGetModifiers(uint8_t type);
void    settingsSetModifiers(uint8_t type, uint8_t modifiers);

uint8_t settingsGetType(uint8_t reg);
void 	settingsSetType(uint8_t reg, uint8_t type);

#endif // __SETTINGS_H__
