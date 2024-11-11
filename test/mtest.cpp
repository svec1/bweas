#include "bs/tools/bwlua.hpp"
#include "bs/tools/bwlz4.hpp"

#include "bs/lang/lang_tests.hpp"


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}