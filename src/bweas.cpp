#include "bs/bwbuild_sys.hpp"

int main(int argv, char **args) {
    try {
        clock_t beg = clock();
        bweas::bwbuilder bw(argv, args);
        bw.set_logging();
        bw.start();
        assist << std::string("Sec: " + std::to_string((double)(clock() - beg) / CLOCKS_PER_SEC));
    }
    catch (bwexception::bweas_exception &excp) {
        assist.call_err(excp.get_assist_err(), excp.what());
    }

    return 0;
}