#include "variableAdjuster.h"

void variableAdjuster::adjustOperators(int targetVar, vector<ExplicitOperator> &operators)
{
    //cout << "reducing variable index for " << targetVar << " in operators by one" << endl;
    for (ExplicitOperator &op : operators)
    {
        for (FactPair &precon : op.preconditions){
            if (precon.var == targetVar){
                precon.var = precon.var - 1;
            }
        }
        for (ExplicitEffect &postcon : op.effects){
            if (postcon.fact.var == targetVar){
                postcon.fact.var = postcon.fact.var - 1;
            }
            for (FactPair &cond : postcon.conditions){
                if (cond.var == targetVar){
                    cond.var = cond.var - 1;
                }
            }
        }
    }
}

void variableAdjuster::adjustInitialValues(int targetVar, vector<int> &initialValues)
{
    //cout << "reducing variable index for " << targetVar << " in initial values by one" << endl;
    initialValues[targetVar-1] = initialValues[targetVar]; //Shifts all initial values "down" one
}

void variableAdjuster::adjustGoals(int targetVar, vector<FactPair> &goals)
{
    for (FactPair &goal : goals)
    {
        if (goal.var == targetVar)
        {
            //cout << "reducing variable index for " << targetVar << " in goal by one" << endl;
            goal.var = goal.var - 1;
        }
    }
}

void variableAdjuster::adjustMutexIndex(int targetVar, vector<vector<set<FactPair>>> &mutexes)
{
    //cout << "reducing variable index for " << targetVar << " in mutex vector by one" << endl;

    mutexes[targetVar-1] = mutexes[targetVar]; //Shifts all mutexes "down" one
}

void variableAdjuster::adjustMutexContent(int targetVar, vector<vector<set<FactPair>>> &mutexes)
{
    //cout << "reducing variable index for " << targetVar << " in mutex content by one" << endl;

    for (int i = 0; i < (int)mutexes.size(); i++)
    {
        for (int j = 0; j < (int)mutexes[i].size(); j++)
        {
            std::set<FactPair> updatedSet;  // Temporary set to store modified elements
            for (const FactPair &fact : mutexes[i][j]) {
                FactPair modifiedFact(fact.var, fact.value);  // Copy the fact
                if (fact.var == targetVar)
                {
                    modifiedFact.var = modifiedFact.var - 1;  // Modify the copy
                }
                updatedSet.insert(modifiedFact);
            }
            mutexes[i][j] = updatedSet;  // Replace the old set with the updated set
        }
    }
}