Edit paste works more sensibly if there's already text on the 
line being appended to...

rel 2.03 (April 1999)

ficlwin:
- File Menu: recent file list and Open now load files.
- Text ouput function is now faster through use of string 
  caching. Cache flushes at the end of each line and each
  time ficlExec returns.
- Edit/paste now behaves more reasonably for text. File/open
  loads the specified file.
- Registry entries specify dictionary and stack sizes. See
  HKEY_CURRENT_USER/Software/CodeLab/ficlwin/Settings

testmain:
- Added CLOCK ( -- u) , wrapper for the ANSI C clock() function.
  Returns the number of clock ticks elapsed since process start.
- MSEC renamed to MS (in line with the ANS)
- Added CLOCKS/SEC ( -- u) , wrapper for ANSI C CLOCKS_PER_SEC
  constant
- Changed gets() in testmain to fgets() to appease the security gods.


Data structures are now 64 bit friendly.

oo.fr: Added alloc and alloc-array methods of METACLASS to
allocate objects and arrays of objects from the heap. Free method
of OBJECT frees the storage. (requires MEMORY wordset)

Added CORE EXT word WITHIN
Added DOUBLE word DNEGATE

Added ficlSetStackSize to specify param and return stack sizes. See ficl.h

Added ficlExecXT in ficl.c/h - executes a FICL_WORD given its address.

Added Michael Gauland's ficlLongMul and ficlLongDiv and support 
routines to math64.c and .h. These routines are coded in C, and are
compiled only if PORTABLE_LONGMULDIV == 1 (default is 0).

Added definition of ficlRealloc to sysdep.c (needed for memory
allocation wordset). If your target OS supports realloc(),
you'll probably want to redefine ficlRealloc in those terms.
The default version does ficlFree followed by ficlMalloc.

[Thanks to Daniel Sobral of FreeBSD for suggesting or implementing 
the next six changes!]
- Added CATCH and THROW (EXCEPTION word set) 
- Added MEMORY allocation word set. Requires ficlRealloc
- EVALUATE respects count parameter, and also passes exceptional
  return conditions back out to the calling instance of ficlExec.
- VM_QUIT clears locals dictionary in ficlExec()
- ficlExec pushes ip and executes interpret at the right times so that
  nested calls to ficlExec behave the way you'd expect them to.
- Control word match check upgraded. Control structure mismatches
  are now errors, not warnings, since the check accepts all 
  syntactally legal constructs.

Added vmInnerLoop to vm.h. This function/macro factors the inner 
interpreter out of ficlExec so it can be used in other places. 
Function/macro behavior is conditioned on INLINE_INNER_LOOP
in sysdep.h. Default: 1 unless _DEBUG is set. In part, this is because
VC++ 5 goes apoplectic when trying to compile it as a function. See 
comments in vm.c

Bug fix in isNumber(): used to treat chars between 'Z' and 'a'
as valid in base 10... (harmless, but weird) (Ficl Finger of Fate
award to Phil Martel for this one ;-)  )

softcore.pl now removes comments, spaces at the start and
  end of lines. As a result:
  sizeof (softWords) == 7663 bytes (used to be 20000)
  and consumes 11384 bytes of dictionary when compiled
  (so it's cheaper to store as text than code, for the 
  memory-conscious)

Deleted 3Com license paste-o in this file (oops)

rel 2.02 -- 17 October 1998

Changed ficlExec so that the search order really does get reset
on an ERREXIT as advertised.

marker   ( "name" -- )
forget   ( "name" -- )
forget-wid  ( wid -- )

SOURCE-ID is now equal to the (<>0) file id when loading a file 
(Win32 only), and -1 when doing EVALUATE. This means that 
REFILL now works correctly when loading a file...
Win32 LOAD command (oops) now complies with the FILE wordset
specification of FILE-INCLUDE (REFILL returns FALSE at EOF)

ficl-wordlist   ( nBins -- wid )  
    Creates a hashed wordlist with the number of bins specified.
    Best hash performance if nBins is prime!
ficl-vocabulary   ( nBins "name" -- )
    Uses ficl-wordlist to make a vocabulary with the given name
    and number of hash bins

:NONAME (bug fix) no longer pushes control marker for colon and
    exec token in wrong order.
WORDS ignores :noname (anonymous) definitions 

dictUnsmudge no longer links anonymous definitions into the hash

HIDE   ( -- wid-was )
new wordlist called HIDDEN and word HIDE for keeping execution
factors from cluttering the default namespace any worse than it 
already is... HIDE sets HIDDEN as the compile wordlist and pushes 
it onto the search order. When finished compiling execution factors,
a call to SET-CURRENT restores the previous compile wordlist. When
finished compiling words that use the execution factors, use PREVIOUS
to restore the prior search order.

Added (my current understanding of) the Johns Hopkins local syntax
in file softwords/jhlocal.fr. This is in the default version of softcore.c
instead of the previous {{ }} local syntax. That syntax is still available
in softwords/ficllocal.fr if you want it instead. Ficl's implementation
of the Johns Hopkins local syntax:
    { a b c | d -- e f }
      ^^^^^   ^    ^^ this is a comment
      |||||   \ this local is cleared initially
      \\\\\ these come off the stack in the correct order

A, b, and c are initialized off the stack in right to left order
(c gets the top of stack). D is initialized to zero. E and f are
treated as comments. The | and -- delimiters are optional. If they
appear, they must appear once only, and in the order shown.


OOP vocabulary - no longer in the search order at startup.
No longer default compile voc at startup

oo.fr 

Revised to make more extensive use of early binding for speed.

META (constant) pushes the address of METACLASS. This word is
    not immediate. Makes it easier to deal with early binding of
    class methods.

object::init now uses metaclass::get-size explicitly rather
    than object::size.

classes.fr

Added c-ptr base class for all pointer classes. derived 
    c-cellPtr, c-bytePtr, and c-wordPtr from c-ptr. These
    classes model pointers to raw scalar types.


rel 2.01
18 sep 98 -- (local) changed so that it does not leave anything 
on the stack after it runs (previously left a marker after the 
first local, consumed it after the last local). Marker is now
a static of (local).

Added {{ -- }} local syntax with variable reordering




========================================================================
       MICROSOFT FOUNDATION CLASS LIBRARY : ficlwin
========================================================================


AppWizard has created this ficlwin application for you.  This application
not only demonstrates the basics of using the Microsoft Foundation classes
but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your ficlwin application.

ficlwin.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    CFiclwinApp application class.

ficlwin.cpp
    This is the main application source file that contains the application
    class CFiclwinApp.

ficlwin.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
	Developer Studio.

res\ficlwin.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file ficlwin.rc.

res\ficlwin.rc2
    This file contains resources that are not edited by Microsoft 
	Developer Studio.  You should place all resources not
	editable by the resource editor in this file.

ficlwin.clw
    This file contains information used by ClassWizard to edit existing
    classes or add new classes.  ClassWizard also uses this file to store
    information needed to create and edit message maps and dialog data
    maps and to create prototype member functions.

/////////////////////////////////////////////////////////////////////////////

For the main frame window:

MainFrm.h, MainFrm.cpp
    These files contain the frame class CMainFrame, which is derived from
    CFrameWnd and controls all SDI frame features.

res\Toolbar.bmp
    This bitmap file is used to create tiled images for the toolbar.
    The initial toolbar and status bar are constructed in the
    CMainFrame class.  Edit this toolbar bitmap along with the
    array in MainFrm.cpp to add more toolbar buttons.

/////////////////////////////////////////////////////////////////////////////

AppWizard creates one document type and one view:

ficlwinDoc.h, ficlwinDoc.cpp - the document
    These files contain your CFiclwinDoc class.  Edit these files to
    add your special document data and to implement file saving and loading
    (via CFiclwinDoc::Serialize).

ficlwinView.h, ficlwinView.cpp - the view of the document
    These files contain your CFiclwinView class.
    CFiclwinView objects are used to view CFiclwinDoc objects.



/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named ficlwin.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Developer Studio reads and updates this file.


/////////////////////////////////////////////////////////////////////////////

