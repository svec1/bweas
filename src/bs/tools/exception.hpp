#ifndef EXCEPTION__H
#define EXCEPTION__H

#include <exception>
#include <string>

namespace bw_excp {
class bweas_exception : std::exception {
  public:
    bweas_exception(std::string _assist_err) noexcept : assist_err(_assist_err) {
    }
    ~bweas_exception() noexcept = default;

  public:
    const char *get_assist_err() const noexcept {
        return assist_err.c_str();
    }

    std::string assist_err;
};
} // namespace bw_excp

#endif