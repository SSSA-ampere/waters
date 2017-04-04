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

		std::swap(fit[index], fit[i]);
		std::swap(s[index], s[i]);
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
	unsigned int noise = dist_noise(generator);

	for (unsigned int i=0; i<noise; ++i)
		newSol[dist_label(generator)].ram = static_cast<RAM_LOC>(1 << dist_ram(generator));

	return newSol;
}

Solution ComputeNewSolutionRAM2RAM(const Solution &s, unsigned int maximum)
{
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> dist_ram(0, 4);
	std::uniform_int_distribution<int> dist_noise(1, maximum);
	Solution newSol(s);
	unsigned int noise = dist_noise(generator);
	RAM_LOC dst;
	RAM_LOC src;

	src = static_cast<RAM_LOC>(1 << dist_ram(generator));
	do {
		dst = static_cast<RAM_LOC>(1 << dist_ram(generator));
	} while (dst == src);

	std::vector<Label *> src_label;
	std::vector<Label *> dst_label;

	for (Label &l : newSol) {
		if (l.ram == src)
			src_label.push_back(&l);
		else if (l.ram == dst)
			dst_label.push_back(&l);
	}

	std::uniform_int_distribution<int> dist_label_src(0, src_label.size()-1);
	std::uniform_int_distribution<int> dist_label_dst(0, dst_label.size()-1);

	for (unsigned int i=0; i<noise && i<src_label.size()-1 && i<dst_label.size()-1; ++i)
		std::swap(dst_label[dist_label_dst(generator)]->ram, src_label[dist_label_src(generator)]->ram);

	return newSol;
}

Solution ComputeNewSolutionRAM2Others(const Solution &s, unsigned int maximum)
{
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> dist_ram(0, 4);
	std::uniform_int_distribution<int> dist_noise(1, maximum);
	Solution newSol(s);
	unsigned int noise = dist_noise(generator);
	RAM_LOC dst;
	RAM_LOC src;

	src = static_cast<RAM_LOC>(1 << dist_ram(generator));

	std::vector<Label *> src_label;

	for (Label &l : newSol) {
		if (l.ram == src)
			src_label.push_back(&l);
	}

	std::uniform_int_distribution<int> dist_label_src(0, src_label.size()-1);

	for (unsigned int i=0; i<noise && i<src_label.size()-1; ++i) {
		do {
			dst = static_cast<RAM_LOC>(1 << dist_ram(generator));
		} while (dst == src);

		src_label[dist_label_src(generator)]->ram = dst;
	}

	return newSol;
}

void performMutation(Solution s[], unsigned int start, unsigned int end)
{
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> choose_mutation(0, 10);

	for (unsigned int p = start; p < end; ++p) {
		if (choose_mutation(generator) < 2)
			s[p] = ComputeNewSolutionRAM2RAM(s[p], 50);
		else if (choose_mutation(generator) < 5)
			s[p] = ComputeNewSolutionRAM2Others(s[p], 50);
		else
			s[p] = ComputeNewSolutionLight(s[p], 50);
	}
}

Solution crossover(const Solution &a, const Solution &b)
{
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> split_dist(0, a.size() - 1);
	Solution newSol;
	newSol.resize(0);
	const unsigned int splits_size = 200;
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

void performReproduction(Solution s[], unsigned int start, unsigned int end, unsigned int sons)
{
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> parents_dist(start, end);

	for (unsigned int p = 0; p < sons; ++p) {

		int father = parents_dist(generator);
		int mother;
		do {
			mother = parents_dist(generator);
		} while (mother == father);

		s[pop - p - 1] = crossover(s[mother], s[father]);
	}
}

void performMitosis(Solution s[], unsigned int start, unsigned int end, unsigned int offset)
{
	for (unsigned int p = start; p < end; ++p)
		s[offset + p] = s[start + p];
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
	std::uniform_int_distribution<int> dist_eval(0, pop);
	int termination = 1;
	uint64_t epoch = 0;

	const double TO_CLONE_F = 0.05;
	const double TO_CLONE_TO_F = 0.80;

	const double TO_REPRODUCE_F = 0.15;
	const double TO_REPRODUCE_CHILDREN_F = TO_REPRODUCE_F;

	const double TO_MUTATE_F = 1 - TO_CLONE_F - TO_REPRODUCE_CHILDREN_F;
	const double TO_MUTATE_FROM_F = TO_CLONE_F;


	const unsigned int TO_CLONE = TO_CLONE_F * pop;
	const unsigned int TO_CLONE_TO = TO_CLONE_TO_F * pop;

	const unsigned int TO_REPRODUCE = TO_REPRODUCE_F * pop;
	const unsigned int TO_REPRODUCE_CHILDREN = TO_REPRODUCE_CHILDREN_F * pop;

	const unsigned int TO_MUTATE = TO_MUTATE_F * pop;
	const unsigned int TO_MUTATE_FROM = TO_MUTATE_FROM_F * pop;


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

	new_optimal_solution_found(fit_opt, s_opt, epoch);
	solution_to_csv(filename, s_opt, fit_opt, epoch);
	do {
		++epoch;
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
			new_optimal_solution_found(fit_opt, s_opt, epoch);
			solution_to_csv(filename, s_opt, fit_opt, epoch);
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

