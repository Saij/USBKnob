#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <stdint.h>

#define REGISTER_MAGIC    0x00
#define REGISTER_VERSION  0x01
#define REGISTER_CCW      0x02
#define REGISTER_CW       0x03
#define REGISTER_BTN      0x04

void settingsInit(void);

uint8_t settingsGetKeycode(uint8_t reg);
void    settingsSetKeycode(uint8_t reg, uint8_t keycode);

uint8_t settingsGetModifiers(uint8_t reg);
void    settingsSetModifiers(uint8_t reg, uint8_t modifiers);

#endif // __SETTINGS_H__
