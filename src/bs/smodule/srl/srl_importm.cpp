#include "srl_importm.hpp"

#define call_err_tk_inf_lex(count_line, count_sym, lexem, expected, err) assist.call_err(err, \
                            "\n(Line: " + std::to_string(count_line) + "; Symbols start pos: " + \
                            std::to_string(count_sym-lexem.size()) + "): Lexem - \"" + lexem + "\"" + \
                            "\n Expected: " + expected + "\n");
    
#define call_err_identify_expected() if(expected_close_ang_br && expected_start_module) call_err_tk_inf_lex(count_line, count_sym, lexem, "<start_module>", "SRL-IMP000") \
                                     else if(expected_close_ang_br && expected_end_module) call_err_tk_inf_lex(count_line, count_sym, lexem, "<end_module>", "SRL-IMP000") \
                                     else if(expected_qouts && expected_enum_params) call_err_tk_inf_lex(count_line, count_sym, lexem, "SYMBOL \'\"\'", "SRL-IMP000") \
                                     else if(expected_separator && expected_filename_dll) call_err_tk_inf_lex(count_line, count_sym, lexem, "SYMBOL \':\'", "SRL-IMP000") \
                                     else if(expected_separator && expected_func_name) call_err_tk_inf_lex(count_line, count_sym, lexem, "OPERATOR \'->\'", "SRL-IMP000") \
                                     else if(expected_separator && expected_enum_params) call_err_tk_inf_lex(count_line, count_sym, lexem, "OPERATOR \',\'", "SRL-IMP000") \
                                     else if(expected_filename_dll) call_err_tk_inf_lex(count_line, count_sym, lexem, "filename.dll", "SRL-IMP000") \
                                     else if(expected_func_name) call_err_tk_inf_lex(count_line, count_sym, lexem, "func_name: ", "SRL-IMP000") \
                                     else if(expected_enum_params) call_err_tk_inf_lex(count_line, count_sym, lexem, "PARAM[INT,STRING...]", "SRL-IMP000") \
                                     else if(expected_open_br) call_err_tk_inf_lex(count_line, count_sym, lexem, "SYMBOL \'(\'", "SRL-IMP000") \
                                     else if(expected_close_br) call_err_tk_inf_lex(count_line, count_sym, lexem, "SYMBOL \')\'", "SRL-IMP000")
                 

static bool init_glob{0};

void srl::init_err(){
    if(!init_glob){
        assist.add_err("SRL-IMP000", "The lexeme does not obey the rules of the grammar of the __import__ scripting language");
        assist.add_err("SRL-IMP001", "The passed function parameters are not part of the expected ones");
        init_glob = 1;
    }
}

char get(std::string sym_s, u32t& pos){
    if(pos >= sym_s.size()) return '\0';
    return sym_s[pos++];
}

std::pair<std::vector<srl::token>, std::vector<std::string>> srl::parser(std::string symbols){
    bool expected_close_ang_br = 0;
    bool expected_start_module = 1, expected_end_module = 0;
    bool expected_filename_dll = 0;
    bool expected_func_name = 0;
    bool expected_enum_params = 0;
    bool expected_qouts = 0;
    bool expected_separator = 0;
    bool expected_open_br = 0, expected_close_br = 0;
    
    u32t pos = 0;
    u32t count_line = 0, count_sym = 0;

    std::vector<srl::token> tokens;
    std::string lexem;

    std::string tmp_name_func;
    std::vector<std::string> name_func_smt;

    char ch;
    while ((ch = get(symbols, pos)))
    {
        ++count_sym;
        if(ch == ' ' || ch == '\n' || ch == '\r') {
            while (ch == ' ' || ch == '\n' || ch == '\r'){
                if(ch == '\n'){
                    ++count_line;
                    count_sym = 1;
                }
                else if(ch == '\0') break;
                ch = get(symbols, pos);
            }
            if((expected_close_ang_br && !lexem.empty() && symbols[pos-1] != '>') || (!expected_close_ang_br && !lexem.empty() && !expected_qouts) )
                expected_separator = 1;
        }
        if(ch == '>'){
            if(!expected_close_ang_br && !expected_func_name)
                call_err_identify_expected()
            expected_close_ang_br = 0;
            if(lexem == "start_module"){
                if(!expected_start_module)
                    call_err_identify_expected()
                expected_start_module = 0;
                expected_filename_dll = 1;
                tokens.push_back(token(lexem, count_line, count_sym, srl::token_type::START_MODULE));
            }
            else if(lexem == "end_module"){
                if(!expected_end_module || expected_start_module)
                    call_err_identify_expected()
                expected_start_module = 1;
                tokens.push_back(token(lexem, count_line, count_sym, srl::token_type::END_MODULE));
            }
            else
                call_err_identify_expected()

            lexem.clear();

            continue;
        }
        else if(ch == '\"'){
            if(!expected_filename_dll)
                call_err_identify_expected()
            
            if(!expected_qouts)
                expected_qouts = 1;
            else
                expected_qouts = 0;
            continue;
        }
        else if(!expected_qouts){
            if(ch == ':'){
                if(!expected_filename_dll || lexem.empty())
                    call_err_identify_expected()
                expected_filename_dll = 0;
                expected_separator = 0;
                tokens.push_back(token(lexem, count_line, count_sym, srl::token_type::NAME_MODULE));
            
                expected_func_name = 1;

                lexem.clear();

                continue;
            }
            else if(ch == '-' && get(symbols, pos) == '>'){
                if(!expected_func_name || lexem.empty())
                    call_err_identify_expected()
                expected_func_name = 0;
                expected_separator = 0;
                tokens.push_back(token(lexem, count_line, count_sym, srl::token_type::NAME_FUNC_OF_MODULE));
                expected_enum_params = 1;
                tmp_name_func = lexem;

                lexem.clear();

                continue;
            }
            else if(ch == ','){
                if(!expected_enum_params || lexem.empty())
                    call_err_identify_expected()
                expected_enum_params = 0;
                expected_separator = 0;
                tokens.push_back(token(lexem, count_line, count_sym, srl::token_type::PARAM, parser::type_param_in_str(lexem)));
                expected_enum_params = 1;
                lexem.clear();

                continue;
            }
            else if(ch == ';'){
                if(!expected_enum_params || lexem.empty())
                    call_err_identify_expected()
                expected_enum_params = 0;
                expected_separator = 0;
                tokens.push_back(token(lexem, count_line, count_sym, srl::token_type::PARAM, parser::type_param_in_str(lexem)));
                expected_open_br = 1;
                lexem.clear();

                continue;
            }
            else if(ch == '<'){
                if (expected_func_name){
                    expected_end_module = 1;
                    expected_func_name = 0;
                }
                expected_close_ang_br = 1;
                continue;
            }
            else if(ch == '('){
                if(!expected_open_br || !lexem.empty())
                    call_err_identify_expected()
                expected_open_br = 0;
                expected_close_br = 1;

                continue;
            }
            else if(ch == ')'){
                if(!expected_close_br || lexem.empty())
                    call_err_identify_expected()
                expected_close_br = 0;
                if(lexem == "SMT")
                    name_func_smt.push_back(tmp_name_func);
                else if(lexem != "INTP")
                    call_err_tk_inf_lex(count_line, count_sym, lexem, "expression - [SMT] or [INTP]", "SRL-IMP000")
                expected_func_name = 1;
                
                lexem.clear();
                tmp_name_func.clear();

                continue;
            }
        }
        
        if(expected_separator)
            call_err_identify_expected()
        lexem += ch;
    }

    if (expected_func_name){
        expected_end_module = 1;
        call_err_identify_expected()
    }

    return std::pair<std::vector<srl::token>, std::vector<std::string>>(tokens, name_func_smt);
}
srl::modules srl::build_modules(std::vector<srl::token> tokens){
    modules mdls;
    module tmp_mdl;

    std::string filename_dll;
    funcs_module funcs;
    
    func_define tmp_func;
    std::string tmp_name_func;

    for(u32t i = 1; i < tokens.size(); ++i){
        if(tokens[i].token_t == token_type::NAME_MODULE){
            if(tokens[i].val.find(".dll") != tokens[i].val.npos)
                tokens[i].val.erase(tokens[i].val.find(".dll"), 4);
            filename_dll = tokens[i].val;
        }
        else if(tokens[i].token_t == token_type::NAME_FUNC_OF_MODULE){
            if(tmp_func.size()){
                funcs.emplace_back(tmp_func);
                tmp_func.clear();
                tmp_name_func.clear();
            }

            // save name of function
            tmp_name_func = tokens[i].val;

            // creating a function declaration
            tmp_func[tmp_name_func];
        }
        else if(tokens[i].token_t == token_type::PARAM){
            if(tokens[i].token_p == parser::params::SIZE_ENUM_PARAMS)
                call_err_tk_inf_lex(tokens[i].lines,
                                    tokens[i].symbols,
                                    tokens[i].val,
                                    "params -> [LIT_NUM, LIT_STR, VAR_ID, "
                                    "FUTURE_VAR_ID, VAR_STRUCT_ID, "
                                    "ANY_VALUE_WITHOUT_FUTUREID_NEXT, "
                                    "LNUM_OR_ID_VAR, LSTR_OR_ID_VAR, "
                                    "NEXT_TOO]", "SRL-IMP001")
            tmp_func[tmp_name_func].push_back(tokens[i].token_p);
        }
        else if(tokens[i].token_t == token_type::END_MODULE){
            // add function to vector of functions
            funcs.emplace_back(tmp_func);

            // creating a function declaration in dll to module
            tmp_mdl.emplace(filename_dll, funcs);
            mdls.push_back(tmp_mdl);

            tmp_mdl.clear();
            funcs.clear();
            tmp_func.clear();
            tmp_name_func.clear();
        }
    }
    return mdls;
}