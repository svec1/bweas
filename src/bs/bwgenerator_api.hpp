#ifndef BWGENERATOR__H
#define BWGENERATOR__H

#include "bw_defs.hpp"
#include "tools/bwlua.hpp"

#define NAME_FUNCTION_GENLUA "generate"
#define DEFINITION_FUNCTION_GENLUA                                                                                     \
<std::map<std::string, std::string>, bwlua::lua::table<std::string, std::vector<std::string>>>

#define NAME_FUNCTION_FILECLUA "get_files_input"
#define DEFINITION_FUNCTION_FILECLUA                                                                                   \
<bwlua::lua::table<std::string, std::vector<std::string>>, bwlua::lua::table<std::string, std::any>, std::vector<bwlua::lua::table<std::string, std::any>>>

namespace bweas {

namespace generator_api {
namespace gninterface {

// Interface class that defines the structure of generator classes
class interface_generator {
  public:
    interface_generator &operator=(const interface_generator &) = delete;

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
    virtual std::map<std::string, std::string> gen_commands(const var::struct_sb::target_out &, bwqueue_templates &,
                                                            std::string,
                                                            std::map<std::string, std::vector<std::string>>) = 0;

  protected:
    virtual ~interface_generator() = default;
};

} // namespace gninterface

using func_generator = std::map<std::string, std::string> (*)(const var::struct_sb::target_out &, bwqueue_templates &,
                                                              const std::vector<var::struct_sb::call_component> &,
                                                              std::map<std::string, std::vector<std::string>>,
                                                              std::string);
using func_get_files_input = std::map<std::string, std::vector<std::string>> (*)(
    const var::struct_sb::target_out &, const bwqueue_templates &,
    const std::vector<var::struct_sb::call_component> &ccmp_p, std::string);

// An abstract class that defines the creation of generator classes and is also a generalization
class base_generator : public gninterface::interface_generator {
  protected:
    ~base_generator() = default;

  public:
    void set_ccomponents(std::vector<var::struct_sb::call_component> &ccmp) {
        ccmp_p = std::shared_ptr<std::vector<var::struct_sb::call_component>>(
            (std::vector<var::struct_sb::call_component> *)&ccmp,
            [](const std::vector<var::struct_sb::call_component> *) {});
    }

  public:
    static inline base_generator *createGeneratorInt(func_generator, func_get_files_input);
    static inline base_generator *createGeneratorLua(std::string);

  public:
    static bool is_exist(base_generator *generator) {
        if (!generator)
            return 0;
        return 1;
    }

  protected:
    std::shared_ptr<std::vector<var::struct_sb::call_component>> ccmp_p;
};

// The class defines the API for internal generators, i.e. built into bweas as basic
class integral_generator : public base_generator {
    friend base_generator *base_generator::createGeneratorInt(func_generator, func_get_files_input);

  private:
    integral_generator(func_generator, func_get_files_input);
    ~integral_generator() = default;

  public:
    void init() override final;
    void deleteGenerator() override final;
    std::map<std::string, std::vector<std::string>> input_files(const var::struct_sb::target_out &,
                                                                const bwqueue_templates &, std::string) override final;
    std::map<std::string, std::string> gen_commands(const var::struct_sb::target_out &, bwqueue_templates &,
                                                    std::string,
                                                    std::map<std::string, std::vector<std::string>>) override final;

  private:
    static inline bool init_glob_gnint{0};

    func_generator generator_p;
    func_get_files_input files_input_p;
};

// The class defines the API for generators written in lua and presented in bweas packages
class lua_generator : public base_generator {
    friend base_generator *base_generator::createGeneratorLua(std::string);

  private:
    lua_generator(std::string);
    ~lua_generator() = default;

  public:
    void init() override final;
    void deleteGenerator() override final;
    std::map<std::string, std::vector<std::string>> input_files(const var::struct_sb::target_out &,
                                                                const bwqueue_templates &, std::string) override final;
    std::map<std::string, std::string> gen_commands(const var::struct_sb::target_out &, bwqueue_templates &,
                                                    std::string,
                                                    std::map<std::string, std::vector<std::string>>) override final;

  private:
    static inline bool init_glob_gnlua{0};

    bwlua::lua lua;
};

base_generator *base_generator::createGeneratorInt(func_generator generator, func_get_files_input files_input) {
    return (base_generator *)new integral_generator(generator, files_input);
}
base_generator *base_generator::createGeneratorLua(std::string src_lua) {
    return (base_generator *)new lua_generator(src_lua);
}
} // namespace generator_api

} // namespace bweas

#endif