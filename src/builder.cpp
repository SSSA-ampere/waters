#include "builder.h"

#include <piresman.hpp>
#include <fcfsresmanager.hpp>

#include <randomvar.hpp>

namespace RTSim {

  using namespace std;
  using namespace MetaSim;

  Builder::Builder(const std::vector<Task2 *> cores[],
                   unsigned int cpu_n)
  {
    buildKernels(cores, cpu_n);
  }

  Builder::~Builder()
  {
    while (_kernels.size() > 0) {
      delete _kernels.back();
      _kernels.pop_back();
    }
    while (_tasks_per.size() > 0) {
      delete _tasks_per.back();
      _tasks_per.pop_back();
    }
    while (_tasks_isr.size() > 0) {
      delete _tasks_isr.back();
      _tasks_isr.pop_back();
    }
    while (_schedulers.size() > 0) {
      delete _schedulers.back();
      _schedulers.pop_back();
    }
    while (_traces.size() > 0) {
      delete _traces.back();
      _traces.pop_back();
    }
    while (_res_managers.size() > 0) {
      delete _res_managers.back();
      _res_managers.pop_back();
    }
    while (_CPUs.size() > 0) {
      delete _CPUs.back();
      _CPUs.pop_back();
    }
  }


  int Builder::buildKernels(const std::vector<Task2 *> cores[],
                            unsigned int cpus)
  {
    for (unsigned int c=0; c<cpus; c++) {
      PSTrace *t = new PSTrace("trace_" + to_string(c) + ".pst");
      FPScheduler *s = new FPScheduler();
      CPU *cpu = new CPU("CPU" + to_string(c));
      RTKernel *k = new RTKernel(s, "Kernel" + to_string(c), cpu);

      PIRManager *rm = new PIRManager("preemptResMng_" + to_string(c));
      //FCFSResManager *rm = new FCFSResManager("preemptResMng_" + to_string(c));
      rm->addResource("preemptRes" + to_string(c));
      k->setResManager(rm);
      _res_managers.push_back(rm);

      _CPUs.push_back(cpu);
      _traces.push_back(t);
      _schedulers.push_back(s);
      _kernels.push_back(k);

      buildTasks(cores[c], c);
    }

    return 0;
  }


  int Builder::buildTasks(const std::vector<Task2 *> &tasks,
                          unsigned int c)
  {
    for (unsigned int i=0; i<tasks.size(); i++) {

      if (tasks.at(i)->getName() == ("Task_2ms")) {
        Task2 * myT = tasks.at(i);
        asm("NOP");
      }

      Task * t;

      if (tasks.at(i)->isPeriodic()) {
        Tick period = tasks.at(i)->getPeriod();
        Tick relative_deadline = period;
        UniformVar activation_phase_periodic(0, period);

        t = new Task(new DeltaVar(period),
                     relative_deadline,
                     (long long int)activation_phase_periodic.get(),
                     tasks.at(i)->getName(),
                     tasks.at(i)->getMultipleActivationTaskLimit() - 1);
        //t->setPeriodic(period);
      } else {
        // TODO
        Tick relative_deadline = 8000000;
        UniformVar activation_phase_sporadic(tasks.at(i)->getMinInterArrivalTime(),
                                             tasks.at(i)->getMaxInterArrivalTime());

        t = new Task(new UniformVar(tasks.at(i)->getMinInterArrivalTime(),
                                    tasks.at(i)->getMaxInterArrivalTime()),
                     relative_deadline,
                     (long long int)activation_phase_sporadic.get(),
                     tasks.at(i)->getName(),
                     tasks.at(i)->getMultipleActivationTaskLimit() - 1);
        //t->setSporadic(new UniformVar(tasks.at(i)->getMinInterArrivalTime(),tasks.at(i)->getMaxInterArrivalTime()));
      }

      string code = buildRunnables(tasks.at(i)->runnables_list,
                                   tasks.at(i)->isCooperative(),
                                   c);

      t->insertCode(code);

      long long int priority = (long long int)tasks.at(i)->getPriority();

      _traces.at(c)->attachToTask(t);
      _kernels.at(c)->addTask(*t, to_string(5000 - priority));
    }

    return 0;
  }

  string Builder::buildRunnables(const vector<Runnable2 *> &runnables, bool cooperative, unsigned int c)
  {
    string instructions;

    for (unsigned int i=0; i<runnables.size(); ++i) {

      if (cooperative)
        instructions.append("wait(preemptRes" + to_string(c) + ");");

      instructions.append("runnable(" + runnables.at(i)->getName() + ");");

      if (cooperative)
        instructions.append("signal(preemptRes" + to_string(c) + ");");
    }

    return instructions;
  }
}
