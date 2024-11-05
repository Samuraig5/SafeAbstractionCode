#include "simplified_task.h"

#include <algorithm>

namespace tasks {
SimplifiedTask::SimplifiedTask(const shared_ptr<RootTask> parent, compositor compositor) : RootTask(*parent)
{
  cout << "> Simplifing Task (Composing Operators)" << endl;

  std::set<int> compositedOperators = compositor.compositedOperatorIDs;
  std::vector<std::vector<OperatorProxy>> compositeOperators = compositor.compositeOperators;

  //Clear the compositedOperators
  for (auto op : compositedOperators)
  {
      operators[op].preconditions.clear();
      operators[op].effects.clear();
      //std::cout << "Removed Operator " << operators[op].name << endl;
  }

  for (auto CompOp : compositeOperators)
  {
      std::map<int, int> state;

      vector<FactPair> preconditions;
      vector<ExplicitEffect> effects;
      int cost;
      string name = "[CO:";
      bool is_an_axiom = false;

      for (auto op : CompOp)
      {
          for (auto precon : op.get_preconditions())
          {
              FactPair fact(precon.get_pair().var, precon.get_pair().value);
              if (state.count(fact.var) > 0) { //Add precondition only if it isn't already covered by a previous operations
                  preconditions.push_back(fact);
              }
          }

          for (auto postcon : op.get_effects())
          {
              auto fact = postcon.get_fact().get_pair();
              ExplicitEffect effect(fact.var, fact.value, vector<FactPair>(preconditions));
              state[fact.var] = fact.value;
          }
          cost += op.get_cost();
          name += op.get_name() + " > ";
      }

      for (auto postcon : state)
      {
          ExplicitEffect effect(postcon.first, postcon.second, vector<FactPair>());
          effects.push_back(effect);
      }

      name += "]";

      ExplicitOperator compositeOP = ExplicitOperator(preconditions, effects, cost, name, is_an_axiom);
      operators.push_back(compositeOP);
      //std::cout << name << endl;
      //print_operators();
  }
}


SimplifiedTask::SimplifiedTask(const shared_ptr<RootTask> parent, std::list<int> safeVariables) : RootTask(*parent) {
  if (safeVariables.empty()) {return;}
    /*
        Remo: Create the simplified, i.e. the safely abstracted, task here by
        modifying the vectors listed below (inherited from RootTask):
        vector<ExplicitVariable> variables;
        vector<vector<set<FactPair>>> mutexes;
        vector<ExplicitOperator> operators;
        vector<ExplicitOperator> axioms;
        vector<int> initial_state_values;
        vector<FactPair> goals;
    */

  	cout << "> Simplifing Task (Abstracting Varialbes)" << endl;


    //print_problem()
    //print_variables();
    //print_mutexes();
    //print_operators();

    //axioms.clear();
    simplifyOperators(safeVariables);
    removeGoals(safeVariables);
    resizeVariableIDs(safeVariables);
    removeVariables(safeVariables);

    //print_variables();
    //print_mutexes();
    //print_operators();
}

void SimplifiedTask::removeVariables(std::list<int> safeVarID)
{
    cout << "Removing Safe Variables..." << endl;
    std::list<string> safeVariables;
    for (int safeVarID : safeVarID) {
        safeVariables.push_back(get_variable(safeVarID).name);

    }

    for (string safe_var : safeVariables)
    {
        //This is very inefficient but .erase resizes the vector so I'm not sure how to do this better yet
        for (int i = 0; i < (int)variables.size(); i++)
        {
            if (safe_var == variables[i].name)
            {
                //cout << "Removing variable: " << safe_var << std::endl;
                variables.erase(variables.begin() + i);
                break;
            }
        }
    }
}

void SimplifiedTask::simplifyOperators(std::list<int> safeVarID)
{
    cout << "Simplifing Operators..." << endl;
    std::list<string> safeOperators;
    for (int i = 0; i < (int)operators.size(); i++)
    {
        ExplicitOperator op = operators[i];
        bool free_operation = true;

        for (const FactPair& precon : op.preconditions){
            for (int var : safeVarID){
                if (var != precon.var){
                    free_operation = false;
                }
            }
        }
        for (const ExplicitEffect& postcon : op.effects){
            for (int var : safeVarID){
                if (var != postcon.fact.var){
                    free_operation = false;
                }
            }
        }
        if (free_operation)
        {
            safeOperators.push_back(op.name);
        }
    }

    //Remove the operators that are safe
    for (string safe_op : safeOperators)
    {
        //This is very inefficient but .erase resizes the vector so I'm not sure how to do this better yet
        for (int i = 0; i < (int)operators.size(); i++)
        {
            if (safe_op == operators[i].name)
            {
                //cout << "Removing operation: " << safe_op << std::endl;
                //operators.erase(operators.begin() + i);
                operators[i].preconditions.clear();
                operators[i].effects.clear();
                break;
            }
        }
    }

    //Remove the preconditions / postconditions using the abstracted variables
    for (int i = 0; i < (int)operators.size(); i++)
    {
        bool all_is_clean = true;
        do //It is horrible to brute force it like this. I'll fix it later (hopefully)
        {
            all_is_clean = true;
            for (int var : safeVarID){
                for (int j = 0; j < operators[i].preconditions.size(); j++) {
                    if (var == operators[i].preconditions[j].var){
                        //cout << operators[i].name << ": Removing precondition: " << operators[i].preconditions[j].var << " = " << operators[i].preconditions[j].value << std::endl;

                        all_is_clean = false;
                        operators[i].preconditions.erase(operators[i].preconditions.begin() + j);
                        break;
                    }
                }
            }
            for (int var : safeVarID){
                for (int j = 0; j < operators[i].effects.size(); j++){ //(const ExplicitEffect& postcon : op.effects){
                    if (var == operators[i].effects[j].fact.var){
                        //cout << operators[i].name << ": Removing postcondition: " << operators[i].effects[j].fact.var << " = " << operators[i].effects[j].fact.value << std::endl;

                        all_is_clean = false;
                        operators[i].effects.erase(operators[i].effects.begin() + j);
                        break;
                    }
                }
            }
        }
        while (!all_is_clean);
    }
}

void SimplifiedTask::removeGoals(std::list<int> safeVarID)
{
    cout << "Removing Goals for safe variables..." << endl;
    std::list<FactPair> safeGoals;
    for (int safeVarID : safeVarID) {
        for (FactPair goal : goals) {
            if (goal.var == safeVarID) {safeGoals.push_back(goal);}
        }
    }

    for (FactPair goal : safeGoals)
    {
        auto index = std::find(goals.begin(), goals.end(), goal);

        if (index != goals.end()) {
            goals.erase(index);
            //cout << "Removing goal: " << goal.var << " = " << goal.value << std::endl;
        }
    }
}

void SimplifiedTask::resizeVariableIDs(std::list<int> safeVarID)
{
    cout << "Adjusting VariableIDs..." << endl;
    int offset = 0;
    safeVarID.sort();
    //cout << "Rezining these variable ids: ";
    //for (int target : safeVarID)
    //{
    //  cout << target << ", ";
    //}
    //cout << endl;
    for (int target : safeVarID) {
        target = target - offset;//This "moves" safeVarIDs as the list is shifted
        //cout << "Adjusting: " << target << endl;

        //Starting from the next higher variable (If we remove variable 3, we want to reduce all variables 4,5,6,... by one)
        for (int i = target+1; i < (int)variables.size()-offset; i++) {
            variableAdjuster::adjustOperators(i, operators);
            variableAdjuster::adjustInitialValues(i,  initial_state_values);
            variableAdjuster::adjustGoals(i,  goals);
            variableAdjuster::adjustMutexIndex(i,  mutexes);
            variableAdjuster::adjustMutexContent(i,  mutexes);
        }

        initial_state_values.pop_back(); //Removing last (now unused) slot
        mutexes.pop_back();
        offset++;
    }
}

void SimplifiedTask::print_variables()
{
    cout << "Variables: ";
    for (auto variable : variables)
    {
        cout << variable.name << ", ";
    }
    cout << std::endl;
}

void SimplifiedTask::print_mutexes()
{
    std::cout << "> MUTEXES" << std::endl;
    for (int i = 0; i < (int)mutexes.size(); i++)
    {
        std::cout << get_variable_name(i) << std::endl;
        for (int j = 0; j < (int)mutexes[i].size(); j++)
        {
            std::cout << "|    " << "= " << j << std::endl;
            for (const FactPair& fact : mutexes[i][j]) {
                std::cout << "|    | (" << get_variable_name(fact.var) << " = " << fact.value << ")" << std::endl;
            }
        }
    }
}

void SimplifiedTask::print_operators(bool detailed)
{
    std::cout << "> OPERATORS" << std::endl;
    for (const ExplicitOperator& op : operators) {
        std::cout << op.name << std::endl << "    precons: ";
        for (const FactPair& precon : op.preconditions)
        {
        	auto variable = precon.var;
        	if (detailed) {std::cout << get_variable_name(variable) << " = " << get_variable(variable).fact_names[precon.value] << ", ";}
        	else {std::cout << get_variable_name(variable) << " = " << precon.value << ", ";}
        }
        std::cout << std::endl;

        std::cout << "    postcon: ";
        for (const ExplicitEffect& postcon : op.effects)
        {
            auto variable = postcon.fact.var;
            if (detailed) {std::cout << get_variable_name(variable) << " = " << get_variable(variable).fact_names[postcon.fact.value] << ", ";}
            else {std::cout << get_variable_name(variable) << " = " << postcon.fact.value << ", ";}
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void SimplifiedTask::print_problem()
{
    cout << "Variables: ";
    for (auto variable : variables)
    {
        cout << "  " << variable.name << std::endl;
        for (auto fact : variable.fact_names)
        {
            cout << "    " << fact << std::endl;
        }
    }
    cout << std::endl;
}

}
