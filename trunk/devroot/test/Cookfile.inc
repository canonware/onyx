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
OTESTS = ;
PTESTS = ;
CBTESTS = ;
OBTESTS = ;

test_clean :
{
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.d_a [CTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [CTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.out [CTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.diff [CTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0% [CTESTS]];

  rm -f [fromto [srcdir]/%0%.nx.in [objdir]/%0%.nx.out [OTESTS]];
  rm -f [fromto [srcdir]/%0%.nx.in [objdir]/%0%.nx.diff [OTESTS]];
  rm -f [fromto [srcdir]/%0%.nx.in [objdir]/%0%.nx [OTESTS]];

  rm -f [fromto [srcdir]/%0%.pl.in [objdir]/%0%.out [PTESTS]];
  rm -f [fromto [srcdir]/%0%.pl.in [objdir]/%0%.pl.out [PTESTS]];
  rm -f [fromto [srcdir]/%0%.pl.in [objdir]/%0%.pl [PTESTS]];

  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.d_a [CBTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [CBTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.out [CBTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.diff [CBTESTS]];
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0% [CBTESTS]];

  rm -f [fromto [srcdir]/%0%.nx.in [objdir]/%0%.nx.out [OBTESTS]];
  rm -f [fromto [srcdir]/%0%.nx.in [objdir]/%0%.nx.diff [OBTESTS]];
  rm -f [fromto [srcdir]/%0%.nx.in [objdir]/%0%.nx [OBTESTS]];
}

test_distclean :
{
  rm -f [fromto [srcdir]/%0%.c [objdir]/%0%.perf [CTESTS]];
  rm -f [fromto [srcdir]/%0%.nx.in [objdir]/%0%.nx.perf [OTESTS]];
  rm -f [fromto [srcdir]/%0%.pl.in [objdir]/%0%.pl.perf [PTESTS]];
}

/*
 * Build rules for tests.
 */
[objdir]/%0test/%1 : [objdir]/%0test/%1.o_a [S_LIBS]
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [objdir]/%0test/%1.o_a
#if [matches elf [abi]]
       [split " "
              [prepost [prepost [RPATH] "/lib/c/" [abs_objdir]]
                       "/lib"
                       [libs]]]
#endif
       [LDFLAGS] [LIBS] [EXTRALIBS];
}

[objdir]/%0test/%1.o_a : [srcdir]/%0test/%1.c
  set mkdir
{
  [CC] [A_CFLAGS] [CPPFLAGS] [CPPDEFS] -D_CW_ASSERT
    -c [srcdir]/%0test/%1.c -o [target];

  c_incl [CPPFLAGS] --no-cache -Absent_Local_Ignore -No_System
         [srcdir]/%0test/%1.c
         "--prefix='"[target]" : "[srcdir]/%0test/%1.c"'"
         "--suffix='set nodefault;'"
         -o [fromto %0test/%1.o_a %0test/%1.d_a [target]];
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
 * Add magic to onyx files.
 */
[objdir]/%0test/%.nx : [srcdir]/%0test/%.nx.in
  set mkdir
{
  [SHELL] -e;
data
#![SHELL]

echo \"[srcdir]/%0test/%.nx.in --> [target]\"

echo \"#![abs_objdir]/[TONYX]\" > [target]
echo \"!#\" >> [target]
[CAT] [srcdir]/%0test/%.nx.in >> [target];
chmod u+x [target];
dataend
}
