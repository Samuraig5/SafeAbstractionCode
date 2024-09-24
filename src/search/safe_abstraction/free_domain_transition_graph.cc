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

void freeDTG::addTransition(int a, int b)
{
    transitions[a].push_back(b); // Add b to a’s list. (Transtion from a to b)
}

void freeDTG::externallyRequired(int val)
{
    externallyRequiredValues[val] = true;
    //std::cout << " (Setting: " << val << " of " << variable << " to " << externallyRequiredValues[val] << ") ";
}

void freeDTG::externallyCaused(int val)
{
    externallyCausedValues[val] = true;
    //std::cout << " (Setting: " << val << " of " << variable << " to " << externallyCausedValues[val] << ") ";
}

/*
Based on https://www.geeksforgeeks.org/connectivity-in-a-directed-graph/
 */
bool freeDTG::isStronglyConnected(std::list<int> targetValues)
{
    if (targetValues.size() == 0) {return true;}
    std::vector<bool> visited(numVal, false);

    freeDTG::DFS(targetValues.front(), &visited);

    for (int i = 0; i < visited.size(); i++)
    {
        if (visited[i] == false)
        {
            for (int target : targetValues)
            {
                if (target == i)
                {
                    //If a target variable wasn't visited we already know its not strongly connected.
                    std::cout << "freeDTG is not strongly connected: couldn't reach: " << i << " from: " << targetValues.front() << std::endl;
                    return false;
                }
            }
        }
    }

    freeDTG inversion = freeDTG::getTranspose();

    std::vector<bool> inversionVisited(targetValues.size(), false);

    inversion.DFS(targetValues.front(), &inversionVisited);

    for (int i = 0; i < inversionVisited.size(); i++)
    {
        if (inversionVisited[i] == false)
        {
            for (int target : targetValues)
            {
                if (target == i)
                {
                    std::cout << "freeDTG is not strongly connected: (inverted) couldn't reach: " << i << " from: " << targetValues.front() << std::endl;
                    return false;
                }
            }
        }
    }

    return true;
}

void freeDTG::DFS(int v, std::vector<bool> *visited)
{
    (*visited)[v] = true;

    for (int value : transitions[v])
    {
        if (!(*visited)[value])
        {
            DFS(value, visited);
        }
    }
}

freeDTG freeDTG::getTranspose()
{
    freeDTG inversion(variable, numVal);
    for (int v = 0; v < numVal; v++)
    {
        for (int w : transitions[v])
        {
            inversion.addTransition(w, v);
        }
    }
    return inversion;
}

bool freeDTG::isReachable(int value, std::list<int> targetValues)
{
    if (targetValues.size() == 0) {return true;}
    std::vector<bool> visited(numVal, false);

    freeDTG::DFS(value, &visited);

    for (int i = 0; i < visited.size(); i++)
    {
        if (visited[i] == false)
        {
            for (int target : targetValues)
            {
                if (target == i)
                {
                    //std::cout << "couldn't reach: " << i << " from: " << targetValues.front() << std::endl;
                    return false;
                }
            }
        }
    }

    return true;
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