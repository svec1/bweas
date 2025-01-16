#ifndef BWGENERATOR_INTEGRAL__H
#define BWGENERATOR_INTEGRAL__H

#include "bw_defs.hpp"
#include <map>

namespace bweas {
namespace generator {
extern std::map<std::string, std::vector<std::string>> bwfile_inputs(
    const var::struct_sb::target_out &target, const bwqueue_templates &target_queue_templates,
    const std::vector<var::struct_sb::call_component> &ccmp_p, std::string dir_work_endv);

// First and basic template-based command generator
extern std::map<std::string, std::string> bwgenerator(const var::struct_sb::target_out &trg, bwqueue_templates &templates,
                                                   const std::vector<var::struct_sb::call_component> &ccmp_p,
                                                   std::map<std::string, std::vector<std::string>> files_input,
                                                   std::string dir_work_endv);
} // namespace generator
} // namespace bweas

#endif