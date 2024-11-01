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
    virtual void deleteGenerator() = 0;
    virtual void init() = 0;
    virtual commands gen_commands(const var::struct_sb::target_out &, bwqueue_templates &) = 0;

  protected:
    virtual ~bwIGenerator() = default;
};

} // namespace bwInterface

namespace tools_generator {
extern std::string get_file_w_index(std::string pattern_file, u32t index);
}

class bwGenerator : public bwInterface::bwIGenerator {
  protected:
    ~bwGenerator() = default;

  public:
    void set_global_data(std::vector<var::struct_sb::call_component> &ccmp, bwargs &external_args) {
        ccmp_p = std::shared_ptr<std::vector<var::struct_sb::call_component>>(
            (std::vector<var::struct_sb::call_component> *)&ccmp,
            [](const std::vector<var::struct_sb::call_component> *) {});
        external_args_p = std::shared_ptr<bwargs>((bwargs *)&external_args, [](const bwargs *) {});
    }

  public:
    static inline bwGenerator *createGeneratorInt(commands (*)(const var::struct_sb::target_out &, bwqueue_templates &,
                                                               const std::vector<var::struct_sb::call_component> &,
                                                               const bwargs &));
    static inline bwGenerator *createGeneratorLua(std::string);

  protected:
    std::shared_ptr<std::vector<var::struct_sb::call_component>> ccmp_p;
    std::shared_ptr<bwargs> external_args_p;
};

class bwGeneratorIntegral : public bwGenerator {
    friend bwGenerator *bwGenerator::createGeneratorInt(
        commands (*)(const var::struct_sb::target_out &, bwqueue_templates &,
                     const std::vector<var::struct_sb::call_component> &, const bwargs &));

  private:
    bwGeneratorIntegral(commands (*)(const var::struct_sb::target_out &, bwqueue_templates &,
                                     const std::vector<var::struct_sb::call_component> &, const bwargs &));
    ~bwGeneratorIntegral() = default;

  public:
    void init() override final;
    void deleteGenerator() override final;
    commands gen_commands(const var::struct_sb::target_out &, bwqueue_templates &) override final;

  private:
    commands (*func_p)(const var::struct_sb::target_out &, bwqueue_templates &,
                       const std::vector<var::struct_sb::call_component> &, const bwargs &);
    static inline bool init_glob_gnint{0};
};

class bwGeneratorLua : public bwGenerator {
    friend bwGenerator *bwGenerator::createGeneratorLua(std::string);

  private:
    bwGeneratorLua(std::string);
    ~bwGeneratorLua() = default;

  public:
    void init() override final;
    void deleteGenerator() override final;
    commands gen_commands(const var::struct_sb::target_out &, bwqueue_templates &) override final;

  private:
    bwlua::lua lua;
    static inline bool init_glob_gnlua{0};
};

bwGenerator *bwGenerator::createGeneratorInt(commands (*func)(const var::struct_sb::target_out &, bwqueue_templates &,
                                                              const std::vector<var::struct_sb::call_component> &,
                                                              const bwargs &)) {
    return (bwGenerator *)new bwGeneratorIntegral(func);
}
bwGenerator *bwGenerator::createGeneratorLua(std::string src_lua) {
    return (bwGenerator *)new bwGeneratorLua(src_lua);
}

extern commands bwign0_1v(const var::struct_sb::target_out &trg, bwqueue_templates &templates,
                          const std::vector<var::struct_sb::call_component> &ccmp_p, const bwargs &global_extern_args);

} // namespace bweas

#endif