//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#include "bs/bwbuild_sys.hpp"

int main(int argc, char **argv) {
#if defined(WIN)
    bwtools::virtual_terminal::init();
#endif

    bweas::builder bw(argc, argv);
    bw.start();

    return 0;
}
