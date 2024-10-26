#ifndef BWGENERATOR__H
#define BWGENERATOR__H

#include "bw_defs.hpp"
#include "bwpackage.hpp"

namespace bweas {

namespace _interface {

class bwIGenerator {
  public:
    virtual void init() = 0;
    virtual void set_call_components(const var::struct_sb::call_component &ccmp) = 0;
    virtual commands gen_commands(const var::struct_sb::target_out &, bwqueue_templates &) = 0;

  private:
    bwpackage pckg;

    std::shared_ptr<std::vector<var::struct_sb::call_component>> ccmp_p;
};

class bwGenerator : public bwIGenerator {
  public:
    void init() override final;
    void set_call_components(const var::struct_sb::call_component &ccmp) override final;
    commands gen_commands(const var::struct_sb::target_out &, bwqueue_templates &) override final;
};

} // namespace _interface
} // namespace bweas

#endif