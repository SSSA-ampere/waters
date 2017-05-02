#include "annealing.h"
#include "milpData.h"
#include "RT.h"

#include <fstream>
#include <vector>
#include <cmath>
#include <iostream>
#include <random>
#include <chrono>

using namespace std;

/*--------------------------*/

double MAX_T;
const double MIN_T = 0.001;
const double COOLING_FACTOR = 0.98;

const unsigned int MAX_E = 100000;
const unsigned int MAX_I = 2000;

const int ELEM_MIN = GRAM;
const int ELEM_MAX = LRAM_3;

const unsigned int MEM_NUM = 5;

/*--------------------------*/

uint8_t coremap[MEM_NUM];

/*-------------------------*/

static inline double P(double dc, double T)
{
	return exp(-dc/T);
}

static inline int one_counter(uint8_t b)
{
	register int counter = 0;
	while (b != 0) {
		counter += b & 1;
		b = b >> 1;
	}
	return counter;
}

static inline void compute_coremap(const vector<Label> &labellist)
{
	for (unsigned int i=0; i<5; ++i)
		coremap[i] = 0;

	for (Label const &l : labellist)
		coremap[loc_to_id(l.ram)] |= l.used_by_CPU;
}

static inline double computeInterf(const Runnable &r, const std::vector<Label> &L)
{
	double interf = 0;
	double local_interf;
	RAM_LOC l;
	uint8_t u;
	int num_label_acc;

	for (unsigned int i = 0; i < r.labels_r.size(); ++i) { // for all labels read
		local_interf = 0;

		l = L[i].ram;
		u = coremap[loc_to_id(l)];
		num_label_acc = r.labels_r_access[i];

		local_interf += one_counter(u & (~l)) * 9.0;
		if (u & l)
			local_interf += 1.0;

		local_interf *= num_label_acc;

		interf += local_interf;
	}

	for (unsigned int i = 0; i < r.labels_w.size(); ++i) { // for all labels written

		l = L[i].ram;
		u = coremap[loc_to_id(l)];

		interf += one_counter(u & (~l)) * 9.0;
		if (u & l)
			interf += 1.0;
		// Remember: only one access per label written
	}

	return cycles2us(interf);
}

static inline void ComputeAnySolution(Solution &s)
{
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> distribution(0, 4);
	unsigned int position;

  for (unsigned int i=0; i<s.first.size(); ++i) {
		do {
			position = distribution(generator);
    } while (ram[position].available - s.first[i].bitLen < 0);

    ram[position].available -= s.first[i].bitLen;
    s.first[i].ram = static_cast<RAM_LOC>(1 << position);
	}
}

static inline Solution ComputeNewSolutionHeavy(const Solution &s)
{
	unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
  std::uniform_int_distribution<int> dist_label(0, s.first.size()-1);
	std::uniform_int_distribution<int> dist_ram(0, 4);
  std::uniform_int_distribution<int> dist_noise(s.first.size() * 0.01, s.first.size() * 0.03);
	Solution newSol(s);
	unsigned int p;
	int64_t res;
	unsigned int noise = dist_noise(generator);

	for (unsigned int i=0; i<noise; ++i) {
    Label &L = newSol.first[dist_label(generator)];

		do {
			p = dist_ram(generator);
			res = ram[p].available - L.bitLen;
		} while (res < 0);

		ram[loc_to_id(L.ram)].available += L.bitLen;
		L.ram = static_cast<RAM_LOC>(1 << p);
		ram[p].available -= L.bitLen;
	}

	return newSol;
}

static inline Solution ComputeNewSolutionMassive(const Solution &s)
{
	unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
  std::uniform_int_distribution<int> dist_label(0, s.first.size()-1);
	std::uniform_int_distribution<int> dist_ram_src(0, 4);
	std::uniform_int_distribution<int> dist_ram_dst(0, 4);

  std::uniform_int_distribution<int> dist_noise(s.first.size() * 0.05, s.first.size() * 0.3);
	Solution newSol(s);
	unsigned int p;
	int64_t res;
	unsigned int noise = dist_noise(generator);

	RAM_LOC src = static_cast<RAM_LOC>(1 << dist_ram_src(generator));
	RAM_LOC dst;
	do {
		dst = static_cast<RAM_LOC>(1 << dist_ram_dst(generator));
	} while (dst == src);

  unsigned int l;
  for (unsigned int i=0; i<newSol.first.size(); ++i) {
		l = dist_label(generator);
    std::swap(newSol.first[i], newSol.first[l]);
	}

  for (Label &L : newSol.first) {
		if (L.ram != src)
			continue;
		if (ram[loc_to_id(dst)].available - L.bitLen < 0)
			break;

		noise--;
		L.ram = dst;
		ram[loc_to_id(dst)].available -= L.bitLen;
		ram[loc_to_id(src)].available += L.bitLen;

		if (noise == 0)
			break;
	}

	return newSol;
}


Solution ComputeNewSolutionLight(const Solution &s)
{
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
  std::uniform_int_distribution<int> dist_label(0, s.first.size()-1);
	std::uniform_int_distribution<int> dist_ram(0, 4);
	std::uniform_int_distribution<int> dist_noise(1, 100);
	Solution newSol(s);
	unsigned int p;
	int64_t res;
	unsigned int noise = dist_noise(generator);

	for (unsigned int i=0; i<noise; ++i) {
    Label &L = newSol.first[dist_label(generator)];

		/*if (L.ram == GRAM) {
			do {
				p = dist_ram(generator);
				res = ram[p].available - L.bitLen;
			} while (res < 0);
		} else {
			p = 4;
		}*/

		do {
			p = dist_ram(generator);
			res = ram[p].available - L.bitLen;
		} while (res < 0);

		ram[loc_to_id(L.ram)].available += L.bitLen;
		L.ram = static_cast<RAM_LOC>(1 << p);
		ram[p].available -= L.bitLen;
	}

	return newSol;
}

void update_wcets(const Solution &s)
{
	double ram_interference = 0;

	for (unsigned int i=0; i<4; ++i) {
		// foreach core
		for (unsigned int j=0; j<CPU[i].size(); ++j) {
			// foreach task
			Task &task_ij = CPU[i].at(j);

			ram_interference = 0;

			for (unsigned int k=0; k<task_ij.runnables.size(); ++k) {
				// foreach runnable
				Runnable &runnable_ijk = runnables[task_ij.runnables[k]];

        ram_interference += computeInterf(runnable_ijk, s.first);
			}

			task_ij.inflated_wcet = task_ij.exec_time + ram_interference;
			task_ij.wcet = task_ij.exec_time + ram_interference; // TODO
		}
	}
}

static inline double EvaluateSolution(const Solution &s)
{
	double ret;

  compute_coremap(s.first);
	//update_wcets(s);
  ret = computeResponseTime(s.first);
	//ret = min_slack(s);
	//cout << "Minimum slack found: " << ret << endl;

	return -ret;
}

static inline bool ExpProb(double dc, double T)
{
	unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_real_distribution<double> distribution(0, 1);
	double V = distribution(generator);

	return V < P(dc, T);
}

std::pair<Solution, double> annealing()
{
	unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> dist_eval(0, 100);
  Solution s, s_new, s_opt;
	double   c, c_new, c_opt;
	double dc;
	unsigned int max_I, max_E;
	int new_sol_chooser;

  s.first = labels;

	ComputeAnySolution(s);

	s_opt = s;
	c_opt = c = EvaluateSolution(s);
	new_optimal_solution_found(c_opt, s_opt);

	for (double T=MAX_T; T>MIN_T; T=T*COOLING_FACTOR) {
		cout << "Temperature: " << T << "\t-- current best ( " << c_opt << " )" << endl;

		max_I = 0;
		max_E = 0;

		while ((max_I < MAX_I) && (max_E < MAX_E)) {

			new_sol_chooser = dist_eval(generator);
			if (new_sol_chooser < 5)
				s_new = ComputeNewSolutionMassive(s);
			else if (new_sol_chooser < 10)
				s_new = ComputeNewSolutionHeavy(s);
			else
				s_new = ComputeNewSolutionLight(s);
			c_new = EvaluateSolution(s_new);

			dc = c_new - c;
			if (dc < 0) {
				s = s_new;
				c = c_new;
				if (c < c_opt) {
					s_opt = s;
					c_opt = c;
					new_optimal_solution_found(c_opt, s_opt);
				}
				++max_I;
			} else {
				if (ExpProb(dc, T)) {
					s = s_new;
					c = c_new;
				}
			}
			++max_E;
		}
	}
	return std::make_pair(s_opt, c_opt);
}

void annealing_run()
{
	MAX_T = static_cast<double>(max_deadline);
	cout << endl << "Performing simulated annealing" << endl << endl;
	auto r = annealing();
	cout << "Done" << endl;
	cout << "Best solution found" << endl;
	printSolution(r.first);
	cout << "With value: " << r.second << endl;
}
