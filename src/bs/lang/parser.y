%{

#include <cstdio>
#include <cstring>

#if defined(WIN)
#include <io.h>
#endif

#include <lang/scope.hpp>

#define YYLLOC_UPDATE_GLOBAL_LOC(loc) last_line = loc.last_line; last_column = loc.last_column; 

using namespace bweas;

logger *log_bison;
scope* current_scope;  

statements               stm_s;
expressions              current_params; // Current statement expressions 
expression::expression_t expr_t_tmp;     // Current expression type 

string current_statements_str;
int last_line = 0, last_column = 0;

bool current_vbi_param = false; // value by id

extern int yylex(void);
void yyerror(const char* str){
     printf("Error: %s\n", str);
}

void init_param(char* value){
    current_params.emplace_back(value, expr_t_tmp, current_vbi_param, last_line, last_column);
    
    free(value);
    current_vbi_param = false;
}

void init_statement(const decl_func* dfunc){
    stm_s.emplace_back(dfunc, current_params, last_line, last_column); 
    stm_s[stm_s.size()-1].view_str = current_statements_str;

    current_params.clear();
    current_statements_str.clear();
}

string get_current_loc(){
    return "[" +  std::to_string(last_line) + ":" + std::to_string(last_column) + "]";   
}
%}

%locations

%union {
    pdiff number;
    char* string;
    
    bool value_by_id;
}

%token OPEN_BR
%token CLOSE_BR
%token COMMA
%token PLUS MINUS MUL DIV

%token NUMBER STRING ID
%token VALUE_BY_ID

%left PLUS MINUS 
%left MUL DIV 

%type<number> NUMBER num_term num_expr 
%type<string> STRING ID VALUE_BY_ID str_term str_expr param 

%%

statement: 
         | statement ID OPEN_BR params CLOSE_BR {   
                                                    if(current_scope->what_type($2) != 10){
                                                        (*log_bison) << (log_message(log_type::error) << "A variable is expected which is a reference to the function: " << $2); 
                                                        YYERROR;        
                                                    }
                                                    init_statement(&current_scope->get_var_value<decl_func>($2));  
                                                }   
         | error {
                    (*log_bison) << (log_message(log_type::fatal) << "[" << @1.last_line << ":" << @1.last_column << "]: syntax error: Invalid statement"); 
                    YYABORT;
                 }
;

params:
      | param              { 
                             YYLLOC_UPDATE_GLOBAL_LOC(@$) 
                             init_param($1); 
                           }
      | params COMMA param { 
                             YYLLOC_UPDATE_GLOBAL_LOC(@$)
                             init_param($3); 
                           } 
;
param: ID           { 
                        $$ = $1; 
                        expr_t_tmp = expression::expression_t::ID; 
                    }
     | VALUE_BY_ID  {
                        $$ = $1; 
                        expr_t_tmp = expression::expression_t::ID;
                        current_vbi_param = true; 
                    }
     | num_expr     { 
                        $$ = (char*)malloc(sizeof(char)*10);
                        sprintf($$, "%td", $1);
                        expr_t_tmp = expression::expression_t::NUMBER; 
                    } 
     | str_expr     { 
                        $$ = $1;
                        expr_t_tmp = expression::expression_t::STRING;
                    }
     | error        { 
                        (*log_bison) << (log_message(log_type::fatal) << "[" << @1.last_line << ":" << @1.last_column << "]: syntax error: Invalid parameter definition"); 
                        YYABORT;
                    }
;

num_expr: OPEN_BR num_expr CLOSE_BR          { $$ = $2; }
         | num_expr PLUS num_expr            { $$ = $1 + $3; }
         | num_expr MINUS num_expr           { $$ = $1 - $3; }
         | num_expr MUL num_expr             { $$ = $1 * $3; }
         | num_expr DIV num_expr             { 
                                                if($3 == 0){ 
                                                    (*log_bison) << (log_message(log_type::fatal) << "Division by zero"); 
                                                    YYERROR;
                                                }                                                
                                                $$ = $1 / $3; 
                                             }
         | num_term                          { $$ = $1; }
;

str_expr: str_expr PLUS str_term             { 
                                                $$ = strdup((string($1)+ $3).data());
                                                free($1);
                                                free($3);
                                             }
        | str_term                           { $$ = $1; }
;

num_term: NUMBER                             { $$ = $1; }
;

str_term: STRING                             { $$ = $1; }
;

%%

