#include "free_domain_transition_graph.h"
#include <list>

freeDTG::freeDTG(int var, int numberOfValues)
{
    variable = var;
    numVal = numberOfValues;
    this->transitions = new std::list<int>[numVal];
}

void freeDTG::addTransition(int a, int b)
{
    transitions[a].push_back(b); // Add b to a’s list. (Transtion from a to b)
}