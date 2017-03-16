#pragma once

#include <string>
#include <vector>

#include <metasim.hpp>

#include "Runnable2.h"

using namespace MetaSim;
using namespace std;

enum Task_Stimuli {periodic, sporadic};
enum Task_Preemption {cooperative, preemptive};

class Task2
{
public:
    Task2();
    ~Task2();

    void setName(const string &n);
    string getName();

    void setPriority(Tick p);
    Tick getPriority();

    void setPeriod(Tick p);
    Tick getPeriod();

    void setStimuli(Task_Stimuli s);
    Task_Stimuli getStimuli();

    void setPreemption(Task_Preemption p);
    Task_Preemption getPreemption();


    void setInterArrivalTime(Tick min, Tick max);
    Tick getMinInterArrivalTime();
    Tick getMaxInterArrivalTime();


    void setMultipleActivationTaskLimit(int matl);
    int getMultipleActivationTaskLimit();

    void setRunnablesListSize(int size);
    int insertRunnable(Runnable2 *r);

    bool isPeriodic();
    bool isCooperative() {
      return preemption == cooperative;
    }

    vector<Runnable2 *> runnables_list;

    void setScalingFactor(double s) {
        scalingFactor = s;
    }

    double getScalingFactor() const {
        return scalingFactor;
    }



private:

    double scalingFactor;

    string name;
    Tick priority;

    union
    {
        Tick period;
        Tick minInterArrivalTime;
    };
    Tick maxInterArrivalTime;


    int multipleActivationTaskLimit;


    Task_Stimuli stimuli;
    Task_Preemption preemption;




};
