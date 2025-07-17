//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwcache_api.hpp>

using namespace bweas;
using namespace cache_api;

static logger _log{"BWCACHE[FAST]"};

string fast_cache::create_cache() {
    string serel_target_tmp;

    uset<string> used_templates;
    uset<string> all_used_globally_args;
    uset<string> all_used_call_component;

    auto &targets                    = _context->targets;
    const auto &templates            = _context->templates;
    const auto &call_components      = _context->call_components;
    const auto &global_external_args = _context->global_external_args;

    serel_target_tmp += "\"" + _context->path_bweas_config + "\" ";

    for (pdiff i = 0; i < targets.size(); ++i) {
        if (!targets[i].templates.size())
            targets[i].templates.push_back("null");
        if (!targets[i].prj.custom_ext_fields.size())
            targets[i].prj.custom_ext_fields["null"] = "";
        if (!targets[i].prj.libs.size())
            targets[i].prj.libs.push_back("null");
        if (!targets[i].dependencies.size())
            targets[i].dependencies.push_back("null");

        char *prj_v = (char *)&targets[i];
        serel_target_tmp +=
            std::to_string(targets[i].prj.src_files.size()) + " " + std::to_string(targets[i].prj.libs.size()) + " " +
            std::to_string(targets[i].prj.include_paths.size()) + " " +
            std::to_string(targets[i].prj.custom_ext_fields.size()) + " " +
            std::to_string(targets[i].templates.size()) + " " + std::to_string(targets[i].dependencies.size()) + " ";
        for (pdiff j = 0; j < sizeof(sc::project); j += sizeof(string)) {
            // standart c
            if (j == sizeof(string) * 7) {
                serel_target_tmp += std::to_string(*(pdiff *)(prj_v + j)) + " " +
                                    std::to_string(*(pdiff *)(prj_v + j + sizeof(pdiff))) + " ";
                break;
            }
            if ((*(string *)(prj_v + j)).find(" ") != serel_target_tmp.npos)
                serel_target_tmp += "\"" + *(string *)(prj_v + j) + "\" ";
            else
                serel_target_tmp += *(string *)(prj_v + j) + " ";
        }
        for (size_t j = 0; j < targets[i].prj.src_files.size(); ++j)
            serel_target_tmp += targets[i].prj.src_files[j] + " ";
        for (size_t j = 0; j < targets[i].prj.libs.size(); ++j)
            serel_target_tmp += targets[i].prj.libs[j] + " ";
        for (size_t j = 0; j < targets[i].prj.include_paths.size(); ++j)
            serel_target_tmp += targets[i].prj.include_paths[j] + " ";

        for (auto it : targets[i].prj.custom_ext_fields)
            serel_target_tmp += it.first + " " + it.second + " ";

        serel_target_tmp.erase(serel_target_tmp.size() - 1, 1);

        serel_target_tmp += sc::target_type_str(targets[i].type) + " " + sc::target_cfg_str(targets[i].cfg) + " " +
                            targets[i].name + " " + targets[i].name_generator + " " + targets[i].ver.get_str_version() +
                            " ";
        for (size_t j = 0; j < targets[i].templates.size(); ++j) {
            used_templates.emplace(targets[i].templates[j]);
            serel_target_tmp += targets[i].templates[j] + " ";
        }
        for (size_t j = 0; j < targets[i].dependencies.size(); ++j)
            serel_target_tmp += targets[i].dependencies[j] + " ";
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
            if (templates[i].args[j].arg_t == sc::template_command::arg::type::extglobal)
                all_used_globally_args.emplace(templates[i].args[j].str_arg);

            serel_target_tmp += "\"" + templates[i].args[j].str_arg + "\" ";
            serel_target_tmp += std::to_string((pdiff)templates[i].args[j].arg_t) + " ";
        }
    }

    serel_target_tmp += std::to_string(all_used_call_component.size()) + " ";

    for (const auto &call_component : all_used_call_component) {
        const auto &ref_call_component =
            find_if(call_components.begin(), call_components.end(),
                    [call_component](const sc::call_component &ccmp) { return ccmp.name == call_component; });
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
string fast_cache::get_path_config(const string &cache_str) {
    try {
        string str_tmp = cache_str;
        str_tmp.erase(0, 1);
        str_tmp.erase(str_tmp.find("\""));
        return str_tmp;
    }
    catch (const std::logic_error &_excp) {
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Invalid structure of the bweas cache file");
    }
}
void fast_cache::extract_cache_data(const string &cache_str) {
    sc::target trg_tmp;
    sc::template_command tcmd_tmp;
    sc::template_command::arg arg_tmp;
    sc::call_component ccmp_tmp;

    string str_tmp, str_tmp_key;

    pdiff count_words_before_first_list = 16;

    pdiff count_word = 0, offset_byte_prj = 0, offset_byte_ccmp = 0;
    pdiff size_src_files = 0, size_libs = 0, size_include_paths, size_custom_ext_fields, size_use_templates = 0,
          size_dependencies  = 0;
    pdiff size_templates     = 0;
    pdiff size_internal_args = 0, size_external_args = 0, size_call_components = 0, size_global_extern_args = 0;

    pdiff end_src_files = 0, end_libs = 0, end_include_paths = 0, end_custom_ext_fields = 0, end_templates = 0,
          end_dependencies = 0;

    char *tproj_p = (char *)&trg_tmp.prj;
    char *ccmp_p  = (char *)&ccmp_tmp;

    bool is_beg_file         = 1;
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
                if (is_beg_file) {
                    _context->path_bweas_config = str_tmp;

                    is_beg_file = 0;
                    goto next;
                }
                ++count_word;
                if (enum_global_extern_args) {
                    if (!size_global_extern_args)
                        break;
                    string name_arg = str_tmp;
                    name_arg.erase(name_arg.find("-"));
                    str_tmp.erase(0, str_tmp.find("-") + 1);
                    _context->global_external_args.push_back(pair<string, string>(name_arg, str_tmp));
                }
                else if (enum_call_component) {
                    if (offset_byte_ccmp / sizeof(string) == 3) {
                        _context->call_components.push_back(ccmp_tmp);
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
                            arg_tmp.arg_t = (sc::template_command::arg::type)std::stoi(str_tmp);
                            tcmd_tmp.args.push_back(arg_tmp);
                            expected_arg_param_str = 1;
                        }
                    }
                    else if (count_word == 6 + size_internal_args + (size_external_args * 2)) {
                        _context->templates.push_back(tcmd_tmp);
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
                        size_libs = std::stoi(str_tmp);
                    else if (count_word == 3)
                        size_include_paths = std::stoi(str_tmp);
                    else if (count_word == 4)
                        size_custom_ext_fields = std::stoi(str_tmp);
                    else if (count_word == 5)
                        size_use_templates = std::stoi(str_tmp);
                    else if (count_word == 6) {
                        size_dependencies = std::stoi(str_tmp);

                        end_src_files         = count_words_before_first_list + size_src_files;
                        end_libs              = end_src_files + size_libs;
                        end_include_paths     = end_libs + size_include_paths;
                        end_custom_ext_fields = end_include_paths + size_custom_ext_fields;
                        end_templates         = end_custom_ext_fields + size_use_templates;
                        end_dependencies      = end_templates + size_dependencies;
                    }
                    else if (count_word == 14 || count_word == 15) {
                        *(pdiff *)(tproj_p + offset_byte_prj) = std::atoll(str_tmp.c_str());
                        offset_byte_prj += sizeof(pdiff);
                    }
                    else if (count_word >= count_words_before_first_list) {
                        if (count_word < end_src_files)
                            trg_tmp.prj.src_files.push_back(str_tmp);
                        else if (count_word < end_libs) {
                            if (str_tmp != "null")
                                trg_tmp.prj.libs.push_back(str_tmp);
                        }
                        else if (count_word < end_include_paths) {
                            if (str_tmp != "null")
                                trg_tmp.prj.include_paths.push_back(str_tmp);
                        }

                        else if (count_word < end_custom_ext_fields) {
                            if (str_tmp == "null")
                                ;
                            else if ((is_beg_custom_field = !is_beg_custom_field)) {
                                trg_tmp.prj.custom_ext_fields[str_tmp] = "";
                                str_tmp_key                            = str_tmp;
                                --count_word;
                            }
                            else
                                trg_tmp.prj.custom_ext_fields.find(str_tmp_key)->second = str_tmp;
                        }
                        else if (count_word == end_custom_ext_fields)
                            trg_tmp.type = sc::to_target_type(str_tmp);
                        else if (count_word == end_custom_ext_fields + 1)
                            trg_tmp.cfg = sc::to_target_cfg(str_tmp);
                        else if (count_word == end_custom_ext_fields + 2)
                            trg_tmp.name = str_tmp;
                        else if (count_word == end_custom_ext_fields + 3)
                            trg_tmp.name_generator = str_tmp;
                        else if (count_word == end_custom_ext_fields + 4)
                            trg_tmp.ver = str_tmp;
                        else if (count_word < end_templates + 5) {
                            if (str_tmp != "null")
                                trg_tmp.templates.push_back(str_tmp);
                        }
                        else if (count_word < end_dependencies + 5)
                            trg_tmp.dependencies.push_back(str_tmp);
                        else if (count_word == end_dependencies + 5) {
                            _context->targets.push_back(trg_tmp);

                            trg_tmp.templates         = {};
                            trg_tmp.dependencies      = {};
                            trg_tmp.prj.src_files     = {};
                            trg_tmp.prj.libs          = {};
                            trg_tmp.prj.include_paths = {};

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
                    }
                    else {
                        *(string *)(tproj_p + offset_byte_prj) = str_tmp;
                        offset_byte_prj += sizeof(string);
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

