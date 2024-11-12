#ifndef EXCEPTION__H
#define EXCEPTION__H

#include <exception>
#include <string>

namespace bw_excp {

// Base exception class bweas, which inherits from std::exception.
// All child exception classes of bweas must inherit from it.
class bweas_exception : std::exception {
  public:
    bweas_exception(std::string _assist_err) noexcept : assist_err(_assist_err) {
    }
    ~bweas_exception() noexcept = default;

  public:
    virtual const char *what() const {
        return "";
    }

    const char *get_assist_err() const noexcept {
        return assist_err.c_str();
    }

    // NAME_CLASS_ERROR_000
    // For example: BWS000
    std::string assist_err;
};
} // namespace bw_excp

#endif