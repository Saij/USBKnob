#include "settings.h"

#include <avr/eeprom.h>

#define NUM_REGISTERS 5

#define EEPROM_SIZE_ATMEGA328 1024  

#define MAGIC_CODE  0x554B
#define VERSION     0x0001

#define MASK_KEY      0x00FF
#define MASK_MOD      0xFF00

uint8_t var_size;
uint8_t buffer_len;
uint8_t addr_status_buffer;

uint16_t settings[NUM_REGISTERS] = {
  MAGIC_CODE,
  VERSION,
  0x0056, // CCW, no modifiers, Keypad -
  0x0057, // CW, no modifiers, Keypad +
  0x0067, // BTN, no modifiers, Keypad =
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
  if (reg < REGISTER_CCW || reg > REGISTER_BTN) {
    return 0;
  }

  return settings[reg] & MASK_KEY;
}

uint8_t settingsGetModifiers(uint8_t reg) {
  if (reg < REGISTER_CCW || reg > REGISTER_BTN) {
    return 0;
  }

  return (settings[reg] & MASK_MOD) >> 8;
}

void settingsSetKeycode(uint8_t reg, uint8_t keycode) {
  if (reg < REGISTER_CCW || reg > REGISTER_BTN) {
    return;
  }

  settings[reg] &= 0xFF00; // clear out old values
  settings[reg] |= keycode;

  _settingsSave();
}

void settingsSetModifiers(uint8_t reg, uint8_t modifiers) {
  if (reg < REGISTER_CCW || reg > REGISTER_BTN) {
    return;
  }

  settings[reg] &= 0x00FF; // clear out old values
  settings[reg] |= (modifiers << 8);

  _settingsSave();
}
