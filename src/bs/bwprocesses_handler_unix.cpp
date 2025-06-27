//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#include <bwprocesses_handler.hpp>

using namespace bweas;

void processes_handler::start(std::function<void(const generator_api::command &cmd)> do_more_func) {
    for (size_t i = 0; i < cmd_s.size(); ++i) {
        if (count_runable_processes >= max_count_processes) {
            size_t tmp_pid = wait_process();

            const auto &completed_cmd =
                std::find_if(cmd_s.begin(), cmd_s.end(), [tmp_pid](const generator_api::command &cmd) {
                    return cmd.pid_execute_process == tmp_pid;
                });

            do_more_func(*completed_cmd);
            --count_runable_processes;
        }

        for (const auto &name_dependence : cmd_s[i].depends_command) {
            const auto &dependence_cmd =
                std::find_if(cmd_s.begin(), cmd_s.end(), [name_dependence](const generator_api::command &cmd) {
                    return cmd.name == name_dependence;
                });

            if (dependence_cmd == cmd_s.end() || std::find(completed_pid.begin(), completed_pid.end(),
                                                           dependence_cmd->pid_execute_process) != completed_pid.end())
                continue;

            (void)wait_process(dependence_cmd->pid_execute_process);

            do_more_func(*dependence_cmd);
            --count_runable_processes;
        }
        create_process(cmd_s[i]);
        ++count_runable_processes;
    }

    while (count_runable_processes) {
        size_t tmp_pid = wait_process();

        do_more_func(*std::find_if(cmd_s.begin(), cmd_s.end(), [tmp_pid](const generator_api::command &cmd) {
            return cmd.pid_execute_process == tmp_pid;
        }));
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

    completed_pid.insert(pid);
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
