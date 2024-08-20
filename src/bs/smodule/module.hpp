#ifndef MODULE__H
#define MODULE__H

#include "../lang/semantic_an.hpp"
#include "srl/srl_importm.hpp"

namespace module {

// this class manages module loads, and must exist with the interpreter class
class mdl_manager {
  protected:
    mdl_manager() = default;

    mdl_manager(mdl_manager &&) = delete;
    mdl_manager(const mdl_manager &) = delete;
    mdl_manager &
    operator=(mdl_manager &&) = delete;

    ~mdl_manager() = default;

  protected:
    void
    import_module_decl(const char *import_file);
    const std::vector<std::string> &
    get_func_with_smt();

    semantic_an::table_func
    load_module(std::string name);
    semantic_an::table_func
    load_modules(const std::vector<std::string> &load_module_name);
    semantic_an::table_func
    load_modules_all();

  private:
    srl::modules mdls;
    std::vector<std::string> func_with_smt;
};

} // namespace module

#endif