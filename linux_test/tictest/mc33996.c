/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "mc33996.h"
#include "mcp2210.h"

extern mcp2210_spi_type *S; // working I/O structure pointer

void mc33996_init(void)
{
	cbufs();
	// MCP33996 config
	S->buf[0] = 0x42; // transfer SPI data command
	S->buf[1] = 3; // no. of SPI bytes to transfer
	S->buf[4] = 0x00; // on/off control
	S->buf[5] = 0x00;
	S->buf[6] = 0x00; // set all outputs to low
	S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);
};

void setup_mc33996_transfer(void)
{
	cbufs();
	S->buf[0] = 0x40; // SPI transfer settings command
	S->buf[4] = 0x40; // set SPI transfer bit rate;
	S->buf[5] = 0x4b; // 32 bits, lsb = buf[4], msb buf[7]
	S->buf[6] = 0x4c; // 5MHz
	S->buf[7] = 0x00;
	S->buf[8] = 0xff; // set CS idle values to 1
	S->buf[9] = 0x01;
	S->buf[10] = 0b11111111; // set CS active values to 0, set the rest to 1
	S->buf[11] = 0b00000000;
	S->buf[18] = 0x03; // set no of bytes to transfer = 3
	S->buf[20] = 0x01; // spi mode 1
	S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);
};

void get_mc33996_transfer(void)
{
	// ---------- Get SPI transfer settings (0x41)-------------
	cbufs();
	S->buf[0] = 0x41; // 0x41 Get SPI transfer settings
	S->res = SendUSBCmd(S->handle, S->buf, S->rbuf);
	printf("SPI MC33996 transfer settings\n   "); // Print out the 0x41 returned buffer.
	for (int i = 0; i < S->rbuf[2]; i++) {
		printf("%02hhx ", S->rbuf[i]);
	}
	printf("\n");
};

void mc33996_update(void)
{
	S->buf[4] = 0x00;
};

