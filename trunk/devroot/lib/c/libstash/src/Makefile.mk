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
# Current revision: $Revision: 131 $
# Last modified: $Date: 1998-07-07 13:43:45 -0700 (Tue, 07 Jul 1998) $
#
##############################################################################

# Current directory.
CURRDIR := src

$(CURRDIR)_LSRCS := bhp.c br.c brblk.c brbs.c buf.c dbg.c depg.c depgn.c \
	glob.c jt.c list.c locks.c log.c mem.c oh.c res.c sock.c thread.c zt.c

# Mutually exclusive.
LIB := hungry

# Include Makefile magic that uses the above lists.
include Makefile.common
