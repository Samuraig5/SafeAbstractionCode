#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include "../abstract_task.h"
#include "../task_proxy.h"
#include "../tasks/root_task.h"
#include "vector"
#include "map"
#include "set"

class compositor {
std::shared_ptr<AbstractTask> abstractTask;
TaskProxy taskProxy;
std::shared_ptr<AbstractTask> compositedTask;

public:
std::set<int> compositedOperatorIDs;
std::vector<tasks::ExplicitOperator> compositeOperators;
std::map<int, std::vector<OperatorProxy>> decompositOperations;

    private:
        void composite();
        std::vector<std::vector<std::pair<int, int>>> getC();
        std::pair<std::set<int>, std::set<int>> getCompositeTargets(std::vector<std::pair<int, int>> c);
        std::vector<std::vector<OperatorProxy>> generateCompositeOperations(std::vector<std::pair<std::set<int>, std::set<int>>> compositeTargets);
        std::vector<std::vector<OperatorProxy>> expandCompositeOperation(std::vector<OperatorProxy> compositeOperation, std::set<int> remainingTargets, tasks::ExplicitOperator parentOperator);
        bool isCompositeOperationExecutable(std::vector<OperatorProxy> compositeOperation);
        tasks::ExplicitOperator createExplicitOperator(std::vector<OperatorProxy> compOp);
        bool areIdenticalOperators(tasks::ExplicitOperator a, tasks::ExplicitOperator b);
    public:
        compositor(std::shared_ptr<AbstractTask> abstractTask)
            : abstractTask(abstractTask), taskProxy(*abstractTask)
        {composite();}
        std::shared_ptr<AbstractTask> getCompositedTask() {return compositedTask;}
};

#endif //COMPOSITOR_H
