//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWLOGGER_HPP
#define BWLOGGER_HPP

#include <functional>
#include <sstream>

#include <utils/file_utils.hpp>

namespace bweas {
class exception;
enum class log_type;
class log_message;
class logger;

template <bool inv = 1> class log_console_lock;
using log_console_unlock = log_console_lock<0>;
} // namespace bweas

class bweas::exception : public std::exception {
    friend class bweas::logger;

  private:
    exception(string _what_str) : what_str(_what_str) {
    }
    ~exception() override = default;

    const char *what() const noexcept override {
        return what_str.c_str();
    }

  private:
    string what_str;
};

enum class bweas::log_type {
    msg = 0,
    success,
    warning,
    error,
    fatal
};

// The message that the logger accepts
class bweas::log_message {
    friend bweas::logger;

  public:
    log_message(log_type);

  public:
    template <typename T> log_message &operator<<(const T &obj);

  private:
    std::stringstream ss;
    bweas::log_type log_t;
};

// A class for logging status
class bweas::logger {
    friend bweas::log_console_lock<0>;
    friend bweas::log_console_lock<1>;

  public:
    using handle_func_t = void(string_v);

  public:
    logger(string_v _owner, bool _is_main = false);

    logger(const logger &)            = delete;
    logger &operator=(const logger &) = delete;

    ~logger();

  public:
    void set_global_log(bool _log);
    void set_debug();

    bool request_yes_no(bool condition, string_v out);
    void set_next_yes();

  public:
    void operator<<(const bweas::log_message &obj);

    void handle_exception(const bweas::exception &excp);

  private:
    void handle(string &&str);

  public:
    static void init();

    static bweas::log_type global_status;
    static bool error_status();

  private:
    static bool log, debug, output_to_console;
    static bweas::utils::file_utils::file file_log;

    string_v owner;
    bool is_main;
    bool next_yes = false;

  private:
    static constexpr auto NAME_FILE_LOG = "bweas-last.log";
};

template <bool inv> class bweas::log_console_lock {
  public:
    log_console_lock() {
        logger::output_to_console = !inv;
    }
    ~log_console_lock() {
        logger::output_to_console = inv;
    }
};

template <typename T> bweas::log_message &bweas::log_message::operator<<(const T &obj) {
    ss << obj;
    return *this;
}

#endif
