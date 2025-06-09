#include <bwprocesses_handler.hpp>

using namespace bweas;

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
        create_process(cmd_s[i]);
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

    if (pid + 1) {
        if (WIFEXITED(pstatus))
            _log << (log_message(log_type::msg) << "Process closed(" << pid << "): returned " << WEXITSTATUS(pstatus));
        else
            _log << (log_message(log_type::msg) << "Process aborted(" << pid << ")");

        std::find_if(cmd_s.begin(), cmd_s.end(), [pid](const generator_api::command &cmd) {
            return cmd.pid_execute_process == pid;
        })->success = !WEXITSTATUS(pstatus);
    }

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
        exit(0);
    }
    cmd.pid_execute_process = pid;

    _log << (log_message(log_type::msg) << "Process created(" << pid << ")");
}

#endif
