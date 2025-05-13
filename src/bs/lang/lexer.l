%{

#include <string>

#include <cstdio>
#include <cstdlib>
#include <string.h>

#include <bwtype.h>
#include <lang/scope.hpp>

#include "parser.hpp"

#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line; \
    yylloc.first_column = yylloc.last_column; \
    for(int i = 0; yytext[i] != '\0'; ++i) \
        if(yytext[i] == '\n') { \
            yylloc.last_line++; \
            yylloc.last_column = 0; \
        } \
        else \
            yylloc.last_column++; 

extern var::scope* current_scope;

%}

%option noyywrap
%option yylineno 

%%

[ \t\n\r]+           { ; } 

"("                { return OPEN_BR;  }
")"                { return CLOSE_BR; }
","                { return COMMA;    }
"+"                { return PLUS;     }
"-"                { return MINUS;    }
"*"                { return MUL;      }
"/"                { return DIV;      }

[0-9]+             { yylval.number = std::stoi(yytext); return NUMBER; }

\"[^"\\]+\"                   { 
                                std::string str_tmp = yytext;
                                str_tmp.erase(0, 1).erase(str_tmp.size()-1, 1);
                                yylval.string = strdup(str_tmp.data()); 
                                return STRING;
                              }
[a-zA-Z_][a-zA-Z0-9_]+        { 
                                std::string str_tmp = yytext;

                                if(str_tmp.find("VAR_") == 0){
                                    str_tmp.erase(0, 4);
                                    yylval.string = strdup(str_tmp.data());
                                    return ID;
                                }
                                else{
                                    u32t type = current_scope->what_type(str_tmp); 
                                    if(type == 1){
                                        yylval.number = current_scope->get_var_value<i32t>(str_tmp);
                                        return NUMBER;
                                    }
                                    else if(type == 2){
                                        yylval.string = strdup(current_scope->get_var_value<std::string>(str_tmp).data()); 
                                        return STRING;    
                                    }       
                                
                                    yylval.string = strdup(str_tmp.data());
                                    return ID;
                                }
                              }

.                 { 
                    printf("Undefined token: %s\n", yytext); 
                    yyterminate();
                  }

%%
