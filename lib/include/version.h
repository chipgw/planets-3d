#pragma once

#ifdef _MSC_VER
#define IMPORT __declspec(dllimport)
#else
#define IMPORT
#endif

namespace version{
    IMPORT extern const char* git_revision;
    IMPORT extern const char* build_type;
    IMPORT extern const char* compiler;
    IMPORT extern const char* cmake_version;
}
