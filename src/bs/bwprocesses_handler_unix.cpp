#include <bwprocesses_handler.hpp>

#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace bweas;

static logger _log{"BWPROCESS"};

void processes_handler::start(std::function<void(const generator_api::command &cmd)> do_more_func) {
    for (size_t i = 0; i < cmd_s.size(); ++i) {
        if (i == max_processes)
            (void)wait_process();

        for (const auto &name_dependence : cmd_s[i].depends_command) {
            const auto &cmd_dependence =
                std::find_if(cmd_s.begin(), cmd_s.end(), [name_dependence](const generator_api::command &cmd) {
                    return cmd.name == name_dependence;
                });

            if (cmd_dependence == cmd_s.end())
                continue;

            (void)wait_process(cmd_dependence->pid_execute_process);
            do_more_func(*cmd_dependence);
        }
        process p(cmd_s[i]);
    }
}

size_t processes_handler::wait_process(size_t process_pid) {
    int pstatus = 0;
    if (process_pid)
        waitpid(process_pid, &pstatus, 0);
    else
        process_pid = wait(&pstatus);

    if (process_pid + 1) {
        if (WIFEXITED(pstatus))
            _log << (log_message(log_type::msg)
                     << "Process closed(" << process_pid << "): returned " << WEXITSTATUS(pstatus));
        else
            _log << (log_message(log_type::msg) << "Process aborted(" << process_pid << ")");

        std::find_if(cmd_s.begin(), cmd_s.end(), [process_pid](const generator_api::command &cmd) {
            return cmd.pid_execute_process == process_pid;
        })->success = !WEXITSTATUS(pstatus);
    }

    return process_pid;
}

process::process(generator_api::command &cmd) {
    pid_t process_pid;
    if ((process_pid = fork()) == -1)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Process cannot be created");
    else if (!process_pid) {
        cmd.args.emplace(cmd.args.begin(), cmd.name_program);

        char **args = new char *[cmd.args.size() + 1];
        for (size_t i = 0; i < cmd.args.size(); ++i)
            args[i] = cmd.args[i].data();

        args[cmd.args.size()] = NULL;

        if (execvp(cmd.name_program.c_str(), args))
            _log << bwtools::fatal
                 << (log_message(log_type::fatal)
                     << "Execution error: " << cmd.name_program << " [" << std::strerror(errno) << "]");
        exit(0);
    }
    cmd.pid_execute_process = process_pid;

    _log << (log_message(log_type::msg) << "Process created(" << process_pid << ")");
}
