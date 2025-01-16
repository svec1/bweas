#include "bwgntools.hpp"

std::string bweas::generator::tools::get_name_output_file(std::string pattern_file, u32t index,
                                                          std::string dir_work_endv) {
    if (pattern_file.find(".") == pattern_file.npos)
        return dir_work_endv + "/" + pattern_file + std::to_string(index);
    std::string name_output_file_curr = pattern_file, extension_output_file_curr = pattern_file;
    name_output_file_curr.erase(name_output_file_curr.find("."), name_output_file_curr.size());
    extension_output_file_curr.erase(0, extension_output_file_curr.find("."));
    if (index != 0)
        return dir_work_endv + "/" + name_output_file_curr + std::to_string(index) + extension_output_file_curr;
    return dir_work_endv + "/" + name_output_file_curr + extension_output_file_curr;
}