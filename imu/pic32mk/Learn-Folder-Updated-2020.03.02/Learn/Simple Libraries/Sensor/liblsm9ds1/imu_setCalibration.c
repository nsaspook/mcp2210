/**
 * @file LSM9DS1_setCalibration.c
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
#include "imu.h"
#include "../../nvram_page.h"
#include "../../eadog.h"

int __gBiasRaw[3];
int __aBiasRaw[3];
int __mBiasRaw[3];
int __pinM;
char __autoCalc;
static char tbuf[] = "LSM9DS1";

void imu_setMagCalibration(int mxBias, int myBias, int mzBias)
{
	int k;
	char buffer[STR_BUF_SIZE];

	__mBiasRaw[X_AXIS] = mxBias;
	__mBiasRaw[Y_AXIS] = myBias;
	__mBiasRaw[Z_AXIS] = mzBias;

	unsigned char msb, lsb;
	for (k = 0; k < 3; k++) {
		msb = (__mBiasRaw[k] & 0xFF00) >> 8;
		lsb = __mBiasRaw[k] & 0x00FF;
		imu_SPIwriteByte(__pinM, OFFSET_X_REG_L_M + (2 * k), lsb);
		imu_SPIwriteByte(__pinM, OFFSET_X_REG_H_M + (2 * k), msb);
	}

	if (!set_nvram_str((void*) pmyflash, tbuf)) {
		sprintf(buffer, "Mag LSM9DS1 write failed");
		eaDogM_WriteStringAtPos(9, 0, buffer);
		OledUpdate();
	};

	char biasBuf[7] = {'m', 0, 0, 0, 0, 0, 0};

	for (k = 0; k < 3; k++) {
		biasBuf[k * 2 + 1] = (__mBiasRaw[k] >> 8) & 0xFF;
		biasBuf[k * 2 + 2] = __mBiasRaw[k] & 0xFF;
	}

	set_nvram_str((void*) &pmyflash[7], biasBuf);

}

void imu_setAccelCalibration(int axBias, int ayBias, int azBias)
{
	int k;
	char buffer[STR_BUF_SIZE];

	__aBiasRaw[X_AXIS] = axBias;
	__aBiasRaw[Y_AXIS] = ayBias;
	__aBiasRaw[Z_AXIS] = azBias;

	if (!set_nvram_str((void*) pmyflash, tbuf)) {
		sprintf(buffer, "accel LSM9DS1 write failed");
		eaDogM_WriteStringAtPos(9, 0, buffer);
		OledUpdate();
	};
	
	char biasBuf[7] = {'a', 0, 0, 0, 0, 0, 0};

	for (k = 0; k < 3; k++) {
		biasBuf[k * 2 + 1] = (__aBiasRaw[k] >> 8) & 0xFF;
		biasBuf[k * 2 + 2] = __aBiasRaw[k] & 0xFF;
	}

	set_nvram_str((void*) &pmyflash[14], biasBuf);

	__autoCalc |= 0b10;
}

void imu_setGyroCalibration(int gxBias, int gyBias, int gzBias)
{
	int k;
	char buffer[STR_BUF_SIZE];

	__gBiasRaw[X_AXIS] = gxBias;
	__gBiasRaw[Y_AXIS] = gyBias;
	__gBiasRaw[Z_AXIS] = gzBias;

	if (!set_nvram_str((void*) pmyflash, tbuf)) {
		sprintf(buffer, "Gyro LSM9DS1 write failed");
		eaDogM_WriteStringAtPos(9, 0, buffer);
		OledUpdate();
	};

	char biasBuf[7] = {'g', 0, 0, 0, 0, 0, 0};

	for (k = 0; k < 3; k++) {
		biasBuf[k * 2 + 1] = (__gBiasRaw[k] >> 8) & 0xFF;
		biasBuf[k * 2 + 2] = __gBiasRaw[k] & 0xFF;
	}

	set_nvram_str((void*) &pmyflash[21], biasBuf);

	__autoCalc = 0b01;
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