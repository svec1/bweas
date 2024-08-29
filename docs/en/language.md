
#  LANGUAGE
**Bweas-lang**  is a configuration  language,  it is cross-platform  (like  the  build  system  itself),  however, it may  have a different  list of statically  global  functions  depending  on the platform. 

This  language  has  its own syntactic  rules  and  its own interpreter.  It  is  somewhat  similar  to  cmake  (this  build  system was originally  inspired by it),  but  this is not the case, the interpreter of this  language  has a very  similar  structure to the  interpreters of programming  languages  (most  likely),  since it contains  lexical,  syntactic  and  semantic  analysis  along  with the interpreter.

This  language  supports  modules  (dynamic  libraries)  that  can  provide  additional  functions  (which  can  also  be  called).  This  support  includes  a  ban  on  certain  system  functions  (depending  on the platform),  which  may  expose the person  who  writes  his  module to certain  **vulnerabilities**:

#### For  Windows:  WinAPI  function  and  others
- CreateProcessW, CreateProcessA
- CreateRemoteThread
- VirtualAlloc, VirtualAllocEx 
- WriteProcessMemory, ReadProcessMemory
- RegOpenKeyExW, RegOpenKeyExA 
- system

#### For  Unix-like  systems:  standard  library  functions
- System
- chmod, chmod
- chown, fchown, lchown
- mmap, munmap
- setuid, setgid, seteuid, setegid
- listen, socket, bind
- recv, recvfrom, recvmsg 
- sending, sendto, sendmsg

##

### Interpreter
The  interpreter  includes,  as  already  mentioned:  lexical,  syntactic  and  semantic  analyzers.  After the analyzers  work, a certain  **abstract  expression of functions**(aef) is obtained - an array  (std::vector)  expressions,  where  each  **expression**  has  its  **representation of the function** and a list of  subexpressions,  where  each  subexpression  is a parameter  passed to the called  function  (is a representation of the function).  These  concepts  will be discussed  further.

The interpreter  is  a  class  in  which there are  two  public  functions  that  perform the main  actions:  building  **aef**,  calling  all  undeclared  and  not  included  in the list of functions  for  semantic  analysis,  functions  in  order:

- ` void build_aef(); `
- ` void interpreter_run(); `

The class  has a configuration  field  that  contains  information  for  building  **aef**:
		
    struct config {
	    bool debug_output{0};
	    bool CDPCM{0};
	    bool import_module{0};
	    bool use_external_scope{0};
	    bool transmit_smt_name_func_with_smt{0};
	    const char *filename_interp{MAIN_FILE};
	    const char *file_import_file_f{IMPORT_FILE};
	};

-  **debug_output**  -  outputs  debugging  information  (execution  time of the lexical,  syntactic,  semantic  analyzer,  as  well as the entire  **aef**). 
-  **CDPCM**  -  (construction of a dynamic  post-semantic command  module  -  **DPCM**  )  generates a dynamic post-semantic  command  module.  This  module  can be connected  and  a  common  function can be called that  will  reproduce  all  the  same  actions  that  were  performed  in the main  file  (bweasconf.txt  ). 
-  **import_module** - flag,  when  set  to  true, it is allowed to import  modules.
-  **use_external_scope**  -  flag,  when  set  to  true, it is allowed to use  only the external  scope  (needed  for  functions that create  another  instance of the interpreter). 
-  **transmitte_smt_name_func_with_smt**  is a flag  that, when  set  to  true, allows you to add  custom  functions  from  modules to the  list of functions  that  must  be  performed  at the semantic  analysis  stage  (more details  below).
-  **filename_interp**  is a file  for  interpretation.
-  **file_import_file_f**  is a file that provides a list of modules  with  their  functions  to  import.

*MAIN_FILE const = "bweasconf.txt";*
*IMPORT_FILE const = "import-modules.imp";*
 
 **Next we will talk about the construction of the entire AEF.**

### - Lexical analysis

Lexical analysis is represented by the parser class ( [ lexer.hpp ](https://github.com/svec1/bweas/blob/main/src/bs/lang/lexer.hpp) ), which accepts the source text (code), and when the analysis function is called, the entire text is analyzed. Lexical analysis identifies tokens among a stream of characters and creates tokens based on them.

##### - [ (declared in token.hpp) ](https://github.com/svec1/bweas/blob/main/src/bs/lang/token.hpp)
Tokens have the following structure:

    struct  token {
		token_type  token_t;
		std::string  token_val;
		u32t  pos_defined_line{0}; // line number
		u32t  pos_beg_defined_sym{0}; // starting character number
	};

All fields are obvious (sort of), however the token type - token_t, is an enum:

    enum  class  token_type {
		KEYWORD  =  0,
		
		// 1,2,3...(UINT_MAX32 / 2)
		LITERAL,
		
		// "text"
		LITERALS,
		
		ID,
		COMMA,
		OPEN_BR,
		CLOSE_BR,
		
		// > < = +
		// only the + operator is a string addition operator
		// (arithmetic operations are prohibited at the operator level)
		// -----------------------------------------------------------
		// Also, strings cannot be compared (for now)
		OPERATOR,
		SIZE_ENUM_TOKEN_TYPE
	};

The lexical analyzer provides lexical rules that prohibit:

 1. **use of symbols: * / ! - ' \.**
 2. **form mixed lexemes: "32ggtn5var".**
 3. **form a literal greater than UINT_MAX32/2.**

Lexical analysis provides comments, they start with #, and can end with either \r\n, \n or #.
The lexical analyzer also provides its own directives that start with @ and end with \r\n or \n. Existing directives:

 - lexer_stop - stops lexical analysis.

### - Syntactic analysis
Parsing is represented by the parser class, which accepts an array of tokens (std::vector), which returns a lexer. This analyzer sets the syntax rules and determines the type of command.

**Command type: `id(param1, param2, ...)`**
As id, the identifier or keyword of the configuration language that also installs the parser is accepted.

##### - [(declared in expression.hpp)](https://github.com/svec1/bweas/blob/main/src/bs/lang/expression.hpp)

The parameters can be the following types presented in the enumeration:

    enum  class  params {
		FUTURE_VAR_ID,
		VAR_ID,
		NCHECK_VAR_ID,
		VAR_STRUCT_ID,
		ANY_VALUE_WITHOUT_FUTUREID_NEXT,
		LIT_STR,
		LIT_NUM,
		LSTR_OR_ID_VAR,
		LNUM_OR_ID_VAR,
		NEXT_TOO,
		SIZE_ENUM_PARAMS // undef
	};

- **FUTURE_VAR_ID** - token type must be an identifier. Moreover, it must be declared for the first time (more below).
- **VAR_ID** - token type must be an identifier. It must be declared during use. 
- **NCHECK_VAR_ID** - token type must be an identifier (ID). An identifier that may or may not be declared (more below).
- **VAR_STRUCT_ID** - token type must be an identifier (ID). An identifier that must point to a structure of type project or target.
- **ANY_VALUE_WITHOUT_FUTUREID_NEXT** - any token type. 
- **LIT_STR** - token must be of type LITERALS 
- **LIT_NUM** - token must be of type LITERALS
- **LSTR_OR_ID_VAR** - token must be of type LITERALS or identifier (ID)
- **LNUM_OR_ID_VAR** - token must be of type LITERAL or identifier (ID)
- **NEXT_TOO** - this parameter is a copy of the previous parameter

Since these parameters are an abstraction for the function declaration, the parameters themselves (which were passed to the function) will be subexpressions of one expression (one command). The structure of a subexpression looks like this: 

    struct subexpressions {
	    enum class type_subexpr;
	    enum class ret_type_subexpr;
	    inline ret_type_subexpr returns_type_subexpr();
	    std::vector < token_expr: :token > token_of_subexpr;
	    type_subexpr subexpr_t;
	};

Each subexpression has its own type and return type (final, since the expression type may change during the semantic analysis stage). Subexpressions include a token stream that creates the concept of a subexpression. The type of a subexpression is determined after checking two syntactic rules:

- **Correct token sequence (3>6, "teext"+TEXT+"txt" (assuming the identifier points to a string variable) or 1).**
- **Correct use of operators (numbers can be compared, but not added; strings can be added, but not compared).**

If the sequence of tokens does not include an identifier, or if the identifier(s) is in a position where there are two or more other tokens between the operator, then these sequences are replaced by a single token that matches their equality, inequality, or addition (if they are strings).

After all checks and substitutions, a subexpression is defined with the following possible types: 

	enum class type_subexpr {
		INT_COMPARE,
		STRING_ADD, 
		INT, 
		STRING, 
		ID, 
		SIZE_ENUM_TYPE_SUBEXPR // undef 
	};

- **INT_COMPARE** is a type in which a sequence of tokens has at least one identifier, and one (or more) comparison operators exist as an action. 
- **STRING_ADD** is a type in which a sequence of tokens has at least one identifier, and one (or more) addition operators exist as an action.
- **INT** is a type having a LITERAL token. 
- **STRING** is a type having a LITERALS token.
- **ID** is a type having an ID token.

If the type is  **INT_COMPARE** or **STRING_ADD**, then at the semantic analysis stage its sequence will turn into a single token.

Subexpressions, as mentioned earlier, are part of an expression (command). An expression has an array of subexpressions and a function definition. The structure of an expression is shown below: 
		
    struct expression {
	    inline bool execute_with_semantic_an(); 
	    notion_expr_func expr_func; 
	    std::vector<subexpressions> sub_expr_s;
    };

**- notion_expr_func** is a structure that includes a command identifier token and a structure describing the function. The definition of the function (which is called by the command) is described in the **notion_func** structure : 

    struct notion_func {
	    void (*func_ref)(const std::vector < subexpressions > &, var::scope &);			   
	    std::vector < params > expected_args; bool is_declaration_var{0}; bool 
	    only_with_semantic{0};
    };
- **func_ref** is a function pointer (binding occurs during the semantic analysis stage and is described below). 
- **expected_args** are the expected arguments, which come in order.
- **is_declaration_var** is a flag indicating that this function is declared (described below). 
- ** only_with_semantic** is a flag indicating that this function should be called during the semantic analysis stage in any case.

### - Semantic analysis
Semantic analysis is the most important part of the interpreter, since it is here that functions are linked, passed parameters are checked, symbols (variables) are created, and the existence or absence of variables specified by identifiers is checked.

Semantic analysis consists of 3 passes.

1. Check the identifiers of the functions to be called and create a table of functions. 
2.  Associate the function definitions from the table with the expression and check the passed parameters for compliance. 
3.  Call the functions that are declared (or not...) to create all variables, checking the use of each identifier. 

#### First pass
The first pass generates a table of functions that are called in the source code, and also determines which functions will be called in the third stage of semantic analysis. Keywords, if they are callable, will have a null function pointer, since they are processed (not called) in the third stage of semantic analysis.

At this stage, static functions are defined, and in case of a call, user-defined functions are defined, which the interpreter imports before calling lexical analysis ( * User-defined functions will be looked up in the table of external function definitions * ). Also at this stage, the function property is defined: ** declarative ** . A function can have this property provided that the function has at least one of the following parameters:

1.  **FUTURE_VAR_ID**
2.  **NCHECK_VAR_ID**
 
In stage 3 , the first parameter will tell the analyzer that it needs to check for non-existence of a variable with the given identifier, which may imply that the function may declare a variable with the given identifier (not exactly true). The second parameter , the semantic analysis in stage 3, will be skipped, since the parameter says that the passed identifier may exist, but it is not required to.

#### Second pass
During the second pass, the passed parameters of each function call will be checked against the expected parameters. The second pass also finds the declaration of the function in the declaration table. Matching the type of the variable pointed to by the identifier with the expected parameter of the called function occurs in the next step.


#### Third pass

The third pass is the most complex, as it includes not only checking the types of variables and their existence, but also converting subexpressions if their token sequence has not been completely replaced by a single token, as well as converting variables of the type: integers, strings, arrays of numbers and strings, into their values. Thus, after this pass, all specified identifiers will be implicitly converted to their values.

Semantic  analysis  performs the following  actions  in  order:

 1. Checking  all  used  identifiers  for  compliance  with the expected  parameters of the called  function.
 2. Keyword processing(if-else-endif), if the boolean variable **skip** == true, all expressions(++i) will be deleted until **nested_if** != 0 (**skip** == true when the if condition is incorrect, or if else is encountered when the if condition is true).
 3. The call of declaring or special functions is checked by calling: *expr_s[i].execute_with_semantic_an()*.

If the function  is  not  declarative, but  it has been  listed as static-special  or  user-special  (described  in the module  documentation),  it  will be called, since the first stage will set  only_with_semantic to true (the interpreter itself will load them(functions) into the **name_func_with_semantic_an** array of the semantic analyzer.).
After  calling the function, the interception  check  function  will be called  (depending  on the platform):

 - `check_safe_call_dll_func();` - WINDOWS
 - `check_safe_call_dl_func();` - UNIX

This stage is the last stage of analysis, after which the interpretation of commands is called().

    aef[curr_expr]
	    .expr_func
	    .func_n
	    .func_ref(aef[curr_expr].sub_expr_s, // param
		    global_scope);

**After the call, the interception check is called in the same way as in semantic analysis.**