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

  std::cout << "============================ SAFE ABSTRACTOR ==========================" << std::endl;
  std::vector<std::unique_ptr<freeDTG>> free_dtgs = abstractor::get_free_domain_transition_graph(dtgs);

  for (const auto &free_dtgs : free_dtgs)
  {
  	free_dtgs->printFreeDTG();
    free_dtgs->printExternalInformation();

  	std::list<int> externallyRequiredValues;
  	for (int i = 0; i < free_dtgs->getExternallyRequiredValues().size(); ++i) {
  		if (free_dtgs->getExternallyRequiredValues()[i]) {externallyRequiredValues.push_back(i);}
  	}
  	if (free_dtgs->isStronglyConnected(externallyRequiredValues))
    {
  		std::cout << "Externally required values of " << free_dtgs->getVariable() << " are strongly connected in the free DTG" << std::endl;
    }
    else
    {
    	std::cout << "Externally required values of " << free_dtgs->getVariable() << " are NOT strongly connected in the free DTG" << std::endl;
    }

    std::cout << std::endl;
  }
  std::cout << "=======================================================================" << std::endl;
}

std::vector<std::unique_ptr<freeDTG>> abstractor::get_free_domain_transition_graph(
    std::vector<std::unique_ptr<domain_transition_graph::DomainTransitionGraph>> &dtgs)
{
	std::vector<std::unique_ptr<freeDTG>> free_dtgs;

	for (const auto &dtg : dtgs)
	{
		freeDTG free_dtg(dtg->get_var(), dtg->get_nodes().size());
		free_dtgs.push_back(std::make_unique<freeDTG>(std::move(free_dtg)));
    }
    for (const auto &dtg : dtgs)
    {
        int var_id = dtg->get_var();
        std::cout << "safe_abstraction > working on DTG of variable: " << var_id << std::endl;
        std::vector<domain_transition_graph::ValueNode> nodes = dtg->get_nodes();
        freeDTG free_dtg = *find_freeDTG_by_variable(free_dtgs, var_id);

        for (const auto &node : nodes)
		{
        	std::cout << "  node: " << node.value << std::endl;
            for (const auto &transition : node.transitions)
			{
            	std::cout << "    transition: " << node.value << "->" << transition.target->value << std::endl;
                bool is_free_transition = true;

                // Check preconditions and effects
                for (const auto &label : transition.labels)
				{
                  	std::cout << "      num of precons: " << label.precond.size() << std::endl;
                	std::cout << "      precon: ";
                	for (const auto &precon : label.precond)
                    {
                		std::cout << precon.local_var << " = " << precon.value;
                        //Skip if variable is not a diffrent variable
                        if (var_id != precon.local_var) //TODO: Figure out how local vs global vars behave
                        {
                        	find_freeDTG_by_variable(free_dtgs, precon.local_var)->externallyRequired(precon.value);

                        	std::cout << " <- precon on variable: " << precon.local_var << "!";;
                        	is_free_transition = false;
                    		break;
                        }
                		std::cout << ", " ;
                    }
                    std::cout << std::endl;

                	std::cout << "      num of postcons: " << label.effect.size() << std::endl;
                	std::cout << "      postcon: ";
                	for (const auto &postcon : label.effect)
                	{
                		std::cout << postcon.local_var << " = " << postcon.value;
                		//Skip if variable is not a diffrent variable
                		if (var_id != postcon.local_var) //TODO: Figure out how local vs global vars behave
                		{
                			find_freeDTG_by_variable(free_dtgs, postcon.local_var)->externallyCaused(postcon.value);

                			std::cout << " <- postcon on variable: " << postcon.local_var << "!";
                			is_free_transition = false;
                			break;
                		}
                		std::cout << ", " ;
                	}
                	std::cout << std::endl;
                    std::cout << "    Is this transition free?: " << is_free_transition << std::endl;
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
