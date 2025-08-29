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
class generator_command;

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
    string build_string_command();

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
class bweas::generator_command {
  public:
    generator_command(context *const __context) : _context(__context) {
    }

  public:
    void get_input_files();
    commands generate();

  private:
    // Generates a file name based on the pattern and the passed index of the given file.
    static string get_name_output_file(string_v pattern_file, string_v name_file = "", size_t index = 0);
    static bool should_uses_src_file(string_v src_file, string_v output_file, const uset<string> &dfiles);

  private:
    context *const _context;
};

#endif
