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
D_BINS = ;
A_BINS = ;
P_BINS = ;

/*
 * Build rules for binary C files --> object files.
 */
[objdir]/bin/%0src/%1.o_d : [srcdir]/bin/%0src/%1.c
  set mkdir
{
  [CC] [D_CFLAGS] [CPPFLAGS] -I[srcdir]/bin/%0include -I[objdir]/bin/%0include
       [D_CPPFLAGS] -D_[upcase [subst "/" "" %0]]_DBG
       -c [srcdir]/bin/%0src/%1.c -o [target];

  c_incl [CPPFLAGS] -I[srcdir]/bin/%0include -I[objdir]/bin/%0include
         --no-cache -Absent_Local_Ignore -No_System [srcdir]/bin/%0src/%1.c
         "--prefix='"[target]" : "[srcdir]/bin/%0src/%1.c"'"
         "--suffix='set nodefault;'"
         -o [fromto bin/%0src/%1.o_d bin/%0src/%1.d_d [target]];
}

[objdir]/bin/%0src/%1.o_a : [srcdir]/bin/%0src/%1.c
  set mkdir
{
  [CC] [A_CFLAGS] [CPPFLAGS] -I[srcdir]/bin/%0include -I[objdir]/bin/%0include
       [A_CPPFLAGS] -c [srcdir]/bin/%0src/%1.c -o [target];

  c_incl [CPPFLAGS] -I[srcdir]/bin/%0include -I[objdir]/bin/%0include
         --no-cache -Absent_Local_Ignore -No_System [srcdir]/bin/%0src/%1.c
         "--prefix='"[target]" : "[srcdir]/bin/%0src/%1.c"'"
         "--suffix='set nodefault;'"
         -o [fromto bin/%0src/%1.o_a bin/%0src/%1.d_a [target]];
}

[objdir]/bin/%0src/%1.o_p : [srcdir]/bin/%0src/%1.c
  set mkdir
{
  [CC] [P_CFLAGS] [CPPFLAGS] -I[srcdir]/bin/%0include -I[objdir]/bin/%0include
       [P_CPPFLAGS] -c [srcdir]/bin/%0src/%1.c -o [target];

  c_incl [CPPFLAGS] -I[srcdir]/bin/%0include -I[objdir]/bin/%0include
         --no-cache -Absent_Local_Ignore -No_System [srcdir]/bin/%0src/%1.c
         "--prefix='"[target]" : "[srcdir]/bin/%0src/%1.c"'"
         "--suffix='set nodefault;'"
         -o [fromto bin/%0src/%1.o_p bin/%0src/%1.d_p [target]];
}

[objdir]/bin/%0src/%1.o_s : [srcdir]/bin/%0src/%1.c
  set mkdir
{
  [CC] [S_CFLAGS] [CPPFLAGS] -I[srcdir]/bin/%0include -I[objdir]/bin/%0include
       [S_CPPFLAGS] -c [srcdir]/bin/%0src/%1.c -o [target];

  c_incl [CPPFLAGS] -I[srcdir]/bin/%0include -I[objdir]/bin/%0include
         --no-cache -Absent_Local_Ignore -No_System [srcdir]/bin/%0src/%1.c
         "--prefix='"[target]" : "[srcdir]/bin/%0src/%1.c"'"
         "--suffix='set nodefault;'"
         -o [fromto bin/%0src/%1.o_s bin/%0src/%1.d_s [target]];
}
