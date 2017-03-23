#include "tinyxml/tinyxml2.h"

#include "strtools.h"

#include "Task2.h"
#include "Runnable2.h"
#include "Label2.h"
#include "EventChains2.h"
#include "EventChains2_elem.h"

#include "shared.h"

#include "copy_in_newstruct.h"

#include "annealing.h"

#include <vector>
#include <map>
#include <iostream>
#include <algorithm>

const unsigned int CPU_NUM = 4;

using namespace tinyxml2;
using namespace std;



#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != XML_SUCCESS) { cerr << "Error: " << a_eResult << endl; return; }
#endif



//////////////////////////////////////////////////////
long long int instructionCost_ns = 5;

vector<Task2 *> CPU_CORES[CPU_NUM];

vector<Task2 *> taskList;
map<string, Task2 *> taskName_taskP;

vector<Runnable2 *> runnableList;
map<string, Runnable2 *> runnableName_runnableP;

vector<Label2 *> labelList;

vector<EventChains2 *> eventChains;
//////////////////////////////////////////////////////



int countSiblingElements(XMLElement *pElement, const char *elem_name)
{
  int i = 0;
  while (pElement != nullptr)
  {
    i++;
    pElement = pElement->NextSiblingElement(elem_name);
    //pElement = pElement->NextSiblingElement();
  }
  return i;
}


int countSiblingElements(XMLElement *pElement, const char *elem_name, const char *attr, const char *value)
{
  int i = 0;
  while (pElement != nullptr)
  {
    if(pElement->Attribute(attr, value))
      i++;
    pElement = pElement->NextSiblingElement(elem_name);
  }
  return i;
}




void parse_XMLmodel(void)
{
  XMLDocument xmlDoc;
  XMLError eResult = xmlDoc.LoadFile(MODEL_PATH);

  XMLCheckResult(eResult);

  XMLNode *pRoot = xmlDoc.FirstChild();
  if (pRoot == nullptr)
  {
    cout << "pRoot == nullptr!" << endl;
    return;
  }

  XMLElement *pElement = pRoot->FirstChildElement("swModel");
  if (pElement == nullptr)
  {
    cout << "swModel not found" << endl;
    return;
  }

  XMLElement *pTaskElement = pElement->FirstChildElement("tasks");
  XMLElement *pRunnableElement_first = pElement->FirstChildElement("runnables");
  XMLElement *pLabelElement_first = pElement->FirstChildElement("labels");

  // read all labels

  int g_label_count = countSiblingElements(pLabelElement_first, "labels");
  cout << "g_label_count = " << g_label_count << endl;
  labelList.reserve(g_label_count);

  XMLElement *pLabelElement = pLabelElement_first;
  while (pLabelElement != nullptr)
  {
    const char *label_name = pLabelElement->Attribute("name");
    //TODO check sul nome per vedere se e' nel formato Label_XXX dove XXX e' un numero
    //altrimenti devo fare con <map>
    int label_id = atoi(NthToken(label_name, "_", 1).c_str());
    const char *attribute_constant = pLabelElement->Attribute("constant");

    int label_size_bit = atoi(pLabelElement->FirstChildElement("size")->Attribute("value"));
    bool label_isconstant;

    if(attribute_constant != nullptr && !strcmp(attribute_constant, "true"))
      label_isconstant = true;
    else
      label_isconstant = false;


    Label2 *label = new Label2();
    label->setid(label_id);
    label->setBitSize(label_size_bit);
    label->setIsConstant(label_isconstant);


    labelList.push_back(label);
    int lpos = labelList.size()-1;

    if(lpos != label_id)
    {
      //should never happens
      cout << "lpos != label_id , quit" << endl;
      return;
    }

    pLabelElement = pLabelElement->NextSiblingElement("labels");
  }

  cout << "Labels extracted" << endl;



  while (pTaskElement != nullptr)
  {
    const char *task_name = pTaskElement->Attribute("name");
    const char *task_priority = pTaskElement->Attribute("priority");
    const char *task_stimuli = pTaskElement->Attribute("stimuli");
    const char *task_preemption = pTaskElement->Attribute("preemption");
    const char *task_mTActLimit = pTaskElement->Attribute("multipleTaskActivationLimit");

    cout << "name = " << task_name << " priority = " << task_priority << " stimuli = " << task_stimuli << endl;

    vector<string> tmp_tokens;
    Tokenize(string(task_stimuli), tmp_tokens, string("_"));

    if(tmp_tokens.size() < 3)
      cout << "0 = " << tmp_tokens[0] << " "
           << "1 = " << tmp_tokens[1] << endl;
    else
      cout << "0 = " << tmp_tokens[0] << " "
           << "1 = " << tmp_tokens[1] << " "
           << "2 = " << tmp_tokens[2] << " "
           << "3 = " << tmp_tokens[3] << " " << endl;

    Task2 *task = new Task2();

    task->setName(string(task_name));
    task->setPriority(atoi(task_priority));


    //type and periods
    if(tmp_tokens[0] == "sporadic")
    {
      task->setStimuli(sporadic);
      task->setInterArrivalTime(tous(tmp_tokens[1]), tous(tmp_tokens[2]));
    }
    else if(tmp_tokens[0] == "periodic")
    {
      task->setStimuli(periodic);

      int period = tous(tmp_tokens[1]);
      task->setPeriod(period);
    }
    else
    {
      cout << "tmp_token[0] == " << tmp_tokens[0] << ", quit!" << endl;
      return;
    }


    //preemption
    if(!strcmp(task_preemption,"preemptive"))
    {
      task->setPreemption(preemptive);
    }
    else if(!strcmp(task_preemption,"cooperative"))
    {
      task->setPreemption(cooperative);
    }
    else
    {
      cout << "task_preemption == " << task_preemption << ", quit!" << endl;
      return;
    }

    task->setMultipleActivationTaskLimit(atoi(task_mTActLimit));


    //count runnables
    XMLElement *pCallsElement_first = pTaskElement->FirstChildElement()->FirstChildElement()->FirstChildElement();
    XMLElement *pCallsElement = pCallsElement_first;
    int runnables_count = countSiblingElements(pCallsElement, nullptr); //"calls"
    cout << "runnables_count = " << runnables_count << endl;



    //alloc runnables_count runnables per questo task
    task->setRunnablesListSize(runnables_count);


    //per ogni runnable, cercalo nella lista generale e prendi tutti i suoi parametri, comprese le label
    //e mettilo in una lista di runnable. TODO crea poi i link task->runnable e runnable->task (compresa la posizione)
    pCallsElement = pCallsElement_first;
    XMLElement *pRunnableElement_last = nullptr;
    while (pCallsElement != nullptr)
    {
      Runnable2 *runnable = new Runnable2();
      string runnable_name = firstToken(pCallsElement->Attribute("runnable"), "?");
      runnable->setName(runnable_name);

      //////scorri tutti i runnable TODO scorrili tutti la prima volta e salvali in una lista, e poi cancellali quando li hai usati
      XMLElement *pRunnableElement = pRunnableElement_first;

      if(pRunnableElement_last)
        pRunnableElement = pRunnableElement_last;

      int g_runnables_count = 0;
      bool name_found = false;
      while (pRunnableElement != nullptr)
      {
        string g_runnable_name = firstToken(pRunnableElement->Attribute("name"), "?");

        if(runnable_name == g_runnable_name)
        {
          cout << "runnable " << runnable_name << " found!!!" << endl;

          //scorri tutti i runnableItem (gli accessi alle label)
          XMLElement *prunnableItemsElement = pRunnableElement->FirstChildElement();

          int runnableItemsRead_count = countSiblingElements(prunnableItemsElement, nullptr, "access", "read");
          int runnableItemsWrite_count = countSiblingElements(prunnableItemsElement, nullptr, "access", "write");

          runnable->setLabelsReadListSize(runnableItemsRead_count);
          runnable->setLabelsWriteListSize(runnableItemsWrite_count);

          while (prunnableItemsElement != nullptr)
          {
            const char *data_value = prunnableItemsElement->Attribute("data");
            if(data_value != nullptr) // read and write instructions
            {
              string label_name = firstToken(data_value, "?");
              string access = string(prunnableItemsElement->Attribute("access"));
              int label_id = atoi(NthToken(label_name, "_", 1).c_str());

              //runnable->label
              //label->runnable (in ordine[?])
              if(access == "read") //read
              {

                int num_access = prunnableItemsElement->FirstChildElement()->FirstChildElement()->IntAttribute("value");
                runnable->insertReadLabel_num_acess(num_access);
                runnable->insertReadLabel(label_id);
                labelList[label_id]->runnablesRead_list.push_back(runnable);



              }
              else //write
              {
                runnable->insertWriteLabel(label_id);
                runnable->insertWriteLabel_num_acess(1); // default 1 access
                labelList[label_id]->runnablesWrite_list.push_back(runnable);
              }

              cout << "\t" << label_name << " " << label_id << " " << access << endl;
            }
            else // RunnableInstruction
            {
              //xsi:type="sw:InstructionsDeviation"
              XMLElement *pdeviationElement = prunnableItemsElement->FirstChildElement()->FirstChildElement("deviation");
              if(pdeviationElement == nullptr)
              {
                cout << "pdeviationElement == nullptr" << endl;
                return;
              }

              int lowerBound = pdeviationElement->FirstChildElement("lowerBound")->IntAttribute("value");
              int upperBound = pdeviationElement->FirstChildElement("upperBound")->IntAttribute("value");
              string pRemainPromille = string(pdeviationElement->FirstChildElement("distribution")->Attribute("pRemainPromille"));
              int mean = pdeviationElement->FirstChildElement("distribution")->FirstChildElement()->IntAttribute("value");

              runnable->setDistribParams(cycles2us(lowerBound), cycles2us(upperBound), pRemainPromille, cycles2us(mean));

              cout << "\tlowerBound = " << lowerBound << " upperBound = " << upperBound << " pRemainPromille = " << pRemainPromille << " mean = " << mean << endl;
            }
            prunnableItemsElement = prunnableItemsElement->NextSiblingElement("runnableItems");
          }

          name_found = true;
          //ottimizzazione valida solo per questo modello xml XXX
          pRunnableElement_last = pRunnableElement->NextSiblingElement("runnables");
          break;
        }

        g_runnables_count++;
        pRunnableElement = pRunnableElement->NextSiblingElement("runnables");
      }
      cout << "g_runnables_count = " << g_runnables_count << endl;
      if(!name_found)
      {
        cout << "runnable_name " << runnable_name << " NOT  found! quit." << endl;
        return;
      }

      //aggiungi runnable appena creato
      int r_pos = task->insertRunnable(runnable);
      runnable->setTask(task);
      runnable->setPosInTask(r_pos);
      runnableList.push_back(runnable);
      runnableName_runnableP[runnable->getName()] = runnable;

      pCallsElement = pCallsElement->NextSiblingElement();
    }

    taskList.push_back(task);
    taskName_taskP[task->getName()] = task;
    pTaskElement = pTaskElement->NextSiblingElement("tasks");
  }



  //
  //CPU_CORE->TASK mapping parsing
  //
  XMLElement *pmappingModelElement = pRoot->FirstChildElement("mappingModel");
  if (pmappingModelElement == nullptr)
  {
    cout << "mappingModel" << endl;
    return;
  }

  XMLElement *taskAllocationElement_first = pmappingModelElement->FirstChildElement("taskAllocation");
  if (taskAllocationElement_first == nullptr)
  {
    cout << "task allocation not found" << endl;
    return;
  }

  XMLElement *taskAllocationElement = taskAllocationElement_first;
  while(taskAllocationElement != nullptr)
  {
    string task_name = firstToken(taskAllocationElement->Attribute("task"), "?");
    int cpu_core_n = atoi(&NthToken(taskAllocationElement->Attribute("scheduler"), "_", 1)[4]);

    cout << task_name << "\t->\tCPU_CORE[ " << cpu_core_n << " ]" << endl;

    //task_name a task_pointer
    CPU_CORES[cpu_core_n].push_back(taskName_taskP[task_name]);

    taskAllocationElement = taskAllocationElement->NextSiblingElement("taskAllocation");
  }

  //
  //event_chain mapping
  //

  XMLElement *pconstraintsModelElement = pRoot->FirstChildElement("constraintsModel");
  if (pconstraintsModelElement == nullptr)
  {
    cout << "constraintsModel" << endl;
    return;
  }

  XMLElement *peventChainsElement_first = pconstraintsModelElement->FirstChildElement("eventChains");
  XMLElement *peventChainsElement = peventChainsElement_first;
  while(peventChainsElement != nullptr)
  {
    const char *stimulus = peventChainsElement->Attribute("stimulus");
    const char *response = peventChainsElement->Attribute("response");
    const char *evtc_name = peventChainsElement->Attribute("name");
    string runnable_stimulus_name = FirsToken_AfterStr(stimulus, "?", "_");
    string runnable_response_name = FirsToken_AfterStr(response, "?", "_");

    cout << "eventChain = " << evtc_name << " stimulus = " << runnable_stimulus_name << " response = " << runnable_response_name << endl;

    EventChains2 *evtc = new EventChains2();
    evtc->runnable_stimulus = runnableName_runnableP[runnable_stimulus_name];
    evtc->runnable_response = runnableName_runnableP[runnable_response_name];
    evtc->name = evtc_name;


    int label_wr_id;
    //bool label_firstinchain = true;
    XMLElement *psegmentElement = peventChainsElement->FirstChildElement("segments");
    while(psegmentElement != nullptr)
    {
      //notice that this peventChainElement doesn't have the S ! (is chain, not chainS)
      XMLElement *peventChainElement = psegmentElement->FirstChildElement("eventChain");

      const char *stimulus = peventChainElement->Attribute("stimulus");
      const char *response = peventChainElement->Attribute("response");
      const char *label_wr = peventChainElement->Attribute("name");

      EventChains2_elem *evtc_elem = new EventChains2_elem();
      evtc_elem->runnable_stimulus = runnableName_runnableP[FirsToken_AfterStr(stimulus, "?", "_")];
      evtc_elem->runnable_response = runnableName_runnableP[FirsToken_AfterStr(response, "?", "_")];

      label_wr_id = atoi(NthToken(label_wr, "_", 2).c_str());
      evtc_elem->label_wr = labelList[label_wr_id];

      cout << "\tWR_Label_" << label_wr_id << " stimulus = " << evtc_elem->runnable_stimulus->getName() << " response = " << evtc_elem->runnable_response->getName() << endl;
      evtc->eventChains_elems.push_back(evtc_elem);
      psegmentElement = psegmentElement->NextSiblingElement("segments");

      evtc_elem->runnable_stimulus->addChain(evtc);
      evtc_elem->runnable_response->addChain(evtc);

      //evtc_elem->label_wr->setInChain(true);

      //if (label_firstinchain)
      //{
      //evtc_elem->label_wr->setFirstInChain(true);
      //label_firstinchain = false;
      //}
    }

    //labelList[label_wr_id]->setLastInChain(true);

    eventChains.push_back(evtc);
    peventChainsElement = peventChainsElement->NextSiblingElement("eventChains");
  }

  //
  // requirements mapping
  //


  XMLElement *prequirementsElement_first = pconstraintsModelElement->FirstChildElement("requirements");
  if (prequirementsElement_first == nullptr)
  {
    cout << "requirements not found" << endl;
    return;
  }

  XMLElement *prequirementsElement = prequirementsElement_first;
  while (prequirementsElement != nullptr) {

    const char *req_taskname_string = prequirementsElement->Attribute("process");
    int req_limit = prequirementsElement->FirstChildElement("limit")->FirstChildElement()->IntAttribute("value");
    const char *req_limit_unit = prequirementsElement->FirstChildElement("limit")->FirstChildElement()->Attribute("unit");

    string task_name = firstToken(req_taskname_string, "?");

    int64_t deadline;

    if (strcmp(req_limit_unit, "ms") == 0)
      deadline = static_cast<int64_t>(1000 * req_limit); // convert to us
    else deadline = static_cast<int64_t>(req_limit);


    cout << "deadline of task " << task_name << " = " << deadline << endl;

    taskName_taskP[task_name]->setDeadline(deadline); // set deadline of relative task

    prequirementsElement = prequirementsElement->NextSiblingElement("requirements");
  }

  cout << "Done loading Amalthea model" << endl;
}

int main()
{
  parse_XMLmodel();

  for (int i=0; i<4; i++)
    ram[i].size = ram[i].available = 131072;
  ram[4].size = ram[4].available = 131072 * 2;

  copy_in_newstruct();

  return 0;
  // Assunzione: priority value alto, priotita` alta
  //annealing_run();

  // Riordina task per priorita`
  for (int i=0; i<4; i++)
    std::sort(CPU[i].begin(), CPU[i].end(),
	[](const Task &a, const Task &b) { return a.prio > b.prio; } );

  for (int i=0; i<4; i++) {
    double U = 0;
    cout << "CPU " << i << endl;
    cout << "Wcet\tPeriod\tPrio\tU\tName" << endl;
    for (Task &t : CPU[i]) {
      U += t.wcet / t.period;
      cout << t.wcet << "\t" << t.period << "\t" << t.prio << "\t" << U << "\t" << t.name << endl;
    }
    cout << "-----------------" << endl;
  }

  fflush(stdout);

#ifdef _WIN32
  system("pause");
#endif
  return 0;
}
