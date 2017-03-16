#pragma once

#include <string>
#include <vector>
#include <memory>

#include <metasim.hpp>

#include "Label2.h"
#include "weibullvar.h"

using namespace MetaSim;
using namespace std;


enum Label_Access {read, write};

class Task2;
class EventChains2;

class Runnable2
{
public:
    Runnable2();
    ~Runnable2();

    void setName(string &s);
    string getName();

    void setLabelsReadListSize(int size);
    void setLabelsWriteListSize(int size);

    void insertReadLabel(int id);
    void insertWriteLabel(int id);

    void setWeibull();

    void setDistribParams(int l, int u, string &prp, int m);

    void setTask(Task2 *t);
    void setPosInTask(int i);
    Task2 *getTask();
    int getPosInTask();

    void addChain(EventChains2 *C);

    void readLabel(int l, const Tick &activationTime);
    void writeLabel(int l);

    Tick getComputationTime();

    void increaseID();
    long long int getID() const;

    vector<int> labelsRead_list;
    vector<int> labelsWrite_list;

    void pushRT(const Tick &rt);

    void saveRT();

private:
    string name;

    vector<EventChains2 *>inChain;

    Tick lowerBound;
    Tick upperBound;

    std::unique_ptr<WeibullVar> wv;

    //distribution params
    string pRemainPromille;
    Tick mean;

    std::vector<Tick> _responseTimes;

    Task2 *task;
    int task_i;

    long long int job_id;
};



