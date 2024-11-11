#include "compositor.h"

void compositor::composite()
{
  	std::cout << "> Running Compositor" << std::endl;

    if (taskProxy.get_variables().size() == 0)
    {
    	std::cout << "Task provided has no remaining variables" << std::endl;
        return;
    }

    std::vector<std::pair<std::set<int>, std::set<int>>> compositeTargets;
    int count = 0;
    double averageA = 0;
    double averageB = 0;

    std::vector<std::vector<std::pair<int, int>>> C = getC();

    for (auto c : C)
    {
		/*
		std::cout << "c: ";
    	for (auto fact : c)
    	{
    		std::cout << taskProxy.get_variables()[fact.first].get_name() << " = " << fact.second << ", ";
    	}
    	std::cout << std::endl;
		 */

    	std::pair<std::set<int>, std::set<int>> newTargets = getCompositeTargets(c);
    	if (!newTargets.first.empty() && notBIsCommutative(newTargets.first, newTargets.second, c))
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
        std::vector<std::vector<OperatorProxy>> compositeChain = generateCompositeOperations(compositeTargets);

        std::cout << "Creating decomposition map..." << std::endl;
    	int newIndex = taskProxy.get_operators().size();
    	for (auto c : compositeChain)
    	{
        	decompositOperations[newIndex] = c;
        	newIndex++;
    	}
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

            std::vector<OperatorProxy> compositeOperation;
        	compositeOperation.push_back(taskProxy.get_operators()[A]);

            tasks::ExplicitOperator emptyTask = tasks::ExplicitOperator(std::vector<FactPair>(), vector<tasks::ExplicitEffect>(), 0, "", false);
            auto expandedOperations = expandCompositeOperation(compositeOperation, B, emptyTask);
            compositionList.insert(compositionList.end(), expandedOperations.begin(), expandedOperations.end());

            if (expandedOperations.size() > 0) {compositedOperatorIDs.insert(A);} //If expanded chains are found mark A
        }
    }
    double sumOfLength = 0;
    for (auto compositeOperation : compositionList)
    {
    	sumOfLength += compositeOperation.size();
    }
    std::cout << "Generated " << compositionList.size() << " composite operatiors of average length: " << sumOfLength/compositionList.size() << std::endl;
    std::cout << compositedOperatorIDs.size() << " out of  " << taskProxy.get_operators().size() << " operators have been replaced with compositions." << std::endl;
    return compositionList;
}

std::vector<std::vector<OperatorProxy>> compositor::expandCompositeOperation(std::vector<OperatorProxy> compositeOperation, std::set<int> targets, tasks::ExplicitOperator parentOperator)
{
	std::vector<std::vector<OperatorProxy>>	compositionList;

    for (auto b : targets)
    {
    	std::vector<OperatorProxy> expandedCompositeOperation = compositeOperation;
        expandedCompositeOperation.push_back(taskProxy.get_operators()[b]);
    	if (isCompositeOperationExecutable(expandedCompositeOperation))
        {
        	tasks::ExplicitOperator newOperator = createExplicitOperator(expandedCompositeOperation);
            if (areIdenticalOperators(parentOperator, newOperator)) {return compositionList;}

            compositeOperators.push_back(newOperator);
        	compositedOperatorIDs.insert(b);
        	compositionList.push_back(expandedCompositeOperation);

        	auto expandedOperations = expandCompositeOperation(expandedCompositeOperation, targets, newOperator);
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
            else
            {
                state[preVar] = preVal;
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

tasks::ExplicitOperator compositor::createExplicitOperator(std::vector<OperatorProxy> compOp)
{
      std::map<int, int> state;

      vector<FactPair> preconditions;
      set<FactPair> preconditionsSet;
      vector<tasks::ExplicitEffect> effects;
      int cost;
      string name = "[CO:";
      bool is_an_axiom = false;

      for (auto op : compOp)
      {
          for (auto precon : op.get_preconditions())
          {
              FactPair fact(precon.get_pair().var, precon.get_pair().value);
              if (state.count(fact.var) == 0) { //Add precondition only if it isn't already covered by a previous operations
                  preconditionsSet.insert(fact);
              }
          }

          for (auto postcon : op.get_effects())
          {
              auto fact = postcon.get_fact().get_pair();
              state[fact.var] = fact.value;
          }
          cost += op.get_cost();
          name += op.get_name() + " > ";
      }

	  std::copy(preconditionsSet.begin(), preconditionsSet.end(), std::back_inserter(preconditions));

      for (auto postcon : state)
      {
          tasks::ExplicitEffect effect(postcon.first, postcon.second, vector<FactPair>());
          effects.push_back(effect);
      }

      name += "]";

      tasks::ExplicitOperator compositeOP = tasks::ExplicitOperator(preconditions, effects, cost, name, is_an_axiom);
      return compositeOP;
}

bool compositor::areIdenticalOperators(tasks::ExplicitOperator a, tasks::ExplicitOperator b)
{
	if (a.preconditions.size() != b.preconditions.size()) {return false;}
    if (a.effects.size() != b.effects.size()) {return false;}

    auto aPreCons = a.preconditions;
    auto bPreCons = b.preconditions;
    for (auto aPre : aPreCons)
    {
    	bool matched = false;
        for (auto bPre : bPreCons)
        {
        	if (aPre.var == bPre.var && aPre.value == bPre.value)
        	{
                matched = true;
        		break;
            }
        }
        if (!matched) {return false;}
    }

    auto aPostCons = a.effects;
    auto bPostCons = b.effects;
    for (auto aPost : aPostCons)
    {
    	bool matched = false;
        for (auto bPost : bPostCons)
        {
        	if (aPost.fact.var == bPost.fact.var && aPost.fact.value == bPost.fact.value)
        	{
                matched = true;
        		break;
            }
        }
        if (!matched) {return false;}
    }
    return true;
}

bool compositor::notBIsCommutative(std::set<int> A, std::set<int> B, std::vector<std::pair<int, int>> c)
{
    bool isCommutative = true;
	std::set<int> notB;
    std::set<int> AUB = A;
    AUB.insert(B.begin(), B.end());

    for (auto op : taskProxy.get_operators())
    {
    	if (B.count(op.get_id()) == 0)
    	{
        	bool consistentWithC = true;

        	for (auto pre : op.get_preconditions())
            {
                bool factConsistent = false;
            	for (auto fact : c)
                {
                	if (pre.get_pair().var == fact.first && pre.get_pair().value == fact.second) {consistentWithC = true; break;}
                }
                if (!factConsistent) {consistentWithC = false; break;}
            }
            if (consistentWithC) {notB.insert(op.get_id());}
        }
    }

    for (auto notb : notB)
    {
    	auto notbOp = taskProxy.get_operators()[notb];
        for (auto aub : AUB)
        {
        	auto aubOp = taskProxy.get_operators()[aub];
            for (auto notbEffect : notbOp.get_effects())
        	{
        		for (auto aubPrecon : aubOp.get_preconditions()){
           			if (notbEffect.get_fact().get_pair().var == aubPrecon.get_pair().var) {
						if (notbEffect.get_fact().get_pair().value != aubPrecon.get_pair().value)
                        {
                            std::cout << notbOp.get_name() << " and " << aubOp.get_name() << " are not commutative (notb.effect -> aub.precon)" << std::endl;
                        	return false;
                        }
                    }
            	}

                for (auto aubEffect : aubOp.get_effects()){
                	if (notbEffect.get_fact().get_pair().var == aubEffect.get_fact().get_pair().var) {
                        if (notbEffect.get_fact().get_pair().value != aubEffect.get_fact().get_pair().value)
                        {
                        	std::cout << notbOp.get_name() << " and " << aubOp.get_name() << " are not commutative (notb.effect <-> aub.effect)" << std::endl;
							return false;
                        }
                    }
        		}
        	}

            for (auto aubEffect : aubOp.get_effects()){
        		for (auto notbPrecon : notbOp.get_preconditions()){
           			if (aubEffect.get_fact().get_pair().var == notbPrecon.get_pair().var){
                    	if (aubEffect.get_fact().get_pair().value != notbPrecon.get_pair().value)
                        {
                        	std::cout << notbOp.get_name() << " and " << aubOp.get_name() << " are not commutative (notb.precon <- aub.effect)" << std::endl;
							return false;
                        }
                	}
            	}
        	}
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
    	//std::cout << "Checking if " << op.get_name() << " should be in A" << std::endl;
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
        	//std::cout << "Adding " << op.get_name() << " to A" << std::endl;
        }
    	else
    	{
    		//std::cout << op.get_name() << " does not contain c in its effects" << std::endl;
    	}
    }
    if (A.size() == 0) {
    	//std::cout << "Skipping: A is empty" << std::endl;
    	return std::make_pair(std::set<int>(), std::set<int>());
    }

    for (OperatorProxy op : taskProxy.get_operators())
    {
        //std::cout << "Checking if " << op.get_name() << " should be in B" << std::endl;
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
        	//std::cout << "Adding " << op.get_name() << " to B" << std::endl;
        }
        else
        {
        	//std::cout << op.get_name() << " does not contain c in its preconditions" << std::endl;
        }
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

    std::cout << "Removing invalid c..." << std::endl;

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
