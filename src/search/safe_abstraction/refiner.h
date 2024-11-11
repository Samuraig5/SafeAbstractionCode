#ifndef REFINER_H
#define REFINER_H

#include "../tasks/root_task.h"
#include "../plan_manager.h"
#include "abstractor.h"
#include "compositor.h"

class refiner {
    private:
        static void decompose_step(Plan &plan, compositor &compositor);
        static void decomposeCompositeOperator(Plan &plan, compositor &compositor, int insertionIndex);
        static void refine_step(Plan &plan, abstractor &abstractor);
        static void insertMissingOperations(Plan &plan, abstractor &abstractor, int insertionIndex , int varID, int startVal, int endVal);
        static void printPlan(Plan &plan, TaskProxy task_proxy);
    public:
        static Plan refine_plan(Plan plan, vector<pair<abstractor, compositor>> &abstraction_hirarchy);
};

#endif //REFINER_H
