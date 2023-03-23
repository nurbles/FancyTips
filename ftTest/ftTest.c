// ------------------------------------------------------------------------
// T O P   O F   F I L E
// ------------------------------------------------------------------------
//
// Test program for FancyTips
//
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

#include "ftTest.h"
#include <windowsx.h>

#include "FancyTips.h"

// ------------------------------------------------------------------------

#if defined(_DEBUG)
#pragma comment( lib, "FancyTipsd.lib" )
#else
#pragma comment( lib, "FancyTips.lib" )
#endif

// ------------------------------------------------------------------------
#define fttToolVisibleTicks 30000
#define fttTimerId           1234
#define fttTimerRate          999

#define TipTextLimit    4096  // This limit is for the test program, the library doesn't care
// ------------------------------------------------------------------------

static struct MyGlobal_s
{
   HWND        hwnd;
   HINSTANCE   hinst;
   HANDLE      hTip;
   BOOL        bTrackingHover;
   DWORD       dwToolTick;
   char        szTipText[TipTextLimit];
} g;

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

static BOOL edit_OnInitDialog( HWND hwnd, HWND hwndFocus, LPARAM lParam )
{
   HWND  hwndParent;
   RECT  rChild, rParent, rScreen;
   int   wChild, hChild, wParent, hParent;
   int   xNew, yNew;
   int   xPercent = 50;
   int   yPercent = 50;

   hwndParent = GetParent( hwnd );
   // Get the Height and Width of the child window
   GetWindowRect( hwnd, &rChild );
   wChild = rChild.right - rChild.left;
   hChild = rChild.bottom - rChild.top;

   // Get the Height and Width of the parent window
   if( !IsWindow(hwndParent) || IsIconic( hwndParent ) )
   {
      GetWindowRect( GetDesktopWindow(), &rParent );
   }
   else
   {
      GetWindowRect( hwndParent, &rParent );
   }
   wParent = rParent.right - rParent.left;
   hParent = rParent.bottom - rParent.top;

   // Get the display limits
   rScreen.left   = 0;
   rScreen.top    = 0;
   rScreen.right  = GetSystemMetrics( SM_CXSCREEN );
   rScreen.bottom = GetSystemMetrics( SM_CYSCREEN );

   // Calculate new X position, then adjust for screen
   xNew = rParent.left + (((wParent * xPercent) - (wChild * 50)) / 100);
   if( xNew < rScreen.left )
   {
      xNew = rScreen.left;
   }
   else if( rScreen.right < (xNew+wChild) )
   {
      xNew = rScreen.right - wChild;
   }

   // Calculate new Y position, then adjust for screen
   yNew = rParent.top + (((hParent * yPercent) - (hChild * 50)) / 100);
   if( yNew < rScreen.top )
   {
      yNew = rScreen.top;
   }
   else if( rScreen.bottom < (yNew+hChild))
   {
      yNew = rScreen.bottom - hChild;
   }

   SetWindowPos( hwnd, NULL,
                 xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

   SetDlgItemText( hwnd, Tbx_TipText, g.szTipText );
   return TRUE;
}

// -------------------------------------------------------------------------

static void edit_OnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify )
{
   char  szTemp[TipTextLimit];

   switch( id )
   {
   case IDOK :
      if( GetDlgItemText( hwnd, Tbx_TipText, szTemp, sizeof(szTemp) ) )
      {
         strcpy_s( g.szTipText, sizeof(g.szTipText), szTemp );
         EndDialog( hwnd, IDOK );
      }
      break;

   case IDCANCEL:
      EndDialog( hwnd, IDCANCEL );
      break;
   }
   return;
}

// -------------------------------------------------------------------------

static LRESULT CALLBACK edit_DlgProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
   switch( uMsg )
   {
   HANDLE_MSG( hwnd, WM_INITDIALOG, edit_OnInitDialog );
   HANDLE_MSG( hwnd, WM_COMMAND,    edit_OnCommand    );
   }
   return FALSE;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

static void test_TrackMouseHover( BOOL bTrack )
{
   TRACKMOUSEEVENT   tme = { .cbSize = sizeof(tme) };

   tme.dwFlags     = TME_HOVER;// | TME_LEAVE;
   if( !bTrack )
   {
      tme.dwFlags |= TME_CANCEL;
   }
   tme.hwndTrack   = g.hwnd;
   tme.dwHoverTime = HOVER_DEFAULT;
   g.bTrackingHover = bTrack;
   TrackMouseEvent( &tme );
   return;
}

// -------------------------------------------------------------------------

static void test_HideToolTip( void )
{
   ft_HideFancyTip( g.hTip );
   g.dwToolTick = 0;
   return;
}

// -------------------------------------------------------------------------

static void test_ShowToolTip( HWND hwnd, int x, int y )
{
   POINT p = { .x = x, .y = y };

   ClientToScreen( hwnd, &p );

   if( ERROR_SUCCESS == ft_ShowFancyTip( g.hTip, p.x, p.y ) )
   {
      g.dwToolTick = GetTickCount();
      if( 0 == g.dwToolTick )
      {  // Zero is used to indicate to tip is active, so fudge past it.
         g.dwToolTick = 1;
      }
   }
   return;
}

// -------------------------------------------------------------------------

static void test_OnMouseMove( HWND hwnd, int x, int y, UINT keyFlags )
{
   static int _x = -1;  // To ensure mouse moves are actual MOVES
   static int _y = -1;

   if( (_x == x) && (_y == y) )
   {  // The mouse didn't actually move so we're done here
      return;
   }
   _x = x;
   _y = y;
   if( 0 != g.dwToolTick )
   {
      test_HideToolTip();
   }
   test_TrackMouseHover( TRUE );
   FORWARD_WM_MOUSEMOVE( hwnd, x, y, keyFlags, DefWindowProc );
   return;
}

// ------------------------------------------------------------------------

static void test_OnMouseHover( HWND hwnd, UINT state, int x, int y )
{
   test_TrackMouseHover( FALSE );
   test_ShowToolTip( hwnd, x, y );
   return;
}

// ------------------------------------------------------------------------

static BOOL test_OnCreate( HWND hwnd, LPCREATESTRUCT lpCreateStruct )
{
   LOGFONT  lf = {0};
   HDC      hdc;
   SIZE     sLogPix;

   UNREFERENCED_PARAMETER(hwnd);
   UNREFERENCED_PARAMETER(lpCreateStruct);

   hdc = GetDC( hwnd );
   {
      sLogPix.cx = GetDeviceCaps( hdc, LOGPIXELSX );
      sLogPix.cy = GetDeviceCaps( hdc, LOGPIXELSY );
   }
   ReleaseDC( hwnd, hdc );

   g.hTip = ft_CreateFancyTip( g.hinst, hwnd );

   lf.lfHeight = -MulDiv( 16, sLogPix.cy, 72 );
   lf.lfWeight = FW_MEDIUM;
   strcpy_s( lf.lfFaceName, sizeof(lf.lfFaceName), "Segoe UI" );
   ft_AddFancyTipFont( g.hTip, "Body", &lf );

   lf.lfHeight = -MulDiv( 16, sLogPix.cy, 72 );
   lf.lfWeight = FW_BOLD;
   strcpy_s( lf.lfFaceName, sizeof(lf.lfFaceName), "Segoe UI" );
   ft_AddFancyTipFont( g.hTip, "Title", &lf );

   lf.lfHeight = -MulDiv( 20, sLogPix.cy, 72 );
   lf.lfWeight = FW_NORMAL;
   strcpy_s( lf.lfFaceName, sizeof(lf.lfFaceName), "Courier New" );
   ft_AddFancyTipFont( g.hTip, "Value", &lf );

   ft_SetFancyTipMargin( g.hTip, 12, 12 );
   sprintf_s( g.szTipText, sizeof(g.szTipText),
              "<title>This line uses the 'title' font</ff>\n"
              "<a:right>Here's the 'body' font</a>\n"
              "<r@:2/5>A Value = </r@><value><fg:#fff><bg:#00A> 1.23E+02 </bg></fg></value>" );
   ft_SetFancyTipText( g.hTip, g.szTipText, NULL, ftiInformation );
   SetTimer( hwnd, fttTimerId, fttTimerRate, NULL );
   return TRUE;
}

// ------------------------------------------------------------------------

static void test_OnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify )
{
   UNREFERENCED_PARAMETER(hwnd);
   UNREFERENCED_PARAMETER(hwndCtl);
   UNREFERENCED_PARAMETER(codeNotify);

   switch( id )
   {
   case IDM_EXIT:
      DestroyWindow( hwnd );
      break;

   case IDM_ABOUT:
      MessageBox( hwnd,
                  "Test program for the FancyTips DLL",
                  "About FancyTip Test Driver",
                  MB_OK|MB_ICONINFORMATION );
      break;

   case ID_TIPTEXT:
      if( IDOK == DialogBox( g.hinst, MAKEINTRESOURCE(Dlg_MsgEdit), hwnd, edit_DlgProc ) )
      {  // Update the TipText...
         ft_SetFancyTipText( g.hTip, g.szTipText, NULL, ftiInformation );
      }
      break;

   default:
      break;
   }
   return;
}

// ------------------------------------------------------------------------

static void test_OnDestroy( HWND hwnd )
{
   UNREFERENCED_PARAMETER(hwnd);

   KillTimer( hwnd, fttTimerId );
   ft_DestroyFancyTip( g.hTip );
   PostQuitMessage(0);
   return;
}

// ------------------------------------------------------------------------

static void test_OnTimer( HWND hwnd, UINT timerId )
{
   DWORD dwTickNow = GetTickCount();

   if( 1 < g.dwToolTick )
   {
      if( fttToolVisibleTicks <= (dwTickNow - g.dwToolTick) )
      {
         test_HideToolTip();
      }
   }
   return;
}

// ------------------------------------------------------------------------

static LRESULT CALLBACK test_WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
   switch( uMsg )
   {
   HANDLE_MSG( hwnd, WM_MOUSEMOVE,  test_OnMouseMove  );
   HANDLE_MSG( hwnd, WM_MOUSEHOVER, test_OnMouseHover );
   HANDLE_MSG( hwnd, WM_CREATE,     test_OnCreate     );
   HANDLE_MSG( hwnd, WM_COMMAND,    test_OnCommand    );
   HANDLE_MSG( hwnd, WM_DESTROY,    test_OnDestroy    );
   HANDLE_MSG( hwnd, WM_TIMER,      test_OnTimer      );
   }
   return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

// ------------------------------------------------------------------------

static int InitWindow( HINSTANCE hinst, int nShowCmd )
{
   const char *szWndClass = "FancyTipTestWClass";
   WNDCLASS    wc;
   BOOL        bUseDefault = FALSE;

   g.hinst = hinst;

   wc.cbClsExtra     = 0;
   wc.cbWndExtra     = 0;
   wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
   wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
   wc.hIcon          = LoadIcon( hinst, MAKEINTRESOURCE(IDI_FTTEST) );
   wc.hInstance      = hinst;
   wc.lpfnWndProc    = test_WindowProc;
   wc.lpszClassName  = szWndClass;
   wc.lpszMenuName   = MAKEINTRESOURCE(IDC_FTTEST);
   wc.style          = 0;//CS_DBLCLKS;

   if( !RegisterClass(&wc) )
   {
      return 1;
   }

   g.hwnd = CreateWindowEx( WS_EX_OVERLAPPEDWINDOW,
                            szWndClass,
                            "FancyTip Test Driver",
                            WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            NULL,
                            NULL,
                            hinst,
                            NULL );
   if( NULL == g.hwnd )
   {
      return 2;
   }

   UpdateWindow( g.hwnd );
   ShowWindow( g.hwnd, nShowCmd );
   return NO_ERROR;
}

// ------------------------------------------------------------------------

int WINAPI WinMain( _In_     HINSTANCE hinst,
                    _In_opt_ HINSTANCE hstupid,
                    _In_     LPSTR     lpCmdLine,
                    _In_     int       nShowCmd )
{
   MSG   msg;
   int   rc;

   UNREFERENCED_PARAMETER(hstupid);
   UNREFERENCED_PARAMETER(lpCmdLine);
   if( 0 != (rc = InitWindow( hinst, nShowCmd )) )
   {
      return rc;
   }

   while( GetMessage( &msg, NULL, 0, 0 ) )
   {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
   }

   return (int)msg.wParam;
}

// ------------------------------------------------------------------------
// E N D   O F   F I L E
// ------------------------------------------------------------------------
