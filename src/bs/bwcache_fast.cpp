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

string fast_cache::create_cache() const {
    string serel_target_tmp;

    uset<string> used_templates;
    uset<string> all_used_call_component;

    auto &targets               = _context->targets;
    const auto &templates       = _context->templates;
    const auto &call_components = _context->call_components;

    serel_target_tmp += "\"" + _context->path_bweas_config.string() + "\" ";

    for (pdiff i = 0; i < targets.size(); ++i) {

        sc::profile::fields &ext_fields = targets[i].ext.get_fields();

        serel_target_tmp += std::to_string(ext_fields.size()) + " " + std::to_string(targets[i].templates.size()) +
                            " " + std::to_string(targets[i].dependencies.size()) + " ";
        serel_target_tmp += targets[i].ext.lang.name + " " + targets[i].ext.lang.dfinder_data.search_regex + " " +
                            string(1, targets[i].ext.lang.dfinder_data.char_global_search) + " ";

        for (const auto &[key, value] : ext_fields) {
            serel_target_tmp += key + " ";
            if (std::holds_alternative<pdiff>(value))
                serel_target_tmp += "1" + string(" ") + std::to_string(std::get<pdiff>(value)) + " ";
            else if (std::holds_alternative<string>(value))
                serel_target_tmp += "\"" + std::get<string>(value) + "\" ";
            else {
                const auto &str_s = std::get<vec<string>>(value);
                serel_target_tmp += std::to_string(str_s.size()) + " ";
                for (const auto &str : str_s)
                    serel_target_tmp += "\"" + str + "\" ";
            }
        }

        serel_target_tmp += targets[i].name + " " + targets[i].ver.get_str_version() + " ";
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
                            templates[i].name_call_component + " " + templates[i].returnable.value + " " +
                            std::to_string((pdiff)templates[i].returnable.type) + " ";

        all_used_call_component.emplace(templates[i].name_call_component);

        for (size_t j = 0; j < templates[i].name_accept_params.size(); ++j)
            serel_target_tmp += templates[i].name_accept_params[j] + " ";
        for (size_t j = 0; j < templates[i].args.size(); ++j) {
            serel_target_tmp +=
                "\"" + (templates[i].args[j].prefix.empty() ? string("null") : templates[i].args[j].prefix) + "\" " +
                "\"" + templates[i].args[j].value + "\" " + std::to_string((pdiff)templates[i].args[j].type) + " ";
        }
    }

    serel_target_tmp += std::to_string(all_used_call_component.size()) + " ";

    for (const auto &call_component : all_used_call_component) {
        if (const auto &it =
                find_if(call_components.begin(), call_components.end(),
                        [call_component](const sc::call_component &ccmp) { return ccmp.name == call_component; });
            it != call_components.end())
            serel_target_tmp += it->name + " \"" + it->name_program + "\" " + it->pattern_ret_files + " ";
        else
            _log << (log_message(log_type::fatal)
                     << "It is impossible to get a used call_component \'" + call_component + "\'.");
    }
    serel_target_tmp += "EOECC "; // end of enum targets

    return serel_target_tmp;
}
string fast_cache::get_path_config(const string &cache_str) const {
    try {
        string str_tmp = cache_str;
        str_tmp.erase(0, 1);
        str_tmp.erase(str_tmp.find("\""));
        return str_tmp;
    }
    catch (const std::logic_error &_excp) {
        _log << (log_message(log_type::fatal) << "Invalid structure of the bweas cache file");
    }

    std::unreachable();
}
void fast_cache::extract_cache_data(const string &cache_str) const {
    sc::target trg_tmp;
    sc::template_command tcmd_tmp;
    sc::template_command::arg arg_tmp;
    sc::call_component ccmp_tmp;

    string str_tmp, str_tmp_key;

    pdiff count_words_before_first_list = 4;

    pdiff count_word = 0, offset_byte_ccmp = 0;
    pdiff size_ext_fields = 0, size_use_templates = 0, size_dependencies = 0;
    pdiff size_templates     = 0;
    pdiff size_internal_args = 0, size_external_args = 0, size_call_components = 0;

    pdiff end_extension = 0, end_templates = 0, end_dependencies = 0;
    pdiff lang_fields = 0;

    size_t count_el_field = 0;

    char *ccmp_p = (char *)&ccmp_tmp;

    bool is_beg_file  = 1;
    bool is_key_field = 0;

    bool open_sk             = 0;
    bool was_sk              = 0;
    bool enum_templates      = 0;
    bool enum_call_component = 0;

    bool expected_arg_prefix_str = 1;
    bool expected_arg_param_str  = 0;

    try {
        for (pdiff i = 0; i < cache_str.size(); ++i) {
            try {
                if (cache_str[i] == '\"' && ((!open_sk && str_tmp.empty()) || open_sk)) {
                    open_sk = !open_sk;
                    was_sk  = 1;
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
                    if (enum_call_component) {
                        if (offset_byte_ccmp / sizeof(string) == 3) {
                            _context->call_components.push_back(ccmp_tmp);
                            offset_byte_ccmp = 0;

                            --size_call_components;

                            if (!size_call_components) {
                                if (str_tmp != "EOECC" || i != cache_str.size() - 2)
                                    throw std::runtime_error("The end of the cache was expected.");
                                break;
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
                        else if (count_word == 5) {
                            if (is_key_field) {
                                tcmd_tmp.returnable.value = str_tmp;

                                is_key_field = 0;

                                --count_word;
                            }
                            else {
                                tcmd_tmp.returnable.type =
                                    (sc::template_command::return_value::e_type)std::stoi(str_tmp);

                                is_key_field = 1;
                            }
                        }
                        else if (count_word >= 6 && count_word < 6 + size_internal_args)
                            tcmd_tmp.name_accept_params.push_back(str_tmp);
                        else if (count_word >= 6 + size_internal_args &&
                                 count_word < 6 + size_internal_args + (size_external_args * 3)) {
                            if (expected_arg_prefix_str) {
                                if (str_tmp != "null")
                                    arg_tmp.prefix = str_tmp;
                                else
                                    arg_tmp.prefix = "";
                                expected_arg_prefix_str = 0;
                                expected_arg_param_str  = 1;
                            }
                            else if (expected_arg_param_str) {
                                arg_tmp.value          = str_tmp;
                                expected_arg_param_str = 0;
                            }
                            else {
                                arg_tmp.type = (sc::template_command::arg::e_type)std::stoi(str_tmp);
                                tcmd_tmp.args.push_back(arg_tmp);
                                expected_arg_prefix_str = 1;
                            }
                        }
                        else if (count_word == 6 + size_internal_args + (size_external_args * 3)) {
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
                        if (count_word == 1)
                            size_ext_fields = std::stoi(str_tmp);
                        else if (count_word == 2) {
                            size_use_templates = std::stoi(str_tmp);
                            end_templates      = size_use_templates;
                        }
                        else if (count_word == 3) {
                            size_dependencies = std::stoi(str_tmp);
                            end_dependencies  = size_dependencies;

                            end_extension = count_words_before_first_list;
                            lang_fields   = 3;
                        }
                        else if (count_word >= count_words_before_first_list) {
                            if (size_ext_fields) {
                                ++end_extension;

                                if (lang_fields) {
                                    if (lang_fields == 3)
                                        trg_tmp.ext.lang.name = str_tmp;
                                    else if (lang_fields == 2)
                                        trg_tmp.ext.lang.dfinder_data.search_regex = str_tmp;
                                    else if (lang_fields == 1) {
                                        trg_tmp.ext.lang.dfinder_data.char_global_search = str_tmp[0];

                                        is_key_field = 1;
                                    }

                                    --lang_fields;
                                    goto next;
                                }

                                if (is_key_field) {
                                    str_tmp_key  = str_tmp;
                                    is_key_field = 0;
                                }
                                else {
                                    if (count_el_field) {
                                        static vec<string> tmp_vec_values;
                                        if (count_el_field == 1 && std::isdigit(str_tmp[0])) {
                                            trg_tmp.ext.get_fields()[str_tmp_key] = std::atoll(str_tmp.c_str());
                                            --count_el_field;
                                            goto new_field;
                                        }
                                        else
                                            tmp_vec_values.push_back(str_tmp);

                                        --count_el_field;
                                        if (!count_el_field) {
                                            trg_tmp.ext.get_fields()[str_tmp_key] = tmp_vec_values;
                                            tmp_vec_values.clear();
                                        new_field:
                                            --size_ext_fields;
                                            is_key_field = 1;
                                        }
                                        goto next;
                                    }

                                    if (!was_sk) {
                                        count_el_field = std::atoll(str_tmp.c_str());
                                        if (!count_el_field) {
                                            trg_tmp.ext.get_fields()[str_tmp_key] = vec<string>();
                                            goto new_field;
                                        }
                                    }
                                    else {
                                        trg_tmp.ext.get_fields()[str_tmp_key] = str_tmp;
                                        goto new_field;
                                    }
                                }
                            }
                            else if (count_word == end_extension) {
                                end_templates += end_extension + 2;
                                end_dependencies += end_templates;

                                trg_tmp.name = str_tmp;
                            }
                            else if (count_word == end_extension + 1)
                                trg_tmp.ver = str_tmp;
                            else if (count_word < end_templates)
                                trg_tmp.templates.push_back(str_tmp);
                            else if (count_word < end_dependencies)
                                trg_tmp.dependencies.push_back(str_tmp);
                            else if (count_word == end_dependencies) {
                                _context->targets.push_back(trg_tmp);

                                trg_tmp.templates    = {};
                                trg_tmp.dependencies = {};

                                count_word = 0;

                                if (str_tmp.find("EOET") != str_tmp.npos) {
                                    str_tmp.erase(0, str_tmp.find("EOET") + 4);

                                    size_templates = std::stoi(str_tmp);

                                    enum_templates = 1;
                                }
                                else
                                    goto next_word;
                            }
                        }
                    }

                next:
                    str_tmp.clear();
                    was_sk = 0;
                    continue;
                }
                else if (cache_str[i] == ' ' && !open_sk)
                    continue;
                str_tmp += cache_str[i];
            }
            catch (std::logic_error &) {
                throw std::runtime_error(str_tmp);
            }
        }
    }
    catch (std::exception &_excp) {
        _log << (log_message(log_type::fatal)
                 << "Invalid structure of the bweas cache file(Last word: " << _excp.what() << ")");
    }
}

