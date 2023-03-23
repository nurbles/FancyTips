// ------------------------------------------------------------------------
//                         T O P   O F   F I L E
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
// -----------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_DLL)
#define FT_DECLSPEC __declspec(dllexport)
#else
#define FT_DECLSPEC __declspec(dllimport)
#endif

#if !defined(MAKEDWORD)
#define MAKEDWORD(L,H)((DWORD)(((WORD)(((DWORD_PTR)(L)) & 0xffff)) |((DWORD)((WORD)(((DWORD_PTR)(H)) & 0xffff))) << 16))
#endif

// Fancy Tip Icons
#define ftiInformation  ((void *)0x0100)   // FancyTip Information Icon
#define ftiQuestion     ((void *)0x0101)   // FancyTip Question Icon
#define ftiAlertAlarm   ((void *)0x0102)   // FancyTip AlertAlarm Icon
#define ftiHighAlarm    ((void *)0x0103)   // FancyTip HighAlarm Icon

// Font selection for auto-icon sizing
// (used for psIcon parameter to ft_SetFancyTipIconSize)
#define ftFont0  ((SIZE *)1)  // Use Font[0] to auto-select icon size
#define ftFont1  ((SIZE *)2)  // Use Font[1] to auto-select icon size
#define ftFont2  ((SIZE *)3)  // Use Font[2] to auto-select icon size
#define ftFont3  ((SIZE *)4)  // Use Font[3] to auto-select icon size
#define ftFont4  ((SIZE *)5)  // Use Font[4] to auto-select icon size
#define ftFont5  ((SIZE *)6)  // Use Font[5] to auto-select icon size
#define ftFont6  ((SIZE *)7)  // Use Font[6] to auto-select icon size
#define ftFont7  ((SIZE *)8)  // Use Font[7] to auto-select icon size
#define ftFont8  ((SIZE *)9)  // Use Font[8] to auto-select icon size
#define ftFont9  ((SIZE *)10) // Use Font[9] to auto-select icon size
#define ftFont10 ((SIZE *)11) // Use Font[10] to auto-select icon size
#define ftFont11 ((SIZE *)12) // Use Font[11] to auto-select icon size
#define ftFont12 ((SIZE *)13) // Use Font[12] to auto-select icon size
#define ftFont13 ((SIZE *)14) // Use Font[13] to auto-select icon size
#define ftFont14 ((SIZE *)15) // Use Font[14] to auto-select icon size
#define ftFont15 ((SIZE *)16) // Use Font[15] to auto-select icon size

/// @brief Use this macro for the cTabs parameter to ft_SetFancyTipTabs()
/// when setting a new tab step for all tabs (for the selected font)
#define ftTabStep(x) (-(x))

/// @brief Get the size of the given icon, in pixels
/// @param hico  Handle to a loaded icon (or cursor)
/// @param psiz  Address of SIZE struct to receive the dimensions
FT_DECLSPEC void   ft_GetIconSize        ( __in HICON hico, __out SIZE *psiz );

/// @brief Set the tab stops for a specific font
/// @param hTip   Handle to the FancyTip to be changed
/// @param xFont  Index of the font whose tabs are being set
/// @param cTabs  If >0, number of entries in the 'tabs' array; If 0, reset all
/// of the font's tabs to the default; If <0, use abs(cTabs) as the step value
/// for the font's tabs.
/// @param tabs   If cTabs >0, address of an array of cTabs values for new tab
/// stops; otherwise NULL (and ignored)
/// @return >0 for Success, <0 for Failure due to an invalid parameter
FT_DECLSPEC int    ft_SetFancyTipTabs    ( HANDLE hTip, int xFont, int cTabs, int *tabs );

/// @brief Add a font to the FancyTip (the first font added replaces the system font as the default)
/// @param hTip    Handle to the FancyTip to be changed
/// @param szName  Name of the font (used in <ff:font> tags)
/// @param plf     Address of LOGFONT used to create the font
/// @return >=0 for the index of the newly added font; <0 for failure
FT_DECLSPEC int    ft_AddFancyTipFont    ( HANDLE hTip, char *szName, LOGFONT *plf );

/// @brief Set the default foreground/background colors for the FancyTip
/// @param hTip   Handle to the FancyTip to be changed
/// @param pcrfg  Pointer to COLORREF containing new foreground color or NULL to reset to the system default
/// @param pcrbg  Pointer to COLORREF containing new background color or NULL to reset to the system default
/// @return 0 for success; <0 for failure
FT_DECLSPEC int    ft_SetFancyTipColors  ( HANDLE hTip, COLORREF *pcrfg, COLORREF *pcrbg );

/// @brief Set the margins around the (optional) icon and text in a FancyTip
/// @param hTip   Handle to the FancyTip to be changed
/// @param xMarg  >=0, new X (horizontal) margin; <0, reset to default margin
/// @param yMarg  >=0, new Y (vertical) margin; <0, reset to default margin
/// @return 0 for success; <0 for failure
FT_DECLSPEC int    ft_SetFancyTipMargin  ( HANDLE hTip, int xMarg, int yMarg );

/// @brief Choose a specific size for an icon
/// @param hTip    Handle to the FancyTip to be changed
/// @param psIcon  NULL to reset to system default icon size;
/// ftFontX (where X is font index, 0..n) to have the size automatically set to the best fit
/// to two rows of the font's text; or a pointer to a SIZE containing the new desired size
/// @return 0 for success; <0 for failure
FT_DECLSPEC int    ft_SetFancyTipIconSize( HANDLE hTip, SIZE *psIcon );

/// @brief Create a new FancyTip
/// @param hInst   Handle to the owner program instance (for loading icons)
/// @param hOwner  Handle of the FancyTip's owner window
/// @return Handle to the newly created FancyTip; NULL for failure
FT_DECLSPEC HANDLE ft_CreateFancyTip     ( HINSTANCE hInst, HWND hOwner );

/// @brief Display the current fancy tip at a specific location
/// @param hTip  Handle to the FancyTip to be displayed
/// @param x     X coordinate of the tip's top,left corner
/// @param y     Y coordinate of the tip's top,left corner
/// @return -1 for invalid parameter; otherwise success
FT_DECLSPEC BOOL   ft_ShowFancyTip       ( HANDLE hTip, int x, int y );

/// @brief Remove a visible FancyTip from the screen
/// @param hTip  Handle to the FancyTip to be hidden
/// @return -1 for invalid parameter; otherwise success
FT_DECLSPEC BOOL   ft_HideFancyTip       ( HANDLE hTip );

/// @brief Apply new content for the tip (this completely replaces any previous content)
/// @param hTip   Handle to the FancyTip to be changed
/// @param sz     Address of string buffer containing new content
/// @param ps     Optional pointer to the size of the new tip rectangle (NULL to ignore)
/// @param pIcon  Optional pointer to name of icon file or system icon symbol or fti symbol
/// @return NO_ERROR for success or system error code for failure
FT_DECLSPEC int    ft_SetFancyTipText    ( HANDLE hTip, char *sz, SIZE *ps, void *pIcon );

/// @brief Destroy a FancyTip when it is no longer needed
/// @param hTip  Handle to the FancyTip to be destroyed
/// @return TRUE if the handle referenced a tip to be destroyed; FALSE otherwise
FT_DECLSPEC BOOL   ft_DestroyFancyTip    ( HANDLE hTip );

#ifdef __cplusplus
}  // end "C" externals
#endif

// ------------------------------------------------------------------------
//                         E N D   O F   F I L E
// ------------------------------------------------------------------------
