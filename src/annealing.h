#ifndef ANNEALING_H__
#define ANNEALING_H__

#include <vector>
#include <stdlib.h>
#include "milpData.h"

void annealing_run();

static inline int loc_to_id(uint8_t b)
{
	register int counter = -1;

	while (b != 0) {
		++counter;
		b = b >> 1;
	}
	return counter;
}

#endif
