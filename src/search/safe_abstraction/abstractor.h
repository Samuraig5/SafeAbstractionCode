#ifndef ABSTRACTOR_H
#define ABSTRACTOR_H

#include "../heuristics/domain_transition_graph.h"
#include "free_domain_transition_graph.h"

class abstractor {
  private:
    static std::vector<std::unique_ptr<freeDTG>> get_free_domain_transition_graph(
      std::vector<std::unique_ptr<domain_transition_graph::DomainTransitionGraph>> &dtgs);
    static freeDTG* find_freeDTG_by_variable(
        std::vector<std::unique_ptr<freeDTG>> &free_dtgs, int i);



  public:
    static void find_safe_variables(std::shared_ptr<AbstractTask> original_task);
};

#endif //ABSTRACTOR_H