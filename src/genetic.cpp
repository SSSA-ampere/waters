#include <vector>
#include <cmath>
#include <iostream>
#include <random>
#include <chrono>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <string>

#include "genetic.h"
#include "RT.h"
#include "milpData.h"
#include "analyse_data.h"

using namespace std;

unsigned int MEM_POP_SIZE;

double TOL_FIT = 0.01;
uint64_t epoch;
uint64_t MAX_EPOCH = 30000;

GeneticPopulation memory_population;

static std::vector<std::string> parse_csv_line(const std::string &line,
                                               unsigned int to_skip)
{
  std::vector<std::string> ret;
  unsigned int first = 0;
  size_t last = 0;
  bool goAhead = true;
  std::string element;
  unsigned int skipped = 0;

  while (goAhead) {
    last = line.find(',', first);
    if (last == string::npos) {
      goAhead = false;
      last = line.size();
    }

    element = line.substr(first, last - first);

    if (skipped < to_skip) {
      ++skipped;
    } else {
      //std::cout << "Position: [" << first << " - " << last << "] :\t" << element << std::endl;
      ret.push_back(element);
    }

    first = last + 1;
  }

  return ret;
}

Solution solution_from_csv(const std::string &fileName, unsigned int to_skip)
{
  Solution s(labels);
  std::string line_next, line;
  ifstream inFile(fileName, std::ifstream::in);

  do {
    line = line_next;
    getline(inFile, line_next);
  } while (inFile.good());

  //std::cout << line << std::endl << std::endl;
  std::vector<std::string> ll = parse_csv_line(line, to_skip);

  for (unsigned int i=0; i<ll.size(); ++i)
    s[i].ram = static_cast<RAM_LOC>(1 << std::stoi(ll[i]));

  std::cout << "Loaded [" << ll.size() << "] labels from \"" << fileName << "\"" << std::endl;

  return s;
}

//std::vector<RAM> ramg[MEM_POP_SIZE];
std::random_device rd;

void sortPopulation(GeneticPopulation &population)
{
  std::sort(population.begin(), population.end(),
            [&](const std::pair<Solution, double> &a, const std::pair<Solution, double> &b) {
    return a.second < b.second;
  });
}

double evaluatePopulation(GeneticPopulation &population)
{
  double mean_fitness = 0;

  for (unsigned int p = 0; p < population.size(); ++p) {
    population[p].second = computeResponseTime(population[p].first);
    mean_fitness += population[p].second;
  }

  sortPopulation(population);

  return mean_fitness / MEM_POP_SIZE;
}

Solution ComputeNewSolutionLight(const Solution &s, unsigned int maximum)
{
  std::default_random_engine generator(rd());
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
  std::default_random_engine generator(rd());
  std::uniform_int_distribution<int> dist_ram(0, 4);
  std::uniform_int_distribution<int> dist_noise(1, maximum);
  Solution newSol(s);
  unsigned int noise = dist_noise(generator);
  RAM_LOC dst;
  RAM_LOC src;
  unsigned int src_pos, dst_pos;

  src = static_cast<RAM_LOC>(1 << dist_ram(generator));
  do {
    dst = static_cast<RAM_LOC>(1 << dist_ram(generator));
  } while (dst == src);

  std::vector<Label *> src_label;

  for (Label &l : newSol) {
    if (l.ram == src)
      src_label.push_back(&l);
  }


  for (int i=0; (i < noise) && (i + 1 < src_label.size()); ++i) {
    std::uniform_int_distribution<int> dist_label_src(0, src_label.size()-1);

    src_pos = dist_label_src(generator);
    src_label[src_pos]->ram = dst;
    src_label.erase(src_label.begin() + src_pos);
  }

  return newSol;
}

Solution ComputeNewSolutionRAM2Others(const Solution &s, unsigned int maximum)
{
  std::default_random_engine generator(rd());
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

  for (int i=0; (i < noise) && (i + 1 < src_label.size()); ++i) {
    std::uniform_int_distribution<int> dist_label_src(0, src_label.size()-1);
    int dst_pos = dist_label_src(generator);

    do {
      dst = static_cast<RAM_LOC>(1 << dist_ram(generator));
    } while (dst == src);

    src_label[dst_pos]->ram = dst;
    src_label.erase(src_label.begin() + dst_pos);
  }

  return newSol;
}


Solution ComputeNewSolutionLowRAM2Others(const Solution &s, unsigned int maximum)
{
  std::default_random_engine generator(rd());
  std::uniform_int_distribution<int> dist_cpu(0, 3);
  std::uniform_int_distribution<int> dist_ram(0, 4);
  std::uniform_int_distribution<int> dist_noise(1, maximum);
  std::uniform_int_distribution<int> dist_chooser(0, 100);
  Solution newSol(s);
  unsigned int noise = dist_noise(generator);
  unsigned int chooser = dist_chooser(generator);
  unsigned int src_cpu = dist_cpu(generator);

  std::vector<int> src_label[5];

  // Create array of label disposition in memories
  for (unsigned int li : CPU_labels[src_cpu]) {
    src_label[loc_to_id(labels.at(li).ram)].push_back(li);
  }

  unsigned int l_size = CPU_labels[src_cpu].size();
  std::vector<unsigned int> notempty_loc;
  std::vector<double> prob;
  double tot_prob = 0;

  // Associate probability inversely prop to number of labels (and remove empty memories)
  for (unsigned int si = 0; si < 5; ++si) {
    if (src_label[si].size() != 0) {
      prob.push_back(static_cast<double>(1.0 / src_label[si].size()));
      notempty_loc.push_back(si);
      tot_prob += static_cast<double>(1.0 / src_label[si].size());
    }
  }

  unsigned int norm_prob = 0;
  unsigned int src_ram;

  // Normalize probabilities and choose memory
  for (unsigned int i = 0; i < notempty_loc.size(); ++i) {
    norm_prob += ceil(prob.at(i) * 100 / tot_prob);
    if (chooser <= norm_prob) {
      src_ram = notempty_loc.at(i);
      break;
    }
  }

  RAM_LOC dst;
  RAM_LOC src;
  src = static_cast<RAM_LOC>(1 << src_ram);

  // Redistribute casually some labels
  for (int i = 0; (i < noise) && (i < src_label[src_ram].size()); ++i) {
    std::uniform_int_distribution<int> dist_label_src(0, src_label[src_ram].size() - 1);
    int dst_pos = dist_label_src(generator);

    do {
      dst = static_cast<RAM_LOC>(1 << dist_ram(generator));
    } while (dst == src);

    newSol.at(src_label[src_ram].at(dst_pos)).ram = dst;
  }

  return newSol;
}

void mutatate_chromosome_waters_GA(Solution &s)
{
  std::default_random_engine generator(rd());
  std::uniform_int_distribution<int> choose_mutation(0, 100);
  unsigned int mutation_kind;
  unsigned int mut_gap = 0;

  mutation_kind = choose_mutation(generator);

  if (epoch > 2000)
    mut_gap = 30;
  if (mutation_kind < (10 + mut_gap))
    s = ComputeNewSolutionLowRAM2Others(s, 50);
  else if (mutation_kind < 40 + mut_gap/3)
    s = ComputeNewSolutionRAM2RAM(s, 200);
  else if (mutation_kind < (70 - mut_gap/3))
    s = ComputeNewSolutionRAM2Others(s, 200);
  else
    s = ComputeNewSolutionLight(s, 100);
}

void performMutation(GeneticPopulation &s, unsigned int start, unsigned int end)
{
  for (unsigned int p = start; p < end; ++p)
    mutatate_chromosome_waters_GA(s[p].first);
}

Solution crossover_waters_GA(const Solution &a, const Solution &b)
{
  std::default_random_engine generator(rd());
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

void performReproduction(GeneticPopulation &s, unsigned int start, unsigned int end, unsigned int sons)
{
  std::default_random_engine generator(rd());
  std::uniform_int_distribution<int> parents_dist(start, end);

  for (unsigned int p = 0; p < sons; ++p) {

    int father = parents_dist(generator);
    int mother;
    do {
      mother = parents_dist(generator);
    } while (mother == father);

    s[MEM_POP_SIZE - p - 1].first = crossover_waters_GA(s[mother].first, s[father].first);
  }
}

void performMitosis(GeneticPopulation &s, unsigned int start, unsigned int end, unsigned int offset)
{
  for (unsigned int p = start; p < end; ++p)
    s[offset + p] = s[start + p];
}

Solution getRandomSolution_waters_GA()
{
  std::default_random_engine generator(rd());
  std::uniform_int_distribution<int> distribution(0, 4);
  unsigned int position;
  Solution s = labels;

  for (unsigned int i = 0; i<s.size(); ++i) {
    //do {
    //do {
    position = distribution(generator);
    //	}	while (static_cast<RAM_LOC>(1 << position) == LRAM_1);
    //} while (ramg[p].at(position).available - s[p].first.at(i).bitLen < 0);

    //ramg[p].at(position).available -= s.at(i).bitLen;
    s.at(i).ram = static_cast<RAM_LOC>(1 << position);
  }

  return s;
}

void InitializePopulation(GeneticPopulation &s)
{
  //RAM ram_temp;

  s.clear();

  for (unsigned int p=0; p<MEM_POP_SIZE; ++p) {
    // Initialize rams
    /*
    for (int i = 0; i < 4; i++) {
      ram_temp.size = 131072;
      ram_temp.available = 131072;
      ramg[p].push_back(ram_temp);
    }
    ram_temp.size = 131072 * 2;
    ram_temp.available = 131072 * 2;
    ramg[p].push_back(ram_temp);
*/
    s.push_back(std::make_pair(getRandomSolution_waters_GA(), 0.0));
  }
}

unsigned int get_genes_size_waters_GA()
{
  return labels.size();
}

double initialize_waters_GA(unsigned int population_size)
{
  GeneticPopulation &population = memory_population;

  MEM_POP_SIZE = population_size;
  population.resize(MEM_POP_SIZE);
  InitializePopulation(population);

  return evaluatePopulation(population);
}

std::pair<Solution, double> genetic()
{
  //solution_from_csv("result_11357313.csv", 3);

  epoch = 0;

  time_t now;
  struct tm newyear;
  time(&now);  /* get current time; same as: now = time(NULL)  */
  newyear = *localtime(&now);
  newyear.tm_hour = 0; newyear.tm_min = 0; newyear.tm_sec = 0;
  newyear.tm_mon = 0;  newyear.tm_mday = 1;
  long int seconds = static_cast<long int>(difftime(now,mktime(&newyear)));

  string filename = string("result_") + std::to_string(seconds) + string(".csv");

  string filename_step = string("step_result_") + std::to_string(seconds) + string(".csv");

  string filename_dist = string("dist_result_") + std::to_string(seconds) + string(".csv");

  cout << "csv name: " << filename << endl << endl;

  GeneticPopulation &population = memory_population;

  Solution s_opt;
  double fit_mean, fit_opt;

  fit_mean = initialize_waters_GA(200);

  /*************************************************************************/

  double CLONE_F = 0.10;
  double CLONE_TO_F = 0.60;

  double REPRODUCE_F = 0.15;
  double REPRODUCE_CHILDREN_F = 0.30;

  double MUTATE_F = 1 - REPRODUCE_CHILDREN_F;
  double MUTATE_FROM_F = CLONE_F+0.05;

  unsigned int CLONE = CLONE_F * MEM_POP_SIZE;
  unsigned int CLONE_TO = CLONE_TO_F * MEM_POP_SIZE;

  unsigned int REPRODUCE_TO = REPRODUCE_F * MEM_POP_SIZE;
  unsigned int REPRODUCE_CHILDREN = REPRODUCE_CHILDREN_F * MEM_POP_SIZE;

  unsigned int MUTATE_TO = MUTATE_F * MEM_POP_SIZE;
  unsigned int MUTATE_FROM = MUTATE_FROM_F * MEM_POP_SIZE;

  /*************************************************************************/

  fit_opt = population[0].second;
  s_opt = population[0].first;


  new_optimal_solution_found(fit_opt, s_opt, fit_mean, epoch);
  solution_to_csv(filename, s_opt, fit_opt, fit_mean, epoch);

  do {
    ++epoch;
    //selectParents();
    //performReproduction();
    performMitosis(population, 0, CLONE, CLONE_TO);
    performReproduction(population, 0, REPRODUCE_TO, REPRODUCE_CHILDREN);
    performMutation(population, MUTATE_FROM, MUTATE_TO);
    //updatePopulation();

    fit_mean = evaluatePopulation(population);

    if (fit_opt > population[0].second) {
      if (fit_opt - population[0].second > TOL_FIT) {
        cout << endl << "Response time step found" << endl << endl;
        solution_to_csv(filename_step, s_opt, fit_opt, fit_mean, epoch - 1);
        solution_to_csv(filename_step, population[0].first, population[0].second, fit_mean, epoch);
      }
      fit_opt = population[0].second;
      s_opt = population[0].first;

      //worstResponseTimeTask(s_opt);
      new_optimal_solution_found(fit_opt, s_opt, fit_mean, epoch);
      solution_to_csv(filename, s_opt, fit_opt, fit_mean, epoch);
      distribution_to_csv(filename_dist, s_opt, epoch);
    } else {
      cout << "." << endl;
    }
  } while (epoch <= MAX_EPOCH);

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

