/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Library build rules.
 *
 ******************************************************************************/

/*
 * Global variables.
 */
INCS = ;
PRIV_INCS = ;
SRCS = ;
LIBS = ;
D_LIBS = ;
A_LIBS = ;
P_LIBS = ;
S_LIBS = ;
S_LINKS = ;

/*
 * Build rules for library C files --> object files.
 */
[objdir]/lib/c/%0src/%1.o_d : [srcdir]/lib/c/%0src/%1.c
  set mkdir
{
  [CC] [D_CFLAGS] [CPPFLAGS] [D_CPPFLAGS] -c [srcdir]/lib/c/%0src/%1.c
       -o [target];

  c_incl [CPPFLAGS] --no-cache -Absent_Local_Ignore -No_System
         [srcdir]/lib/c/%0src/%1.c
         "--prefix='"[target]" : "[srcdir]/lib/c/%0src/%1.c"'"
         "--suffix='set nodefault;'"
         -o [fromto lib/c/%0src/%1.o_d lib/c/%0src/%1.d_d [target]];
}

[objdir]/lib/c/%0src/%1.o_a : [srcdir]/lib/c/%0src/%1.c
  set mkdir
{
  [CC] [A_CFLAGS] [CPPFLAGS] [A_CPPFLAGS] -c [srcdir]/lib/c/%0src/%1.c
       -o [target];

  c_incl [CPPFLAGS] --no-cache -Absent_Local_Ignore -No_System
         [srcdir]/lib/c/%0src/%1.c
         "--prefix='"[target]" : "[srcdir]/lib/c/%0src/%1.c"'"
         "--suffix='set nodefault;'"
         -o [fromto lib/c/%0src/%1.o_a lib/c/%0src/%1.d_a [target]];
}

[objdir]/lib/c/%0src/%1.o_p : [srcdir]/lib/c/%0src/%1.c
  set mkdir
{
  [CC] [P_CFLAGS] [CPPFLAGS] [P_CPPFLAGS] -c [srcdir]/lib/c/%0src/%1.c
       -o [target];

  c_incl [CPPFLAGS] --no-cache -Absent_Local_Ignore -No_System
         [srcdir]/lib/c/%0src/%1.c
         "--prefix='"[target]" : "[srcdir]/lib/c/%0src/%1.c"'"
         "--suffix='set nodefault;'"
         -o [fromto lib/c/%0src/%1.o_p lib/c/%0src/%1.d_p [target]];
}

[objdir]/lib/c/%0src/%1.o_s : [srcdir]/lib/c/%0src/%1.c
  set mkdir
{
  [CC] [S_CFLAGS] [CPPFLAGS] [S_CPPFLAGS] -c [srcdir]/lib/c/%0src/%1.c
       -o [target];

  c_incl [CPPFLAGS] --no-cache -Absent_Local_Ignore -No_System
         [srcdir]/lib/c/%0src/%1.c
         "--prefix='"[target]" : "[srcdir]/lib/c/%0src/%1.c"'"
         "--suffix='set nodefault;'"
         -o [fromto lib/c/%0src/%1.o_s lib/c/%0src/%1.d_s [target]];
}
