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
        std::vector<std::vector<OperatorProxy>> expandCompositeOperation(std::vector<OperatorProxy> compositeOperation, std::set<int> targets);
        bool isCompositeOperationExecutable(std::vector<OperatorProxy> compositeOperation);
        tasks::ExplicitOperator createExplicitOperator(std::vector<OperatorProxy> compOp);
        bool isUniqueOperator(tasks::ExplicitOperator newOperator);
        bool areIdenticalOperators(tasks::ExplicitOperator a, tasks::ExplicitOperator b);
        bool notBIsCommutative(std::set<int> A, std::set<int> B, std::vector<std::pair<int, int>> c);
    public:
        compositor(std::shared_ptr<AbstractTask> abstractTask, bool enable)
            : abstractTask(abstractTask), taskProxy(*abstractTask)
        {
          if (enable) { composite(); }
        }
        std::shared_ptr<AbstractTask> getCompositedTask() {return compositedTask;}
};

#endif //COMPOSITOR_H
