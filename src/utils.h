#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <inttypes.h>

#define CHUNK_SIZE(ptr) *((uint64_t *) ((ptr) - sizeof(uint64_t)))

/*
  ptr1 is greater than ptr2 -> 1 <= cmp
  ptr2 is greater than ptr1 -> -1 >= cmp
  ptr1 and ptr2 are equal -> 0 = cmp
 */
#define CHUNK_CMP(ptr1, ptr2) ((int64_t) ((CHUNK_SIZE(ptr1)) - (CHUNK_SIZE(ptr2))))

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

#define DBG_LOG_PREFIX "[DBG-MEM]: "
#define LOG_DBG_INF(msg, ...) do {				\
		char buff[1024];					\
		bzero(buff, 1024);					\
		strcat(buff, DBG_LOG_PREFIX);				\
		sprintf(buff + strlen(buff), msg __VA_OPT__(,) __VA_ARGS__); \
		fprintf(stdout, "%s\n", buff);				\
	} while (0)							\


#endif
