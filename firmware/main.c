#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdint.h>

#include "usbdrv.h"
#include "encoder.h"
#include "settings.h"
#include "usb.h"

void send_keyboard_key(uint8_t modifiers, uint8_t keycode) {
    report_buffer[0] = REPID_KEYBOARD;
    report_buffer[1] = modifiers;
    report_buffer[2] = 0; // reserved
    report_buffer[3] = keycode;
    report_buffer[4] = 0;
    report_buffer[5] = 0;
    report_buffer[6] = 0;
    report_buffer[7] = 0;
    
    usbReportSend(REPSIZE_KEYBOARD);
}

void send_mm_key(uint8_t key) {
    report_buffer[0] = REPID_MMKEY;
    report_buffer[1] = key;
    report_buffer[2] = 0;
    usbReportSend(REPSIZE_MMKEY);

    // Immediate release
    report_buffer[0] = REPID_MMKEY;
    report_buffer[1] = 0;
    report_buffer[2] = 0;
    usbReportSend(REPSIZE_MMKEY);
}

void send(uint8_t type, uint8_t modifiers, uint8_t keycode) {
    switch (type) {
        case TYPE_KEYBOARD:
            send_keyboard_key(modifiers, keycode);
            break;

        case TYPE_MM:
            send_mm_key(keycode);
            break;
    }
}

int main() {
    uchar i;

    // PB0 as output for LED
    DDRB = 1 << PB0;

    settingsInit();
    encInit();

    // enable 1s watchdog timer
    wdt_enable(WDTO_1S); 

    usbInit();
    
    // enforce re-enumeration
    usbDeviceDisconnect(); 
    for (i = 0; i < 250; i++) { 
        // wait 500 ms
        // keep the watchdog happy
        wdt_reset(); 
        _delay_ms(2);
    }
    usbDeviceConnect();
    
    // Enable interrupts after re-enumeration
    sei(); 

    uint8_t last_btn_state = 0;
    uint8_t last_enc_state = 0;
    uint8_t btn_state = 0;
    uint8_t enc_state = 0;

    while (1) {
        // keep the watchdog happy
        wdt_reset(); 
        encPoll();
        usbPoll();

        btn_state = encGetButtonState();
        enc_state = encGetState();

        if (btn_state != last_btn_state) {
            if (btn_state) {
                // Pressed
                send(settingsGetType(SETTINGS_BTN), settingsGetModifiers(SETTINGS_BTN), settingsGetKeycode(SETTINGS_BTN));
            } else {
                // Released
                send(settingsGetType(SETTINGS_BTN), 0x00, 0x00);
            }

            last_btn_state = btn_state;
        } else if (enc_state != last_enc_state && !btn_state) {
            if (enc_state == SPIN_CW) {
                send(settingsGetType(SETTINGS_CW), settingsGetModifiers(SETTINGS_CW), settingsGetKeycode(SETTINGS_CW));
            } else if (enc_state == SPIN_CCW) {
                send(settingsGetType(SETTINGS_CCW), settingsGetModifiers(SETTINGS_CCW), settingsGetKeycode(SETTINGS_CCW));
            } else {
                // No spin
                if (last_enc_state == SPIN_CW) {
                    send(settingsGetType(SETTINGS_CW), 0x00, 0x00);
                } else if (last_enc_state == SPIN_CCW) {
                    send(settingsGetType(SETTINGS_CCW), 0x00, 0x00);
                }
                
            }

            last_enc_state = enc_state;
        }
    }
    
    return 0;
}
