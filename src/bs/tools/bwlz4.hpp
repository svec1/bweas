//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWLZ4__h
#define BWLZ4__h

#include <bwaliases.hpp>

#include <lz4.h>
#include <string>
#include <string_view>

#define MB *1048576

namespace bwlz4 {

// Compresses data based on lz4. If unsuccessful, returns an empty string
static inline std::string compress_data(std::string_view data) {
    if (data.size() >= LZ4_MAX_INPUT_SIZE || data.size() == 0)
        return "";
    char *comp_data = (char *)malloc(LZ4_compressBound(data.size()));
    pdiff size_comp_data;
    if ((size_comp_data = LZ4_compress_default(data.data(), comp_data, data.size(), LZ4_compressBound(data.size()))) <=
        0) {
        free(comp_data);
        return "";
    }

    std::string comp_data_str(comp_data, size_comp_data);
    free(comp_data);

    return comp_data_str;
}

// Decompresses data based on lz4. In case of failure, expected_size = 0 or >= LZ4_MAX_INPUT_SIZE, an empty string is
// returned.
static inline std::string decompress_data(std::string_view compress_data, size_t expected_size = 2 MB) {
    if (compress_data.size() >= LZ4_MAX_INPUT_SIZE || compress_data.size() == 0)
        return "";
    char *decomp_data = (char *)malloc(expected_size);
    pdiff size_data;
    if ((size_data = LZ4_decompress_safe(compress_data.data(), decomp_data, compress_data.size(), expected_size)) < 0) {
        free(decomp_data);
        return "";
    }

    std::string decomp_data_str(decomp_data, size_data);
    free(decomp_data);

    return decomp_data_str;
}

} // namespace bwlz4

#endif
