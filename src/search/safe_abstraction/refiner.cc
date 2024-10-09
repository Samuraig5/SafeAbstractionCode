#include "refiner.h"
#include "../task_proxy.h"

void refiner::refine_plan(Plan plan, vector<shared_ptr<AbstractTask>> abstraction_hirarchy)
{
    for (auto task : abstraction_hirarchy)
    {
        TaskProxy task_proxy(*task);
        refiner::refine_step(plan, task_proxy);
    }
}

void refiner::refine_step(Plan plan, TaskProxy task_proxy)
{
    auto operations = task_proxy.get_operators();
    for (OperatorID opID : plan)
    {
        cout << "opID: " << opID.get_index() << endl;
        //auto op = operations[opID];
    }
}
