/**
 * @file imu_init.c
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


#include "definitions.h"                // SYS function prototypes
#include "../../Utility/libsimpletools/simpletools.h"
#include "lsm9ds1.h"
#include "imu.h"
#include "../../eadog.h"

int __pinAG, __pinM, __pinSDIO, __pinSCL;
char __autoCalc = 0;

int imu_init(int pinSCL, int pinSDIO, int pinAG, int pinM)
{
	char buffer[STR_BUF_SIZE];

	__pinAG = pinAG;
	__pinM = pinM;
	__pinSDIO = pinSDIO;
	__pinSCL = pinSCL;

	// Set both the Accel/Gyro and Mag to 3-wire SPI mode
	/*
	 * We don't use SPI bit-banging for 3-wire because it's possible to setup a hardware 3-wire mode on the PIC32 module
	 * on the pic32mk hardware SPI module jumper mosi to miso for half-duplex 3-wire mode and config for SPI MODE 3
	 * set the TRIS mode on the SPI TX port pin for input so we can turn-off the transmitter voltage during SPI receive 
	 * on the bidirectional SPI line
	 * 
	 * SPIxCON: SPI CONTROL REGISTER
	 * bit 12 DISSDO: Disable SDOx pin bit(4)
	 * 1 = SDOx pin is not used by the module. Pin is controlled by associated PORT register
	 * 0 = SDOx pin is controlled by the module
	 */
	TRISCbits.TRISC7 = 1; // SPI2 TX port set to port control as a input so we don't affect received data when in port mode
	imu_SPIwriteByte(__pinAG, CTRL_REG8, 0b00001100);
	imu_SPIwriteByte(__pinM, CTRL_REG3_M, 0b10000100);

	// To verify communication, we can read from the WHO_AM_I register of
	// each device. Store those in a variable so we can return them.
	char xgTest = 0, mTest = 0;
	imu_SPIreadBytes(__pinM, WHO_AM_I_M, (unsigned char *) &mTest, 1); // Read the gyro WHO_AM_I
	imu_SPIreadBytes(__pinAG, WHO_AM_I_XG, (unsigned char *) &xgTest, 1); // Read the accel/mag WHO_AM_I
	int whoAmICombined = (xgTest << 8) | mTest;

	if (whoAmICombined != ((WHO_AM_I_AG_RSP << 8) | WHO_AM_I_M_RSP)) return 0;

	//Init Gyro
	imu_SPIwriteByte(__pinAG, CTRL_REG1_G, 0xC0);
	imu_SPIwriteByte(__pinAG, CTRL_REG2_G, 0x00);
	imu_SPIwriteByte(__pinAG, CTRL_REG3_G, 0x00);
	imu_SPIwriteByte(__pinAG, CTRL_REG4, 0x38);
	imu_SPIwriteByte(__pinAG, ORIENT_CFG_G, 0x00);

	//Init Accel
	imu_SPIwriteByte(__pinAG, CTRL_REG5_XL, 0x38);
	imu_SPIwriteByte(__pinAG, CTRL_REG6_XL, 0xC0);
	imu_SPIwriteByte(__pinAG, CTRL_REG7_XL, 0x00);

	//Init Mag
	imu_SPIwriteByte(__pinM, CTRL_REG2_M, 0x00);
	imu_SPIwriteByte(__pinM, CTRL_REG4_M, 0x0C);
	imu_SPIwriteByte(__pinM, CTRL_REG5_M, 0x00);

	//Set Scales
	imu_setGyroScale(2000); // 500
	imu_setAccelScale(16);  // 8
	imu_setMagScale(16);   // 12

	//Look for calibrations in NVRAM
	char biasStamp[8] = {0};
	char mBiasStored[8] = {0};
	char aBiasStored[8] = {0};
	char gBiasStored[8] = {0};
	get_nvram_str(0, biasStamp);
	get_nvram_str(7, mBiasStored);
	get_nvram_str(14, aBiasStored);
	get_nvram_str(21, gBiasStored);

	if (strncmp(biasStamp, "LSM9DS1", 7) == 0) {
		sprintf(buffer, "LSM9DS1 cal header found");
		eaDogM_WriteStringAtPos(8, 0, buffer);
		OledUpdate();
		if ((mBiasStored[0] = 'm')) {
			int mxB = (mBiasStored[1] << 8) | mBiasStored[2];
			int myB = (mBiasStored[3] << 8) | mBiasStored[4];
			int mzB = (mBiasStored[5] << 8) | mBiasStored[6];

			imu_setMagCalibration(mxB, myB, mzB);
		}

		if ((aBiasStored[0] = 'a')) {
			int axB = (aBiasStored[1] << 8) | aBiasStored[2];
			int ayB = (aBiasStored[3] << 8) | aBiasStored[4];
			int azB = (aBiasStored[5] << 8) | aBiasStored[6];

			imu_setAccelCalibration(axB, ayB, azB);
		}

		if ((gBiasStored[0] = 'g')) {
			int gxB = (gBiasStored[1] << 8) | gBiasStored[2];
			int gyB = (gBiasStored[3] << 8) | gBiasStored[4];
			int gzB = (gBiasStored[5] << 8) | gBiasStored[6];

			imu_setGyroCalibration(gxB, gyB, gzB);
		}
	} else {
		sprintf(buffer, "No LSM9DS1 cal header found %s", biasStamp);
		eaDogM_WriteStringAtPos(8, 0, buffer);
		OledUpdate();
	}



	// Once everything is initialized, return the WHO_AM_I registers we read:
	return whoAmICombined;
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