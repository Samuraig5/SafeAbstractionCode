#ifndef REFINER_H
#define REFINER_H

#include "../tasks/root_task.h"
#include "../plan_manager.h"
#include "abstractor.h"

class refiner {
    private:
        static void refine_step(Plan &plan, abstractor &abstractor);
        static void insertMissingOperations(Plan &plan, abstractor &abstractor,int insertionIndex , int varID, int startVal, int endVal);
    public:
        static Plan refine_plan(Plan plan, vector<abstractor> &abstraction_hirarchy);
};

#endif //REFINER_H
