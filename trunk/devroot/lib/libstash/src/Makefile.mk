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
# Current revision: $Revision: 152 $
# Last modified: $Date: 1998-07-29 16:55:01 -0700 (Wed, 29 Jul 1998) $
#
##############################################################################

# Current directory.
CURRDIR := src

$(CURRDIR)_LSRCS := bhp.c br.c brblk.c brbs.c buf.c dbg.c \
	glob.c jt.c list.c locks.c log.c mem.c oh.c res.c sock.c thread.c zt.c

# Mutually exclusive.
LIB := stash

# Include Makefile magic that uses the above lists.
include Makefile.common
