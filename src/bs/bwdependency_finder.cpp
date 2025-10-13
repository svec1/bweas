//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwdependency_finder.hpp>
#include <bwlogger.hpp>

using namespace bweas;

static bweas::logger _log{"BWDEPENDENCY_FINDER"};

dependency_finder::dependency_map &dependency_finder::build_graphs(const vec<string> &_include_paths,
                                                                   const vec<string> &src_files) {
    include_paths = _include_paths;
    for (const auto &file : src_files) {
        try {
            if (mdepends.find(file) == mdepends.end())
                build_graph_depends_file(file, mdepends[file]);
        }
        catch (std::exception &excp) {
            _log << (log_message(log_type::fatal) << "Failed to build a graph of file dependencies: \n" << excp.what());
        }
    }
    return mdepends;
}
dependency_finder::dependency_map &dependency_finder::build_graph(const vec<string> &_include_paths,
                                                                  const string &name_file) {
    try {
        include_paths = _include_paths;
        build_graph_depends_file(name_file, mdepends[name_file]);
    }
    catch (std::exception &excp) {
        _log << (log_message(log_type::fatal) << "Failed to build a graph of file dependencies: \n" << excp.what());
    }

    return mdepends;
}
dependency_finder::dependency_map &dependency_finder::build_graph_depends_file_string(string name_file,
                                                                                      string depends_file) {
    auto it_depends_file = mdepends.find(mdepends[name_file].size() ? name_file : name_file);
    pdiff it             = 0;
    while ((it = depends_file.find("\n")) != depends_file.npos) {
        string name_dependences;
        for (size_t i = 0; i < it; ++i)
            name_dependences += depends_file[i];
        it_depends_file->second.insert(name_dependences);
        depends_file.erase(0, it + 1);
    }
    return mdepends;
}
dependency_finder::dependency_map &dependency_finder::get_graphs_depends_files() {
    return mdepends;
}
string dependency_finder::get_string_depends_file(string name_file) {
    string str;
    for (const auto &dependence : mdepends[name_file])
        str += dependence + "\n";

    return str;
}

void dependency_finder::build_graph_depends_file(string_v name_file, uset<string> &graph_depends_file) {
    string last_current_path = fs::current_path().string();

    fs::current_path(fs::path(name_file).parent_path());

    try {
        auto file       = utils::file_utils::open_file(name_file);
        string src_file = utils::file_utils::read_file(file);

        std::regex include_line_syntax(search_regex);
        for (auto it_match = std::sregex_iterator(src_file.begin(), src_file.end(), include_line_syntax);
             it_match != std::sregex_iterator(); ++it_match) {
            bool is_system_header = 0;
            string include_file   = (*it_match)[1].str();

            if (include_file[0] == char_global_finder)
                is_system_header = 1;

            include_file = include_file.substr(1, include_file.size() - 2);

            if (is_system_header && include_file.find(".") == include_file.npos)
                continue;

            if (is_system_header)
                include_file = utils::file_utils::get_path_file(include_file, include_paths);
            else
                include_file = utils::file_utils::get_path_file(include_file);

            if (graph_depends_file.contains(include_file) || include_file.empty())
                continue;

            graph_depends_file.insert(include_file);
            build_graph_depends_file(include_file, graph_depends_file);
        }
    }
    catch (std::exception &excp) {
        throw std::runtime_error((fs::path(name_file).filename().string() + string(" -> ") + excp.what()));
    }

    fs::current_path(last_current_path);
}

