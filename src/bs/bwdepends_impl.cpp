//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwdepends_api.hpp>
#include <bwlogger.hpp>

using namespace bweas;

static bweas::logger _log{"BWDEPENDS_FILES_SYSTEM"};

depends_files::depends_map &depends_files::build_graphs_depends_files(const vec<string> &src_files) {
    for (const auto &file : src_files) {
        try {
            if (mdepends.find(file) == mdepends.end())
                mdepends[file] = build_graph_depends_file(file);
        }
        catch (std::exception &excp) {
            _log << (log_message(log_type::fatal) << "Failed to build a graph of file dependencies: " << excp.what());
        }
    }
    return mdepends;
}
depends_files::depends_map &depends_files::build_graphs_depends_file_v(const string &name_file) {
    try {
        mdepends[name_file] = build_graph_depends_file(name_file);
    }
    catch (std::exception &excp) {
        _log << (log_message(log_type::fatal) << "Failed to build a graph of file dependencies: " << excp.what());
    }

    return mdepends;
}
depends_files::depends_map &depends_files::build_graph_depends_file_string(string name_file, string depends_file) {
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
depends_files::depends_map &depends_files::get_graphs_depends_files() {
    return mdepends;
}
string depends_files::get_string_depends_file(string name_file) {
    string str;
    for (const auto &dependence : mdepends[name_file])
        str += dependence + "\n";

    return str;
}
void depends_files::set_include_paths(const vec<string> _include_paths) {
    include_paths = _include_paths;
}
