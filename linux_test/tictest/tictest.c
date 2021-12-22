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
	 buf[5] bit 0 = GPIO 8  input, jumper to CS 0

 *******************************************************/

#include "mcp2210.h"
uint32_t fspeed = 20000; // led movement speed
sBmx160SensorData_t magn, gyro, accel;
extern const char *build_date, *build_time;
char fifo_buf[256];

static void do_switch_state(void);

int main(int argc, char* argv[])
{
	mcp2210_spi_type* mcp2210;
	uint8_t imu_id, imu_status, data_status;

	int pipefd;
	char * myfifo = "/tmp/myfifo";

	/*
	 * init the hidraw device interface
	 */
	if (hid_init()) {
		printf("hidapi init error.\n");
		return false;
	}

	/*
	 * setup the hidraw* device to communicate with the MCP2210
	 */
	if ((mcp2210 = hidrawapi_mcp2210_init(NULL)) == NULL) {
		printf("MCP2210 device init error.\n");
		return false;
	}

	cancel_spi_transfer(); // cleanup

	/* remove the old FIFO */
	unlink(myfifo);
	/* create the FIFO (named pipe) */
	mkfifo(myfifo, 0777);

	/* Open the pipe. Just like you open a file */
	pipefd = open(myfifo, O_RDWR);

	/*
	 * BMX160 IMU in SPI mode 3 testing
	 */
	printf("\r--- BMX160 Driver Version %s %s %s ---\r\n", BMX160_DRIVER, build_date, build_time);
	setup_bmx160_transfer(2); // 2 byte transfer, address and one data register
	bmx160_get(2, BMX160_REG_DUMMY); // toggle CS to set bmx160 SPI mode
	// check for bmx160 boot status and configure if needed
	if ((imu_id = bmx160_get(2, BMX160_ID_REG)) == BMX160_ID) {
		imu_status = bmx160_get(2, BMX160_PS_REG); // read power status
		printf("BMX160 IMU detected, Chip ID %02hhX, Chip Power Status %02hhX.\n", imu_id, imu_status);
		if (imu_status == BMX160_ALL_PM_NORMAL) {
			printf("BMX160 IMU Sensors All Operational.\n");
		} else {
			printf("BMX160 IMU Sensors Not Ready.\n");
			printf("BMX160 IMU Sensors Configuration Started. Chip Power Status: %02hhX.\n", imu_status);
			bmx160_set(0b00001000, 0x40);
			bmx160_set(BMX160_CMD_ACCEL_PM_NORMAL, BMX160_REG_CMD);
			sleep_us(us_5ms);
			bmx160_set(0b00101000, 0x42);
			bmx160_set(BMX160_CMD_GYRO_PM_NORMAL, BMX160_REG_CMD);
			sleep_us(us_100ms);
			bmx160_set(BMX160_CMD_MAG_PM_NORMAL, BMX160_REG_CMD);
			sleep_us(us_2ms);
			// BMX160 2.4.3.1.3 magnetometer Configuration Example
			bmx160_set(0x80, 0x4C); // mag_if0, setup mode
			bmx160_set(0x01, 0x4F); // mag_if3, indirect write
			bmx160_set(0x4B, 0x4E); // mag_if2, sleep mode
			bmx160_set(0x17, 0x4F); // mag_if3, indirect write
			bmx160_set(0x51, 0x4E); // mag_if2, regular preset XY
			bmx160_set(0x52, 0x4F); // mag_if3, indirect write
			bmx160_set(0x52, 0x4E); // mag_if2, regular preset Z
			bmx160_set(0x02, 0x4F); // mag_if3, data set
			bmx160_set(0x4C, 0x4E); // mag_if2, data set
			bmx160_set(0x42, 0x4D); // mag_if1, data set
			bmx160_set(0x05, 0x44); // 12.5Hz pool rate
			bmx160_set(0x00, 0x4C); // mag_if0, data mode
			bmx160_set(BMX160_CMD_MAG_PM_NORMAL, BMX160_REG_CMD);
			sleep_us(us_2ms);
			imu_status = bmx160_get(2, BMX160_PS_REG); // read power status
			printf("BMX160 IMU Sensors Configuration Complete. Chip Status: %02hhX.\n", imu_status);
			if (bmx160_get(2, BMX160_NV_CONF) != BMX160_SPI_SET) {
				bmx160_set(BMX160_SPI_SET, BMX160_NV_CONF);
				bmx160_set(BMX160_SPI_4WIRE, BMX160_IF_CONF);
				printf("BMX160 Interface set to SPI in NVRAM.\n");
			}
		}
	} else {
		printf("BMX160 IMU NOT detected, Bad Chip ID %02hhX.\n", imu_id);
	}

	if ((imu_id == BMX160_ID) && (imu_status == BMX160_ALL_PM_NORMAL)) {
		do {
			setup_bmx160_transfer(24); // 24 byte transfer, address and 23 data registers
			sleep_us(us_10ms);
			data_status = bmx160_get(2, BMX160_STATUS_REG);
			bmx160_get(24, BMX160_DATA_REG);
			//			show_bmx160_transfer();
			getAllData(&magn, &gyro, &accel);
			printf("\rBMX160 IMU: M %7.3f %7.3f %7.3f, G %7.3f %7.3f %7.3f, A %7.3f %7.3f %7.3f  %02hhX  \r", magn.x, magn.y, magn.z, gyro.x, gyro.y, gyro.z, accel.x, accel.y, accel.z, data_status);
			/* Write to the pipe */
			sprintf(fifo_buf, "%7.3f,%7.3f,%7.3f,%7.3f,%7.3f,%7.3f,%7.3f,%7.3f,%7.3f\n", magn.x, magn.y, magn.z, gyro.x, gyro.y, gyro.z, accel.x, accel.y, accel.z);
			write(pipefd, fifo_buf, sizeof(fifo_buf));
			/*
			 * handle the MC33966 chip MCP2210 SPI setting
			 */
			setup_mc33996_transfer(); // CS 7 and mode 1
			/*
			 * handle the MC33966 chip setting
			 */
			mc33996_init();
			/*
			 * SPI data to update the MC33966 outputs
			 */
			mc33996_update();
		} while (true);
	}

	close(pipefd);

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
		 * handle the MC33966 chip MCP2210 SPI setting
		 */
		setup_mc33996_transfer(); // CS 7 and mode 1
		/*
		 * handle the MC33966 chip setting
		 */
		mc33996_init();
		/*
		 * SPI data to update the MC33966 outputs
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
