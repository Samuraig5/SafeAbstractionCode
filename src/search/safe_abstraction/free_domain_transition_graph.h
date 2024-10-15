#ifndef FREE_DOMAIN_TRANSITION_GRAPH_H
#define FREE_DOMAIN_TRANSITION_GRAPH_H

#include "../abstract_task.h"
#include <list>
#include <vector>

struct Transition {
    int destination;
    int operation_id;

    Transition(int dest, int op_id) : destination(dest), operation_id(op_id) {}
};

class freeDTG
{
  int variable;
  int numVal; // Number of values
  std::list<Transition> *transitions;
  std::vector<bool> externallyRequiredValues;
  std::vector<bool> externallyCausedValues;

  public:
    freeDTG(int var, int numVal);
    int getVariable() {return variable;}
    std::vector<bool> getExternallyRequiredValues() { return externallyRequiredValues; }
    std::vector<bool> getExternallyCausedValues() {return externallyCausedValues;}
    void addTransition(int a, int b, int operation_id);
    void externallyRequired(int val);
    void externallyCaused(int val);
    bool isStronglyConnected(std::list<int> targetValues);
    void DFS(int value, std::vector<bool> *visited);
    freeDTG getTranspose();
    bool isReachable(int value, std::list<int> targetValues);
    void printFreeDTG(std::shared_ptr<AbstractTask> original_task);
    void printExternalInformation(std::shared_ptr<AbstractTask> original_task);
};

#endif //FREE_DOMAIN_TRANSITION_GRAPH_H
