#include "bwgntools.hpp"

std::string bweas::generator::tools::get_name_output_file(std::string pattern_file, u32t index,
                                                          std::string dir_work_endv) {
    if (pattern_file.find(".") == pattern_file.npos)
        return dir_work_endv + "/" + pattern_file + std::to_string(index);
    std::string name_output_file_curr = pattern_file, extension_output_file_curr = pattern_file;
    name_output_file_curr.erase(name_output_file_curr.find("."), name_output_file_curr.size());
    extension_output_file_curr.erase(0, extension_output_file_curr.find("."));
    if (index != 0)
        return dir_work_endv + "/" + name_output_file_curr + std::to_string(index) + extension_output_file_curr;
    return dir_work_endv + "/" + name_output_file_curr + extension_output_file_curr;
}

std::vector<std::string> bweas::generator::tools::file_slc_mask(std::string mask,
                                                                const std::vector<std::string> &files) {
    std::vector<std::string> slc_files;
    std::vector<std::string> masks;

    std::string mask_tmp;
    for (u32t i = 0; i < mask.size() + 1; ++i) {
        if (mask[i] == '|' || i == mask.size()) {
            if (mask_tmp.empty())
                continue;
            else if (std::find(masks.begin(), masks.end(), mask_tmp) == masks.end())
                masks.push_back(mask_tmp);
            mask_tmp.clear();
            continue;
        }
        mask_tmp += mask[i];
    }

    for (u32t i = 0; i < masks.size(); ++i) {
        mask = masks[i];
        for (u32t j = 0; j < files.size(); ++j) {
            std::string mask_tmp{mask};
            bool success_file = 1;

            u32t sym_file = 0;
            for (u32t sym = 0; sym < mask_tmp.size(); ++sym, ++sym_file) {
                if (mask_tmp[sym] == files[j][sym_file])
                    continue;
                else if (mask_tmp[sym] == '?') {
                    if (sym >= files[j].size() || files[j][sym] == '.') {
                        success_file = 0;
                        break;
                    }
                    mask_tmp.erase(sym--, 1);
                    ++sym_file;
                    continue;
                }
                else if (mask_tmp[sym] == '*') {
                    for (; sym_file < files[j].size(); ++sym_file) {
                        if (files[j][sym_file] == mask_tmp[sym + 1] ||
                            (sym + 2 < mask_tmp.size() && mask_tmp[sym + 1] == '?' &&
                             files[j][sym_file] != mask_tmp[sym + 2]))
                            goto next_sym_mask;
                    }
                    if (sym == mask_tmp.size() - 1)
                        goto next_sym_mask;
                skip_file:
                    success_file = 0;
                    break;

                next_sym_mask:
                    if (sym == mask_tmp.size() - 1 && sym_file < files[j].size())
                        goto skip_file;

                    mask_tmp.erase(sym--, 1);
                    --sym_file;
                    continue;
                }
                else {
                    success_file = 0;
                    break;
                }
            }
            if (sym_file < files[j].size())
                continue;

            if (success_file)
                slc_files.push_back(files[j]);
        }
    }

    return slc_files;
}