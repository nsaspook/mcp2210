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
#define BMX160_CMD_ACCEL_PM_NORMAL      0x11
#define BMX160_CMD_GYRO_PM_NORMAL       0x15
#define BMX160_CMD_MAG_PM_NORMAL        0x19
#define BMX160_ALL_PM_NORMAL            0x15
#define BMX160_REG_CMD                  0x7E
#define BMX160_REG_DUMMY                0x00


#ifdef __cplusplus
}
#endif

#endif /* BMX160_H */

