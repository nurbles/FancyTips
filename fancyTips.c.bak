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

// ------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
// ------------------------------------------------------------------------
#pragma warning( disable : 4100 ) // unreferenced formal parameter
#pragma warning( disable : 4200 ) // nonstandard extension used : zero-sized array in struct/union
#pragma warning( disable : 4210 ) // nonstandard extension used: function given file scope

#include "fancyTips.h"

// -- Functions from multimon.c --
extern void mm_GetProcs( void );
extern BOOL mm_GetCurrentScreenSize( HWND hwnd, SIZE *ps );
extern BOOL mm_GetCurrentScreenInfo( HWND hwnd, POINT *pp, SIZE *ps );
extern BOOL mm_GetScreenRectFromPoint( POINT *pp, RECT *prc );

#if defined(FT_DECLSPEC)
#undef FT_DECLSPEC
#endif

#if defined(_DLL)
#include "resource.h"
extern HINSTANCE ghDLL;
#define FT_DECLSPEC __declspec(dllexport)
#endif

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
static void dbg_printf( const char *szFormat, ... )
{
   char     szLine[1024];
   va_list  args;

   va_start( args, szFormat );
   {
      vsnprintf( szLine, sizeof(szLine), szFormat, args );
   }
   va_end( args );

   OutputDebugString( szLine );
   return;
}
//------------------------------------------------------------------------------
static LPSTR _GetSystemErrorText( LPSTR lpMsgBuf, size_t sMsgBufLen, DWORD dwErrorCode )
{
   char *cp;
   DWORD dwLen;

   if( sMsgBufLen )
   {
      dwLen = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                             NULL,
                             dwErrorCode,
                             MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),
                             lpMsgBuf,
                             (DWORD)sMsgBufLen,
                             NULL );
      if( dwLen )
      {
         // Remove any trailing whitespace/cr/lf
         cp = lpMsgBuf + (dwLen - 1);
         while( dwLen && isspace( (BYTE)(*cp) ) )
         {
            *cp = '\0';
            cp--;
            dwLen--;
         }
      }
      else
      {
         char  szTemp[256];

         sprintf_s( szTemp, sizeof(szTemp), "%#8.8x (%u):  Unknown system error code",
                  dwErrorCode, dwErrorCode );
         sprintf_s( lpMsgBuf, sMsgBufLen, "%s", szTemp );
      }
      return lpMsgBuf;
   }
   return NULL;
}
// ------------------------------------------------------------------------

#if !defined(HANDLE_WM_MOUSEHOVER)
// ------------------------------------------------------------------------
// Missing WM_MOUSEHOVER message cracker...
// ------------------------------------------------------------------------
/* void Cls_OnMouseHover(HWND hwnd, UINT state, int x, int y) */
#define HANDLE_WM_MOUSEHOVER(hwnd, wParam, lParam, fn) \
((fn)((hwnd),(UINT)(wParam),(int)(short)LOWORD(lParam),(int)(short)HIWORD(lParam)), 0L)
#define FORWARD_WM_MOUSEHOVER(hwnd, state, x, y, fn) \
(void)(fn)((hwnd), WM_MOUSEHOVER,(WPARAM)(UINT)(state), MAKELPARAM((cx),(cy)))
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
#define MaxFancyFonts      16
#define MaxFancyName       32
#define MaxFancyTabs       32
#define MaxFancyControls   32
#define DefFancyTab         8
#define DefFancyMargin      8
// ------------------------------------------------------------------------

typedef struct FancyFont_s
{
   char        szName[MaxFancyName];
   LOGFONT     lf;
   TEXTMETRIC  tm;
   HFONT       hf;
   int         tabChars;
   int         tab[MaxFancyTabs];
   int         ctab;
} FancyFont;

typedef enum FancyControl_e
{
   fcUnknown,

   fcFgColor,
   fcBgColor,
   fcFont,
   fcAlign,
   fcColumn,
   fcBreak,
   fcLeftAt,
   fcCenterAt,
   fcRightAt,
   fcDecimalAt,
   fcNoEscape,

   fcEnd = 0x08000
} FancyCtrlType;

typedef enum FancyAlign_e
{
   faNone,
   faLeft,
   faCenter,
   faRight,
   faDecimal,  // possible future enhancement
} FancyAlign;

typedef struct FancyControl_s
{
   FancyCtrlType  eType;   // Control type
   DWORD          val;     // New control value
   DWORD          prev;    // Previous value, restored when this control is closed
} FancyControl;

typedef struct FancyData_s
{
   HINSTANCE      hinst;
   HWND           hOwner;
   HWND           hTip;
   HWND           hwnd;
   POINT          pTip;
   RECT           rTip;
   BOOL           bVisible;
   int            idTip;
   volatile BOOL  bTipReady;  // TRUE when the tip is ready to be drawn, false while tip is being assembled
   HDC            hdc;
   HBITMAP        hbmNew;
   HBITMAP        hbmPrev;
   RECT           rBuf;
   HBRUSH         hbrBg;
   SIZE           sIcon;   // Icon size to use for custom icons
   FancyFont      f[MaxFancyFonts];
   int            cf;      // Number of fonts that have been registered
   int            xMarg;
   int            yMarg;
   COLORREF       crDefFg; // Default foreground
   COLORREF       crDefBg; // Default background
   COLORREF       crFg;    // Current foreground
   COLORREF       crBg;    // Current background
   BOOL           bNoEscape;  // TRUE when escape character processing is disabled
   int            iAlign;  // Current text alignment
   int            iFont;   // Currently selected font
   int            iAt;     // Current fancy alignment position
   int            iAtPrev; // Previous fancy alignment position
   FancyAlign     eAt;     // Current fancy alignment
   POINT          p;       // Current location where text emission begins
   SIZE           sLine;   // Current height and width of the text
   SIZE           sMax;    // Total height and width of the text
   FancyControl   fc[MaxFancyControls];
   int            cfc;     // Fancy control 'stack'
} FancyData;

#define IsEndSymbol( sym )((0==((sym)&fcEnd))?FALSE:TRUE)
#define MAKEDWORD(L,H)((DWORD)(((WORD)(((DWORD_PTR)(L)) & 0xffff)) |((DWORD)((WORD)(((DWORD_PTR)(H)) & 0xffff))) << 16))
//-------------------------------------------------------------------------

static int _FileExists( const char *szfqn )
{
   DWORD dw;

   dw = GetFileAttributes( szfqn );

   if( INVALID_FILE_ATTRIBUTES == dw )
   {
      SetLastError( ERROR_FILE_NOT_FOUND );
      return 0;
   }
   else if( dw & (FILE_ATTRIBUTE_DEVICE               // object is a device
                   |FILE_ATTRIBUTE_DIRECTORY            // object is a directory
                   |FILE_ATTRIBUTE_OFFLINE) )           // object is offline
   {
      SetLastError( ERROR_FILE_EXISTS );
      return -1;
   }
   SetLastError( ERROR_SUCCESS );
   return 1;
}

//-------------------------------------------------------------------------

static struct ColorNameTable_s
{
   char    *sz;
   COLORREF cr;
} s_ColorName[] =
{
   { "AliceBlue"           , RGB(0xF0,0xF8,0xFF) },
   { "AntiqueWhite"        , RGB(0xFA,0xEB,0xD7) },
   { "Aqua"                , RGB(0x00,0xFF,0xFF) },
   { "Aquamarine"          , RGB(0x70,0xDB,0x93) },
   { "Azure"               , RGB(0xF0,0xFF,0xFF) },
   { "Beige"               , RGB(0xF5,0xF5,0xDC) },
   { "Bisque"              , RGB(0xFF,0xE4,0xC4) },
   { "Black"               , RGB(0x00,0x00,0x00) },
   { "BlanchedAlmond"      , RGB(0xFF,0xFF,0xCD) },
   { "Blue"                , RGB(0x00,0x00,0xFF) },
   { "BlueBlack"           , RGB(0x2F,0x2F,0x4F) },
   { "BlueViolet"          , RGB(0x9E,0x5E,0x9E) },
   { "BrightGreenYellow"   , RGB(0xAD,0xFF,0x2F) },
   { "Brown"               , RGB(0xA5,0x2A,0x2A) },
   { "Burlywood"           , RGB(0xDE,0xB8,0x87) },
   { "CadetBlue"           , RGB(0x5F,0x9E,0xA0) },
   { "Caucasian"           , RGB(0xFF,0xA0,0x7A) },
   { "Chartreuse"          , RGB(0x7F,0xFF,0x00) },
   { "Chocolate"           , RGB(0xD2,0x69,0x1E) },
   { "Coral"               , RGB(0xFF,0x7F,0x50) },
   { "CornflowerBlue"      , RGB(0x64,0x95,0xED) },
   { "Cornsilk"            , RGB(0xFF,0xF8,0xDC) },
   { "Crimson"             , RGB(0xDC,0x14,0x3C) },
   { "Cyan"                , RGB(0x00,0x80,0x80) },
   { "DarkBlue"            , RGB(0x00,0x00,0x80) },
   { "DarkCornflowerBlue"  , RGB(0x42,0x42,0x6E) },
   { "DarkCyan"            , RGB(0x00,0x8B,0x8B) },
   { "DarkGoldenrod"       , RGB(0xB8,0x86,0x0B) },
   { "DarkGray"            , RGB(0xA9,0xA9,0xA9) },
   { "DarkGreen"           , RGB(0x00,0x64,0x00) },
   { "DarkIndianRed"       , RGB(0x4F,0x2F,0x2F) },
   { "DarkKhaki"           , RGB(0xBD,0xB7,0x6B) },
   { "DarkMagenta"         , RGB(0x8B,0x00,0x8B) },
   { "DarkOliveGreen"      , RGB(0x55,0x6B,0x2F) },
   { "DarkOrange"          , RGB(0xFF,0x8C,0x00) },
   { "DarkOrchid"          , RGB(0x99,0x32,0xCC) },
   { "DarkRed"             , RGB(0x8B,0x00,0x00) },
   { "DarkSalmon"          , RGB(0xE9,0x96,0x7A) },
   { "DarkSeaGreen"        , RGB(0x8F,0xBC,0x8F) },
   { "DarkSlateBlue"       , RGB(0x48,0x3D,0x8B) },
   { "DarkSlateGray"       , RGB(0x2F,0x4F,0x4F) },
   { "DarkSlateGrey"       , RGB(0x2F,0x4F,0x4F) },
   { "DarkTurquoise"       , RGB(0x70,0x93,0xDB) },
   { "DarkViolet"          , RGB(0x94,0x00,0xD3) },
   { "DeepPink"            , RGB(0xFF,0x14,0x93) },
   { "DeepSkyBlue"         , RGB(0x00,0xBF,0xFF) },
   { "DGray"               , RGB(0x40,0x40,0x40) },
   { "DimGray"             , RGB(0x54,0x54,0x54) },
   { "DimGrey"             , RGB(0x54,0x54,0x54) },
   { "DodgerBlue"          , RGB(0x1E,0x90,0xFF) },
   { "FadedBlueViolet"     , RGB(0x8A,0x2B,0xE2) },
   { "Firebrick"           , RGB(0x8E,0x23,0x23) },
   { "FloralWhite"         , RGB(0xFF,0xFA,0xF0) },
   { "ForestGreen"         , RGB(0x23,0x8E,0x23) },
   { "Fuchsia"             , RGB(0xFF,0x00,0xFF) },
   { "Gainsboro"           , RGB(0xDC,0xDC,0xDC) },
   { "GhostWhite"          , RGB(0xF8,0xF8,0xFF) },
   { "Gold"                , RGB(0xCC,0x7E,0x31) },
   { "Goldenrod"           , RGB(0xDA,0xA5,0x20) },
   { "Gray"                , RGB(0x80,0x80,0x80) },
   { "Green"               , RGB(0x00,0x80,0x00) },
   { "GreenYellow"         , RGB(0x93,0xDB,0x70) },
   { "Grey"                , RGB(0xBF,0xBF,0xBF) },
   { "Honeydew"            , RGB(0xF0,0xFF,0xF0) },
   { "HotPink"             , RGB(0xFF,0x69,0xB4) },
   { "IndianRed"           , RGB(0xCD,0x5C,0x5C) },
   { "Indigo"              , RGB(0x4B,0x00,0x82) },
   { "Ivory"               , RGB(0xFF,0xF0,0xF0) },
   { "Khaki"               , RGB(0x9E,0x9E,0x5E) },
   { "Lavender"            , RGB(0xE6,0xE6,0xFA) },
   { "LavenderBlush"       , RGB(0xFF,0xF0,0xF5) },
   { "Lawngreen"           , RGB(0x7C,0xFC,0x00) },
   { "LBlack"              , RGB(0x20,0x20,0x20) },
   { "LBlue"               , RGB(0x00,0x00,0xFF) },
   { "LCyan"               , RGB(0x00,0xFF,0xFF) },
   { "LemonChiffon"        , RGB(0xFF,0xFA,0xCD) },
   { "LGray"               , RGB(0xC0,0xC0,0xC0) },
   { "LGreen"              , RGB(0x00,0xFF,0x00) },
   { "LightAquamarine"     , RGB(0x7F,0xFF,0xD4) },
   { "LightBlue"           , RGB(0xAD,0xD8,0xE6) },
   { "LightCoral"          , RGB(0xF0,0x80,0x80) },
   { "LightCyan"           , RGB(0xE0,0xFF,0xFF) },
   { "LightFirebrick"      , RGB(0xB2,0x22,0x22) },
   { "LightGold"           , RGB(0xFF,0xD7,0x00) },
   { "LightGoldenrodYellow", RGB(0xFA,0xFA,0xD2) },
   { "LightGray"           , RGB(0xA8,0xA8,0xA8) },
   { "LightGreen"          , RGB(0x90,0xEE,0x90) },
   { "LightGrey"           , RGB(0xA8,0xA8,0xA8) },
   { "LightKhaki"          , RGB(0xF0,0xE6,0x8C) },
   { "LightPink"           , RGB(0xFF,0xC0,0xCB) },
   { "LightSalmon"         , RGB(0xFA,0x80,0x72) },
   { "LightSeaGreen"       , RGB(0x20,0xB2,0xAA) },
   { "LightSienna"         , RGB(0xA0,0x52,0x2D) },
   { "LightSkyBlue"        , RGB(0x87,0xCE,0xEB) },
   { "LightSlateGray"      , RGB(0x77,0x88,0x99) },
   { "LightSteelBlue"      , RGB(0x8E,0x8E,0xBC) },
   { "LightTurquoise"      , RGB(0xAC,0xE9,0xE9) },
   { "LightViolet"         , RGB(0xEE,0x82,0xEE) },
   { "LightWheat"          , RGB(0xF5,0xDE,0xB3) },
   { "LightYellow"         , RGB(0xFF,0xFF,0xE0) },
   { "Lime"                , RGB(0x00,0xFF,0x00) },
   { "LimeGreen"           , RGB(0x31,0xCC,0x31) },
   { "Linen"               , RGB(0xFA,0xF0,0xE6) },
   { "LMagenta"            , RGB(0xFF,0x00,0xFF) },
   { "LOrange"             , RGB(0xFF,0xBE,0x7D) },
   { "LRed"                , RGB(0xFF,0x00,0x00) },
   { "LWhite"              , RGB(0xFF,0xFF,0xFF) },
   { "LYellow"             , RGB(0xFF,0xFF,0x00) },
   { "Magenta"             , RGB(0x80,0x00,0x80) },
   { "Maroon"              , RGB(0x8E,0x23,0x6B) },
   { "MediumAquamarine"    , RGB(0x31,0xCC,0x99) },
   { "MediumBlue"          , RGB(0x31,0x31,0xCC) },
   { "MediumForestGreen"   , RGB(0x6B,0x8E,0x23) },
   { "MediumGoldenrod"     , RGB(0xE9,0xE9,0xAC) },
   { "MediumOrchid"        , RGB(0xBA,0x55,0xD3) },
   { "MediumPurple"        , RGB(0x93,0x70,0xDB) },
   { "MediumSeaBreen"      , RGB(0x3C,0xB3,0x71) },
   { "MediumSeaGreen"      , RGB(0x42,0x6E,0x42) },
   { "MediumSlateBlue"     , RGB(0x7B,0x68,0xEE) },
   { "MediumSpringGreen"   , RGB(0x00,0xFA,0x9A) },
   { "MediumTan"           , RGB(0xDB,0x93,0x70) },
   { "MediumTurquoise"     , RGB(0x48,0xD1,0xCC) },
   { "MediumVioletRed"     , RGB(0xC7,0x15,0x85) },
   { "MidnightBlue"        , RGB(0x19,0x19,0x70) },
   { "MintCream"           , RGB(0xF5,0xFF,0xFA) },
   { "MistyRose"           , RGB(0xFF,0xE4,0xE1) },
   { "Moccasin"            , RGB(0xFF,0xE4,0xB5) },
   { "NavajoWhite"         , RGB(0xFF,0xDE,0xAD) },
   { "Navy"                , RGB(0x23,0x23,0x8E) },
   { "NavyBlue"            , RGB(0x23,0x23,0x8E) },
   { "Oldlace"             , RGB(0xFD,0xF5,0xE6) },
   { "Olive"               , RGB(0x80,0x80,0x00) },
   { "Olivedrab"           , RGB(0x6B,0x8E,0x23) },
   { "Orange"              , RGB(0xFF,0xA5,0x00) },
   { "OrangeRed"           , RGB(0xFF,0x45,0x00) },
   { "Orchid"              , RGB(0xDA,0x70,0xD6) },
   { "PaleGoldenrod"       , RGB(0xEE,0xE8,0xAA) },
   { "PaleGreen"           , RGB(0x98,0xFB,0x98) },
   { "PaleTurquoise"       , RGB(0xAF,0xEE,0xEE) },
   { "PaleVioletred"       , RGB(0xDB,0x70,0x93) },
   { "PapayaWhip"          , RGB(0xFF,0xEF,0xD5) },
   { "PeachPuff"           , RGB(0xFF,0xEF,0xD5) },
   { "Peru"                , RGB(0xCD,0x85,0x3F) },
   { "Pink"                , RGB(0xBC,0x8E,0x8E) },
   { "Plum"                , RGB(0xDD,0xA0,0xDD) },
   { "PowderBlue"          , RGB(0xB0,0xE0,0xE6) },
   { "Purple"              , RGB(0x80,0x00,0x80) },
   { "Red"                 , RGB(0x80,0x00,0x00) },
   { "RosyBrown"           , RGB(0xBC,0x8F,0x8F) },
   { "RoyalBlue"           , RGB(0x41,0x69,0xE1) },
   { "SaddleBrown"         , RGB(0x8B,0x45,0x13) },
   { "Salmon"              , RGB(0x6E,0x42,0x42) },
   { "SandyBrown"          , RGB(0xF4,0xA4,0x60) },
   { "SeaGreen"            , RGB(0x23,0x8E,0x6B) },
   { "Seashell"            , RGB(0xFF,0xF5,0xEE) },
   { "Sienna"              , RGB(0x8E,0x6B,0x23) },
   { "Silver"              , RGB(0xC0,0xC0,0xC0) },
   { "SkyBlue"             , RGB(0x31,0x99,0xCC) },
   { "SlateBlue"           , RGB(0x00,0x7E,0xFF) },
   { "SlateGray"           , RGB(0x70,0x80,0x90) },
   { "Snow"                , RGB(0xFF,0xFA,0xFA) },
   { "SpringGreen"         , RGB(0x00,0xFF,0x7F) },
   { "SteelBlue"           , RGB(0x23,0x6B,0x8E) },
   { "Tan"                 , RGB(0xD2,0xB4,0x8C) },
   { "Teal"                , RGB(0x00,0x80,0x80) },
   { "Thistle"             , RGB(0xD8,0xBF,0xD8) },
   { "Tomato"              , RGB(0xFD,0x63,0x47) },
   { "Turquoise"           , RGB(0x40,0xE0,0xD0) },
   { "Violet"              , RGB(0x4F,0x2F,0x4F) },
   { "VioletRed"           , RGB(0xCC,0x31,0x99) },
   { "Wheat"               , RGB(0xD8,0xD8,0xBF) },
   { "White"               , RGB(0x80,0x80,0x80) },
   { "WhiteSmoke"          , RGB(0xF5,0xF5,0xF5) },
   { "Yellow"              , RGB(0x80,0x80,0x00) },
   { "YellowGreen"         , RGB(0x99,0xCC,0x31) },
};

static int s_nColors = sizeof(s_ColorName) / sizeof(s_ColorName[0]);

// ------------------------------------------------------------------------

static BYTE _CharToHex( char ch )
{
   switch( ch )
   {
   case '0': return 0x00;
   case '1': return 0x01;
   case '2': return 0x02;
   case '3': return 0x03;
   case '4': return 0x04;
   case '5': return 0x05;
   case '6': return 0x06;
   case '7': return 0x07;
   case '8': return 0x08;
   case '9': return 0x09;
   case 'A': return 0x0A;
   case 'B': return 0x0B;
   case 'C': return 0x0C;
   case 'D': return 0x0D;
   case 'E': return 0x0E;
   case 'F': return 0x0F;
   case 'a': return 0x0A;
   case 'b': return 0x0B;
   case 'c': return 0x0C;
   case 'd': return 0x0D;
   case 'e': return 0x0E;
   case 'f': return 0x0F;
   }
   return 0;
}

// ------------------------------------------------------------------------
// String compare ignoring whitespace, punctuation and case

static int _strispccmp( char *a, char *b )
{
   // First, parameter safety...
   if( !a && !b ) return  0;  // both strings NULL
   if( !a )       return +1;  // B greater than A
   if( !b )       return -1;  // A greater than B

   // Now, compare strings, ignoring case *AND* non-alpha-numeric chars
   while( *a && *b )
   {
      // Skip non-alpha-numeric chars
      while( *a && (isspace((BYTE)(*a)) || ispunct((BYTE)(*a))) )
      {
         a++;
      }
      while( *b && (isspace((BYTE)(*b)) || ispunct((BYTE)(*b))) )
      {
         b++;
      }
      // Are we done?
      if( !(*a) || !(*b) )
      {  // One of the strings has ended, we're outta here.
         break;
      }
      if( tolower((BYTE)(*a)) != tolower((BYTE)(*b)) )
      {  // Characters are different, we're done here.
         break;
      }
      // Next char in each string
      a++;
      b++;
   }
   return tolower((BYTE)(*a)) - tolower((BYTE)(*b));
}

//------------------------------------------------------------------------

static COLORREF _StringToColor( char *sz, COLORREF crDefault )
{
   union    bydw_u
   {
      DWORD dw;
      BYTE  by[4];

   }        hex = {0};
   COLORREF cr;
   int      cv[3];
   int      c;
   char    *cp;
   char    *cpDelim;
   char    *cpContext;
   char     szColor[256];

   cr = crDefault;
   if( sz && (*sz) )
   {  // There's a string included, check it out.
      if( ('#' == *sz) && *(sz+1) )
      {
         switch( strlen(sz) )
         {
         case 4:  // HTML / CSS - style color string  "#rgb"
            hex.by[2] = (_CharToHex( sz[1] ) << 4) | _CharToHex( sz[1] );
            hex.by[1] = (_CharToHex( sz[2] ) << 4) | _CharToHex( sz[2] );
            hex.by[0] = (_CharToHex( sz[3] ) << 4) | _CharToHex( sz[3] );
            cr = RGB( hex.by[2], hex.by[1], hex.by[0] );
            break;

         case 7:  // Assume HTML/CSS-style color string  "#rrggbb"
            hex.dw = strtoul( sz+1, NULL, 16 );
            cr = RGB( hex.by[2], hex.by[1], hex.by[0] );
            break;
         }
      }
      else if( !_strnicmp( sz, "rgb", 3 ) )
      {  // work in a WRITABLE buffer for strtok
         snprintf( szColor, sizeof(szColor), "%.*s", (int)sizeof(szColor)-1, sz );
         sz = szColor;
         memset( cv, 0, sizeof(cv) );
         cpDelim = "() ,";
         cp = strtok_s( sz+3, cpDelim, &cpContext );
         if( cp )
         {
            c = 0;
            do
            {
               cv[c++] = strtol( cp, NULL, 0 );
               cp = strtok_s( NULL, cpDelim, &cpContext );
            }while( cp && (c < 3) );
         }
         cr = RGB( cv[0], cv[1], cv[2] );
      }
      else // See if we know the name...
      {
         for( c = 0; c < s_nColors; c++ )
         {
            if( !_strispccmp( s_ColorName[c].sz, sz ) )
            {
               cr = s_ColorName[c].cr;
               break;
            }
         }
      }
   }
   return cr;
}

// -------------------------------------------------------------------------

static DWORD _GetFancyFont( FancyData *pfd, char *szVal )
{
   char *ep;
   int   f = 0;
   int   x;

   if( isdigit( *szVal ) )
   {
      f = strtol( szVal, &ep, 10 );
      if( ep && *ep )
      {
         goto FontName;
      }
      if( (f < 0) || (pfd->cf <= f) )
      {  // Out of range, use default font
         f = 0;
      }
   }
   else
   {
FontName:
      for( f = 0, x = 0; x < pfd->cf; x++ )
      {
         if( !_stricmp( szVal, pfd->f[x].szName ) )
         {
            f = x;
            break;
         }
      }
   }
   return f;
}

// -------------------------------------------------------------------------

static DWORD _GetFancyAlign( char *szVal )
{
   if( !_stricmp( szVal, "left" ) )
   {
      return TA_LEFT;
   }
   if( !_stricmp( szVal, "center" ) )
   {
      return TA_CENTER;
   }
   if( !_stricmp( szVal, "right" ) )
   {
      return TA_RIGHT;
   }
   return TA_LEFT;
}

// -------------------------------------------------------------------------

static void _CreateFancyFont( HWND hwnd, FancyFont *pff, LOGFONT *plf, char *szName )
{
   HDC   hdc;
   HFONT hf;

   pff->lf = *plf;
   pff->hf = CreateFontIndirect( &pff->lf );
   strcpy_s( pff->szName, sizeof(pff->szName), szName );

   hdc = GetDC( hwnd );
   {
      hf = SelectObject( hdc, pff->hf );
      GetTextMetrics( hdc, &pff->tm );
      SelectObject( hdc, hf );
   }
   ReleaseDC( hwnd, hdc );

   // Set default tab stops for the new font
   for( pff->ctab = 0; pff->ctab < MaxFancyTabs; pff->ctab++ )
   {
      pff->tab[pff->ctab] = (pff->ctab * 8) * pff->tm.tmAveCharWidth;
   }
   return;
}

// -------------------------------------------------------------------------

static FancyCtrlType _StrToCtrlType( FancyData *pfd, char *szCtrl )
{
   FancyCtrlType  eType = fcUnknown;
   BOOL           bEndSymbol;
   int            f;
   FancyFont     *pff;

   bEndSymbol = ( '/' == *szCtrl );
   if( bEndSymbol )
   {
      szCtrl++;
   }

   if(      !_stricmp( szCtrl, "ff" ) )
   {
      eType = fcFont;
   }
   else if( !_stricmp( szCtrl, "fg" ) )
   {
      eType = fcFgColor;
   }
   else if( !_stricmp( szCtrl, "bg" ) )
   {
      eType = fcBgColor;
   }
   else if( !_stricmp( szCtrl, "sc" ) )
   {
      eType = fcColumn;
   }
   else if( !_stricmp( szCtrl, "a" ) )
   {
      eType = fcAlign;
   }
   else if( !_stricmp( szCtrl, "l@" ) )
   {
      eType = fcLeftAt;
   }
   else if( !_stricmp( szCtrl, "c@" ) )
   {
      eType = fcCenterAt;
   }
   else if( !_stricmp( szCtrl, "r@" ) )
   {
      eType = fcRightAt;
   }
   else if( !_stricmp( szCtrl, "d@" ) )
   {
      eType = fcDecimalAt;
   }
   else if( !_stricmp( szCtrl, "br" ) )
   {
      eType = fcBreak;
   }
   else if( !_stricmp( szCtrl, "ne" ) )
   {
      eType = fcNoEscape;
   }
   else if( NULL != pfd )  // Look for a font name, which we will also allow as a tag
   {
      pff = pfd->f;
      for( f = 0; f < pfd->cf; f++ )
      {
         if( !_stricmp( szCtrl, pff[f].szName ) )
         {
            eType = fcFont;
            break;
         }
      }
   }

   if( bEndSymbol )
   {
      eType |= fcEnd;
   }

   return eType;
}

// -------------------------------------------------------------------------

static int _GetAtValue( FancyCtrlType eType, FancyData *pfd, char *szVal )
{
   int   denom;
   int   numer;
   int   val;
   char *ep;

   UNREFERENCED_PARAMETER(eType);

   if( '^' == *szVal )
   {  // @ center
      val = pfd->xMarg + ((pfd->sMax.cx - pfd->xMarg) / 2);
   }
   else if( '*' == *szVal )
   {  // most recently generated location
      val = pfd->iAtPrev;
   }
   else if( strchr( szVal, '/' ) )
   {  // @ fractional part of current width
      denom = strtol( szVal, &ep, 10 );
      if( ep && ('/' == *ep) )
      {
         numer = strtol( ep+1, NULL, 10 ) ;
         val = pfd->xMarg + MulDiv( pfd->sMax.cx - pfd->xMarg, denom, numer );
      }
      else
      {  // Missing numerator means we'll center by default
         val = pfd->xMarg + ((pfd->sMax.cx - pfd->xMarg) / 2);
      }
   }
   else
   {  // @ specific column
      val = strtol( szVal, NULL, 10 ) * pfd->f[pfd->iFont].tm.tmAveCharWidth;
   }
   pfd->iAtPrev = val;
   return val;
}

// -------------------------------------------------------------------------

static int _GetFancyCtrlType( FancyData *pfd, FancyControl *pfc, char **sz )
{
   char  szCtrl[256];
   char  szVal[256];
   char *cpTok;
   char *cp;
   BOOL  bDefaultValue = FALSE;

   cp = *sz;
   cpTok = szCtrl;
   while( *cp && ('>' != *cp) )
   {
      switch( *cp )
      {
      case '\n' : // Error!  not allowed within control symbol
         dbg_printf( "FancyTips:  Newline not allowed within control symbol\n" );
         return -1;

      case '=' :  // Control=value separator
      case ':' :  // Control:value separator
         *cpTok = '\0';
         cpTok = szVal; // Switch to collecting the value
         cp++;
         if( '>' == *cp )
         {  // There's no value present
            bDefaultValue = TRUE;
            continue;
         }
         break;
      }
      *cpTok = *cp;
      cp++;
      cpTok++;
   }
   *cpTok = '\0';
   cp++;

   *sz = cp;

   // Determine control type and value...
   pfc->eType = _StrToCtrlType( pfd, szCtrl );
   switch( pfc->eType )
   {
   case fcNoEscape: pfc->val = TRUE; break;  // Any value provided will be silently ignored
   case fcFont:
      if( _stricmp( szCtrl, "ff" ) )
      {  // This was a font name, move the tag to the value
         strcpy_s( szVal, sizeof(szVal), szCtrl );
         bDefaultValue = FALSE;
      }
      // else this was a <ff> tag, so choose the default font
   case fcFgColor:
   case fcBgColor:
   case fcColumn:
   case fcAlign:
   case fcLeftAt:
   case fcCenterAt:
   case fcRightAt:
   case fcDecimalAt:
   case fcBreak:
      if( bDefaultValue )
      {
         switch( pfc->eType )
         {
         case fcFont:      pfc->val = 0;                             break;
         case fcFgColor:   pfc->val = pfd->crDefFg;                  break;
         case fcBgColor:   pfc->val = pfd->crDefBg;                  break;
         case fcColumn:    pfc->val = pfd->p.x;                      break;
         case fcAlign:     pfc->val = TA_LEFT;                       break;
         case fcLeftAt:    pfc->val = pfd->p.x;                      break;
         case fcDecimalAt:
         case fcCenterAt:  pfc->val = (pfd->p.x + pfd->sMax.cx) / 2; break;
         case fcRightAt:   pfc->val = pfd->sMax.cx;                  break;
         }
      }
      else
      {
         switch( pfc->eType )
         {
         case fcFont:      pfc->val = _GetFancyFont( pfd, szVal );                     break;
         case fcFgColor:   pfc->val = (DWORD)_StringToColor( szVal, pfd->crDefFg ); break;
         case fcBgColor:   pfc->val = (DWORD)_StringToColor( szVal, pfd->crDefBg ); break;
         case fcColumn:    pfc->val = strtoul( szVal, NULL, 10 );                      break;
         case fcAlign:     pfc->val = _GetFancyAlign( szVal );                         break;
         case fcLeftAt:
         case fcDecimalAt:
         case fcCenterAt:
         case fcRightAt:   pfc->val = _GetAtValue( pfc->eType, pfd, szVal );           break;
         }
      }
      break;

   case fcUnknown:
      dbg_printf( "FancyTips:  Unknown control type '%s' (end)\n", szCtrl );
      return -1;

   case fcUnknown|fcEnd:
      dbg_printf( "FancyTips:  Unknown control type '%s'\n", szCtrl );
      return -1;
   }
   return 0;
}

//-------------------------------------------------------------------------

static void _FancyTextOut( HDC hdc, int x, int y, BYTE byCharSet, UINT uOpts,
                           LPRECT pr, char *str, int len, INT *lpDx )
{
   if( OEM_CHARSET == byCharSet )
   {  // Use ASCII text emitter
      ExtTextOutA( hdc, x, y, uOpts, pr, str, len, lpDx );
   }
   else
   {  // Use WIDE text emitter
      WCHAR wsz[1024];

      OemToCharBuffW( str, wsz, len );
      ExtTextOutW( hdc, x, y, uOpts, pr, wsz, len, lpDx );
   }
   return;
}

// -------------------------------------------------------------------------

static void _EmitFancyText( FancyData *pfd, char *sz, int len )
{
   SIZE        s;
   RECT        r;
   POINT       pAnchor;
   HFONT       hf;
   FancyFont  *pff;
   char       *cp;
   int         dot;

   if( len <= 0 )
   {  // nothing to emit (this happens when there's no text between control symbols)
      return;
   }

   pff = &pfd->f[pfd->iFont];

   // First, prepare the DC
   SetTextAlign( pfd->hdc, (faNone == pfd->eAt) ? pfd->iAlign : TA_LEFT );
   SetTextColor( pfd->hdc, pfd->crFg );
   SetBkColor(   pfd->hdc, pfd->crBg );
   hf = SelectObject( pfd->hdc, pff->hf );

   // Second, measure
   GetTextExtentPoint32( pfd->hdc, sz, len, &s );
   s.cy += pfd->f[pfd->iFont].tm.tmExternalLeading +
           pfd->f[pfd->iFont].tm.tmInternalLeading;
   pfd->sLine.cy = max( s.cy, pfd->sLine.cy );

   r.top    = pfd->p.y;
   r.bottom = r.top  + s.cy;

   switch( pfd->eAt )
   {
   default:
   case faNone:
      //r.left  = pfd->p.x;
      //r.right = max( pfd->sMax.cx - pfd->xMarg, r.left + s.cx );
      switch( pfd->iAlign )
      {
      default:
      case TA_LEFT:
         pfd->iAt = pfd->p.x;
         goto LeftAt;

      case TA_CENTER:
         pfd->iAt = (pfd->xMarg + max( pfd->sMax.cx, pfd->p.x + s.cx )) / 2;
         SetTextAlign( pfd->hdc, TA_LEFT );
         goto CenterAt;

      case TA_RIGHT:
         pfd->iAt = max( pfd->sMax.cx, pfd->p.x + s.cx );
         SetTextAlign( pfd->hdc, TA_LEFT );
         goto RightAt;

      }
      break;

   case faLeft:
LeftAt:
      r.left    = pfd->iAt;
      r.right   = max( pfd->sMax.cx - pfd->xMarg, r.left + s.cx );
      pAnchor.x = r.left;
      pfd->p.x = pAnchor.x + s.cx;
      break;

   case faCenter:
CenterAt:
      r.left    = pfd->iAt - (s.cx / 2);
      r.right   = r.left + s.cx;
      pAnchor.x = r.left;
      pfd->p.x = r.right;
      break;

   case faRight:
RightAt:
      r.left    = pfd->iAt - s.cx;
      r.right   = r.left + s.cx;
      pAnchor.x = r.left;
      pfd->p.x = pAnchor.x + s.cx;
      break;

   case faDecimal:   // Find the decimal and emit the first half right-aligned to the @ location
      cp = strchr( sz, '.' );
      if( cp )
      {
         SetTextAlign( pfd->hdc, TA_LEFT );
         dot = (int)(cp - sz);
         GetTextExtentPoint32( pfd->hdc, sz, dot, &s );
         s.cy += pfd->f[pfd->iFont].tm.tmExternalLeading +
                 pfd->f[pfd->iFont].tm.tmInternalLeading;
         r.top     = pfd->p.y;
         r.bottom  = r.top + s.cy;
         r.left    = pfd->iAt - s.cx;
         r.right   = r.left + s.cx;
         pAnchor.x = r.left;
         pAnchor.y = pfd->p.y;

         _FancyTextOut( pfd->hdc, pAnchor.x, pAnchor.y, pff->tm.tmCharSet, 0, &r, sz, dot, NULL );
         len      -= dot;
         sz        = cp;
         GetTextExtentPoint32( pfd->hdc, sz, len, &s );
         r.left    = pfd->iAt;
         r.right   = r.left + (max( pfd->sMax.cx - pfd->xMarg, r.left + s.cx ) - pfd->iAt);
         pAnchor.x = r.left;
      }
      else
      {  // No decimal, treat as <r@>
         r.left    = pfd->iAt - s.cx;
         r.right   = r.left + s.cx;
         pAnchor.x = r.left;
      }
      pfd->p.x = pAnchor.x + s.cx;
      break;
   }
   pAnchor.y = pfd->p.y;

   pfd->sLine.cx = max( pfd->sLine.cx, pfd->p.x );

   // Finally, emit to working screen buffer
   _FancyTextOut( pfd->hdc, pAnchor.x, pAnchor.y, pff->tm.tmCharSet, 0, &r, sz, len, NULL );
   SelectObject( pfd->hdc, hf );
   return;
}

// -------------------------------------------------------------------------
// Also works for cursors

static void _GetIconSize( __in HICON hico, __out SIZE *psiz )
{
   ICONINFO ii;
   BOOL     fResult;
   BITMAP   bm;

   fResult = GetIconInfo( hico, &ii );
   if( fResult )
   {
      fResult = GetObject( ii.hbmMask, sizeof(bm), &bm ) == sizeof(bm);
      if( fResult )
      {
         psiz->cx = bm.bmWidth;
         psiz->cy = ii.hbmColor ? bm.bmHeight : bm.bmHeight / 2;
      }
      if( ii.hbmMask  ) DeleteObject( ii.hbmMask  );
      if( ii.hbmColor ) DeleteObject( ii.hbmColor );
   }
   if( !fResult )
   {
      psiz->cx = GetSystemMetrics( SM_CXICON );
      psiz->cy = GetSystemMetrics( SM_CYICON );
   }
   return;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
#define isodigit(c) (('0'<=(c))&&((c)<='7'))
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

FT_DECLSPEC int ft_SetFancyTipTabs( HANDLE hTip, int xFont, int cTabs, int *tabs )
{
   FancyData  *pfd = (FancyData *)hTip;
   FancyFont  *pff;

   if( (NULL == pfd) || (xFont < 0) || (pfd->cf <= xFont) )
   {  // Bad parameter
      dbg_printf( "FancyTips:  bad param in " __FUNCTION__ "\n" );
      return -1;
   }

   pff = &pfd->f[xFont];

   if( cTabs < 0 )
   {  // Set custom tab step
      pff->ctab     = 0;
      pff->tabChars = abs(cTabs);
   }
   else if( 0 == cTabs )
   {  // restore default tabs
      pff->ctab     = 0;
      pff->tabChars = DefFancyTab;
   }
   else if( 0 < cTabs )
   {  // Set custom tabs
      cTabs = min( cTabs, MaxFancyTabs ); // Limit to prevent overrunning internal tab list
      for( pff->ctab = 0; pff->ctab < cTabs; pff->ctab++ )
      {
         pff->tab[pff->ctab] = tabs[pff->ctab] * pff->tm.tmAveCharWidth;
      }
   }

   if( 0 == pff->ctab )
   {
      pff->tab[0] = pfd->xMarg + (pff->tabChars * pff->tm.tmAveCharWidth);
      pff->ctab   = 1;
   }
   for( ; pff->ctab < MaxFancyTabs; pff->ctab++ )
   {
      pff->tab[pff->ctab] = pff->tab[pff->ctab-1] + (pff->tabChars * pff->tm.tmAveCharWidth);
   }

   return pff->ctab;
}

// -------------------------------------------------------------------------

FT_DECLSPEC int ft_AddFancyTipFont( HANDLE hTip, char *szName, LOGFONT *plf )
{
   FancyData  *pfd = (FancyData *)hTip;
   int         x;

   if( (NULL == pfd) ||
       (NULL == plf) ||
       (MaxFancyFonts <= pfd->cf) )
   {  // Bad parameter or font table full
      return -1;
   }

   x = pfd->cf;
   _CreateFancyFont( pfd->hTip, &pfd->f[x], plf, szName );
   pfd->cf += 1;

   return x;
}

// -------------------------------------------------------------------------

FT_DECLSPEC int ft_SetFancyTipColors( HANDLE hTip, COLORREF *pcrfg, COLORREF *pcrbg )
{
   FancyData  *pfd = (FancyData *)hTip;

   if( NULL == pfd )
   {  // Bad parameter
      //dbg_printf( "FancyTips:  bad param in " __FUNCTION__ "\n" );
      return -1;
   }

   if( pcrfg )
   {
      pfd->crDefFg  = *pcrfg;
   }
   else
   {
      pfd->crDefFg  = GetSysColor( COLOR_INFOTEXT );
   }
   if( pcrbg )
   {
      pfd->crDefBg  = *pcrbg;
   }
   else
   {
      pfd->crDefBg  = GetSysColor( COLOR_INFOBK   );
   }
   pfd->crFg        = pfd->crDefFg;
   pfd->crBg        = pfd->crDefBg;

   if( pfd->hbrBg )
   {
      DeleteObject( pfd->hbrBg );
   }
   pfd->hbrBg       = CreateSolidBrush( pfd->crDefBg );

   return 0;
}

// -------------------------------------------------------------------------

FT_DECLSPEC int ft_SetFancyTipMargin( HANDLE hTip, int xMarg, int yMarg )
{
   FancyData  *pfd = (FancyData *)hTip;

   if( NULL == pfd )
   {  // Bad parameter
      //dbg_printf( "FancyTips:  bad param in " __FUNCTION__ "\n" );
      return -1;
   }

   if( 0 <= xMarg )
   {
      pfd->xMarg = xMarg;
   }
   else
   {
      pfd->xMarg = DefFancyMargin;
   }
   if( 0 <= yMarg )
   {
      pfd->yMarg = yMarg;
   }
   else
   {
      pfd->yMarg = DefFancyMargin;
   }
   return 0;
}

// -------------------------------------------------------------------------

static int  iconDim[] = { 16, 20, 24, 32, 48, 64, 256 };
static int  cIconDim  = ARRAYSIZE(iconDim);

FT_DECLSPEC int ft_SetFancyTipIconSize( HANDLE hTip, SIZE *psIcon )
{
   FancyData  *pfd   = (FancyData *)hTip;
   SIZE        s;
   LONG_PTR    font  = (LONG_PTR)psIcon;
   int         h;
   int         d;
   int         lo;
   int         hi;

   if( NULL == pfd )
   {  // Bad parameter
      //dbg_printf( "FancyTips:  bad param in " __FUNCTION__ "\n" );
      return -1;
   }

   if( NULL == psIcon )
   {  // Restore default size
      psIcon = &s;
      psIcon->cx = GetSystemMetrics( SM_CXICON );
      psIcon->cy = GetSystemMetrics( SM_CYICON );
   }
   else if( (1 <= font) && (font <= MaxFancyFonts) )
   {  // 'Guess' icon size from specific font entry
      psIcon = &s;
      font -= 1;  // Adjust back to zero-relative indexing
      if( 0 < pfd->f[font].tm.tmHeight )
      {
         h = pfd->f[font].tm.tmHeight +
             pfd->f[font].tm.tmExternalLeading;

         h *= 2;  // We're looking for an icon that is about 2 text rows high

         if( h <= iconDim[0] )
         {
            psIcon->cx = iconDim[0];
         }
         else if( iconDim[cIconDim-1] <= h )
         {
            psIcon->cx = iconDim[cIconDim-1];
         }
         else
         {
            for( d = 1; d < cIconDim; d++ )
            {
               if( h < iconDim[d] )
               {
                  lo = h - iconDim[d-1];
                  hi = iconDim[d] - h;
                  if( lo < hi )
                  {
                     psIcon->cx = iconDim[d-1];
                  }
                  else
                  {
                     psIcon->cx = iconDim[d];
                  }
                  break;
               }
            }
         }
         psIcon->cy = psIcon->cx;
      }
      else
      {  // Unusable font, so...
         psIcon->cx = GetSystemMetrics( SM_CXICON );
         psIcon->cy = GetSystemMetrics( SM_CYICON );
      }
   }
   else if( (psIcon->cx < 16) || (256 < psIcon->cx) ||
            (psIcon->cy < 16) || (256 < psIcon->cy) )
   {  // Range error, clamp to fit.
      if( psIcon->cx < 16 )       psIcon->cx =  16;
      else if( 256 < psIcon->cx ) psIcon->cx = 256;

      if( psIcon->cy < 16 )       psIcon->cy =  16;
      else if( 256 < psIcon->cy ) psIcon->cy = 256;
   }

   pfd->sIcon.cx = psIcon->cx;
   pfd->sIcon.cy = psIcon->cy;
   return 0;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

static BOOL ftt_OnCreate( HWND hwnd, LPCREATESTRUCT lpCreateStruct )
{
   FancyData  *pfd;
   SIZE        size;
   HDC         hdc;

   pfd = (FancyData *)lpCreateStruct->lpCreateParams;

   SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)pfd );
   pfd->hTip = hwnd;

   hdc = GetDC( hwnd );
   {
      pfd->f[0].hf = GetCurrentObject( hdc, OBJ_FONT );
      GetObject( pfd->f[0].hf, sizeof(LOGFONT), &pfd->f[0].lf );
      GetTextMetrics( hdc, &pfd->f[0].tm );

      // Set default tab stops for the new font
      for( pfd->f[0].ctab = 0; pfd->f[0].ctab < MaxFancyTabs; pfd->f[0].ctab++ )
      {
         pfd->f[0].tab[pfd->f[0].ctab] = (pfd->f[0].ctab * 8) * pfd->f[0].tm.tmAveCharWidth;
      }

      ft_SetFancyTipColors( pfd, NULL, NULL );
      ft_SetFancyTipIconSize( pfd, NULL );

      pfd->hdc         = CreateCompatibleDC( hdc );

      size.cx          = GetSystemMetrics( SM_CXVIRTUALSCREEN );
      size.cy          = GetSystemMetrics( SM_CYVIRTUALSCREEN );

      pfd->hbmNew      = CreateCompatibleBitmap( hdc, size.cx, size.cy );
      pfd->hbmPrev     = SelectObject( pfd->hdc, pfd->hbmNew );

      pfd->xMarg       = DefFancyMargin;
      pfd->yMarg       = DefFancyMargin;

      pfd->rBuf.left   = 0;
      pfd->rBuf.top    = 0;
      pfd->rBuf.right  = size.cx;
      pfd->rBuf.bottom = size.cy;
   }
   ReleaseDC( hwnd, hdc );

   ft_SetFancyTipText( (HANDLE)pfd, " \n(new tool tip)\n ", NULL, NULL );
   return TRUE;
}

// -------------------------------------------------------------------------

static void ftt_OnDestroy( HWND hwnd )
{
   FancyData  *pfd;
   int         x;

   pfd = (FancyData *)GetWindowLongPtr( hwnd, GWLP_USERDATA );

   if( NULL != pfd )
   {
      SelectObject( pfd->hdc, pfd->hbmPrev );
      DeleteObject( pfd->hbmNew );
      DeleteObject( pfd->hbrBg );
      DeleteDC( pfd->hdc );

      for( x = 0; x < pfd->cf; x++ )
      {
         DeleteObject( pfd->f[x].hf );
      }
      free( pfd );
      SetWindowLongPtr( hwnd, GWLP_USERDATA, 0 );
   }
   return;
}

// -------------------------------------------------------------------------

static void ftt_OnPaint( HWND hwnd )
{
   FancyData  *pfd;
   PAINTSTRUCT ps;
   HDC         hdc;
   SIZE        size;
   RECT        r;

   pfd = (FancyData *)GetWindowLongPtr( hwnd, GWLP_USERDATA );

   hdc = BeginPaint( hwnd, &ps );
   if( NULL != pfd )
   {
      do // once
      {
         if( (pfd->sMax.cx <= pfd->xMarg) ||
             (pfd->sMax.cy <= pfd->yMarg) )
         {  // Nothing to draw!
            break;
         }

         r.left   = 1;
         r.top    = 1;
         r.right  = pfd->sMax.cx-1;
         r.bottom = pfd->sMax.cy-1;

         FrameRect( pfd->hdc, &r, GetStockObject( DKGRAY_BRUSH ) );
         InflateRect( &r, 1, 1 );
         FrameRect( pfd->hdc, &r, GetStockObject( DKGRAY_BRUSH ) );

         size.cx = (r.right  - r.left);// + 2;
         size.cy = (r.bottom - r.top );// + 2;

         if( 0 == BitBlt( hdc,          // Target DC
                          r.left,       // Target Left(X) Coord
                          r.top,        // Target Top(Y) Coord
                          size.cx,      // Copy X Dimension
                          size.cy,      // Copy Y Dimension
                          pfd->hdc,     // Source DC
                          r.left,       // Source Left(X) Coord
                          r.top,        // Source Top(Y) Coord
                          SRCCOPY ) )   // Operation
         {
            char  sz[256];
            DWORD le = GetLastError();

            dbg_printf( "FancyTips:  BitBlt error:  %s\n",
                        _GetSystemErrorText(sz,sizeof(sz),le) );
         }

         if( pfd->bVisible )
         {
            SetWindowPos( pfd->hTip, pfd->hOwner,
                          pfd->rTip.left, pfd->rTip.top,
                          pfd->rTip.right - pfd->rTip.left,
                          pfd->rTip.bottom - pfd->rTip.top,
                          SWP_NOACTIVATE );
            ShowWindow( hwnd, SW_SHOW );
         }
      }while(FALSE);
   }
   EndPaint( hwnd, &ps );
   return;
}

// -------------------------------------------------------------------------

static LPARAM _lpPointClientToClient( HWND hsrc, LPARAM lParam, HWND htgt )
{
   POINT p;

   p.x = (int)LOWORD(lParam);
   p.y = (int)HIWORD(lParam);

   ClientToScreen( hsrc, &p );
   ScreenToClient( htgt, &p );
   return MAKELPARAM( p.x, p.y );
}

// -------------------------------------------------------------------------

static LRESULT CALLBACK ftt_WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
   switch( uMsg )
   {
   HANDLE_MSG( hwnd, WM_CREATE,     ftt_OnCreate   );
   HANDLE_MSG( hwnd, WM_DESTROY,    ftt_OnDestroy  );
   HANDLE_MSG( hwnd, WM_PAINT,      ftt_OnPaint    );

   // Behave as if we're transparent and forward these events to our parent
   case WM_KEYDOWN      :
   case WM_KEYUP        :
   case WM_CHAR         :
   case WM_DEADCHAR     :
   case WM_SYSKEYDOWN   :
   case WM_SYSKEYUP     :
   case WM_SYSCHAR      :
   case WM_SYSDEADCHAR  :
      PostMessage( GetParent(hwnd), uMsg, wParam, lParam );
      return 0;

   case WM_MOUSEMOVE    :
      PostMessage( GetParent(hwnd), uMsg, wParam, _lpPointClientToClient( hwnd, lParam, GetParent(hwnd) ) );
      return 0;

   case WM_LBUTTONDOWN  :
   case WM_LBUTTONUP    :
   case WM_LBUTTONDBLCLK:
   case WM_RBUTTONDOWN  :
   case WM_RBUTTONUP    :
   case WM_RBUTTONDBLCLK:
   case WM_MBUTTONDOWN  :
   case WM_MBUTTONUP    :
   case WM_MBUTTONDBLCLK:
   case WM_XBUTTONDOWN  :
   case WM_XBUTTONUP    :
   case WM_XBUTTONDBLCLK:
      PostMessage( GetParent(hwnd), uMsg, wParam, _lpPointClientToClient( hwnd, lParam, GetParent(hwnd) ) );
      return 0;

   case WM_MOUSEWHEEL   :
   case WM_MOUSEHWHEEL  :
      PostMessage( GetParent(hwnd), uMsg, wParam, _lpPointClientToClient( hwnd, lParam, GetParent(hwnd) ) );
      return 0;

   }
   return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

#define EviFancyTipClass   TEXT("FancyTip")

FT_DECLSPEC HANDLE ft_CreateFancyTip( HINSTANCE hInst, HWND hOwner )
{
   static ATOM aClass = 0;
   FancyData  *pfd;
   WNDCLASS    wc;

   mm_GetProcs();

   wc.cbClsExtra     = 0;
   wc.cbWndExtra     = 0;
   wc.hbrBackground  = (HBRUSH)COLOR_INFOBK;
   wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
   wc.hIcon          = NULL;
   wc.hInstance      = hInst;
   wc.lpfnWndProc    = ftt_WindowProc;
   wc.lpszClassName  = EviFancyTipClass;
   wc.lpszMenuName   = NULL;
   wc.style          = 0;//CS_DBLCLKS;

   if( 0 == aClass )
   {
      aClass = RegisterClass(&wc);
      if( 0 == aClass )
      {
         return NULL;
      }
   }

   pfd = (FancyData *)calloc( 1, sizeof(FancyData) );
   pfd->hOwner = hOwner;
   pfd->hinst  = hInst;

   // Create the tooltip window
   pfd->hwnd = CreateWindowEx( WS_EX_NOACTIVATE,
                               EviFancyTipClass,
                               NULL,
                               WS_POPUP,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               hOwner, NULL, hInst, pfd );
   if( NULL == pfd->hwnd )
   {  // Window creation failed!
      free( pfd );
      pfd = NULL;
   }

   return (HANDLE)pfd;

}

// -------------------------------------------------------------------------

FT_DECLSPEC BOOL ft_ShowFancyTip( HANDLE hTip, int x, int y )
{
   FancyData  *pfd;
   BOOL        bResult = -1;
   RECT        rMon;

   pfd = (FancyData *)hTip;
   if( (NULL != pfd) && (GetForegroundWindow() == pfd->hOwner) )
   {
      pfd->pTip.x = x;
      pfd->pTip.y = y;

      pfd->rTip.left   = pfd->pTip.x;
      pfd->rTip.top    = pfd->pTip.y;
      pfd->rTip.right  = pfd->pTip.x + pfd->sMax.cx;
      pfd->rTip.bottom = pfd->pTip.y + pfd->sMax.cy;
      mm_GetScreenRectFromPoint( &pfd->pTip, &rMon );

      if( pfd->rTip.left < rMon.left )
      {  // Need to move right
         pfd->rTip.right -= rMon.left - pfd->rTip.left;
         pfd->rTip.left   = pfd->rTip.right - pfd->sMax.cx - 3;
      }
      if( pfd->rTip.top < rMon.top )
      {  // Need to move down
         pfd->rTip.bottom -= rMon.top - pfd->rTip.top;
         pfd->rTip.top     = pfd->rTip.bottom - pfd->sMax.cy - 1;
      }

      if( rMon.right < pfd->rTip.right )
      {  // Need to move left
         pfd->rTip.left  -= pfd->rTip.right - rMon.right;
         pfd->rTip.right  = pfd->rTip.left + pfd->sMax.cx - 3;
      }
      if( rMon.bottom < pfd->rTip.bottom )
      {  // Need to move up
         pfd->rTip.top   -= pfd->rTip.bottom - rMon.bottom;
         pfd->rTip.bottom = pfd->rTip.top + pfd->sMax.cy - 1;
      }

      pfd->bVisible = TRUE;
      SetWindowPos( pfd->hTip, HWND_TOPMOST, //pfd->hOwner,
                    pfd->rTip.left, pfd->rTip.top,
                    pfd->rTip.right - pfd->rTip.left,
                    pfd->rTip.bottom - pfd->rTip.top,
                    SWP_NOACTIVATE );

      bResult = ShowWindow( pfd->hTip, SW_SHOW );

      InvalidateRect( pfd->hTip, NULL, FALSE );
   }
   return bResult;
}

// -------------------------------------------------------------------------

FT_DECLSPEC BOOL ft_HideFancyTip( HANDLE hTip )
{
   FancyData  *pfd;
   BOOL        bResult = -1;

   pfd = (FancyData *)hTip;
   if( NULL != pfd )
   {
      pfd->bVisible = FALSE;
      SetWindowPos( pfd->hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                    SWP_NOSIZE | SWP_NOMOVE );
      SetWindowPos( pfd->hOwner, pfd->hwnd, 0, 0, 0, 0,
                    SWP_NOSIZE | SWP_NOMOVE );
      bResult = ShowWindow( pfd->hTip, SW_HIDE );
   }
   return bResult;
}

// -------------------------------------------------------------------------

static int _AdjustIconSize( int origSize, int bump )
{
   int   d;

   // Find current size in our table
   for( d = 1; d < (cIconDim-1); d++ )
   {
      if( origSize < iconDim[d] )
      {
         break;
      }
   }

   d += bump;

   return iconDim[d];
}

// -------------------------------------------------------------------------

FT_DECLSPEC int ft_SetFancyTipText( HANDLE hTip, char *sz, SIZE *ps, void *pIcon )
{
   HINSTANCE      hinst;
   FancyData     *pfd;
   FancyControl  *pfc;
   FancyControl  *pfcPrev;
   int            len = 0;
   int            t;
   int            rc;
   int            xMarg;
   int            saveMarg;
   int            tab;
   char          *cp;
   char           szTemp[1024];
   char           szCode[8];
   BOOL           bIcon;
   HANDLE         hIcon = NULL;
   SIZE           sIcon = {0};
   BOOL           bNeedSize = TRUE;

   pfd = (FancyData *)hTip;
   if( NULL != pfd )
   {
      hinst    = pfd->hinst;
      sIcon.cx = pfd->sIcon.cx;
      sIcon.cy = pfd->sIcon.cy;

      if( (NULL == pfd) || !IsWindow(pfd->hTip) || (NULL == sz) )
      {  // bad parameter or window has been destroyed
         dbg_printf( "FancyTips:  bad param in " __FUNCTION__ "\n" );
         return ERROR_INVALID_PARAMETER;
      }
      pfd->bTipReady = FALSE;
      pfd->iAtPrev   = pfd->xMarg;  // Make sensible in case of foolishness

      switch( (DWORD_PTR)pIcon )
      {
      case 0:  // i.e. NULL, so no icon
         bIcon = FALSE;
         break;

      case ftiInformation: pIcon = (void *)(DWORD_PTR)Ico_BlueInfo;     hinst = ghDLL; bNeedSize = FALSE; goto LoadIconResource;// 0x0100
      case ftiQuestion   : pIcon = (void *)(DWORD_PTR)Ico_GrayQuestion; hinst = ghDLL; bNeedSize = FALSE; goto LoadIconResource;// 0x0101
      case ftiAlertAlarm : pIcon = (void *)(DWORD_PTR)Ico_YellowAlert;  hinst = ghDLL; bNeedSize = FALSE; goto LoadIconResource;// 0x0102
      case ftiHighAlarm  : pIcon = (void *)(DWORD_PTR)Ico_RedHigh;      hinst = ghDLL; bNeedSize = FALSE; goto LoadIconResource;// 0x0103

      case IDI_APPLICATION:   // System message icon
      case IDI_HAND       :
      case IDI_QUESTION   :
      case IDI_EXCLAMATION:
      case IDI_ASTERISK   :
      case IDI_WINLOGO    :
      case IDI_SHIELD     :
         hinst = NULL;
LoadIconResource:
         hIcon = LoadIcon( hinst, MAKEINTRESOURCE((DWORD_PTR)pIcon) );
         bIcon = (NULL != hIcon);
         if( bNeedSize )
         {
            _GetIconSize( hIcon, &sIcon );
         }
         break;

      default: // Custom icon
         if( IS_INTRESOURCE((DWORD_PTR)pIcon) )
         {
            if( 32000 < (DWORD_PTR)pIcon )
            {  // This is likely a system resource request
               hIcon = LoadIcon( NULL, MAKEINTRESOURCE((DWORD_PTR)pIcon) );
            }
            else
            {
               hIcon = LoadIcon( hinst, MAKEINTRESOURCE((DWORD_PTR)pIcon) );
            }
            bIcon = (NULL != hIcon);
            if( bNeedSize )
            {
               _GetIconSize( hIcon, &sIcon );
            }
         }
         else
         {
            cp = (char *)pIcon;
            if( 0 < _FileExists( cp ) )
            {
               hIcon = (HICON)LoadImage( NULL,             // hInstance must be NULL when loading from a file
                                         cp,               // the icon file name
                                         IMAGE_ICON,       // specifies that the file is an icon
                                         pfd->sIcon.cx,    // width of the image
                                         pfd->sIcon.cy,    // height of the image
                                         LR_LOADFROMFILE|  // we want to load a file (as opposed to a resource)
                                         //LR_DEFAULTSIZE|   // default metrics based on the type (IMAGE_ICON, 32x32)
                                         LR_SHARED );      // let the system release the handle when it's no longer used
               bIcon = (NULL != hIcon);
               _GetIconSize( hIcon, &sIcon );
            }
            else
            {
               hIcon = NULL;
               bIcon = FALSE;
            }
         }
         break;
      }

      FillRect( pfd->hdc, &pfd->rBuf, pfd->hbrBg );

      xMarg = pfd->xMarg;
      if( bIcon )
      {
         DrawIconEx( pfd->hdc, xMarg, pfd->yMarg, hIcon,
                     sIcon.cx, sIcon.cy, 0,
                     NULL, DI_NORMAL);
         DeleteObject( hIcon );
         xMarg += sIcon.cx + pfd->xMarg;
      }
      saveMarg = pfd->xMarg;
      pfd->xMarg = xMarg;

      pfd->p.x      = pfd->xMarg; pfd->p.y      = pfd->yMarg;
      pfd->sMax.cx  = pfd->xMarg; pfd->sMax.cy  = pfd->yMarg;
      pfd->sLine.cx = 0;          pfd->sLine.cy = 0;

      cp = szTemp;
      while( *sz )
      {
         if( ('\\' == *sz) && (FALSE == pfd->bNoEscape) )
         {
            sz++;
            switch( *sz )
            {
            case '0' :  // Octal char code  \ooo
            case '1' :  // Octal char code  \ooo
            case '2' :  // Octal char code  \ooo
            case '3' :  // Octal char code  \ooo
            case '4' :  // Octal char code  \ooo
            case '5' :  // Octal char code  \ooo
            case '6' :  // Octal char code  \ooo
            case '7' :  // Octal char code  \ooo
               for( t = 0; (t < 3) && isodigit(*sz); t++ )
               {
                  szCode[t] = *sz;
                  sz++;
               }
               szCode[t] = '\0';
               *cp = (char)strtoul( szCode, NULL, 8 );
               cp++;
               len += 1;
               continue;

            case 'x' :  // Hex char code  \xXXX
               sz++;
               for( t = 0; (t < 3) && isxdigit(*sz); t++ )
               {
                  szCode[t] = *sz;
                  sz++;
               }
               szCode[t] = '\0';
               *cp = (char)strtoul( szCode, NULL, 16 );
               cp++;
               len += 1;
               continue;

            case 'n' :  // Newline
               goto NewLineChar;

            case 't' :  // Tab
               goto TabChar;

            default  :  // any other escaped character is itself
               goto CopyChar;
            }
         }
         switch( *sz )
         {
         case '<'  : // Start of a control token
            if( pfd->bNoEscape )
            {  // Check for the ONLY value control sequence...
               if( !_strnicmp( sz, "</ne>", 5 ) )
               {  // Found it!  Reactivate checking for escapes & controls
                  pfd->bNoEscape = FALSE;
                  sz += 5;
                  continue;
               }
            }
            else if( pfd->cfc < MaxFancyControls )
            {
               sz++;
               pfc = &pfd->fc[pfd->cfc];
               rc = _GetFancyCtrlType( pfd, pfc, &sz );
               if( rc < 0 )
               {
                  dbg_printf( "FancyTips:  unknown control type\n" );
                  return rc;
               }
               if( IsEndSymbol( pfc->eType ) )
               {  // Attempt to pop the previous symbol value off the 'stack'
                  if( 0 < pfd->cfc )
                  {
                     _EmitFancyText( pfd, szTemp, len );
                     len = 0;
                     cp = sz;
                     pfc->eType &= ~fcEnd;
                     pfcPrev = &pfd->fc[pfd->cfc-1];
                     if( pfc->eType == pfcPrev->eType )
                     {  // OK, restore the previous value of this control setting
                        switch( pfc->eType )
                        {
                        case fcNoEscape:  pfd->bNoEscape = (BOOL)pfcPrev->prev;  break;
                        case fcFgColor:   pfd->crFg   = (COLORREF)pfcPrev->prev; break;
                        case fcBgColor:   pfd->crBg   = (COLORREF)pfcPrev->prev; break;
                        case fcFont:      pfd->iFont  = (int)pfcPrev->prev;      break;
                        case fcAlign:     pfd->iAlign = (int)pfcPrev->prev;      break;
                        case fcLeftAt:
                        case fcCenterAt:
                        case fcDecimalAt:
                        case fcRightAt:   pfd->eAt = (FancyAlign)HIWORD(pfcPrev->prev);
                                          pfd->iAt = (int)LOWORD(pfcPrev->prev); break;
                        }
                        pfd->cfc -= 1;
                     }
                     else
                     {
                        switch( pfc->eType )
                        {
                        case fcNoEscape:  pfd->bNoEscape = FALSE; break;
                        case fcColumn:    break;   // Not required, but OK.
                        case fcBreak:     break;   // Not required, but OK.
                        default:
                           // Error!  Previous 'start' doesn't match this 'end'!
                           dbg_printf( "FancyTips:  Previous 'start' doesn't match this 'end'!\n" );
                           break;
                        }
                     }
                  }
                  else
                  {  // Error!  No matching 'start' for ANY symbol!
                     dbg_printf( "FancyTips:  No matching 'start' for ANY symbol!\n" );
                  }
               }
               else
               {
                  do // once
                  {
                     _EmitFancyText( pfd, szTemp, len );
                     len = 0;
                     switch( pfc->eType )
                     {
                     case fcNoEscape:  pfd->bNoEscape = TRUE; continue;
                     case fcFgColor:   pfc->prev = pfd->crFg  ; pfd->crFg   = (COLORREF)pfc->val;  break;
                     case fcBgColor:   pfc->prev = pfd->crBg  ; pfd->crBg   = (COLORREF)pfc->val;  break;
                     case fcFont:      pfc->prev = pfd->iFont ; pfd->iFont  = (int)pfc->val;       break;
                     case fcAlign:     pfc->prev = pfd->iAlign; pfd->iAlign = (int)pfc->val;       break;
                     case fcLeftAt:
                        pfc->prev = MAKEDWORD( pfd->iAt, pfd->eAt );
                        pfd->eAt  = faLeft;
                        pfd->iAt  = (int)pfc->val;
                        //pfd->iAt  = pfd->xMarg + (int)pfc->val;// * pfd->f[pfd->iFont].tm.tmAveCharWidth;
                        break;

                     case fcCenterAt:
                        pfc->prev = MAKEDWORD( pfd->iAt, pfd->eAt );
                        pfd->eAt  = faCenter;
                        pfd->iAt  = (int)pfc->val;
                        //pfd->iAt  = pfd->xMarg + (int)pfc->val;// * pfd->f[pfd->iFont].tm.tmAveCharWidth;
                        break;

                     case fcRightAt:
                        pfc->prev = MAKEDWORD( pfd->iAt, pfd->eAt );
                        pfd->eAt  = faRight;
                        pfd->iAt  = (int)pfc->val;
                        //pfd->iAt  = pfd->xMarg + (int)pfc->val;// * pfd->f[pfd->iFont].tm.tmAveCharWidth;
                        break;

                     case fcDecimalAt:
                        pfc->prev = MAKEDWORD( pfd->iAt, pfd->eAt );
                        pfd->eAt  = faDecimal;
                        pfd->iAt  = (int)pfc->val;
                        //pfd->iAt  = pfd->xMarg + (int)pfc->val;// * pfd->f[pfd->iFont].tm.tmAveCharWidth;
                        break;

                     case fcColumn:
                        pfd->p.x  = pfd->xMarg + (int)pfc->val * pfd->f[0].tm.tmAveCharWidth;
                        continue;

                     case fcBreak:
                        goto ResetLine;
                     }
                     pfd->cfc += 1;
                  }while(FALSE);
               }
               cp = szTemp;
               continue;
            }
            else
            {
               dbg_printf( "FancyTips:  Too many nested format controls!  (Limit = %d)\n", MaxFancyControls );
            }
            break;   // Escaped '<', so process like a normal character

         case '\n' : // End of line, time to emit what we have and start a new line
NewLineChar:
            sz++;
            _EmitFancyText( pfd, szTemp, len );
ResetLine:
            pfd->sMax.cy += pfd->sLine.cy;
            pfd->sMax.cx  = max( pfd->sMax.cx, pfd->p.x );
            pfd->p.x      = pfd->xMarg;
            pfd->p.y     += pfd->sLine.cy;
            pfd->sLine.cx = 0;
            pfd->sLine.cy = 0;
            cp = szTemp;
            len = 0;
            continue;

         case '\t' : // Advance the output location to the next tab stop for this font
TabChar:
            sz++;
            _EmitFancyText( pfd, szTemp, len );
            cp = szTemp;
            len = 0;
            for( t = 0; t < pfd->f[pfd->iFont].ctab; t++ )
            {
               tab = pfd->xMarg + pfd->f[pfd->iFont].tab[t];
               if( pfd->p.x < tab )
               {
                  pfd->p.x = tab;
                  break;
               }
            }
            pfd->sMax.cx = max( pfd->sMax.cx, pfd->p.x );
            continue;
         }
CopyChar:
         len += 1;
         *cp = *sz;
         cp++;
         sz++;
      }

      pfd->xMarg = saveMarg;

      if( 0 < len )
      {
         _EmitFancyText( pfd, szTemp, len );
         pfd->sMax.cy += pfd->sLine.cy;
         pfd->sMax.cx  = max( pfd->sMax.cx, pfd->p.x + pfd->xMarg );
         pfd->p.x      = pfd->xMarg;
         pfd->p.y     += pfd->sLine.cy;
         pfd->sLine.cx = 0;
         pfd->sLine.cy = 0;
      }
      else if( xMarg < pfd->p.x )
      {  // There was a 'lingering line' with no newline at the end, so account for it
         pfd->p.y    += pfd->sLine.cy;
         pfd->sMax.cx = max( pfd->sMax.cx, pfd->p.x + pfd->xMarg );
         pfd->sMax.cy = max( pfd->sMax.cy, pfd->p.y );
      }

      pfd->sMax.cx += pfd->xMarg;
      pfd->sMax.cy += pfd->yMarg;

      if( NULL != ps )
      {
         ps->cx = pfd->sMax.cx;
         ps->cy = pfd->sMax.cy;
      }
      pfd->bTipReady = TRUE;
   }
   return ('\0' == *sz ? NO_ERROR : ERROR_INVALID_DATA);
}

// -------------------------------------------------------------------------

FT_DECLSPEC BOOL ft_DestroyFancyTip( HANDLE hTip )
{
   FancyData  *pfd;
   BOOL        bResult = FALSE;

   pfd = (FancyData *)hTip;
   if( NULL != pfd )
   {
      if( IsWindow( pfd->hwnd ) )
      {
         DestroyWindow( pfd->hwnd );
      }
      bResult = TRUE;
   }
   return bResult;
}

///////////////////////////////////////////////////////////////////////////
//                        E N D   O F   F I L E
///////////////////////////////////////////////////////////////////////////
