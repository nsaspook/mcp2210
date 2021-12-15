/* ****************************************************

  MCP2210 HID programming example using C with the HIDAPI
  library.

  Command response data from the MCP2210 is sent to
  the terminal.

  The commands that this program uses configure the MCP2210's
  VM ram only. The NVM is left unchanged.

 *   -------------- connections to the MCP2210 for DIO board -----------------

	 buf[4] bit 0 = CS 0    BMX160
	 buf[4] bit 1 = GPIO 1
	 buf[4] bit 2 = GPIO 2
	 buf[4] bit 3 =	FUNC2	SPI ACTIVE LED
	 buf[4] bit 4 = FUNC2   LOWPWR led
	 buf[4] bit 5 = CS 5	TIC12400
	 buf[4] bit 6 = FUNC2	Ext Interrupt counter
	 buf[4] bit 7 = CS 7    MC33996
	 buf[5] bit 0 = GPIO 8  input

 *******************************************************/

#include "mcp2210.h"
uint32_t fspeed = 20000; // led movement speed
struct timespec req, rem;

static void do_switch_state(void);

int main(int argc, char* argv[])
{
	mcp2210_spi_type* mcp2210;
	uint8_t imu_id, imu_status;

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
	 * BMX160 IMU in SPI mode 3 testing
	 */
	req.tv_sec = 0;
	req.tv_nsec = 5000000;

	setup_bmx160_transfer(2); // 16-bit transfer, address and one data register
	get_bmx160_transfer(); // display MCP2210 config registers
	bmx160_init(2, 0x00); // toggle CS to set bmx160 SPI mode

	bmx160_set(0b00010010, 0x7e);
	nanosleep(&req, &rem);
	bmx160_init(2, 0x00); // toggle CS to set bmx160 SPI mode
	bmx160_set(0b00010110, 0x7e);
	nanosleep(&req, &rem);
	nanosleep(&req, &rem);
	nanosleep(&req, &rem);
	nanosleep(&req, &rem);
	bmx160_init(2, 0x00); // toggle CS to set bmx160 SPI mode
	bmx160_set(0b00011010, 0x7e);
	nanosleep(&req, &rem);
	bmx160_init(2, 0x00); // toggle CS to set bmx160 SPI mode

	if ((imu_id = bmx160_init(2, 0x00)) == BMX160_ID) {
		imu_status = bmx160_init(2, 0x03); // read power status
		imu_status = bmx160_init(2, 0x03); // read power status
		printf("BMX160 IMU detected, Chip ID %02hhX, Chip Status %02hhX.\n", imu_id, imu_status);
	} else {
		printf("BMX160 IMU NOT detected, Chip ID %02hhX.\n", imu_id);
	}

	do {
		nanosleep(&req, &rem);
		bmx160_init(2, 0x03);
	} while (true);

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
		printf("tic12400_init failed code %i\n", tic12400_fail_value);
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
		setup_mc33996_transfer(); // CS 7 and mode 1
		/*
		 * handle the MCP23S08 chip setting
		 */
		mc33996_init();
		/*
		 * SPI data to update the MCP23S08 outputs
		 */
		mc33996_update();

		/*
		 * light show sequence
		 */
		for (int k = 0; k < 10; k++) {
			//lights up LED0 through LED7 one by one
			for (int i = 0; i < MCP23s08_DATA_LEN; i++) {
				mcp2210->buf[MCP23s08_DATA] = 1 << i;
				mcp2210->buf[MCP23s08_DATA - 1] = 1 << i;
				SPI_WriteRead(mcp2210->handle, mcp2210->buf, mcp2210->rbuf);
				//				sleep_us(10);
				//				SPI_WriteRead(mcp2210->handle, mcp2210->offbuf, mcp2210->rbuf);
				sleep_us(fspeed);
			}
			//			if (get_MCP2210_ext_interrupt()) {
			//				break;
			//			}
			//lights up LED7 through LED0 one by one
			for (int i = 0; i < MCP23s08_DATA_LEN; i++) {
				mcp2210->buf[MCP23s08_DATA] = 0x80 >> i;
				mcp2210->buf[MCP23s08_DATA - 1] = 0x80 >> i;
				SPI_WriteRead(mcp2210->handle, mcp2210->buf, mcp2210->rbuf);
				//				sleep_us(10);
				//				SPI_WriteRead(mcp2210->handle, mcp2210->offbuf, mcp2210->rbuf);
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
