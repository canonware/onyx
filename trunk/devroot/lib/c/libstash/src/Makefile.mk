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
# Current revision: $Revision: 138 $
# Last modified: $Date: 1998-07-10 13:45:20 -0700 (Fri, 10 Jul 1998) $
#
##############################################################################

# Current directory.
CURRDIR := src

$(CURRDIR)_LSRCS := bhp.c br.c brblk.c brbs.c buf.c dbg.c depg.c depgn.c \
	glob.c jt.c list.c locks.c log.c mem.c oh.c res.c sock.c thread.c zt.c

# Mutually exclusive.
LIB := stash

# Include Makefile magic that uses the above lists.
include Makefile.common
