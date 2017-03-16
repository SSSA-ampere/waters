#include "Task2.h"

using namespace std;

Task2::Task2()
{

}

Task2::~Task2()
{

}

void Task2::setName(const string &n)
{
    name = n;
}

string Task2::getName()
{
    return name;
}

void Task2::setPriority(Tick p)
{
    priority = p;
}

Tick Task2::getPriority()
{
    return priority;
}

void Task2::setPeriod(Tick p)
{
    period = p;
    maxInterArrivalTime = -1;
    printf("Task2->setPeriod = %lldus\n", (long long)p);
}

Tick Task2::getPeriod()
{
    return period;
}

void Task2::setStimuli(Task_Stimuli s)
{
    stimuli = s;
}

Task_Stimuli Task2::getStimuli()
{
    return stimuli;
}

void Task2::setPreemption(Task_Preemption p)
{
    preemption = p;
}

Task_Preemption Task2::getPreemption()
{
    return preemption;
}

void Task2::setInterArrivalTime(Tick min, Tick max)
{
    minInterArrivalTime = min;
    maxInterArrivalTime = max;

    printf("Task2->setInterArrivaltime min=%lldus max=%lldus\n", (long long)min, (long long)max);
}

Tick Task2::getMinInterArrivalTime()
{
    return minInterArrivalTime;
}

Tick Task2::getMaxInterArrivalTime()
{
    return maxInterArrivalTime;
}

void Task2::setMultipleActivationTaskLimit(int matl)
{
    multipleActivationTaskLimit = matl;
    printf("Task2->setMultipleActivationTaskLimit = %d\n", matl);
}

int Task2::getMultipleActivationTaskLimit()
{
    return multipleActivationTaskLimit;
}

void Task2::setRunnablesListSize(int size)
{
    runnables_list.reserve(size);
}

int Task2::insertRunnable(Runnable2 *r)
{
    runnables_list.push_back(r);
    return runnables_list.size();
}

bool Task2::isPeriodic()
{
    if (maxInterArrivalTime == -1)
        return true;
    else
        return false;
}


