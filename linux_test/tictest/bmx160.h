/* 
 * File:   bmx160.h
 * Author: root
 * parts from various web BMX160 drivers
 * https://github.com/DFRobot/DFRobot_BMX160/blob/master/DFRobot_BMX160.h
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
#define BMX160_ID_REG                   0x00
#define BMX160_PS_REG                   0x03
#define BMX160_DATA_REG                 0x04
#define BMX160_SPI_SET                  0x01
#define BMX160_SPI_4WIRE                0x00
#define BMX160_SPI_3WIRE                0x01
#define BMX160_CMD_ACCEL_PM_NORMAL      0x11
#define BMX160_CMD_GYRO_PM_NORMAL       0x15
#define BMX160_CMD_MAG_PM_NORMAL        0x19
#define BMX160_ALL_PM_NORMAL            0x15
#define BMX160_REG_CMD                  0x7E
#define BMX160_NV_CONF                  0x70
#define BMX160_IF_CONF                  0x6B
#define BMX160_REG_DUMMY                0x7F

#define ns_10ms                         10000000
#define ns_100ms                        100000000
#define ns_2ms                          2000000
#define ns_5ms                          5000000

#define BMX160_MAG_DATA_ADDR            0x04
#define BMX160_GYRO_DATA_ADDR           0x0C
#define BMX160_ACCEL_DATA_ADDR          0x12

        /** Error code definitions */
#define BMX160_OK                                0
#define BMX160_E_NULL_PTR                        -1
#define BMX160_E_COM_FAIL                        -2
#define BMX160_E_DEV_NOT_FOUND                   -3
#define BMX160_E_OUT_OF_RANGE                    -4
#define BMX160_E_INVALID_INPUT                   -5
#define BMX160_E_ACCEL_ODR_BW_INVALID            -6
#define BMX160_E_GYRO_ODR_BW_INVALID             -7
#define BMX160_E_LWP_PRE_FLTR_INT_INVALID        -8
#define BMX160_E_LWP_PRE_FLTR_INVALID            -9
#define BMX160_E_MAGN_NOT_FOUND                  -10
#define BMX160_FOC_FAILURE                       -11
#define BMX160_ERR_CHOOSE                        -12

        /**\name UTILITY MACROS */
#define BMX160_SET_LOW_BYTE     0x00FF
#define BMX160_SET_HIGH_BYTE    0xFF00

#define BMX160_GET_LSB(var) (uint8_t)(var & BMX160_SET_LOW_BYTE)
#define BMX160_GET_MSB(var) (uint8_t)((var & BMX160_SET_HIGH_BYTE) >> 8)    

#define BMX160_ACCEL_MG_LSB_2G      0.000061035F   ///< Macro for mg per LSB at +/- 2g sensitivity (1 LSB = 0.000061035mg) */
#define BMX160_ACCEL_MG_LSB_4G      0.000122070F   ///< Macro for mg per LSB at +/- 4g sensitivity (1 LSB = 0.000122070mg) */
#define BMX160_ACCEL_MG_LSB_8G      0.000244141F   ///< Macro for mg per LSB at +/- 8g sensitivity (1 LSB = 0.000244141mg) */
#define BMX160_ACCEL_MG_LSB_16G     0.000488281F   ///< Macro for mg per LSB at +/- 16g sensitivity (1 LSB = 0.000488281mg) */

#define BMX160_GYRO_SENSITIVITY_125DPS  0.0038110F ///< Gyroscope sensitivity at 125dps */
#define BMX160_GYRO_SENSITIVITY_250DPS  0.0076220F ///< Gyroscope sensitivity at 250dps */
#define BMX160_GYRO_SENSITIVITY_500DPS  0.0152439F ///< Gyroscope sensitivity at 500dps */
#define BMX160_GYRO_SENSITIVITY_1000DPS 0.0304878F ///< Gyroscope sensitivity at 1000dps */
#define BMX160_GYRO_SENSITIVITY_2000DPS 0.0609756F ///< Gyroscope sensitivity at 2000dps */        

        typedef struct {
                float x; /**< X-axis sensor data */
                float y; /**< Y-axis sensor data */
                float z; /**< Z-axis sensor data */
                uint32_t sensortime; /**< sensor time */
        } sBmx160SensorData_t;

#define BMX160_MAGN_UT_LSB      (0.3F)  ///< Macro for micro tesla (uT) per LSB (1 LSB = 0.1uT) */


        void getAllData(sBmx160SensorData_t *, sBmx160SensorData_t *, sBmx160SensorData_t *);

#ifdef __cplusplus
}
#endif

#endif /* BMX160_H */

