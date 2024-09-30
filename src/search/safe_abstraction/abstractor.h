#ifndef ABSTRACTOR_H
#define ABSTRACTOR_H

#include "../heuristics/domain_transition_graph.h"
#include "free_domain_transition_graph.h"
#include "../tasks/root_task.h"


class abstractor {
  private:
    static std::vector<std::unique_ptr<freeDTG>> get_free_domain_transition_graph(
    	std::shared_ptr<AbstractTask> original_task,
    	std::vector<std::unique_ptr<domain_transition_graph::DomainTransitionGraph>> &dtgs);
    static freeDTG* find_freeDTG_by_variable(
        std::vector<std::unique_ptr<freeDTG>> &free_dtgs, int i);
    static void printResults(std::shared_ptr<AbstractTask> original_task, bool extReqValAreStronglyConnected, bool allReqReachableByCaused, bool goalReachableByRequired, freeDTG *free_dtg);
    static void printDTG(std::shared_ptr<AbstractTask> original_task, std::unique_ptr<domain_transition_graph::DomainTransitionGraph> &dtg);


  public:
    static std::list<int> find_safe_variables(std::shared_ptr<AbstractTask> original_task);
};

#endif //ABSTRACTOR_H