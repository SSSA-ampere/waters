#ifndef GENETIC_H__
#define GENETIC_H__

#include <vector>
#include <cstdlib>
#include "milpData.h"

#define pop 500

void genetic_run();

static inline int loc_to_id_g(uint8_t b)
{
	register int counter = -1;

	while (b != 0) {
		++counter;
		b = b >> 1;
	}
	return counter;
}

#endif
