/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   mcp2210.h
 * Author: root
 *
 * Created on May 31, 2021, 9:53 AM
 */

#ifndef MCP2210_H
#define MCP2210_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <hidapi/hidapi.h>
#include "tictest.h"
#include "tic12400.h"
#include "mc33996.h"

#define MCP2210_DRIVER "V0.6"

#define MAX_STR 255
#define OPERATION_SUCCESSFUL 0
#define ERROR_UNABLE_TO_OPEN_DEVICE -1
#define ERROR_UNABLE_TO_WRITE_TO_DEVICE -2
#define ERROR_UNABLE_TO_READ_FROM_DEVICE -3
#define ERROR_INVALID_DEVICE_HANDLE -99

#define COMMAND_BUFFER_LENGTH 64
#define RESPONSE_BUFFER_LENGTH 64

#define SPI_STATUS_FINISHED_NO_DATA_TO_SEND 0x10
#define SPI_STATUS_STARTED_NO_DATA_TO_RECEIVE 0x20
#define SPI_STATUS_SUCCESSFUL 0x30

#define MCP23s08_DATA 6
#define MCP23s08_DATA_LEN 8

	/*
	 * HIDAPI I/O structure
	 */
	typedef struct {
		hid_device *handle;
		struct hid_device_info *devs, *cur_dev;
		uint8_t buf[COMMAND_BUFFER_LENGTH]; // command buffer written to MCP2210
		uint8_t rbuf[RESPONSE_BUFFER_LENGTH]; // response buffer
		int32_t res; // # of bytes sent from hid_read(), hid_write() functions
		uint8_t offbuf[COMMAND_BUFFER_LENGTH]; // command buffer written to MCP2210
	} mcp2210_spi_type;

	void cbufs();
	int32_t SendUSBCmd(hid_device *, uint8_t *, uint8_t *);
	void sleep_us(const uint32_t);
	bool get_MCP2210_ext_interrupt(void);
	int32_t cancel_spi_transfer(void);
	bool SPI_WriteRead(hid_device *, uint8_t *, uint8_t *);
	bool SPI_MCP2210_WriteRead(uint8_t* pTransmitData, const size_t txSize, uint8_t* pReceiveData, const size_t rxSize);
	void setup_tic12400_transfer(void);
	void get_tic12400_transfer(void);
	void setup_mcp23s08_transfer(void);
	void get_mcp23s08_transfer(void);
	void mcp23s08_init(void);
	void mcp23s08_update(void);
	void mc33996_init(void);
	void setup_mc33996_transfer(void);
	void get_mc33996_transfer(void);
	void mc33996_update(void);
	mcp2210_spi_type* hidrawapi_mcp2210_init(const wchar_t *serial_number);


#ifdef __cplusplus
}
#endif

#endif /* MCP2210_H */

