#ifndef FREE_DOMAIN_TRANSITION_GRAPH_H
#define FREE_DOMAIN_TRANSITION_GRAPH_H
#include <list>

class freeDTG
{
  int variable;
  int numVal; // Number of values
  std::list<int> *transitions;

  public:
    freeDTG(int var, int numVal);
    void addTransition(int a, int b);
};
#endif //FREE_DOMAIN_TRANSITION_GRAPH_H
