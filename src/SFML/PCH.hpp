////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

#ifndef SFML_ENABLE_PCH
#error "PCH header included, but `SFML_ENABLE_PCH` was not defined"
#else // SFML_ENABLE_PCH

////////////////////////////////////////////////////////////
// Precompiled Headers
////////////////////////////////////////////////////////////

#include <SFML/Config.hpp>

#ifdef SFML_SYSTEM_WINDOWS

#include <SFML/System/Win32/WindowsHeader.hpp>

#include <dinput.h>
#include <mmsystem.h>

#endif // SFML_SYSTEM_WINDOWS

#include <SFML/System/AlgorithmUtils.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/InPlacePImpl.hpp>
#include <SFML/System/Macros.hpp>
#include <SFML/System/MathUtils.hpp>
#include <SFML/System/Path.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/UniquePtr.hpp>
#include <SFML/System/Vector2.hpp>

#include <algorithm>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#ifdef SFML_BUILD_EXAMPLES
#include <random>
#endif // SFML_BUILD_EXAMPLES

#ifdef SFML_BUILD_TEST_SUITE
#include <doctest/parts/doctest_fwd.h>
#endif // SFML_BUILD_TEST_SUITE

#endif // SFML_ENABLE_PCH
