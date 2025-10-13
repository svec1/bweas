//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwlang.hpp>
#include <bwmodule.hpp>

#include <bwluatools.hpp>

using namespace bweas;
using namespace bweas::utils;

static logger _log{"BWMODULE"};

vec<module_manager::_module> module_manager::init_modules(vec<module_cfg> &modules_cfg) {
    vec<module_manager::_module> md_s;

    for (const auto &module_cfg : modules_cfg) {
        if (!module_cfg.name_src_file.empty()) {
            auto src_file = file_utils::open_file(module_cfg.name_src_file);
            if (!src_file.is_open)
                _log << (log_message(log_type::fatal)
                         << "The module file \'" << module_cfg.name_src_file << "\' could not be opened.");

            try {
                lang l(nullptr, file_utils::read_file(src_file));
                l.import_std_module();
                l.import_modules(md_s);
                l.execute();
                md_s.emplace_back(module_cfg.name, l.get_context());
                bwlang::parser::dump_global_context();
            }
            catch (std::runtime_error &excp) {
                _log << (log_message(log_type::fatal) << "\'" << module_cfg.name << "\' module initialization error: \n"
                                                      << excp.what());
            }
        }
        else
            md_s.emplace_back(module_cfg.name);
    }

    return md_s;
}
