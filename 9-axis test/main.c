/*
 * main.c
 *
 *  Created on: Jul 26, 2015
 *      Author: jimparker
 */

#include <avr/io.h>
#include <util/delay.h>
#include "i2cmaster.h"
#include "USART.h"

#define NineAxis 0b11010000  // bit 0 = AD0 = pin9 = gnd
#define NineAxisMag 0x18  // (0x0C << 1)

void write_nineAxis(uint8_t address, uint8_t byte) {
    unsigned char ret;
    ret = i2c_start(NineAxis+I2C_WRITE);       // set device address and write mode
    if ( ret ) {
        /* failed to issue start condition, possibly no device found */
        i2c_stop();
    } else {
        /* issuing start condition ok, device accessible */
        i2c_write(address);                    // write register address
        i2c_write(byte);                       // ret=0 -> OK, ret=1 -> no ACK
        i2c_stop();                            // set stop conditon = release bus
    }
}

void write_nineAxisMag(uint8_t address, uint8_t byte) {
    unsigned char ret;
    ret = i2c_start(NineAxisMag+I2C_WRITE);       // set device address and write mode
    if ( ret ) {
        /* failed to issue start condition, possibly no device found */
    	printString("Could not write to Nine-Axis Mag !!!\r\n");
        i2c_stop();
    } else {
        /* issuing start condition ok, device accessible */
        i2c_write(address);                    // write register address
        i2c_write(byte);                       // ret=0 -> OK, ret=1 -> no ACK
        i2c_stop();                            // set stop conditon = release bus
    }
}

int read_nineAxis(uint8_t address, uint8_t* data, uint8_t numBytes) {
	int i = 0;
	if (numBytes > 0) {
		i2c_start_wait(NineAxis+I2C_WRITE);   // set device address and write mode
		/* issuing start condition ok, device accessible */
		i2c_write(address);                     // write register address
		i2c_rep_start(NineAxis+I2C_READ);       // set device address and read mode

		for(i=0; i < (numBytes-1); i++)
			data[i] = i2c_read(1);
		data[i] = i2c_read(0);
		i++;
		i2c_stop();
	}
	return(i);
}

int read_NineAxisMag(uint8_t address, uint8_t* data, uint8_t numBytes) {
	int i = 0;
	if (numBytes > 0) {
    	write_nineAxisMag(0x0A, 0x02);          //enable the magnetometer in one-shot mode
		i2c_start_wait(NineAxisMag+I2C_WRITE);   // set device address and write mode
		/* issuing start condition ok, device accessible */
		i2c_write(address);                     // write register address
		i2c_rep_start(NineAxisMag+I2C_READ);    // set device address and read mode

		for(i=0; i < (numBytes-1); i++)
			data[i] = i2c_read(1);
		data[i] = i2c_read(0);
		i++;
		i2c_stop();
	}
	return(i);
}

uint8_t init_nineAxisMag() {
	write_nineAxis(55, 0x02); //set i2c bypass enable bit to true to access magnetometer
	_delay_ms(100);
	write_nineAxis(106, 0x01); //reset i2c master
	_delay_ms(100);

    unsigned char ret;
    ret = i2c_start(NineAxisMag+I2C_WRITE);       // set device address and write mode
    if ( ret ) {
        /* failed to issue start condition, possibly no device found */
    	printString("Mag device not found on I2C port !!!\r\n");
    	i2c_stop();
    	return(0);
    } else {
    	printString("Found MagNine-Axis device on I2C port\r\n");
    	i2c_stop();
    	return(1);
    }
}

void search_i2c() {
	//write_nineAxis(55, 0x02); //set i2c bypass enable bit to true
	//write_nineAxis(106, 0x01); //reset i2c master

    unsigned char ret;
    for (uint8_t addy=0; addy < 128; addy++) {
    	printBinaryByte(addy<<1);
    	printString(" (0x");
    	printHexByte(addy<<1);
    	printString(")");
        ret = i2c_start((addy<<1)+I2C_WRITE);       // set device address and write mode
        if ( ret == 0 ) {
        	printString(": Device found on I2C port !!!");
        	i2c_stop();
        }
    	printString("\r\n");
    }
    printString("Finished searching I2C bus\r\n");
    return;
}

uint8_t init_nineAxisMagSlave0() {
	// TODO:  get mag to work as slave0
	write_nineAxis(55, 0x00); //clear i2c bypass enable bit
	write_nineAxis(37,NineAxisMag >> 1);  // load mag I2C address to slave0 address register
	write_nineAxis(38,0x03);  // load mag starting data register to slave0 data register aaddress register
	write_nineAxis(39,0b11010110);  // set up slave 0 contol register
	write_nineAxis(106, 0x01); //reset i2c master

	// this needs to be written to mag register 0x0a
	write_nineAxis(99, 0x02); //load slave0 data output register with value to enable the magnetometer
   	return(1);
}

uint8_t init_nineAxis() {
    unsigned char ret;
    ret = i2c_start(NineAxis+I2C_WRITE);       // set device address and write mode
    if ( ret ) {
        /* failed to issue start condition, possibly no device found */
    	printString("Nine-Axis device not found on I2C port !!!\r\r");
    	i2c_stop();
    	return(0);
    } else {
    	printString("Found Nine-Axis device on I2C port\r\n");
    	i2c_stop();

    	write_nineAxis(26,0);		// config
    	write_nineAxis(27,0x02<<4);	// gyro full scale (1000)
    	write_nineAxis(28,0x02<<4);	// accel full scale (8g)
    	write_nineAxis(107,1);		// clock source
    	return(1);
    }
}

uint16_t uimod(uint16_t x, uint16_t y) {
	return(x - (x/y));
}

void convertBuffer(uint8_t *buffer, int *data, uint8_t numConversions) {
	for(int i=0; i < numConversions; i++) {
		data[i] = (((int16_t)buffer[2*i]) << 8) | buffer[2*i+1];
	}
}

void swapBuffer(uint8_t *buffer, uint8_t numValues) {
	uint8_t temp;
	for(int i=0; i < numValues; i+=2) {
		temp = buffer[i+1];
		buffer[i+1] = buffer[i];
		buffer[i] = temp;
	}
}

void printInt(int i) {
	if (i < 0) {
		printString("-");
		i = -i;
	}
	if (i == 0) {
		transmitByte('0');
	} else {
		transmitByte('0' + (i / 10000));                 /* Ten-thousands */
		transmitByte('0' + ((i / 1000) % 10));           /* Thousands */
		transmitByte('0' + ((i / 100) % 10));            /* Hundreds */
		transmitByte('0' + ((i / 10) % 10));             /* Tens */
		transmitByte('0' + (i % 10));                    /* Ones */
	}
}

void positionCursor(uint8_t row, uint8_t col) {
	transmitByte(0x1b);  // <ESC>
	printString("[");
	printByte(row);
	printString(";");
	printByte(col);
	printString("H");
}

int main(void) {
	uint8_t byteBuffer[14+6];
	int data[7+3];

	initUSART();
    i2c_init();           // init I2C interface

	// clear screen
	transmitByte(0x1b);
	printString("[2J");

    //search_i2c();
    /* configure NineAxis registers*/
    if (init_nineAxis() & init_nineAxisMag())
    {
    	char b;
    	while ((b = receiveByte()) != 'x')
    		transmitByte(b);
        while (1) {
        	read_nineAxis(59,byteBuffer,14);
        	read_NineAxisMag(0x03,byteBuffer+14,6);

        	//swapBuffer(byteBuffer, 14+6);
        	convertBuffer(byteBuffer, data, 7+3);
        	// clear screen
        	transmitByte(0x1b);
        	printString("[2J");

        	positionCursor(1,8);
        	printString("9-Axis readings");

        	positionCursor(3,14);
        	printString("X");
        	positionCursor(3,24);
        	printString("Y");
        	positionCursor(3,34);
        	printString("Z");

        	positionCursor(5,4);
        	printString("accel:");

        	positionCursor(7,5);
        	printString("gyro:");

        	positionCursor(9,6);
        	printString("mag:");

        	positionCursor(11,5);
        	printString("temp:");

        	positionCursor(5,10);
        	printInt(data[0]);
        	positionCursor(5,20);
        	printInt(data[1]);
        	positionCursor(5,30);
        	printInt(data[2]);

           	positionCursor(7,10);
           	printInt(data[4]);
           	positionCursor(7,20);
        	printInt(data[5]);
           	positionCursor(7,30);
        	printInt(data[6]);

           	positionCursor(9,10);
           	printInt(data[7]);
           	positionCursor(9,20);
        	printInt(data[8]);
           	positionCursor(9,30);
        	printInt(data[9]);

        	positionCursor(11,10);
        	float tempc = data[3]/340.0 + 35;
        	float tempf = tempc * 1.8 + 32;
        	printInt((int) tempc);
        	transmitByte(0xc2); transmitByte(0xb0);  // degree symbol
        	printString("C (");
        	printInt((int) tempf);
        	transmitByte(0xc2); transmitByte(0xb0);  // degree symbol
        	printString("F)");

        	_delay_ms(1000);
        }
    }
	return(0);
}




