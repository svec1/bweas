//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwdepends_api.hpp>
#include <bwlogger.hpp>
#include <utils/file_utils.hpp>

using namespace bweas;
using namespace bweas::utils;

static logger _log{"BWDEPENDS_FINDER[INTEGRAL]"};

void bweas::depends_integral_cxx::build_graph_depends_file(string_v name_file, uset<string> &graph_depends_file) {
    if (name_file.find(".c") == name_file.npos && name_file.find(".h") == name_file.npos)
        return;
    string last_current_path = fs::current_path();

    fs::current_path(fs::path(name_file).parent_path());

    try {
        file_utils::file_it file = file_utils::open_file(name_file);
        string src_file          = file_utils::read_file(file_utils::get_ref_file(file));
        bool is_system_header;

        std::regex include_line_syntax(R"(#include\s+((<[\/\w+]+(?:\.\w+)?>)|(\"[\/\w+]+(?:\.\w+)?\")))");
        for (auto it_match = std::sregex_iterator(src_file.begin(), src_file.end(), include_line_syntax);
             it_match != std::sregex_iterator(); ++it_match) {
            is_system_header    = 0;
            string include_file = (*it_match)[1].str();

            if (include_file[0] == '<')
                is_system_header = 1;

            include_file.erase(0, 1);
            include_file.erase(include_file.size() - 1, 1);

            if (is_system_header && include_file.find(".") == include_file.npos)
                continue;

            if (is_system_header)
                include_file = file_utils::get_path_file(include_file, include_paths);
            else
                include_file = file_utils::get_path_file(include_file);

            if (graph_depends_file.contains(include_file) || include_file.empty())
                continue;

            graph_depends_file.insert(include_file);
            build_graph_depends_file(include_file, graph_depends_file);
        }

        if (file_utils::get_ref_file(file).file_opened)
            file_utils::close_file(file);
    }
    catch (std::exception &excp) {
        throw std::runtime_error(fs::path(name_file).filename().c_str() + string("->") + excp.what());
    }

    fs::current_path(last_current_path);
}

