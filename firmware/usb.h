#ifndef __USB_H__
#define __USB_H__

#define REPID_MOUSE         1
#define REPID_KEYBOARD      2
#define REPID_MMKEY         3
#define REPID_SYSCTRLKEY    4

#define REPSIZE_MOUSE       4
#define REPSIZE_KEYBOARD    8
#define REPSIZE_MMKEY       3
#define REPSIZE_SYSCTRLKEY  2

extern uint8_t report_buffer[8];
extern uint8_t usb_connected;
extern uint8_t idle_rate;
extern uint8_t protocol_version;

void usbReportSend(uint8_t sz);

#endif // __USB_H__