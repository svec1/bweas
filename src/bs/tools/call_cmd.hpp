//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef CALL_CMD__H
#define CALL_CMD__H

#include <bwtype.h>

#include <string>
#include <vector>

class call_cmd_manager {
  public:
    call_cmd_manager()                         = default;
    call_cmd_manager(const call_cmd_manager &) = default;

    ~call_cmd_manager() = default;

  public:
    void add_call_before(std::string call_command) {
        before_build.push_back(call_command);
    }
    void add_call_after(std::string call_command) {
        after_build.push_back(call_command);
    }

  protected:
    void call_bef() {
        for (u32t i = 0; i < before_build.size(); ++i) {
            system(before_build[i].c_str());
        }
    }
    void call_after() {
        for (u32t i = 0; i < after_build.size(); ++i) {
            system(after_build[i].c_str());
        }
    }

  private:
    std::vector<std::string> before_build;
    std::vector<std::string> after_build;
};

#endif
