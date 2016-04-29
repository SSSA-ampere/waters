/*
  In this example, a simple system is simulated, consisting of two
  real-time tasks scheduled by EDF on a single processor.
*/
#include <kernel.hpp>
#include <edfsched.hpp>
#include <jtrace.hpp>
#include <texttrace.hpp>
#include <json_trace.hpp>
#include <rttask.hpp>
#include <instr.hpp>

#include "tinyxml/tinyxml2.h"


#include "strtools.h"

#include "Task2.h"
#include "Runnable2.h"
#include "Label2.h"

#include "builder.h"

#include <vector>
#include <map>

const unsigned int CPU_NUM = 4;

using namespace MetaSim;
using namespace RTSim;
using namespace tinyxml2;
using namespace std;


#ifndef XMLCheckResult
    #define XMLCheckResult(a_eResult) if (a_eResult != XML_SUCCESS) { printf("Error: %i\n", a_eResult); return; }
#endif



//////////////////////////////////////////////////////

vector<Task2 *> CPU_CORES[CPU_NUM];


vector<Task2 *> taskList;
map<string, Task2 *> taskName_taskP;

vector<Runnable2 *> runnableList;
map<string, Runnable2 *> runnableName_runnableP;

vector<Label2 *> labelList;
//////////////////////////////////////////////////////



int countSiblingElements(XMLElement *pElement, char *elem_name)
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


int countSiblingElements(XMLElement *pElement, char *elem_name, char *attr, char *value)
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




void test_tinyXML(void)
{
    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.LoadFile("ChallengeModelModified.amxmi");

    XMLCheckResult(eResult);

    XMLNode *pRoot = xmlDoc.FirstChild();
    if (pRoot == nullptr)
    {
        printf("pRoot == nullptr!\n");
        return;
    }

    XMLElement *pElement = pRoot->FirstChildElement("swModel");
    if (pElement == nullptr)
    {
        printf("swModel not found\n");
        return;
    }

    XMLElement *pTaskElement = pElement->FirstChildElement("tasks");
    XMLElement *pRunnableElement_first = pElement->FirstChildElement("runnables");
    XMLElement *pLabelElement_first = pElement->FirstChildElement("labels");

    // read all labels

    int g_label_count = countSiblingElements(pLabelElement_first, "labels");
    printf("g_label_count = %d\n", g_label_count);
    labelList.reserve(g_label_count);

    XMLElement *pLabelElement = pLabelElement_first;
    while (pLabelElement != nullptr)
    {
        const char *label_name = pLabelElement->Attribute("name");
        //TODO check sul nome per vedere se e' nel formato Label_XXX dove XXX e' un numero
        //altrimenti devo fare con <map>
        int label_id = atoi(NthToken(label_name, "_", 1).c_str());
        const char *attribute_constant = pLabelElement->Attribute("constant");

        int label_size_bit = atoi(pLabelElement->FirstChildElement("size")->Attribute("numberBits"));
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
            //should never happen
            printf("lpos != label_id , quit\n");
            return;
        }

        pLabelElement = pLabelElement->NextSiblingElement("labels");
    }




    while (pTaskElement != nullptr)
    {
        const char *task_name = pTaskElement->Attribute("name");
        const char *task_priority = pTaskElement->Attribute("priority");
        const char *task_stimuli = pTaskElement->Attribute("stimuli");
        const char *task_preemption = pTaskElement->Attribute("preemption");
        const char *task_mTActLimit = pTaskElement->Attribute("multipleTaskActivationLimit");

        printf("name=%s priority=%s stimuli=%s\n", task_name, task_priority, task_stimuli);

        vector<string> tmp_tokens;
        Tokenize(string(task_stimuli), tmp_tokens, string("_"));

        if(tmp_tokens.size() < 3)
            printf("0=%s 1=%s\n", tmp_tokens[0].c_str(), tmp_tokens[1].c_str());
        else
            printf("0=%s 1=%s 2=%s 3=%s\n", tmp_tokens[0].c_str(), tmp_tokens[1].c_str(), tmp_tokens[2].c_str(), tmp_tokens[3].c_str() );



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
            printf("tmp_token[0] == %s, quit!\n", tmp_tokens[0].c_str());
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
            printf("task_preemption == %s, quit!\n", task_preemption);
            return;
        }

        task->setMultipleActivationTaskLimit(atoi(task_mTActLimit));


        //count runnables
        XMLElement *pCallsElement_first = pTaskElement->FirstChildElement()->FirstChildElement()->FirstChildElement();
        XMLElement *pCallsElement = pCallsElement_first;
        int runnables_count = countSiblingElements(pCallsElement, nullptr); //"calls"
        printf("runnables_count = %d\n", runnables_count);



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
                    printf("runnable %s found!!!\n", runnable_name.c_str());

                    //scorri tutti i runnableItem (gli accessi alle label)
                    XMLElement *prunnableItemsElement = pRunnableElement->FirstChildElement();

                    int runnableItemsRead_count = countSiblingElements(prunnableItemsElement, nullptr, "access", "read");
                    int runnableItemsWrite_count = countSiblingElements(prunnableItemsElement, nullptr, "access", "write");

                    runnable->setLabelsReadListSize(runnableItemsRead_count);
                    runnable->setLabelsWriteListSize(runnableItemsWrite_count);

                    while (prunnableItemsElement != nullptr)
                    {
                        const char *data_value = prunnableItemsElement->Attribute("data");
                        if(data_value != nullptr)
                        {
                            string label_name = firstToken(data_value, "?");
                            string access = string(prunnableItemsElement->Attribute("access"));
                            int label_id = atoi(NthToken(label_name, "_", 1).c_str());

                            //runnable->label
                            //label->runnable (in ordine[?])
                            if(access == "read")
                            {
                                runnable->insertReadLabel(label_id);
                                labelList[label_id]->runnablesRead_list.push_back(runnable);
                            }
                            else
                            {
                                runnable->insertWriteLabel(label_id);
                                labelList[label_id]->runnablesWrite_list.push_back(runnable);
                            }

                            printf("\t%s %d %s\n", label_name.c_str(),label_id, access.c_str());
                        }
                        else
                        {
                            //xsi:type="sw:InstructionsDeviation"
                            XMLElement *pdeviationElement = prunnableItemsElement->FirstChildElement("deviation");
                            if(pdeviationElement == nullptr)
                            {
                                printf("pdeviationElement == nullptr\n");
                                return;
                            }

                            int lowerBound = pdeviationElement->FirstChildElement("lowerBound")->IntAttribute("value");
                            int upperBound = pdeviationElement->FirstChildElement("upperBound")->IntAttribute("value");
                            string pRemainPromille = string(pdeviationElement->FirstChildElement("distribution")->Attribute("pRemainPromille"));
                            int mean = pdeviationElement->FirstChildElement("distribution")->FirstChildElement()->IntAttribute("value");


                            runnable->setDistribParams(lowerBound, upperBound, pRemainPromille, mean);

                            printf("\t lowerBound=%d upperBound=%d pRemainPromille=%s mean=%d\n", lowerBound, upperBound, pRemainPromille.c_str(), mean);

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
            printf("g_runnables_count = %d\n", g_runnables_count);
            if(!name_found)
            {
                printf("runnable_name %s NOT  found! quit.\n", runnable_name.c_str());
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

        printf("\n");
        taskList.push_back(task);
        taskName_taskP[task->getName()] = task;
        pTaskElement = pTaskElement->NextSiblingElement("tasks");
    }


    XMLElement *pmappingModelElement = pRoot->FirstChildElement("mappingModel");
    if (pmappingModelElement == nullptr)
    {
        printf("mappingModel\n");
        return;
    }

    XMLElement *pprocessAllocationElement_first = pmappingModelElement->FirstChildElement("processAllocation");
    XMLElement *pprocessAllocationElement = pprocessAllocationElement_first;
    while(pprocessAllocationElement != nullptr)
    {
        string task_name = firstToken(pprocessAllocationElement->Attribute("process"), "?");
        int cpu_core_n = atoi(&NthToken(pprocessAllocationElement->Attribute("scheduler"), "_", 1)[4]);

        printf("%s->\tCPU_CORE[%d]\n", task_name.c_str(), cpu_core_n);

        //task_name a task_pointer
        CPU_CORES[cpu_core_n].push_back(taskName_taskP[task_name]);

        pprocessAllocationElement = pprocessAllocationElement->NextSiblingElement("processAllocation");
    }





    printf("fine!\n");

}







int main()
{


    test_tinyXML();

    Builder b(CPU_CORES, CPU_NUM);
    SIMUL.run(500);

    //system("pause");
    return 1;

    try {

        ///ciao balsini

        SIMUL.dbg.enable("All");
        SIMUL.dbg.setStream("debug.txt");
        // set the trace file. This can be visualized by the
        // rttracer tool
        //JavaTrace jtrace("trace.trc");

        TextTrace ttrace("trace.txt");
        JSONTrace jtrace("trace.json");
  
        cout << "Creating Scheduler and kernel" << endl;
        EDFScheduler edfsched;
        RTKernel kern(&edfsched);

        cout << "Creating the first task" << endl;
        PeriodicTask t1(4, 4, 0, "Task0");

        cout << "Inserting code" << endl;
        t1.insertCode("fixed(2);");
        //t1.setAbort(false);


        cout << "Creating the second task" << endl;
        PeriodicTask t2(5, 5, 0, "Task1");

        cout << "Inserting code" << endl;
        t2.insertCode("fixed(2);");
        //t2.setAbort(false);

        cout << "Creating the third task" << endl;
        PeriodicTask t3(6, 6, 0, "Task2");
        cout << "Inserting code" << endl;
        t3.insertCode("fixed(2);");
        //t3.setAbort(false);


        cout << "Setting up traces" << endl;
	
        // new way
        ttrace.attachToTask(&t1);
        ttrace.attachToTask(&t2);
        ttrace.attachToTask(&t3);

        jtrace.attachToTask(&t1);
        jtrace.attachToTask(&t2);
        jtrace.attachToTask(&t3);

        cout << "Adding tasks to schedulers" << endl;

        kern.addTask(t1, "");
        kern.addTask(t2, "");
        kern.addTask(t3, "");
//        kern.addTask(t4, "");
  
        cout << "Ready to run!" << endl;
        // run the simulation for 500 units of time
        SIMUL.run(500);
    } catch (BaseExc &e) {
        cout << e.what() << endl;
    }
}