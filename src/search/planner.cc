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

        cout << "> Original Task has: " << task_proxy.get_variables().size() << " variables" << endl;
        bool continiueAbstraction = true;
        bool foundCompsitableOperators = false;
        bool foundSafeVariables = false;
        int numSafeVariables = 0;
        int step = 0;
        do
        {
        	cout << endl;
        	cout << "=> Step: " << step << endl;
            step++;

            // = ABSTRACTOR =
            original_task = tasks::g_root_task;
            abstractor abstractor(original_task);
            std::list<int> safe_variables = abstractor.find_safe_variables();

            if (!safe_variables.empty())
            {
                foundSafeVariables = true;
                foundCompsitableOperators = false;
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
            compositor compositor(original_task);
            if (!compositor.compositeOperators.empty())
            {
                foundCompsitableOperators = true;
                foundSafeVariables = false;
            }

            original_root_task = dynamic_pointer_cast<tasks::RootTask>(original_task);
            simplified_task = make_shared<tasks::SimplifiedTask>(original_root_task, compositor);
            tasks::g_root_task = simplified_task;

            abstraction_hirarchy.push_back(make_pair(abstractor, compositor));

            task_proxy = TaskProxy(*tasks::g_root_task);
            if (task_proxy.get_variables().size() == 0)
            {
                std::cout << "> Problem was fully solved by abstraction" << endl;
                continiueAbstraction = false;
            }
            else if (!foundCompsitableOperators && !foundSafeVariables)
            {
                std::cout << "> Found no compositable operators nor any safe variables" << endl;
                continiueAbstraction = false;
            }
        }
        while (continiueAbstraction);

        cout << endl;
        cout << "Abstracted " << numSafeVariables << " safe variables." << endl;
        cout << task_proxy.get_variables().size() << " variables remain." << endl;
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
      shared_ptr<SearchAlgorithm> search_algorithm = parse_cmd_line(argc, argv, unit_cost);
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
