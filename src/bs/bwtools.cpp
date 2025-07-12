//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <stdlib.h>

#include <bwmacros_platform.h>
#include <bwtools.hpp>

#define IF_ERRNO()                                                                                                     \
    if (errno)                                                                                                         \
        bwtools::fatal(std::strerror(errno));

#if defined(WIN)
static HANDLE STD_HANDLE = GetStdHandle(STD_OUTPUT_HANDLE);

virtual_terminal::virtual_terminal() {
    DWORD mode;
    GetConsoleMode(STD_HANDLE, &mode);
    mode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(STD_HANDLE, mode);
}
virtual_terminal::~virtual_terminal() {
    SetConsoleMode(STD_HANDLE, 0);
}
#endif

using namespace bweas;

vec<bwtools::file> bwtools::files;

void bwtools::message(std::string_view str) {
    std::fprintf(stdout, "%s\n", str.data());
}
void bwtools::success(std::string_view str) {
    std::fprintf(stdout, "\e[1;32m%s\e[0m\n", str.data());
}
void bwtools::warning(std::string_view str_warn) {
    std::fprintf(stderr, "\e[1;33m%s\e[0m\n", str_warn.data());
}
void bwtools::error(std::string_view str_err) {
    std::fprintf(stderr, "\e[1;31m");
    if (str_err.empty())
        std::fprintf(stderr, "%s\e[0m\n", std::strerror(errno));
    else
        std::fprintf(stderr, "%s\e[0m\n", str_err.data());
}
void bwtools::fatal(std::string_view str_err) {
    std::fprintf(stderr, "\e[1;31m");
    if (str_err.empty())
        std::fprintf(stderr, "%s\e[0m\n", std::strerror(errno));
    else
        std::fprintf(stderr, "%s\e[0m\n", str_err.data());

    exit(1);
}

std::string bwtools::get_time() {
    auto time    = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    tm *time_now = std::localtime(&time);

    return (time_now->tm_mon < 10 ? "0" + std::to_string(time_now->tm_mon) : std::to_string(time_now->tm_mon)) + "." +
           (time_now->tm_mday < 10 ? "0" + std::to_string(time_now->tm_mday) : std::to_string(time_now->tm_mday)) +
           " " + std::to_string(time_now->tm_hour) + ":" + std::to_string(time_now->tm_min);
}

std::string bwtools::get_path_program() {
#if defined(WIN)
    std::string str(MAX_PATH, '\0');
    GetModuleFileNameA(NULL, str.data(), MAX_PATH);
#elif defined(UNIX)
    std::string str(PATH_MAX, '\0');
    readlink("/proc/self/exe", str.data(), PATH_MAX);
#endif
    str.erase(str.find_last_of("/\\"), str.size());

    IF_ERRNO();
    return str + "/";
}
std::string bwtools::get_current_path() {
    return fs::current_path().string();
}
bwtools::file_it bwtools::open_file(std::string_view name_file, file::mode_file::open mode) {
    file_it it = get_iterator_file(name_file);
    if (exist_file(it)) {
        if ((files.begin() + it)->file_opened)
            return it;

        files[it].open(mode);
        return it;
    }
    files.emplace_back(fs::absolute(name_file), mode);

    return files.size() - 1;
}
void bwtools::close_file(bwtools::file_it file) {
    if (!exist_file(file))
        fatal("There are no files with this index.");
    files.erase(files.begin() + file);
}
bool bwtools::exist_file(file_it file) {
    if ((files.begin() + file) == files.end())
        return 0;
    return 1;
}
bool bwtools::exist_file(std::string_view name_file) {
    if (fs::exists(name_file))
        return 1;
    return 0;
}
bwtools::file_it bwtools::get_iterator_file(std::string_view name_file) {
    return std::distance(files.begin(),
                         std::find_if(files.begin(), files.end(), [name_file](const bwtools::file &file) {
                             return file.path_to == fs::absolute(name_file);
                         }));
}

bwtools::file &bwtools::get_ref_file(file_it file) {
    return *(files.begin() + file);
}

std::string bwtools::read_file(file &file, file::mode_file::input mode) {
    std::string data_file;
    if (mode == file::mode_file::input::read_binary) {
        size_t size_file;
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
void bwtools::write_file(file &file, std::string_view buf, file::mode_file::output mode) {
    if (mode == file::mode_file::output::write_binary)
        file.stream.write(buf.data(), buf.size());
    else
        file.stream << buf;
}

