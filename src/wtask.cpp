#include "wtask.hpp"

#include <regvar.hpp>

namespace RTSim {

  WTask::WTask(RandomVar *iat, Tick rdl, Tick ph,
               const std::string &name, long qs)
    : Task(iat, rdl, ph, name, qs)
  {}

  WTask* WTask::createInstance(vector<string>& par)
  {
    RandomVar* i = nullptr;
    if (par[0].compare("0"))
      i = parsevar(par[0]);
    Tick d = Tick(par[1]);
    Tick p = Tick(par[2]);
    const char* n = "";
    if (par.size() > 2) n = par[3].c_str();
    long q = 1000;
    if (par.size() > 4) q = 1000;//atoi(par[4].c_str());
    bool a = true;
    if (par.size() > 5 && !par[5].compare("false"))
      a = false;
    WTask* t = new WTask(i, d, p, n, q);
    return t;
  }
}
