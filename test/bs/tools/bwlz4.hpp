#include <gtest/gtest.h>

#include <bs/utils/lz4.hpp>

#include <ctype.h>

using namespace bweas::utils;

TEST(BWWRAP_LZ4, CorrectStatementClassBwlz4CompressionDataConv) {
    std::string comp_data, decomp_data;
    ASSERT_NO_THROW({ comp_data = lz4::compress_data("Hello, World!"); });
    ASSERT_NO_THROW({ decomp_data = lz4::decompress_data(comp_data); });
    ASSERT_EQ(decomp_data, "Hello, World!");
    ASSERT_NE(comp_data, decomp_data);
}
