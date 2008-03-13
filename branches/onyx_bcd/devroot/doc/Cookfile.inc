/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Documentation build rules.  Documentation is only rebuilt if the --with-docs
 * option is specified.
 *
 ******************************************************************************/

/*
 * Global variables.
 */
TEX_SRCS = ;
FIG_SRCS = ;

T_ONYX = [objdir]/doc/latex/onyx.tex;
#if [with_docs]
  PS_ONYX = [objdir]/doc/ps/onyx.ps;
  PDF_ONYX = [objdir]/doc/pdf/onyx.pdf;
  HTML_ONYX = [objdir]/doc/html/onyx.html;
  htmldir = [objdir]/doc/html;
#else
  PS_ONYX = [srcdir]/doc/ps/onyx.ps;
  PDF_ONYX = [srcdir]/doc/pdf/onyx.pdf;
  HTML_ONYX = [srcdir]/doc/html/onyx.html;
  htmldir = [srcdir]/doc/html;
#endif

/* LaTeX source files. */
{
  local tex_srcs = front.tex back.tex onyx_lang.tex onyx_tut.tex onyx_ex.tex;
  tex_srcs = [fromto % [srcdir]/doc/latex/% [tex_srcs]];

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

install_docs : docs_install_ps docs_install_pdf docs_install_html;
docs_install_ps :
#if [with_docs]
                  [PS_ONYX]
#endif
{
  [INSTALL] -d [PREFIX]/share/onyx/doc/ps;
  [INSTALL] -m 0444 [PS_ONYX] [PREFIX]/share/onyx/doc/ps;
}

docs_install_pdf :
#if [with_docs]
                   [PDF_ONYX]
#endif
{
  [INSTALL] -d [PREFIX]/share/onyx/doc/pdf;
  [INSTALL] -m 0444 [PDF_ONYX] [PREFIX]/share/onyx/doc/pdf;
}

/* [srcdir] instead of [objdir] for user builds in separate objdir. */
docs_install_html :
#if [with_docs]
                    [HTML_ONYX]
#endif
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/share/onyx/doc/html
[INSTALL] -d [PREFIX]/share/onyx/doc/html

for i in `find [htmldir] -type f`; do
  echo [INSTALL] -m 0444 $i [PREFIX]/share/onyx/doc/html
  [INSTALL] -m 0444 $i [PREFIX]/share/onyx/doc/html
done
dataend
}

uninstall_docs :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -rf [PREFIX]/share/onyx/doc
rm -rf [PREFIX]/share/onyx/doc
dataend
}

docs_clean :
{
  rm -f [objdir]/doc/latex/onyx.alg;
  rm -f [objdir]/doc/latex/onyx.aux;
  rm -f [objdir]/doc/latex/onyx.aux-prev;
  rm -f [objdir]/doc/latex/onyx.dvi;
  rm -f [objdir]/doc/latex/onyx.idx;
  rm -f [objdir]/doc/latex/onyx.idx-prev;
  rm -f [objdir]/doc/latex/onyx.ilg;
  rm -f [objdir]/doc/latex/onyx.ind;
  rm -f [objdir]/doc/latex/onyx.lof;
  rm -f [objdir]/doc/latex/onyx.log;
  rm -f [objdir]/doc/latex/onyx.lot;
  rm -f [objdir]/doc/latex/onyx.out;
  rm -f [objdir]/doc/latex/onyx.pdf;
  rm -f [objdir]/doc/latex/onyx.toc;
  rm -f [objdir]/doc/latex/texput.log;
}

docs_distclean :
{
  rm -f [objdir]/doc/latex/onyx.tex;
}

docs_relclean :
{
  rm -rf [objdir]/doc/html;
  rm -rf [objdir]/doc/pdf;
  rm -rf [objdir]/doc/ps;
}

[HTML_ONYX] : [PS_ONYX];

[objdir]/%0%.ps : [srcdir]/%0%.fig
  set mkdir
{
  [FIG2DEV] -L ps [srcdir]/%0%.fig [objdir]/%0%.ps;
}

[objdir]/%0ps/%.ps : [objdir]/%0latex/%.tex [TEX_SRCS]
                     [fromto [srcdir]/%0latex/%.fig [objdir]/%0latex/%.ps
                             [FIG_SRCS]]
  set mkdir
{
  /*
   * Clean up the temp files to keep {pdf}latex from stumbling over the output
   * of each other.
   *
   * Change directory to the object tree, since latex can't be made to use a
   * different location for output files.
   */

  rm -f [objdir]/%0latex/%.alg;
  rm -f [objdir]/%0latex/%.aux;
  rm -f [objdir]/%0latex/%.aux-prev;
  rm -f [objdir]/%0latex/%.idx;
  rm -f [objdir]/%0latex/%.idx-prev;
  rm -f [objdir]/%0latex/%.ilg;
  rm -f [objdir]/%0latex/%.ind;
  rm -f [objdir]/%0latex/%.lof;
  rm -f [objdir]/%0latex/%.log;
  rm -f [objdir]/%0latex/%.lot;
  rm -f [objdir]/%0latex/%.out;
  rm -f [objdir]/%0latex/%.toc;
  rm -f [objdir]/%0latex/texput.log;

  cd [objdir]/%0latex/\; [PERL] [abs_srcdir]/%0latex/ltx2e [LATEX] %.tex;
  cd [objdir]/%0latex/\; [MAKEINDEX] -c -l -s [abs_srcdir]/%0latex/%.mst %.idx;
  cd [objdir]/%0latex/\; [PERL] [abs_srcdir]/%0latex/ltx2e [LATEX] %.tex;
  [DVIPS] [objdir]/%0latex/%.dvi -o [objdir]/%0ps/%.ps;
}

/*
 * This rule depends on the PostScript version of the manual so that the two
 * rules will not run in parallel.  This is necessary in order to keep them
 * from stomping all over each others' temporory files.
 */
[objdir]/%0pdf/%.pdf : [objdir]/%0latex/%.tex [objdir]/%0ps/%.ps [TEX_SRCS]
                       [fromto [srcdir]/%0latex/%.fig [objdir]/%0latex/%.ps
                               [FIG_SRCS]]
  set mkdir
{
  /*
   * Clean up the temp files to keep {pdf}latex from stumbling over the output
   * of each other.
   *
   * Change directory to the object tree, since latex can't be made to use a
   * different location for output files.
   */

  rm -f [objdir]/%0latex/%.alg;
  rm -f [objdir]/%0latex/%.aux;
  rm -f [objdir]/%0latex/%.aux-prev;
  rm -f [objdir]/%0latex/%.idx;
  rm -f [objdir]/%0latex/%.idx-prev;
  rm -f [objdir]/%0latex/%.ilg;
  rm -f [objdir]/%0latex/%.ind;
  rm -f [objdir]/%0latex/%.lof;
  rm -f [objdir]/%0latex/%.log;
  rm -f [objdir]/%0latex/%.lot;
  rm -f [objdir]/%0latex/%.out;
  rm -f [objdir]/%0latex/%.toc;
  rm -f [objdir]/%0latex/texput.log;

  cd [objdir]/%0latex/\; [PERL] [abs_srcdir]/%0latex/ltx2e [PDFLATEX] %.tex;
  cd [objdir]/%0latex/\; [MAKEINDEX] -c -l -s [abs_srcdir]/%0latex/%.mst %.idx;
  cd [objdir]/%0latex/\; [PERL] [abs_srcdir]/%0latex/ltx2e [PDFLATEX] %.tex;
  cp [objdir]/%0latex/%.pdf [objdir]/%0pdf/%.pdf;
}

[objdir]/%0html/%.html : [objdir]/%0latex/%.tex [TEX_SRCS]
                         [fromto [srcdir]/%0latex/%.fig [objdir]/%0latex/%.ps
                                 [FIG_SRCS]]
  set mkdir
{
  [LATEX2HTML] -init_file [srcdir]/%0latex/.latex2html-init
    -dir [abs_objdir]/%0html -split 4 -toc_depth 4
    -footnode -noinfo -auto_link -transparent -image_type png -local_icons
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

  /*
   * Clean up temp files so that they won't get installed.
   */
  rm -f [objdir]/%0html/WARNINGS;
  rm -f [objdir]/%0html/images.aux;
  rm -f [objdir]/%0html/images.idx;
  rm -f [objdir]/%0html/images.log;
  rm -f [objdir]/%0html/images.pl;
  rm -f [objdir]/%0html/images.tex;
  rm -f [objdir]/%0html/internals.pl;
  rm -f [objdir]/%0html/labels.pl;
}
