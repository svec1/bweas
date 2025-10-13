//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#include "bs/bwbuild_sys.hpp"
#include "bs/utils/virtual_terminal.hpp"

int main(int argc, char **argv) {
#if defined(WIN)
    bweas::utils::virtual_terminal::init();
#endif

    bweas::logger::init();

    bweas::builder bw(argc, argv);
    bw.start();

    return 0;
}
