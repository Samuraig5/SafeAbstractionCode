#ifndef ABSTRACTOR_H
#define ABSTRACTOR_H

class abstractor {
  private:
    static domain_transition_graph::DomainTransitionGraph get_free_DTG(std::vector<std::unique_ptr<DomainTransitionGraph>> dtgs)

  public:
    static void find_safe_variables(std::shared_ptr<AbstractTask> orignial_task);
};

#endif //ABSTRACTOR_H