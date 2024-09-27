#ifndef _ASSITANT__H_
#define _ASSITANT__H_

#include "../low_level/ll.h"

#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#define MODE_READ_FILE 1
#define MODE_WRITE_TO_FILE 2

typedef u32t HND;

class assistant {
  public:
    // output to console = true, log = false
    assistant();
    assistant(const assistant &) = delete;
    assistant &&
    operator=(assistant &&) = delete;

    assistant(bool _output, bool _log);

    ~assistant();

  public:
    void
    file_log_name(std::string name_file);

    // if log = false, the function does nothing
    // *turns logging on or off (subsequent log requests will be ignored)
    void
    switch_log(bool val);

    // if output = false, the function does nothing
    // *turns console output on or off (subsequent output requests will be ignored)
    void
    switch_otp(bool val);

    // create err{struct *err}
    void
    add_err(std::string name_err, std::string desc);
    // detects and handles the error
    void
    call_err(std::string name_err);
    void
    call_err(std::string name_err, std::string addit);

    HND
    open_file(std::string name_file, i32t mode = MODE_READ_FILE);
    void
    close_file(HND handle);

    bool
    exist_file(HND handle);

    HND
    get_handle_file(std::string name_file);

    std::string
    read_file(HND handle);
    void
    write_file(HND handle, std::string buf);

#if defined(WIN)
    DWORD
    get_error_win32();

    HMODULE
    load_dll(std::string dll_name);
    void
    dump_dll(std::string dll_name);

    void
    safe_call_dll_func_begin();
    void
    check_safe_call_dll_func();
    void
    safe_call_dll_func_end();

    HMODULE
    get_handle_module_dll(std::string dll_name);

#elif defined(UNIX)

#define SBW_LIB_HOOK_STAT_SYM "HOOK_STATUS"
#define HOOK_DETECTED_STR                                                                                              \
    "WARNING!!!A system call was detected, most likely the dynamically loaded dll library is hostile."

    const char *
    get_error_dl();

    u32t
    load_dl(std::string dl_name);
    u32t
    dump_dl(std::string dl_name);

    void
    check_safe_call_dl_func();

#endif
    void *
    get_ptr_func(std::string dl_name, std::string name_func);

    // output to console(if output = true) and logging(if log = true)
    void
    operator<<(std::string text);

  private:
    HND
    open_file(i32t mode, std::string &path);

    std::vector<_err *> err_assistant;
    std::vector<std::string> name_all_files;
#if defined(WIN)
    std::unordered_map<std::string, HMODULE> hDLL_s;
#elif defined(UNIX)
    std::unordered_map<std::string, void *> hDL_s;
#endif

    HND log_file_handle{0};

    bool output{1}, log{1};
};

#endif