#ifndef TASKS_SIMPLIFIED_TASK_H
#define TASKS_SIMPLIFIED_TASK_H

#include "root_task.h"
#include "../safe_abstraction/variableAdjuster.h"

#include <set>
#include <vector>
#include <list>

using namespace std;

namespace tasks {

class SimplifiedTask : public RootTask {
private:
    void removeVariables(std::list<int> safeVarID);
    void removeOperators(std::list<int> safeVarID);
    void removeGoals(std::list<int> safeVarID);
    void resizeVariableIDs(std::list<int> safeVarID);
    void print_variables();
    void print_mutexes();
    void print_operators();

public:
    SimplifiedTask(const shared_ptr<RootTask> parent, std::list<int> safeVariables);
};

}
#endif
