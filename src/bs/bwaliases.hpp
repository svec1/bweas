//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#ifndef BWALIASES_HPP
#define BWALIASES_HPP

#include <any>
#include <map>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <bwmacros_platform.h>

template <typename T> using vec              = std::vector<T>;
template <typename T> using uset             = std::unordered_set<T>;
template <typename K, typename V> using pair = std::pair<K, V>;
template <typename K, typename V> using map  = std::map<K, V>;
template <typename K, typename V> using umap = std::unordered_map<K, V>;

using pdiff    = std::ptrdiff_t;
using string   = std::string;
using string_v = std::string_view;
using any      = std::any;

#endif
