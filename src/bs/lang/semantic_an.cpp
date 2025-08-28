//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#include <lang/semantic_an.hpp>

using namespace bweas;

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

void semantic_analyzer::analysis(statement &st, scope &current_scope) {
    smt_first_pass(st, current_scope);
    smt_second_pass(st, current_scope);
}
void semantic_analyzer::check_end_statements() {
    if (branch_s.size())
        _log << (log_message(log_type::fatal) << "End of statements: " << EXPECTED_ENDIF);
}

void semantic_analyzer::smt_first_pass(statement &st, scope &current_scope) {
    if (st.expr_func->expected_params.size() != st.expr_s.size()) {
        if ((st.expr_func->expected_params.size() != 0 &&
             st.expr_func->expected_params[st.expr_func->expected_params.size() - 1].type == param_type::NEXT_TOO &&
             st.expr_func->expected_params.size() - 1 <= st.expr_s.size()) ||
            (st.expr_func->expected_params[st.expr_func->expected_params.size() - 1].type == param_type::NEXT_TOO
                 ? (st.expr_func->expected_params.size() - st.expr_s.size() - 1 > 0 &&
                    st.expr_func->expected_params.size() - st.expr_s.size() - 1 == st.expr_func->count_default_params())
                 : (st.expr_func->expected_params.size() - st.expr_s.size() > 0 &&
                    st.expr_func->expected_params.size() - st.expr_s.size() == st.expr_func->count_default_params())))
            goto check_valid_exp_type;
        _log << (log_message(log_type::fatal) << st.build_string_error(
                     PTRDIFF_MAX, NOT_MATCHING_W_PARAMETERS, st.get_string_expected_params(st.expr_s.size())));
    }
    else if (st.expr_s.size() == 0)
        return;

check_valid_exp_type:
    for (size_t j = 0, current_expected_param = 0;
         j < st.expr_s.size() || current_expected_param < st.expr_func->expected_params.size();
         ++j, ++current_expected_param) {
        if (st.expr_func->expected_params[current_expected_param].decl_default_val()) {
            if (j >= st.expr_s.size() ||
                (st.expr_s[j].type !=
                     conv_param_type_to_expr_type(st.expr_func->expected_params[current_expected_param].type) &&
                 !(st.expr_s[j].type == expression::expression_t::ID && st.expr_s[j].value_by_id))) {
                st.expr_s.emplace(st.expr_s.begin() + current_expected_param,
                                  expression{st.expr_func->expected_params[current_expected_param].default_val.value(),
                                             conv_param_type_to_expr_type(
                                                 st.expr_func->expected_params[current_expected_param].type)});
                continue;
            }
        }
        else if (j >= st.expr_s.size())
            continue;

        if (is_id_param(st.expr_func->expected_params[current_expected_param].type) &&
            (st.expr_s[j].type != expression::expression_t::ID || st.expr_s[j].value_by_id))
            _log << (log_message(log_type::fatal) << st.build_string_error(j + 1, NOT_MATCHING_W_PARAMETERS, "VAR ID"));
        else if (st.expr_func->expected_params[current_expected_param].type == param_type::NEXT_TOO) {
            if (current_expected_param == 0 || current_expected_param != st.expr_func->expected_params.size() - 1)
                _log << (log_message(log_type::fatal) << st.build_string_error(0, INCORECT_DEFINITION_FUNC, "VAR ID"));
            for (size_t b = j; b < st.expr_s.size(); ++b) {
                if (is_id_param(st.expr_func->expected_params[current_expected_param - 1].type) &&
                    (st.expr_s[b].type != expression::expression_t::ID || st.expr_s[b].value_by_id))
                    _log << (log_message(log_type::fatal)
                             << st.build_string_error(b + 1, NOT_MATCHING_W_PARAMETERS, "[VAR ID]"));
                else if (st.expr_func->expected_params[current_expected_param - 1].type ==
                         param_type::ANY_VALUE_WITHOUT_FUTUREID_NEXT) {
                    if (st.expr_s[j - 1].type == expression::expression_t::NUMBER &&
                        (st.expr_s[b].type != expression::expression_t::NUMBER && !st.expr_s[b].value_by_id))
                        _log << (log_message(log_type::fatal) << st.build_string_error(b + 1, NOT_MATCHING_W_PARAMETERS,
                                                                                       "[NUMBER] or VAR ID->[NUMBER]"));
                    else if (st.expr_s[j - 1].type == expression::expression_t::STRING &&
                             (st.expr_s[b].type != expression::expression_t::STRING && !st.expr_s[b].value_by_id))
                        _log << (log_message(log_type::fatal) << st.build_string_error(
                                     b + 1, NOT_MATCHING_W_PARAMETERS, "[STRING] or [VAR ID->[STRING]]"));
                }
                else if (st.expr_func->expected_params[current_expected_param - 1].type == param_type::LIT_STR &&
                         (st.expr_s[b].type != expression::expression_t::STRING && !st.expr_s[b].value_by_id))
                    _log << (log_message(log_type::fatal)
                             << st.build_string_error(b + 1, NOT_MATCHING_W_PARAMETERS, "[STRING]"));
                else if (st.expr_func->expected_params[current_expected_param - 1].type == param_type::LIT_NUM &&
                         (st.expr_s[b].type != expression::expression_t::NUMBER && !st.expr_s[b].value_by_id))
                    _log << (log_message(log_type::fatal)
                             << st.build_string_error(b + 1, NOT_MATCHING_W_PARAMETERS, "[NUMBER]"));
            }
            break;
        }
        else if (st.expr_func->expected_params[current_expected_param].type ==
                     param_type::ANY_VALUE_WITHOUT_FUTUREID_NEXT &&
                 st.expr_s[j].type != expression::expression_t::STRING &&
                 st.expr_s[j].type != expression::expression_t::NUMBER &&
                 st.expr_s[j].type != expression::expression_t::ID)
            _log << (log_message(log_type::fatal)
                     << st.build_string_error(j + 1, NOT_MATCHING_W_PARAMETERS, "[STRING], [NUMBER] OR [VAR ID]"));
        else if (st.expr_func->expected_params[current_expected_param].type == param_type::LIT_STR &&
                 st.expr_s[j].type != expression::expression_t::STRING && !st.expr_s[j].value_by_id)
            _log << (log_message(log_type::fatal)
                     << st.build_string_error(j + 1, NOT_MATCHING_W_PARAMETERS, "[STRING]"));
        else if (st.expr_func->expected_params[current_expected_param].type == param_type::LIT_NUM &&
                 st.expr_s[j].type != expression::expression_t::NUMBER && !st.expr_s[j].value_by_id)
            _log << (log_message(log_type::fatal)
                     << st.build_string_error(j + 1, NOT_MATCHING_W_PARAMETERS, "[NUMBER]"));

        if (j == st.expr_s.size() - 1 && current_expected_param < st.expr_func->expected_params.size() - 1 &&
            st.expr_func->expected_params[current_expected_param + 1].type != param_type::NEXT_TOO &&
            !st.expr_func->expected_params[current_expected_param + 1].decl_default_val())
            _log << (log_message(log_type::fatal)
                     << st.build_string_error(-1, NOT_MATCHING_W_PARAMETERS,
                                              "[" +
                                                  string(get_string_expr_type(conv_param_type_to_expr_type(
                                                      st.expr_func->expected_params[j + 1].type))) +
                                                  "]"));
    }
}
void semantic_analyzer::smt_second_pass(statement &st, scope &current_scope) {
    param_type before_nextt_param   = param_type::SIZE_ENUM_PARAMS;
    size_t index_before_nextt_param = 0;
    for (size_t j = 0; j < st.expr_s.size(); ++j) {
        if (st.expr_s[0].value.size() == 0)
            continue;
        else if (before_nextt_param == param_type::SIZE_ENUM_PARAMS &&
                 st.expr_func->expected_params[j].type == param_type::NEXT_TOO) {
            before_nextt_param = st.expr_func->expected_params[j - 1].type;
            if (before_nextt_param == param_type::ANY_VALUE_WITHOUT_FUTUREID_NEXT)
                index_before_nextt_param = j - 1;
        }
        if (before_nextt_param != param_type::SIZE_ENUM_PARAMS) {
            if (before_nextt_param == param_type::FUTURE_VAR_ID) {
                size_t index_type = current_scope.what_type(st.expr_s[j].value);
                if (index_type != 0)
                    _log << (log_message(log_type::fatal)
                             << st.build_string_error(j + 1, EXPECTED_NEW_SYMBOL, "NEW [VAR ID]"));
            }
            else if (before_nextt_param == param_type::NCHECK_VAR_ID)
                continue;
            else if (before_nextt_param == param_type::VAR_STRUCT_ID) {
                size_t index_type = current_scope.what_type(st.expr_s[j].value);
                if (!scope::is_struct(index_type))
                    _log << (log_message(log_type::fatal)
                             << st.build_string_error(j + 1, EXPECTED_EXIST_SYMBOL, "[VAR STRUCT ID]"));
            }
            else if (before_nextt_param == param_type::ANY_VALUE_WITHOUT_FUTUREID_NEXT) {
                if (st.expr_s[index_before_nextt_param].type == expression::expression_t::NUMBER &&
                    st.expr_s[j].type != expression::expression_t::NUMBER) {
                    if (st.expr_s[j].type == expression::expression_t::ID) {
                        size_t index_type = current_scope.what_type(st.expr_s[j].value);
                        if (index_type == 1 || index_type == 3)
                            continue;
                    }
                    _log << (log_message(log_type::fatal)
                             << st.build_string_error(j + 1, EXPECTED_ANOTHER_EXPR_TYPE, "[NUMBER]"));
                }
                else if (st.expr_s[index_before_nextt_param].type == expression::expression_t::STRING &&
                         st.expr_s[j].type != expression::expression_t::STRING) {
                    if (st.expr_s[j].type == expression::expression_t::ID && st.expr_s[j].value_by_id) {
                        size_t index_type = current_scope.what_type(st.expr_s[j].value);
                        if (index_type == 2 || index_type == 4)
                            continue;
                    }
                    _log << (log_message(log_type::fatal)
                             << st.build_string_error(j + 1, EXPECTED_ANOTHER_EXPR_TYPE, "[STRING]"));
                }
                else if (st.expr_s[index_before_nextt_param].type == expression::expression_t::ID) {
                    size_t index_type = current_scope.what_type(st.expr_s[index_before_nextt_param].value);
                    if ((index_type == 1 || index_type == 3) && st.expr_s[j].type != expression::expression_t::NUMBER) {
                        if (st.expr_s[j].type == expression::expression_t::ID && st.expr_s[j].value_by_id) {
                            size_t index_type = current_scope.what_type(st.expr_s[j].value);
                            if (index_type == 1 || index_type == 3)
                                continue;
                        }
                        _log << (log_message(log_type::fatal)
                                 << st.build_string_error(j + 1, EXPECTED_ANOTHER_EXPR_TYPE, "[NUMBER]"));
                    }
                    else if ((index_type == 2 || index_type == 4) &&
                             st.expr_s[j].type != expression::expression_t::STRING) {
                        if (st.expr_s[j].type == expression::expression_t::ID && st.expr_s[j].value_by_id) {
                            size_t index_type = current_scope.what_type(st.expr_s[j].value);
                            if (index_type == 2 || index_type == 4)
                                continue;
                        }
                        _log << (log_message(log_type::fatal)
                                 << st.build_string_error(j + 1, EXPECTED_ANOTHER_EXPR_TYPE, "[STRING]"));
                    }
                }
            }
            else if (st.expr_s[j].type == expression::expression_t::ID) {
                size_t index_type = current_scope.what_type(st.expr_s[j].value);
                if (index_type == 0)
                    _log << (log_message(log_type::fatal) << st.build_string_error(j + 1, IMPOSSIBLE_DETERMINE_TYPE_VAR,
                                                                                   "exist variable with any type"));
                else if (before_nextt_param == param_type::LIT_NUM && (index_type != 1 && index_type != 3))
                    _log << (log_message(log_type::fatal)
                             << st.build_string_error(j + 1, NOT_EXIST_VAR_W_EXPECTED_TYPE, "[NUMBER]"));
                else if (before_nextt_param == param_type::LIT_STR && (index_type != 2 && index_type != 4))
                    _log << (log_message(log_type::fatal)
                             << st.build_string_error(j + 1, NOT_EXIST_VAR_W_EXPECTED_TYPE, "[STRING]"));
            }
        }
        else if (st.expr_func->expected_params[j].type == param_type::FUTURE_VAR_ID) {
            size_t index_type = current_scope.what_type(st.expr_s[j].value);
            if (index_type != 0)
                _log << (log_message(log_type::fatal)
                         << st.build_string_error(j + 1, EXPECTED_NEW_SYMBOL, "NEW [VAR ID]"));
        }
        else if (st.expr_func->expected_params[j].type == param_type::NCHECK_VAR_ID)
            continue;
        else if (st.expr_func->expected_params[j].type == param_type::VAR_STRUCT_ID) {
            size_t index_type = current_scope.what_type(st.expr_s[j].value);
            if (!scope::is_struct(index_type))
                _log << (log_message(log_type::fatal)
                         << st.build_string_error(j + 1, EXPECTED_EXIST_SYMBOL, "[VAR STRUCT ID]"));
        }
        else if (st.expr_s[j].type == expression::expression_t::ID) {
            size_t index_type = current_scope.what_type(st.expr_s[j].value);
            if (index_type == 0)
                _log << (log_message(log_type::fatal) << st.build_string_error(j + 1, IMPOSSIBLE_DETERMINE_TYPE_VAR,
                                                                               "exist variable with any type"));
            else if (st.expr_func->expected_params[j].type == param_type::LIT_NUM && index_type != 1 &&
                     !(index_type == 3 && j < st.expr_func->expected_params.size() - 1 &&
                       st.expr_func->expected_params[j + 1].type == param_type::NEXT_TOO))
                _log << (log_message(log_type::fatal)
                         << st.build_string_error(j + 1, NOT_EXIST_VAR_W_EXPECTED_TYPE, "[NUMBER]"));
            else if (st.expr_func->expected_params[j].type == param_type::LIT_STR && index_type != 2 &&
                     !(index_type == 4 && j < st.expr_func->expected_params.size() - 1 &&
                       st.expr_func->expected_params[j + 1].type == param_type::NEXT_TOO))
                _log << (log_message(log_type::fatal)
                         << st.build_string_error(j + 1, NOT_EXIST_VAR_W_EXPECTED_TYPE, "[STRING]"));
        }
    }
    for (size_t j = 0; j < st.expr_s.size(); ++j)
        parse_expr_param(st.expr_s[j], st.expr_s, j, current_scope);

    // skips branches
    if (skip) {
        if (st.expr_func->name == STR_KEYWORD_IF)
            ++nested_if;

        else if (st.expr_func->name == STR_KEYWORD_ELSE)
            ++nested_if;

        else if (st.expr_func->name == STR_KEYWORD_ENDIF) {
            if (!nested_if)
                goto endif_end;

            --nested_if;
        }
        return;
    }
    // keyword handler(if, else, endif)
    if (st.expr_func->name == STR_KEYWORD_IF) {
        size_t val = std::stoi(st.expr_s[0].value);
        if (!val) {
            skip = 1;
            branch_s.push_back(std::pair<bool, bool>(0, 0));
        }
        else
            branch_s.push_back(std::pair<bool, bool>(1, 0));
        return;
    }
    else if (st.expr_func->name == STR_KEYWORD_ELSE) {
        if (!branch_s.size() || branch_s[branch_s.size() - 1].second || last_st_is_endif)
            _log << (log_message(log_type::fatal) << st.build_string_error(PTRDIFF_MAX, EXPECTED_IF));

        if (branch_s[branch_s.size() - 1].first)
            skip = 1;
        return;
    }
    else if (st.expr_func->name == STR_KEYWORD_ENDIF) {
    endif_end:
        if (!branch_s.size())
            _log << (log_message(log_type::fatal) << st.build_string_error(PTRDIFF_MAX, INVALID_USAGE_ENDIF));

        skip             = 0;
        last_st_is_endif = 1;
        if (branch_s[branch_s.size() - 1].second)
            branch_s.pop_back();
        return;
    }
    st.expr_func->func(st.expr_s, current_scope);
    last_st_is_endif = 0;
}
void semantic_analyzer::parse_expr_param(expression &expr, expressions &expr_s, size_t &pos_expr_in_vec,
                                         scope &current_scope) {
    if (!expr.value_by_id)
        return;

    expression parse_expr, tmp_parse_expr;
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
        tmp_expr.type                             = expression::expression_t::STRING;
        const std::vector<std::string> vec_str_id = current_scope.get_var_value<std::vector<std::string>>(expr.value);
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
}
