
#ifndef STORAGE_TINYDB_PORT_PORT_H_
#define STORAGE_TINYDB_PORT_PORT_H_

#include <string.h>

// Include the appropriate platform specific file below.  If you are
// porting to a new platform, see "port_example.h" for documentation
// of what the new port_<platform>.h file must provide.
#if defined(TINYDB_PLATFORM_POSIX) || defined(TINYDB_PLATFORM_WINDOWS)
#include "port/port_stdcxx.h"
#endif

#endif  // STORAGE_TINYDB_PORT_PORT_H_
