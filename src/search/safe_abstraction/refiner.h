#ifndef REFINER_H
#define REFINER_H

#include "../tasks/root_task.h"
#include "../plan_manager.h"

class refiner {
    private:
        static void refine_step(Plan plan, TaskProxy task_proxy);
    public:
        static void refine_plan(Plan plan, vector<shared_ptr<AbstractTask>> abstraction_hirarchy);
};

#endif //REFINER_H
