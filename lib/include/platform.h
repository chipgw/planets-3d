#pragma once

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#ifdef _MSC_VER
#define IMPORT __declspec(dllimport)
#else
#define IMPORT
#endif
