#include "abstractor.h"
#include "../abstract_task.h"
#include "../task_utils/causal_graph.h"
#include "../heuristics/domain_transition_graph.h"
#include "free_domain_transition_graph.h"
#include "../tasks/root_task.h"

std::list<int> abstractor::find_safe_variables(std::shared_ptr<AbstractTask> original_task )
{
  /*
  We can get the causal graph with the following line but as far as I can tell it is not required in any of the following
  calculations. Theoretically, any information in the causal graph is also in the DTG (in the form of pre- and
  postcons over other variables).
   */
  //const causal_graph::CausalGraph &causal_graph = causal_graph::get_causal_graph(original_task.get());

  //We need a TaskProxy but original_task is an AbstractTask.
  TaskProxy task_proxy(*original_task);

  /*
  This makes sure that postcons on diffrent variables are included in the DTG.
  This is very important information (I wasted a few hours trying to understand why postcons weren't handled)
  Make sure this is true
   */
  bool collect_side_effects = true;

  //Should anything be pruned?
  auto pruning_condition = [](int var, int fact_var) { return false; }; // Never prune any transitions

  //Get the DTGs for the task
  domain_transition_graph::DTGFactory dtg_factory(task_proxy, collect_side_effects, pruning_condition);
  std::vector<std::unique_ptr<domain_transition_graph::DomainTransitionGraph>> dtgs = dtg_factory.build_dtgs();

  std::list<int> safe_variables;

  std::cout << "============================ SAFE ABSTRACTOR ==========================" << std::endl;

  /*
  Calls the main function of abstractor to calculate the free domain transition graphs.
  At the same time it collects information about which values per variable are externally required and externally caused.
   */
  std::vector<std::unique_ptr<freeDTG>> free_dtgs = abstractor::get_free_domain_transition_graph(dtgs);

  for (const auto &free_dtg : free_dtgs)
  {
    // > Check for Strong Connectedness of externally required values
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
    free_dtg->printFreeDTG();
    free_dtg->printExternalInformation();
    printResults(extReqValAreStronglyConnected, allReqReachableByCaused, goalReachableByRequired, free_dtg.get());
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

std::vector<std::unique_ptr<freeDTG>> abstractor::get_free_domain_transition_graph(
    std::vector<std::unique_ptr<domain_transition_graph::DomainTransitionGraph>> &dtgs)
{
    //Create the (empty) free DTGs
	std::vector<std::unique_ptr<freeDTG>> free_dtgs;
	for (auto &dtg : dtgs)
	{
		freeDTG free_dtg(dtg->get_var(), dtg->get_nodes().size());
		free_dtgs.push_back(std::make_unique<freeDTG>(std::move(free_dtg)));
    }
    for (auto &dtg : dtgs)
    {
        int var_id = dtg->get_var();
        const vector<int> &loc_to_glob = dtg->local_to_global_child;

        std::cout << "variable: " << var_id << std::endl;
        //Get the free_dtg with the same variable as the dtg being worked on
        freeDTG free_dtg = *find_freeDTG_by_variable(free_dtgs, var_id);

        for (auto &node : dtg->get_nodes())
		{
        	std::cout << "  value: " << node.value << std::endl;
            for (auto &transition : node.transitions)
			{
            	std::cout << "    transition: " << var_id << "=" << node.value << "->" << transition.target->parent_graph->get_var() << "=" << transition.target->value << std::endl;
                bool is_free_transition = true;

                // Check preconditions and effects
                //auto &label = transition.labels[0];
                for (auto &label : transition.labels)
				{
				    std::cout << "      transition label: " << label.op_id << std::endl;
                  	std::cout << "        num of precons: " << label.precond.size() << std::endl;
                	std::cout << "        precon: ";
                	for (auto &precon : label.precond)
                    {
                        //int label_var = precon.local_var;
                        int label_var =  loc_to_glob[precon.local_var];
                        int label_val = precon.value;

                		/*
                		If the precon has a diffrent variable then the variable of the DTG then we know that the
                		transition should not be included in the free DTG.
                		 */
                        if (var_id != label_var)
                        {
                       		/*
                       		Get the free DTG with the variable of this precon and mark the required value as
                       		externally required.
                       		 */
                        	find_freeDTG_by_variable(free_dtgs, label_var)->externallyRequired(label_val);

                        	std::cout << "[" << label_var << " = " << label_val << "]";
                        	is_free_transition = false;
                        }
                        else { std::cout << label_var << " = " << label_val; }
                		std::cout << ", " ;
                    }
                    std::cout << std::endl;

                	std::cout << "        num of postcons: " << label.effect.size() << std::endl;
                	std::cout << "        postcon: ";
                	for (auto &postcon : label.effect)
                	{
                        //int label_var = postcon.local_var;
                        int label_var = loc_to_glob[postcon.local_var];
                        int label_val = postcon.value;

                        //Same logic as with the precons
                		if (var_id != label_var)
                		{
                		    /*
                       		Get the free DTG with the variable of this postcon and mark the required value as
                       		externally caused.
                       		 */
                			find_freeDTG_by_variable(free_dtgs, label_var)->externallyCaused(label_val);

                			std::cout << "[" << label_var << " = " << label_val << "]";
                			is_free_transition = false;
                		}
                		else { std::cout << label_var << " = " << label_val; }
                		std::cout << ", " ;
                	}
                	std::cout << std::endl;
                }

                if (is_free_transition) {
                    /*
                    If the transition is free (doesn't have precons or postcons on diffrent variables),
                    add it to the free DTG.
                     */
                    free_dtg.addTransition(node.value, transition.target->value);
                    std::cout << "      Transition is free" << std::endl;
                } else { std::cout << "      Transition is NOT free" << std::endl; }
            }
        }
    	std::cout << std::endl;
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

void abstractor::printResults(bool extReqValAreStronglyConnected, bool allReqReachableByCaused, bool goalReachableByRequired, freeDTG *free_dtg)
{
  	if (extReqValAreStronglyConnected)
    {
  		std::cout << "Externally required values of " << free_dtg->getVariable() << " are strongly connected in the free DTG" << std::endl;
    }
    else
    {
    	std::cout << "Externally required values of " << free_dtg->getVariable() << " are NOT strongly connected in the free DTG" << std::endl;
    }

    if (allReqReachableByCaused)
  	{
  		std::cout << "Externally required values of " << free_dtg->getVariable() << " are reachable by externally caused values" << std::endl;
  	}
  	else
  	{
  		std::cout << "Externally required values of " << free_dtg->getVariable() << " are NOT reachable by externally caused values" << std::endl;
  	}

  	if (goalReachableByRequired)
  	{
  		std::cout << "Goal value of variable " << free_dtg->getVariable() << " is reachable from externally required values" << std::endl;
  	}
  	else
  	{
  		std::cout << "Goal value of variable " << free_dtg->getVariable() << " is NOT reachable from externally required values" << std::endl;
  	}
}
