#include "crbase/compiler/build_config.h"

#if defined(MINI_CHROMIUM_COMPILER_MSVC)
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "version.lib")
#endif