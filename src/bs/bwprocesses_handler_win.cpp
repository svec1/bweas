//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#include <bwprocesses_handler.hpp>

#if defined(WIN)

static logger _log{"BWPROCESS[WIN]"};

size_t processes_handler::wait_process(size_t pid) {
    if (!pids_win.size() ||
        (pid && std::find_if(pids_win.begin(), pids_win.end(),
                             [pid](const PROCESS_INFORMATION &pi) { return pi.dwProcessId == pid; }) == pids_win.end()))
        return SIZE_MAX;

    PROCESS_INFORMATION pid_tmp;

    if (pid) {
        const auto &it_pid = std::find_if(pids_win.begin(), pids_win.end(),
                                          [pid](const PROCESS_INFORMATION &pi) { return pi.dwProcessId == pid; });

        pid_tmp = *it_pid;
        pids_win.erase(it_pid);
    }
    else {
        pid_tmp = pids_win[0];

        pid = pids_win[0].dwProcessId;
        pids_win.erase(pids_win.begin());
    }

    WaitForSingleObject(pid_tmp.hProcess, INFINITE);

    DWORD returned = 0;
    GetExitCodeProcess(pid_tmp.hProcess, &returned);

    std::find_if(cmd_s.begin(), cmd_s.end(), [pid](const generator_api::command &cmd) {
        return cmd.pid_execute_process == pid;
    })->success = !returned;

    CloseHandle(pid_tmp.hProcess);
    CloseHandle(pid_tmp.hThread);

    return pid;
}

void processes_handler::create_process(generator_api::command &cmd) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    string str_args;
    for (const auto &arg : cmd.args)
        str_args += arg + " ";

    if (!CreateProcess(NULL, ("\"" + cmd.name_program + "\" " + str_args).data(), NULL, NULL, FALSE, 0, NULL, NULL, &si,
                       &pi)) {
        if (GetLastError() == 2)
            _log << bwtools::fatal << (log_message(log_type::fatal) << "No such file exists: " << cmd.name_program);
        else
            _log << bwtools::fatal << (log_message(log_type::fatal) << "Process cannot be created: " << GetLastError());
    }

    pids_win.push_back(pi);
    cmd.pid_execute_process = pi.dwProcessId;
}

#endif
