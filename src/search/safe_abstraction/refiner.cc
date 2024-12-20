#include "refiner.h"
#include "../task_proxy.h"

Plan refiner::refine_plan(Plan plan, vector<pair<abstractor, compositor>> &abstraction_hirarchy)
{
    std::cout << "=============================== REFINEMNET =============================" << std::endl;

    cout << "> Simplified Plan Length: " << plan.size() << endl;

	int i = 0;
	std::reverse(abstraction_hirarchy.begin(), abstraction_hirarchy.end());

    if (abstraction_hirarchy.size() > 0) { printPlan(plan, abstraction_hirarchy[0].first.getTaskProxy()); }

    for (auto step : abstraction_hirarchy)
    {
        cout << "> Refining Step: " << i << endl;
        i++;

        cout << "Decomposing Operators..." << endl;
        refiner::decompose_step(plan, step.second);

        cout << "Inserting missing Operators..." << endl;
        refiner::refine_step(plan, step.first);

        cout << "Intermediate Plan Length: " << plan.size() << endl;
        printPlan(plan, step.first.getTaskProxy());

    }
    cout << endl;
    cout << "Took " << i << " steps to refine plan" << endl;
    cout << "Refined Plan Length: " << plan.size() << endl;
    std::cout << "========================================================================" << std::endl;
    return plan;
}

void refiner::decompose_step(Plan &plan, compositor &compositor)
{
    std::map<int, std::vector<OperatorProxy>> decompositOperations = compositor.decompositOperations;

    if (decompositOperations.empty()) {
        std::cout << "Nothing to decompose" << std::endl;
        return;
    }

    for (int i = 0; i < plan.size(); i++)
    {
        int opID = plan[i].get_index();
        if (decompositOperations.count(opID) > 0)
        {
        	decomposeCompositeOperator(plan, compositor, i);
            decompose_step(plan, compositor);
            return;
        }
    }
}

void refiner::decomposeCompositeOperator(Plan &plan, compositor &compositor, int insertionIndex)
{
    int opID = plan[insertionIndex].get_index();
    int offset = insertionIndex;

    //cout << "Decomposing Operator " << opID << " at position " << insertionIndex << endl;

	plan.erase(plan.begin() + insertionIndex);
	for (auto operatorProxy : compositor.decompositOperations[opID])
	{
        //cout << "Inserting Operator " << operatorProxy.get_name() << " at position " << offset << endl;
    	plan.insert(plan.begin()+offset, OperatorID(operatorProxy.get_id()));
    	offset++;
	}
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
    //cout << "Intermediate task has " << goals.size() << " goals" << endl;
    for (auto goal : goals)
    {
        int goalVar = goal.get_variable().get_id();
        int goalVal = goal.get_value();
        if (state[goalVar].value == goalVal){
            continue;}
        else
        {
            //cout << goal.get_variable().get_name() << " is not in its goal value " << goal.get_value() << ". Instead it has value: " << state[goalVar].value << endl;
            int index;
            if (!plan.empty())
            {
                index = plan.size();
            }
            else
            {
                index = 0;
            }
            refiner::insertMissingOperations(plan, abstractor, index, goalVar, state[goalVar].value, goalVal);
            refiner::refine_step(plan, abstractor);
            return;
        }
    }
}

void refiner::insertMissingOperations(Plan &plan, abstractor &abstractor, int insertionIndex , int varID, int startVal, int endVal)
{
    //cout << "Inserting new opertaion at position " << insertionIndex << endl;
    freeDTG freeDTG = *abstractor.find_freeDTG_by_variable(varID);
    //cout << "  Searching for path" << endl;
    std::vector<int> newOperations = freeDTG.getPath(startVal, endVal);
    //cout << "  Found Length of path: " << newOperations.size() << endl;
    std::reverse(newOperations.begin(), newOperations.end());
    for (int opID : newOperations)
    {
        auto op = abstractor.getTaskProxy().get_operators()[opID];
        cout << "  Inserting operation: " << "(" << op.get_id() << ") " << op.get_name() << endl;
        /*
        cout << "    " << "Precon: ";
        for (auto precondition : op.get_preconditions())
        {
            auto precon_var = precondition.get_variable();
            int precon_val = precondition.get_value();
            cout << precon_var.get_name() << " = " << precon_val << ", ";
        }
        cout << endl << "    " << "Postcon: ";
        for (auto postcondition : op.get_effects())
        {
            auto postcondition_fact = postcondition.get_fact();
            auto postcon_var = postcondition_fact.get_variable();
            int postcon_val = postcondition_fact.get_value();
            cout << postcon_var.get_name() << " = " << postcon_val << ", ";
        }
        cout << endl << endl;
         */
        plan.insert(plan.begin()+insertionIndex, OperatorID(opID));
    }
}

void refiner::printPlan(Plan &plan, TaskProxy task_proxy)
{
    auto operators = task_proxy.get_operators();
    if (plan.empty()) { cout << "Empty Plan" << endl; return; }
    cout << "Current Plan:" << endl;
    for (auto step : plan)
    {
        auto op = operators[step];
        cout << "    " << op.get_name() << endl;

        /*
        cout << "        " << "Precon: ";
        for (auto precon : op.get_preconditions())
        {
            cout << precon.get_variable().get_name() << " = " << precon.get_value() << ", ";
        }
        cout << endl;

        cout << "        " << "Postcon: ";
        for (auto postcon : op.get_effects())
        {
            cout << postcon.get_fact().get_variable().get_name() << " = " << postcon.get_fact().get_value() << ", ";
        }
        cout << endl;
         */
    }
}
