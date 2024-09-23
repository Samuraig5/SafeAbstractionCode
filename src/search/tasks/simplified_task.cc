#include "simplified_task.h"

namespace tasks {

SimplifiedTask::SimplifiedTask(const shared_ptr<RootTask> parent, int /* TODO */) : RootTask(*parent) {
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
}

}
