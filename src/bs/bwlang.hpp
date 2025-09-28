//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWLANG_HPP
#define BWLANG_HPP

#include <bw_defs.hpp>
#include <bwmodule.hpp>

#include <lang/parser.hpp>

namespace bweas {
class lang;
}

// A wrapper around the lang interpreter that installs all
// the standard bweas functions and also provides interaction with the global scope
class bweas::lang {
  public:
    inline lang(context *const __context, string_v src);

    lang(lang &&)            = delete;
    lang(const lang &)       = delete;
    lang &operator=(lang &&) = delete;

  private:
    // Sets standard functions and keyword operators corresponding to the bweas specification
    inline void init_scope();

  public:
    // Starts the internal interpreter
    inline void execute();

    // Loads external functions (passed into this function) into the interpreter (semantic analyzer)
    inline void import_modules(vec<module_manager::_module> modules);

    inline void init_context();

  public:
    bwlang::parser_utils::context get_context() {
        return p.get_context();
    }

    template <typename T> inline T get_variable(string name_var) {
        if (std::holds_alternative<T>(p.get_context().get_variable(name_var)))
            return std::get<T>(p.get_context().get_variable(name_var));
        return T{};
    }

  private:
    template <typename T> inline void create_variable(string name_var, T val = {}) {
        p.get_context().sc[name_var] = val;
    }
    void create_function(string name_func, bwlang::parser_utils::func::func_t func_ref) {
        p.get_context().funcs[name_func] = bwlang::parser_utils::func{func_ref, {}, false};
    }

  private:
    vec<sc::target> get_targets();
    vec<sc::template_command> get_templates();
    vec<sc::call_component> get_call_components();
    vec<pair<string, string>> get_global_external_args();

  private:
    context *const _context;
    bwlang::parser p;
};
bweas::lang::lang(context *const __context, string_v src) : _context(__context), p(src) {
    init_scope();
}

void bweas::lang::init_scope() {
    bwlang::parser::dump_global_context();

    create_variable<pdiff>("debug", 0);
    create_variable<pdiff>("release", 1);
    create_variable<pdiff>("false", 0);
    create_variable<pdiff>("true", 1);
    create_variable<pdiff>("executable", 0);
    create_variable<pdiff>("library", 1);
    create_variable<pdiff>("message", 0);
    create_variable<pdiff>("warning", 1);
    create_variable<pdiff>("error", 2);
    create_variable<pdiff>("get_files", 0);
    create_variable<string>("config_path", fs::current_path().string());

    static auto expected_argument = [](auto &&type, bwlang::parser_utils::scope &sc,
                                       pdiff number_arg) -> std::decay_t<decltype(type)> {
        using T = std::decay_t<decltype(type)>;

        string name_arg = std::to_string(number_arg);

        if (!sc.contains(name_arg) || !std::holds_alternative<T>(sc.at(name_arg)))
            throw std::runtime_error("Expected " + name_arg + " args as " + bwlang::parser_utils::get_type_name<T>());

        auto val = std::get<T>(sc.at(name_arg));
        sc.erase(name_arg);
        return val;
    };

    create_function("build", [&](string, bwlang::parser_utils::context &c_ctx) -> bwlang::parser_utils::value {
        if (!_context)
            throw std::runtime_error("Bweas the context is nullptr.");

        for (const auto &[key, value] : c_ctx.sc) {
            if (std::holds_alternative<sc::target>(value))
                _context->targets.emplace_back(std::move(std::get<sc::target>(value)));
            else if (std::holds_alternative<vec<sc::target>>(value)) {
                const auto &targets = std::get<vec<sc::target>>(value);
                for (const auto &target : targets)
                    _context->targets.emplace_back(std::move(target));
            }
            else
                throw std::runtime_error("Expected target type.");
        }
        return {};
    });

    create_function("status", [](string, bwlang::parser_utils::context &c_ctx) -> bwlang::parser_utils::value {
        static constexpr pdiff MESSAGE = 0;
        static constexpr pdiff WARNING = 1;
        static constexpr pdiff ERROR   = 2;

        pdiff number_type_output = expected_argument(pdiff{}, c_ctx.sc, 0);

        string output;

        for (const auto &[key, value] : c_ctx.sc) {
            if (std::holds_alternative<string>(value))
                output += std::get<string>(value) + " ";
            else if (std::holds_alternative<vec<string>>(value)) {
                for (const auto &str : std::get<vec<string>>(value)) {
                    output += str + " ";
                }
            }
            else
                throw std::runtime_error("Expected string type.");
        }

        switch (number_type_output) {
        case MESSAGE:
        default:
            bweas::logger{""} << (log_message(log_type::msg) << output);
            break;
        case WARNING:
            bweas::logger{""} << (log_message(log_type::warning) << output);
            break;
        case ERROR:
            bweas::logger{""} << (log_message(log_type::error) << output);
            break;
        }
        return {};
    });
    create_function("file", [](string, bwlang::parser_utils::context &c_ctx) -> bwlang::parser_utils::value {
        static constexpr pdiff GET_FILES = 0;
        static auto get_files            = [](string file) -> vec<string> {
            if (fs::exists(file))
                return {utils::file_utils::get_path_file(file)};
            else {
                std::function<vec<string>(string file)> get_dir_files = [&](string file) -> vec<string> {
                    vec<string> dir_files;
                    string dir = file.substr(0, file.find_last_of("/\\"));
                    for (const auto &it : fs::directory_iterator{dir}) {
                        if (fs::is_regular_file(it))
                            dir_files.push_back(utils::file_utils::get_path_file(it.path().c_str()));
                        else if (fs::is_directory(it)) {
                            auto vec_tmp = get_dir_files(string(it.path()) + "/");
                            dir_files.insert(dir_files.end(), vec_tmp.begin(), vec_tmp.end());
                        }
                    }

                    return dir_files;
                };

                vec<string> files = get_dir_files(file);
                if (auto it = file.find_last_of("/\\"); it != file.npos)
                    files = utils::file_utils::file_slc_mask(file.substr(it + 1), files);

                return files;
            }
        };

        if (c_ctx.sc.size() < 2)
            throw std::runtime_error("Expected a more args.");

        vec<string> return_value;
        pdiff number_function = expected_argument(pdiff{}, c_ctx.sc, 0);

        if (number_function == GET_FILES) {
            for (const auto &[key, value] : c_ctx.sc) {
                if (std::holds_alternative<string>(value)) {
                    vec<string> vec_tmp = get_files(std::get<string>(value));
                    return_value.insert(return_value.end(), vec_tmp.begin(), vec_tmp.end());
                }
                else if (std::holds_alternative<vec<string>>(value)) {
                    for (const auto &file : std::get<vec<string>>(value)) {
                        vec<string> vec_tmp = get_files(file);
                        return_value.insert(return_value.end(), vec_tmp.begin(), vec_tmp.end());
                    }
                }
                else
                    throw std::runtime_error("Expected string type.");
            }
        }
        return return_value;
    });
}

void bweas::lang::execute() {
    p.parse();
}
void bweas::lang::import_modules(vec<module_manager::_module> modules) {
    p.import_modules(std::move(modules));
}

void bweas::lang::init_context() {
    if (!_context)
        bweas::logger{""} << (log_message(log_type::fatal) << "Bweas the context is nullptr.");

    for (auto &[key, value] : p.get_context().sc)
        if (std::holds_alternative<sc::template_command>(value)) {
            auto &tcmd = std::get<sc::template_command>(value);
            if (size_t it = tcmd.name_call_component.find(":"); it != tcmd.name_call_component.npos) {
                string name_ccmp = "anon_cc_" + tcmd.name;
                _context->call_components.emplace_back(name_ccmp, tcmd.name_call_component.substr(0, it),
                                                       tcmd.name_call_component.substr(it + 1));

                tcmd.name_call_component = name_ccmp;
            }
            _context->templates.push_back(tcmd);
        }
        else if (std::holds_alternative<sc::call_component>(value))
            _context->call_components.push_back(std::get<sc::call_component>(value));
}

#endif
