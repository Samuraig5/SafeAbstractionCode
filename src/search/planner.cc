#include "command_line.h"
#include "search_algorithm.h"

#include "tasks/root_task.h"
#include "tasks/simplified_task.h"
#include "task_utils/task_properties.h"
#include "utils/logging.h"
#include "utils/system.h"
#include "utils/timer.h"
#include "safe_abstraction/compositor.h"
#include "safe_abstraction/abstractor.h"
#include "safe_abstraction/refiner.h"

#include <iostream>

using namespace std;
using utils::ExitCode;

int main(int argc, const char **argv) {
    utils::register_event_handlers();

    if (argc < 2) {
        utils::g_log << usage(argv[0]) << endl;
        utils::exit_with(ExitCode::SEARCH_INPUT_ERROR);
    }

    bool unit_cost = false;
    /*
      Remo: We'll need the original task below, when expanding the plan for the
      simplified task to a plan for the original task.
    */
    shared_ptr<AbstractTask> original_task;

    vector<pair<abstractor, compositor>> abstraction_hirarchy;

    if (static_cast<string>(argv[1]) != "--help") {
        utils::g_log << "reading input..." << endl;
        tasks::read_root_task(cin);
        utils::g_log << "done reading input!" << endl;
        TaskProxy task_proxy(*tasks::g_root_task);
        unit_cost = task_properties::is_unit_cost(task_proxy);
        original_task = tasks::g_root_task;

        std::cout << std::endl << "============================ SAFE ABSTRACTION ==========================" << std::endl;

        /*
        Remo: Assume that the first of the arguments passed to the search
        component (the one after '--search') encodes the info we need.
        */
        string myargstring = static_cast<string>(argv[2]);
        // TODO: Come up with some format for that string and parse it here.
		// --all - both
        // --abstraction - no composition
        // --composition - Irrelevant

        cout << "> Original Task has: " << task_proxy.get_variables().size() << " variables" << endl;
        bool doAbstraction = true;
        bool doComposition = true;
        // How often should we perform a composition without a new abstraction before giving up? (-1 means no limit)
        int numCompositionWithoutAbstraction = -1;

        bool continiueAbstraction = true;
        bool foundCompsitableOperators = false;
        bool foundSafeVariables = false;
        bool noNewAbstractionAfterComposition = false;

        int step = 0;
        int numSafeVariables = 0;
        int numOperatorsInOriginalTask = task_proxy.get_operators().size();
        int numCompositeOperators = 0;
        int numCompositionRemaining = numCompositionWithoutAbstraction;

        while (continiueAbstraction)
        {
        	cout << endl;
        	cout << "=> Step: " << step << endl;

            // = ABSTRACTOR =
            original_task = tasks::g_root_task;
            abstractor abstractor(original_task);
            std::list<int> safe_variables;
            if (doAbstraction) {safe_variables = abstractor.find_safe_variables();}

            if (!safe_variables.empty())
            {
                foundSafeVariables = true;
                foundCompsitableOperators = false;
                numCompositionRemaining = numCompositionWithoutAbstraction;
                cout << "Found safe variable: ";
                for (int safe_variable : safe_variables)
                {
                  cout << original_task->get_variable_name(safe_variable) << ", ";
                  numSafeVariables++;
                }
                cout << endl;
            }
            else
            {
                foundSafeVariables = false;
                cout << "No safe variables found!" << endl;
            }

            shared_ptr<tasks::RootTask> original_root_task = dynamic_pointer_cast<tasks::RootTask>(original_task);
            shared_ptr<tasks::SimplifiedTask> simplified_task = make_shared<tasks::SimplifiedTask>(original_root_task, safe_variables);
            tasks::g_root_task = simplified_task;

            // = COMPOSITOR ==
            original_task = tasks::g_root_task;
            if (numCompositionRemaining == 0)
            {
            	noNewAbstractionAfterComposition = true;
                doComposition = false;
            }
            compositor compositor(original_task, doComposition);
            if (!compositor.compositeOperators.empty())
            {
            	numCompositeOperators += compositor.compositeOperators.size();
                foundCompsitableOperators = true;
                foundSafeVariables = false;
                numCompositionRemaining--;
                //cout << "Remaining " << numCompositionRemaining << endl;
            }
            else
            {
                foundCompsitableOperators = false;
            }

            original_root_task = dynamic_pointer_cast<tasks::RootTask>(original_task);
            simplified_task = make_shared<tasks::SimplifiedTask>(original_root_task, compositor);
            tasks::g_root_task = simplified_task;

            if (!foundCompsitableOperators && !foundSafeVariables)
            {
            	std::cout << "=> Nothing was done in this step" << endl;
                std::cout << endl << "> Found no compositable operators nor any safe variables" << endl;
                continiueAbstraction = false;
            }
            else
            {
            	step++;
                abstraction_hirarchy.push_back(make_pair(abstractor, compositor));
            }
            task_proxy = TaskProxy(*tasks::g_root_task);

            if (task_proxy.get_variables().size() == 0)
            {
                std::cout << endl << "> Problem was fully solved by abstraction" << endl;
                continiueAbstraction = false;
            }
            else if (noNewAbstractionAfterComposition)
            {
                std::cout << endl << "> Found no new abstraction after " << numCompositionWithoutAbstraction << " compositions" << endl;
                continiueAbstraction = false;
            }
        }

        cout << endl;
        cout << "Abstraction took " << step << " steps" << endl;
        cout << "Abstracted " << numSafeVariables << " safe variables." << endl;
        cout << task_proxy.get_variables().size() << " variables remain." << endl;
        cout << "Created " << numCompositeOperators << " composite operators." << endl;
        cout << "Original task had: " << numOperatorsInOriginalTask << " operators" << endl;
        std::cout << "========================================================================" << std::endl;
    }
    TaskProxy task_proxy = TaskProxy(*tasks::g_root_task);
    cout << endl;
    /*
    Remo: Expand plan for the simplified task to a plan for the original task
    around here.
    */
    if (task_proxy.get_variables().size() > 0)
    {
      cout << "Running search algorithm" << endl;
      /*
      Remo : Create a new argv that removes the argument(s) we parsed above so
      that the `parse_cmd_line` function receives the argument list as it would
      look without our additional argument(s).
      */
      int my_argc = argc-1;
      const char** my_argv = new const char*[my_argc];
      my_argv[0] = argv[0]; // The first argument is always the name of the programm.
      my_argv[1] = argv[1]; // The second is '--search', which we need to keep.
      for (int i = 3; i < argc; ++i) {
          my_argv[i-1] = argv[i];
      }
      shared_ptr<SearchAlgorithm> search_algorithm = parse_cmd_line(my_argc, my_argv, unit_cost);
      utils::Timer search_timer;
      search_algorithm->search();
      search_timer.stop();
      utils::g_timer.stop();
      if (search_algorithm->found_solution())
      {
        cout << endl;
        Plan refinedPlan = refiner::refine_plan(search_algorithm->get_plan(), abstraction_hirarchy);
        search_algorithm->set_plan(refinedPlan);
      }
      cout << endl;
      search_algorithm->save_plan_if_necessary();
      search_algorithm->print_statistics();

      utils::g_log << "Search time: " << search_timer << endl;

      ExitCode exitcode = search_algorithm->found_solution()
      ? ExitCode::SUCCESS
      : ExitCode::SEARCH_UNSOLVED_INCOMPLETE;
      utils::report_exit_code_reentrant(exitcode);
      return static_cast<int>(exitcode);
    }
    else
    {
      cout << "Abstraction solved the problem." << endl;
      cout << "Skipping search algorithm." << endl;
      Plan emptyPlan;
      cout << endl;
      Plan refinedPlan = refiner::refine_plan(emptyPlan, abstraction_hirarchy);
      cout << endl;
      PlanManager plan_manager;
      plan_manager.save_plan(refinedPlan, task_proxy);
      utils::g_log << "Search time: 0.0s" << endl;
    }

    utils::g_log << "Total time: " << utils::g_timer << endl;
}
