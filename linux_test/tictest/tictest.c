/* ****************************************************

  MCP2210 HID programming example using C with the HIDAPI
  library.

  Command response data from the MCP2210 is sent to
  the terminal.

  The commands that this program uses configure the MCP2210's
  VM ram only. The NVM is left unchanged.

 *   -------------- connections to the MCP2210 for ADM00420 -----------------

	 buf[4] bit 0 = CS 0    EEPROM CS
	 buf[4] bit 1 = CS 1    MCP3204 CS pin
	 buf[4] bit 2 = GPIO 2
	 buf[4] bit 3 =	FUNC2	SPI ACTIVE LED
	 buf[4] bit 4 = CS 4    MCP23S08
	 buf[4] bit 5 = CS 5	TIC12400
	 buf[4] bit 6 = FUNC2	Ext Interrupt counter
	 buf[4] bit 7 = CS 7    temp chip CS
	 buf[5] bit 0 = GPIO 8  MC33996

 *******************************************************/

#include "mcp2210.h"
uint32_t fspeed = 20000; // led movement speed

static void do_switch_state(void);

int main(int argc, char* argv[])
{
	mcp2210_spi_type* mcp2210;

	/*
	 * init the hidraw device interface
	 */
	if (hid_init()) {
		printf("hidapi init error\n");
		return false;
	}

	/*
	 * setup the hidraw* device to communicate with the MCP2210
	 */
	if ((mcp2210 = hidrawapi_mcp2210_init(NULL)) == NULL) {
		printf("MCP2210 device init error\n");
		return false;
	}

	cancel_spi_transfer(); // cleanup
	/*
	 * handle the TIC12400 chip MCP2210 SPI setting
	 */
	setup_tic12400_transfer();
	get_tic12400_transfer();

	/*
	 * init and program 24 switch inputs from the TIC12400 chip
	 */
	tic12400_reset();
	if (!tic12400_init()) {
		printf("tic12400_init failed\n");
	}
	get_MCP2210_ext_interrupt(); // read switch data
	tic12400_read_sw(0, 0);
	/*
	 * setup program state using switch settings
	 */
	do_switch_state();

	/*
	 * we need to change SPI speed, mode, transfer size and cs as we switch to different devices.
	 */
	while (true) { // blink LED loop
		/*
		 * handle the MCP23S08 chip MCP2210 SPI setting
		 */
		setup_mcp23s08_transfer(); // CS 4 and mode 0
		/*
		 * handle the MCP23S08 chip setting
		 */
		mcp23s08_init();
		/*
		 * SPI data to update the MCP23S08 outputs
		 */
		mcp23s08_update();

		/*
		 * light show sequence
		 */
		for (int k = 0; k < 10; k++) {
			//lights up LED0 through LED7 one by one
			for (int i = 0; i < MCP23s08_DATA_LEN; i++) {
				mcp2210->buf[MCP23s08_DATA] = 1 << i;
				SPI_WriteRead(mcp2210->handle, mcp2210->buf, mcp2210->rbuf);
				sleep_us(fspeed);
			}
			//			if (get_MCP2210_ext_interrupt()) {
			//				break;
			//			}
			//lights up LED7 through LED0 one by one
			for (int i = 0; i < MCP23s08_DATA_LEN; i++) {
				mcp2210->buf[MCP23s08_DATA] = 0x80 >> i;
				SPI_WriteRead(mcp2210->handle, mcp2210->buf, mcp2210->rbuf);
				sleep_us(fspeed);
			}
		}
		if (get_MCP2210_ext_interrupt()) {
			/*
			 * handle the TIC12400 chip MCP2210 SPI setting
			 */
			setup_tic12400_transfer(); // CS 5 and mode 1
			/*
			 * read 24 switch inputs after light show sequence
			 */
			tic12400_read_sw(0, 0);
			/*
			 * look for switch 0 changes for led speeds
			 */
			do_switch_state();
		}
	}
	hid_close(mcp2210->handle);
	hid_exit(); /* Free static HIDAPI objects. */
	return 0;

}

void do_switch_state(void)
{
	if (tic12400_get_sw() & raw_mask_0) {
		fspeed = 20000;
	} else {
		fspeed = 2000;
	}
}
