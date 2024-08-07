#ifndef _HAND_ARG__H
#define _HAND_ARG__H

#include "../../kernel/low_level/type.h"

#include <vector>
#include <string>

// attibute(command) begin with of symbol: '-';
// attibute(subcom) begin with of symbol: '/',
// the value all of subcom indicated after ':';
namespace attrib{
    // this attribute starts the build system to build all targets
    extern const char* build;

    // With this attribute, the build system, when completing its work,
    // exports all the values ​​of global and user variables to a separate file
    extern const char* _export;
            
    enum subcom : char{
        // subcom for change value of global config variable (the value depends on the type of the variable)
        v = 'V',
        // subcom for change name of file for logging(value is string)
        l = 'L',
        // subcom for on/off output(debug) info of system build
        d = 'D',
        // subcom for on/off output compilers
        c = 'C'
    };
}

// handler arg
class hax{
    public:

        hax(char** args, int argv);

        hax(hax&&) = delete;
        hax&& operator=(hax&&) = delete;
        void* operator new(size_t size) = delete;
        
        ~hax() = default;
    public:

        std::string operator[](u32t ind);
        
        void handle(std::string arg);
        void handle_all();
    private:

        static inline bool init_glob{0};
        std::vector<std::string> arg_s;
};

#endif