// ------------------------------------------------------------------------
// T O P   O F   F I L E
// ------------------------------------------------------------------------
// MIT License:
//
// Copyright 2023 Steven Valliere
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the “Software”), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ------------------------------------------------------------------------

#pragma once

// Including SDKDDKVer.h defines the highest available Windows platform.
// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "resource.h"

#if !defined(HANDLE_WM_MOUSEHOVER)
// ------------------------------------------------------------------------
// Missing WM_MOUSEHOVER message cracker...
// ------------------------------------------------------------------------
/* void Cls_OnMouseHover(HWND hwnd, UINT state, int x, int y) */
#define HANDLE_WM_MOUSEHOVER(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)
#define FORWARD_WM_MOUSEHOVER(hwnd, state, x, y, fn) \
    (void)(fn)((hwnd), WM_MOUSEHOVER, (WPARAM)(UINT)(state), MAKELPARAM((cx), (cy)))
#endif

#if !defined(HANDLE_WM_MOUSELEAVE)
// ------------------------------------------------------------------------
// Missing WM_MOUSELEAVE message cracker...
// ------------------------------------------------------------------------
/* void Cls_OnMouseLeave(HWND hwnd) */
#define HANDLE_WM_MOUSELEAVE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd)), 0L)
#define FORWARD_WM_MOUSELEAVE(hwnd, state, x, y, fn) \
    (void)(fn)((hwnd), WM_MOUSELEAVE, 0, 0 )
#endif

// ------------------------------------------------------------------------
// E N D   O F   F I L E
// ------------------------------------------------------------------------
