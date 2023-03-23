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

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <Shlwapi.h>

HINSTANCE ghDLL;

// -----------------------------------------------------------------------
#define PACKVERSION(major,minor) MAKELONG(minor,major)
// -----------------------------------------------------------------------

HRESULT DllGetVersion( DLLVERSIONINFO *pdvi )
{
   if( pdvi->cbSize == sizeof(struct _DLLVERSIONINFO) )
   {
      pdvi->dwMajorVersion = 2;
      pdvi->dwMinorVersion = 6;
      pdvi->dwBuildNumber  = 95;
      pdvi->dwPlatformID   = DLLVER_PLATFORM_NT;
      return ERROR_SUCCESS;
   }
   return E_INVALIDARG;
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// Standard entry point for DLLs
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

BOOL WINAPI DllMain( HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved )
{
   UNREFERENCED_PARAMETER(lpReserved);

   switch( dwReason )
   {
   case DLL_PROCESS_ATTACH:   ghDLL = hDLL;  break;
   case DLL_THREAD_ATTACH :   break;
   case DLL_THREAD_DETACH :   break;
   case DLL_PROCESS_DETACH:   break;
   }
   return TRUE;
}

// -----------------------------------------------------------------------
//                        E N D   O F   F I L E
// -----------------------------------------------------------------------
