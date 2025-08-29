//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef VIRTUAL_TERMINAL_HPP
#define VIRTUAL_TERMINAL_HPP

#include <bwaliases.hpp>

#if defined(WIN)

#include <windows.h>

namespace bweas {
namespace utils {
class virtual_terminal;
}
} // namespace bweas

class bweas::utils::virtual_terminal {
  private:
    virtual_terminal() {
        static HANDLE STD_HANDLE = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(STD_HANDLE, &mode);
        SetConsoleMode(STD_HANDLE, mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    ~virtual_terminal() {
        static HANDLE STD_HANDLE = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleMode(STD_HANDLE, 0);
    }

  public:
    static void init() {
        static virtual_terminal vterminal;
    }
};

#endif

#endif
