/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
 * Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
 *
 * Subject to your compliance with these terms, you may use Microchip software
 * and any derivatives exclusively with Microchip products. It is your
 * responsibility to comply with third party license terms applicable to your
 * use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
 * ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END

/*
 * program code and info links
 * 
 * https://github.com/parallaxinc/Simple-Libraries/tree/master/Learn/Simple%20Libraries/Sensor/liblsm9ds1
 * https://x-io.co.uk/open-source-imu-and-ahrs-algorithms/
 * https://github.com/farrellf/Balancing_Robot_Firmware/blob/a6a2d89da8498413812b75e70234a9cc3eda72fa/Main.java
 * youtube video  https://youtu.be/WqQ3HZCI3nA
 */

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************


#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <limits.h>
#include  <sys/kmem.h>
#include "definitions.h"                // SYS function prototypes
#include "config/pic32mk_mcj_curiosity_pro/peripheral/dmac/plib_dmac.h"
#include "imu.h"
#include "../../../../../../Learn-Folder-Updated-2020.03.02/Learn/Simple Libraries/Sensor/liblsm9ds1/lsm9ds1.h"
#include "../pic32mk_mcj_curiosity_pro.X/MadgwickAHRS/MadgwickAHRS.h"
#include "../pic32mk_mcj_curiosity_pro.X/MahonyAHRS/MahonyAHRS.h"
#include "display_type.h"
#include "eadog.h" 
#include "dogm-graphic.h"
#include "OledDriver.h"
#include "OledChar.h"
#include "OledGrph.h"
#include "gfx.h"
#include "magic.h"

uint32_t *pmyflash;
static void NVMerase_page(void);
char cbuffer[256] = "\r\n parallax LSM9DS1 9-axis IMU ";
int gx, gy, gz, ax, ay, az, mx, my, mz;
double g[] = {0.0, 0.0, 0.0}, accel[] = {0.0, 0.0, 0.0};
extern CORETIMER_OBJECT coreTmr;

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main(void)
{
	char buffer[STR_BUF_SIZE];

	/* Initialize all modules */
	SYS_Initialize(NULL);
	pmyflash = (uint32_t *) KVA_TO_PA(myflash);

	/* Start system tick timer */
	CORETIMER_Start();

	/*
	 * setup graphics display
	 */
#ifdef EDOGS
	CSB_Set();
	CORETIMER_DelayMs(500);
	lcd_init();
	OledInit();
	OledSetCharUpdate(0); // manual LCD screen updates for speed
#endif

	if (OSCCONbits.CF) { // check for sysclock proper operation
		sprintf(buffer, "IMU Clock Error");
		eaDogM_WriteStringAtPos(0, 0, buffer);
		sprintf(buffer, "Clock Status %04x", CLKSTAT);
		eaDogM_WriteStringAtPos(1, 0, buffer);
		OledUpdate();
		CORETIMER_DelayMs(5000);
	} else {
		sprintf(buffer, "IMU %s %s", build_date, build_time);
		eaDogM_WriteStringAtPos(0, 0, buffer);
		sprintf(buffer, "%s", VERSION);
		eaDogM_WriteStringAtPos(1, 0, buffer);
		OledUpdate();
	}
	/*
	 * talk to the parallax LSM9DS1 9-axis IMU
	 */

	UART1_Write((uint8_t *) cbuffer, strlen(cbuffer));
	if (!imu_init(1, 2, 3, 4)) {
		// Trouble in River-City, not talking to the IMU
		UART1_Write((uint8_t *) imu_missing, strlen(imu_missing));
		sprintf(buffer, "IMU SPI init error.");
		eaDogM_WriteStringAtPos(2, 0, buffer);
		OledUpdate();
	} else {
		sprintf(buffer, "IMU SPI init complete.");
		eaDogM_WriteStringAtPos(2, 0, buffer);
		OledUpdate();
		if (!SWITCH_Get()) {
			sprintf(buffer, "Erasing IMU CAL data.     ");
			eaDogM_WriteStringAtPos(9, 0, buffer);
			OledUpdate();
			NVMerase_page(); // erase the calibrations data page
			sprintf(buffer, "IMU calibration starting.");
			eaDogM_WriteStringAtPos(3, 0, buffer);
			OledUpdate();
			imu_calibrateAG(); // write new cal data
			sprintf(buffer, "IMU AG done. Starting Mag");
			eaDogM_WriteStringAtPos(4, 0, buffer);
			OledUpdate();
			imu_calibrateMag(); // write new cal data
			sprintf(buffer, "IMU Mag cal done. IMU ready.");
			eaDogM_WriteStringAtPos(5, 0, buffer);
			OledUpdate();
		}

	};
	CORETIMER_DelayMs(500);

	while (true) {
		if (imu_gyroAvailable()) imu_readGyro(&gx, &gy, &gz);
		if (imu_accelAvailable()) imu_readAccel(&ax, &ay, &az);
		if (imu_magAvailable()) imu_readMag(&mx, &my, &mz);
		/*
		 * GYRO data must be in radians per second
		 */
		MadgwickAHRSupdate((float) gx*rps, (float) gy*rps, (float) gz*rps, (float) ax, (float) ay, (float) az, (float) mx, (float) my, (float) mz);
		//MahonyAHRSupdate((float) gx*rps, (float) gy*rps, (float) gz*rps, (float) ax, (float) ay, (float) az, (float) mx, (float) my, (float) mz);
		/*
		 * linear acceleration without gravity
		 * rotate vector to earth
		 */
		g[0] = 2.0 * (q1 * q3 - q0 * q2);
		g[1] = 2.0 * (q0 * q1 + q2 * q3);
		g[2] = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

		/*
		 * compensate accelerometer readings with the expected direction of gravity 
		 * https://diydrones.com/forum/topics/accelerometer-sensor-gravity-compensation
		 */
		accel[0] = ((double) ax / SHRT_MAX - g[0])*9.8;
		accel[1] = ((double) ay / SHRT_MAX - g[1])*9.8;
		accel[2] = ((double) az / SHRT_MAX - g[2])*9.8;

		dtog_Set();
		OledClearBuffer();
#ifdef SHOW_STATS
		sprintf(cbuffer, "G%4d %4d %4d A %4d %4d %4d M %4d %4d %4d", gx, gy, gz, ax, ay, az, mx, my, mz);
		eaDogM_WriteStringAtPos(10, 0, cbuffer);
		sprintf(cbuffer, "%2.4f %2.4f %2.4f %2.4f ", q0, q1, q2, q3);
		UART1_Write((uint8_t *) cbuffer, strlen(cbuffer));
		eaDogM_WriteStringAtPos(12, 0, cbuffer);
		sprintf(cbuffer, "%3.2f %3.2f %3.2f\n\r", accel[0], accel[1], accel[2]);
		UART1_Write((uint8_t *) cbuffer, strlen(cbuffer));
		sprintf(cbuffer, "LA %4.4f %4.4f %4.4f", accel[0], accel[1], accel[2]);
		eaDogM_WriteStringAtPos(13, 0, cbuffer);
		sprintf(cbuffer, "GR %4.4f %4.4f %4.4f", g[0], g[1], g[2]);
		eaDogM_WriteStringAtPos(14, 0, cbuffer);
#else
		sprintf(cbuffer, "%2.4f %2.4f %2.4f %2.4f ", q0, q1, q2, q3);
		UART1_Write((uint8_t *) cbuffer, strlen(cbuffer));
		sprintf(cbuffer, "%3.2f %3.2f %3.2f\n\r", accel[0], accel[1], accel[2]);
		UART1_Write((uint8_t *) cbuffer, strlen(cbuffer));
#endif
		vector_graph();
		{
			//	100 Hz updates, processing takes 5ms
			uint32_t tickStart, delayTicks;
			tickStart = coreTmr.tickCounter;
			delayTicks = (1000 * update_delay) / CORE_TIMER_INTERRUPT_PERIOD_IN_US; // Number of tick interrupts to wait for the delay
			LA_gfx(false, false, 0);
			while ((coreTmr.tickCounter - tickStart) < delayTicks) {
				// extra processing loop while waiting for clock time to expire
				LA_gfx(false, false, 1400);
			}
		}
#ifdef SHOW_STATS
		sprintf(cbuffer, "SA %d %d %d", xa, ya, za);
		eaDogM_WriteStringAtPos(15, 0, cbuffer);
#endif
		dtog_Set();
		OledUpdate();
		dtog_Clear();
		LED_Toggle();
	}

	/* Execution should not come here during normal operation */

	return( EXIT_FAILURE);
}

static void NVMInitiateOperation(void)
{
	unsigned int saved_state;
	int dma_susp; // storage for current DMA state

	saved_state = __builtin_get_isr_state();
	__builtin_disable_interrupts();
	// Disable DMA
	if (!(dma_susp = DMACONbits.SUSPEND)) {
		DMACONSET = _DMACON_SUSPEND_MASK; // suspend
		while ((DMACONbits.DMABUSY)); // wait to be actually suspended
	}
	NVMKEY = 0x0;
	NVMKEY = 0xAA996655;
	NVMKEY = 0x556699AA;
	NVMCONSET = 1 << 15; // must be an atomic instruction
	// Restore DMA
	if (!dma_susp) {
		DMACONCLR = _DMACON_SUSPEND_MASK; // resume DMA activity
	}
	// Restore Interrupts
	__builtin_set_isr_state(saved_state); /* Set back to what was before. */
}

/*
 * erase the calibration page data
 */
static void NVMerase_page(void)
{
	// set destination page address
	NVMADDR = NVM_STARTPADDRESS; // page physical address
	// define Flash operation
	NVMCONbits.NVMOP = 0x4; // NVMOP for Page Erase
	// Enable Flash Write
	NVMCONbits.WREN = 1;
	// commence programming
	NVMInitiateOperation(); // see Example 52-1
	// Wait for WR bit to clear
	while (NVMCONbits.WR);
	// Disable future Flash Write/Erase operations
	NVMCONbits.WREN = 0;
	// Check Error Status
	if (NVMCON & 0x3000) // mask for WRERR and LVDERR bits
	{
		// process errors
	}
}

static unsigned int NVMWriteWord(void* address, unsigned int data)
{
	unsigned int res = 1;
	// Load data into NVMDATA register
	NVMDATA0 = data;
	NVMDATA1 = data;
	NVMDATA2 = data;
	NVMDATA3 = data;
	// Load address to program into NVMADDR register
	NVMADDR = (unsigned int) address;
	// Unlock and Write Word
	// set the operation, assumes WREN = 0
	NVMCONbits.NVMOP = 0x1; // NVMOP for Word programming
	// Enable Flash for write operation and set the NVMOP
	NVMCONbits.WREN = 1;
	// Start programming
	NVMInitiateOperation(); // see Example 52-1
	// Wait for WR bit to clear
	while (NVMCONbits.WR);
	// Disable future Flash Write/Erase operations
	NVMCONbits.WREN = 0;
	// Check Error Status
	if (NVMCON & 0x3000) // mask for WRERR and LVDERR
	{
		res = 0; // TESTING CLEAR POSSIBLE ERROR set 0
	}
	return res;
}

/*
 * read data from the virtual program address of the nvram variable
 */
uint32_t nvram_in(uint8_t adr)
{
	return myflash[adr];
}

/*
 * write data to the physical address of the nvram variable
 * using the flash unlock sequence
 */
uint32_t nvram_out(void *adr, uint32_t data)
{
	return NVMWriteWord((void*) adr, data);
}

/*
 * a few calibrations data specific sized read/write routines
 */

bool get_nvram_str(uint8_t adr, char * str)
{
	bool done = true;
	uint8_t sz = 7;
	char buffer[STR_BUF_SIZE];

	while (sz--) {
		str[sz] = (char) nvram_in(adr + sz);
		sprintf(buffer, "%d   %X %c           ", sz, (unsigned int) &pmyflash[adr + sz], str[sz]);
		eaDogM_WriteStringAtPos(10, 0, buffer);
		OledUpdate();
		CORETIMER_DelayMs(CAL_DIS_MS);
	}
	return done;
}

bool set_nvram_str(uint32_t * adr, char * str)
{
	bool done = false;
	uint8_t sz = 7;
	char buffer[STR_BUF_SIZE];

	while (sz--) {
		done += NVMWriteWord(&adr[sz], (uint32_t) str[sz]);
		sprintf(buffer, "%d %d %X %c             ", sz, done, (unsigned int) &adr[sz], str[sz]);
		eaDogM_WriteStringAtPos(11, 0, buffer);
		OledUpdate();
		CORETIMER_DelayMs(CAL_DIS_MS);
	}
	return done;
}

/*******************************************************************************
 End of File
 */

