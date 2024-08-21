#ifndef _CRT_DEBUG__H
#define _CRT_DEBUG__H

#ifdef _WIN32

#include <crtdbg.h>

class crtd {
  public:
    crtd();
    ~crtd();
};

crtd::crtd() {
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
}
crtd::~crtd() {
    //_CrtDumpMemoryLeaks();
}

#endif

#endif