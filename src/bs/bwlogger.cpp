//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwlogger.hpp>

using namespace bweas;

bwtools::file_it logger::file_log;
log_type logger::global_status;

log_message::log_message(log_type _log_t) : log_t(_log_t) {
    switch (log_t) {
    case log_type::warning:
        ss << "[WARNING]: ";
        break;
    case log_type::error:
        ss << "[ERROR]: ";
        break;
    case log_type::fatal:
        ss << "[FATAL]: ";

    case log_type::msg:
    default:
        break;
    }
}
void logger::set_global_status() {
    global_status = status;
}
log_type logger::get_status() {
    return status;
}
void logger::dump_status() {
    status = bweas::log_type::msg;
}
bool logger::error_status() {
    return status == log_type::error || status == log_type::fatal;
}

logger &logger::operator<<(std::function<logger::handle_func_t> handle_func_callback) {
    handle_func = handle_func_callback;
    return *this;
}

void logger::operator<<(const log_message &obj) {
    status = obj.log_t;
    bwtools::write_file(bwtools::get_ref_file(file_log),
                        std::string("[") + owner.data() + std::string("]: ") + obj.ss.str());

    if (handle_func) {
        handle_func(std::string("[") + owner.data() + std::string("]: ") + obj.ss.str());
        handle_func = nullptr;
    }
}
