#ifndef ABSTRACTOR_H
#define ABSTRACTOR_H

#include "domain_transition_graph.h"

class abstractor {
  private:
    static std::vector<std::unique_ptr<DomainTransitionGraph>> get_free_DTG(std::vector<std::unique_ptr<DomainTransitionGraph>> dtgs)

  public:
    static void find_safe_variables(std::shared_ptr<AbstractTask> orignial_task);
};

#endif //ABSTRACTOR_H