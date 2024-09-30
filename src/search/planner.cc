#include "command_line.h"
#include "search_algorithm.h"

#include "tasks/root_task.h"
#include "tasks/simplified_task.h"
#include "task_utils/task_properties.h"
#include "utils/logging.h"
#include "utils/system.h"
#include "utils/timer.h"
#include "safe_abstraction/abstractor.h"

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
    if (static_cast<string>(argv[1]) != "--help") {
        utils::g_log << "reading input..." << endl;
        tasks::read_root_task(cin);
        utils::g_log << "done reading input!" << endl;
        TaskProxy task_proxy(*tasks::g_root_task);
        unit_cost = task_properties::is_unit_cost(task_proxy);

        bool foundSafeVariables = false;
        do
        {
            foundSafeVariables = false;
            original_task = tasks::g_root_task;
            /*
              Remo: Implement a procedure that takes this original task and returns
              the set of variables that can be safely abstracted away. This
              information is then past on to the constructor of SimplifiedTask
              below. The 'int' is just a placeholder for whatever data type we will
              need here in the end.
            */
            std::list<int> safe_variables = abstractor::find_safe_variables(original_task);
            if (!safe_variables.empty())
            {
                foundSafeVariables = true;
                cout << "> Found safe variable: ";
                for (int safe_variable : safe_variables) {cout << safe_variable << ", ";}
                cout << endl;
            }
            else {cout << "> No safe variables found!" << endl;}

            /*
              Remo: We need this cast because the constructor of SimplifiedTask
              needs a RootTask as input, not an AbstractTask.
            */
            shared_ptr<tasks::RootTask> original_root_task =
                dynamic_pointer_cast<tasks::RootTask>(original_task);
            shared_ptr<tasks::SimplifiedTask> simplified_task =
                make_shared<tasks::SimplifiedTask>(original_root_task, safe_variables);
            /*
              Remo: It seems that the parts of the code that need access to the task
              read it directly from the global g_root_task variable. We set it to
              the simplified task we just created so that all parts access it
              directly. A potentially cleaner solution would be to implement the
              simplification as a task transformation akin to cost_adapted_task.*
              for example, but I think we can run with this setup as long as it does
              not cause issues.
            */
            tasks::g_root_task = simplified_task;
        }
        while (foundSafeVariables);
    }

    shared_ptr<SearchAlgorithm> search_algorithm =
        parse_cmd_line(argc, argv, unit_cost);

    utils::Timer search_timer;
    search_algorithm->search();
    search_timer.stop();
    utils::g_timer.stop();

    /*
      Remo: Expand plan for the simplified task to a plan for the original task
      around here.
    */

    search_algorithm->save_plan_if_necessary();
    search_algorithm->print_statistics();
    utils::g_log << "Search time: " << search_timer << endl;
    utils::g_log << "Total time: " << utils::g_timer << endl;

    ExitCode exitcode = search_algorithm->found_solution()
        ? ExitCode::SUCCESS
        : ExitCode::SEARCH_UNSOLVED_INCOMPLETE;
    utils::report_exit_code_reentrant(exitcode);
    return static_cast<int>(exitcode);
}
