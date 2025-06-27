//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#include <lang/semantic_an.hpp>

static constexpr auto NOT_MATCHING_W_PARAMETERS  = "The function values passed do not correspond to the expected ones.";
static constexpr auto INCORECT_DEFINITION_FUNC   = "Incorrect definition of function parameter types was observed";
static constexpr auto EXPECTED_NEW_SYMBOL        = "A new symbol was expected.";
static constexpr auto EXPECTED_EXIST_SYMBOL      = "Expected an existing symbol.";
static constexpr auto EXPECTED_ANOTHER_EXPR_TYPE = "Another type of expression was expected.";
static constexpr auto IMPOSSIBLE_DETERMINE_TYPE_VAR =
    "It's impossible to determine the type of a non-existent variable.";
static constexpr auto NOT_EXIST_VAR_W_EXPECTED_TYPE = "There is no variable with the expected type.";
static constexpr auto EXPECTED_IF                   = "The expected keyword was if";
static constexpr auto EXPECTED_ENDIF                = "The expected keyword was endif";
static constexpr auto INVALID_USAGE_ENDIF           = "The endif keyword is only expected after the if or else keyword";

semantic_analyzer::semantic_analyzer(logger &__log) : _log(__log) {
}

void semantic_analyzer::analysis(statements &stm_s, scope &current_scope) {
    smt_first_pass(stm_s, current_scope);
    smt_second_pass(stm_s, current_scope);
}

void semantic_analyzer::smt_first_pass(statements &st_s, scope &current_scope) {
    for (size_t i = 0; i < st_s.size(); ++i) {
        if (st_s[i].expr_func->expected_params.size() != st_s[i].expr_s.size()) {
            if ((st_s[i].expr_func->expected_params.size() != 0 &&
                 st_s[i].expr_func->expected_params[st_s[i].expr_func->expected_params.size() - 1].type ==
                     param_type::NEXT_TOO &&
                 st_s[i].expr_func->expected_params.size() - 1 <= st_s[i].expr_s.size()) ||
                (st_s[i].expr_func->expected_params[st_s[i].expr_func->expected_params.size() - 1].type ==
                         param_type::NEXT_TOO
                     ? (st_s[i].expr_func->expected_params.size() - st_s[i].expr_s.size() - 1 > 0 &&
                        st_s[i].expr_func->expected_params.size() - st_s[i].expr_s.size() - 1 ==
                            st_s[i].expr_func->count_default_params())
                     : (st_s[i].expr_func->expected_params.size() - st_s[i].expr_s.size() > 0 &&
                        st_s[i].expr_func->expected_params.size() - st_s[i].expr_s.size() ==
                            st_s[i].expr_func->count_default_params())))
                goto check_valid_exp_type;
            _log << bwtools::fatal
                 << (log_message(log_type::fatal)
                     << st_s[i].build_string_error(PTRDIFF_MAX, NOT_MATCHING_W_PARAMETERS,
                                                   st_s[i].get_string_expected_params(st_s[i].expr_s.size())));
        }
        else if (st_s[i].expr_s.size() == 0)
            continue;

    check_valid_exp_type:
        for (size_t j = 0, current_expected_param = 0;
             j < st_s[i].expr_s.size() || current_expected_param < st_s[i].expr_func->expected_params.size();
             ++j, ++current_expected_param) {
            if (st_s[i].expr_func->expected_params[current_expected_param].decl_default_val()) {
                if (j >= st_s[i].expr_s.size() ||
                    !(st_s[i].expr_s[j].type == st_s[i].expr_func->expected_params[current_expected_param].type)) {
                    st_s[i].expr_s.emplace(st_s[i].expr_s.begin() + current_expected_param,
                                           expression{
                                               st_s[i].expr_func->expected_params[current_expected_param].default_val,
                                               conv_param_type_to_expr_type(
                                                   st_s[i].expr_func->expected_params[current_expected_param].type),
                                           });
                    continue;
                }
            }
            else if (j >= st_s[i].expr_s.size())
                continue;

            if ((st_s[i].expr_func->expected_params[current_expected_param].type == param_type::FUTURE_VAR_ID ||
                 st_s[i].expr_func->expected_params[current_expected_param].type == param_type::VAR_ID ||
                 st_s[i].expr_func->expected_params[current_expected_param].type == param_type::NCHECK_VAR_ID ||
                 st_s[i].expr_func->expected_params[current_expected_param].type == param_type::VAR_STRUCT_ID) &&
                st_s[i].expr_s[j].type != expression::expression_t::ID)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << st_s[i].build_string_error(j + 1, NOT_MATCHING_W_PARAMETERS, "VAR ID"));
            else if (st_s[i].expr_func->expected_params[current_expected_param].type == param_type::NEXT_TOO) {
                if (current_expected_param == 0 ||
                    current_expected_param != st_s[i].expr_func->expected_params.size() - 1)
                    _log << bwtools::fatal
                         << (log_message(log_type::fatal)
                             << st_s[i].build_string_error(0, INCORECT_DEFINITION_FUNC, "VAR ID"));
                for (size_t b = j; b < st_s[i].expr_s.size(); ++b) {
                    if ((st_s[i].expr_func->expected_params[current_expected_param - 1].type ==
                             param_type::FUTURE_VAR_ID ||
                         st_s[i].expr_func->expected_params[current_expected_param - 1].type == param_type::VAR_ID ||
                         st_s[i].expr_func->expected_params[current_expected_param - 1].type ==
                             param_type::VAR_STRUCT_ID) &&
                        st_s[i].expr_s[b].type != expression::expression_t::ID)
                        _log << bwtools::fatal
                             << (log_message(log_type::fatal)
                                 << st_s[i].build_string_error(b + 1, NOT_MATCHING_W_PARAMETERS, "[VAR ID]"));
                    else if (st_s[i].expr_func->expected_params[current_expected_param - 1].type ==
                             param_type::ANY_VALUE_WITHOUT_FUTUREID_NEXT) {
                        if (st_s[i].expr_s[j - 1].type == expression::expression_t::NUMBER &&
                            st_s[i].expr_s[b].type != expression::expression_t::NUMBER &&
                            st_s[i].expr_s[b].type != expression::expression_t::ID)
                            _log << bwtools::fatal
                                 << (log_message(log_type::fatal) << st_s[i].build_string_error(
                                         b + 1, NOT_MATCHING_W_PARAMETERS, "[NUMBER] or VAR ID->[NUMBER]"));
                        else if (st_s[i].expr_s[j - 1].type == expression::expression_t::STRING &&
                                 st_s[i].expr_s[b].type != expression::expression_t::STRING &&
                                 st_s[i].expr_s[b].type != expression::expression_t::ID)
                            _log << bwtools::fatal
                                 << (log_message(log_type::fatal) << st_s[i].build_string_error(
                                         b + 1, NOT_MATCHING_W_PARAMETERS, "[STRING] or [VAR ID->[STRING]]"));
                    }
                    else if (st_s[i].expr_func->expected_params[current_expected_param - 1].type ==
                                 param_type::LIT_STR &&
                             st_s[i].expr_s[b].type != expression::expression_t::STRING)
                        _log << bwtools::fatal
                             << (log_message(log_type::fatal)
                                 << st_s[i].build_string_error(b + 1, NOT_MATCHING_W_PARAMETERS, "[STRING]"));
                    else if (st_s[i].expr_func->expected_params[current_expected_param - 1].type ==
                                 param_type::LIT_NUM &&
                             st_s[i].expr_s[b].type != expression::expression_t::NUMBER)
                        _log << bwtools::fatal
                             << (log_message(log_type::fatal)
                                 << st_s[i].build_string_error(b + 1, NOT_MATCHING_W_PARAMETERS, "[NUMBER]"));
                    else if (st_s[i].expr_func->expected_params[current_expected_param - 1].type ==
                                 param_type::LSTR_OR_ID_VAR &&
                             st_s[i].expr_s[b].type != expression::expression_t::STRING &&
                             st_s[i].expr_s[b].type != expression::expression_t::ID)
                        _log << bwtools::fatal
                             << (log_message(log_type::fatal) << st_s[i].build_string_error(
                                     b + 1, NOT_MATCHING_W_PARAMETERS, "[STRING] or [VAR ID->[STRING]]"));
                    else if (st_s[i].expr_func->expected_params[current_expected_param - 1].type ==
                                 param_type::LNUM_OR_ID_VAR &&
                             st_s[i].expr_s[b].type != expression::expression_t::NUMBER &&
                             st_s[i].expr_s[b].type != expression::expression_t::ID)
                        _log << bwtools::fatal
                             << (log_message(log_type::fatal) << st_s[i].build_string_error(
                                     b + 1, NOT_MATCHING_W_PARAMETERS, "[NUMBER] or VAR ID->[NUMBER]"));
                }
                break;
            }
            else if (st_s[i].expr_func->expected_params[current_expected_param].type ==
                         param_type::ANY_VALUE_WITHOUT_FUTUREID_NEXT &&
                     st_s[i].expr_s[j].type != expression::expression_t::STRING &&
                     st_s[i].expr_s[j].type != expression::expression_t::NUMBER &&
                     st_s[i].expr_s[j].type != expression::expression_t::ID)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal) << st_s[i].build_string_error(j + 1, NOT_MATCHING_W_PARAMETERS,
                                                                                    "[STRING], [NUMBER] OR [VAR ID]"));
            else if (st_s[i].expr_func->expected_params[current_expected_param].type == param_type::LIT_STR &&
                     st_s[i].expr_s[j].type != expression::expression_t::STRING)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << st_s[i].build_string_error(j + 1, NOT_MATCHING_W_PARAMETERS, "[STRING]"));
            else if (st_s[i].expr_func->expected_params[current_expected_param].type == param_type::LIT_NUM &&
                     st_s[i].expr_s[j].type != expression::expression_t::NUMBER)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << st_s[i].build_string_error(j + 1, NOT_MATCHING_W_PARAMETERS, "[NUMBER]"));
            else if (st_s[i].expr_func->expected_params[current_expected_param].type == param_type::LSTR_OR_ID_VAR &&
                     st_s[i].expr_s[j].type != expression::expression_t::STRING &&
                     st_s[i].expr_s[j].type != expression::expression_t::ID)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal) << st_s[i].build_string_error(j + 1, NOT_MATCHING_W_PARAMETERS,
                                                                                    "[STRING] or [VAR ID->[STRING]]"));
            else if (st_s[i].expr_func->expected_params[current_expected_param].type == param_type::LNUM_OR_ID_VAR &&
                     st_s[i].expr_s[j].type != expression::expression_t::NUMBER &&
                     st_s[i].expr_s[j].type != expression::expression_t::ID)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal) << st_s[i].build_string_error(j + 1, NOT_MATCHING_W_PARAMETERS,
                                                                                    "[NUMBER] or [VAR ID->[NUMBER]]"));

            if (j == st_s[i].expr_s.size() - 1 &&
                current_expected_param < st_s[i].expr_func->expected_params.size() - 1 &&
                st_s[i].expr_func->expected_params[current_expected_param + 1].type != param_type::NEXT_TOO &&
                !st_s[i].expr_func->expected_params[current_expected_param + 1].decl_default_val())
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << st_s[i].build_string_error(-1, NOT_MATCHING_W_PARAMETERS,
                                                       "[" +
                                                           string(get_string_expr_type(conv_param_type_to_expr_type(
                                                               st_s[i].expr_func->expected_params[j + 1].type))) +
                                                           "]"));
        }
    }
}
void semantic_analyzer::smt_second_pass(statements &st_s, scope &current_scope) {
    // <0 - if_skip, 1 - else_skip; 0 - current if, 1- current else>
    std::vector<std::pair<bool, bool>> branch_s;

    size_t nested_if = 0;

    bool skip = 0;

    for (size_t i = 0; i < st_s.size(); ++i) {
        param_type before_nextt_param   = param_type::SIZE_ENUM_PARAMS;
        size_t index_before_nextt_param = 0;
        for (size_t j = 0; j < st_s[i].expr_s.size(); ++j) {
            if (st_s[i].expr_s.size() == 0 || st_s[i].expr_s[0].value.size() == 0)
                continue;
            else if (before_nextt_param == param_type::SIZE_ENUM_PARAMS &&
                     st_s[i].expr_func->expected_params[j].type == param_type::NEXT_TOO) {
                before_nextt_param = st_s[i].expr_func->expected_params[j - 1].type;
                if (before_nextt_param == param_type::ANY_VALUE_WITHOUT_FUTUREID_NEXT)
                    index_before_nextt_param = j - 1;
            }
            if (before_nextt_param != param_type::SIZE_ENUM_PARAMS) {
                if (before_nextt_param == param_type::FUTURE_VAR_ID) {
                    size_t index_type = current_scope.what_type(st_s[i].expr_s[j].value);
                    if (index_type != 0)
                        _log << bwtools::fatal
                             << (log_message(log_type::fatal)
                                 << st_s[i].build_string_error(j + 1, EXPECTED_NEW_SYMBOL, "NEW [VAR ID]"));
                }
                else if (before_nextt_param == param_type::NCHECK_VAR_ID)
                    continue;
                else if (before_nextt_param == param_type::VAR_STRUCT_ID) {
                    size_t index_type = current_scope.what_type(st_s[i].expr_s[j].value);
                    if (!is_struct(index_type))
                        _log << bwtools::fatal
                             << (log_message(log_type::fatal)
                                 << st_s[i].build_string_error(j + 1, EXPECTED_EXIST_SYMBOL, "[VAR STRUCT ID]"));
                }
                else if (before_nextt_param == param_type::ANY_VALUE_WITHOUT_FUTUREID_NEXT) {
                    if (st_s[i].expr_s[index_before_nextt_param].type == expression::expression_t::NUMBER &&
                        st_s[i].expr_s[j].type != expression::expression_t::NUMBER) {
                        if (st_s[i].expr_s[j].type == expression::expression_t::ID) {
                            size_t index_type = current_scope.what_type(st_s[i].expr_s[j].value);
                            if (index_type == 1 || index_type == 3)
                                continue;
                        }
                        _log << bwtools::fatal
                             << (log_message(log_type::fatal)
                                 << st_s[i].build_string_error(j + 1, EXPECTED_ANOTHER_EXPR_TYPE, "[NUMBER]"));
                    }
                    else if (st_s[i].expr_s[index_before_nextt_param].type == expression::expression_t::STRING &&
                             st_s[i].expr_s[j].type != expression::expression_t::STRING) {
                        if (st_s[i].expr_s[j].type == expression::expression_t::ID) {
                            size_t index_type = current_scope.what_type(st_s[i].expr_s[j].value);
                            if (index_type == 2 || index_type == 4)
                                continue;
                        }
                        _log << bwtools::fatal
                             << (log_message(log_type::fatal)
                                 << st_s[i].build_string_error(j + 1, EXPECTED_ANOTHER_EXPR_TYPE, "[STRING]"));
                    }
                    else if (st_s[i].expr_s[index_before_nextt_param].type == expression::expression_t::ID) {
                        size_t index_type = current_scope.what_type(st_s[i].expr_s[index_before_nextt_param].value);
                        if ((index_type == 1 || index_type == 3) &&
                            st_s[i].expr_s[j].type != expression::expression_t::NUMBER) {
                            if (st_s[i].expr_s[j].type == expression::expression_t::ID) {
                                size_t index_type = current_scope.what_type(st_s[i].expr_s[j].value);
                                if (index_type == 1 || index_type == 3)
                                    continue;
                            }
                            _log << bwtools::fatal
                                 << (log_message(log_type::fatal)
                                     << st_s[i].build_string_error(j + 1, EXPECTED_ANOTHER_EXPR_TYPE, "[NUMBER]"));
                        }
                        else if ((index_type == 2 || index_type == 4) &&
                                 st_s[i].expr_s[j].type != expression::expression_t::STRING) {
                            if (st_s[i].expr_s[j].type == expression::expression_t::ID) {
                                size_t index_type = current_scope.what_type(st_s[i].expr_s[j].value);
                                if (index_type == 2 || index_type == 4)
                                    continue;
                            }
                            _log << bwtools::fatal
                                 << (log_message(log_type::fatal)
                                     << st_s[i].build_string_error(j + 1, EXPECTED_ANOTHER_EXPR_TYPE, "[STRING]"));
                        }
                    }
                }
                else if (st_s[i].expr_s[j].type == expression::expression_t::ID) {
                    size_t index_type = current_scope.what_type(st_s[i].expr_s[j].value);
                    if (before_nextt_param == param_type::VAR_ID && index_type == 0)
                        _log << bwtools::fatal
                             << (log_message(log_type::fatal) << st_s[i].build_string_error(
                                     j + 1, IMPOSSIBLE_DETERMINE_TYPE_VAR, "exist variable with any type"));
                    else if (index_type == 3 || index_type == 4)
                        continue;
                    else if (before_nextt_param == param_type::LNUM_OR_ID_VAR && index_type != 1 &&
                             st_s[i].expr_s[j].type != expression::expression_t::NUMBER)
                        _log << bwtools::fatal
                             << (log_message(log_type::fatal) << st_s[i].build_string_error(
                                     j + 1, NOT_EXIST_VAR_W_EXPECTED_TYPE, "[VAR ID->[NUMBER]]"));
                    else if (before_nextt_param == param_type::LSTR_OR_ID_VAR && index_type != 2 &&
                             st_s[i].expr_s[j].type != expression::expression_t::STRING)
                        _log << bwtools::fatal
                             << (log_message(log_type::fatal) << st_s[i].build_string_error(
                                     j + 1, NOT_EXIST_VAR_W_EXPECTED_TYPE, "[VAR ID->[STRING]]"));
                }
            }
            else if (st_s[i].expr_func->expected_params[j].type == param_type::FUTURE_VAR_ID) {
                size_t index_type = current_scope.what_type(st_s[i].expr_s[j].value);
                if (index_type != 0)
                    _log << bwtools::fatal
                         << (log_message(log_type::fatal)
                             << st_s[i].build_string_error(j + 1, EXPECTED_NEW_SYMBOL, "NEW [VAR ID]"));
            }
            else if (st_s[i].expr_func->expected_params[j].type == param_type::NCHECK_VAR_ID)
                continue;
            else if (st_s[i].expr_func->expected_params[j].type == param_type::VAR_STRUCT_ID) {
                size_t index_type = current_scope.what_type(st_s[i].expr_s[j].value);
                if (!is_struct(index_type))
                    _log << bwtools::fatal
                         << (log_message(log_type::fatal)
                             << st_s[i].build_string_error(j + 1, EXPECTED_EXIST_SYMBOL, "[VAR STRUCT ID]"));
            }
            else if (st_s[i].expr_s[j].type == expression::expression_t::ID) {
                size_t index_type = current_scope.what_type(st_s[i].expr_s[j].value);
                if (index_type == 0)
                    _log << bwtools::fatal
                         << (log_message(log_type::fatal) << st_s[i].build_string_error(
                                 j + 1, IMPOSSIBLE_DETERMINE_TYPE_VAR, "exist variable with any type"));
                else if ((index_type == 3 || index_type == 4) && j < st_s[i].expr_func->expected_params.size() - 1) {
                    if (st_s[i].expr_func->expected_params[j + 1].type != param_type::NEXT_TOO)
                        if (st_s[i].expr_func->expected_params[j].type == param_type::LNUM_OR_ID_VAR)
                            _log << bwtools::fatal
                                 << (log_message(log_type::fatal) << st_s[i].build_string_error(
                                         j + 1, NOT_EXIST_VAR_W_EXPECTED_TYPE, "[VAR ID->[NUMBER]]"));
                        else
                            _log << bwtools::fatal
                                 << (log_message(log_type::fatal) << st_s[i].build_string_error(
                                         j + 1, NOT_EXIST_VAR_W_EXPECTED_TYPE, "[VAR ID->[STRING]]"));
                }
                else if (st_s[i].expr_func->expected_params[j].type == param_type::LNUM_OR_ID_VAR && index_type != 1 &&
                         st_s[i].expr_s[j].type != expression::expression_t::NUMBER)
                    _log << bwtools::fatal
                         << (log_message(log_type::fatal)
                             << st_s[i].build_string_error(j + 1, NOT_EXIST_VAR_W_EXPECTED_TYPE, "[VAR ID->[NUMBER]]"));
                else if (st_s[i].expr_func->expected_params[j].type == param_type::LSTR_OR_ID_VAR && index_type != 2 &&
                         st_s[i].expr_s[j].type != expression::expression_t::STRING)
                    _log << bwtools::fatal
                         << (log_message(log_type::fatal)
                             << st_s[i].build_string_error(j + 1, NOT_EXIST_VAR_W_EXPECTED_TYPE, "[VAR ID->[STRING]]"));
                else if (index_type == 5 || index_type == 6)
                    _log << bwtools::fatal
                         << (log_message(log_type::fatal) << st_s[i].build_string_error(
                                 j + 1, NOT_EXIST_VAR_W_EXPECTED_TYPE, "[VAR ID->([NUMBER] or [STRING])]"));
            }
        }
        for (size_t j = 0, b = 0; j < st_s[i].expr_s.size(); ++j, ++b) {
            if (b >= st_s[i].expr_func->expected_params.size() && before_nextt_param != param_type::SIZE_ENUM_PARAMS) {
                parse_expr_param(
                    st_s[i].expr_s[j], st_s[i].expr_s, j, current_scope,
                    st_s[i].expr_func->expected_params[st_s[i].expr_func->expected_params.size() - 1].type);
                continue;
            }
            parse_expr_param(st_s[i].expr_s[j], st_s[i].expr_s, j, current_scope,
                             st_s[i].expr_func->expected_params[b].type);
        }

        // skips branches
        if (skip) {
            if (st_s[i].expr_func->name_func == STR_KEYWORD_IF)
                ++nested_if;

            else if (st_s[i].expr_func->name_func == STR_KEYWORD_ELSE)
                ++nested_if;

            else if (st_s[i].expr_func->name_func == STR_KEYWORD_ENDIF) {
                if (!nested_if)
                    goto endif_end;

                --nested_if;
            }
            st_s.erase(st_s.begin() + i);
            --i;
        }
        // keyword handler(if, else, endif)
        if (st_s[i].expr_func->name_func == STR_KEYWORD_IF) {
            size_t val = std::stoi(st_s[i].expr_s[0].value);
            if (!val) {
                skip = 1;
                branch_s.push_back(std::pair<bool, bool>(0, 0));
            }
            else
                branch_s.push_back(std::pair<bool, bool>(1, 0));
            goto delete_entry;
        }
        else if (st_s[i].expr_func->name_func == STR_KEYWORD_ELSE) {
            if (!branch_s.size() || branch_s[branch_s.size() - 1].second)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal) << st_s[i].build_string_error(PTRDIFF_MAX, EXPECTED_IF));

            if (branch_s[branch_s.size() - 1].first)
                skip = 1;
            goto delete_entry;
        }
        else if (st_s[i].expr_func->name_func == STR_KEYWORD_ENDIF) {
        endif_end:
            if (!branch_s.size())
                _log << bwtools::fatal
                     << (log_message(log_type::fatal) << st_s[i].build_string_error(PTRDIFF_MAX, INVALID_USAGE_ENDIF));

            skip = 0;
            if (branch_s[branch_s.size() - 1].second ||
                (i < st_s.size() - 1 && st_s[i + 1].expr_func->name_func != STR_KEYWORD_ELSE))
                branch_s.pop_back();
            goto delete_entry;
        }
        st_s[i].expr_func->func(st_s[i].expr_s, current_scope);
        continue;
    delete_entry:
        st_s.erase(st_s.begin() + i);
        --i;
    }

    if (branch_s.size())
        _log << bwtools::fatal << (log_message(log_type::fatal) << "End of statements: " << EXPECTED_ENDIF);
}
void semantic_analyzer::parse_expr_param(expression &expr, expressions &expr_s, size_t &pos_expr_in_vec,
                                         scope &current_scope, param_type expected_param) {
    expression parse_expr, tmp_parse_expr;
    if (expr.type == expression::expression_t::ID &&
        (expected_param != param_type::VAR_ID && expected_param != param_type::FUTURE_VAR_ID &&
         expected_param != param_type::NCHECK_VAR_ID)) {
        size_t index_var = current_scope.what_type(expr.value);
        if (index_var == 1) {
            parse_expr.type  = expression::expression_t::NUMBER;
            parse_expr.value = std::to_string(current_scope.get_var_value<pdiff>(expr.value));
        }
        else if (index_var == 2) {
            parse_expr.type  = expression::expression_t::STRING;
            parse_expr.value = current_scope.get_var_value<std::string>(expr.value);
        }
        else if (index_var == 3) {
            expressions new_expr_s;
            expression tmp_expr;
            tmp_expr.type                       = expression::expression_t::NUMBER;
            const std::vector<pdiff> vec_int_id = current_scope.get_var_value<std::vector<pdiff>>(expr.value);
            for (size_t i = 0; i < pos_expr_in_vec; ++i)
                new_expr_s.push_back(expr_s[i]);
            for (size_t i = 0; i < vec_int_id.size(); ++i) {
                tmp_expr.value = std::to_string(vec_int_id[i]);
                new_expr_s.push_back(tmp_expr);
                tmp_expr.value = "";
            }
            for (size_t i = pos_expr_in_vec + 1; i < expr_s.size(); ++i)
                new_expr_s.push_back(expr_s[i]);
            pos_expr_in_vec += vec_int_id.size() - 1;
            expr_s = new_expr_s;
        }
        else if (index_var == 4) {
            expressions new_expr_s;
            expression tmp_expr;
            tmp_expr.type = expression::expression_t::STRING;
            const std::vector<std::string> vec_str_id =
                current_scope.get_var_value<std::vector<std::string>>(expr.value);
            for (size_t i = 0; i < pos_expr_in_vec; ++i)
                new_expr_s.push_back(expr_s[i]);
            for (size_t i = 0; i < vec_str_id.size(); ++i) {
                tmp_expr.value = vec_str_id[i];
                new_expr_s.push_back(tmp_expr);
                tmp_expr.value = "";
            }
            for (size_t i = pos_expr_in_vec + 1; i < expr_s.size(); ++i)
                new_expr_s.push_back(expr_s[i]);
            pos_expr_in_vec += vec_str_id.size() - 1;
            expr_s = new_expr_s;
        }
        if (!parse_expr.value.empty())
            expr = parse_expr;
        return;
    }
}
