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
# Current revision: $Revision: 165 $
# Last modified: $Date: 1998-08-11 17:09:33 -0700 (Tue, 11 Aug 1998) $
#
##############################################################################

# Current directory.
CURRDIR := src

$(CURRDIR)_LSRCS := bhp.c br.c brblk.c brbs.c buf.c dbg.c \
	glob.c jt.c list.c locks.c log.c matrix.c mem.c oh.c res.c sock.c \
	socks.c thread.c zt.c

LIB := stash

# Include Makefile magic that uses the above lists.
include Makefile.common
