//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwgntools.hpp>

using namespace bweas;

string generator_tools::get_name_output_file(string pattern_file, string name_file, size_t index) {
    if (pattern_file.find(".") == pattern_file.npos)
        return pattern_file + (index ? std::to_string(index) : "");

    string name_output_file_curr = pattern_file, extension_output_file_curr = pattern_file;

    name_output_file_curr.erase(name_output_file_curr.find("."), name_output_file_curr.size());
    extension_output_file_curr.erase(0, extension_output_file_curr.find("."));

    if (name_output_file_curr.find("{}") == name_output_file_curr.size() - 2 && !name_file.empty()) {
        name_output_file_curr.erase(name_output_file_curr.size() - 2);
        return name_output_file_curr + fs::path(name_file).filename().c_str() + extension_output_file_curr;
    }

    if (index != 0)
        return name_output_file_curr + std::to_string(index) + extension_output_file_curr;
    return name_output_file_curr + extension_output_file_curr;
}

bool generator_tools::should_uses_src_file(string_v src_file, string_v output_file, const uset<string> &dfiles) {
    if (fs::is_regular_file(output_file) && fs::last_write_time(CACHE_FILE) > fs::last_write_time(output_file))
        return 1;
    else if (!fs::is_regular_file(output_file) || fs::last_write_time(output_file) < fs::last_write_time(src_file))
        return 1;

    for (const auto &dfile : dfiles)
        if (fs::last_write_time(output_file) < fs::last_write_time(dfile))
            return 1;

    return 0;
}

string generator_tools::build_string_command(const command &cmd) {
    string cmd_str = cmd.name_program + " ";
    for (const auto &arg : cmd.args)
        cmd_str += arg + " ";

    return cmd_str;
}
