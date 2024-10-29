#include "compositor.h"

void compositor::composite()
{
  	std::cout << "> Running Compositor" << std::endl;

    std::map<int, std::vector<int>> intermediateStates = getIntermediateStates();
    std::vector<std::pair<int, int>> c;
    std::vector<std::pair<std::set<int>, std::set<int>>> compositeTargets;

    //std::cout << "Building 'c'..." << std::endl;
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
                	c.push_back(std::make_pair(i, val1));
                    c.push_back(std::make_pair(j, val2));

    	    	    //std::cout << "c: " << i << " = " << val1 << ", " << j << " = " << val2 << std::endl;
                    std::pair<std::set<int>, std::set<int>> newTargets = getCompositeTargets(c);
                    if (!newTargets.first.empty())
                    {
						compositeTargets.push_back(newTargets);
                    }
    		        c.clear();
    	        }
    	    }
    	}
    }

    std::cout << "Found " << compositeTargets.size() << " composition target set pairs " << std::endl;
}

std::pair<std::set<int>, std::set<int>> compositor::getCompositeTargets(std::vector<std::pair<int, int>> c)
{
    std::cout << "Generating composition targets..." << std::endl;
    std::set<int> A; //set of all actions whose effects include c
    std::set<int> B; //set of all actions whose preconditions include c

    for (OperatorProxy op : taskProxy.get_operators())
    {
        auto effects = op.get_effects();
        bool containsC = true;

        if (effects.size() < c.size() )
        {
        	containsC = false;
            continue; //Not enough effects to contain c
        }
        else
        {
        	for (auto fact : c)
            {
                bool containsFact = false;
            	for (auto post : effects)
            	{
                    auto postVar = post.get_fact().get_pair().var;
                	auto postVal = post.get_fact().get_pair().value;
                	if (postVar != fact.first){ continue; }
                	if (postVal == fact.second)
                    {
                    	containsFact = true;
                    	break;
                    }
            	}
                if (!containsFact)
                {
                	containsC = false;
                	break;
                }
            }
        }
        if (containsC) { A.insert(op.get_id()); }
    }
    if (A.size() == 0) {
    	//std::cout << "Skipping: A is empty" << std::endl;
    	return std::make_pair(std::set<int>(), std::set<int>());
    }

    for (OperatorProxy op : taskProxy.get_operators())
    {
        auto preconditions = op.get_preconditions();
        bool containsC = true;

        if (preconditions.size() < c.size() )
        {
        	containsC = false;
            continue; //Not enough effects to contain c
        }
        else
        {
        	for (auto fact : c)
            {
                bool containsFact = false;
            	for (auto pre : preconditions)
            	{
                    auto preVar = pre.get_pair().var;
                	auto preVal = pre.get_pair().value;
                	if (preVar != fact.first) {continue;}
                	if (preVal == fact.second)
                    {
                    	containsFact = true;
                    	break;
                    }
            	}
                if (!containsFact)
                {
                	containsC = false;
                	break;
                }
            }
        }
        if (containsC) { B.insert(op.get_id()); }
    }
    if (B.size() == 0)
    {
        //std::cout << "Skipping: B is empty" << std::endl;
    	return std::make_pair(std::set<int>(), std::set<int>());
    }
    //std::cout << "A has " << A.size() << " actions" << std::endl;
    //std::cout << "B has " << B.size() << " actions" << std::endl << std::endl;

    return std::make_pair(A, B);
}

std::map<int, std::vector<int>> compositor::getIntermediateStates()
{
    std::cout << "Getting intermediate states..." << std::endl;

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
    //std::cout << "Found " << intermediateStates.size() << " intermediate states" << std::endl;
    return intermediateStates;
}
