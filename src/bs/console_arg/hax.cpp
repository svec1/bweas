#include "hand_arg.hpp"

#include "../../mdef.hpp"
#include "../conf_var.hpp"

const char *attrib::build = "build";
const char *attrib::_export = "exp";

hax::hax(char **args, int argv) {
    if (init_glob == NULL) {
        init_glob = 1;
        assist.add_err("HAX000", "The number of instances of the \"hax\" class cannot exceed one");
        assist.add_err("HAX001", "Unknown syntax");
        assist.add_err("HAX002", "Unknown command");
        assist.add_err("HAX003", "Unknown subcommand");
        for (u32t i = 0; i < argv; ++i) {
            arg_s.push_back(std::string(args[i]));
        }
        return;
    }
    assist.call_err("HAX000");
}

std::string
hax::operator[](u32t ind) {
    if (ind >= arg_s.size() || ind < 0)
        return {};
    return arg_s[ind];
}

void
hax::handle(std::string arg) {
    switch (arg[0]) {
    case '-':
        arg.erase(0, 1);
        // assist.call_err("HAX002", "\"" + arg + "\"(???)");
        break;
    case '/':
        arg.erase(0, 1);
        if (arg.find(":") == arg.npos) {
            assist.call_err("HAX001", "\":\" is expected after the name- " + arg);
        }
        else if (arg.find(":") == 1) {
            assist.call_err("HAX001", "\"name variable\" is expected before \":\"- " + arg);
        }

        switch (arg[0]) {
        case attrib::subcom::v:

            break;
        case attrib::subcom::l:

            break;
        case attrib::subcom::d:

            break;
        case attrib::subcom::c:

            break;
        default:
            assist.call_err("HAX003", "\"" + arg + "\"(???)");
            break;
        }

        break;

    default:

        break;
    }
}
void
hax::handle_all() {
    for (u32t i = 0; i < arg_s.size(); ++i) {
        handle(arg_s[i]);
    }
}