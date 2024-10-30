#include "compositor.h"

void compositor::composite()
{
  	std::cout << "> Running Compositor" << std::endl;

    std::map<int, std::vector<int>> intermediateStates = getIntermediateStates();
    std::vector<std::pair<int, int>> c;
    std::vector<std::pair<std::set<int>, std::set<int>>> compositeTargets;
    int count = 0;
    double averageA = 0;
    double averageB = 0;

    //std::cout << "Building 'c'..." << std::endl;
    std::cout << "Generating composition targets..." << std::endl;
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
                    	count++;
                    	averageA += (newTargets.first.size() - averageA) / count;
        				averageB += (newTargets.second.size() - averageB) / count;
                    }
    		        c.clear();
    	        }
    	    }
    	}
    }
    std::cout << "Found " << compositeTargets.size() << " composition target set pairs " << std::endl;
    std::cout << "Average length of A: " << averageA << std::endl;
    std::cout << "Average length of B: " << averageB << std::endl;
    compositeOperators = generateCompositeOperations(compositeTargets);
}

std::vector<std::vector<OperatorProxy>> compositor::generateCompositeOperations(std::vector<std::pair<std::set<int>, std::set<int>>> compositeTargets)
{
	std::cout << "Generating composite operations..." << std::endl;
	std::vector<std::vector<OperatorProxy>> compositionList;

    for (auto compositeTarget : compositeTargets)
    {
    	for (auto A : compositeTarget.first)
        {
        	std::vector<OperatorProxy> compositeOperation;
        	compositeOperation.push_back(taskProxy.get_operators()[A]);

            auto expandedOperations = expandCompositeOperation(compositeOperation, compositeTarget.second);
            compositionList.insert(compositionList.end(), expandedOperations.begin(), expandedOperations.end());

            if (expandedOperations.size() > 1) {compositedOperatorIDs.insert(A);}
        }
    }
    double sumOfLength = 0;
    for (auto compositeOperation : compositionList)
    {
    	sumOfLength += compositeOperation.size();
    }
    std::cout << "Generated " << compositionList.size() << " composite operations of average length: " << sumOfLength/compositionList.size() << std::endl;
    std::cout << compositedOperatorIDs.size() << " out of  " << taskProxy.get_operators().size() << " operators have been replaced with compositions." << std::endl;
    return compositionList;
}

std::vector<std::vector<OperatorProxy>> compositor::expandCompositeOperation(std::vector<OperatorProxy> compositeOperation, std::set<int> remainingTargets)
{
	std::vector<std::vector<OperatorProxy>>	compositionList;

    for (auto b : remainingTargets)
    {
    	std::vector<OperatorProxy> expandedCompositeOperation = compositeOperation;
        expandedCompositeOperation.push_back(taskProxy.get_operators()[b]);
    	if (isCompositeOperationExecutable(expandedCompositeOperation))
        {
        	compositedOperatorIDs.insert(b);
        	compositionList.push_back(expandedCompositeOperation);
        	std::set<int> targets = remainingTargets;
            targets.erase(b);
        	auto expandedOperations = expandCompositeOperation(expandedCompositeOperation, targets);
            compositionList.insert(compositionList.end(), expandedOperations.begin(), expandedOperations.end());
        }
    }

    return compositionList;
}

bool compositor::isCompositeOperationExecutable(std::vector<OperatorProxy> compositeOperation)
{
    std::map<int, int> state;
    for (int i = 0; i < compositeOperation.size(); i++)
    {
    	OperatorProxy op = compositeOperation[i];
        for (auto pre : op.get_preconditions())
        {
        	auto preVar = pre.get_pair().var;
        	auto preVal = pre.get_pair().value;
            if (state.count(preVar) > 0)
            {
            	if (state[preVar] != preVal)
                {
                	return false;
                }
            }
    	}
   		for (auto post : op.get_effects())
    	{
    		auto postVar = post.get_fact().get_pair().var;
    		auto postVal = post.get_fact().get_pair().value;
        	state[postVar] = postVal;
    	}
    }
    return true;
}

std::pair<std::set<int>, std::set<int>> compositor::getCompositeTargets(std::vector<std::pair<int, int>> c)
{
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
