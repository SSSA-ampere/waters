#pragma once

#include <string>
#include <vector>

#include "Runnable2.h"
#include "EventChains2_elem.h"


using namespace std;

class EventChains2
{
public:
    EventChains2();
    ~EventChains2();


    Runnable2 *runnable_stimulus;
    Runnable2 *runnable_response;

    string name;

    vector<EventChains2_elem *> eventChains_elems;

    void write(const Runnable2 *runnable, const Label2 *label);
    void read(const Runnable2 *runnable, const Label2 *label, const int64_t &activationTime);

    void saveFF(const string & filename);
    void saveLL(const string & filename);

private:

    vector<int64_t> _FF;
    vector<int64_t> _LL;
    vector< pair<int64_t, int64_t> > _status;
};
