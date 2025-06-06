%{

#include <cstdio>
#include <cstring>

#include <lang/scope.hpp>

#define YYLLOC_UPDATE_GLOBAL_LOC(loc) last_line = loc.last_line; last_column = loc.last_column; 

bweas::logger *log_bison;
var::scope* current_scope;  

statements               stm_s;
expressions              current_params; // Current statement expressions 
expression::expression_t expr_t_tmp;     // Current expression type 

int last_line = 0, last_column = 0;

extern int yylex(void);
void yyerror(const char* str){
     printf("Error: %s\n", str);
}

void init_param(char* value){
    current_params.emplace_back(value, expr_t_tmp, last_line, last_column);
    free(value);
}

void init_statement(const decl_func* dfunc){
    stm_s.emplace_back(dfunc, current_params, last_line, last_column); 
    current_params.clear();
}

string get_current_loc(){
    return "[" +  std::to_string(last_line) + ":" + std::to_string(last_column) + "]";   
}
%}

%locations

%union {
    pdiff number;
    char* string;
}

%token OPEN_BR
%token CLOSE_BR
%token COMMA
%token PLUS MINUS MUL DIV

%token NUMBER STRING ID

%left PLUS MINUS 
%left MUL DIV 

%type<number> NUMBER num_term num_expr 
%type<string> STRING ID str_term str_expr param 

%%

statement: 
         | statement ID OPEN_BR params CLOSE_BR {   //printf("---Statement %s(%s,%s,%s)---\n", $2, current_params[0].value.c_str(), current_params[1].value.c_str(), current_params[2].value.c_str());
                                                    if(current_scope->what_type($2) != 10){
                                                        (*log_bison) << bwtools::error << (log_message(log_type::error) << "A variable is expected which is a reference to the function: " << $2); 
                                                        YYERROR;        
                                                    }
                                                    init_statement(&current_scope->get_var_value<decl_func>($2));  
                                                }   
         | error {
                    (*log_bison) << bwtools::fatal << (log_message(log_type::fatal) << "[" << @1.last_line << ":" << @1.last_column << "]: syntax error: Invalid statement"); 
                    YYABORT;
                 }
;

params: param              { 
                             YYLLOC_UPDATE_GLOBAL_LOC(@$) 
                             init_param($1); 
                           }
      | params COMMA param { 
                             YYLLOC_UPDATE_GLOBAL_LOC(@$)
                             init_param($3); 
                           } 
;
param: ID       { 
                    $$ = $1; 
                    expr_t_tmp = expression::expression_t::ID; 
                }
     | num_expr { 
                    $$ = (char*)malloc(sizeof(char)*10);
                    sprintf($$, "%d", $1);
                    expr_t_tmp = expression::expression_t::NUMBER; 
                } 
     | str_expr { 
                    $$ = $1;
                    expr_t_tmp = expression::expression_t::STRING;
                }
     | error    { 
                    (*log_bison) << bwtools::fatal << (log_message(log_type::fatal) << "[" << @1.last_line << ":" << @1.last_column << "]: syntax error: Invalid parameter definition"); 
                    YYABORT;
                }
;

num_expr: OPEN_BR num_expr CLOSE_BR          { $$ = $2; }
         | num_expr PLUS num_expr            { $$ = $1 + $3; }
         | num_expr MINUS num_expr           { $$ = $1 - $3; }
         | num_expr MUL num_expr             { $$ = $1 * $3; }
         | num_expr DIV num_expr             { 
                                                if($3 == 0){ 
                                                    (*log_bison) << bwtools::fatal << (log_message(log_type::fatal) << "Division by zero"); 
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

