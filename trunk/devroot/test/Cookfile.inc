/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Test build rules.
 *
 ******************************************************************************/

/*
 * Global variables.
 */
CTESTS = ;
STESTS = ;
PTESTS = ;
CBTESTS = ;
SBTESTS = ;

test_clean :
{
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.d_d [CTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [CTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.out [CTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.diff [CTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.perf [CTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0% [CTESTS]];

  rm -f [fromto [srcdir]/%0%.st.in [objdir]/%0%.st.out [STESTS]];
  rm -f [fromto [srcdir]/%0%.st.in [objdir]/%0%.st.diff [STESTS]];
  rm -f [fromto [srcdir]/%0%.st.in [objdir]/%0%.st.perf [STESTS]];
  rm -f [fromto [srcdir]/%0%.st.in [objdir]/%0%.st [STESTS]];

  rm -f [fromto [srcdir]/%0%.pl.in [objdir]/%0%.out [PTESTS]];
  rm -f [fromto [srcdir]/%0%.pl.in [objdir]/%0%.pl.out [PTESTS]];
  rm -f [fromto [srcdir]/%0%.pl.in [objdir]/%0%.pl.perf [PTESTS]];
  rm -f [fromto [srcdir]/%0%.pl.in [objdir]/%0%.pl [PTESTS]];

  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.d_d [CBTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [CBTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.out [CBTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.diff [CBTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0% [CBTESTS]];

  rm -f [fromto [srcdir]/%0%.st.in [objdir]/%0%.st.out [SBTESTS]];
  rm -f [fromto [srcdir]/%0%.st.in [objdir]/%0%.st.diff [SBTESTS]];
  rm -f [fromto [srcdir]/%0%.st.in [objdir]/%0%.st [SBTESTS]];
}

test_distclean :
{
}

/*
 * Build rules for tests.
 */
[objdir]/%0test/%1 : [objdir]/%0test/%1.o_d [D_LIBS]
  set mkdir
{
  [CC] -o [target] [objdir]/%0test/%1.o_d [D_LIBS] [LDFLAGS] [EXTRALIBS];
}

[objdir]/%0test/%1.o_d : [srcdir]/%0test/%1.c
  set mkdir
{
  [CC] [D_CFLAGS] [CPPFLAGS] [D_CPPFLAGS] -c [srcdir]/%0test/%1.c -o [target];

  c_incl [CPPFLAGS] --no-cache -Absent_Local_Ignore -No_System
         [srcdir]/%0test/%1.c
         "--prefix='"[target]" : "[srcdir]/%0test/%1.c"'"
         "--suffix='set nodefault;'"
         -o [fromto %0test/%1.o_d %0test/%1.d_d [target]];
}

/*
 * Add magic to perl files.
 */
[objdir]/%0%.pl : [srcdir]/%0%.pl.in
  set mkdir
{
  [SHELL] -e;
data
#![SHELL]

echo \"[srcdir]/%0%.pl.in --> [target]\"

echo \"#![PERL] -w\" > [target]
[CAT] [srcdir]/%0%.pl.in >> [target];
chmod u+x [target];
dataend
}

/*
 * Add magic to stil files.
 */
[objdir]/%0%.st : [srcdir]/%0%.st.in [STIL_D]
  set mkdir
{
  [SHELL] -e;
data
#![SHELL]

echo \"[srcdir]/%0%.st.in --> [target]\"

echo \"#![abs_objdir]/[STIL_D]\" > [target]
echo \"!#\" >> [target]
[CAT] [srcdir]/%0%.st.in >> [target];
chmod u+x [target];
dataend
}
