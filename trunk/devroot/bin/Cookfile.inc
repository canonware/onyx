/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Program build rules.
 *
 ******************************************************************************/

/*
 * Global variables.
 */
BINS = ;
MAN1 = ;

/*
 * Build rules for binary C files --> object files.
 */
[objdir]/bin/%0src/%1.o_a : [srcdir]/bin/%0src/%1.c
  set mkdir
{
  [CC] [A_CFLAGS] [CPPFLAGS] -I[srcdir]/bin/%0include -I[objdir]/bin/%0include
       [CPPDEFS] -c [srcdir]/bin/%0src/%1.c -o [target];

  c_incl [CPPFLAGS] -I[srcdir]/bin/%0include -I[objdir]/bin/%0include
         --no-cache -Absent_Local_Ignore -No_System [srcdir]/bin/%0src/%1.c
         "--prefix='"[target]" : "[srcdir]/bin/%0src/%1.c"'"
         "--suffix='set nodefault;'"
         -o [fromto bin/%0src/%1.o_a bin/%0src/%1.d_a [target]];
}
