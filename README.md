# FancyTips
DLL to provide simple, formatted tool tips  
_Last updated **March 23, 2023**_

NOTE: This is my very first attempt to use git to share a
project, so please bear with me while I make mistakes.  
Probably quite a few.

This DLL is the result of my inability to get Windows' built-in
tooltip control to behave in a reliably consistent manner when I
tried to add custom draw features to my existing tips.  So I
created my own tip windows that I could control completely and
this is the result.

I'm sorry to say that I haven't yet mastered the use of UNICODE,
so this project uses only plain ASCII strings.  If anyone finds
this useful, perhaps they might add that capability... (smile)

I had two (self-imposed) primary requirements:

1. Allow much more text formatting
2. Support resizing the fonts used

### Examples

![Sample 1](https://github.com/nurbles/FancyTips/images/ftSample1.png "Sample 1")

![Sample 2](https://github.com/nurbles/FancyTips/images/ftSample2.png "Sample 2")

![Sample 3](https://github.com/nurbles/FancyTips/images/ftSample3.png "Sample 3")


### For Text Formatting
This package takes a specially encoded string and renders it into tooltip
(like) window that may be displayed on demand at a specific location.

The string encoding was inspired by html, and supports:

`<ff:name></ff>`    For use registered font named 'name'

Font names may also be used as tags without the leading `ff:`,
if desired.  Any closing font tag will be accepted to stop
using the currently selected font.

`<fg:color></fg>` To set a foreground color (`bg` for background)

`<br>` For a line break.  NOTE:  There is no terminating symbol for a line break

`<a:align>` Set alignment for text

`<l@:#>` Left Align text, starting at column #

`<c@:#>` Center Align text, around column #

`<r@:#>` Right Align text, ending at column #

`<d@:#>` Decimal Align text, with decimal in column # |

`#` may be a specific column (determined using the current font's average character width)
or a fraction (i.e. `<r@:2/5>`) to a fraction of the current total width
or `^` for the current center point (based on the current total width)
or `*` to re-use the most recently computed @ value (useful in case variable
text may increase the width)

`<ne>` No tags or escapes between this symbol and its end `</ne>`

This means that NONE of the html-like or escaped
character symbols will be interpreted (except for
the end of 'no escapes' sequence.)

To terminate an effect, use \</xx\> where `xx` is the symbol being ended.
These controls may be nested, but may not overlap.  That means that
termination symbols MUST appear in the opposite sequence from where
the symbols were started!

`<sc:#>` Set start column to column # (using DEFAULT font's tmAveCharWidth)

NOTE: `<sc:#>` does not allow a terminating symbol

Escaped character strings supported:

   `\n`     Same as binary newline character

   `\t`     Same as binary tab character

   `\ooo`   Octal character code (1-3 octal digits)

   `\xxxx`  Hex character code (`x` followed by 1-3 hex digits)

### For Resizing
I simply destroy the current FancyTip and create a new one using
the appropriate font sizes when my application is resized.
