/**
 * @file LSM9DS1_SPIread.c
 *
 * @author Matthew Matz
 *
 * @version 0.5
 *
 * @copyright
 * Copyright (C) Parallax, Inc. 2016. All Rights MIT Licensed.
 *
 * @brief This Propeller C library was created for the Parallax 9-axis IMU Sensor, based
 * on the STMicroelectronics LSM9DS1 inertial motion sensor chip.
 */

#include "lsm9ds1.h"
#include "config/pic32mk_mcj_curiosity_pro/peripheral/gpio/plib_gpio.h"
#include "config/pic32mk_mcj_curiosity_pro/peripheral/spi/spi_master/plib_spi2_master.h"

extern int __pinM, __pinSDIO, __pinSCL;

/*
 * PIC32: changed to use hardware SPI module and plib_gpio pin functions
 * IMU SPI is half-duplex so we short pic32 SPI TX to RX signals and toggle TX pin state to PORT pin mode input during SPI receive
 */
void imu_SPIreadBytes(unsigned char csPin, unsigned char subAddress, unsigned char *dest, unsigned char count)
{
	int i;

	// To indicate a read, set bit 0 (msb) of first byte to 1
	unsigned char rAddress = 0x80 | (subAddress & 0x3F);

	// Mag SPI port is different. If we're reading multiple bytes, 
	// set bit 1 to 1. The remaining six bytes are the address to be read
	if ((csPin == __pinM) && count > 1) rAddress |= 0x40;

	SPI2CONbits.DISSDO = 0; // half-duplex switching, SPI TX ON
	if (csPin == __pinM)
		csPin_m_Clear();
	else
		csPin_ag_Clear();

	SPI2_Write(&rAddress, 1);
	SPI2CONbits.DISSDO = 1;
	for (i = 0; i < count; i++) {
		SPI2_Read(&dest[i], 1); // half-duplex switching, SPI TX OFF, PORT pin mode input
	}
	if (csPin == __pinM)
		csPin_m_Set();
	else
		csPin_ag_Set();

}


/*
 * Based on the Arduino Library for the LSM9SD1 by Jim Lindblom of Sparkfun Electronics
 */

/**
 * TERMS OF USE: MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */