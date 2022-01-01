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
#include "tictest.h"

uint32_t fspeed = 20000; // led movement speed
sBmx160SensorData_t magn, gyro, accel;
static const char *build_date = __DATE__, *build_time = __TIME__;
static char fifo_buf[256];
static uint8_t stat_buf[16];

const uint8_t led_pattern[0x10] = {
	0b00000001,
	0b00000010,
	0b00000100,
	0b00001000,
	0b00010000,
	0b00100000,
	0b01000000,
	0b10000000,
	0b10000000,
	0b01000000,
	0b00100000,
	0b00010000,
	0b00001000,
	0b00000100,
	0b00000010,
	0b00000001,
};

static void do_switch_state(void);

int main(int argc, char* argv[])
{
	mcp2210_spi_type* mcp2210;
	uint8_t imu_id = 0, imu_status = 0, data_status = 0, k = 0, j = 0;
	int16_t imu_temp_raw = 0;
	double imu_temp_c = 0.0;
	uint8_t data_int[3] = {0, 0, 0};

	int pipefd;
	char * myfifo = "/tmp/myfifo";

	printf("\r--- MCP2210 USB To SPI Testing Version %s %s %s ---\r\n", MCP2210_VERSION, build_date, build_time);
	/*
	 * init the hidraw device interface
	 */
	if (hid_init()) {
		printf("hidapi init error.\n");
		return false;
	}

	/*
	 * setup the hidraw* device to communicate with the MCP2210
	 * return hidapi handle
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
	bmx160_version();
	setup_bmx160_transfer(BMX160_REG_LEN); // 2 byte transfer, address and one data register
	bmx160_get(BMX160_REG_LEN, BMX160_REG_DUMMY); // toggle CS to set bmx160 SPI mode
	// check for bmx160 boot status and configure if needed
	if ((imu_id = bmx160_get(BMX160_REG_LEN, BMX160_ID_REG)) == BMX160_ID) {
		imu_status = bmx160_get(BMX160_REG_LEN, BMX160_PS_REG); // read power status
		printf("BMX160 IMU detected, Chip ID %02hhX, Chip Power Status %02hhX.\n", imu_id, imu_status);
		if (imu_status == BMX160_ALL_PM_NORMAL) {
			printf("BMX160 IMU Sensors All Operational.\n");
		} else {
			printf("BMX160 IMU Sensors Not Ready.\n");
			printf("BMX160 IMU Sensors Configuration Started. Chip Power Status: %02hhX.\n", imu_status);

			bmx160_set(0x88, 0x53); // enable interrupt pins, active low
			data_int[0] = 0x04;
			data_int[1] = 0x08;
			bmx160_set3(data_int, 0x55); // map interrupt
			data_int[0] = 0x00;
			data_int[1] = 0x10;
			bmx160_set3(data_int, 0x50); // enable data interrupt
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
			bmx160_set(0x51, 0x4E); // mag_if2, high preset XY
			bmx160_set(0x52, 0x4F); // mag_if3, indirect write
			bmx160_set(0x52, 0x4E); // mag_if2, high preset Z
			bmx160_set(0x02, 0x4F); // mag_if3, data set
			bmx160_set(0x4C, 0x4E); // mag_if2, data set
			bmx160_set(0x42, 0x4D); // mag_if1, data set
			bmx160_set(0x05, 0x44); // 12.5Hz pool rate
			bmx160_set(0x00, 0x4C); // mag_if0, data mode
			bmx160_set(BMX160_CMD_MAG_PM_NORMAL, BMX160_REG_CMD);
			sleep_us(us_2ms);
			imu_status = bmx160_get(BMX160_REG_LEN, BMX160_PS_REG); // read power status
			data_status = bmx160_get(BMX160_REG_LEN, BMX160_STATUS_REG);
			printf("BMX160 IMU Sensors Configuration Complete. Chip Status: %02hhX.\n", imu_status);
			if (bmx160_get(BMX160_REG_LEN, BMX160_NV_CONF) != BMX160_SPI_SET) {
				bmx160_set(BMX160_SPI_SET, BMX160_NV_CONF);
				bmx160_set(BMX160_SPI_4WIRE, BMX160_IF_CONF);
				printf("BMX160 Interface set to SPI in NVRAM.\n");
			}
		}
	} else {
		printf("BMX160 IMU NOT detected, Bad Chip ID %02hhX.\n", imu_id);
	}

	/*
	 * Check for proper mc33996 output chip SPI response
	 */
	mc33996_version();
	setup_mc33996_transfer(6);
	mc33996_set(mc33996_reset, mc33996_magic_h, mc33996_magic_l);
	if (mc33996_check()) {
		printf("MC33996 detected, Reset Magic Words Match.\n");
	} else {
		printf("MC33996 NOT detected. No or Wrong Magic Response.\n");
	}

	/*
	 * handle the TIC12400 input chip MCP2210 SPI setting
	 */
	tic12400_version();
	setup_tic12400_transfer();
	/*
	 * init and program 24 switch inputs from the TIC12400 chip
	 */
	tic12400_reset();

	if (!tic12400_init()) {
		printf("TIC12400 detection and setup failed code %i\n", tic12400_fail_value);
		//		return 0;
	} else {
		printf("TIC12400 detected and configured.\n");
		printf("TIC12400 Chip ID %X \n", tic12400_status & 0b111111111111);
	}
	get_MCP2210_ext_interrupt(); // read switch data
	tic12400_read_sw(0, 0);
	do_switch_state();
	setup_bmx160_transfer(BMX160_DATA_LEN); // byte transfer, address and data registers

	if ((imu_id == BMX160_ID) && ((imu_status == BMX160_ALL_PM_NORMAL) || (imu_status == BMX160_ALL_PM_OK))) {
		do {
			sleep_us(fspeed);
			bmx160_get(BMX160_DATA_LEN, BMX160_DATA_REG);
			getAllData(&magn, &gyro, &accel);
			move_bmx160_transfer_status(stat_buf);
			imu_temp_raw = stat_buf[5] + (stat_buf[6] << 8); // bmx160 chip temp to int16_t
			imu_temp_c = ((double) imu_temp_raw * BMX160_TEMP_SCALAR) + 23.0; // bmx160 chip temp to C
			printf("\rBMX160 IMU: M %7.3f %7.3f %7.3f, G %7.3f %7.3f %7.3f, A %7.3f %7.3f %7.3f  %02hhX %5.2fC %02hhX \r", magn.x, magn.y, magn.z, gyro.x, gyro.y, gyro.z, accel.x, accel.y, accel.z,
				data_status, imu_temp_c, stat_buf[2]);
			/* Write to the pipe */
			snprintf(fifo_buf, 255, "%7.3f,%7.3f,%7.3f,%7.3f,%7.3f,%7.3f,%7.3f,%7.3f,%7.3f\n", magn.x, magn.y, magn.z, gyro.x, gyro.y, gyro.z, accel.x, accel.y, accel.z);
			write(pipefd, fifo_buf, sizeof(fifo_buf));
			/*
			 * handle the MC33966 chip MCP2210 SPI setting
			 */
			//			setup_mc33996_transfer(3);
			/*
			 * send data to the output ports
			 */
			//			mc33996_set(mc33996_control, led_pattern[k & 0x0f], led_pattern[j & 0x0f]);
			/*
			 * check for change in MCP2210 interrupt counter
			 */
			if (get_MCP2210_ext_interrupt()) {
				/*
				 * handle the TIC12400 chip MCP2210 SPI setting
				 */
				setup_tic12400_transfer(); // CS 5 and mode 1
				/*
				 * read 24 switch inputs
				 */
				tic12400_read_sw(0, 0);
				/*
				 * look for switch 0 changes for led speeds
				 */
				do_switch_state();
				printf("tic12400 switch value %X , status %X \n", tic12400_value, tic12400_status);
				setup_bmx160_transfer(BMX160_DATA_LEN); // byte transfer, address and data registers
			} else {
				//+				fspeed = abs((int32_t) (magn.z * 1000.0));
			}
			k++;
			j--;
		} while (true);
	}

	close(pipefd);
	hid_close(mcp2210->handle);
	hid_exit(); /* Free static HIDAPI objects. */
	return 0;
}

void do_switch_state(void)
{
	if (tic12400_get_sw() & raw_mask_0) {
		fspeed = abs((int32_t) (magn.x * 1000.0));
	} else {
		fspeed = us_50ms;
	}
}

