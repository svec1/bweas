//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <format>

#include <bwlogger.hpp>
#include <iostream>

using namespace bweas;
using namespace bweas::utils;

bool logger::log = true, logger::debug = false;

file_utils::file logger::file_log;
log_type logger::global_status;
bool logger::output_to_console = true;

logger::logger(string_v _owner, bool _is_main) : owner(_owner), is_main(_is_main) {
}
logger::~logger() {
    if (!log && fs::exists(NAME_FILE_LOG)) {
        if (file_log.is_open)
            file_log.close();
        fs::remove(NAME_FILE_LOG);
    }
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
        break;
    default:
        break;
    }
}
void logger::set_global_log(bool _log) {
    log = _log;
}
void logger::set_global_debug() {
    debug = true;
}
bool logger::error_status() {
    return global_status == log_type::error || global_status == log_type::fatal;
}

void logger::operator<<(const log_message &obj) {
    if (!log)
        return;

    global_status = obj.log_t;

    size_t text_color;
    switch (global_status) {
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

    string _owner  = !owner.empty() ? string(" - [") + owner.data() + string("]: ") : " - ";
    string out_str = std::format("{}\033[3{}m{}\033[0m", _owner, text_color, obj.ss.str());

    handle(std::move(out_str));
}
void logger::handle_exception(const bweas::exception &excp) {
    global_status = log_type::fatal;
    handle(excp.what());
}
void logger::handle(string &&str) {
    if (!log)
        return;
    init();

    string out_to_file = str;
    out_to_file.erase(str.find("\033[3"), 5);
    out_to_file.erase(out_to_file.size() - 4, 4);

    if (global_status == log_type::fatal && !is_main)
        throw bweas::exception(str);

    file_utils::write_file(file_log, out_to_file + "\n");
    if (output_to_console || debug)
        std::fprintf(stdout, "%s\n", str.data());

    if (global_status == log_type::fatal)
        exit(1);
}
void logger::init() {
    if (!file_log.is_open)
        file_log = file_utils::open_file(NAME_FILE_LOG, file_utils::file::mode_file::open::w);
}
