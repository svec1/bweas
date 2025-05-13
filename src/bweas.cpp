//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#include "bs/bwbuild_sys.hpp"

int main(int argv, char **args) {
    bweas::bwbuilder bw(argv, args);
    bw.start();

    return 0;
}
