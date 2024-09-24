#include "simplified_task.h"

namespace tasks {

SimplifiedTask::SimplifiedTask(const shared_ptr<RootTask> parent, std::list<int> safeVariables) : RootTask(*parent) {
    /*
        Remo: Create the simplified, i.e. the safely abstracted, task here by
        modifying the vectors listed below (inherited from RootTask):
        vector<ExplicitVariable> variables;
        vector<vector<set<FactPair>>> mutexes;
        vector<ExplicitOperator> operators;
        vector<ExplicitOperator> axioms;
        vector<int> initial_state_values;
        vector<FactPair> goals;
    */

    std::list<ExplicitVariable> variablesToAbstract;
    std::list<FactPair> goalsToAbstract;
    for (int safeVarID : safeVariables) {
        variablesToAbstract.push_back(get_variable(safeVarID));

        for (FactPair goal : goals) {
            if (goal.var == safeVarID) {goalsToAbstract.push_back(goal);}
        }
    }






    for (ExplicitVariable var : variablesToAbstract){
        cout << var.name << endl;
    }
    for (FactPair goal : goalsToAbstract){
        cout << goal.var << ": " << goal.value << endl;
    }
}
}
