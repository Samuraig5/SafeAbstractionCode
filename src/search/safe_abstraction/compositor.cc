#include "compositor.h"

void compositor::composite()
{
  	std::cout << "> Running Compositor" << std::endl;

    std::vector<std::pair<std::set<int>, std::set<int>>> compositeTargets;
    int count = 0;
    double averageA = 0;
    double averageB = 0;

    std::vector<std::vector<std::pair<int, int>>> C = getC();

    for (auto c : C)
    {
		std::cout << "c: ";
    	for (auto fact : c)
    	{
    		std::cout << taskProxy.get_variables()[fact.first].get_name() << " = " << fact.second << ", ";
    	}
    	std::cout << std::endl;

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

    std::cout << "Found " << compositeTargets.size() << " composition target set pairs " << std::endl;
    if (compositeTargets.size() > 0)
    {
        std::cout << "Average length of A: " << averageA << std::endl;
        std::cout << "Average length of B: " << averageB << std::endl;
        compositeOperators = generateCompositeOperations(compositeTargets);
    }
}

std::vector<std::vector<OperatorProxy>> compositor::generateCompositeOperations(std::vector<std::pair<std::set<int>, std::set<int>>> compositeTargets)
{
	std::cout << "Generating composite operations..." << std::endl;
	std::vector<std::vector<OperatorProxy>> compositionList;

    for (auto compositeTarget : compositeTargets)
    {
    	for (auto A : compositeTarget.first)
        {
            auto B = compositeTarget.second;
            B.erase(A);
        	std::vector<OperatorProxy> compositeOperation;
        	compositeOperation.push_back(taskProxy.get_operators()[A]);

            auto expandedOperations = expandCompositeOperation(compositeOperation, B);
            compositionList.insert(compositionList.end(), expandedOperations.begin(), expandedOperations.end());

            if (expandedOperations.size() > 0) {compositedOperatorIDs.insert(A);} //If expanded chains are found mark A
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
               		std::cout << "Precondition violated in current state" << std::endl;
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
    	std::cout << "Checking if " << op.get_name() << " should be in A" << std::endl;
        auto effects = op.get_effects();
        bool containsC = true;

        if (effects.size() < c.size() )
        {
        	containsC = false;
        	//std::cout << op.get_name() << " doesn't contain c in its effects" << std::endl;
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
        if (containsC)
        {
        	A.insert(op.get_id());
        	std::cout << "Adding " << op.get_name() << " to A" << std::endl;
        }
    	else
    	{
    		//std::cout << op.get_name() << " does not contain c in its effects" << std::endl;
    	}
    }
    if (A.size() == 0) {
    	std::cout << "Skipping: A is empty" << std::endl;
    	return std::make_pair(std::set<int>(), std::set<int>());
    }

    for (OperatorProxy op : taskProxy.get_operators())
    {
        std::cout << "Checking if " << op.get_name() << " should be in B" << std::endl;
        auto preconditions = op.get_preconditions();
        bool containsC = true;

        if (preconditions.size() < c.size() )
        {
        	containsC = false;
        	//std::cout << op.get_name() << " doesn't contain c in its preconditions" << std::endl;
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
        if (containsC)
        {
        	B.insert(op.get_id());
        	std::cout << "Adding " << op.get_name() << " to B" << std::endl;
        }
        else
        {
        	//std::cout << op.get_name() << " does not contain c in its preconditions" << std::endl;
        }
    }
    if (B.size() == 0)
    {
        std::cout << "Skipping: B is empty" << std::endl;
    	return std::make_pair(std::set<int>(), std::set<int>());
    }
    std::cout << "A has " << A.size() << " actions" << std::endl;
    std::cout << "B has " << B.size() << " actions" << std::endl << std::endl;

    return std::make_pair(A, B);
}

std::vector<std::vector<std::pair<int, int>>> compositor::getC()
{
    std::cout << "Getting c..." << std::endl;

    std::vector<std::vector<std::pair<int, int>>> possibleC;

    for (int i = 0; i < taskProxy.get_variables().size()-1; i++)
    {
        for (int j = i+1; j < taskProxy.get_variables().size(); j++)
    	{
    		for (int ii = 0; ii < taskProxy.get_variables()[i].get_domain_size(); ii++)
    	    {
    			for (int jj = 0; jj < taskProxy.get_variables()[j].get_domain_size(); jj++)
    			{
    	        	std::vector<std::pair<int, int>> c;
    	        	c.push_back(std::make_pair(i, ii));
    	        	c.push_back(std::make_pair(j, jj));
    	        	possibleC.push_back(c);
    	        }
    	    }
    	}
    }

    std::vector<int> initialStates = abstractTask->get_initial_state_values();
    GoalsProxy goalFacts = taskProxy.get_goals();

    std::vector<std::vector<std::pair<int, int>>> C;
	for (auto c : possibleC)
	{
		bool trueInitially = true;
		bool trueInGoal = true;
		for (auto fact : c)
		{
			if (initialStates[fact.first] != fact.second) {trueInitially = false;}
        	for (auto goal : goalFacts){
            	if (goal.get_variable().get_id() != fact.first){ } //trueInGoal = false; }
            	else if (goal.get_value() != fact.second){ trueInGoal = false; }
        	}
		}
		if (!trueInitially && !trueInGoal) {C.push_back(c);}
	}

    //std::cout << "Found " << C.size() << " c sets" << std::endl;
    return C;
}
