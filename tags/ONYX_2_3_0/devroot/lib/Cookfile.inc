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
A_LIBS = ;
S_LIBS = ;
S_LINKS = ;

/*
 * Build rules for library C files --> object files.
 */
[objdir]/lib/%0src/%1.o_a : [srcdir]/lib/%0src/%1.c
  set mkdir
{
  [CC] [A_CFLAGS] [CPPFLAGS] [CPPDEFS] -c [srcdir]/lib/%0src/%1.c
       -o [target];

  c_incl [CPPFLAGS] --no-cache -Absent_Local_Ignore -No_System
         [srcdir]/lib/%0src/%1.c
         "--prefix='"[target]" : "[srcdir]/lib/%0src/%1.c"'"
         "--suffix='set nodefault;'"
         -o [fromto lib/%0src/%1.o_a lib/%0src/%1.d_a [target]];
}

[objdir]/lib/%0src/%1.o_s : [srcdir]/lib/%0src/%1.c
  set mkdir
{
  [CC] [S_CFLAGS] [CPPFLAGS] [CPPDEFS] -c [srcdir]/lib/%0src/%1.c
       -o [target];

  c_incl [CPPFLAGS] --no-cache -Absent_Local_Ignore -No_System
         [srcdir]/lib/%0src/%1.c
         "--prefix='"[target]" : "[srcdir]/lib/%0src/%1.c"'"
         "--suffix='set nodefault;'"
         -o [fromto lib/%0src/%1.o_s lib/%0src/%1.d_s [target]];
}