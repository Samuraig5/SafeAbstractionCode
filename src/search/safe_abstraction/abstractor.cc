#include "abstractor.h"
#include "../abstract_task.h"
#include "../task_utils/causal_graph.h"
#include "../heuristics/domain_transition_graph.h"
#include "free_domain_transition_graph.h"

void abstractor::find_safe_variables(std::shared_ptr<AbstractTask> original_task )
{
  //Get the causal graph for the task
  const causal_graph::CausalGraph &causal_graph = causal_graph::get_causal_graph(original_task.get());

  //I need a TaskProxy but original_task is an AbstractTask.
  TaskProxy task_proxy(*original_task);

  //Should this be false or true?
  bool collect_side_effects = false;
  //Should anything be pruned?
  auto pruning_condition = [](int var, int fact_var) { return false; }; // Never prune any transitions

  //Get the DTGs for the task
  domain_transition_graph::DTGFactory dtg_factory(task_proxy, collect_side_effects, pruning_condition);
  std::vector<std::unique_ptr<domain_transition_graph::DomainTransitionGraph>> dtgs = dtg_factory.build_dtgs();

  std::vector<std::unique_ptr<freeDTG>> free_dtgs = abstractor::get_free_domain_transition_graph(dtgs);
}

std::vector<std::unique_ptr<freeDTG>> abstractor::get_free_domain_transition_graph(
    std::vector<std::unique_ptr<domain_transition_graph::DomainTransitionGraph>> &dtgs)
{
	std::vector<std::unique_ptr<freeDTG>> free_dtgs;

	for (const auto &dtg : dtgs)
	{
        int var_id = dtg->get_var();
        std::vector<domain_transition_graph::ValueNode> nodes = dtg->get_nodes();
        freeDTG free_dtg(var_id, nodes.size());

        for (const auto &node : nodes)
		{
            for (const auto &transition : node.transitions)
			{
                bool is_free_transition = true;

                // Check preconditions and effects
                for (const auto &label : transition.labels)
				{
                	for (const auto &precon : label.precond)
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
                    	for (const auto &postcon : label.effect)
                    	{
                        	//Skip if variable is not a diffrent variable
                        	if (var_id != postcon.local_var) //TODO: Figure out how local vs global vars behave
                        	{
                        		is_free_transition = false;
                    			break;
                        	}
                    	}
                    }
                }

                if (is_free_transition) {
                    /*
                    TODO: Make sure node.value (and target.value) doesn't break anything here or fix it.
                    The idea should work but this implementation probably doesn't since values can be arbitrary but we
                    have a limited range defined by free_dtg.numVal.
                    */
                    free_dtg.addTransition(node.value, transition.target->value);
                }
            }
        }
        free_dtgs.push_back(std::make_unique<freeDTG>(std::move(free_dtg)));
	}

	return free_dtgs;
}
