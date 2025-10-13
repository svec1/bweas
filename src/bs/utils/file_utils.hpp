//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <bwaliases.hpp>

namespace bweas {

/** \brief Defines auxiliary utilities for bweas.*/
namespace utils {
class file_utils;
}
} // namespace bweas

/** \brief Defines functions for easy interaction with files. */
class bweas::utils::file_utils final {
    file_utils() = delete;

  public:
    struct file {
        struct mode_file {
          private:
            mode_file() = delete;

          public:
            /** \brief Mode of open. */
            enum class open {
                rb = 0, ///< Open for read of binary.
                r,      ///< Open for default reading.
                w,      ///< Open for overwrite or create file with writing.
                wa,     ///< Open for writes to the end of the file.
                wb,     ///< Open for overwrite or create file with writing of binary.
                wba     ///< Open for writes to the end of the file of binary.
            };

            /** \brief Mode of reading. */
            enum class input {
                read_binary, ///< Read of binary.
                read_default ///< Default reading.
            };
            /** \brief Mode of writing. */
            enum class output {
                write_binary, ///< Write of binary(all bytes).
                write_default ///< Default writing.
            };
        };

      public:
        file()                        = default;
        file(const file &)            = delete;
        file &operator=(const file &) = delete;
        file(file &&_file) noexcept {
            if (_file.is_open)
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
            if (_file.is_open)
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
                is_open = 1;
        }

        // Closes the file if the stream is open
        void close() {
            if (is_open) {
                stream.close();
                is_open = 0;
            }
        }

      public:
        std::fstream stream;
        fs::path path_to;
        mode_file::open mode_open;
        bool is_open = 0;
    };

  public:
    static std::string get_time() {
        auto time    = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        tm *time_now = std::localtime(&time);

        return (time_now->tm_mon < 10 ? "0" + std::to_string(time_now->tm_mon) : std::to_string(time_now->tm_mon)) +
               "." +
               (time_now->tm_mday < 10 ? "0" + std::to_string(time_now->tm_mday) : std::to_string(time_now->tm_mday)) +
               " " + std::to_string(time_now->tm_hour) + ":" + std::to_string(time_now->tm_min);
    }

    static fs::path get_path_program() {
#if defined(WIN)
        std::string str(MAX_PATH, '\0');
        GetModuleFileNameA(NULL, str.data(), MAX_PATH);
#elif defined(UNIX)
        std::string str(PATH_MAX, '\0');
        readlink("/proc/self/exe", str.data(), PATH_MAX);
#endif
        str.erase(str.find_last_of("/\\") + 1, str.size());

        return str;
    }

  public:
    static file open_file(string_v name_file, file::mode_file::open mode = file::mode_file::open::rb) {
        return file_utils::file{fs::absolute(name_file), mode};
    }

    static string read_file(file &file, file::mode_file::input mode = file::mode_file::input::read_default) {
        std::string data_file;
        if (mode == file::mode_file::input::read_binary) {
            size_t size_file;
            file.stream.seekg(0, std::ios::end);
            size_file = file.stream.tellg();
            file.stream.seekg(0, std::ios::beg);
            data_file.resize(size_file);
            file.stream.read(data_file.data(), size_file);
        }
        else {
            std::string tmp;
            while (std::getline(file.stream, tmp))
                data_file += tmp + "\n";
        }

        return data_file;
    }
    static string read_file(file &&file, file::mode_file::input mode = file::mode_file::input::read_default) {
        return read_file(file, mode);
    }
    static void write_file(file &file, string_v buf,
                           file::mode_file::output mode = file::mode_file::output::write_default) {
        if (mode == file::mode_file::output::write_binary)
            file.stream.write(buf.data(), buf.size());
        else
            file.stream << buf;
    }
    static void write_file(file &&file, string_v buf,
                           file::mode_file::output mode = file::mode_file::output::write_default) {
        write_file(file, buf, mode);
    }

  public:
    /** \brief Creates an array of files matching the name mask.
     * The syntax is fully compliant with the glob() standard.
     */
    static vec<string> file_slc_mask(string mask, const vec<string> &files) {
        if (mask.empty())
            return files;

        vec<string> slc_files;
        string files_str;

        for (const auto &file : files)
            files_str += file + " ";

        string mask_regex;
        for (size_t j = 0; j < mask.size(); ++j)
            if (mask[j] == '*')
                mask_regex += "\\w+";
            else if (mask[j] == '.')
                mask_regex += "\\.";
            else if (mask[j] == '?')
                mask_regex += "\\w";
            else if (mask[j] == '/')
                mask_regex += "\\/";
            else
                mask_regex += mask[j];

        if (mask_regex.find("/") == mask_regex.npos) {
            std::regex file_mask("[\\/\\w+]*" + mask_regex);
            for (auto it_match = std::sregex_iterator(files_str.begin(), files_str.end(), file_mask);
                 it_match != std::sregex_iterator(); ++it_match)
                slc_files.push_back(it_match->str());
        }
        else {

            std::regex file_mask(mask_regex);
            for (auto it_match = std::sregex_iterator(files_str.begin(), files_str.end(), file_mask);
                 it_match != std::sregex_iterator(); ++it_match)
                slc_files.push_back(it_match->str());
        }
        return slc_files;
    }

    /** \brief Returns the absolute path to an existing file, considering the current directory. */
    static string get_path_file(string name_file) {
        if (auto path_file = fs::weakly_canonical(fs::current_path() / name_file); fs::is_regular_file(path_file))
            return path_file.string();

        return {};
    }

    /** \brief Returns the absolute path to an existing file. */
    static string get_path_file(string name_file, const vec<string> &possible_paths) {
        if (auto path_file = get_path_file(name_file); !path_file.empty())
            return path_file;

        fs::path current_path_tmp = fs::current_path();
        fs::path find_path_file;

        for (const auto &path : possible_paths) {
            fs::current_path(fs::weakly_canonical(path));
            if (auto path_file = fs::weakly_canonical(fs::current_path() / name_file); fs::is_regular_file(path_file)) {
                find_path_file = path_file;
                break;
            }
        }

        fs::current_path(current_path_tmp);

        return find_path_file.string();
    }
};

#endif
