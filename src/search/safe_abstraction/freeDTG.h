#ifndef FREEDTG_H
#define FREEDTG_H

class freeDTG
{
  int variable;
  int numVal; // Number of values
  list<int> *transitions; // IntRelation transitions (as described in causal_graph.h)

  public:
    freeDTG(int var, int numVal);
    void addTransition(int a, int b);
};

#endif //FREEDTG_H
