/* 
 * File:   gfx.h
 * Author: root
 *
 * Created on October 30, 2020, 9:35 AM
 */

#ifndef GFX_H
#define	GFX_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "imu.h"
#include <math.h>
#include "OledGrph.h"
#include "../pic32mk_mcj_curiosity_pro.X/MadgwickAHRS/MadgwickAHRS.h"

	static const double sine_res = 3600.0;
	static const double sinea = 0.0;
	static const double sineb = 1200.0;
	static const double sinec = 2400.0;

	extern int32_t xa, ya, za;

	void line_rot(uint32_t, uint32_t, uint32_t, uint32_t);
	void vector_graph(void);
	void LA_gfx(bool, bool, uint32_t);

#ifdef	__cplusplus
}
#endif

#endif	/* GFX_H */

