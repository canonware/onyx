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

# Current directory, don't need to touch this.
CURRDIR := include

$(CURRDIR)_INCS := bhp_priv.h br_priv.h brblk_priv.h brbs_priv.h buf_priv.h \
	dbg_priv.h depg_priv.h depgn_priv.h jt_priv.h list_priv.h \
	locks_priv.h log_priv.h mem_priv.h oh_priv.h res_priv.h sock_priv.h \
	thread_priv.h
$(CURRDIR)_LINCS := common_incs.h bhp.h br.h brblk.h brbs.h buf.h dbg.h \
	depg.h depgn.h glob.h jt.h list.h locks.h log.h mem.h oh.h res.h \
	sock.h thread.h

LIB := hungry

# Include Makefile magic that uses the above lists.
include Makefile.common
