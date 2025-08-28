//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <format>

#include <bwlogger.hpp>

using namespace bweas;
using namespace bweas::utils;

bool logger::log = true;

file_utils::file_it logger::file_log;
log_type logger::global_status;
bool logger::output_to_console = true;

logger::logger(string_v _owner) : owner(_owner) {
    file_log = file_utils::open_file(NAME_FILE_LOG, file_utils::file::mode_file::open::w);
}
logger::~logger() {
    if (file_utils::get_ref_file(file_log).file_opened)
        file_utils::close_file(file_log);

    if (!log && fs::exists(NAME_FILE_LOG))
        fs::remove(NAME_FILE_LOG);
}

log_message::log_message(log_type _log_t) : log_t(_log_t) {
    switch (log_t) {
    case log_type::warning:
        ss << "WARNING ";
        break;
    case log_type::error:
        ss << "ERROR ";
        break;
    case log_type::fatal:
        ss << "FATAL ";

    case log_type::msg:
    default:
        break;
    }
}
void logger::set_global_log(bool _log) {
    log = _log;
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

void logger::operator<<(const log_message &obj) {
    if (!log)
        return;

    status = obj.log_t;

    size_t text_color;
    switch (status) {
    case log_type::error:
    case log_type::fatal:
        text_color = 1;
        break;
    case log_type::success:
        text_color = 2;
        break;
    case log_type::warning:
        text_color = 3;
        break;
    default:
        text_color = 7;
    }

    string _owner  = !owner.empty() ? string("[") + owner.data() + string("]: ") : "";
    string out_str = std::format("{}\e[3{}m{}\e[0m", _owner, text_color, obj.ss.str());

    file_utils::write_file(file_utils::get_ref_file(file_log), _owner + obj.ss.str() + "\n");

    if (output_to_console)
        std::fprintf(stdout, "%s\n", out_str.data());

    if (status == log_type::fatal)
        exit(1);
}
