#pragma once

#include <string>
#include <vector>

#include "Runnable2.h"

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

    void setPriority(int64_t p);
    int64_t getPriority();

    void setPeriod(int64_t p);
    int64_t getPeriod();

    void setStimuli(Task_Stimuli s);
    Task_Stimuli getStimuli();

    void setPreemption(Task_Preemption p);
    Task_Preemption getPreemption();


    void setInterArrivalTime(int64_t min, int64_t max);
    int64_t getMinInterArrivalTime();
    int64_t getMaxInterArrivalTime();


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

	void setDeadline(int64_t d);



private:

    double scalingFactor;

    string name;
    int64_t priority;

    union
    {
        int64_t period;
        int64_t minInterArrivalTime;
    };
    int64_t maxInterArrivalTime;


    int multipleActivationTaskLimit;


    Task_Stimuli stimuli;
    Task_Preemption preemption;

	int64_t deadline;




};
