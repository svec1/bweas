#ifndef INTERPRETER__H
#define INTERPRETER__H

#include "semantic_an.hpp"
#include "../smodule/module.hpp"

namespace interpreter{

    #define MAIN_FILE "bweasconf.txt"
    #define IMPORT_FILE "import-modules.imp"
    
    #define wrap_interpreter(action) try{\
                                        action\
                                     }\
                                     catch(std::exception& excp){\
                                        assist.call_err("RTT000", excp.what());\
                                     }

    #define wrap_callf_interpreter(action, debug) try{\
                                                    action \
                                                    } \
                                                catch(realtime_excp& excp){ \
                                                    debug \
                                                    assist.call_err(excp.get_assist_err(), excp.what()); \
                                                }
    #define debug_mark_time_func(func, func_name) beg = clock(); \
                                       func; \
                                       assist << std::string("Run time " func_name ": " + std::to_string((double)(clock() - beg) / CLOCKS_PER_SEC));

    class realtime_excp : public bw_excp::bweas_exception{
        public:
            realtime_excp(std::string _what_hp, std::string number_err)
                : what_hp(_what_hp), bweas_exception("RTT"+number_err){}
            ~realtime_excp() noexcept override final = default;
        public:
            const char* what() const noexcept override final{
                return what_hp.c_str();
            }

        private:
            std::string what_hp;
    };

    class interpreter_exec : private module::mdl_manager{
        public:
            struct config{
                // output:
                // - tree after semanticanalyzer
                // - run time every of components interpreter(lexer, parser...)
                bool debug_output{0};

                bool analyzer{0};
                bool optimyzer{0};
                
                // construction of dynamic post-semantic command module(DPCM)
                bool CDPCM{0};

                // whether to import modules
                bool import_module{0};

                // whether to use external scope
                bool use_external_scope{0};

                // whether to pass the list of external functions that 
                // should be called in semantic analysis to semantic analysis
                bool transmit_smt_name_func_with_smt{0};

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
            // Compiles an abstract expression of function
            void build_aef();

            // Interpretation of functions (not declaring, and which are not 
            // explicitly marked as called in the semantic analysis)
            void interpreter_run();

            void set_config(config conf);
            void set_external_scope(var::scope* _external_scope);
        private:
            void build_external_func_table();

        private:
            static inline bool init_glob{0};
            
            config interp_conf;

            var::scope global_scope;
            var::scope* external_scope{&global_scope};

            lexer::lex_an lexer;
            parser::pars_an parser;
            semantic_an::semantic_analyzer smt_analyzer;

            parser::abstract_expr_func aef;
            semantic_an::table_func external_func_table;

            u32t curr_expr{0};
    };
}

#endif