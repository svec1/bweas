//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include "bwdepends_files.hpp"

static bweas::logger log{"BWDEPENDS_FILES_SYSTEM"};

bweas::bwdepends_files::depends_map &bweas::bwdepends_files::build_graphs_depends_files(
    const std::vector<std::string> &src_files) {
    for (const auto &file : src_files) {
        try {
            if (mdepends.find(file) == mdepends.end())
                mdepends[file] = build_graph_depends_file(file);
        }
        catch (std::exception &excp) {
            log << bwtools::fatal
                << (log_message(log_type::fatal) << "Failed to build a graph of file dependencies: " << excp.what());
        }
    }
    return mdepends;
}
bweas::bwdepends_files::depends_map &bweas::bwdepends_files::build_graphs_depends_file_v(const std::string &name_file) {
    try {
        mdepends[name_file] = build_graph_depends_file(name_file);
    }
    catch (std::exception &excp) {
        log << bwtools::fatal
            << (log_message(log_type::fatal) << "Failed to build a graph of file dependencies: " << excp.what());
    }

    return mdepends;
}
bweas::bwdepends_files::depends_map &bweas::bwdepends_files::build_graph_depends_file_string(std::string name_file,
                                                                                             std::string depends_file) {
    auto it_depends_file = mdepends.find(mdepends[name_file].size() ? name_file : name_file);
    i32t it              = 0;
    while ((it = depends_file.find("\n")) != depends_file.npos) {
        std::string name_dependences;
        for (u32t i = 0; i < it; ++i)
            name_dependences += depends_file[i];
        it_depends_file->second.insert(name_dependences);
        depends_file.erase(0, it + 1);
    }
    return mdepends;
}
bweas::bwdepends_files::depends_map &bweas::bwdepends_files::get_graphs_depends_files() {
    return mdepends;
}
std::string bweas::bwdepends_files::get_string_depends_file(std::string name_file) {
    std::string str;
    for (const auto &dependence : mdepends[name_file])
        str += dependence + "\n";

    return str;
}
void bweas::bwdepends_files::set_include_paths(const std::vector<std::string> _include_paths) {
    include_paths = _include_paths;
}

