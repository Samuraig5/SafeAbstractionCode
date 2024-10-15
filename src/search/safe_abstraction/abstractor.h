#ifndef ABSTRACTOR_H
#define ABSTRACTOR_H

#include "../heuristics/domain_transition_graph.h"
#include "free_domain_transition_graph.h"
#include "../tasks/root_task.h"


class abstractor {
  std::shared_ptr<AbstractTask> abstractTask;
  TaskProxy taskProxy;
  std::vector<freeDTG> freeDTGs;

  private:
    void create_free_domain_transition_graphs();
    void printResults(bool extReqValAreStronglyConnected, bool allReqReachableByCaused, bool goalReachableByRequired, freeDTG *free_dtg);
    void printOperations();
    void printTask();

  public:
      abstractor(std::shared_ptr<AbstractTask> abstractTask)
          : abstractTask(abstractTask), taskProxy(*abstractTask)
      {}
      std::list<int> find_safe_variables();
      std::shared_ptr<AbstractTask> getAbstractTask() { return abstractTask; }
      TaskProxy getTaskProxy() { return taskProxy; }
      freeDTG* find_freeDTG_by_variable(int var_id);
};

#endif //ABSTRACTOR_H