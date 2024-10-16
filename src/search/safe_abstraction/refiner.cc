#include "refiner.h"
#include "../task_proxy.h"

Plan refiner::refine_plan(Plan plan, vector<abstractor> &abstraction_hirarchy)
{
	int i = 0;
	std::reverse(abstraction_hirarchy.begin(), abstraction_hirarchy.end());

    for (auto step : abstraction_hirarchy)
    {
        cout << "refining step: " << i << endl;
        i++;
        refiner::refine_step(plan, step);
    }
    cout << "Refined Plan Length: " << plan.size() << endl;
    return plan;
}

void refiner::refine_step(Plan &plan, abstractor &abstractor)
{
    //cout << "=== Refining" << endl;
    TaskProxy task_proxy = abstractor.getTaskProxy();

    auto operations = task_proxy.get_operators();
    auto variables = task_proxy.get_variables();
    State initial_state = task_proxy.get_initial_state();
    auto goals = task_proxy.get_goals();
    std::vector<FactPair> state;

    for (auto fact : initial_state)
    {
        state.push_back(fact.get_pair());
    }

    for (int i = 0; i < plan.size(); i++)
    {
      OperatorID opID = plan[i];

      /*
    	cout << "  Current State: " << endl;
        for (auto fact : state)
       	{
       	    auto var_name = initial_state[fact.var].get_variable().get_name();
       		cout << "    " << var_name << " = " << fact.value << endl;
        }

       */

        //cout << "opID: " << opID.get_index() << endl;
        auto op = operations[opID];
        for (auto precon : op.get_preconditions())
        {
        	auto var = precon.get_variable();
        	auto val = precon.get_value();

        	if (state[var.get_id()].value == val)
        	{
        		//cout << "  Precon " << var.get_name() << " = " << val << " is okay" << endl;
        		continue;
        	}
        	else
        	{
        		//cout << "  Precon " << var.get_name() << " = " << val << " is NOT okay" << endl;
                //Add missing operations here
                refiner::insertMissingOperations(plan, abstractor, i, var.get_id(), state[var.get_id()].value, val);
                refiner::refine_step(plan, abstractor);
                return;
        	}
        }

        for (auto effect : op.get_effects())
        {
			auto prostcon= effect.get_fact();
			auto var = prostcon.get_variable();
        	auto val = prostcon.get_value();

			state[var.get_id()].value = val;
        }
    }
    for (auto goal : goals)
    {
        int goalVar = goal.get_variable().get_id();
        int goalVal = goal.get_value();
        if (state[goalVar].value == goalVal){
            continue;}
        else
        {
            refiner::insertMissingOperations(plan, abstractor, plan.size()-1, goalVar, state[goalVar].value, goalVar);
            refiner::refine_step(plan, abstractor);
            return;
        }
    }
}

void refiner::insertMissingOperations(Plan &plan, abstractor &abstractor,int insertionIndex , int varID, int startVal, int endVal)
{
    freeDTG freeDTG = *abstractor.find_freeDTG_by_variable(varID);
    std::vector<int> newOperations = freeDTG.getPath(startVal, endVal);
    //cout << "Length of path: " << newOperations.size() << endl;
    std::reverse(newOperations.begin(), newOperations.end());
    for (int opID : newOperations)
    {
        //cout << "    " << " inserting operation: " << opID << endl;
        plan.insert(plan.begin()+insertionIndex, OperatorID(opID));
    }
}
