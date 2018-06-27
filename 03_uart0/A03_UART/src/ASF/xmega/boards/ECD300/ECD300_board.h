/*
 * ECD300_board.h
 *
 * Created: 2014/12/17 9:58:24
 *  Author: user
 */ 


#ifndef ECD300_BOARD_H_
#define ECD300_BOARD_H_

/* Turn on all LEDs in ECD300 */
void onAllLeds(void);

/* Turn off all LEDs in ECD300 */
void offAllLeds(void);

/* disable JTAG port in PORTB */
void disableJtagPort(void);

/* Turn on the designated LED */
void onLed(unsigned char ledIndex);

/* Turn off the designated LED */
void offLed(unsigned char ledIndex);


#endif /* ECD300_BOARD_H_ */