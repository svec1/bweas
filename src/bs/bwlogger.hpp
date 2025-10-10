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

/** \brief The exception class for bweas. */
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

/** \brief An enumeration that includes all types of logging. */
enum class bweas::log_type {
    msg = 0, ///< The message indicates the status of the current work.
    success, ///< A message indicating the current successful status.
    warning, ///< A warning message about possible inconsistencies.
    error,   ///< A status error notification message.
    fatal    ///< A condition in which bweas is immediately closed with a corresponding message.
};

/** \brief The message that the logger accepts. */
class bweas::log_message {
    friend bweas::logger;

  public:
    /** \brief Construct.
     * \param [in] log_type The type of status that this object will have.
     */
    log_message(log_type);

  public:
    /** \brief A function that builds a message.
     * \param [in] obj Any object for which the << std::stringstream operator exists.
     * \return log_message& The object that the bweas::logger can accept.
     */
    template <typename T> log_message &operator<<(const T &obj);

  private:
    std::stringstream ss;
    bweas::log_type log_t;
};

/** \brief A class for logging status. */
class bweas::logger final {
    friend bweas::log_console_lock<0>;
    friend bweas::log_console_lock<1>;

  public:
    using handle_func_t = void(string_v);

  public:
    /** \brief Construct.
     * \param [in] _owner The name of the object from which logging will be performed.
     * \param [in] _is_main If the logger is not the main one, then when processing messages with the fatal status, a
     * bweas::expectation exception will be thrown. Otherwise, exit(1) will be called.
     */
    logger(string_v _owner, bool _is_main = false);

    logger(const logger &)            = delete;
    logger &operator=(const logger &) = delete;

    ~logger();

  public:
    /** \brief Determines whether to log messages.
     * \detail Sets the static variable log, where if the value is false, the logger will skip all messages.
     * \param [in] _log
     */
    void set_global_log(bool _log);

    /** \brief Determines whether to output messages to the console with log_console_lock.
     * \detail Sets the static variable debug, where if the value is true, the logger will output the entire message to
     * the console even if output_to_console is false.
     */
    void set_global_debug();

  public:
    /** \brief Logs the message based on the internal state of the logger.
     * \param [in] obj A message object containing the current state of bweas.
     */
    void operator<<(const bweas::log_message &obj);

    /** \brief Exception handler for bweas::exception.
     * \param [in] excp The exception that should be thrown is not the main logger.
     */
    void handle_exception(const bweas::exception &excp);

  public:
    /** \brief Creates a logging file in the current directory(if it is not open).*/
    static void init();
    /** \brief Returns true if the bweas condition is error or fatal.
     * \return bool
     */
    static bool error_status();

  private:
    void handle(string &&str);

  private:
    static bool log, debug, output_to_console;
    static bweas::log_type global_status;
    static bweas::utils::file_utils::file file_log;

    string_v owner;
    bool is_main;

  private:
    static constexpr auto NAME_FILE_LOG = "bweas-last.log";
};

/** \brief The RAII class for blocking the output of messages to the console. */
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
