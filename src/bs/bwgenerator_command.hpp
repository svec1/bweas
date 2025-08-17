//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWGENERATOR_HPP
#define BWGENERATOR_HPP

#include <bw_defs.hpp>

namespace bweas {

struct command;

class command_generator;

// name_input_file, command
using commands = vec<command>;
} // namespace bweas

struct bweas::command {
  public:
    command() = default;
    command(string_v _name_output_file, string_v _name_program, vec<string> _args)
        : name_output_file(_name_output_file), name_program(_name_program), args(_args) {
    }

  public:
    string name;
    string name_output_file;
    string name_program;

    vec<string> args;
    uset<string> depends_command;

  public:
    size_t pid_execute_process = 0;
    std::optional<bool> success;
};

// The class defines the API for internal generators, i.e. built into bweas as basic
class bweas::command_generator {
  public:
    command_generator(const context &__context) : _context(__context) {
    }

  public:
    void get_input_files();
    commands generate();

  private:
    std::reference_wrapper<const context> _context;
};

#endif
