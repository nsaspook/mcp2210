#include "mcp2210.h"

static const char *build_date = __DATE__, *build_time = __TIME__;

void mc33996_version(void)
{
	printf("\r--- MC33996 Driver Version %s %s %s ---\r\n", MC33996_DRIVER, build_date, build_time);
}