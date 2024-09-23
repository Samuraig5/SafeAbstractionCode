#include "free_domain_transition_graph.h"
#include <list>
#include <vector>
#include <iostream>

freeDTG::freeDTG(int var, int numberOfValues)
{
    variable = var;
    numVal = numberOfValues;
    this->transitions = new std::list<int>[numVal];
    this->externallyRequiredValues = std::vector<bool>(numVal);
    this->externallyCausedValues = std::vector<bool>(numVal);
}

int freeDTG::getVariable()
{
    return variable;
}

void freeDTG::addTransition(int a, int b)
{
    transitions[a].push_back(b); // Add b to a’s list. (Transtion from a to b)
}

void freeDTG::externallyRequired(int val)
{
    externallyRequiredValues[val] = true;
    std::cout << " (Setting: " << val << " of " << variable << " to " << externallyRequiredValues[val] << ") ";
}

void freeDTG::externallyCaused(int val)
{
    externallyCausedValues[val] = true;
    std::cout << " (Setting: " << val << " of " << variable << " to " << externallyCausedValues[val] << ") ";

}

void freeDTG::printFreeDTG()
{
    std::cout << "safe_abstraction > FreeDTG of variable: " << variable << std::endl;
    for (int i = 0; i < numVal; ++i) {
        std::cout << "  Transitions for value " << i << ": [ ";
        for (int value : transitions[i]) {
            std::cout << value << " ";
        }
        std::cout << "]" << std::endl;
    }
    std::cout << std::endl;
}

void freeDTG::printExternalInformation()
{
    std::cout << "safe_abstraction > External information of variable: " << variable << std::endl;
    for (int i = 0; i < numVal; ++i) {
        std::cout << "Value " << i << " is:" << std::endl;
        if (externallyRequiredValues[i]) {std::cout << "    Externally required" << std::endl;}
        else {std::cout << "    NOT externally required" << std::endl;}
        if (externallyCausedValues[i]) {std::cout << "    Externally caused" << std::endl;}
        else {std::cout << "    NOT externally caused" << std::endl;}
    }
    std::cout << std::endl;
}