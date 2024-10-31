#include <iostream>
#include <time.h>

#include "bs/bwbuild_sys.hpp"

#include "bs/tools/bwlua.hpp"

int main(int argv, char **args) {
    clock_t beg = clock();
    /*
        _err a;
        a.name_e = "ERRRR";
        a.desc_e = "TI LOX";
        a.ind = 23;
        init_assistf(KL_DEBUG_OUTPUT);

        pfile file = open_file_inp("dd.txt", "read text");
        pfile file2 = open_file_inp("dd2.txt", "read text2");
        pfile file3 = open_file_inp("dd3.txt", "read text3");

        file = get_pfile("dd.txt");

        char* buf2 = get_list_open_files();
        printf("File: %s", buf2);

        char* buf = read_file_inp(file);

        printf("Text: %s\n", buf);

        free(buf2);
        free(buf);

        close_file_assist("dd.txt");

        //err_fatal_ref(get_kernel_err(0));

        dump_assistf();
    */

    // system("echo lox");
    //    mmap(NULL, NULL, NULL, NULL, NULL, NULL);
    //     void *f = malloc(45);

    // std::string data = "Hello, World!";
    // std::string comp_data = bwlz4::compress_data(data);
    // assist << bwlz4::decompress_data(comp_data, data.size());

    // assist.set_file_log_name("log.txt");
    // bwlua::lua l("function _f(n, b, y, h) io.write(n*y+h) io.write(b) end");

    // for (u32t i = 0; i < 233333; ++i)
    //     l.call_function<void, int, const char *, int, int>("_f", i + 1, "lox", i, i - 1);
    // return 0;
    {
        try {

            bweas::bwbuilder bw(argv, args);
            bw.start();
            assist << std::string("Sec: " + std::to_string((double)(clock() - beg) / CLOCKS_PER_SEC));
        }
        catch (bweas::exception::bwbuilder_excp &excp) {
            assist.call_err(excp.get_assist_err(), excp.what());
        }
    }

    return 0;
}