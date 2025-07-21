//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#include <bwprocesses_handler.hpp>

using namespace bweas;

void processes_handler::start(std::function<void(string_v, bool)> do_more_func) {
    for (size_t i = 0; i < cmd_s.size(); ++i) {
        if (count_runable_processes >= max_count_processes) {
            size_t tmp_pid = wait_process();

            const auto &completed_cmd =
                std::find_if(cmd_s.begin(), cmd_s.end(), [tmp_pid](const generator_api::command &cmd) {
                    return cmd.pid_execute_process == tmp_pid;
                });

            do_more_func(completed_cmd->name_output_file, completed_cmd->success.value());
            --count_runable_processes;
        }

        for (const auto &name_dependence : cmd_s[i].depends_command) {
            const auto &dependence_cmd =
                std::find_if(cmd_s.begin(), cmd_s.end(), [name_dependence](const generator_api::command &cmd) {
                    return cmd.name == name_dependence;
                });

            if (dependence_cmd->success.has_value())
                continue;

            (void)wait_process(dependence_cmd->pid_execute_process);
            --count_runable_processes;

            do_more_func(dependence_cmd->name_output_file, dependence_cmd->success.value());
            if (!dependence_cmd->success.value())
                goto wait_all_process;
        }
        create_process(cmd_s[i]);
        ++count_runable_processes;
    }

wait_all_process:
    while (count_runable_processes) {
        const auto &it =
            *std::find_if(cmd_s.begin(), cmd_s.end(), [tmp_pid = wait_process()](const generator_api::command &cmd) {
                return cmd.pid_execute_process == tmp_pid;
            });

        do_more_func(it.name_output_file, it.success.value());
        --count_runable_processes;
    }
}

#if defined(UNIX)

static logger _log{"BWPROCESS[UNIX]"};

size_t processes_handler::wait_process(size_t pid) {
    int pstatus = 0;
    if (pid)
        waitpid(pid, &pstatus, 0);
    else
        pid = wait(&pstatus);

    if (pid + 1)
        std::find_if(cmd_s.begin(), cmd_s.end(), [pid](const generator_api::command &cmd) {
            return cmd.pid_execute_process == pid;
        })->success = !WEXITSTATUS(pstatus);

    return pid;
}

void processes_handler::create_process(generator_api::command &cmd) {
    pid_t pid;
    if ((pid = fork()) == -1)
        _log << bwtools::fatal
             << (log_message(log_type::fatal) << "Process cannot be created" << " [" << std::strerror(errno) << "]");
    else if (!pid) {
        cmd.args.emplace(cmd.args.begin(), cmd.name_program);

        char **args = new char *[cmd.args.size() + 1];
        for (size_t i = 0; i < cmd.args.size(); ++i)
            args[i] = cmd.args[i].data();

        args[cmd.args.size()] = NULL;

        if (execvp(cmd.name_program.c_str(), args))
            _log << bwtools::fatal
                 << (log_message(log_type::fatal)
                     << "Execution error: " << cmd.name_program << " [" << std::strerror(errno) << "]");
    }
    cmd.pid_execute_process = pid;

    _log << (log_message(log_type::msg) << "Process created(" << pid << ")");
}

#endif
