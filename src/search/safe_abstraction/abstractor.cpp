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
}

domain_transition_graph abstractor::get_free_domain_transition_graph(std::vector<std::unique_ptr<DomainTransitionGraph>> dtgs)
{

}
