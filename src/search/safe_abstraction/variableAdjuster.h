#ifndef VARIABLEADJUSTER_H
#define VARIABLEADJUSTER_H

#include "../tasks/root_task.h"

using namespace tasks;

class variableAdjuster {
  public:
    static void adjustOperators(int targetVar, vector<ExplicitOperator> &operators);
    static void adjustInitialValues(int targetVar, vector<int> &initialValues);
    static void adjustGoals(int targetVar, vector<FactPair> &goals);
    static void adjustMutexIndex(int targetVar, vector<vector<set<FactPair>>> &mutexes);
    static void adjustMutexContent(int targetVar, vector<vector<set<FactPair>>> &mutexes);
};

#endif //VARIABLEADJUSTER_H
