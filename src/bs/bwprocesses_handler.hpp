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

/** \brief The process handler. */
class bweas::processes_handler final {
  public:
    /** \brief Constructor.
     * \param [in] _cmd_s An array of commands to execute.
     * \param [in] _max_count_processes The maximum number of processes that can be started simultaneously.
     */
    processes_handler(bweas::commands &_cmd_s, size_t _max_count_processes)
        : cmd_s(_cmd_s),
          max_count_processes(_max_count_processes > cmd_s.size() ? cmd_s.size() : _max_count_processes + 1) {
    }

  public:
    /** \brief Executes commands.
     * \param [in] do_more_func The function that will be executed when one of the processes is completed.
     */
    void start(std::function<void(string_v name_output_file, bool success)> do_more_func = nullptr);

  protected:
    /** \brief Creates a process with a specific command.
     * \param [in] cmd The command that needs to be executed.
     */
    void create_process(bweas::command &cmd);

    /**\brief Waiting for the end of the process.
     * \param [in] pid The PID of the process to be expected(if 0, then any child process is expected to terminate).
     */
    size_t wait_process(size_t pid = 0);

  private:
    bweas::commands &cmd_s;
    size_t max_count_processes, count_runable_processes = 0;
};

#endif
