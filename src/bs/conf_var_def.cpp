#include "conf_var.hpp"

namespace config_var {
const char *version{"0.1d"};

bool logging{1};
bool warning_prj{1};

bool export_var{0};

const char *logfile_name{"bwlog.txt"};
} // namespace config_var