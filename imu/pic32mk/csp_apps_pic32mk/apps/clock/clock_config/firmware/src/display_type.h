/* 
 * File:   display_type.h
 * Author: root
 *
 * Created on August 18, 2020, 3:04 PM
 */

#ifndef DISPLAY_TYPE_H
#define	DISPLAY_TYPE_H

#ifdef	__cplusplus
extern "C" {
#endif

	/*
	 * Display hardware PINS
	 * name			port	motor connector pin
	 * RS			RC1	14
	 * CSB			RC2	13
	 * RESET_LCD		RC10	16
	 * BACKLIGHT_PIN	RA1	10
	 * 
	 * DATA PINS
	 * 
	 * MOSI			RA0	12
	 * MISO			RC0	15
	 * MSCK			RB7	2
	 */

	/*
	 * display IMU text of calculations
	 */
	//#define SHOW_STATS

	/*
	 * use DMA for display buffer updates
	 * configure SPI for no interrupts. DMA triggers on the SPITX flag bit
	 */
#define USE_DMA
#ifndef USE_DMA
	/*
	 * using the interrupt version of the harmony plib SPI driver, 1/4 pure cpu speed
	 * undefine USE_INT for the pure cpu version
	 */
	//#define USE_INT
#endif

	//Select the display type: DOGS102: 102, DOGM128/DOGL128: 128, DOGM132: 132, DOGXL160: 160, DOGXL240: 240
#define DISPLAY_TYPE  240
	//Display Orientation: Normal (0) or upside-down (1)?
#define ORIENTATION_UPSIDEDOWN 0

	//Should chip select (CS) be used?
#define LCD_USE_CHIPSELECT  1

	//Use Backlight?  (0: no backlight, 1: backlight (on when pin is high), 2: backlight (on when pin is low))
#define LCD_USE_BACKLIGHT   1
	//#define EDOGM
#define EDOGS
	//#define EDOGS_DEMO


#include "definitions.h"                // SYS function prototypes
#include "config/pic32mk_mcj_curiosity_pro/peripheral/spi/spi_master/plib_spi1_master.h"
#include "config/pic32mk_mcj_curiosity_pro/peripheral/../peripheral/gpio/plib_gpio.h"
#include "config/pic32mk_mcj_curiosity_pro/peripheral/dmac/plib_dmac.h"

	extern void RS_SetLow(void);
	extern void RS_SetHigh(void);
	extern void CSB_SetLow(void);
	extern void CSB_SetHigh(void);
	extern void SPI_Exchange8bit(uint8_t);
	extern void SPI_ExchangeBuffer(uint8_t *, uint16_t);
	extern uint16_t SPI_to_Buffer(uint8_t *, uint16_t, uint8_t *);
	void wait_lcd_done(void);

#ifdef	__cplusplus
}
#endif

#endif	/* DISPLAY_TYPE_H */

