//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include "bwgenerator_integral.hpp"
#include "bwgntools.hpp"
#include "tools/bwfile.hpp"

using namespace bweas;
using namespace generator_api;
using namespace generator;

static logger log{"BWGENERATOR[INTERNAL]"};

integral_generator::integral_generator(func_generator _generator_p,
                                       func_build_graph_depends_file _build_graph_depends_file_p,
                                       func_get_input_files _get_input_files_p) {
    generator_p                = _generator_p;
    build_graph_depends_file_p = _build_graph_depends_file_p;
    get_input_files_p          = _get_input_files_p;
}

void integral_generator::init() {
}
void integral_generator::_delete() {
    delete this;
}

std::unordered_set<std::string> integral_generator::build_graph_depends_file(std::string_view language,
                                                                             std::string_view name_file,
                                                                             std::string_view work_directory,
                                                                             std::vector<std::string> include_paths) {
    return build_graph_depends_file_p(language, name_file, work_directory, include_paths);
}

void integral_generator::get_input_files(data_transfer &data_t) {
    get_input_files_p(data_t);
}

gen_command integral_generator::generate_command(data_transfer &data_t) {
    generator::tools::parse_basic_args(*data_t.context->current_target, data_t.context->templates,
                                       data_t.context->global_external_args);
    return generator_p(data_t);
}

std::unordered_set<std::string> bweas::generator::bwbuild_graph_depends_file(std::string_view language,
                                                                             std::string_view name_file,
                                                                             std::string_view work_directory,
                                                                             std::vector<std::string> include_paths) {
    return {};
}

void bweas::generator::bwget_input_files(data_transfer &data_t) {
}

gen_command bweas::generator::bwgenerator(data_transfer &data_t) {
    return std::map<std::string, std::string>{{"test", "none"}};
}
