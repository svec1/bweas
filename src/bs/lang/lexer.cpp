#include "../../mdef.hpp"

#include "lexer.hpp"

using namespace lexer;

lex_an::lex_an(){
    if(!init_glob){
        init_glob = 1;
        assist.add_err("LEX000", "The lexeme does not obey the rules of the grammar of the scripting language");
    }
}

lex_an::lex_an(const std::string& _symbols){
    lex_an();
    set_symbols(_symbols);
}

char lex_an::get(){
    if(pos >= symbols.size()) return '\0';
    return symbols[pos++];
}

bool lex_an::check_sym_valid_grammar(char ch){
    if(ch == '*' || ch == '/' || ch == '!' || ch == '-' || ch == '\'' || ch == '\\') return 0;
    return 1;
}

void lex_an::set_symbols(const std::string& _symbols){
    symbols = _symbols;
}
std::vector<token> lex_an::get_tokens(){
    return tokens;
}
void lex_an::clear_tokens(){
    tokens.clear();
}
std::vector<token> lex_an::analysis(){
    std::string lexem;
    token tmp_curr_token;

    bool literal = 0, num = 0, br = 0;
    bool err_lexem = 0;

    bool expected_separate = 0;

    bool was_br = 0;

    bool lexer_option = 0;

    u32t count_line = 1;
    u32t count_sym = 0;

    pos = 0;

    char ch;
    while(ch = get()){
        ++count_sym;
        if((ch == ' ' || ch == '\n' || ch == '\r' | ch == '\t') && !br){
            if(lexer_option && ch == '\r'){
                if(lexem.empty())
                    assist.call_err("LEX000", "\n(Line: " + std::to_string(count_line) + "; Symbols start pos: " + std::to_string(count_sym-lexem.size()) + "): Lexem - \"" + lexem + "\"\n");
                if(lexem == "lexer_stop") break;
                else
                    assist.call_err("LEX000", "\n(Line: " + std::to_string(count_line) + "; Symbols start pos: " + std::to_string(count_sym-lexem.size()) + "): Lexem - \"" + lexem + "\"\n");
            }
            while (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') {
                if (ch == '\n') {
                    ++count_line;
                    count_sym = 1;

                }
                else if (ch == '\0') break;
                ch = get();
            }
            if(!lexem.empty())
                expected_separate = 1;
        }
        
        if(ch == '\"'){
            if(!br) br = 1;
            else{
                br = 0;
                was_br = 1;
            }
            continue;
        }
        else if(ch == '@'){
            if(!lexem.empty() || lexer_option)
                assist.call_err("LEX000", "\n(Line: " + std::to_string(count_line) + "; Symbols start pos: " + std::to_string(count_sym-lexem.size()) + "): Lexem - \"" + lexem + "\"\n");

            lexer_option = 1;
            continue;
        }
        else if((ch == '(' || ch == ')' || ch == ',' || ch == '>' || ch == '<' || ch == '=' || ch == '+') && !br){
            if(err_lexem)
                assist.call_err("LEX000", "\n(Line: " + std::to_string(count_line) + "; Symbols start pos: " + std::to_string(count_sym-lexem.size()) + "): Lexem - \"" + lexem + "\"\n");
            
            tmp_curr_token.pos_beg_defined_sym = count_sym-lexem.size();
            tmp_curr_token.pos_defined_line = count_line;
            if(!lexem.empty()){
                tmp_curr_token.token_val = lexem;
                if(lexem == "if" || lexem == "else"){
                    tmp_curr_token.token_t = token_type::KEYWORD;
                    tokens.push_back(tmp_curr_token);
                }
                else if(literal){
                    if(was_br){
                        tmp_curr_token.token_t = token_type::LITERALS;
                        tokens.push_back(tmp_curr_token);
                        was_br = 0;
                    }
                    else{
                        if(std::atoll(tmp_curr_token.token_val.c_str()) > INT32_MAX)
                            assist.call_err("LEX000", "\n(Line: " + std::to_string(count_line) + "; Symbols start pos: " + std::to_string(count_sym-lexem.size()) + "): Lexem - \"" + lexem + "\"\n");
                        tmp_curr_token.token_t = token_type::LITERAL;
                        tokens.push_back(tmp_curr_token);
                    }
                }
                else{
                    tmp_curr_token.token_t = token_type::ID;
                    tokens.push_back(tmp_curr_token);
                }
                
                tmp_curr_token.token_val.clear();
                lexem.clear();
            }
            if(ch == '>' || ch == '<' || ch == '=' || ch == '+'){
                tmp_curr_token.token_t = token_type::OPERATOR;
                tmp_curr_token.token_val += ch;
                tokens.push_back(tmp_curr_token);
                tmp_curr_token.token_val.clear();
            }
            else if(ch == '(' || ch == ')' || ch == ','){
                if(ch == '(')
                    tmp_curr_token.token_t = token_type::OPEN_BR;
                else if(ch == ')')
                    tmp_curr_token.token_t = token_type::CLOSE_BR;
                else if(ch == ',')
                    tmp_curr_token.token_t = token_type::COMMA;
                tokens.push_back(tmp_curr_token);
            }
            num = 0, literal = 0;
            expected_separate = 0;

            continue;
        }
        if(lexem.empty() && (isdigit(ch) || br)) {
            literal = 1;
            num = 1;
        }
        if((num && !isdigit(ch) && !br) || (!check_sym_valid_grammar(ch) && !br) || was_br || expected_separate)
            err_lexem = 1;
        lexem += ch;
    }
    return tokens;
}