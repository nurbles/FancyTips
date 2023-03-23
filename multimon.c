// -----------------------------------------------------------------------
//                        T O P   O F   F I L E
// -----------------------------------------------------------------------
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
// -----------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef HMONITOR (WINAPI * FNMFW)( HWND hwnd, DWORD dwFlags );
typedef HMONITOR (WINAPI * FNMFP)( POINT pt, DWORD dwFlags );
typedef HMONITOR (WINAPI * FNMFR)( LPCRECT lprc, DWORD dwFlags );
typedef BOOL     (WINAPI * FNGMI)( HMONITOR hMonitor, LPMONITORINFO lpmi );

FNMFW fnMonitorFromWindow;
FNMFP fnMonitorFromPoint;
FNMFR fnMonitorFromRect;
FNGMI fnGetMonitorInfo;

// -----------------------------------------------------------------------

void mm_GetProcs( void )
{
   HMODULE  hm;

   hm = GetModuleHandle( TEXT("USER32") );
   if( hm )
   {
      fnMonitorFromWindow = (FNMFW)GetProcAddress( hm, TEXT("MonitorFromWindow") );
      fnMonitorFromPoint  = (FNMFP)GetProcAddress( hm, TEXT("MonitorFromPoint")  );
      fnMonitorFromRect   = (FNMFR)GetProcAddress( hm, TEXT("MonitorFromRect")   );
#ifdef UNICODE
      fnGetMonitorInfo    = (FNGMI)GetProcAddress( hm, TEXT("GetMonitorInfoW") );
#else
      fnGetMonitorInfo    = (FNGMI)GetProcAddress( hm, TEXT("GetMonitorInfoA")   );
#endif // !UNICODE
   }
   return;
}

// -----------------------------------------------------------------------

BOOL mm_GetCurrentScreenSize( HWND hwnd, SIZE *ps )
{
   HANDLE      hmon;
   MONITORINFO mi;

   if( (NULL == ps) || !IsWindow(hwnd) )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return FALSE;
   }

   if( fnMonitorFromWindow )
   {
      hmon = (fnMonitorFromWindow)( hwnd, MONITOR_DEFAULTTONEAREST );
      if( NULL != hmon )
      {
         mi.cbSize = sizeof(mi);
         if( (fnGetMonitorInfo)( hmon, &mi ) )
         {
            ps->cx = mi.rcWork.right - mi.rcWork.left;
            ps->cy = mi.rcWork.bottom - mi.rcWork.top;
            return TRUE;
         }
      }
   }
   else
   {
      SetLastError( ERROR_NOT_READY );
   }
   return FALSE;
}

// -----------------------------------------------------------------------

BOOL mm_GetCurrentScreenInfo( HWND hwnd, POINT *pp, SIZE *ps )
{
   HANDLE      hmon;
   MONITORINFO mi;
   BOOL        bResult = FALSE;

   if( IsWindow(hwnd) && (((NULL != pp)) || ((NULL != ps))) )
   {  // I need at least a window and one return value pointer
      if( fnMonitorFromWindow )
      {
         hmon = (fnMonitorFromWindow)( hwnd, MONITOR_DEFAULTTONEAREST );
         if( NULL != hmon )
         {
            mi.cbSize = sizeof(mi);
            if( (fnGetMonitorInfo)( hmon, &mi ) )
            {
               if( ps )
               {
                  ps->cx = mi.rcWork.right - mi.rcWork.left;
                  ps->cy = mi.rcWork.bottom - mi.rcWork.top;
               }
               if( pp )
               {
                  pp->x = mi.rcWork.left;
                  pp->y = mi.rcWork.top;
               }
               bResult = TRUE;
            }
         }
      }
      else
      {
         SetLastError( ERROR_NOT_READY );
      }
   }
   else
   {
      SetLastError( ERROR_INVALID_PARAMETER );
   }
   return bResult;
}

// -----------------------------------------------------------------------

BOOL mm_GetScreenRectFromPoint( POINT *pp, RECT *prc )
{
   HANDLE      hmon;
   MONITORINFO mi;
   BOOL        bResult = FALSE;

   if( (NULL == pp) || (NULL == prc) )
   {  // Bad parameter
      SetLastError( ERROR_INVALID_PARAMETER );
   }
   else if( fnMonitorFromPoint )
   {
      hmon = (fnMonitorFromPoint)( *pp, MONITOR_DEFAULTTONULL );
      if( NULL != hmon )
      {
         mi.cbSize = sizeof(mi);
         if( (fnGetMonitorInfo)( hmon, &mi ) )
         {
            *prc = mi.rcWork;
            bResult = TRUE;
         }
      }
   }
   else
   {
      SetLastError( ERROR_NOT_READY );
   }
   return bResult;
}

// -----------------------------------------------------------------------
//                      E N D   O F   F I L E
// -----------------------------------------------------------------------
