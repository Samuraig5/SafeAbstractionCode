#include "compositor.h"

void compositor::composite()
{
    std::map<int, std::vector<int>> intermediateStates = getIntermediateStates();
    std::map<int, int> c;

}

std::vector<std::vector<OperatorProxy>> compositor::getCompositeTargets(std::map<int, int> c)
{
    std::vector<std::vector<OperatorProxy>> compositeTargets;
    std::set<OperatorProxy*> A; //set of all actions whose effects include c
    std::set<OperatorProxy*> B; //set of all actions whose preconditions include c

    for (OperatorProxy op : taskProxy.get_operators())
    {
        auto effects = op.get_effects();
        auto preconditions = op.get_preconditions();

        for (auto post : effects)
        {
            auto postVar = post.get_fact().get_pair().var;
            auto postVal = post.get_fact().get_pair().value;
            if (postVal == c[postVar])
                { A.insert(&op); }
        }

        for (auto pre : preconditions)
        {
            auto preVar = pre.get_pair().var;
            auto preVal = pre.get_pair().value;
            if (preVal == c[preVar])
                { B.insert(&op); }
        }
    }
    for (OperatorProxy *a : A)
    {
        std::vector<OperatorProxy> compositeAction;
        compositeAction.push_back(*a);
        //Which b should be added here?
        //All possible combinations??

        compositeTargets.push_back(compositeAction);
    }
    return compositeTargets;
}

std::map<int, std::vector<int>> compositor::getIntermediateStates()
{
    std::vector<int> initialStates = abstractTask->get_initial_state_values();
    std::map<int, std::vector<int>> intermediateStates;
    GoalsProxy goalFacts = taskProxy.get_goals();

    for (auto variable : taskProxy.get_variables())
    {
        bool hasGoal = false;
        int goalValue;
        for (auto goal : goalFacts)
        {
            if (goal.get_variable().get_id() == variable.get_id())
            {
                goalValue = goal.get_value();
                hasGoal = true;
                break;
            }
        }

        for (int i = 0; i < variable.get_domain_size(); i++)
        {
            if (initialStates[variable.get_id()] != i)
            {
                if (!hasGoal || goalValue == i)
                {
                    intermediateStates[variable.get_id()].push_back(i);
                }
            }
        }
    }

    return intermediateStates;
}
