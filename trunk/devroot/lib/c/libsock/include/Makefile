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
TARGETS += tests check install uninstall doc clean distclean etags

$(TARGETS) :
	@$(SHELL) -c \
	"cd .. ; \
	pdir=`dirname \`pwd\`` ; \
	printf \"%s %s\n\" \"make: Entering directory\" \\\`\$$pdir\'; \
	$(MAKE) -i $@; \
	printf \"%s %s\n\" \"make: Leaving directory\" \\\`\$$pdir\'; \
	cd - \
	"
