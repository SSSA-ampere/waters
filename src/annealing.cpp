#include "annealing.h"
#include "milpData.h"
#include "RT.h"

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
const unsigned int MAX_I = 10000;

const int ELEM_MIN = GRAM;
const int ELEM_MAX = LRAM_3;

const unsigned int MEM_NUM = 5;

/*--------------------------*/

typedef RAM_LOC Element;
typedef vector<Label> Solution;

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

static inline int loc_to_id(uint8_t b)
{
  register int counter = -1;
  while (b != 0) {
    ++counter;
    b = b >> 1;
  }
  return counter;
}

static inline void compute_coremap(const vector<Label> &labellist)
{
  for (Label const &l : labellist) {
    if ( ~(coremap[loc_to_id(l.ram)] && 0x0F) )
      coremap[loc_to_id(l.ram)] |= l.used_by_CPU; // TODO optimize this function
  }
}

static inline double computeInterf(unsigned int run_id, const std::vector<Label> &L)
{
  Runnable &r = runnables[run_id];
  double interf = 0;
  double local_interf;
  RAM_LOC l;
  uint8_t u;
  int num_label_acc;

  for (int i = 0; i < r.labels_r.size(); ++i) { // for all labels read
    local_interf = 0;

    l = L[i].ram;
    u = coremap[loc_to_id(l)];
    num_label_acc = r.labels_r_access[i];

    local_interf += one_counter(u&(~l)) * 9;
    if (u&l)
      local_interf += 1.0;

    local_interf *= num_label_acc;

    interf += local_interf;
  }

  for (int i = 0; i < r.labels_w.size(); ++i) { // for all labels written

    l = L[i].ram;
    u = coremap[loc_to_id(l)];

    interf += one_counter(u&(~l)) * 9;
    if (u&l)
      interf += 1.0;
    // Remember: only one access per label written
  }

  return cycles2us(interf);
}

static void printSolution(const Solution &s)
{
#if 0
  for (auto v : s)
    cout << v << " ";
  cout << endl;
#endif
}

static inline void ComputeAnySolution(Solution &s)
{
  unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator(seed);
  std::uniform_int_distribution<int> distribution(0, 3);
  unsigned int position;
  enum RAM_LOC ram_id;

  for (unsigned int i=0; i<s.size(); ++i) {
    do {
      position = distribution(generator);
    } while (ram[position].available - s[i].bitLen < 0);

    ram[position].available -= s[i].bitLen;
    s[i].ram = static_cast<RAM_LOC>(1 << position);
  }
}

static inline Solution ComputeNewSolutionHeavy(const Solution &s)
{
  unsigned int noise;
  unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator(seed);
  std::uniform_int_distribution<int> dist_label(0, s.size()-1);
  std::uniform_int_distribution<int> dist_ram(0, 4);
  std::uniform_int_distribution<int> dist_noise(1, 10);
  Solution newSol(s);
  unsigned int l, p;

  noise = dist_noise(generator);

  for (unsigned int i=0; i<noise; ++i) {
    l = dist_label(generator);

    int64_t res;
    do {
      p = dist_ram(generator);
      res = ram[p].available - newSol[l].bitLen;
    } while (res < 0);

    ram[loc_to_id(newSol[l].ram)].available += newSol[l].bitLen;
    ram[p].available -= newSol[l].bitLen;
    newSol[l].ram = static_cast<RAM_LOC>(1 << p);
  }

  return newSol;
}

static inline Solution ComputeNewSolutionLight(const Solution &s)
{
  unsigned int noise;
  unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator(seed);
  std::uniform_int_distribution<int> dist_label(0, s.size()-1);
  std::uniform_int_distribution<int> dist_ram(0, 3);
  std::uniform_int_distribution<int> dist_noise(1, 3);
  Solution newSol(s);
  unsigned int l, p;

  noise = dist_noise(generator);

  for (unsigned int i=0; i<noise; ++i) {
    l = dist_label(generator);

    if (newSol[l].ram & GRAM) {
      int64_t res;
      do {
        p = dist_ram(generator);
        res = ram[p].available - newSol[l].bitLen;
      } while (res < 0);
    } else {
      p = 4;
    }

    ram[loc_to_id(newSol[l].ram)].available += newSol[l].bitLen;
    ram[p].available -= newSol[l].bitLen;
    newSol[l].ram = static_cast<RAM_LOC>(1 << p);
  }

  return newSol;
}

void update_wcets(const Solution &s)
{
  for (unsigned int i=0; i<4; ++i) {
    // foreach core
    for (unsigned int j=0; j<CPU[i].size(); ++j) {
      // foreach task

      double ram_interference = 0;
      double runnable_wcet = 0;

      for (unsigned int k=0; k<CPU[i].at(j).runnables.size(); ++k) {
        // foreach runnable
        ram_interference += computeInterf(CPU[i].at(j).runnables.at(k), s);
        runnable_wcet += runnables[CPU[i].at(j).runnables.at(k)].exec_time_min;
      }

      CPU[i].at(j).wcet = runnable_wcet + ram_interference;
    }
  }
}

static inline double EvaluateSolution(const Solution &s)
{
  double ret;

  update_wcets(s);
  ret = min_slack(s);
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

template<class T>
inline void new_optimal_solution_found(const T &v)
{
  cout << "------) Optimal solution: " << v << endl;
}

std::pair<Solution, double> annealing()
{
  unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator(seed);
  std::uniform_int_distribution<int> dist_eval(0, 10);
  Solution s, s_new, s_opt;
  double   c, c_new, c_opt;
  unsigned int max_I, max_E;

  s = labels;

  ComputeAnySolution(s);
  printSolution(s);
  compute_coremap(s);

  s_opt = s;
  c_opt = c = EvaluateSolution(s);

  new_optimal_solution_found(c_opt);

  for (double T=MAX_T; T>MIN_T; T=T*COOLING_FACTOR) {
    cout << "Temperature: " << T << endl;

    max_I = 0;
    max_E = 0;

    while (max_I < MAX_I && max_E < MAX_E) {

      if (dist_eval(generator) < 2)
        s_new = ComputeNewSolutionHeavy(s);
      else
        s_new = ComputeNewSolutionLight(s);
      c_new = EvaluateSolution(s_new);

      double dc = c_new - c;

      //cout << "DC: " << dc << " - ";
      if (dc < 0) {
        //cout << "Better solution: " << c_new << endl;
        s = s_new;
        c = c_new;

        printSolution(s);
        if (c < c_opt) {
          s_opt = s;
          c_opt = c;
          new_optimal_solution_found(c_opt);
        }

        max_I++;
      } else {
        //cout << "Worst solution - ";
        if (ExpProb(dc, T)) {
          //cout << "Take anyway" << endl;
          s = s_new;
          c = c_new;

          //printSolution(s);
        } else {
          //cout << "Go ahead" << endl;
        }
      }

      max_E++;
    }
  }
  return std::make_pair(s_opt, c_opt);
}

void annealing_run()
{
  MAX_T = max_deadline;
  cout << "Performing simulated annealing" << endl;
  auto r = annealing();
  cout << "Done" << endl;
  cout << "Best solution found" << endl;
  printSolution(r.first);
  cout << "With value: " << r.second << endl;
}
