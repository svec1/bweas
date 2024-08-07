#ifndef INTERPRETER__H
#define INTERPRETER__H

#include "semantic_an.hpp"
#include "../smodule/module.hpp"

namespace interpreter{

    #define MAIN_FILE "bweasconf.txt"
    #define IMPORT_FILE "import-modules.imp"

    #define wrap_callf_interpreter(action, debug) try{\
                                                    action \
                                                    } \
                                                catch(realtime_excp& what){ \
                                                    debug \
                                                    assist.call_err(what.rtt_err(), what.what()); \
                                                }
    
    #define debug_mark_time_func(func, func_name) beg = clock(); \
                                       func; \
                                       assist << std::string("Run time " func_name ": " + std::to_string((double)(clock() - beg) / CLOCKS_PER_SEC));

    class realtime_excp : std::exception{
        public:
            realtime_excp(std::string _rtt_err_name, std::string _what_hp) noexcept 
                : rtt_err_name(_rtt_err_name), what_hp(_what_hp){}
            ~realtime_excp() noexcept override = default;
        
        public:
            const char* rtt_err() const noexcept{
                return rtt_err_name.c_str();
            }
            const char* what() const noexcept override{
                return what_hp.c_str();
            }
        
        private:
            std::string rtt_err_name;
            std::string what_hp;
    };

    class interpreter_exec : private module::mdl_manager{
        public:
            struct config{
                // output:
                // - tree after semanticanalyzer
                // - run time every of components interpreter(lexer, parser...)
                bool debug_output{0};

                bool mode_tas{0};
                bool analysis{0};
                
                // construction of dynamic post-semantic command module(DPCM)
                bool CDPCM{0};

                const char* filename_interp{MAIN_FILE};
                const char* file_import_file_f{IMPORT_FILE};
            };

        public:
            interpreter_exec();
            interpreter_exec(config conf);
            
            interpreter_exec(interpreter_exec&&) = delete;
            interpreter_exec(const interpreter_exec&) = delete;
            interpreter_exec& operator=(interpreter_exec&&) = delete;

            ~interpreter_exec() = default;

        public:
            void build_aef();

            void interpreter_run();
            void debug_run();

            void set_config(config conf);
        private:
            void build_external_func_table();

            void interpreter_next_expr();
        private:
            static inline bool init_glob{0};
            
            config interp_conf;

            var::scope global_scope;

            lexer::lex_an lexer;
            parser::pars_an parser;
            semantic_an::semantic_analyzer smt_analyzer;

            parser::abstract_expr_func aef;
            semantic_an::table_func external_func_table;

            u32t curr_expr{0};
    };
}

#endif