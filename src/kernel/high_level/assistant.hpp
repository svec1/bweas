#ifndef _ASSITANT__H_
#define _ASSITANT__H_

#include "../low_level/ll.h"

#include <unordered_map>
#include <fstream>
#include <vector>
#include <string>

#define MODE_READ_FILE     1
#define MODE_WRITE_TO_FILE 2

typedef u32t HND;

class assistant{
    public:
        // output to console = true, log = false
        assistant();
        assistant(const assistant&) = delete;
        assistant&& operator=(assistant&&) = delete;

        assistant(bool _output, bool _log);

        ~assistant();

    public:
        void file_log_name(std::string name_file);

        // if log = false, the function does nothing
        // *turns logging on or off (subsequent log requests will be ignored)
        void switch_log(bool val);

        // if output = false, the function does nothing
        // *turns console output on or off (subsequent output requests will be ignored)
        void switch_otp(bool val);

        // create err{struct *err}
        void add_err(std::string name_err, std::string desc);
        // detects and handles the error
        void call_err(std::string name_err);
        void call_err(std::string name_err, std::string addit);

        HND open_file(std::string name_file, i32t mode = MODE_READ_FILE);
        void close_file(HND handle);


        HND get_handle_file(std::string name_file);

        std::string read_file(HND handle);
        void write_file(HND handle, std::string buf);

#ifdef _WIN32
        DWORD get_error_win32();

        HMODULE load_dll(std::string dll_name);
        void dump_dll(std::string dll_name);

        void* get_ptr_func(std::string dll_name, std::string name_func);

        HMODULE get_handle_module_dll(std::string dll_name);
#endif

        // output to console(if output = true) and logging(if log = true)
        void operator<<(std::string text);

    private:
        HND open_file(i32t mode, std::string &path);

        std::vector<_err*> err_assistant;
        std::vector<std::string> name_all_files;
        #ifdef _WIN32
        std::unordered_map<std::string, HMODULE> hDLL_s;
        #endif

        std::ofstream log_file;

        bool output{1}, log{1};
};

#endif