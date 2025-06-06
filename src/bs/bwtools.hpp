//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWTOOLS_HPP
#define BWTOOLS_HPP

#include <bwaliases.hpp>

#include <filesystem>
#include <fstream>

#ifndef _DEBUG
#define FATAL(str)                                                                                                     \
    if (errno)                                                                                                         \
    bwtools::fatal(str)

#else
#define FATAL(str)                                                                                                     \
    if (errno)                                                                                                         \
    bwtools::fatal(std::string(__FILE__) + ":" + std::to_string(__LINE__) + " " + std::string(__func__) + "() " str)
#endif

#define FATAL_ERROR 1

namespace bweas {
class bwtools;
extern bwtools _bwtools;
} // namespace bweas

// A base class that implements many auxiliary functions
class bweas::bwtools {
  public:
    bwtools() = delete;

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
        file(std::filesystem::path _path_to, mode_file::open _mode_open) : path_to(_path_to), mode_open(_mode_open) {
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

        void open(mode_file::open _mode_open, std::filesystem::path _path_to = {}) {
            path_to   = _path_to;
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
        std::filesystem::path path_to;
        mode_file::open mode_open;
        bool file_opened{0};
    };

  public:
    using file_it = size_t;

    static void message(std::string_view str);
    static void success(std::string_view str);

    static void warning(std::string_view str_warn);
    static void error(std::string_view str_error = "");
    static void fatal(std::string_view str_error = "");

    static std::string get_time();

    static std::string get_path_program();
    static std::string get_current_path();

  public:
    static file_it open_file(std::string_view name_file, file::mode_file::open mode = file::mode_file::open::r);
    static void close_file(file_it file);

    static bool exist_file(file_it file);
    static bool exist_file(std::string_view name_file);

    static file_it get_iterator_file(std::string_view name_file);
    static file &get_ref_file(file_it file);

    static std::string read_file(file &file, file::mode_file::input mode = file::mode_file::input::read_default);
    static void write_file(file &file, std::string_view buf,
                           file::mode_file::output mode = file::mode_file::output::write_default);

  private:
    static std::vector<file> files;
};

#endif
