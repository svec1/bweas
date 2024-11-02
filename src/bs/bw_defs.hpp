#ifndef BWDEFS__H
#define BWDEFS__H

// bweas main header file

#include "lang/interpreter.hpp"
#include "tools/exception.hpp"

#include <memory>
#include <vector>

#define BWEAS_VERSION_STR "0.1.0"

#define BWEAS_VERSION_MAJOR "0"
#define BWEAS_VERSION_MINOR "1"
#define BWEAS_VERSION_PATCH "0"

#define BWEAS_HELP                                                                                                     \
    "bweas-call: \n   bweas <parameter>... path_depending\n   bweas path_bweas_config <parameter>..."                  \
    "\nAcceptable parameters:"                                                                                         \
    "\n   --build - builds the project (either by executing the configuration file or deserializing the cache file "   \
    "if it exists)"                                                                                                    \
    "\n   --cfg - executes the configuration file if it has been changed and creates a new cache file"                 \
    "\n   --package - creates a bweas package based on the transferred files (json config, lua - generator script)"

namespace bweas {
using bwarg = std::pair<std::string, std::string>;
using bwargs = std::vector<bwarg>;
using bwarg_files = std::pair<std::string, std::vector<std::string>>;
using bwargs_files = std::vector<bwarg_files>;
using bwqueue_templates = std::vector<std::shared_ptr<var::struct_sb::template_command>>;

using command = std::string;
using commands = std::vector<command>;

namespace exception {

// Exception class for builder only.
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

// Exception class for bweas-package only.
class bwpackage_excp : public bwbuilder_excp {
  public:
    bwpackage_excp(std::string _what_hp, std::string number_err) : bwbuilder_excp(_what_hp, number_err, "-PCKG") {
    }
    ~bwpackage_excp() noexcept override final = default;
};

// Exception class for bweas-generator only.
class bwgenerator_excp : public bwbuilder_excp {
  public:
    bwgenerator_excp(std::string _what_hp, std::string number_err) : bwbuilder_excp(_what_hp, number_err, "-GNRT") {
    }
    ~bwgenerator_excp() noexcept override final = default;
};
} // namespace exception

} // namespace bweas

#endif