/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cFiles/file.c to edit this template
 */

#include "mcp2210.h"

float accelRange = BMX160_ACCEL_MG_LSB_2G * 9.8;
float gyroRange = BMX160_GYRO_SENSITIVITY_250DPS;

void getAllData(sBmx160SensorData_t *magn, sBmx160SensorData_t *gyro, sBmx160SensorData_t *accel)
{

	uint8_t data[23] = {0};
	int16_t x = 0, y = 0, z = 0;
	// put your main code here, to run repeatedly:
	move_bmx160_transfer(data);
	//    readReg(BMX160_MAG_DATA_ADDR, data, 23);
	if (magn) {
		x = (int16_t) (((uint16_t) data[1] << 8) | data[0]);
		y = (int16_t) (((uint16_t) data[3] << 8) | data[2]);
		z = (int16_t) (((uint16_t) data[5] << 8) | data[4]);
		magn->x = x * BMX160_MAGN_UT_LSB;
		magn->y = y * BMX160_MAGN_UT_LSB;
		magn->z = z * BMX160_MAGN_UT_LSB;
	}
	if (gyro) {
		x = (int16_t) (((uint16_t) data[9] << 8) | data[8]);
		y = (int16_t) (((uint16_t) data[11] << 8) | data[10]);
		z = (int16_t) (((uint16_t) data[13] << 8) | data[12]);
		gyro->x = x * gyroRange;
		gyro->y = y * gyroRange;
		gyro->z = z * gyroRange;
	}
	if (accel) {
		x = (int16_t) (((uint16_t) data[15] << 8) | data[14]);
		y = (int16_t) (((uint16_t) data[17] << 8) | data[16]);
		z = (int16_t) (((uint16_t) data[19] << 8) | data[18]);
		accel->x = x * accelRange;
		accel->y = y * accelRange;
		accel->z = z * accelRange;
	}
}
