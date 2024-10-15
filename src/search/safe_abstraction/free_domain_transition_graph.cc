#include "free_domain_transition_graph.h"
#include "../abstract_task.h"
#include <list>
#include <vector>
#include <queue>
#include <iostream>

freeDTG::freeDTG(int var, int numberOfValues)
{
    variable = var;
    numVal = numberOfValues;
    this->transitions = new std::list<Transition>[numVal];
    this->externallyRequiredValues = std::vector<bool>(numVal);
    this->externallyCausedValues = std::vector<bool>(numVal);
}

void freeDTG::addTransition(int a, int b, int operation_id)
{
    transitions[a].emplace_back(b, operation_id); // Add b to a’s list. (Transtion from a to b)
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

    for (Transition transition : transitions[v])
    {
   		//Don't deepen if value was already visited (avoids getting stuck in cycles).
        if (!(*visited)[transition.destination])
        {
            DFS(transition.destination, visited);
        }
    }
}

//Inverts the DTG such that transision a->b becomes a<-b.
freeDTG freeDTG::getTranspose()
{
    freeDTG inversion(variable, numVal);
    for (int v = 0; v < numVal; v++)
    {
        for (Transition w : transitions[v])
        {
            inversion.addTransition(w.destination, v, w.operation_id);
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

struct PathNode {
    int value;
    std::vector<int> operation_ids;

    PathNode(int value, std::vector<int> ops)
        : value(value), operation_ids(std::move(ops)) {}
};

std::vector<int> freeDTG::getPath(int sourceVal, int destinationVal) {
    if (sourceVal == destinationVal) return {};

    std::queue<PathNode> queue;
    std::unordered_map<int, bool> visited;

    queue.emplace(sourceVal, std::vector<int>{});
    visited[sourceVal] = true;

    while (!queue.empty()) {
        PathNode current = queue.front();
        queue.pop();

        // Explore all transitions from the current node
        for (Transition& transition : transitions[current.value]) {
            if (visited[transition.destination]) continue; // Skip if visited

            // Create a new path with the current operation ID added
            std::vector<int> new_path = current.operation_ids;
            new_path.push_back(transition.operation_id);

            // If we reached the destination, return the path
            if (transition.destination == destinationVal) {
                return new_path;
            }

            queue.emplace(transition.destination, new_path);
            visited[transition.destination] = true;
        }
    }

    // If no path is found, return an empty vector
    return {};
}

void freeDTG::printFreeDTG(std::shared_ptr<AbstractTask> original_task)
{
    std::cout << "safe_abstraction > FreeDTG of variable: " << original_task->get_variable_name(variable) << std::endl;
    for (int i = 0; i < numVal; ++i) {
        std::cout << "  Transitions for value " << i << ": [ ";
        for (Transition transition : transitions[i]) {
            std::cout << transition.destination << " ";
        }
        std::cout << "]" << std::endl;
    }
    std::cout << std::endl;
}

void freeDTG::printExternalInformation(std::shared_ptr<AbstractTask> original_task)
{
    std::cout << "safe_abstraction > External information of variable: " << original_task->get_variable_name(variable) << std::endl;
    for (int i = 0; i < numVal; ++i) {
        std::cout << "Value " << i << " is:" << std::endl;
        if (externallyRequiredValues[i]) {std::cout << "    Externally required" << std::endl;}
        else {std::cout << "    NOT externally required" << std::endl;}
        if (externallyCausedValues[i]) {std::cout << "    Externally caused" << std::endl;}
        else {std::cout << "    NOT externally caused" << std::endl;}
    }
    std::cout << std::endl;
}