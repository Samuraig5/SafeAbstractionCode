#include "simplified_task.h"

#include <algorithm>

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

    //print_variables();
    //print_mutexes();
    //print_operators();
    axioms.clear();
    //removeVariables(safeVariables); // <- Breaks the search (I think stuff like removing mutexes & inital_states must be implemented first)
    removeMutexes(safeVariables);
    removeOperators(safeVariables);
    removeInitialValues(safeVariables);
    removeGoals(safeVariables);
}

void SimplifiedTask::removeVariables(std::list<int> safeVarID)
{
    std::list<string> safeVariables;
    for (int safeVarID : safeVarID) {
        safeVariables.push_back(get_variable(safeVarID).name);

    }

    for (string safe_var : safeVariables)
    {
        //This is very inefficient but .erase resizes the vector so I'm not sure how to do this better yet
        for (int i = 0; i < (int)variables.size(); i++)
        {
            if (safe_var == variables[i].name)
            {
                cout << "Removing variable: " << safe_var << std::endl;
                variables.erase(variables.begin() + i);
                break;
            }
        }
    }
}

void SimplifiedTask::removeMutexes(std::list<int> safeVarID)
{
    for (int safeVarID : safeVarID) {
        cout << "Clearing Mutexes for: " << safeVarID << std::endl;
        for (set<FactPair> mutexSet : mutexes[safeVarID])
        {
            mutexSet.clear();
        }
     }
}

void SimplifiedTask::removeOperators(std::list<int> safeVarID)
{
    std::list<string> safeOperators;
    for (int i = 0; i < (int)operators.size(); i++)
    {
        ExplicitOperator op = operators[i];
        bool free_operation = true;

        for (const FactPair& precon : op.preconditions){
            for (int var : safeVarID){
                if (var != precon.var){
                    free_operation = false;
                }
            }
        }
        for (const ExplicitEffect& postcon : op.effects){
            for (int var : safeVarID){
                if (var != postcon.fact.var){
                    free_operation = false;
                }
            }
        }
        if (free_operation)
        {
            safeOperators.push_back(op.name);
        }
    }

    //Remove the operators that are safe
    for (string safe_op : safeOperators)
    {
        //This is very inefficient but .erase resizes the vector so I'm not sure how to do this better yet
        for (int i = 0; i < (int)operators.size(); i++)
        {
            if (safe_op == operators[i].name)
            {
                cout << "Removing operation: " << safe_op << std::endl;
                operators.erase(operators.begin() + i);
                break;
            }
        }
    }

    //Remove the preconditions / postconditions using the abstracted variables
    for (int i = 0; i < (int)operators.size(); i++)
    {
        bool all_is_clean = true;
        do //It is horrible to brute force it like this. I'll fix it later (hopefully)
        {
            all_is_clean = true;
            for (int var : safeVarID){
                for (int j = 0; j < operators[i].preconditions.size(); j++) {
                    if (var == operators[i].preconditions[j].var){
                        //cout << operators[i].name << ": Removing precondition: " << operators[i].preconditions[j].var << " = " << operators[i].preconditions[j].value << std::endl;

                        all_is_clean = false;
                        operators[i].preconditions.erase(operators[i].preconditions.begin() + j);
                        break;
                    }
                }
            }
            for (int var : safeVarID){
                for (int j = 0; j < operators[i].effects.size(); j++){ //(const ExplicitEffect& postcon : op.effects){
                    if (var == operators[i].effects[j].fact.var){
                        //cout << operators[i].name << ": Removing postcondition: " << operators[i].effects[j].fact.var << " = " << operators[i].effects[j].fact.value << std::endl;

                        all_is_clean = false;
                        operators[i].effects.erase(operators[i].effects.begin() + j);
                        break;
                    }
                }
            }
        }
        while (!all_is_clean);
    }
}

void SimplifiedTask::removeInitialValues(std::list<int> safeVarID)
{
    /*
    Technically we don't have to do anything here since each variable will inherently only access their own initial value.
    If we removed a value (and shortened the vector) we'd have to change the ID for each variable.
    Including its apperances in all operations, precons, postcons, mutextes, goals, ect.
    I feel that will bring in far more complexity and error risk than any "good" it would do.
     */
    /*
    for (int safeVarID : safeVarID) {
        cout << "Removing initial state: " << initial_state_values[safeVarID] << " for: " << safeVarID << std::endl;
        initial_state_values.erase(initial_state_values.begin() + safeVarID);
    }
     */
}

void SimplifiedTask::removeGoals(std::list<int> safeVarID)
{
    std::list<FactPair> safeGoals;
    for (int safeVarID : safeVarID) {
        for (FactPair goal : goals) {
            if (goal.var == safeVarID) {safeGoals.push_back(goal);}
        }
    }

    for (FactPair goal : safeGoals)
    {
        auto index = std::find(goals.begin(), goals.end(), goal);

        if (index != goals.end()) {
            goals.erase(index);
            cout << "Removing goal: " << goal.var << " = " << goal.value << std::endl;
        }
    }
}

void SimplifiedTask::print_variables()
{
    cout << "Variables: ";
    for (auto variable : variables)
    {
        cout << variable.name << ", ";
    }
    cout << std::endl;
}

void SimplifiedTask::print_mutexes()
{
    std::cout << "> MUTEXES" << std::endl;
    for (int i = 0; i < (int)mutexes.size(); i++)
    {
        std::cout << get_variable_name(i) << std::endl;
        for (int j = 0; j < (int)mutexes[i].size(); j++)
        {
            std::cout << "|    " << j << std::endl;
            for (const FactPair& fact : mutexes[i][j]) {
                std::cout << "|    |    (" << get_variable_name(fact.var) << " = " << fact.value << ")" << std::endl;
            }
        }
    }
}

void SimplifiedTask::print_operators()
{
    std::cout << "> OPERATORS" << std::endl;
    for (const ExplicitOperator& op : operators) {
        std::cout << op.name << std::endl << "precons: ";
        for (const FactPair& precon : op.preconditions)
        {
            std::cout << precon.var << " = " << precon.value << ", ";
        }
        std::cout << std::endl;

        std::cout << "postcon: ";
        for (const ExplicitEffect& postcon : op.effects)
        {
            std::cout << postcon.fact.var << " = " << postcon.fact.value << ", ";
        }
        std::cout << std::endl;
    }
}

}
