/**
 * @file    newlib_stubs.c
 * @author  Winfred Lu
 * @brief   Provide the required stubs for newlib.
 */
#include <errno.h>
#include <sys/unistd.h>

#include "stm32f4xx_hal.h"

void _exit(int status)
{
	while (1) {
	}
}

/**
 * @brief   Increase program data space.
 * @param   incr:
 * @retval  current heap end or -1 when error
*/
caddr_t _sbrk(int incr)
{
	extern char _end;
	static char *heap_end;
	char *prev_heap_end;

	if (heap_end == 0) {
		heap_end = &_end;
	}
	prev_heap_end = heap_end;

	char * stack = (char*) __get_MSP();
	if (heap_end + incr >  stack) {
		errno = ENOMEM;
		return  (caddr_t) -1;
	}

	heap_end += incr;
	return (caddr_t) prev_heap_end;
}
