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

TARGETS := all stat_dbg stat_opt shar_opt stat_prof makefile libs bins
TARGETS += tests check install uninstall doc clean distclean etags

$(TARGETS) :
	$(SHELL) -c 'cd ..; $(MAKE) -i $@; cd -'
