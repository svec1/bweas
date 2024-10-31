#ifndef BWGENERATOR__H
#define BWGENERATOR__H

#include "bw_defs.hpp"
#include "bwpackage.hpp"
#include "tools/bwlua.hpp"

namespace bweas {

namespace bwInterface {

class bwIGenerator {
  public:
    bwIGenerator &operator=(const bwIGenerator &) = delete;

  public:
    void set_call_components(std::vector<var::struct_sb::call_component> &ccmp) {
        ccmp_p = std::shared_ptr<std::vector<var::struct_sb::call_component>>(
            (std::vector<var::struct_sb::call_component> *)&ccmp,
            [](const std::vector<var::struct_sb::call_component> *) {});
    }

    virtual void init() = 0;
    virtual command gen_commands(const var::struct_sb::target_out &, bwqueue_templates &) = 0;

  protected:
    virtual ~bwIGenerator() = default;

  protected:
    std::shared_ptr<std::vector<var::struct_sb::call_component>> ccmp_p;
};

} // namespace bwInterface

class bwGeneratorIntegral : public bwInterface::bwIGenerator {
  public:
    bwGeneratorIntegral();

  public:
    void set_func_generator(command (*)(const var::struct_sb::target_out &, bwqueue_templates &));

    void init() override final;
    command gen_commands(const var::struct_sb::target_out &, bwqueue_templates &) override final;

  private:
    command (*func_p)(const var::struct_sb::target_out &, bwqueue_templates &);
    static inline bool init_glob{0};
};

class bwGeneratorLua : public bwInterface::bwIGenerator {
  public:
    bwGeneratorLua();

  public:
    void load_lua(std::string src_lua);

    void init() override final;
    command gen_commands(const var::struct_sb::target_out &, bwqueue_templates &) override final;

  private:
    bwlua::lua lua;
    static inline bool init_glob{0};
};
} // namespace bweas

#endif