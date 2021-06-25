/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   mc33996.h
 * Author: root
 *
 * Created on June 21, 2021, 6:53 PM
 */

#ifndef MC33996_H
#define MC33996_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

	/*
	 * MC33996 command structure
	 */
	typedef struct __attribute__((packed))
	{
		uint16_t out;
		uint8_t cmd;
	}
	mc33996buf_type;

	/*
	 * MC33996 response structure
	 */
	typedef struct __attribute__((packed))
	{
		uint16_t out_faults;
		uint8_t faults;
	}
	mc33996read_type;

#define mc33996_control  0x000000
#define mc33996_load  0x040000
#define mc33996_reset  0x180000

#ifdef __cplusplus
}
#endif

#endif /* MC33996_H */

