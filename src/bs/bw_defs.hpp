#ifndef BWDEFS__H
#define BWDEFS__H

#include "lang/static_struct.hpp"
#include "tools/exception.hpp"

#include <memory>
#include <vector>

namespace bweas {
using bwarg = std::pair<std::string, std::string>;
using bwargs = std::vector<bwarg>;
using bwarg_files = std::pair<std::string, std::vector<std::string>>;
using bwargs_files = std::vector<bwarg_files>;
using bwqueue_templates = std::vector<std::shared_ptr<var::struct_sb::template_command>>;

using commands = std::vector<std::string>;

class bwbuilder_excp : public bw_excp::bweas_exception {
  public:
    bwbuilder_excp(std::string _what_hp, std::string number_err)
        : what_hp(_what_hp), bweas_exception("BWS" + number_err) {
    }
    ~bwbuilder_excp() noexcept override final = default;

  public:
    const char *what() const noexcept override final {
        return what_hp.c_str();
    }

  private:
    std::string what_hp;
};

} // namespace bweas

#endif