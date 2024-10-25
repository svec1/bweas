#ifndef _BUILD_SYS__H
#define _BUILD_SYS__H

#include "../mdef.hpp"
#include "conf_var.hpp"

#include "console_arg/hand_arg.hpp"
#include "lang/interpreter.hpp"
#include "tools/bwfile.hpp"
#include "tools/call_cmd.hpp"
#include "tools/exception.hpp"

#include <stack>

#define MAIN_FILE "bweasconf.txt"
#define IMPORT_FILE "import-modules.imp"
#define CACHE_FILE "bwcache"

#define FEATURE_FIELD_BS_CURRENT_IF "FBS_CURRENT_INPUT_FILE"
#define FEATURE_FIELD_BS_CURRENT_OF "FBS_CURRENT_OUTPUT_FILE"

class bwbuilder_excp : public bw_excp::bweas_exception {
  public:
    bwbuilder_excp(std::string _what_hp, std::string number_err)
        : what_hp(_what_hp), bweas_exception("BWS" + number_err) {
    }
    ~bwbuilder_excp() noexcept override final = default;

  public:
    const char *
    what() const noexcept override final {
        return what_hp.c_str();
    }

  private:
    std::string what_hp;
};

class bwbuilder {
  public:
    bwbuilder();

    bwbuilder(bwbuilder &&) = delete;
    bwbuilder(const bwbuilder &) = delete;
    bwbuilder &
    operator=(bwbuilder &&) = delete;

    ~bwbuilder() = default;

  public:
    void
    start_build();

  protected:
    void
    run_interpreter();

    u32t
    gen_cache_target();

    void
    gen_DPCM();

    void
    build_on_aef();

    void
    build_on_cache();

  public:
    void
    switch_log(u32t value);
    void
    switch_output_log(u32t value);

  private:
    void
    build_targets();

    u32t
    deserl_cache();

    void
    load_target();

    void
    imp_data_interpreter_for_bs();

  private:
    std::stack<std::string>
    create_stack_target_templates(const var::struct_sb::target_out &target);
    u32t
    recovery_stack_templates(std::vector<var::struct_sb::template_command> &vec_templates,
                             const std::string &name_internal_param, std::stack<std::string> &stack_templates);

  private:
    std::string
    get_file_w_index(std::string pattern_file, u32t index);

  private:
    interpreter::interpreter_exec _interpreter;

    std::vector<var::struct_sb::target_out> out_targets;
    std::vector<var::struct_sb::template_command> templates;
    std::vector<var::struct_sb::call_component> call_components;
    std::vector<std::pair<std::string, std::string>> global_extern_args;

    static inline bool init_glob{0};

    var::struct_sb::version bwbuilde_ver{"0.0.1"};
    bool log{1}, output_log{1};
};

#endif