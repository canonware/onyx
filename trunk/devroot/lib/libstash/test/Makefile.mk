# -*-mode:makefile-*-
#############################################################################
#
# <Copyright = "jasone">
# <License>
#
#############################################################################
#
# $Source$
# $Author: jasone $
# Current revision: $Revision: 161 $
# Last modified: $Date: 1998-08-07 13:10:54 -0700 (Fri, 07 Aug 1998) $
#
##############################################################################

# Current directory path relative to the main build directory.
CURRDIR := test

$(CURRDIR)_CTESTS := c19980411a.c c19980412a.c c19980419a.c c19980430a.c \
	c19980627a.c c19980630a.c c19980731a.c
$(CURRDIR)_BTESTS :=# List of b*.c back end C code driven by the perl scripts.
$(CURRDIR)_PTESTS := p19980701a.pl.in

# Include Makefile magic that uses the above lists.
include Makefile.common
