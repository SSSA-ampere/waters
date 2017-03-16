#ifndef BUILDER_H
#define BUILDER_H

#include <kernel.hpp>
#include <rttask.hpp>
#include <fpsched.hpp>

#include <ps_trace.hpp>

#include <cpu.hpp>

#include "Task2.h"

namespace RTSim {

  using namespace std;
  using namespace MetaSim;

  class Builder {
      std::vector<CPU *>_CPUs;                // Resources managers
      std::vector<ResManager *>_res_managers; // Resources managers
      std::vector<RTKernel *>_kernels;        // Kernels
      std::vector<Scheduler *>_schedulers;    // Schedulers
      std::vector<Task *>_tasks_per;          // Periodic tasks
      std::vector<Task *>_tasks_isr;          // ISR tasks
      std::vector<PSTrace *> _traces;

    public:
      Builder(const std::vector<Task2 *> cores[],
              unsigned int cpu_n);
      ~Builder();

      int buildKernels(const std::vector<Task2 *> cores[],
                       unsigned int cpus);
      int buildTasks(const std::vector<Task2 *> &tasks, unsigned int c);
      string buildRunnables(const vector<Runnable2 *> &runnables, bool cooperative, unsigned int c);
  };
}

#endif // BUILDER_H
