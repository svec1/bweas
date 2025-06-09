#if defined(WIN)

#include <bwprocesses_handler.hpp>

static logger _log{"BWPROCESS[WIN]"};

size_t processes_handler::wait_process(size_t pid) {
    if (!pids_win.size() ||
        (pid && std::find_if(pids_win.begin(), pids_win.end(), [process_pid](const PROCESS_INFORMATION &pi) {
                    return pid.hProcess == pid;
                }) == pids_win.end()))
        return SIZE_MAX;

    if (pid) {
        WaitForSingleObject(pid, INFINITE);

        const auto &it_pid = std::find_if(pids_win.begin(), pids_win.end(),
                                          [process_pid](const PROCESS_INFORMATION &pi) { return pid.hProcess == pid; });

        CloseHandle(pid);
        CloseHandle(it_pid->hThread);

        pids_win.erase(it_pid);
    }
    else {
        WaitForSingleObject(pids_win[0].hThread, INFINITE);

        CloseHandle(pids_win[0].hProcess);
        CloseHandle(pids_win[0].hThread);

        pid = pids_win[0].hProcess;
        pids_win.erase(pids_win.begin());
    }

    _log << (log_message(log_type::msg) << "Process closed(" << pid << ")");

    return pid;
}

void processes_handler::create_process(generator_api::command &cmd) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    pids_win.push_back(pi);

    cmd.args.emplace(cmd.args.begin(), cmd.name_program);

    char **args = new char *[cmd.args.size() + 1];
    for (size_t i = 0; i < cmd.args.size(); ++i)
        args[i] = cmd.args[i].data();

    args[cmd.args.size()] = NULL;

    if (!CreateProcess(cmd.name_program.c_str(), args, NULL, NULL, FALSE, 0, NULL, 1, &si,
                       &pids_win[pids_win.size() - 1]))
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Process cannot be created: " << GetLastError());

    cmd.pid_execute_process = pi.hProcess;
}

#endif
