// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <windowsx.h>
#include <crtdbg.h>
#include <Uxtheme.h>

#ifndef ASSERT
#define ASSERT(expr)  _ASSERTE(expr)
#endif

// The arraysize(arr) macro returns the # of elements in an array arr.
// The expression is a compile-time constant, and therefore can be
// used in defining new arrays, for example.  If you use arraysize on
// a pointer by mistake, you will get a compile-time error.
template <typename T, size_t N>
char (&ArraySizeHelperT(T (&array)[N]))[N];

#define arraysize(array) (sizeof(ArraySizeHelperT(array)))
