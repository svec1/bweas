//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#ifndef BWPROCESSES_HANDLER_HPP
#define BWPROCESSES_HANDLER_HPP

#include <bw_defs.hpp>

#include <bwgenerator_command.hpp>

namespace bweas {
class processes_handler;
}

class bweas::processes_handler {
  public:
    processes_handler(bweas::commands &_cmd_s, size_t _max_count_processes)
        : cmd_s(_cmd_s),
          max_count_processes(_max_count_processes > cmd_s.size() ? cmd_s.size() : _max_count_processes + 1) {
    }

  public:
    void start(std::function<void(string_v name_output_file, bool success)> do_more_func = nullptr);

  private:
    void create_process(bweas::command &cmd);
    size_t wait_process(size_t pid = 0);

  private:
    bweas::commands &cmd_s;

    size_t max_count_processes;

    // Platform-independent running process counter
    size_t count_runable_processes = 0;

#if defined(WIN)
    vec<PROCESS_INFORMATION> pids_win;
#endif
};

#endif
