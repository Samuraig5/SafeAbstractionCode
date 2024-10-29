#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include "../abstract_task.h"
#include "../task_proxy.h"
#include "vector"
#include "map"
#include "set"

class compositor {
std::shared_ptr<AbstractTask> abstractTask;
TaskProxy taskProxy;
std::shared_ptr<AbstractTask> compositedTask;

    private:
        void composite();
        std::map<int, std::vector<int>> getIntermediateStates();
        std::pair<std::set<int>, std::set<int>> getCompositeTargets(std::vector<std::pair<int, int>> c);
        std::vector<std::vector<OperationProxy>> generateCompositeOperations(std::vector<std::pair<std::set<int>, std::set<int>>> compositeTargets)
    public:
        compositor(std::shared_ptr<AbstractTask> abstractTask)
            : abstractTask(abstractTask), taskProxy(*abstractTask)
        {composite();}
        std::shared_ptr<AbstractTask> getCompositedTask() {return compositedTask;}
};

#endif //COMPOSITOR_H
