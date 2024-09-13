#ifndef _BUILD_SYS__H
#define _BUILD_SYS__H

#include "../mdef.hpp"

#include "console_arg/hand_arg.hpp"
#include "lang/interpreter.hpp"
#include "tools/call_cmd.hpp"

class bwbuilder {
  public:
    bwbuilder() = default;

    bwbuilder(bwbuilder &&) = delete;
    bwbuilder(const bwbuilder &) = delete;
    bwbuilder &
    operator=(bwbuilder &&) = delete;

    ~bwbuilder();

  public:
    void
    run_interpreter();

    void
    gen_cache_target();

    void
    build_on_cache();

  private:
    interpreter::interpreter_exec _interpreter;
    var::struct_sb::version bwbuilde_ver;
};

#endif