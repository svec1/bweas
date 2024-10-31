#ifndef _ASSITANT__H_
#define _ASSITANT__H_

#include "bwmacros_platform.h"
#include "bwtype.h"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#define CHECK_OPEN_FR()                                                                                                \
    if (file.mode_open != file::mode_file::open::rb && file.mode_open != file::mode_file::open::r)                     \
        call_err("KNL003", file.path_to.string());

#define CHECK_OPEN_FW()                                                                                                \
    if (file.mode_open != file::mode_file::open::w && file.mode_open != file::mode_file::open::wa &&                   \
        file.mode_open != file::mode_file::open::wb)                                                                   \
        call_err("KNL002", file.path_to.string());

class assistant {
  public:
    // output to console = true, log = false
    assistant();
    assistant(const assistant &) = delete;
    assistant &&operator=(assistant &&) = delete;

    assistant(bool _output, bool _log);

    ~assistant();

  public:
    struct file {
        struct mode_file {
          private:
            mode_file() = delete;

          public:
            // mode of open
            enum class open {
                // open for read of binary
                rb = 0,
                // open for default reading
                r,
                // open for overwrite or create file with writing
                w,
                // open for writes to the end of the file
                wa,
                // open for overwrite or create file with writing of binary
                wb,
                // open for writes to the end of the file of binary
                wba
            };

            // mode of reading
            enum class input {
                // read of binary
                read_binary,
                // default reading
                read_default
            };
            // mode of writing
            enum class output {
                // write of binary(all bytes)
                write_binary,

                // default writing
                write_default
            };
        };

      public:
        file() = default;
        file(const file &) = delete;
        file &operator=(const file &) = delete;
        file(file &&_file) noexcept {
            if (_file.file_opened)
                _file.stream.close();
            path_to = _file.path_to;
            mode_open = _file.mode_open;
            open();
        }
        file(std::filesystem::path _path_to, mode_file::open _mode_open) : path_to(_path_to), mode_open(_mode_open) {
            open();
        }
        ~file() {
            close();
        }
        file &operator=(file &&_file) noexcept {
            if (_file.file_opened)
                _file.stream.close();
            path_to = _file.path_to;
            mode_open = _file.mode_open;
            open();

            return *this;
        }

        void open(mode_file::open _mode_open, std::filesystem::path _path_to = {}) {
            path_to = _path_to;
            mode_open = _mode_open;
            open();
        }

        // Opens a file with the specified mode
        void open() {
            if (mode_open == mode_file::open::rb)
                stream.open(path_to.string(), std::ios::in | std::ios::binary);
            else if (mode_open == mode_file::open::r)
                stream.open(path_to.string(), std::ios::in);
            else if (mode_open == mode_file::open::w)
                stream.open(path_to.string(), std::ios::out | std::ios::trunc);
            else if (mode_open == mode_file::open::wb)
                stream.open(path_to.string(), std::ios::out | std::ios::binary | std::ios::trunc);
            else if (mode_open == mode_file::open::wba)
                stream.open(path_to.string(), std::ios::out | std::ios::binary | std::ios::app);
            else
                stream.open(path_to.string(), std::ios::out | std::ios::app);

            if (!stream.is_open())
                throw std::runtime_error("[" + path_to.string() + "] Failed to open file!");

            file_opened = 1;
        }

        // Closes the file if the stream is open
        void close() {
            if (file_opened) {
                stream.close();
                file_opened = 0;
            }
        }

      public:
        std::fstream stream;
        std::filesystem::path path_to;
        mode_file::open mode_open;
        bool file_opened{0};
    };

  public:
    using file_it = u32t;

    void set_file_log_name(std::string name_file);

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

    file_it open_file(std::string name_file, file::mode_file::open mode = file::mode_file::open::r);
    void close_file(file_it file);

    bool exist_file(file_it file);

    file_it get_iterator_file(std::string name_file);
    file &get_ref_file(file_it file);

    std::string read_file(file &file, file::mode_file::input mode = file::mode_file::input::read_default);
    void write_file(file &file, std::string buf, file::mode_file::output mode = file::mode_file::output::write_default);

    std::string get_time();

    void next_output_unsuccess();
    void next_output_important();
    void next_output_success();

    std::string get_path_program();
#if defined(WIN)
    DWORD
    get_error_win32();

    HMODULE
    load_dll(std::string dll_name);
    void dump_dll(std::string dll_name);

    void safe_call_dll_func_begin();
    void check_safe_call_dll_func();
    void safe_call_dll_func_end();

    HMODULE
    get_handle_module_dll(std::string dll_name);

#elif defined(UNIX)

#define SBW_LIB_HOOK_STAT_SYM "HOOK_STATUS"
#define HOOK_DETECTED_STR                                                                                              \
    "WARNING!!!A system call was detected, most likely the dynamically loaded dll library is hostile."

    const char *get_error_dl();

    u32t load_dl(std::string dl_name);
    u32t dump_dl(std::string dl_name);

    void check_safe_call_dl_func();
    void *get_realsystem_func();

#endif
    void *get_ptr_func(std::string dl_name, std::string name_func);

    // output to console(if output = true) and logging(if log = true)
    void operator<<(std::string text);

  private:
    struct err {
        err() = default;
        err(std::string _name_e, std::string _desc_e, u32t _ind) : name_e(_name_e), desc_e(_desc_e), ind(_ind) {
        }

      public:
        std::string name_e;
        std::string desc_e;
        u32t ind;
    };

    std::vector<err> err_s;
    std::vector<file> files;

    std::string log_file_name;

#if defined(WIN)
    std::unordered_map<std::string, HMODULE> hDLL_s;
#elif defined(UNIX)
    std::unordered_map<std::string, void *> hDL_s;
    void *realsystem_func;
#endif

    bool output{1}, log{1};
};

#endif