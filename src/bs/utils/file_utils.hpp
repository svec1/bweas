//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWFILE__H
#define BWFILE__H

#include <bwaliases.hpp>

namespace bweas {
namespace utils {
class file_utils;
}
} // namespace bweas

class bweas::utils::file_utils {
  public:
    file_utils() = delete;

  public:
    struct file {
        struct mode_file {
          private:
            mode_file() = delete;

          public:
            // mode of open
            enum class open {
                // open for read of binary
                rb = 0,
                // open for default reading
                r,
                // open for overwrite or create file with writing
                w,
                // open for writes to the end of the file
                wa,
                // open for overwrite or create file with writing of binary
                wb,
                // open for writes to the end of the file of binary
                wba
            };

            // mode of reading
            enum class input {
                // read of binary
                read_binary,
                // default reading
                read_default
            };
            // mode of writing
            enum class output {
                // write of binary(all bytes)
                write_binary,

                // default writing
                write_default
            };
        };

      public:
        file()                        = default;
        file(const file &)            = delete;
        file &operator=(const file &) = delete;
        file(file &&_file) noexcept {
            if (_file.file_opened)
                _file.stream.close();
            path_to   = _file.path_to;
            mode_open = _file.mode_open;
            open();
        }
        file(fs::path _path_to, mode_file::open _mode_open) : path_to(_path_to), mode_open(_mode_open) {
            open();
        }
        ~file() {
            close();
        }
        file &operator=(file &&_file) noexcept {
            if (_file.file_opened)
                _file.stream.close();
            path_to   = _file.path_to;
            mode_open = _file.mode_open;
            open();

            return *this;
        }

        void open(mode_file::open _mode_open, fs::path _path_to = {}) {
            if (!_path_to.empty())
                path_to = _path_to;
            mode_open = _mode_open;
            open();
        }

        // Opens a file with the specified mode
        void open() {
            if (mode_open == mode_file::open::rb)
                stream.open(path_to.string(), std::ios::in | std::ios::binary);
            else if (mode_open == mode_file::open::r)
                stream.open(path_to.string(), std::ios::in);
            else if (mode_open == mode_file::open::w)
                stream.open(path_to.string(), std::ios::out | std::ios::trunc);
            else if (mode_open == mode_file::open::wb)
                stream.open(path_to.string(), std::ios::out | std::ios::binary | std::ios::trunc);
            else if (mode_open == mode_file::open::wba)
                stream.open(path_to.string(), std::ios::out | std::ios::binary | std::ios::app);
            else
                stream.open(path_to.string(), std::ios::out | std::ios::app);

            if (stream.is_open())
                file_opened = 1;
        }

        // Closes the file if the stream is open
        void close() {
            if (file_opened) {
                stream.close();
                file_opened = 0;
            }
        }

      public:
        std::fstream stream;
        fs::path path_to;
        mode_file::open mode_open;
        bool file_opened{0};
    };

  public:
    using file_it = size_t;

    static string get_time();

    static string get_path_program();
    static string get_current_path();

  public:
    static file_it open_file(std::string_view name_file, file::mode_file::open mode = file::mode_file::open::r);
    static void close_file(file_it file);

    static bool exist_file(file_it file);

    static file_it get_iterator_file(std::string_view name_file);
    static file &get_ref_file(file_it file);

    static string read_file(file &file, file::mode_file::input mode = file::mode_file::input::read_default);
    static void write_file(file &file, string_v buf,
                           file::mode_file::output mode = file::mode_file::output::write_default);

  public:
    // Creates an array of file names based on the mask
    // passed to the function and an array of all files.
    // ### The syntax is fully compliant with the glob() standard.
    static vec<string> file_slc_mask(string mask, const vec<string> &files);

    // Returns the absolute path to an existing file, considering the current directory.
    static string get_path_file(string name_file);
    // Returns the absolute path to an existing file, considering all possible paths, including the current directory.
    static string get_path_file(string name_file, const vec<string> &possible_paths);

  private:
    static vec<file> files;
};

#endif
