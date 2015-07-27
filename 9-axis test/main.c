/*
 * main.c
 *
 *  Created on: Jul 26, 2015
 *      Author: jimparker
 */

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
	DDRB |= (0b00000001); // set B0 to output

	while (1) {
		PORTB= (1 << 0);
		_delay_ms(1000);
		PORTB= 0;
		_delay_ms(500);
	}
	return(0);

}




