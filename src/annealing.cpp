#include "annealing.h"
#include "milpData.h"

#include <vector>
#include <cmath>
#include <iostream>
#include <random>
#include <chrono>

using namespace std;

/*--------------------------*/

const double MAX_T = 2000;
const double MIN_T = 0.001;
const double COOLING_FACTOR = 0.98;

const unsigned int MAX_E = 1000;
const unsigned int MAX_I = 1000;


const int ELEM_MIN = GRAM;
const int ELEM_MAX = LRAM_3;

/*--------------------------*/

typedef RAM_LOC Element;
typedef vector<Element> Solution;

/*--------------------------*/

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine generator(seed);

static inline double P(double dc, double T)
{
  return exp(-dc/T);
}

static void printSolution(const Solution &s)
{
  for (auto v : s)
    cout << v << " ";
  cout << endl;
}

static inline void ComputeAnySolution(Solution &s)
{
  std::uniform_int_distribution<int> distribution(ELEM_MIN, ELEM_MAX);

  for (unsigned int i=0; i<s.size(); ++i)
    s.at(i) = static_cast<Element>(distribution(generator));
}

static inline Solution ComputeNewSolution(const Solution &s)
{
  std::uniform_int_distribution<int> distribution(0, s.size()-1);
  unsigned int id1, id2;
  Element appo;
  Solution newSol(s);

  for (unsigned int i=0; i<s.size(); ++i) {
    id1 = distribution(generator);
    id2 = distribution(generator);

    appo =  newSol.at(id1);
    newSol.at(id1) = newSol.at(id2);
    newSol.at(id2) = appo;
  }

  return newSol;
}

static inline double EvaluateSolution(const Solution &s)
{
  unsigned int counter = 0; // Test trick to make position in vector significant
  unsigned int tot = 0;
  for (auto v : s) {
    if (v == GRAM)
      tot += 8;
    else
      tot += 1;
    tot += counter;
    counter++;
  }
  return tot;
}

static inline bool ExpProb(double dc, double T)
{
  std::uniform_real_distribution<double> distribution(0, 1);
  double V = distribution(generator);

  return V < P(dc, T);
}

std::pair<Solution, double> annealing()
{
  Solution s, s_new, s_opt;
  double   c, c_new, c_opt;
  unsigned int max_I, max_E;

  s.resize(20);
  ComputeAnySolution(s);
  printSolution(s);

  s_opt = s;
  c_opt = EvaluateSolution(s);;

  for (double T=MAX_T; T>MIN_T; T=T*COOLING_FACTOR) {
    cout << "Temperature: " << T << endl;

    max_I = 0;
    max_E = 0;

    while (max_I < MAX_I && max_E < MAX_E) {

      s_new = ComputeNewSolution(s);
      c_new = EvaluateSolution(s_new);

      double dc = c_new - c;

      //cout << "DC: " << dc << " - ";
      if (dc < 0) {
        //cout << "Better solution" << endl;
        s = s_new;
        c = c_new;

        printSolution(s);
        if (c < c_opt) {
          s_opt = s;
          c_opt = c;
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

void annealing_test()
{
  cout << "Performing annealing test" << endl;
  auto r = annealing();
  cout << "Done" << endl;
  cout << "Found solution" << endl;
  printSolution(r.first);
  cout << "With cost: " << r.second << endl;
}

