#include "abstractor.h"
#include "../abstract_task.h"
#include "../task_utils/causal_graph.h"
#include "../heuristics/domain_transition_graph.h"
#include "free_domain_transition_graph.h"
#include "../tasks/root_task.h"

std::list<int> abstractor::find_safe_variables(std::shared_ptr<AbstractTask> original_task )
{
  //We need a TaskProxy but original_task is an AbstractTask.
  TaskProxy task_proxy(*original_task);

  std::cout << "============================ SAFE ABSTRACTOR ==========================" << std::endl;
  printTask(task_proxy);
  printOperations(task_proxy);

  std::list<int> safe_variables;

  /*
  Calls the main function of abstractor to calculate the free domain transition graphs.
  At the same time it collects information about which values per variable are externally required and externally caused.
   */
  std::vector<std::unique_ptr<freeDTG>> free_dtgs = abstractor::get_free_domain_transition_graph(task_proxy);

  for (const auto &free_dtg : free_dtgs)
  {
    //Check for Strong Connectedness of externally required values
  	std::list<int> externallyRequiredValues;
  	//Convert bool list into a list of ints
  	for (int i = 0; i < free_dtg->getExternallyRequiredValues().size(); ++i) {
  		if (free_dtg->getExternallyRequiredValues()[i]) {externallyRequiredValues.push_back(i);}
  	}

    bool extReqValAreStronglyConnected = free_dtg->isStronglyConnected(externallyRequiredValues);

	// > Check for Reachability of externally required values from externally caused values
  	std::list<int> externallyCausedValues;
  	//Convert bool list into a list of ints
  	for (int i = 0; i < free_dtg->getExternallyCausedValues().size(); ++i) {
  		if (free_dtg->getExternallyCausedValues()[i]) {externallyCausedValues.push_back(i);}
  	}

    bool allReqReachableByCaused = true;
    //Check if all externally required values are reachable from the externally caused values
    for (int extCausedVal : externallyCausedValues)
    {
    	if (!free_dtg->isReachable(extCausedVal, externallyRequiredValues))
        {
        	allReqReachableByCaused = false;
            break;
        }
    }

    // > Check for Reachability of goal value from externally required values
    bool hasGoal = false;
    int goalValue;
    GoalsProxy goals = task_proxy.get_goals();

    for (int i = 0; i < goals.size(); ++i) //Is there a better way to do this?
    {
    	FactProxy goalFact = goals[i];
    	if (goalFact.get_variable().get_id() == free_dtg->getVariable())
    	{
    		goalValue = goalFact.get_value();
    		hasGoal = true;
    		break;
    	}
    }

    bool goalReachableByRequired = true;
    if (hasGoal)
    {
        //Check if goal value is reachable from the externally required values
        for (int extReqVal : externallyRequiredValues)
        {
     	    //Since the externally req val would have to be stronly connected,
     	    // being reachable from any ext.req.val should be sufficient to proof this property.
    	    if (!free_dtg->isReachable(extReqVal, { goalValue }))
            {
        	    goalReachableByRequired = false;
                break;
            }
        }
    }


	//Print results
    free_dtg->printFreeDTG(original_task);
    free_dtg->printExternalInformation(original_task);
    printResults(original_task, extReqValAreStronglyConnected, allReqReachableByCaused, goalReachableByRequired, free_dtg.get());
    if (hasGoal) { std::cout << "Goal State: " << goalValue << std::endl; } else { std::cout << "No Goal State found" << std::endl; }
  	std::cout << std::endl;

    if (extReqValAreStronglyConnected && allReqReachableByCaused && goalReachableByRequired) //TODO: Check if goal value is free reachable from all externally required values
    {
    	safe_variables.push_back(free_dtg->getVariable());
    }
  }
  std::cout << "=======================================================================" << std::endl;

  return safe_variables;
}

std::vector<std::unique_ptr<freeDTG>> abstractor::get_free_domain_transition_graph(TaskProxy task_proxy)
{
    //Create the (empty) free DTGs
	std::vector<std::unique_ptr<freeDTG>> free_dtgs;
    for (VariableProxy variable : task_proxy.get_variables())
    {
		freeDTG free_dtg(variable.get_id(), variable.get_domain_size());
		free_dtgs.push_back(std::make_unique<freeDTG>(std::move(free_dtg)));
    }

    for (auto op : task_proxy.get_operators())
    {
    	std::vector<std::pair<int, int>> precon_facts;
    	for (auto precondition : op.get_preconditions())
        {
        	int precon_var = precondition.get_variable().get_id();
            int precon_val = precondition.get_value();
            precon_facts.push_back(std::make_pair(precon_var, precon_val));
        }

        std::vector<std::pair<int, int>> postcon_facts;
        for (auto postcondition : op.get_effects())
        {
        	auto postcondition_fact = postcondition.get_fact();
            int postcon_var = postcondition_fact.get_variable().get_id();
            int postcon_val = postcondition_fact.get_value();
            postcon_facts.push_back(std::make_pair(postcon_var, postcon_val));
        }

        bool freeOperation = false;

        if (precon_facts.size() < 2 && postcon_facts.size() < 2)
        {
        	//if (precon_facts.size() == 0) { freeOperation = true; }
            //if (postcon_facts.size() == 0) { freeOperation = true; }
            if (precon_facts.size() == 1 && postcon_facts.size() == 1)
            {
            	if (precon_facts[0].first == postcon_facts[0].first)
                {
                	freeOperation = true;
        			freeDTG free_dtg = *find_freeDTG_by_variable(free_dtgs, precon_facts[0].first);
                	free_dtg.addTransition(precon_facts[0].second, postcon_facts[0].second);
                }
            }
        }

        if (!freeOperation)
        {
        	//Mark relevant precons as externally required
        	for (auto precon_fact : precon_facts) {
            	for (auto postcon_fact : postcon_facts) {
            		if (precon_fact.first != postcon_fact.first)
            		{
            			find_freeDTG_by_variable(free_dtgs, precon_fact.first)->externallyRequired(precon_fact.second);
            		}
            	}
        	}

        	//Mark relevant postcons as externally caused
        	//This can be optimised by not looping over all postcons for each postcon, instead only those who wasn't compared to yet.
        	for (auto postcon_fact_1 : postcon_facts) {
        		for (auto postcon_fact_2 : postcon_facts) {
                	if (postcon_fact_1.first != postcon_fact_2.first)
            		{
            			find_freeDTG_by_variable(free_dtgs, postcon_fact_1.first)->externallyCaused(postcon_fact_1.second);
                    	find_freeDTG_by_variable(free_dtgs, postcon_fact_2.first)->externallyCaused(postcon_fact_2.second);
            		}
            	}
        	}
        }
    }
	return free_dtgs;
}

freeDTG* abstractor::find_freeDTG_by_variable(
    std::vector<std::unique_ptr<freeDTG>> &free_dtgs, int i)
{
	for (const auto &free_dtg : free_dtgs) {
		if (free_dtg->getVariable() == i) {
			return free_dtg.get();
		}
	}
	return nullptr;
}

void abstractor::printResults(std::shared_ptr<AbstractTask> original_task, bool extReqValAreStronglyConnected, bool allReqReachableByCaused, bool goalReachableByRequired, freeDTG *free_dtg)
{
  	if (extReqValAreStronglyConnected)
    {
  		std::cout << "Externally required values of " << original_task->get_variable_name(free_dtg->getVariable()) << " are strongly connected in the free DTG" << std::endl;
    }
    else
    {
    	std::cout << "Externally required values of " << original_task->get_variable_name(free_dtg->getVariable()) << " are NOT strongly connected in the free DTG" << std::endl;
    }

    if (allReqReachableByCaused)
  	{
  		std::cout << "Externally required values of " << original_task->get_variable_name(free_dtg->getVariable()) << " are reachable by externally caused values" << std::endl;
  	}
  	else
  	{
  		std::cout << "Externally required values of " << original_task->get_variable_name(free_dtg->getVariable()) << " are NOT reachable by externally caused values" << std::endl;
  	}

  	if (goalReachableByRequired)
  	{
  		std::cout << "Goal value of variable " << original_task->get_variable_name(free_dtg->getVariable()) << " is reachable from externally required values" << std::endl;
  	}
  	else
  	{
  		std::cout << "Goal value of variable " << original_task->get_variable_name(free_dtg->getVariable()) << " is NOT reachable from externally required values" << std::endl;
  	}
}

void abstractor::printOperations(TaskProxy task_proxy)
{
    cout << "Operations of task:" << endl;
    for (auto op : task_proxy.get_operators())
    {
        cout << "  " << op.get_name() << endl;
        cout << "    " << "Precon: ";
    	for (auto precondition : op.get_preconditions())
        {
        	auto precon_var = precondition.get_variable();
            int precon_val = precondition.get_value();
            cout << precon_var.get_name() << " = " << precon_val << ", ";
        }
		cout << endl << "    " << "Postcon: ";
        for (auto postcondition : op.get_effects())
        {
        	auto postcondition_fact = postcondition.get_fact();
            auto postcon_var = postcondition_fact.get_variable();
            int postcon_val = postcondition_fact.get_value();
            cout << postcon_var.get_name() << " = " << postcon_val << ", ";
        }
        cout << endl << endl;
    }
}

void abstractor::printTask(TaskProxy task_proxy)
{
    cout << endl;
    cout << "Task: " << endl;
    for (VariableProxy variable : task_proxy.get_variables())
    {
       	cout << "  " << variable.get_name() << std::endl;
        for (int i = 0; i < variable.get_domain_size(); i++)
       	{
           	cout << "    " << variable.get_fact(i).get_name() << std::endl;
       	}
   	}
   	cout << std::endl;
}

