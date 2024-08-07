#ifndef _STATIC_STRUCT__H
#define _STATIC_STRUCT__H

#include <string>
#include "../../mdef.hpp"

namespace var{
    namespace struct_sb{

        enum class language{
            c = 0,
            cpp,
            csharp,
            asm_
        };
        enum class type_target{
            exe = 0,
            lib,
            interpret
        };
        enum class configuration{
            RELEASE,
            DEBUG
        };

        struct version{
            version() = default;
            version(u32t mj, u32t mn, u32t ptch)
                : major{mj}, minor{mn}, patch{ptch}{}
            
            std::string get_str_version(){
                return std::to_string(major) + "." +
                       std::to_string(minor) + "." +
                       std::to_string(patch);
            }

            u32t major{0};
            u32t minor{0};
            u32t patch{0};
        };

        struct project{
            project() = default;
            std::string name_project;
            version version_project;

            language lang;

            std::string path_compiler, path_linker;
            std::string rflags_compiler, rflags_linker;
            std::string dflags_compiler, dflags_linker;
            u32t standart_c{98}, standart_cpp{14};

            bool use_it_templates = 0;

            std::vector<std::string> src_files;
            std::vector<std::string> vec_templates;
        };

        struct target{
            target() = default;
            std::shared_ptr<project> prj;
            type_target target_t;
            configuration target_cfg;
            
            std::string name_target;
            version version_target;

            std::vector<std::string> target_vec_libs;
        };
    }
}

#endif