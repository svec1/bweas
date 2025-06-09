#ifndef BWPROCESSES_HANDLER_HPP
#define BWPROCESSES_HANDLER_HPP

#include <bw_defs.hpp>

#include <bwgenerator_api.hpp>

namespace bweas {
class processes_handler;
class process;
} // namespace bweas

class bweas::processes_handler {
  public:
    processes_handler(generator_api::commands &_cmd_s, size_t _max_processes)
        : cmd_s(_cmd_s), max_processes(_max_processes > cmd_s.size() ? cmd_s.size() : _max_processes) {
    }

  public:
    void start(std::function<void(const generator_api::command &cmd)> do_more_func = NULL);
    size_t wait_process(size_t pid = 0);

  private:
    void create_process(generator_api::command &cmd);

  private:
    generator_api::commands &cmd_s;

    size_t max_processes;

#if defined(WIN)
    vec<PROCESS_INFORMATION> pids_win;
#endif
};

#endif
