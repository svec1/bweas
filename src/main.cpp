#include <iostream>
#include <time.h>

#include "bs/bwbuild_sys.hpp"

int
main(int argv, char **args) {
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

    {
        hax cons(args, argv);
        assist.file_log_name("log.txt");
        u32t hand = assist.open_file("dd.txt");
        assist << assist.read_file(hand);
        assist.close_file(hand);

        bwbuilder bw;
        bw.start_build();

        assist << std::string("Sec: " + std::to_string((double)(clock() - beg) / CLOCKS_PER_SEC));
    }

    return 0;
}