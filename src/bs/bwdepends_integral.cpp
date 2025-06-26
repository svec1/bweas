//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwdepends_integral.hpp>
#include <tools/bwfile.hpp>

using namespace bweas;

static logger _log{"BWDEPENDS_FILES_SYSTEM[INTEGRAL]"};

uset<string> bweas::depends_integral::build_graph_depends_file(string_v name_file) {
    return build_graph_depends_file_c_cpp(name_file, include_paths);
}

uset<string> bweas::depends_integral::build_graph_depends_file_c_cpp(string_v name_file,
                                                                     const vec<string> &include_paths) {
    if (name_file.find(".cpp") == name_file.npos)
        return {};

    string last_current_path = fs::current_path();

    fs::current_path(fs::path(name_file).parent_path());
    uset<string> graph_depends_file;

    try {
        bwtools::file_it file = bwtools::open_file(name_file);
        string src_file       = bwtools::read_file(bwtools::get_ref_file(file));
        bool is_system_header = 0;

        std::regex include_line_syntax(R"(#include\s+(<[\/\w+]+(?:\.\w+)?>)|(\"[\/\w+]+(?:\.\w+)?\"))");
        for (auto it_match = std::sregex_iterator(src_file.begin(), src_file.end(), include_line_syntax);
             it_match != std::sregex_iterator(); ++it_match) {
            string include_file = (*it_match)[0].str();

            if (include_file.find("#include ") == 0)
                include_file.erase(0, 9);

            if (include_file.find('<') == 0)
                is_system_header = 1;

            include_file.erase(0, 1);
            include_file.erase(include_file.size() - 1, 1);

            if (include_file.find(".hpp") == include_file.npos)
                continue;

            if (is_system_header)
                include_file = bwfile::get_path_file(include_file, include_paths);
            else
                include_file = bwfile::get_path_file(include_file);

            graph_depends_file.insert(include_file);
            graph_depends_file.merge(build_graph_depends_file_c_cpp(include_file, include_paths));
        }

        bwtools::close_file(file);
    }
    catch (std::exception &excp) {
        throw std::runtime_error(fs::path(name_file).filename().c_str() + string("->") + excp.what());
    }

    fs::current_path(last_current_path);

    return graph_depends_file;
}
