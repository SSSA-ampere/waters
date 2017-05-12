#ifndef GENETIC_H__
#define GENETIC_H__

#include <vector>
#include <cstdlib>

#include "milpData.h"
#include "optimization.h"

typedef std::vector<std::pair<Solution, double> > GeneticPopulation;

extern unsigned int MEM_POP_SIZE;
extern GeneticPopulation memory_population;

void genetic_run();
Solution getRandomSolution_waters_GA();
unsigned int get_genes_size_waters_GA();
Solution crossover_waters_GA(const Solution &a, const Solution &b);
void mutatate_chromosome_waters_GA(Solution &s);

static inline int loc_to_id_g(uint8_t b)
{
	register int counter = -1;

	while (b != 0) {
		++counter;
		b = b >> 1;
	}
	return counter;
}

Solution solution_from_csv(const std::string &fileName,
                           unsigned int to_skip = 3);

#endif
