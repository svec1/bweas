#ifndef _CONF_VAR__H
#define _CONF_VAR__H

namespace config_var{
    extern const char* version;

    extern bool build;
    extern bool logging;
    extern bool warning_prj;
    extern bool optimization_script;

    extern bool export_var;

    extern const char* logfile_name;

    namespace basic_project{
        namespace configuration{
            namespace release{
                
            }
            namespace debug{

            }
        }
    }

}

#endif