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
# Current revision: $Revision: 158 $
# Last modified: $Date: 1998-07-29 17:57:57 -0700 (Wed, 29 Jul 1998) $
#
##############################################################################

# Current directory, don't need to touch this.
CURRDIR := include

$(CURRDIR)_INCS := bhp_priv.h br_priv.h  brbs_priv.h \
	dbg_priv.h jt_priv.h list_priv.h \
	locks_priv.h log_priv.h mem_priv.h oh_priv.h res_priv.h
$(CURRDIR)_LINCS := stash_incs.h bhp.h br.h brblk.h brbs.h buf.h dbg.h \
	glob.h jt.h list.h locks.h log.h mem.h oh.h res.h \
	sock.h socks.h thread.h

LIB := stash

# Include Makefile magic that uses the above lists.
include Makefile.common
