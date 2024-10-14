#include "refiner.h"
#include "../task_proxy.h"

void refiner::refine_plan(Plan plan, vector<shared_ptr<AbstractTask>> abstraction_hirarchy)
{
	int i = 0;
	std::reverse(abstraction_hirarchy.begin(), abstraction_hirarchy.end());

    for (auto task : abstraction_hirarchy)
    {
        TaskProxy task_proxy(*task);
        cout << "refining step: " << i << endl;
        i++;
        refiner::refine_step(plan, task_proxy);
    }
}

void refiner::refine_step(Plan plan, TaskProxy task_proxy)
{
    auto operations = task_proxy.get_operators();
    auto variables = task_proxy.get_variables();
    State initial_state = task_proxy.get_initial_state();
    std::vector<FactPair> state;

    for (auto fact : initial_state)
    {
        state.push_back(fact.get_pair());
    }

    for (OperatorID opID : plan)
    {
    	cout << "  Current State: " << endl;
        for (auto fact : state)
       	{
       	    auto var_name = initial_state[fact.var].get_variable().get_name();
       		cout << "    " << var_name << " = " << fact.value << endl;
        }

        //cout << "opID: " << opID.get_index() << endl;
        auto op = operations[opID];
        for (auto precon : op.get_preconditions())
        {
        	auto var = precon.get_variable();
        	auto val = precon.get_value();

        	if (state[var.get_id()].value == val)
        	{
        		cout << "  Precon " << var.get_name() << " = " << val << " is okay" << endl;
        		continue;
        	}
        	else
        	{
        		cout << "  Precon " << var.get_name() << " = " << val << " is NOT okay" << endl;

        		//Add missing operations here
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
}
