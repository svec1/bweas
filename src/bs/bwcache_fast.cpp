//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <unordered_set>

#include "bwcache_api.hpp"

using namespace bweas;
using namespace cache_api;

static logger _log{"BWCACHE[FAST]"};

fast_cache::fast_cache(bw_context *const _context) : base_cache(_context) {
    if (!context)
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Bweas the context is not defined");
}

string fast_cache::create_cache() {
    string serel_target_tmp;

    uset<string> used_templates;
    uset<string> all_used_globally_args;
    uset<string> all_used_call_component;

    auto &out_targets                = context->out_targets;
    const auto &templates            = context->templates;
    const auto &call_components      = context->call_components;
    const auto &global_external_args = context->global_external_args;

    for (pdiff i = 0; i < out_targets.size(); ++i) {
        if (!out_targets[i].prj.vec_templates.size())
            out_targets[i].prj.vec_templates.push_back("null");
        if (!out_targets[i].prj.custom_ext_fields.size())
            out_targets[i].prj.custom_ext_fields["null"] = "";
        if (!out_targets[i].target_vec_libs.size())
            out_targets[i].target_vec_libs.push_back("null");

        char *prj_v = (char *)&out_targets[i];
        serel_target_tmp += std::to_string(out_targets[i].prj.src_files.size()) + " " +
                            std::to_string(out_targets[i].prj.include_paths.size()) + " " +
                            std::to_string(out_targets[i].prj.vec_templates.size()) + " " +
                            std::to_string(out_targets[i].prj.custom_ext_fields.size()) + " " +
                            std::to_string(out_targets[i].target_vec_libs.size()) + " ";
        for (pdiff j = 0; j < sizeof(var::struct_sb::project); j += sizeof(string)) {
            // version project
            if (j == sizeof(string)) {
                serel_target_tmp += (*(var::struct_sb::version *)(prj_v + j)).get_str_version() + " " +
                                    *(string *)(prj_v + j + sizeof(var::struct_sb::version)) + " ";
                j += sizeof(var::struct_sb::version) + sizeof(string);
            }

            // standart c
            else if (j == sizeof(string) * 7 + sizeof(var::struct_sb::version) + sizeof(string)) {
                serel_target_tmp += std::to_string(*(pdiff *)(prj_v + j)) + " " +
                                    std::to_string(*(pdiff *)(prj_v + j + sizeof(pdiff))) + " ";
                break;
            }
            if ((*(string *)(prj_v + j)).find(" ") != serel_target_tmp.npos)
                serel_target_tmp += "\"" + *(string *)(prj_v + j) + "\" ";
            else
                serel_target_tmp += *(string *)(prj_v + j) + " ";
        }
        for (size_t j = 0; j < out_targets[i].prj.src_files.size(); ++j)
            serel_target_tmp += out_targets[i].prj.src_files[j] + " ";
        for (size_t j = 0; j < out_targets[i].prj.include_paths.size(); ++j)
            serel_target_tmp += out_targets[i].prj.include_paths[j] + " ";
        for (size_t j = 0; j < out_targets[i].prj.vec_templates.size(); ++j) {
            used_templates.emplace(out_targets[i].prj.vec_templates[j]);
            serel_target_tmp += out_targets[i].prj.vec_templates[j] + " ";
        }
        for (auto it : out_targets[i].prj.custom_ext_fields)
            serel_target_tmp += it.first + " " + it.second + " ";

        serel_target_tmp += var::struct_sb::target_t_str(out_targets[i].target_t) + " " +
                            var::struct_sb::cfg_str(out_targets[i].target_cfg) + " " + out_targets[i].name_target +
                            " " + out_targets[i].name_generator + " " +
                            out_targets[i].version_target.get_str_version() + " ";
        for (size_t j = 0; j < out_targets[i].target_vec_libs.size(); ++j)
            serel_target_tmp += out_targets[i].target_vec_libs[j] + " ";
    }

    serel_target_tmp += "EOET" + std::to_string(templates.size()) + " "; // end of enum targets

    for (size_t i = 0; i < templates.size(); ++i) {
        serel_target_tmp += std::to_string(templates[i].name_accept_params.size()) + " " +
                            std::to_string(templates[i].args.size()) + " " + templates[i].name + " " +
                            templates[i].name_call_component + " " + templates[i].returnable + " ";

        all_used_call_component.emplace(templates[i].name_call_component);

        for (size_t j = 0; j < templates[i].name_accept_params.size(); ++j)
            serel_target_tmp += templates[i].name_accept_params[j] + " ";
        for (size_t j = 0; j < templates[i].args.size(); ++j) {
            if (templates[i].args[j].arg_t == var::struct_sb::template_command::arg::type::extglobal)
                all_used_globally_args.emplace(templates[i].args[j].str_arg);
            else if (templates[i].args[j].arg_t == var::struct_sb::template_command::arg::type::string)
                serel_target_tmp += "\"" + templates[i].args[j].str_arg + "\" ";
            else
                serel_target_tmp += templates[i].args[j].str_arg + " ";
            serel_target_tmp += std::to_string((pdiff)templates[i].args[j].arg_t) + " ";
        }
    }

    serel_target_tmp += std::to_string(all_used_call_component.size()) + " ";

    for (const auto &call_component : all_used_call_component) {
        const auto &ref_call_component = find_if(
            call_components.begin(), call_components.end(),
            [call_component](const var::struct_sb::call_component &ccmp) { return ccmp.name == call_component; });
        serel_target_tmp += ref_call_component->name + " \"" + ref_call_component->name_program + "\" " +
                            ref_call_component->pattern_ret_files + " ";
    }

    serel_target_tmp += std::to_string(all_used_globally_args.size()) + " ";

    for (const auto &g_arg : all_used_globally_args) {
        const auto &g_arg_it = std::find_if(
            global_external_args.begin(), global_external_args.end(),
            [g_arg](const pair<string, string> &global_external_arg) { return global_external_arg.first == g_arg; });
        serel_target_tmp += g_arg_it->first + "-\"" + g_arg_it->second + "\" ";
    }

    return serel_target_tmp;
}

void fast_cache::extract_cache_data(string &&cache_str) {
    var::struct_sb::target_out trg_tmp;
    var::struct_sb::template_command tcmd_tmp;
    var::struct_sb::template_command::arg arg_tmp;
    var::struct_sb::call_component ccmp_tmp;

    string str_tmp, str_tmp_key;

    pdiff count_word = 0, offset_byte_prj = 0, offset_byte_ccmp = 0;
    pdiff size_src_files = 0, size_include_paths, size_vec_templates = 0, size_custom_ext_fields, size_vec_libs = 0;
    pdiff size_templates     = 0;
    pdiff size_internal_args = 0, size_external_args = 0, size_call_components = 0, size_global_extern_args = 0;

    char *tproj_p = (char *)&trg_tmp.prj;
    char *ccmp_p  = (char *)&ccmp_tmp;

    bool is_beg_custom_field = 0;

    bool open_sk                 = 0;
    bool enum_templates          = 0;
    bool enum_call_component     = 0;
    bool enum_global_extern_args = 0;

    bool expected_arg_param_str = 1;

    try {
        for (pdiff i = 0; i < cache_str.size(); ++i) {
            if (cache_str[i] == '\"') {
                open_sk = !open_sk;
                continue;
            }

            if (cache_str[i] == ' ' && !str_tmp.empty() && !open_sk) {
            next_word:
                ++count_word;
                if (enum_global_extern_args) {
                    if (!size_global_extern_args)
                        break;
                    string name_arg = str_tmp;
                    name_arg.erase(name_arg.find("-"));
                    str_tmp.erase(0, str_tmp.find("-") + 1);
                    context->global_external_args.push_back(pair<string, string>(name_arg, str_tmp));
                }
                else if (enum_call_component) {
                    if (offset_byte_ccmp / sizeof(string) == 3) {
                        context->call_components.push_back(ccmp_tmp);
                        offset_byte_ccmp = 0;

                        --size_call_components;

                        if (!size_call_components) {
                            enum_global_extern_args = 1;
                            enum_call_component     = 0;

                            size_global_extern_args = std::stoi(str_tmp);
                            goto next;
                        }
                    }
                    *(string *)(ccmp_p + offset_byte_ccmp) = str_tmp;
                    offset_byte_ccmp += sizeof(string);
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
                            arg_tmp.str_arg        = str_tmp;
                            expected_arg_param_str = 0;
                        }
                        else {
                            arg_tmp.arg_t = (var::struct_sb::template_command::arg::type)std::stoi(str_tmp);
                            tcmd_tmp.args.push_back(arg_tmp);
                            expected_arg_param_str = 1;
                        }
                    }
                    else if (count_word == 6 + size_internal_args + (size_external_args * 2)) {
                        context->templates.push_back(tcmd_tmp);
                        --size_templates;

                        tcmd_tmp.name_accept_params = {};
                        tcmd_tmp.args               = {};

                        if (!size_templates) {
                            enum_call_component = 1;
                            enum_templates      = 0;

                            size_call_components = std::stoi(str_tmp);
                        }
                        else {
                            count_word = 0;
                            goto next_word;
                        }
                    }
                }
                else {
                    if (count_word == 1) {
                        trg_tmp.prj.include_paths.clear();
                        size_src_files = std::stoi(str_tmp);
                    }
                    else if (count_word == 2)
                        size_include_paths = std::stoi(str_tmp);
                    else if (count_word == 3)
                        size_vec_templates = std::stoi(str_tmp);
                    else if (count_word == 4)
                        size_custom_ext_fields = std::stoi(str_tmp);
                    else if (count_word == 5)
                        size_vec_libs = std::stoi(str_tmp);
                    else if (count_word > 6) {
                        if (count_word == 16) {
                            *(pdiff *)(tproj_p + offset_byte_prj) = std::atoll(str_tmp.c_str());
                            offset_byte_prj += sizeof(pdiff);
                        }
                        else if (count_word == 7) {
                            trg_tmp.prj.version_project = str_tmp;
                            offset_byte_prj += sizeof(var::struct_sb::version);
                        }
                        else if (count_word == 8) {
                            trg_tmp.prj.language = str_tmp;
                            offset_byte_prj += sizeof(string);
                        }
                        else {
                            if (count_word >= 17 && count_word < 17 + size_src_files)
                                trg_tmp.prj.src_files.push_back(str_tmp);
                            else if (count_word >= 17 + size_src_files &&
                                     count_word < 17 + size_src_files + size_include_paths) {
                                if (str_tmp != "null")
                                    trg_tmp.prj.include_paths.push_back(str_tmp);
                            }
                            else if (count_word >= 17 + size_src_files + size_include_paths &&
                                     count_word < 17 + size_src_files + size_include_paths + size_vec_templates) {
                                if (str_tmp != "null")
                                    trg_tmp.prj.vec_templates.push_back(str_tmp);
                            }
                            else if (count_word >= 17 + size_src_files + size_include_paths + size_vec_templates &&
                                     count_word < 17 + size_src_files + size_include_paths + size_vec_templates +
                                                      size_custom_ext_fields) {
                                if (str_tmp == "null")
                                    ;
                                else if (is_beg_custom_field = !is_beg_custom_field) {
                                    trg_tmp.prj.custom_ext_fields[str_tmp] = "";
                                    str_tmp_key                            = str_tmp;
                                    --count_word;
                                }
                                else
                                    trg_tmp.prj.custom_ext_fields.find(str_tmp_key)->second = str_tmp;
                            }
                            else if (count_word == 17 + size_src_files + size_include_paths + size_vec_templates +
                                                       size_custom_ext_fields)
                                trg_tmp.target_t = var::struct_sb::to_type_target(str_tmp);
                            else if (count_word == 17 + size_src_files + size_include_paths + size_vec_templates +
                                                       size_custom_ext_fields + 1)
                                trg_tmp.target_cfg = var::struct_sb::to_cfg(str_tmp);
                            else if (count_word == 17 + size_src_files + size_include_paths + size_vec_templates +
                                                       size_custom_ext_fields + 2)
                                trg_tmp.name_target = str_tmp;
                            else if (count_word == 17 + size_src_files + size_include_paths + size_vec_templates +
                                                       size_custom_ext_fields + 3)
                                trg_tmp.name_generator = str_tmp;
                            else if (count_word == 17 + size_src_files + size_include_paths + size_vec_templates +
                                                       size_custom_ext_fields + 4)
                                trg_tmp.version_target = str_tmp;
                            else if (count_word >= 17 + size_src_files + size_include_paths + size_vec_templates +
                                                       size_custom_ext_fields + 5 &&
                                     count_word < 17 + size_src_files + size_include_paths + size_vec_templates +
                                                      size_custom_ext_fields + 5 + size_vec_libs)
                                trg_tmp.target_vec_libs.push_back(str_tmp);
                            else if (count_word == 17 + size_src_files + size_include_paths + size_vec_templates +
                                                       size_custom_ext_fields + 5 + size_vec_libs) {
                                context->out_targets.push_back(trg_tmp);

                                trg_tmp.target_vec_libs   = {};
                                trg_tmp.prj.src_files     = {};
                                trg_tmp.prj.vec_templates = {};

                                offset_byte_prj = 0;
                                count_word      = 0;

                                if (str_tmp.find("EOET") != str_tmp.npos) {
                                    str_tmp.erase(0, str_tmp.find("EOET") + 4);

                                    size_templates = std::stoi(str_tmp);

                                    enum_templates = 1;
                                }
                                else
                                    goto next_word;
                            }
                            else {
                                *(string *)(tproj_p + offset_byte_prj) = str_tmp;
                                offset_byte_prj += sizeof(string);
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
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Invalid structure of the bweas cache file");
    }
}
