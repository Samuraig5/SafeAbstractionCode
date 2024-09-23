#include "abstractor.h"
#include "AbstractTask.h"
#include "causal_graph.h"
#include "domain_transition_graph.h"

void abstractor::find_safe_variables(std::shared_ptr<AbstractTask> orignial_task)
{
  //Get the causal graph for the task
  const causal_graph &causal_graph = get_causal_graph(original_task.get());

  //I need a TaskProxy but orignial_task is an AbstractTask.
  TaskProxy task_proxy(*tasks::g_root_task);
  //Should this be false or true?
  bool collect_side_effects = false;
  //Should anything be pruned?
  auto pruning_condition = [](int var, int fact_var) { return false; }; // Never prune any transitions

  //Get the DTGs for the task
  domain_transition_graph::DTGFactory dtg_factory(task_proxy, collect_side_effects, pruning_condition);
  std::vector<std::unique_ptr<DomainTransitionGraph>> dtgs = dtg_factory.build_dtgs();

  std::vector<std::unique_ptr<DomainTransitionGraph>> free_dtgs = get_free_dtgs(dtgs);
}

std::vector<std::unique_ptr<DomainTransitionGraph>> abstractor::get_free_domain_transition_graph(std::vector<std::unique_ptr<DomainTransitionGraph>> dtgs)
{
	std::vector<std::unique_ptr<DomainTransitionGraph>> free_dtgs;

	for (const auto &dtg : dtgs)
	{
        //TODO: Create freeDTG object
        for (const auto &node : dtg->nodes)
		{
        	bool nodeHandled = false;
            for (const auto &transition : node.transitions)
			{
                bool is_free_transition = true;

                // Check preconditions and effects
                for (const auto &label : transition.labels)
				{
                	for (const auto &precon : label.preconditions)
                    {
                        //Skip if variable is not a diffrent variable
                        if (var_id != precon.local_var) //TODO: Figure out how local vs global vars behave
                        {
                        	is_free_transition = false;
                    		break;
                        }
                    }
                    if (!is_free_transition) //If it is not a free transition we don't have to check for postconditions
               		{
                    	for (const auto &postcon : label.postconditions)
                    	{
                        	//Skip if variable is not a diffrent variable
                        	if (var_id != precon.local_var) //TODO: Figure out how local vs global vars behave
                        	{
                        		is_free_transition = false;
                    			break;
                        	}
                    	}
                    }
                }

                if (is_free_transition) {
                    //TODO: Add free nodes and free transitions to freeDTG
                }
            }
        }
        //TODO: Add new freeDTG to free_dtgs
	}

	return free_dtgs;
}
