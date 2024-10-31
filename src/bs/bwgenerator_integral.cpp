#include "bwgenerator.hpp"

using namespace bweas;

bwGeneratorIntegral::bwGeneratorIntegral() {
    if (!init_glob) {

        init_glob = 1;
    }
}

void bwGeneratorIntegral::init() {
}
void bwGeneratorIntegral::set_func_generator(command (*generator)(const var::struct_sb::target_out &,
                                                                  bwqueue_templates &)) {
    func_p = func_p;
}
command bwGeneratorIntegral::gen_commands(const var::struct_sb::target_out &trg, bwqueue_templates &templates) {
    return func_p(trg, templates);
}