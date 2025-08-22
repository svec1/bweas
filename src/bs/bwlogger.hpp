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
enum class log_type;
class log_message;
class logger;

template <bool inv = 1> class log_console_lock;
using log_console_unlock = log_console_lock<0>;
} // namespace bweas

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
    logger(string_v _owner) : owner(_owner) {
        using namespace bweas::utils;
        file_log = file_utils::open_file("bweas-last.log", file_utils::file::mode_file::open::w);
    }

    logger(const logger &)            = delete;
    logger &operator=(const logger &) = delete;

  public:
    void set_global_status();

    bweas::log_type get_status();
    void dump_status();

    bool error_status();

  public:
    void operator<<(const bweas::log_message &obj);

  public:
    static bweas::log_type global_status;

  private:
    static bweas::utils::file_utils::file_it file_log;
    static bool output_to_console;

    string_v owner;
    bweas::log_type status;
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
