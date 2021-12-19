/* 
 * File:   magic.h
 * Author: root
 *
 * Created on October 31, 2020, 7:38 AM
 */

#ifndef MAGIC_H
#define	MAGIC_H

#ifdef	__cplusplus
extern "C" {
#endif

	static const double rps = 0.0174532925f; // degrees per second -> radians per second
	static const uint8_t CAL_DIS_MS = 1; // calibration data element screen display time
	static const char *build_date = __DATE__, *build_time = __TIME__;
	static const char imu_missing[] = " MISSING \r\n";
	/*
	 * NVRAM storage page variable
	 * const volatile here means it's stored in FLASH for read-only program data access in normal program flow but
	 * also set to volatile because the FLASH write controller can modify the contents out of normal program flow
	 * so we need the compiler TO NOT optimize out reads from the actual memory locations because of read-only data caching
	 * optimizations
	 */
#include "nvram_page.h"
	const volatile uint32_t myflash[4096] __attribute__((section("myflash"), space(prog), address(NVM_STARTVADDRESS)));

	const uint32_t update_delay = 5; // ms delay to make a total of 10ms between IMU updates

#ifdef	__cplusplus
}
#endif

#endif	/* MAGIC_H */

