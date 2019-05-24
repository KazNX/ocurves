// Copyright (c) 2017
// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
// ABN 41 687 119 230
//
// Author: Kazys Stepanas

//
// Project configuration header. This is a generated header; do not modify
// it directly. Instead, modify the config.h.in version and run CMake again.
//
#ifndef PLOTSCONFIG_H_
#define PLOTSCONFIG_H_

// #include "plots_export.h"

// MSVC does not define common maths constants like M_PI by default.
// Make sure they are defined if we include cmath
// Include qwt_math.h to ensure _USE_MATH_DEFINES is defined in one place.
#include "qwt_math.h"
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES 1
#endif // _USE_MATH_DEFINES
#ifndef NOMINMAX
// Disable MSVC min() and max() macros in favour of of function implementations.
#define NOMINMAX
#endif // NOMINMAX
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#ifdef _MSC_VER
// Avoid dubious security warnings for plenty of legitimate code
# ifndef _SCL_SECURE_NO_WARNINGS
#   define _SCL_SECURE_NO_WARNINGS
# endif // _SCL_SECURE_NO_WARNINGS
# ifndef _CRT_SECURE_NO_WARNINGS
#   define _CRT_SECURE_NO_WARNINGS
# endif // _CRT_SECURE_NO_WARNINGS
//#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#endif // _MSC_VER

#cmakedefine ALLOCTRACK_ENABLE

#ifdef ALLOCTRACK_ENABLE
#include <alloctrack.h>
#endif // ALLOCTRACK_ENABLE

#endif // PLOTSCONFIG_H_
