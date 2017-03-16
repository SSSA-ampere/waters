#include "wkernel.hpp"

#include <algorithm>

#include <simul.hpp>

#include <cpu.hpp>
#include <resmanager.hpp>
#include <scheduler.hpp>
#include <task.hpp>
#include <reginstr.hpp>

namespace RTSim {

    using namespace std;

    WKernel::WKernel(Scheduler *s, const std::string& name, CPU* c) :
      RTKernel(s, name, c)
    {}
    
    WKernel::~WKernel() {}

}
