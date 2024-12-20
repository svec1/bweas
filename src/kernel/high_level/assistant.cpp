#include "assistant.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <stdlib.h>

#if defined(_WIN32)
#include "hook_winapi.hpp"
#include <windows.h>
#elif defined(UNIX)
#include <dlfcn.h>
#include <limits.h>
#include <unistd.h>

extern "C" int init_hook();
#endif

assistant assist;

assistant::assistant() {
    err_s.emplace_back("KNL000", "Unable to open file", 0);
    err_s.emplace_back("KNL001", "Unable to close file", 1);
    err_s.emplace_back("KNL002", "File opened for reading", 2);
    err_s.emplace_back("KNL003", "File opened for writing", 3);
}
assistant::assistant(bool _output, bool _log) : output(_output), log(_log) {
}

assistant::~assistant() {
#if defined(WIN)
    for (const auto &it : hDLL_s)
        FreeLibrary(it.second);
#elif defined(UNIX)
    for (const auto &it : hDL_s)
        dlclose(it.second);
#endif
}

void assistant::set_file_log_name(std::string name_file) {
    if (name_file.empty())
        return;
    log_file_name = name_file;
    if (files.begin() + get_iterator_file(log_file_name) == files.end())
        open_file(name_file, file::mode_file::open::wa);
}
void assistant::switch_log(bool val) {
    log = val;
}
void assistant::switch_otp(bool val) {
    output = val;
}
void assistant::add_err(std::string name_err, std::string desc) {
    err_s.emplace_back(name_err, desc, err_s.size());
}
void assistant::call_err(std::string name_err) {
    const auto &it =
        std::find_if(err_s.begin(), err_s.end(), [name_err](const err &_err) { return _err.name_e == name_err; });
    if (it == err_s.end()) {
        this->operator<<("An error with the name \"" + name_err + "\" was not found");
        exit(EXIT_FAILURE);
    }

    this->operator<<(std::string(it->name_e) + "(" + std::to_string(it->ind) + "): " + it->desc_e);
    exit(EXIT_FAILURE);
}
void assistant::call_err(std::string name_err, std::string addit) {
    const auto &it =
        std::find_if(err_s.begin(), err_s.end(), [name_err](const err &_err) { return _err.name_e == name_err; });
    if (it == err_s.end()) {
        this->operator<<("An error with the name \"" + name_err + "\" was not found");
        exit(EXIT_FAILURE);
    }

#if defined(WIN)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
#endif
    if (addit.empty())
        this->operator<<(it->name_e + "(" + std::to_string(it->ind) + ")");
    else
        this->operator<<(it->name_e + "(" + std::to_string(it->ind) + "): " + it->desc_e + "\nDetail: [" + addit + "]");
#if defined(WIN)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#endif
    exit(EXIT_FAILURE);
}
std::string assistant::get_desc_err(std::string name_err) {
    const auto &it =
        find_if(err_s.begin(), err_s.end(), [name_err](const err &err_c) { return err_c.name_e == name_err; });
    if (it != err_s.end())
        return it->desc_e;
    return "";
}

assistant::file_it assistant::open_file(std::string name_file, file::mode_file::open mode) {
    try {
        file_it it = get_iterator_file(name_file);
        if (exist_file(it) && !(files.begin() + it)->file_opened) {
            files[it].open(mode);
            return it;
        }
        files.emplace_back(std::filesystem::absolute(name_file), mode);
        return files.size() - 1;
    }
    catch (std::exception &excp) {
        call_err("KNL000", excp.what());
    }
}
void assistant::close_file(assistant::file_it file) {
    if (!exist_file(file))
        call_err("KNL001");
    files.erase(files.begin() + file);
}
bool assistant::exist_file(file_it file) {
    if ((files.begin() + file) == files.end() || !(files.begin() + file)->file_opened)
        return 0;
    return 1;
}

assistant::file_it assistant::get_iterator_file(std::string name_file) {
    return std::distance(files.begin(),
                         std::find_if(files.begin(), files.end(), [name_file](const assistant::file &file) {
                             return file.path_to == std::filesystem::absolute(name_file);
                         }));
}

assistant::file &assistant::get_ref_file(file_it file) {
    return *(files.begin() + file);
}

std::string assistant::read_file(file &file, file::mode_file::input mode) {
    CHECK_OPEN_FR()

    std::string data_file;
    if (mode == file::mode_file::input::read_binary) {
        u32t size_file;
        file.stream.seekg(0, std::ios::end);
        size_file = file.stream.tellg();
        file.stream.seekg(0, std::ios::beg);
        data_file.resize(size_file);
        file.stream.read(data_file.data(), size_file);
    }
    else {
        std::string tmp;
        while (std::getline(file.stream, tmp))
            data_file += tmp + "\n";
    }
    return data_file;
}
void assistant::write_file(file &file, std::string buf, file::mode_file::output mode) {
    CHECK_OPEN_FW()
    if (mode == file::mode_file::output::write_binary)
        file.stream.write(buf.data(), buf.size());
    else
        file.stream << buf;
}

std::string assistant::get_time() {
    auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    tm *time_now = localtime(&time);
    return std::to_string(time_now->tm_mon) + "." + std::to_string(time_now->tm_mday) + " " +
           std::to_string(time_now->tm_hour) + ":" + std::to_string(time_now->tm_min);
}

void assistant::next_output_unsuccess() {
#if defined(WIN)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
#elif defined(UNIX)
    printf("\033[31m");
#endif
}
void assistant::next_output_important() {
#if defined(WIN)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#elif defined(UNIX)
    printf("\033[33m");
#endif
}
void assistant::next_output_success() {
#if defined(WIN)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
#elif defined(UNIX)
    printf("\033[32m");
#endif
}
std::string assistant::get_path_program() {
#if defined(WIN)
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
#elif defined(UNIX)
    char buffer[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buffer, PATH_MAX);
#endif
    std::string str(buffer);
    str.erase(str.find_last_of("/\\"), str.size());
    return str;
}
#if defined(WIN)
DWORD
assistant::get_error_win32() {
    return GetLastError();
}
HMODULE
assistant::load_dll(std::string name_file) {
    HMODULE hDLL = LoadLibraryExA(name_file.c_str(), NULL, NULL);
    if (!hDLL)
        return NULL;
    hDLL_s[name_file] = hDLL;
    return hDLL;
}
void assistant::dump_dll(std::string dll_name) {
    if (hDLL_s.find(dll_name) == hDLL_s.end())
        return;
    FreeLibrary(hDLL_s[dll_name]);
    hDLL_s.erase(dll_name);
}
void assistant::safe_call_dll_func_begin() {
    _winapi_call_hook();
}
void assistant::check_safe_call_dll_func() {
    if (_winapi_call_knw_was_hook()) {
        printf("\n");
        if (log)
            write_file(*(files.begin() + get_iterator_file(log_file_name)), HOOK_TEXT_DETECTED,
                       file::mode_file::output::write_default);
        exit(1);
    }
}
void assistant::safe_call_dll_func_end() {
    _winapi_call_remove();
}
void *assistant::get_ptr_func(std::string dll_name, std::string name_func) {
    if (hDLL_s.find(dll_name) == hDLL_s.end())
        return nullptr;
    return GetProcAddress(hDLL_s[dll_name], name_func.c_str());
}
HMODULE
assistant::get_handle_module_dll(std::string dll_name) {
    if (hDLL_s.find(dll_name) == hDLL_s.end())
        return 0;
    return hDLL_s[dll_name];
}
#elif defined(UNIX)

std::string assistant::get_error_dl() {
    return dlerror();
}

u32t assistant::load_dl(std::string dl_name) {
    if (hDL_s.find(dl_name) != hDL_s.end())
        return 1;
    void *dlmem_p = dlopen(dl_name.c_str(), RTLD_LAZY);
    if (dlmem_p == nullptr)
        return 1;
    hDL_s[dl_name] = dlmem_p;
    return 0;
}
u32t assistant::dump_dl(std::string dl_name) {
    if (hDL_s.find(dl_name) == hDL_s.end())
        return 1;
    dlclose(hDL_s[dl_name]);
    return 0;
}

void *assistant::get_ptr_func(std::string dl_name, std::string name_func) {
    if (hDL_s.find(dl_name) == hDL_s.end())
        return nullptr;
    void *ptr_sym = dlsym(hDL_s[dl_name], name_func.c_str());
    return ptr_sym;
}

void assistant::check_safe_call_dl_func() {
    void *ptr_sym = dlsym(RTLD_DEFAULT, SBW_LIB_HOOK_STAT_SYM);
    if (ptr_sym == nullptr)
        return;
    if (*((int *)ptr_sym)) {
        if (log)
            write_file(*(files.begin() + get_iterator_file(log_file_name)), HOOK_DETECTED_STR,
                       file::mode_file::output::write_default);
        exit(1);
    }
}

void *assistant::get_realsystem_func() {
    if (!realsystem_func)
        realsystem_func = dlsym(RTLD_NEXT, "system");

    return realsystem_func;
}

#endif
void assistant::operator<<(std::string text) {
    if (output)
        printf("%s\n", text.c_str());
    const auto &log_file = files.begin() + get_iterator_file(log_file_name);
    if (log && log_file != files.end() && log_file->file_opened)
        write_file(*log_file, text + "\n", file::mode_file::output::write_default);

#if defined(WIN)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
#elif defined(UNIX)
    printf("\033[0m");
#endif
}