#ifndef _STORAGE_TINYDB_INCLUDE_EXPORT_H_
#define _STORAGE_TINYDB_INCLUDE_EXPORT_H_

#if !defined(TINYDB_EXPORT)

#if defined(TINYDB_SHARED_LIBRARY)
#if defined(_WIN32)

#if defined(TINYDB_COMPILE_LIBRARY)
#define TINYDB_EXPORT __declspec(dllexport)
#else
#define TINYDB_EXPORT __declspec(dllimport)
#endif  // defined(TINYDB_COMPILE_LIBRARY)

#else  // defined(_WIN32)
#if defined(TINYDB_COMPILE_LIBRARY)
#define TINYDB_EXPORT __attribute__((visibility("default")))
#else
#define TINYDB_EXPORT
#endif
#endif  // defined(_WIN32)

#else  // defined(TINYDB_SHARED_LIBRARY)
#define TINYDB_EXPORT
#endif

#endif  // !defined(TINYDB_EXPORT)

#endif  // _STORAGE_TINYDB_INCLUDE_EXPORT_H_
