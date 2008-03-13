/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Onyx module build rules.
 *
 ******************************************************************************/

/*
 * Global variables.
 */
MINCS = ;
MSRCS = ;
MODS = ;

/*
 * Build rules for module C files --> object files.
 */
[objdir]/mod/%0src/%1.o_s : [srcdir]/mod/%0src/%1.c
  set mkdir
{
  [CC] [S_CFLAGS] [CPPFLAGS] [CPPDEFS] -c [srcdir]/mod/%0src/%1.c
       -o [target];

  c_incl [CPPFLAGS] --no-cache -Absent_Local_Ignore -No_System
         [srcdir]/mod/%0src/%1.c
         "--prefix='"[target]" : "[srcdir]/mod/%0src/%1.c"'"
         "--suffix='set nodefault;'"
         -o [fromto mod/%0src/%1.o_s mod/%0src/%1.d_s [target]];
}
