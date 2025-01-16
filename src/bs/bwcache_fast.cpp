#include "bwcache_api.hpp"

#include <unordered_set>

using namespace bweas;
using namespace cache_api;
using namespace bweas::bwexception;

fast_bwcache::fast_bwcache() {
    if (!init_glob_chfast) {
        assist.add_err("BWS-CACHE000", "Incorrect cache structure");

        init_glob_chfast = 1;
    }
}

void fast_bwcache::delete_cache() {
    delete this;
}

std::string fast_bwcache::create_cache() {
    std::string serel_target_tmp;

    std::unordered_set<std::string> used_templates;
    std::unordered_set<std::string> all_used_globally_args;
    std::unordered_set<std::string> all_used_call_component;

    const auto &out_targets = _cache_data.targets_o_p != nullptr ? *_cache_data.targets_o_p : _cache_data.targets_o;

    for (i32t i = 0; i < out_targets.size(); ++i) {
        char *prj_v = (char *)&out_targets[i];
        serel_target_tmp += std::to_string(out_targets[i].prj.src_files.size()) + " " +
                            std::to_string(out_targets[i].prj.vec_templates.size()) + " " +
                            std::to_string(out_targets[i].target_vec_libs.size()) + " ";
        for (u32t j = 0; j < sizeof(var::struct_sb::project); j += sizeof(std::string)) {

            // version project
            if (j == sizeof(std::string)) {
                serel_target_tmp += (*(var::struct_sb::version *)(prj_v + j)).get_str_version() + " " +
                                    std::to_string(*(unsigned long *)(prj_v + j + sizeof(var::struct_sb::version))) +
                                    " ";
                j += sizeof(var::struct_sb::version) + sizeof(size_t);
            }

            // standart c
            else if (j == sizeof(std::string) * 7 + sizeof(var::struct_sb::version) + sizeof(size_t)) {
                serel_target_tmp += std::to_string(*(i32t *)(prj_v + j)) + " " +
                                    std::to_string(*(i32t *)(prj_v + j + sizeof(i32t))) + " " +
                                    std::to_string(*(bool *)(prj_v + j + sizeof(i32t) * 2)) + " ";
                break;
            }
            if ((*(std::string *)(prj_v + j)).find(" ") != serel_target_tmp.npos)
                serel_target_tmp += "\"" + *(std::string *)(prj_v + j) + "\" ";
            else
                serel_target_tmp += *(std::string *)(prj_v + j) + " ";
        }
        for (u32t j = 0; j < out_targets[i].prj.src_files.size(); ++j)
            serel_target_tmp += out_targets[i].prj.src_files[j] + " ";
        for (u32t j = 0; j < out_targets[i].prj.vec_templates.size(); ++j) {
            used_templates.emplace(out_targets[i].prj.vec_templates[j]);
            serel_target_tmp += out_targets[i].prj.vec_templates[j] + " ";
        }

        serel_target_tmp += var::struct_sb::target_t_str(out_targets[i].target_t) + " " +
                            var::struct_sb::cfg_str(out_targets[i].target_cfg) + " " + out_targets[i].name_target +
                            " " + out_targets[i].name_generator + " " +
                            out_targets[i].version_target.get_str_version() + " ";
        for (u32t j = 0; j < out_targets[i].target_vec_libs.size(); ++j)
            serel_target_tmp += out_targets[i].target_vec_libs[j] + " ";
    }

    serel_target_tmp += "EOET" + std::to_string(_cache_data.templates.size()) + " "; // end of enum targets

    for (u32t i = 0; i < _cache_data.templates.size(); ++i) {
        serel_target_tmp += std::to_string(_cache_data.templates[i].name_accept_params.size()) + " " +
                            std::to_string(_cache_data.templates[i].args.size()) + " " + _cache_data.templates[i].name +
                            " " + _cache_data.templates[i].name_call_component + " " +
                            _cache_data.templates[i].returnable + " ";

        all_used_call_component.emplace(_cache_data.templates[i].name_call_component);

        for (u32t j = 0; j < _cache_data.templates[i].name_accept_params.size(); ++j)
            serel_target_tmp += _cache_data.templates[i].name_accept_params[j] + " ";
        for (u32t j = 0; j < _cache_data.templates[i].args.size(); ++j) {
            if (_cache_data.templates[i].args[j].arg_t == var::struct_sb::template_command::arg::type::extglobal)
                all_used_globally_args.emplace(_cache_data.templates[i].args[j].str_arg);
            else if (_cache_data.templates[i].args[j].arg_t == var::struct_sb::template_command::arg::type::string)
                serel_target_tmp += "\"" + _cache_data.templates[i].args[j].str_arg + "\" ";
            else
                serel_target_tmp += _cache_data.templates[i].args[j].str_arg + " ";
            serel_target_tmp += std::to_string((i32t)_cache_data.templates[i].args[j].arg_t) + " ";
        }
    }

    serel_target_tmp += std::to_string(all_used_call_component.size()) + " ";

    for (const auto &c_component : all_used_call_component) {
        const auto &ref_c_component =
            find_if(_cache_data.call_components.begin(), _cache_data.call_components.end(),
                    [c_component](const var::struct_sb::call_component &ccmp) { return ccmp.name == c_component; });
        serel_target_tmp += ref_c_component->name + " " + ref_c_component->name_program + " " +
                            ref_c_component->pattern_ret_files + " ";
    }

    serel_target_tmp += std::to_string(all_used_globally_args.size()) + " ";

    for (const auto &g_arg : all_used_globally_args) {
        const auto &g_arg_it =
            std::find_if(_cache_data.global_external_args.begin(), _cache_data.global_external_args.end(),
                         [g_arg](const std::pair<std::string, std::string> &global_external_arg) {
                             return global_external_arg.first == g_arg;
                         });
        serel_target_tmp += g_arg_it->first + "-\"" + g_arg_it->second + "\" ";
    }

    return serel_target_tmp;
}

const base_bwcache::cache_data &fast_bwcache::get_cache_data(std::string cache_str) {
    var::struct_sb::target_out trg_tmp;
    var::struct_sb::template_command tcmd_tmp;
    var::struct_sb::template_command::arg arg_tmp;
    var::struct_sb::call_component ccmp_tmp;

    std::string str_tmp;

    i32t count_word = 0, offset_byte_prj = 0, offset_byte_ccmp = 0;
    i32t size_src_files = 0, size_vec_templates = 0, size_vec_libs = 0;
    i32t size_templates = 0;
    i32t size_internal_args = 0, size_external_args = 0, size_call_components = 0, size_global_extern_args = 0;

    char *tproj_p = (char *)&trg_tmp.prj;
    char *ccmp_p = (char *)&ccmp_tmp;

    bool open_sk = 0;
    bool enum_templates = 0;
    bool enum_call_component = 0;
    bool enum_global_extern_args = 0;

    bool expected_arg_param_str = 1;

    try {
        for (i32t i = 0; i < cache_str.size(); ++i) {
            if (cache_str[i] == '\"') {
                if (!open_sk)
                    open_sk = 1;
                else
                    open_sk = 0;
                continue;
            }

            if (cache_str[i] == ' ' && !open_sk) {
            next_word:
                ++count_word;
                if (enum_global_extern_args) {
                    if (!size_global_extern_args)
                        break;
                    std::string name_arg = str_tmp;
                    name_arg.erase(name_arg.find("-"));
                    str_tmp.erase(0, str_tmp.find("-") + 1);
                    _cache_data.global_external_args.push_back(std::pair<std::string, std::string>(name_arg, str_tmp));
                }
                else if (enum_call_component) {
                    if (offset_byte_ccmp / sizeof(std::string) == 3) {
                        _cache_data.call_components.push_back(ccmp_tmp);
                        offset_byte_ccmp = 0;

                        --size_call_components;

                        if (!size_call_components) {
                            enum_global_extern_args = 1;
                            enum_call_component = 0;

                            size_global_extern_args = std::stoi(str_tmp);
                            goto next;
                        }
                    }
                    *(std::string *)(ccmp_p + offset_byte_ccmp) = str_tmp;
                    offset_byte_ccmp += sizeof(std::string);
                }
                else if (enum_templates) {
                    if (count_word == 1)
                        size_internal_args = std::stoi(str_tmp);
                    else if (count_word == 2)
                        size_external_args = std::stoi(str_tmp);
                    else if (count_word == 3)
                        tcmd_tmp.name = str_tmp;
                    else if (count_word == 4)
                        tcmd_tmp.name_call_component = str_tmp;
                    else if (count_word == 5)
                        tcmd_tmp.returnable = str_tmp;
                    else if (count_word >= 6 && count_word < 6 + size_internal_args)
                        tcmd_tmp.name_accept_params.push_back(str_tmp);
                    else if ((count_word >= 6 + size_internal_args &&
                              count_word < 6 + size_internal_args + (size_external_args * 2)) ||
                             count_word == 6 + size_internal_args) {
                        if (expected_arg_param_str) {
                            arg_tmp.str_arg = str_tmp;
                            expected_arg_param_str = 0;
                        }
                        else {
                            arg_tmp.arg_t = (var::struct_sb::template_command::arg::type)std::stoi(str_tmp);
                            tcmd_tmp.args.push_back(arg_tmp);
                            expected_arg_param_str = 1;
                        }
                    }
                    else if (count_word == 6 + size_internal_args + (size_external_args * 2)) {
                        _cache_data.templates.push_back(tcmd_tmp);
                        --size_templates;

                        tcmd_tmp.name_accept_params = {};
                        tcmd_tmp.args = {};

                        if (!size_templates) {
                            enum_call_component = 1;
                            enum_templates = 0;

                            size_call_components = std::stoi(str_tmp);
                        }
                        else {
                            count_word = 0;
                            goto next_word;
                        }
                    }
                }
                else {
                    if (count_word == 1)
                        size_src_files = std::stoi(str_tmp);
                    else if (count_word == 2)
                        size_vec_templates = std::stoi(str_tmp);
                    else if (count_word == 3)
                        size_vec_libs = std::stoi(str_tmp);
                    else if (count_word > 3) {
                        if (count_word == 6 || (count_word >= 13 && count_word <= 15)) {
                            *(i32t *)(tproj_p + offset_byte_prj) = std::atoll(str_tmp.c_str());
                            offset_byte_prj += sizeof(i32t);
                        }
                        else if (count_word == 5) {
                            trg_tmp.prj.version_project = str_tmp;
                            offset_byte_prj += sizeof(var::struct_sb::version);
                        }
                        else {

                            if (count_word >= 16 && count_word < 16 + size_src_files)
                                trg_tmp.prj.src_files.push_back(str_tmp);
                            else if (count_word >= 16 + size_src_files &&
                                     count_word < 16 + size_src_files + size_vec_templates) {
                                if (str_tmp != "null")
                                    trg_tmp.prj.vec_templates.push_back(str_tmp);
                            }
                            else if (count_word == 16 + size_src_files + size_vec_templates)
                                trg_tmp.target_t = var::struct_sb::to_type_target(str_tmp);
                            else if (count_word == 16 + size_src_files + size_vec_templates + 1)
                                trg_tmp.target_cfg = var::struct_sb::to_cfg(str_tmp);
                            else if (count_word == 16 + size_src_files + size_vec_templates + 2)
                                trg_tmp.name_target = str_tmp;
                            else if (count_word == 16 + size_src_files + size_vec_templates + 3)
                                trg_tmp.name_generator = str_tmp;
                            else if (count_word == 16 + size_src_files + size_vec_templates + 4)
                                trg_tmp.version_target = str_tmp;
                            else if (count_word >= 16 + size_src_files + size_vec_templates + 5 &&
                                     count_word < 16 + size_src_files + size_vec_templates + 5 + size_vec_libs)
                                trg_tmp.target_vec_libs.push_back(str_tmp);
                            else if (count_word == 16 + size_src_files + size_vec_templates + 5 + size_vec_libs) {
                                _cache_data.targets_o.push_back(trg_tmp);

                                trg_tmp.target_vec_libs = {};
                                trg_tmp.prj.src_files = {};
                                trg_tmp.prj.vec_templates = {};

                                offset_byte_prj = 0;
                                count_word = 0;

                                if (str_tmp.find("EOET") != str_tmp.npos) {
                                    str_tmp.erase(0, str_tmp.find("EOET") + 4);

                                    size_templates = std::stoi(str_tmp);

                                    enum_templates = 1;
                                }
                                else
                                    goto next_word;
                            }
                            else {
                                *(std::string *)(tproj_p + offset_byte_prj) = str_tmp;
                                offset_byte_prj += sizeof(std::string);
                            }
                        }
                    }
                }

            next:
                str_tmp.clear();
                continue;
            }
            str_tmp += cache_str[i];
        }
    }
    catch (const std::logic_error &_excp) {
        assist.next_output_unsuccess();
        throw bwcache_excp("", "000");
    }

    return _cache_data;
}