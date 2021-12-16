/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cFiles/file.h to edit this template
 */

/* 
 * File:   bmx160.h
 * Author: root
 *
 * Created on December 13, 2021, 7:32 PM
 */

#ifndef BMX160_H
#define BMX160_H

#ifdef __cplusplus
extern "C" {
#endif

#define BMX160_R                        0b10000000
#define BMX160_W                        0b00000000
#define BMX160_ID                       0xD8
#define BMX160_SPI_SET   0x01
#define BMX160_SPI_4WIRE   0x00
#define BMX160_SPI_3WIRE   0x01
#define BMX160_CMD_ACCEL_PM_NORMAL      0x11
#define BMX160_CMD_GYRO_PM_NORMAL       0x15
#define BMX160_CMD_MAG_PM_NORMAL        0x19
#define BMX160_ALL_PM_NORMAL            0x15
#define BMX160_REG_CMD                  0x7E
#define BMX160_NV_CONF   0x70
#define BMX160_IF_CONF   0x6B
#define BMX160_REG_DUMMY                0x7F

#define ns_10ms    10000000
#define ns_100ms   100000000
#define ns_2ms    2000000
#define ns_5ms    5000000


#ifdef __cplusplus
}
#endif

#endif /* BMX160_H */

