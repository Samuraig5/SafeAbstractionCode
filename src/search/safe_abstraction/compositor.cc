#include "compositor.h"

void compositor::composite()
{
    std::cout << std::endl << "============================ SAFE COMPOSITION ==========================" << std::endl;

    std::map<int, std::vector<int>> intermediateStates = getIntermediateStates();
    std::map<int, int> c;

    std::cout << "Building 'c'..." << std::endl;
    for (int i = 0; i < intermediateStates.size()-1; i++)
    {
        auto values1 = intermediateStates[i];
        for (int j = i+1; j < intermediateStates.size(); j++)
    	{
    	    auto values2 = intermediateStates[j];
    	    for (auto val1 : values1)
    	    {
    	        for (auto val2 : values2)
    	        {
        	        c[i] = val1;
        		    c[j] = val2;
    	    	    std::cout << "c: " << i << " = " << val1 << ", " << j << " = " << val2 << std::endl;
    		        getCompositeTargets(c);
    		        c.clear();
    	        }
    	    }
    	}
    }


    std::cout << "========================================================================" << std::endl;
}

std::vector<std::vector<OperatorProxy>> compositor::getCompositeTargets(std::map<int, int> c)
{
    std::vector<std::vector<OperatorProxy>> compositeTargets;
    std::set<int> A; //set of all actions whose effects include c
    std::set<int> B; //set of all actions whose preconditions include c

    for (OperatorProxy op : taskProxy.get_operators())
    {
        auto effects = op.get_effects();
        auto preconditions = op.get_preconditions();

        bool containsC = true;
        if (effects.size() < c.size() )
        {
            containsC = false;
        }
        else
        {
            for (auto post : effects)
            {
                auto postVar = post.get_fact().get_pair().var;
                auto postVal = post.get_fact().get_pair().value;
                if (c.count(postVar) == 0)
                {
                    containsC = false;
                    break;
                }
                else if (c.count(postVar) > 0)
                {
                    if (postVal != c[postVar])
                    {
                        containsC = false;
                        break;
                    }
                }
            }
        }
        if (containsC) { A.insert(op.get_id()); }

        containsC = true;
        if (preconditions.size() < c.size() )
        {
            containsC = false;
        }
        else
        {
            for (auto pre : preconditions)
            {
                auto preVar = pre.get_pair().var;
                auto preVal = pre.get_pair().value;
                if (c.count(preVar) > 0)
                {
                    if (c.count(preVar) == 0)
                    {
                        containsC = false;
                        break;
                    }
                    else if (c.count(preVar) > 0)
                    {
                        if (preVal != c[preVar])
                        {
                            containsC = false;
                            break;
                        }
                    }
                }
            }
        }
        if (containsC) { B.insert(op.get_id()); }
    }
    std::cout << "A has " << A.size() << " actions" << std::endl;

    std::cout << "Elements in A: " << std::endl;
    for (auto elem : A) {
        std::cout << "  " << taskProxy.get_operators()[elem].get_name() << std::endl;
    }
    std::cout << std::endl;

    std::cout << "B has " << B.size() << " actions" << std::endl;
    std::cout << "Elements in B: " << std::endl;
    for (auto elem : B) {
        std::cout << "  " << taskProxy.get_operators()[elem].get_name() << std::endl;
    }
    std::cout << std::endl;

    std::cout << "=" << std::endl;

    return compositeTargets;
}

std::map<int, std::vector<int>> compositor::getIntermediateStates()
{
    std::cout << "Getting intermediate states..." << std::endl;
    std::cout << "Test: " << abstractTask->get_num_variables() << std::endl;

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
                if (!hasGoal || goalValue != i)
                {
                    intermediateStates[variable.get_id()].push_back(i);
                }
            }
        }
    }
    std::cout << "Found " << intermediateStates.size() << " intermediate states" << std::endl;
    return intermediateStates;
}
