//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwmodule.hpp>
#include <lang/parser.hpp>

#include <bwluatools.hpp>

using namespace bweas;
using namespace bweas::utils;

static logger _log{"BWMODULE"};

vec<module_manager::_module> module_manager::init_modules(vec<module_cfg> &modules_cfg) {
    vec<module_manager::_module> md_s;

    for (const auto &module_cfg : modules_cfg) {
        if (!module_cfg.name_src_file.empty()) {
            auto src_file = file_utils::open_file(module_cfg.name_src_file);
            bwlang::parser p(file_utils::read_file(src_file));
            p.parse();
            md_s.emplace_back(module_cfg.name, std::move(p.get_context()));
        }
        else
            md_s.emplace_back(module_cfg.name);

        for (const auto &[name, profile] : module_cfg.profiles)
            md_s[md_s.size() - 1].ctx.sc[name] = profile;
    }

    return md_s;
}
