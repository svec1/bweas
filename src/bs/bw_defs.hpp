#ifndef BWDEFS__H
#define BWDEFS__H

#include "lang/interpreter.hpp"
#include "tools/exception.hpp"

#include <memory>
#include <vector>

#define BWEAS_VERSION_STR "0.1.0"

#define BWEAS_VERSION_MAJOR "0"
#define BWEAS_VERSION_MINOR "1"
#define BWEAS_VERSION_PATCH "0"

#define BWEAS_HELP                                                                                                     \
    "bweas-call: \n\tbweas <parameter><option>... path_to_build\n\t bweas path_bweas_config <parameter><option>..."

namespace bweas {
using bwarg = std::pair<std::string, std::string>;
using bwargs = std::vector<bwarg>;
using bwarg_files = std::pair<std::string, std::vector<std::string>>;
using bwargs_files = std::vector<bwarg_files>;
using bwqueue_templates = std::vector<std::shared_ptr<var::struct_sb::template_command>>;

using commands = std::vector<std::string>;

namespace exception {

class bwbuilder_excp : public bw_excp::bweas_exception {
  public:
    bwbuilder_excp(std::string _what_hp, std::string number_err, std::string prefix_err = "")
        : what_hp(_what_hp), bweas_exception("BWS" + prefix_err + number_err) {
    }
    ~bwbuilder_excp() noexcept override = default;

  public:
    const char *what() const noexcept override final {
        return what_hp.c_str();
    }

  protected:
    std::string what_hp;
};

class bwpackage_excp : public bwbuilder_excp {
  public:
    bwpackage_excp(std::string _what_hp, std::string number_err) : bwbuilder_excp(_what_hp, number_err, "-PCKG") {
    }
    ~bwpackage_excp() noexcept override final = default;
};
} // namespace exception

} // namespace bweas

#endif