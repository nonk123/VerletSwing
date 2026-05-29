#pragma once

#include "cmake.h" // IWYU pragma: export

#ifdef __EMSCRIPTEN__
#define ASSETS "assets/"
#else
#define ASSETS DEBUG_ASSETS_DIR "/"
#endif
