/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Documentation build rules.  The install targets do not depend on the
 * manual build targets, so that the manual won't be automatically rebuilt.
 *
 ******************************************************************************/

/*
 * Global variables.
 */
TEX_SRCS = ;
FIG_SRCS = ;

T_MANUAL = [objdir]/doc/latex/manual.tex;
D_MANUAL = [objdir]/doc/latex/manual.dvi;
P_MANUAL = [objdir]/doc/latex/manual.pdf;
PS_MANUAL = [objdir]/doc/ps/manual.ps;
PDF_MANUAL = [objdir]/doc/pdf/manual.pdf;
HTML_MANUAL = [objdir]/doc/html/manual.html;

/* LaTeX source files. */
{
  local tex_srcs = front.tex back.tex onyx.tex;
  tex_srcs = [fromto % [srcdir]/doc/latex/% [tex_srcs]];

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

install_doc : doc_install_ps doc_install_pdf doc_install_html;
doc_install_ps : /* [PS_MANUAL] */
{
  [INSTALL] -d [PREFIX]/share/canonware/doc/ps;
  [INSTALL] -m 0444 [PS_MANUAL] [PREFIX]/share/canonware/doc/ps;
}

doc_install_pdf : /* [PDF_MANUAL] */
{
  [INSTALL] -d [PREFIX]/share/canonware/doc/pdf;
  [INSTALL] -m 0444 [PDF_MANUAL] [PREFIX]/share/canonware/doc/pdf;
}

doc_install_html : /* [HTML_MANUAL] */
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/share/canonware/doc/html
[INSTALL] -d [PREFIX]/share/canonware/doc/html

for i in `find [objdir]/doc/html -type f`; do
  echo [INSTALL] -m 0444 $i [PREFIX]/share/canonware/doc/html
  [INSTALL] -m 0444 $i [PREFIX]/share/canonware/doc/html
done
dataend
}

uninstall_doc :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -rf [PREFIX]/share/canonware/doc
rm -rf [PREFIX]/share/canonware/doc
dataend
}

doc_clean :
{
  rm -f [objdir]/doc/latex/manual.alg;
  rm -f [objdir]/doc/latex/manual.aux;
  rm -f [objdir]/doc/latex/manual.aux-prev;
  rm -f [objdir]/doc/latex/manual.dvi;
  rm -f [objdir]/doc/latex/manual.idx;
  rm -f [objdir]/doc/latex/manual.idx-prev;
  rm -f [objdir]/doc/latex/manual.ilg;
  rm -f [objdir]/doc/latex/manual.ind;
  rm -f [objdir]/doc/latex/manual.lof;
  rm -f [objdir]/doc/latex/manual.log;
  rm -f [objdir]/doc/latex/manual.out;
  rm -f [objdir]/doc/latex/manual.pdf;
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
  rm -rf [objdir]/doc/pdf;
  rm -rf [objdir]/doc/ps;
}

[D_MANUAL] : [T_MANUAL] [TEX_SRCS]
             [fromto [srcdir]/%0%.fig [objdir]/%0%.ps [FIG_SRCS]];

[P_MANUAL] : [T_MANUAL] [TEX_SRCS]
             [fromto [srcdir]/%0%.fig [objdir]/%0%.ps [FIG_SRCS]];

[PS_MANUAL] : [D_MANUAL];

[PDF_MANUAL] : [P_MANUAL];

[HTML_MANUAL] : [D_MANUAL];

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
   *
   * Clean up the temp files to keep latex from stumbling over them, in case
   * the last run was building a pdf.
   */

  rm -f [objdir]/doc/latex/manual.alg;
  rm -f [objdir]/doc/latex/manual.aux;
  rm -f [objdir]/doc/latex/manual.aux-prev;
  rm -f [objdir]/doc/latex/manual.idx;
  rm -f [objdir]/doc/latex/manual.idx-prev;
  rm -f [objdir]/doc/latex/manual.ilg;
  rm -f [objdir]/doc/latex/manual.ind;
  rm -f [objdir]/doc/latex/manual.lof;
  rm -f [objdir]/doc/latex/manual.log;
  rm -f [objdir]/doc/latex/manual.out;
  rm -f [objdir]/doc/latex/manual.toc;
  rm -f [objdir]/doc/latex/texput.log;

  cd [objdir]/%0\; ./ltx2e [LATEX] [objdir]/%.tex;
  cd [objdir]/%0\; [MAKEINDEX] -c -l [objdir]/%.idx;
  cd [objdir]/%0\; ./ltx2e [LATEX] [objdir]/%.tex;
}

[objdir]/%0%.pdf : [objdir]/%0%.tex
  set mkdir
{
  /*
   * Change directory to the object tree, since latex can't be made to use a
   * different location for output files.
   *
   * Clean up the temp files to keep latex from stumbling over them, in case
   * the last run was building a dvi.
   */

  rm -f [objdir]/doc/latex/manual.alg;
  rm -f [objdir]/doc/latex/manual.aux;
  rm -f [objdir]/doc/latex/manual.aux-prev;
  rm -f [objdir]/doc/latex/manual.idx;
  rm -f [objdir]/doc/latex/manual.idx-prev;
  rm -f [objdir]/doc/latex/manual.ilg;
  rm -f [objdir]/doc/latex/manual.ind;
  rm -f [objdir]/doc/latex/manual.lof;
  rm -f [objdir]/doc/latex/manual.log;
  rm -f [objdir]/doc/latex/manual.out;
  rm -f [objdir]/doc/latex/manual.toc;
  rm -f [objdir]/doc/latex/texput.log;

  cd [objdir]/%0\; ./ltx2e [PDFLATEX] [objdir]/%.tex;
  cd [objdir]/%0\; [MAKEINDEX] -c -l [objdir]/%.idx;
  cd [objdir]/%0\; ./ltx2e [PDFLATEX] [objdir]/%.tex;
}

[objdir]/%0ps/%.ps : [objdir]/%0latex/%.dvi
  set mkdir
{
  [DVIPS] [objdir]/%0latex/%.dvi -o [objdir]/%0ps/%.ps;
}

[objdir]/%0pdf/%.pdf : [objdir]/%0latex/%.pdf
  set mkdir
{
  cp [objdir]/%0latex/%.pdf [objdir]/%0pdf/%.pdf;
}

[objdir]/%0html/%.html : [objdir]/%0latex/%.pdf
  set mkdir
{
  [LATEX2HTML] -init_file [srcdir]/%0latex/.latex2html-init
    -dir [abs_objdir]/%0html -split 3 -toc_depth 4
    -footnode -noinfo -auto_link -transparent -image_type png
    [abs_objdir]/%0latex/%.tex;
  /*
   * Fix up [, ], and \ characters.
   * Get rid of spurious <> at the end of tables.
   */
  [SHELL] -e;
data
  for i in `find [objdir]/%0html/ -type f |grep "\.html$"` ; do
    mv $i $i.tmp
    cat $i.tmp | sed -e s/YYYbsZZZ/\\\\\\\\/g \\
      | sed -e s/YYYlbZZZ/\[/g \\
      | sed -e s/YYYrbZZZ/\]/g \\
      | sed -e \\
        "s/^\\\\\\(\\\\\\<\\\\/TABLE\\\\\\>\\\\\\)\\\\\\<\\\\\\>/\\\\1/g" \\
      > $i
    rm $i.tmp
  done
dataend
}
