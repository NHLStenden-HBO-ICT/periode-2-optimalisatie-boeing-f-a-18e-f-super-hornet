#pragma once
// add your includes to this file instead of to individual .cpp files
// to enjoy the benefits of precompiled headers:
// - fast compilation
// - solve issues with the order of header files once (here)
// do not include headers in header files (ever).

// Prevent expansion clashes (when using std::min and std::max):
#define NOMINMAX

#define HEALTHBAR_OFFSET 70
#define SCRWIDTH (1280 + (HEALTHBAR_OFFSET * 2))
#define SCRHEIGHT 720

// #define FULLSCREEN
// #define ADVANCEDGL	// faster if your system supports it

// Glew should be included first
#include <GL/glew.h>
// Comment for autoformatters: prevent reordering these two.
#include <GL/gl.h>

#ifdef _WIN32
// Followed by the Windows header
#include <Windows.h>

// Then import wglext: This library tries to include the Windows
// header WIN32_LEAN_AND_MEAN, unless it was already imported.
#include <GL/wglext.h>

#endif

// External dependencies:
#include <FreeImage.h>

#pragma warning(push)
#pragma warning(disable : 26812)
#include <SDL.h>
#pragma warning(pop)

// C++ headers
#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <deque>
#include <queue>
#include <future>
#include <mutex>
#include <thread>
#include <filesystem>

// Namespaced C headers:
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Header for AVX, and every technology before it.
// If your CPU does not support this, include the appropriate header instead.
// See: https://stackoverflow.com/a/11228864/2844473
#include <immintrin.h>

// clang-format off

// "Leak" common namespaces to all compilation units. This is not standard
// C++ practice but a mere simplification for this small project.
using namespace std;

#include "template.h"
#include "surface.h"

using namespace Tmpl8;

#include "thread_pool.h"

#include "tank.h"
#include "terrain.h"
#include "rocket.h"
#include "smoke.h"
#include "explosion.h"
#include "particle_beam.h"

#include "game.h"

// clang-format on
