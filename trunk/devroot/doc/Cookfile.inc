/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Documentation build rules.
 *
 ******************************************************************************/

/*
 * Global variables.
 */
TEX_SRCS = ;
FIG_SRCS = ;

T_MANUAL = [srcdir]/doc/latex/manual.tex;
D_MANUAL = [objdir]/doc/latex/manual.dvi;
P_MANUAL = [objdir]/doc/ps/manual.ps;
H_MANUAL = [objdir]/doc/html/manual.html;

/* LaTeX source files. */
{
  local tex_srcs = front.tex back.tex;
  tex_srcs = [fromto % [srcdir]/doc/latex/% [tex_srcs]];

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

doc_clean :
{
  rm -f [objdir]/doc/latex/manual.alg;
  rm -f [objdir]/doc/latex/manual.aux;
  rm -f [objdir]/doc/latex/manual.dvi;
  rm -f [objdir]/doc/latex/manual.lof;
  rm -f [objdir]/doc/latex/manual.log;
  rm -f [objdir]/doc/latex/manual.toc;
  rm -f [objdir]/doc/latex/texput.log;
}

doc_distclean :
{
  rm -f [objdir]/doc/latex/manual.tex;
  rm -f [objdir]/doc/latex/ltx2e;
}

doc_relclean :
{
  rm -rf [objdir]/doc/html;
  rm -rf [objdir]/doc/ps;
}

[D_MANUAL] : [T_MANUAL] [TEX_SRCS]
             [fromto [srcdir]/%0%.fig [objdir]/%0%.ps [FIG_SRCS]];

[P_MANUAL] : [D_MANUAL];

[H_MANUAL] : [P_MANUAL];

[objdir]/%0%.ps : [objdir]/%0%.fig
  set mkdir
{
  [FIG2DEV] -L ps [objdir]/%0%.fig [objdir]/%0%.ps;
}

[objdir]/%0%.dvi : [objdir]/%0%.tex
  set mkdir
{
  /*
   * Change directory to the object tree, since latex can't be made to use a
   * different location for output files.
   */
  cd [objdir]/%0\; [LATEX] [objdir]/%.tex;
}

[objdir]/%0ps/%.ps : [objdir]/%0latex/%.dvi
  set mkdir
{
  [DVIPS] [objdir]/%0latex/%.dvi -o [objdir]/%0ps/%.ps;
}

[objdir]/%0html/%.html : [objdir]/%0ps/%.ps
  set mkdir
{
  [LATEX2HTML] -dir [abs_objdir]/%0html -split 3 -toc_depth 4 -footnode
    -noinfo -auto_link -transparent -image_type png -show_section_numbers
    [abs_srcdir]/%0latex/%.tex;
}