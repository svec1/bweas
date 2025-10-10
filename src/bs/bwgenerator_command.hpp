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

using commands = vec<command>;
} // namespace bweas

/** \brief The structure defining the command object. */
struct bweas::command final {
  public:
    command() = default;
    /** \brief Constructor.
     *  \param [in] _name_output_file The name of the output file.
     *  \param [in] _name_program The name of the program being called.
     *  \param [in] _args Array of arguments.
     */
    command(string_v _name_output_file, string_v _name_program, vec<string> _args)
        : name_output_file(_name_output_file), name_program(_name_program), args(_args) {
    }

  public:
    /** \return string The string corresponding to the program call command. */
    string build_string_command();

  public:
    string name;             ///< Command name.
    string name_output_file; ///< The name of the output file (the file that will be created by the invoked program).
    string name_program;     ///< The name of the program.
    vec<string> args;        ///< An array of arguments with which the program will be invoked.
    uset<string> depends_command; ///< An array of commands that the current command depends on..

  public:
    size_t pid_execute_process = 0;
    std::optional<bool> success;
};

/** \brief The command generator class generates command based on templates. */
class bweas::generator_command final {
  public:
    generator_command(context *const __context) : _context(__context) {
    }

  public:
    /** \brief Defines the input files and a single-generate mod for each template.
     * \detail Sets context::ifiles.
     * Parses the arguments: [source_files], INPUT_FILE.
     * If the INPUT_FILE or [source_files:1] parameter has been parsed, the current template will be single-generate.
     * */
    void get_input_files() const;
    /**
     * \brief Generates a set of commands for the current context template,
     * which should be executed in the specified order.
     *
     * \detail If a single-generate mode is defined for the template, then as many commands will be generated as there
     * are total input files for the current template. Otherwise, a single command will be generated based on such a
     * template.
     *
     * \return commands
     * */
    commands generate() const;

  private:
    /** \brief Generates a file name based on the pattern and the passed index of the given file.
     *
     * \detail If there is a '{}', it is replaced by the name of the input file. Otherwise, the transmitted index is
     * added to the file name.
     *
     * \param [in] pattern_file Template for the name of the future output file.
     * \param [in] name_target The name of the target object.
     * \param [in] name_file The name of the corresponding input file.
     * \param [in] index The index of the input file (must be its sequence number).
     * \return string The name of the output file.
     */
    static string get_name_output_file(string pattern_file, string_v name_target, string_v name_file = "",
                                       size_t index = 0);
    /** \brief Determines whether to use the input file to generate the command.
     *
     * \detail Returns true if there was a change in the cache file after creating the output file, if there was a
     * change in the input file or one of its dependencies.
     *
     * \param [in] name_file The name of the input file.
     * \param [in] output_file The name of the output file corresponds to the input file.
     * \param [in] dfiles An array of dependent files.
     * \return bool
     */
    static bool should_uses_src_file(string_v name_file, string_v output_file, const uset<string> &dfiles);

  private:
    context *const _context;
};

#endif
