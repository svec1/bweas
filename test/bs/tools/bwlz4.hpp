#include <gtest/gtest.h>
#include <bs/tools/bwlz4.hpp>

#include <ctype.h>

TEST(BWWRAP_LZ4, CorrectStatementClassBwlz4CompressionDataConv){
    std::string comp_data, decomp_data;
    ASSERT_NO_THROW({ comp_data = bwlz4::compress_data("Hello, World!"); });
    ASSERT_NO_THROW({ decomp_data = bwlz4::decompress_data(comp_data); });
    ASSERT_EQ(decomp_data, "Hello, World!");
}