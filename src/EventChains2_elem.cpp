#include "EventChains2_elem.h"

using namespace std;

EventChains2_elem::EventChains2_elem()
{

}

EventChains2_elem::~EventChains2_elem()
{

}

inline void insertElementsInVector(vector< pair<long long int, int64_t> > &dst,
														const vector< pair<long long int, int64_t> > &src)
{
    if (dst.size() > 0)
        return;

    dst.push_back(src.front());
    /*
    for (auto s : src)
        dst.push_back(s);

    if (dst.size() > 2)
        dst.erase(dst.begin() + 1, dst.end() - 1);
        */
}

inline int moveElementsFromVectors(vector< pair<long long int, int64_t> > &dst,
														vector< pair<long long int, int64_t> > &src)
{
    if (src.size() == 0)
        return -1;

    dst.clear();
    dst.push_back(src.front());
    /*
    dst.clear();
    dst.push_back(src.front());
    dst.push_back(src.back());
    */
    src.clear();

    return 0;
}

int EventChains2_elem::pushRunnable(const vector< pair<long long int, int64_t> > &status)
{
    insertElementsInVector(_statusStimulusRunnable, status);

    return 0;
}

int EventChains2_elem::pushLastRunnable(const vector< pair<long long int, int64_t> > &status)
{
    insertElementsInVector(_statusResponseRunnable, status);

    return 0;
}

int EventChains2_elem::pullLabel(vector< pair<long long int, int64_t> > &status)
{
    return moveElementsFromVectors(status, _statusLabel);
}


int EventChains2_elem::pushLabel(const vector< pair<long long int, int64_t> > &status)
{
    _statusLabel = status;

    return 0;
}


int EventChains2_elem::pullRunnable(vector< pair<long long int, int64_t> > &status)
{
    return moveElementsFromVectors(status, _statusStimulusRunnable);
}

int EventChains2_elem::pullLastRunnable(vector< pair<long long int, int64_t> > &status)
{
    return moveElementsFromVectors(status, _statusResponseRunnable);
}
