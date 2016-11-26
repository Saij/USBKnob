#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "encoder.h"
#include "settings.h"
#include "usbdrv.h"

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
                buildReport(0, 0);
                usbMsgPtr = (void *)&keyboard_report;
                return sizeof(keyboard_report);

            case USBRQ_HID_GET_IDLE: 
                // send idle rate to PC as required by spec
                usbMsgPtr = &idle_rate;
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

    // Set report initialy to 0    
    buildReport(0, 0);
    
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
    
    while(1) {
        // keep the watchdog happy
        wdt_reset(); 
        usbPoll();
    }
    
    return 0;
}
