#include "settings.h"

#include <avr/eeprom.h>

#define NUM_REGISTERS 6

#define EEPROM_SIZE_ATMEGA328 1024  

#define MAGIC_CODE  0x554B
#define VERSION     0x0002

#define MASK_KEY      0x00FF
#define MASK_MOD      0xFF00

#define REGISTER_MAGIC    0x00
#define REGISTER_VERSION  0x01
#define REGISTER_CCW      0x02
#define REGISTER_CW       0x03
#define REGISTER_BTN      0x04
#define REGISTER_TYPE     0x05

uint8_t var_size;
uint8_t buffer_len;
uint8_t addr_status_buffer;

uint16_t settings[NUM_REGISTERS] = {
  MAGIC_CODE,
  VERSION,
  0x00EA, // CCW, no modifiers, Volume Down
  0x00E9, // CW, no modifiers, Volume Up
  0x00E2, // BTN, no modifiers, Mute
  0x0015, // CCW => MM, CW => MM, BTN => MM

  // 0x0056, // CCW, no modifiers, Keypad -
  // 0x0057, // CW, no modifiers, Keypad +
  // 0x0055, // BTN, no modifiers, Keypad =
  // 0x0000, // CCW => KB, CW => KB, BTN => KB
};

uint16_t _settingsFindNextWriteIndex() { 
  uint16_t i;
  for (i = addr_status_buffer; i < (buffer_len + addr_status_buffer); i++) {
      uint16_t prev_index;
    
      // Wrap around case
      if (i == addr_status_buffer) {
          prev_index = addr_status_buffer + buffer_len - 1;
      } else {
          prev_index = i - 1;
      }

      uint8_t prev_elem = eeprom_read_byte((uint8_t*)prev_index);
      uint8_t curr_elem = eeprom_read_byte((uint8_t*)i);

      // Must truncate the addition because the index tracking relies of wrap around
      if (((prev_elem + 1) & 0xFF) != curr_elem) {
          return i - addr_status_buffer;
      }
  }

  return 0;
}

void _settingsLoad() {
  uint16_t write_offset = _settingsFindNextWriteIndex() * var_size;
  uint16_t addr_read;

  addr_read = write_offset - var_size;
  if (write_offset == 0) {
    addr_read = addr_status_buffer - var_size;
  }

  uint16_t magic_code = eeprom_read_word((uint16_t*)addr_read);
  uint16_t version = eeprom_read_word((uint16_t*)(addr_read + 0x02));

  if (magic_code == settings[REGISTER_MAGIC] && version == settings[REGISTER_VERSION]) {
    uint8_t i;
    for (i = 0; i < NUM_REGISTERS; i++) {
      eeprom_read_block((void*)&settings[i], (const void*)(addr_read + i * sizeof(uint16_t)), sizeof(settings[i]));
    }
  }
}

void _settingsSave() {
  uint16_t write_offset = _settingsFindNextWriteIndex();
  
  uint8_t reg;
  for (reg = 0; reg < NUM_REGISTERS; reg++) {
    eeprom_update_word((uint16_t*)((write_offset * var_size) + (reg * sizeof(uint16_t))), settings[reg]);
  }

  // Update status buffer 
  uint16_t curr_index = addr_status_buffer + write_offset;
  uint16_t prev_index;

  // Wrap around case
  if (curr_index == addr_status_buffer) {
    prev_index = addr_status_buffer + buffer_len - 1;
  } else {
    prev_index = curr_index - 1;
  }

  uint8_t sb_val = eeprom_read_byte((uint8_t*)prev_index) + 1;
  eeprom_update_byte((uint8_t*)curr_index, sb_val);
}

void settingsInit() {
    var_size           = ((NUM_REGISTERS << 1) | 1);
    buffer_len         = (EEPROM_SIZE_ATMEGA328 / var_size);
    addr_status_buffer = EEPROM_SIZE_ATMEGA328 - buffer_len;

    _settingsLoad();
}

uint8_t settingsGetKeycode(uint8_t reg) {
  if (reg < SETTINGS_CCW || reg > SETTINGS_BTN) {
    return 0;
  }

  return settings[reg] & MASK_KEY;
}

uint8_t settingsGetModifiers(uint8_t reg) {
  if (reg < SETTINGS_CCW || reg > SETTINGS_BTN) {
    return 0;
  }

  return (settings[reg] & MASK_MOD) >> 8;
}

void settingsSetKeycode(uint8_t reg, uint8_t keycode) {
  if (reg < SETTINGS_CCW || reg > SETTINGS_BTN) {
    return;
  }

  settings[reg] &= 0xFF00; // clear out old values
  settings[reg] |= keycode;

  _settingsSave();
}

void settingsSetModifiers(uint8_t reg, uint8_t modifiers) {
  if (reg < SETTINGS_CCW || reg > SETTINGS_BTN) {
    return;
  }

  settings[reg] &= 0x00FF; // clear out old values
  settings[reg] |= (modifiers << 8);

  _settingsSave();
}

uint8_t settingsGetType(uint8_t reg) {
  if (reg < SETTINGS_CCW || reg > SETTINGS_BTN) {
    return 0;
  }

  uint16_t val = settings[REGISTER_TYPE];

  if (reg == SETTINGS_CCW) {
    val &= 0x03;
    return val;
  }

  if (reg == SETTINGS_CW) {
    val &= 0x0C;
    return val >> 2;
  }

  if (reg == SETTINGS_BTN) {
    val &= 0x30;
    return val >> 4;
  }

  return 0;
}

void settingsSetType(uint8_t reg, uint8_t type) {
  if (reg < SETTINGS_CCW || reg > SETTINGS_BTN) {
    return;
  }

  if (reg == SETTINGS_CCW) {
    settings[REGISTER_TYPE] &= 0x3C;
    settings[REGISTER_TYPE] |= (type & 0x03);
  } 
  
  if (reg == SETTINGS_CW) {
    settings[REGISTER_TYPE] &= 0x33;
    settings[REGISTER_TYPE] |= ((type & 0x03) << 2);
  } 

  if (reg == SETTINGS_BTN) {
    settings[REGISTER_TYPE] &= 0x0F;
    settings[REGISTER_TYPE] |= ((type & 0x03) << 4);
  }
}