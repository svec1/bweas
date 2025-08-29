%{

#include <string>

#include <cstdio>
#include <cstdlib>
#include <string.h>

#include <lang/scope.hpp>

#include "parser.hpp"

#define YY_USER_ACTION \
    { \
        int yytext_len = strlen(yytext); \
        if(yytext[yytext_len-1] != ' ' && yytext[yytext_len-1] != '\t' && yytext[yytext_len-1] != '\n' && yytext[yytext_len-1] != '\r') \
            current_statements_str += yytext; \
    } \
    yylloc.first_line = yylloc.last_line; \
    yylloc.first_column = yylloc.last_column; \
    for(int i = 0; i < yytext[i] != '\0'; ++i) \
        if(yytext[i] == '\n') { \
            yylloc.last_line++; \
            yylloc.last_column = 0; \
        } \
        else \
            yylloc.last_column++; 

extern string current_statements_str;
YY_BUFFER_STATE buffer;
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
                                string str_tmp = yytext;
                                str_tmp.erase(0, 1).erase(str_tmp.size()-1, 1);
                                yylval.string = strdup(str_tmp.data()); 
                                return STRING;
                              }

[a-zA-Z_][a-zA-Z0-9_]+        { 
                                yylval.string = strdup(yytext);
                                if(!strcmp(yylval.string, "import"))
                                    return IMPORT;
                                return ID;
                              }

\{[a-zA-Z_][a-zA-Z0-9_]+\}    {
                                string str_tmp = yytext;
                                str_tmp.erase(0, 1);
                                str_tmp.erase(str_tmp.size()-1, 1); 
                                yylval.string = strdup(str_tmp.data());
                                return VALUE_BY_ID;
                              }


.                 { 
                    printf("Undefined token: %s\n", yytext); 
                    yyterminate();
                  }

%%

void init_buffer(string_v src){
    buffer = yy_scan_string(src.data());
}

void delete_buffer(){
    yy_delete_buffer(buffer);
}

