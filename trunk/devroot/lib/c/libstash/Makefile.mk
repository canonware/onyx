# -*-mode:makefile-*-
# @configure_input@
#############################################################################
#
# <Copyright = "jasone">
# <License>
#
#############################################################################
#
# $Source$
# $Author: jasone $
# Current revision: $Revision: 131 $
# Last modified: $Date: 1998-07-07 13:43:45 -0700 (Tue, 07 Jul 1998) $
#
# Description: Makefile stub for a subdirectory.
#              
##############################################################################

# Current directory path relative to the main build directory.
CURRDIR :=

$(CURRDIR)_INCS :=# List of .h headers that don't need installed.
$(CURRDIR)_LINCS :=# List of .h headers that do need installed.
$(CURRDIR)_LSRCS :=# List of .c library sources.

$(CURRDIR)_CTESTS :=# List of c*.c stand-alone C tests.
$(CURRDIR)_BTESTS :=# List of b*.c back end C code driven by the perl scripts.
$(CURRDIR)_PTESTS :=# List of .pl.in test sources that drive the b*.c programs.

# Mutually exclusive.
$(CURRDIR)_EXECS :=# List of .c sources that define main().
$LIB :=# Name of library to integrate sources into.

# Include Makefile magic that uses the above lists.
include Makefile.common
