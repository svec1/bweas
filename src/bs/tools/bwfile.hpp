//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWFILE__H
#define BWFILE__H

#include <algorithm>
#include <filesystem>

#include <bwaliases.hpp>
#include <bwtools.hpp>

class bwfile {
  public:
    bwfile() = delete;

  public:
    // Creates an array of file names based on the mask
    // passed to the function and an array of all files.
    // ### The syntax is fully compliant with the glob() standard.
    static inline vec<string> file_slc_mask(string mask, const vec<string> &files);

    // Returns the absolute path to an existing file, considering the current directory.
    static inline string get_path_file(string name_file);
    // Returns the absolute path to an existing file, considering all possible paths, including the current directory.
    static inline string get_path_file(string name_file, const vec<string> &possible_paths);
};

vec<string> bwfile::file_slc_mask(string mask, const vec<string> &files) {
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

string bwfile::get_path_file(string name_file) {
    if (auto path_file = fs::weakly_canonical(fs::current_path() / name_file); fs::is_regular_file(path_file))
        return path_file.string();

    return {};
}

string bwfile::get_path_file(string name_file, const vec<string> &possible_paths) {
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

#endif
