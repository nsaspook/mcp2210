#include "mcp2210.h"

static wchar_t wstr[MAX_STR]; // buffer for id settings strings from MPC2210

static mcp2210_spi_type SPI_buffer; //  MCP2210 I/O structure
mcp2210_spi_type *S = &SPI_buffer; // working I/O structure pointer
static const char *build_date = __DATE__, *build_time = __TIME__;

void cbufs(void)
{
	memset(S->buf, 0, sizeof(S->buf)); // initialize bufs to zeros
	memset(S->rbuf, 0, sizeof(S->rbuf));
}

int32_t SendUSBCmd(hid_device *handle, uint8_t *cmdBuf, uint8_t *responseBuf)
{
	int32_t r;

	r = hid_write(handle, cmdBuf, COMMAND_BUFFER_LENGTH);
	if (r < 0) {
		return ERROR_UNABLE_TO_WRITE_TO_DEVICE;
	}

	//when the hid device is configured as synchronous, the first
	//hid_read returns the desired results. and the while() loop
	//is skipped.
	//
	//when the hid device is configured as asynchronous, the first
	//hid_read may or may not succeed, depending on the latency
	//of the attached device. When no data is returned, r = 0 and
	//the while loop keeps polling the returned data until it is
	//received.
	r = hid_read(handle, responseBuf, RESPONSE_BUFFER_LENGTH);
	if (r < 0) {
		return ERROR_UNABLE_TO_READ_FROM_DEVICE;
	}

	while (r == 0) {
		r = hid_read(handle, responseBuf, RESPONSE_BUFFER_LENGTH);
		if (r < 0) {
			return ERROR_UNABLE_TO_READ_FROM_DEVICE;
		}
		sleep_us(10);
	}

	return responseBuf[1];
}

int32_t nanosleep(const struct timespec *, struct timespec *);

void sleep_us(const uint32_t microseconds)
{
	struct timespec ts;
	ts.tv_sec = microseconds / 1000000; // whole seconds
	ts.tv_nsec = (microseconds % 1000000) * 1000; // remainder, in nanoseconds
	nanosleep(&ts, NULL);
}

/*
 * when connected to the TIC12400 interrupt pin it shows a switch has changed state
 */
bool get_MCP2210_ext_interrupt(void)
{
#ifdef EXT_INT_DPRINT
	static uint32_t counts = 0;
#endif

	cbufs();
	S->buf[0] = 0x12; // Get (VM) the Current Number of Events From the Interrupt Pin, GPIO 6 FUNC2
	S->buf[1] = 0x00; // reads, then resets the event counter
	S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);
	if (S->rbuf[4] || S->rbuf[5]) {
#ifdef EXT_INT_DPRINT
		printf("rbuf4 %x: rbuf5 %x: counts %i\n", S->rbuf[4], S->rbuf[5], ++counts);
#endif
		return true;
	}
	return false;
}

int32_t cancel_spi_transfer(void)
{
	cbufs();
	S->buf[0] = 0x11; // 0x11 cancel SPI transfer
	S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);
	return S->res;
}

bool SPI_WriteRead(hid_device *handle, uint8_t *buf, uint8_t *rbuf)
{
	S->res = SendUSBCmd(handle, buf, rbuf);
	while (rbuf[3] == SPI_STATUS_STARTED_NO_DATA_TO_RECEIVE || rbuf[3] == SPI_STATUS_SUCCESSFUL) {
		S->res = SendUSBCmd(handle, buf, rbuf);
	}
	return true;
}

bool SPI_MCP2210_WriteRead(uint8_t* pTransmitData, const size_t txSize, uint8_t* pReceiveData, const size_t rxSize)
{
#ifdef DPRINT
	static uint32_t tx_count = 0;
#endif

	cbufs();
	S->buf[0] = 0x42; // transfer SPI data command
	S->buf[1] = rxSize; // no. of SPI bytes to transfer
	S->buf[4] = pTransmitData[3];
	S->buf[5] = pTransmitData[2];
	S->buf[6] = pTransmitData[1];
	S->buf[7] = pTransmitData[0];
	S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);
#ifdef DPRINT
	printf("TX SPI res %x - tx count %i\n", S->res, ++tx_count);
#endif

#ifdef DPRINT
	uint32_t rcount = 0;
#endif
	while (S->rbuf[3] == SPI_STATUS_STARTED_NO_DATA_TO_RECEIVE || S->rbuf[3] == SPI_STATUS_SUCCESSFUL) {
#ifdef DPRINT
		printf("SPI RX wait %i: code %x\n", ++rcount, S->rbuf[3]);
#endif
		S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);
	}
	pReceiveData[3] = S->rbuf[4];
	pReceiveData[2] = S->rbuf[5];
	pReceiveData[1] = S->rbuf[6];
	pReceiveData[0] = S->rbuf[7];
	return true;
}

/*
 * open the USB device with a optional serial number
 */
mcp2210_spi_type* hidrawapi_mcp2210_init(const wchar_t *serial_number)
{
	printf("\r\n--- Driver Version %s %s %s ---\r\n",MCP2210_DRIVER,build_date,build_time);
	cbufs(); // clear command and response buffers
	//------------------ Open MCP2210 and display info ---------------
	printf("Open Device: 04d8:00de\n");
	// Open the device using the VID(vendor ID, PID(product ID),
	// and optionally the Serial number.
	S->handle = hid_open(0x4d8, 0xde, serial_number); // open the MCP2210 device
	if (!S->handle) {
		printf("unable to open the MCP2210\n");
		return NULL;
	}
	// Read the Manufacturer String
	wstr[0] = 0x0000;
	S->res = hid_get_manufacturer_string(S->handle, wstr, MAX_STR);
	if (S->res < 0) {
		printf("Unable to read manufacturer string\n");
	}
	printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	wstr[0] = 0x0000;
	S->res = hid_get_product_string(S->handle, wstr, MAX_STR);
	if (S->res < 0) {
		printf("Unable to read product string\n");
	}
	printf("Product String: %ls\n", wstr);

	// Read the Serial Number String
	wstr[0] = 0x0000;
	S->res = hid_get_serial_number_string(S->handle, wstr, MAX_STR);
	if (S->res < 0) {
		printf("Unable to read serial number string\n");
	}
	printf("Serial Number String: (%d) %ls", wstr[0], wstr);
	printf("\n");

	// Read Indexed String 1
	wstr[0] = 0x0000;
	S->res = hid_get_indexed_string(S->handle, 1, wstr, MAX_STR);
	if (S->res < 0) {
		printf("Unable to read indexed string 1\n");
	}
	printf("Indexed String 1: %ls\n\n", wstr);
	hid_set_nonblocking(S->handle, 1); // async operation, don't block

	//-------------- Set GPIO pin function (0x21) -------------
	// configure chip selects and interrupts for all devices on the SPI buss
	cbufs();
	S->buf[0] = 0x21; // command 21 - set GPIO pin's functions
	S->buf[4] = 0x01; // GPIO 0 set to 0x01 - SPI CS, 25LC020
	S->buf[5] = 0x01; // GPIO 1 set to 0x01 - SPI CS, MCP3204 
	S->buf[6] = 0x00; // GPIO 2
	S->buf[7] = 0x02; // act led
	S->buf[8] = 0x01; // GPIO 4 set to 0x01 - SPI CS, mcp23s08
	S->buf[9] = 0x01; // GPIO 5 set to 0x01 - SPI CS, tic12400
	S->buf[10] = 0x02; // GPIO 6 external interrupt input
	S->buf[11] = 0x02; // GPIO 7 set to 0x01 - SPI CS, TC77
	S->buf[12] = 0x00; // GPIO 8
	S->buf[15] = 0b01000000; // set GPIO 6 to input
	S->buf[17] = 0b00000010; // count Falling edges
	S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);

	// ------------ Set GPIO pin direction (0x32)--------------
	cbufs();
	S->buf[0] = 0x32; // command 32 - set GPIO pin's directions
	// function:  0 = output, 1 = input
	S->buf[4] = 0b01000000; // set GPIO 0-5,7 to outputs, GPIO 6 for input
	S->buf[5] = 0x00; // set GPIO 8 to output
	S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);

	// ------------ Set GPIO pin level (0x30)--------------
	cbufs();
	S->buf[0] = 0x30; // command 30 - set GPIO pin's level to all high
	S->buf[4] = 0xff;
	S->buf[5] = 0xff;
	S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);

	cancel_spi_transfer(); // reset the SPI engine
	printf("Ctrl c to exit\n"); // ctrl c to exit blink loop and exit program
	return S;
}

/*
 * chip setup via SPI data transfers
 */
void mcp23s08_init(void)
{
	cbufs();
	// MCP23S08 config
	S->buf[0] = 0x42; // transfer SPI data command
	S->buf[1] = 3; // no. of SPI bytes to transfer
	S->buf[4] = 0x40; //device address is 01000A1A0, write
	S->buf[5] = 0x00; //write to IODIR register,
	S->buf[6] = 0x00; //set all outputs to low
	S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);
}

void setup_mcp23s08_transfer(void)
{
	cbufs();
	S->buf[0] = 0x40; // SPI transfer settings command
	S->buf[4] = 0x40; // set SPI transfer bit rate;
	S->buf[5] = 0x4b; // 32 bits, lsb = buf[4], msb buf[7]
	S->buf[6] = 0x4c; // 5MHz
	S->buf[7] = 0x00;
	S->buf[8] = 0xff; // set CS idle values to 1
	S->buf[9] = 0x01;
	S->buf[10] = 0b11101111; // set CS active values to 0, set the rest to 1
	S->buf[11] = 0b00000001;
	S->buf[18] = 0x03; // set no of bytes to transfer = 3
	S->buf[20] = 0x00; // spi mode 0
	S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);
}

void setup_tic12400_transfer(void)
{
	cbufs();
	S->buf[0] = 0x40; // SPI transfer settings command
	S->buf[4] = 0x00; // set SPI transfer bit rate;
	S->buf[5] = 0x09; // 32 bits, lsb = buf[4], msb buf[7]
	S->buf[6] = 0x3d; // 4MHz
	S->buf[7] = 0x00;
	S->buf[8] = 0xff; // set CS idle values to 1
	S->buf[9] = 0x01;
	S->buf[10] = 0b11011111; // set CS active values to 0, set the rest to 1
	S->buf[11] = 0b00000001;
	S->buf[18] = 0x4; // set no of bytes to transfer = 4 // 32-bit transfers
	S->buf[20] = 0x01; // spi mode 1
	S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);
}

void get_mcp23s08_transfer(void)
{
	// ---------- Get SPI transfer settings (0x41)-------------
	cbufs();
	S->buf[0] = 0x41; // 0x41 Get SPI transfer settings
	S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);
	printf("SPI MCP23S08 transfer settings\n   "); // Print out the 0x41 returned buffer.
	for (int i = 0; i < S->rbuf[2]; i++) {
		printf("%02hhx ", S->rbuf[i]);
	}
	printf("\n");
}

/*
 * setup SPI command for GPIO updates
 */
void mcp23s08_update(void)
{
	S->buf[4] = 0x40;
	S->buf[5] = 0x0a;
}

void get_tic12400_transfer(void)
{
	// ---------- Get SPI transfer settings (0x41)-------------
	cbufs();
	S->buf[0] = 0x41; // 0x41 Get SPI transfer settings
	S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);
	printf("SPI TIC12400 transfer settings\n   "); // Print out the 0x41 returned buffer.
	for (int i = 0; i < S->rbuf[2]; i++) {
		printf("%02hhx ", S->rbuf[i]);
	}
	printf("\n");
}