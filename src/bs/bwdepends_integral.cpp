//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <regex>

#include "bwdepends_integral.hpp"

using namespace bweas;

static logger log{"BWDEPENDS_FILES_SYSTEM[INTEGRAL]"};

std::unordered_set<std::string> bweas::bwdepends_integral::build_graph_depends_file(std::string_view name_file) {
    return build_graph_depends_file_c_cpp(name_file, include_paths);
}

std::unordered_set<std::string> bweas::bwdepends_integral::build_graph_depends_file_c_cpp(
    std::string_view name_file, const std::vector<std::string> &include_paths) {
    std::unordered_set<std::string> graph_depends_file;

    try {
        bwtools::file_it file = bwtools::open_file(name_file);
        std::string src_file  = bwtools::read_file(bwtools::get_ref_file(file));
        bool is_system_header = 0;

        i32t pos_include = 0;

        src_file = std::regex_replace(src_file, std::regex{" "}, "");

        while ((pos_include = src_file.find("#include")) != src_file.npos) {
            if (auto pos_path = src_file.find_first_of("\"<"); pos_path == pos_include + 8) {
                if (src_file[pos_path++] == '<')
                    is_system_header = 1;

                std::string include_file_path;
                while (pos_path < src_file.size() && src_file[pos_path] != '\"' && src_file[pos_path] != '>')
                    include_file_path += src_file[pos_path++];

                if (include_file_path.find(".hpp") == include_file_path.npos)
                    goto next_include;

                if (is_system_header)
                    include_file_path = bwfile::get_path_file(include_file_path, include_paths);
                else
                    include_file_path = bwfile::get_path_file(include_file_path);

                graph_depends_file.insert(include_file_path);
                graph_depends_file.merge(build_graph_depends_file_c_cpp(include_file_path, include_paths));

            next_include:
                src_file.erase(pos_include, pos_path + 1);
            }
            else
                throw std::runtime_error("Invalid syntax");
        }

        bwtools::close_file(file);
    }
    catch (std::exception &excp) {
        throw std::runtime_error(name_file.data() + std::string("->") + excp.what());
    }

    return graph_depends_file;
}
