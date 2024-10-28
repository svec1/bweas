#ifndef BWPACKAGE__H
#define BWPACKAGE__H

#include <nlohmann/json.hpp>
#include <string>

namespace bweas {
class bwpackage {
  public:
    bwpackage() = delete;
    bwpackage(std::string path_pckg);

  private:
};
} // namespace bweas

#endif