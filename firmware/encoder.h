#ifndef  __ENCODER_H__
#define  __ENCODER_H__

#include <avr/io.h>
#include <stdint.h>

/*
 * Ports and Pins
 */
#define ENC_PIN_A PB4
#define ENC_PIN_B PB3
#define ENC_BTN   PB2

#define ENC_PIN   PINB
#define ENC_DDR   DDRB
#define ENC_PORT  PORTB

/*
 * Spin direction
 */
#define SPIN_CW   0x01 
#define SPIN_CCW  0x10

void encInit(void);
void encPoll(void);
uint8_t encGetState(void);
uint8_t encGetButtonState(void);

#endif // __ENCODER_H__

