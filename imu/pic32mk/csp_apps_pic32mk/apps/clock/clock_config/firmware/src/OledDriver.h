/************************************************************************/
/*																		*/
/*	OledDriver.h -- Interface Declarations for OLED Display Driver 		*/
/*																		*/
/************************************************************************/
/*	Author:		Gene Apperson											*/
/*	Copyright 2011, Digilent Inc.										*/
/************************************************************************/
/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
/************************************************************************/
/*  File Description:													*/
/*																		*/
/*	Interface to OledDriver.c											*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	04/29/2011(GeneA): created											*/
/*																		*/
/************************************************************************/
#include <stdint.h>
#include "dogm-graphic.h"

#if !defined(OLEDDRIVER_INC)
#define	OLEDDRIVER_INC

/* ------------------------------------------------------------ */
/*					Miscellaneous Declarations					*/
/* ------------------------------------------------------------ */
#if DISPLAY_TYPE == 240                 // Autor: Robert Steigemann
#define	cbOledDispMax		3840		//max number of bytes in display buffer
#define	ccolOledMax		240l		//number of display columns
#define	crowOledMax		128l		//number of display rows
#define	cpagOledMax		16		//number of display memory pages
#define STR_BUF_SIZE		160		//number of chars for display strings
#endif

#if DISPLAY_TYPE == 102
#define	cbOledDispMax		816		//max number of bytes in display buffer
#define	ccolOledMax		102		//number of display columns
#define	crowOledMax		64		//number of display rows
#define	cpagOledMax		8		//number of display memory pages
#define STR_BUF_SIZE		20		//number of chars for display strings
#endif

#define	cbOledChar		8		//font glyph definitions is 8 bytes long
#define	chOledUserMax		0x20	//number of character defs in user font table
#define	cbOledFontUser		(chOledUserMax*cbOledChar) // used for array size

/* Graphics drawing modes.
 */
#define	modOledSet		0
#define	modOledOr		1
#define	modOledAnd		2
#define	modOledXor		3

/* ------------------------------------------------------------ */
/*					General Type Declarations					*/
/* ------------------------------------------------------------ */

/* Pin definitions for access to OLED control signals on ChipKitUno
 */


/* ------------------------------------------------------------ */
/*					Object Class Declarations					*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*					Variable Declarations						*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*					Procedure Declarations						*/
/* ------------------------------------------------------------ */

void OledInit(void);
void OledTerm(void);
void OledDisplayOn(void);
void OledDisplayOff(void);
void OledClear(void);
void OledClearBuffer(void);
void OledUpdate(void);

/* ------------------------------------------------------------ */

#endif

/************************************************************************/
