#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#define KILOBYTE 1024
#define MEGABYTE (1024 * KILOBYTE)
#define GIGABYTE (1024 * MEGABYTE)

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))


#define DBG_LOG_PREFIX "[DBG-MEM]: "
#define LOG_DBG_INF(msg, ...) do {					\
		char buff[1024];					\
		bzero(buff, 1024);					\
		sprintf(buff, (msg) __VA_OPT__(, __VA_ARGS__));		\
		fprintf(stdout, "%s%s\n", DBG_LOG_PREFIX, buff);	\
	} while (0)							\


static inline uint64_t aling_to_mul_4kb(uint64_t n)
{
	if (n % 4096 == 0)
		return n;
	return n + (4096 - (n % 4096));
}

static inline uint64_t aling_to_mul_8(uint64_t n)
{
	if (n % 8 == 0)
		return n;
	return n + (8 - (n % 8));
}



#endif
