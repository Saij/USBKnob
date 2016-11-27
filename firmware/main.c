#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "encoder.h"
#include "settings.h"
#include "usbdrv.h"
#include "oddebug.h"

#define DBG_INIT        0x00
#define DBG_PRE_MAIN    0x01
#define DBG_SEND_IRQ    0xF0
#define DBG_BTN_STATE   0xF1
#define DBG_ENC_STATE   0xF2

// From Frank Zhao's USB Business Card project
// http://www.frank-zhao.com/cache/usbbusinesscard_details.php
PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)(224)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs) ; Modifier byte
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs) ; Reserved byte
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs) ; LED report
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs) ; LED report padding
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))(0)
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)(101)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION
};

typedef struct {
    uint8_t modifier;
    uint8_t reserved;
    uint8_t keycode[6];
} keyboard_report_t;

static keyboard_report_t keyboard_report; // sent to PC
static uchar idle_rate; // repeat rate for keyboards

void buildReport(uint8_t send_key, uint8_t modifier) {
    keyboard_report.modifier = modifier;
    keyboard_report.keycode[0] = send_key;
    keyboard_report.keycode[1] = 0;
    keyboard_report.keycode[2] = 0;
    keyboard_report.keycode[3] = 0;
    keyboard_report.keycode[4] = 0;
    keyboard_report.keycode[5] = 0;    
}

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
    usbRequest_t *rq = (void *)data;

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        switch(rq->bRequest) {
            case USBRQ_HID_GET_REPORT: 
                // send "no keys pressed" if asked here
                buildReport(0x00, 0x00);
                usbMsgPtr = (usbMsgPtr_t)&keyboard_report;
                return sizeof(keyboard_report);

            case USBRQ_HID_GET_IDLE: 
                // send idle rate to PC as required by spec
                usbMsgPtr = (usbMsgPtr_t)&idle_rate;
                return 1;

            case USBRQ_HID_SET_IDLE: 
                // save idle rate as required by spec
                idle_rate = rq->wValue.bytes[1];
                return 0;
        }
    }
    
    return 0;
}

int main() {
    uchar i;

    // PB0 as output for LED
    DDRB  = 1 << PB0;

    settingsInit();
    encInit();

    // Set report initialy to 0    
    buildReport(0x00, 0x00);
    
    // enable 1s watchdog timer
    wdt_enable(WDTO_1S); 

    // enable debug
    odDebugInit();
    DBG1(DBG_INIT, 0, 0);

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
    uint8_t send_report = 0;

    DBG1(DBG_PRE_MAIN, 0, 0);

    while(1) {
        // DBG1(0x02, 0, 0);
        // keep the watchdog happy
        wdt_reset(); 
        encPoll();
        usbPoll();

        btn_state = encGetButtonState();
        enc_state = encGetState();

        if (btn_state != last_btn_state) {
            DBG1(DBG_BTN_STATE, (uchar *)&btn_state, 1);
            
            if (btn_state) {
                // Pressed
                buildReport(settingsGetKeycode(REGISTER_BTN), settingsGetModifiers(REGISTER_BTN));
            } else {
                // Released
                buildReport(0x00, 0x00);
            }

            last_btn_state = btn_state;
            send_report = 1;
        } else if (enc_state != last_enc_state) {
            DBG1(DBG_ENC_STATE, (uchar *)&enc_state, 1);

            if (enc_state == SPIN_CW) {
                buildReport(settingsGetKeycode(REGISTER_CW), settingsGetModifiers(REGISTER_CW));
            } else if (enc_state == SPIN_CCW) {
                buildReport(settingsGetKeycode(REGISTER_CCW), settingsGetModifiers(REGISTER_CCW));
            } else {
                // No spin
                buildReport(0x00, 0x00);
            }

            last_enc_state = enc_state;
            send_report = 1;
        }

        if (usbInterruptIsReady() && send_report) {
            // Send
            DBG1(DBG_SEND_IRQ, (uchar *)&keyboard_report, sizeof(keyboard_report));
            usbSetInterrupt((uchar *)&keyboard_report, sizeof(keyboard_report));
            send_report = 0;
        }
    }
    
    return 0;
}
