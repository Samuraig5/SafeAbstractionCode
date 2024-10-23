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
        std::vector<std::vector<OperatorProxy>> getCompositeTargets(std::map<int, int> c);
    public:
        compositor(std::shared_ptr<AbstractTask> abstractTask)
            : abstractTask(abstractTask), taskProxy(*abstractTask)
        {composite();}
        std::shared_ptr<AbstractTask> getCompositedTask() {return taskProxy.getCompositedTask();}
};

#endif //COMPOSITOR_H
