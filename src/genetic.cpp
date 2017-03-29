// Genetic algorithm

#include "genetic.h"
#include "RT.h"
#include "milpData.h"

#include <vector>
#include <cmath>
#include <iostream>
#include <random>
#include <chrono>
#include <algorithm>

using namespace std;

std::vector <RAM> ramg[pop];
typedef std::vector<Label> Solution;


void evaluatePopulation(Solution s[], double fit[])
{
	for (unsigned int p = 0; p < pop; ++p) {
		fit[p] = computeResponseTime(s[p]);
	}	
}

void selectSurvivors(Solution s[], double fit[])
{

}


void InitializePopulation(Solution s[])
{
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> distribution(0, 4);
	unsigned int position;
	RAM ram_temp;

	for (unsigned int p = 0; p < pop; ++p) {

		// Initialize rams
		for (int i = 0; i < 4; i++) {
			ram_temp.size = 131072;
			ram_temp.available = 131072;
			ramg[p].push_back(ram_temp);
		}			
		ram_temp.size = 131072 * 2;
		ram_temp.available = 131072 * 2;
		ramg[p].push_back(ram_temp);

		s[p] = labels;

		for (unsigned int i = 0; i<s[p].size(); ++i) {
			do {
				position = distribution(generator);
			} while (ramg[p].at(position).available - s[p].at(i).bitLen < 0);

			ramg[p].at(position).available -= s[p].at(i).bitLen;
			s[p].at(i).ram = static_cast<RAM_LOC>(1 << position);
		}
	}	
}


std::pair<Solution, double> genetic()
{
	unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> dist_eval(0, 100);

	Solution s[pop], s_opt;
	double fit[pop], fit_opt;

	InitializePopulation(s);
	evaluatePopulation(s,fit);

	do {
		selectSurvivors(s, fit);

		selectParents();

		performReproduction();

		performMutation();

		updatePopulation();

		evaluatePopulation(s, fit);



	} while (termination != 0)

	return std::make_pair(s_opt, fit_opt);
}

static void printSolution(const Solution &s)
{
	unsigned int labels_in_memory[5];

	for (unsigned int i = 0; i<5; ++i)
		labels_in_memory[i] = 0;

	for (Label const &l : s)
		++labels_in_memory[loc_to_id_g(l.ram)];

	for (unsigned int i = 0; i<5; ++i)
		cout << "[" << labels_in_memory[i] << "]";
}


void genetic_run()
{
	cout << endl << "Performing genetic algorithm" << endl << endl;
	auto r = genetic();
	cout << "Done" << endl;
	cout << "Best solution found" << endl;
	printSolution(r.first);
	cout << "With value: " << r.second << endl;
}

