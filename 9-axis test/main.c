/*
 * main.c
 *
 *  Created on: Jul 26, 2015
 *      Author: jimparker
 */

#include <avr/io.h>
#include <util/delay.h>
#include "i2cmaster.h"

#define NineAxis 0xa4

int main(void) {
    unsigned char ret;
    i2c_init();                                // init I2C interface

    /* write configurations to NineAxis registers*/
    ret = i2c_start(NineAxis+I2C_WRITE);       // set device address and write mode
    if ( ret ) {
        /* failed to issue start condition, possibly no device found */
        i2c_stop();
    } else {
        /* issuing start condition ok, device accessible */
        i2c_write(0x05);                       // write register address
        i2c_write(0x75);                       // ret=0 -> Ok, ret=1 -> no ACK

        i2c_stop();                            // set stop conditon = release bus

        /* check the written values */
        i2c_start_wait(NineAxis+I2C_WRITE);     // set device address and write mode
        i2c_write(0x05);                        // write address = 5
        i2c_rep_start(NineAxis+I2C_READ);       // set device address and read mode
        ret = i2c_readNak();                    // read one byte
        i2c_stop();
    }

    /* read values */


	return(0);
}




