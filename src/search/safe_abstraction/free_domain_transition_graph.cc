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

Checks if the targetValues are strongly connected within the freeDTG.
Note that they may travel via non-target values to reach eachother.
 */
bool freeDTG::isStronglyConnected(std::list<int> targetValues)
{
	//Trivial case. Not sure if true or false should be returned
    if (targetValues.size() == 0) {return true;}

    //Check if all targetValues are reachable from the first target value v
    if (!freeDTG::isReachable(targetValues.front(), targetValues)){return false;}

	//Invert the freeDTG a->b => a<-b
    freeDTG inversion = freeDTG::getTranspose();

    /*
    Check if all targetValues are reachable from the first target value v in the inversion.
    If this is the case, we know that in the original (uninverted) free DTG, all targetValues can reach v.

    This together with the knowledge that v can reach all target values means that all target values are
    reachable by any target value via v.
     */
    if (!inversion.isReachable(targetValues.front(), targetValues)){return false;}

    return true;
}

//Simple recursive DFS
void freeDTG::DFS(int v, std::vector<bool> *visited)
{
    (*visited)[v] = true;

    for (int value : transitions[v])
    {
   		//Don't deepen if value was already visited (avoids getting stuck in cycles).
        if (!(*visited)[value])
        {
            DFS(value, visited);
        }
    }
}

//Inverts the DTG such that transision a->b becomes a<-b.
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

//Check if targetValues are reachable by startingValue in the DTG
bool freeDTG::isReachable(int startingValue, std::list<int> targetValues)
{
    if (targetValues.size() == 0) {return true;}
    std::vector<bool> visited(numVal, false);

    freeDTG::DFS(startingValue, &visited); //Perform DFS

    for (int i = 0; i < visited.size(); i++)
    {
        if (visited[i] == false) //If a value wasn't visited its not reachable from the starting value
        {
            for (int target : targetValues)
            {
                if (target == i) //If a target value isn't reachable we retun false.
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