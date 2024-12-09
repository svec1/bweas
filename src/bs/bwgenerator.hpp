#ifndef BWGENERATOR__H
#define BWGENERATOR__H

#include "bw_defs.hpp"
#include "bwpackage.hpp"
#include "tools/bwlua.hpp"

#define NAME_FUNCTION_GENLUA "generate"
#define DEFINITION_FUNCTION_GENLUA <std::vector<std::string>, bwlua::lua::table<std::string, std::vector<std::string>>>

#define NAME_FUNCTION_FILECLUA "get_files_input"
#define DEFINITION_FUNCTION_FILECLUA                                                                                   \
<bwlua::lua::table<std::string, std::vector<std::string>>, bwlua::lua::table<std::string, std::any>, std::vector<bwlua::lua::table<std::string, std::any>>>

namespace bweas {

namespace bwInterface {

// Interface class that defines the structure of generator classes
class bwIGenerator {
  public:
    bwIGenerator &operator=(const bwIGenerator &) = delete;

  public:
    virtual void deleteGenerator() = 0;
    // Generator initialization function
    virtual void init() = 0;
    // Returns a list of files of the passed target that need to be compiled
    virtual std::map<std::string, std::vector<std::string>> input_files(const var::struct_sb::target_out &target,
                                                                        const bwqueue_templates &target_queue_templates,
                                                                        std::string) = 0;
    // The generator function returns the commands generated by it in the order of the templates used for the
    // transmitted target
    virtual commands gen_commands(const var::struct_sb::target_out &, bwqueue_templates &, std::string,
                                  std::map<std::string, std::vector<std::string>>) = 0;

  protected:
    virtual ~bwIGenerator() = default;
};

} // namespace bwInterface

namespace tools_generator {

// Generates a file name based on the pattern and the passed index of the given file
extern std::string get_name_output_file(std::string pattern_file, u32t index, std::string dir_work_endv);

} // namespace tools_generator

using func_generator = commands (*)(const var::struct_sb::target_out &, bwqueue_templates &,
                                    const std::vector<var::struct_sb::call_component> &, std::string,
                                    std::map<std::string, std::vector<std::string>>);
using func_get_files_input = std::map<std::string, std::vector<std::string>> (*)(
    const var::struct_sb::target_out &, const bwqueue_templates &,
    const std::vector<var::struct_sb::call_component> &ccmp_p, std::string);

// An abstract class that defines the creation of generator classes and is also a generalization
class bwGenerator : public bwInterface::bwIGenerator {
  protected:
    ~bwGenerator() = default;

  public:
    void set_ccomponents(std::vector<var::struct_sb::call_component> &ccmp) {
        ccmp_p = std::shared_ptr<std::vector<var::struct_sb::call_component>>(
            (std::vector<var::struct_sb::call_component> *)&ccmp,
            [](const std::vector<var::struct_sb::call_component> *) {});
    }

  public:
    static inline bwGenerator *createGeneratorInt(func_generator, func_get_files_input);
    static inline bwGenerator *createGeneratorLua(std::string);

  public:
    static bool is_exist(bwGenerator *generator) {
        if (!generator)
            return 0;
        return 1;
    }

  protected:
    std::shared_ptr<std::vector<var::struct_sb::call_component>> ccmp_p;
};

// The class defines the API for internal generators, i.e. built into bweas as basic
class bwGeneratorIntegral : public bwGenerator {
    friend bwGenerator *bwGenerator::createGeneratorInt(func_generator, func_get_files_input);

  private:
    bwGeneratorIntegral(func_generator, func_get_files_input);
    ~bwGeneratorIntegral() = default;

  public:
    void init() override final;
    void deleteGenerator() override final;
    std::map<std::string, std::vector<std::string>> input_files(const var::struct_sb::target_out &,
                                                                const bwqueue_templates &, std::string) override final;
    commands gen_commands(const var::struct_sb::target_out &, bwqueue_templates &, std::string,
                          std::map<std::string, std::vector<std::string>>) override final;

  private:
    func_generator generator_p;
    func_get_files_input files_input_p;
    static inline bool init_glob_gnint{0};
};

// The class defines the API for generators written in lua and presented in bweas packages
class bwGeneratorLua : public bwGenerator {
    friend bwGenerator *bwGenerator::createGeneratorLua(std::string);

  private:
    bwGeneratorLua(std::string);
    ~bwGeneratorLua() = default;

  public:
    void init() override final;
    void deleteGenerator() override final;
    std::map<std::string, std::vector<std::string>> input_files(const var::struct_sb::target_out &,
                                                                const bwqueue_templates &, std::string) override final;
    commands gen_commands(const var::struct_sb::target_out &, bwqueue_templates &, std::string,
                          std::map<std::string, std::vector<std::string>>) override final;

  private:
    bwlua::lua lua;
    static inline bool init_glob_gnlua{0};
};

bwGenerator *bwGenerator::createGeneratorInt(func_generator generator, func_get_files_input files_input) {
    return (bwGenerator *)new bwGeneratorIntegral(generator, files_input);
}
bwGenerator *bwGenerator::createGeneratorLua(std::string src_lua) {
    return (bwGenerator *)new bwGeneratorLua(src_lua);
}

extern std::map<std::string, std::vector<std::string>> bwfile_inputs_internal(
    const var::struct_sb::target_out &target, const bwqueue_templates &target_queue_templates,
    const std::vector<var::struct_sb::call_component> &ccmp_p, std::string dir_work_endv);

// First and basic template-based command generator
extern commands bwgenerator_internal(const var::struct_sb::target_out &trg, bwqueue_templates &templates,
                                     const std::vector<var::struct_sb::call_component> &ccmp_p,
                                     std::string dir_work_endv,
                                     std::map<std::string, std::vector<std::string>> files_input);

} // namespace bweas

#endif