// Genetic algorithm

#include "genetic.h"
#include "RT.h"
#include "milpData.h"
#include "annealing.h"

#include <vector>
#include <cmath>
#include <iostream>
#include <random>
#include <chrono>
#include <algorithm>
#include <ctime>

using namespace std;

std::vector <RAM> ramg[pop];
typedef std::vector<Label> Solution;


void sortPopulation(Solution s[], double fit[])
{
	Solution appo;
	double appo_d;
	double min;
	unsigned int index;

	for (unsigned int i=0; i<pop; ++i) {
		min = fit[i];
		index = i;

		for (unsigned int j=i; j<pop; j++) {
			if (fit[j] < min) {
				min = fit[j];
				index = j;
			}
		}
		appo_d = fit[index];
		fit[index] = fit[i];
		fit[i] = appo_d;

		appo = s[index];
		s[index] = s[i];
		s[i] = appo;
	}

}

void evaluatePopulation(Solution s[], double fit[])
{
	for (unsigned int p = 0; p < pop; ++p) {
		fit[p] = computeResponseTime(s[p]);
	}

	sortPopulation(s, fit);
}

Solution ComputeNewSolutionLight(const Solution &s, unsigned int maximum)
{
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> dist_label(0, s.size()-1);
	std::uniform_int_distribution<int> dist_ram(0, 4);
	std::uniform_int_distribution<int> dist_noise(1, maximum);
	Solution newSol(s);
	unsigned int p;
	int64_t res;
	unsigned int noise = dist_noise(generator);

	for (unsigned int i=0; i<noise; ++i) {
		p = dist_ram(generator);
		newSol[dist_label(generator)].ram = static_cast<RAM_LOC>(1 << p);
	}

	return newSol;
}

void performMutation(Solution s[], double min, double max)
{
	for (unsigned int p = min * pop; p < max * pop; ++p) {
		s[p] = ComputeNewSolutionLight(s[p], 0.1 * pop);
	}
}

Solution crossover(const Solution &a, const Solution &b)
{
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> split_dist(0, a.size() - 1);
	Solution newSol;
	newSol.resize(0);
	const unsigned int splits_size = 100;
	std::vector<unsigned int> splits;

	for (unsigned int i = 0; i<splits_size; ++i)
		splits.push_back(split_dist(generator));
	std::sort(splits.begin(), splits.end());

	for (unsigned int i=0; i<splits.size(); ++i) {

		for (unsigned int j=newSol.size(); j<splits[i]; ++j) {
			newSol.push_back(a[j]);
		}

		++i;
		if (i>=splits.size())
			break;

		for (unsigned int j=newSol.size(); j<splits[i]; ++j) {
			newSol.push_back(b[j]);
		}
	}

	while (newSol.size() < a.size())
		newSol.push_back(a[newSol.size()]);

	return newSol;
}

void performReproduction(Solution s[], double min, double max, double sons)
{
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> parents_dist(pop * min, pop * max);

	for (unsigned int p = 0; p < pop * sons; ++p) {

		int father = parents_dist(generator);
		int mother;
		do {
			mother = parents_dist(generator);
		} while (mother == father);

		s[pop - p - 1] = crossover(s[mother], s[father]);
	}
}

void performMitosis(Solution s[], double from, double to, double dest)
{
	unsigned int start = pop * from;
	unsigned int end = pop * to;
	unsigned int offset = pop * dest;

	for (unsigned int p = start; p < end; ++p)
		s[offset + p] = s[start + p];
}

void InitializePopulation(Solution s[])
{
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> distribution(0, 3);
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
	std::uniform_int_distribution<int> dist_eval(0, pop);
	int termination = 1;

	const double TO_CLONE = 0.05;
	const double TO_CLONE_TO = 0.65;

	const double TO_REPRODUCE = 0.15;
	const double TO_REPRODUCE_CHILDREN = TO_REPRODUCE;

	const double TO_MUTATE = 1 - TO_CLONE - TO_REPRODUCE_CHILDREN;
	const double TO_MUTATE_FROM = TO_CLONE;


	time_t now;
	struct tm newyear;
	time(&now);  /* get current time; same as: now = time(NULL)  */
	newyear = *localtime(&now);
	newyear.tm_hour = 0; newyear.tm_min = 0; newyear.tm_sec = 0;
	newyear.tm_mon = 0;  newyear.tm_mday = 1;
	long int seconds = static_cast<long int>(difftime(now,mktime(&newyear)));

	string filename = string("result_") + std::to_string(seconds) + string(".csv");

	Solution s[pop], s_opt;
	double fit[pop], fit_opt;

	InitializePopulation(s);
	evaluatePopulation(s,fit);

	fit_opt = fit[0];
	s_opt = s[0];

	new_optimal_solution_found(fit_opt, s_opt);
	solution_to_csv(filename, s_opt, fit_opt);
	do {
		//selectParents();
		//performReproduction();
		performMitosis(s, 0, TO_CLONE, TO_CLONE_TO);
		performReproduction(s, 0, TO_REPRODUCE, TO_REPRODUCE_CHILDREN);
		performMutation(s, TO_MUTATE_FROM, TO_MUTATE_FROM + TO_MUTATE);
		//updatePopulation();

		evaluatePopulation(s, fit);

		if (fit_opt > fit[0]) {
			fit_opt = fit[0];
			s_opt = s[0];
			new_optimal_solution_found(fit_opt, s_opt);
			solution_to_csv(filename, s_opt, fit_opt);
		} else {
			cout << "." << endl;
		}
	} while (termination != 0);

	return std::make_pair(s_opt, fit_opt);
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

