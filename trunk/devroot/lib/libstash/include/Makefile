# -*-mode:makefile-*-
#############################################################################
#
# <Copyright = jasone>
# <License>
#
#############################################################################
#
# Version: <Version>
#
# Description: Jump makefile.
#              
##############################################################################

SHELL=/bin/sh
TARGETS := all stat_dbg stat_opt shar_opt stat_prof makefile libs bins
TARGETS += stat_dbg_r stat_opt_r shar_opt_r stat_prof_r all_s all_r
TARGETS += tests check install uninstall doc clean distclean etags

$(TARGETS) :
	@$(SHELL) -c \
	"cd .. ; \
	pdir=`dirname \`pwd\`` ; \
	printf \"%s %s\n\" \"make: Entering directory\" \\\`\$$pdir\'; \
	$(MAKE) -i $@; \
	printf \"%s %s\n\" \"make: Leaving directory\" \\\`\$$pdir\'; \
	"
