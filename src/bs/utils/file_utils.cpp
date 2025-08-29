//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <algorithm>
#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <stdlib.h>

#include <bwmacros_platform.h>
#include <utils/file_utils.hpp>

using namespace bweas::utils;

std::string file_utils::get_time() {
    auto time    = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    tm *time_now = std::localtime(&time);

    return (time_now->tm_mon < 10 ? "0" + std::to_string(time_now->tm_mon) : std::to_string(time_now->tm_mon)) + "." +
           (time_now->tm_mday < 10 ? "0" + std::to_string(time_now->tm_mday) : std::to_string(time_now->tm_mday)) +
           " " + std::to_string(time_now->tm_hour) + ":" + std::to_string(time_now->tm_min);
}

std::string file_utils::get_path_program() {
#if defined(WIN)
    std::string str(MAX_PATH, '\0');
    GetModuleFileNameA(NULL, str.data(), MAX_PATH);
#elif defined(UNIX)
    std::string str(PATH_MAX, '\0');
    readlink("/proc/self/exe", str.data(), PATH_MAX);
#endif
    str.erase(str.find_last_of("/\\") + 1, str.size());

    return str;
}
std::string file_utils::get_current_path() {
    return fs::current_path().string();
}
vec<file_utils::file>& bweas::utils::file_utils::get_files() {
    static vec<file_utils::file> files;
    return files;
}
file_utils::file_it file_utils::open_file(std::string_view name_file, file::mode_file::open mode) {
    auto &files = get_files();

    file_it it = get_iterator_file(name_file);
    if (exist_file(it)) {
        if (files[it].file_opened)
            return it;

        files[it].open(mode);
        return it;
    }
    files.emplace_back(fs::absolute(name_file), mode);

    return files.size() - 1;
}
void file_utils::close_file(file_utils::file_it file) {
    auto &files = get_files();

    if (!exist_file(file))
        throw std::runtime_error("There are no files with this index.");
    files.erase(files.begin() + file);
}
bool file_utils::exist_file(file_it file) {
    auto &files = get_files();
    if ((files.begin() + file) == files.end())
        return 0;
    return 1;
}
file_utils::file_it file_utils::get_iterator_file(std::string_view name_file) {
    auto &files = get_files();
    return std::distance(files.begin(),
                         std::find_if(files.begin(), files.end(), [name_file](const file_utils::file &file) {
                             return file.path_to == fs::absolute(name_file);
                         }));
}

file_utils::file &file_utils::get_ref_file(file_it file) {
    auto &files = get_files();
    return *(files.begin() + file);
}

std::string file_utils::read_file(file &file, file::mode_file::input mode) {
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
void file_utils::write_file(file &file, string_v buf, file::mode_file::output mode) {
    if (mode == file::mode_file::output::write_binary)
        file.stream.write(buf.data(), buf.size());
    else
        file.stream << buf;
}
vec<string> file_utils::file_slc_mask(string mask, const vec<string> &files) {
    if (mask.empty())
        return files;

    vec<string> slc_files;
    string files_str;

    for (const auto &file : files)
        files_str += file + " ";

    string mask_regex;
    for (size_t j = 0; j < mask.size(); ++j)
        if (mask[j] == '*')
            mask_regex += "\\w+";
        else if (mask[j] == '.')
            mask_regex += "\\.";
        else if (mask[j] == '?')
            mask_regex += "\\w";
        else if (mask[j] == '/')
            mask_regex += "\\/";
        else
            mask_regex += mask[j];

    if (mask_regex.find("/") == mask_regex.npos) {
        std::regex file_mask("[\\/\\w+]*" + mask_regex);
        for (auto it_match = std::sregex_iterator(files_str.begin(), files_str.end(), file_mask);
             it_match != std::sregex_iterator(); ++it_match)
            slc_files.push_back(it_match->str());
    }
    else {

        std::regex file_mask(mask_regex);
        for (auto it_match = std::sregex_iterator(files_str.begin(), files_str.end(), file_mask);
             it_match != std::sregex_iterator(); ++it_match)
            slc_files.push_back(it_match->str());
    }
    return slc_files;
}

string file_utils::get_path_file(string name_file) {
    if (auto path_file = fs::weakly_canonical(fs::current_path() / name_file); fs::is_regular_file(path_file))
        return path_file.string();

    return {};
}

string file_utils::get_path_file(string name_file, const vec<string> &possible_paths) {
    if (auto path_file = get_path_file(name_file); !path_file.empty())
        return path_file;

    fs::path current_path_tmp = fs::current_path();
    fs::path find_path_file;

    for (const auto &path : possible_paths) {
        fs::current_path(fs::weakly_canonical(path));
        if (auto path_file = fs::weakly_canonical(fs::current_path() / name_file); fs::is_regular_file(path_file)) {
            find_path_file = path_file;
            break;
        }
    }

    fs::current_path(current_path_tmp);

    return find_path_file.string();
}

