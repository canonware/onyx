/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Christos Zoulas of Cornell University.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * refresh.c: Lower level screen refreshing functions
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "../include/libedit/libedit.h"

private	void	re_addc 		__P((EditLine *, int));
private	void	re_update_line 		__P((EditLine *, char *, char *, int));
private	void	re_insert		__P((EditLine *, char *, int, int,
					     char *, int));
private	void	re_delete		__P((EditLine *, char *, int, int,
					     int));
private	void	re_fastputc		__P((EditLine *, int));

private	void	re__strncopy		__P((char *, char *, size_t));
private	void	re__copy_and_pad	__P((char *, char *, size_t));

#ifdef DEBUG_REFRESH
private	void	re_printstr		__P((EditLine *, char *, char *,
					     char *));
# define __F el->el_errfile
# define _REFRESH_DEBUG(a, b, c)	do 				\
				    if (a) {			\
					_cw_out_put_f b;	\
					c;			\
				    }				\
				while (0)
/* re_printstr():
 *	Print a string on the debugging pty
 */
private void
re_printstr(el, str, f, t)
    EditLine *el;
    char *str;
    char *f, *t;
{
    _REFRESH_DEBUG(1,(__F, "[s]:\"", str),);
    while (f < t)
	_REFRESH_DEBUG(1,(__F, "[c]", *f++ & 0177),);
    _REFRESH_DEBUG(1,(__F, "\"\r\n"),);
}
#else
# define _REFRESH_DEBUG(a, b, c)
#endif


/* re_addc():
 *	Draw c, expanding tabs, control chars etc.
 */
private void
re_addc(el, c)
    EditLine *el;
    int c;
{
    c = (unsigned char)c;

    if (isprint(c)) {
	re_putc(el, c);
	return;
    }
    if (c == '\n') {			/* expand the newline	 */
	re_putc(el, '\0');		/* assure end of line	 */
	el->el_refresh.r_cursor.h = 0;	/* reset cursor pos	 */
	el->el_refresh.r_cursor.v++;
	return;
    }
    if (c == '\t') {		/* expand the tab 	 */
	for (;;) {
	    re_putc(el, ' ');
	    if ((el->el_refresh.r_cursor.h & 07) == 0)
		break;		/* go until tab stop	 */
	}
    }
    else if (iscntrl(c)) {
	re_putc(el, '^');
	if (c == 0177)
	    re_putc(el, '?');
	else
	    /* uncontrolify it; works only for iso8859-1 like sets */
	    re_putc(el, (toascii(c) | 0100));
    }
    else {
	re_putc(el, '\\');
	re_putc(el, ((c >> 6) & 07) + '0');
	re_putc(el, ((c >> 3) & 07) + '0');
	re_putc(el, (c & 07) + '0');
    }
} /* end re_addc */


/* re_putc():
 *	Draw the character given
 */
protected void
re_putc(el, c)
    EditLine *el;
    int c;
{
    _REFRESH_DEBUG(1,(__F, "printing 0x[i|b:16] '[c]'\r\n", c, c),);

    el->el_vdisplay[el->el_refresh.r_cursor.v][el->el_refresh.r_cursor.h] = c;
    el->el_refresh.r_cursor.h++;		/* advance to next place */
    if (el->el_refresh.r_cursor.h >= el->el_term.t_size.h) {
	el->el_vdisplay[el->el_refresh.r_cursor.v][el->el_term.t_size.h] = '\0';
						/* assure end of line */
	el->el_refresh.r_cursor.h = 0;				/* reset it. */
	el->el_refresh.r_cursor.v++;
	_REFRESH_DEBUG(el->el_refresh.r_cursor.v >= el->el_term.t_size.v,
		 (__F, "\r\nre_putc: overflow! r_cursor.v == [i] > [i]\r\n",
		  el->el_refresh.r_cursor.v, el->el_term.t_size.v), abort());
    }
} /* end re_putc */


/* re_refresh():
 *	draws the new virtual screen image from the current input
 *  	line, then goes line-by-line changing the real image to the new
 *	virtual image. The routine to re-draw a line can be replaced
 *	easily in hopes of a smarter one being placed there.
 */
protected void
re_refresh(el)
    EditLine *el;
{
    int i;
    char *cp;
    coord_t     cur;

    _REFRESH_DEBUG(1,(__F, "el->el_line.buffer = :[s]:\r\n",
        el->el_line.buffer),);

    /* reset the Drawing cursor */
    el->el_refresh.r_cursor.h = 0;
    el->el_refresh.r_cursor.v = 0;

    cur.h = -1;			/* set flag in case I'm not set */
    cur.v = 0;

    prompt_print(el);

    /* draw the current input buffer */
    for (cp = el->el_line.buffer; cp < el->el_line.lastchar; cp++) {
	if (cp == el->el_line.cursor) {
	    cur.h = el->el_refresh.r_cursor.h;	/* save for later */
	    cur.v = el->el_refresh.r_cursor.v;
	}
	re_addc(el, *cp);
    }

    if (cur.h == -1) {		/* if I haven't been set yet, I'm at the end */
	cur.h = el->el_refresh.r_cursor.h;
	cur.v = el->el_refresh.r_cursor.v;
    }
    /* must be done BEFORE the NUL is written */
    el->el_refresh.r_newcv = el->el_refresh.r_cursor.v;
    re_putc(el, '\0');		/* put NUL on end */

    _REFRESH_DEBUG(1,(__F,
	     "term.h=[i] vcur.h=[i] vcur.v=[i] vdisplay[[0]=\r\n:[s|w:80]:\r\n",
	     el->el_term.t_size.h, el->el_refresh.r_cursor.h,
	     el->el_refresh.r_cursor.v, el->el_vdisplay[0]),);

    _REFRESH_DEBUG(1,(__F, "updating [i] lines.\r\n", el->el_refresh.r_newcv),);
    for (i = 0; i <= el->el_refresh.r_newcv; i++) {
	/* NOTE THAT re_update_line MAY CHANGE el_display[i] */
	re_update_line(el, el->el_display[i], el->el_vdisplay[i], i);

	/*
	 * Copy the new line to be the current one, and pad out with spaces
	 * to the full width of the terminal so that if we try moving the
	 * cursor by writing the character that is at the end of the
	 * screen line, it won't be a NUL or some old leftover stuff.
	 */
	re__copy_and_pad(el->el_display[i], el->el_vdisplay[i],
			el->el_term.t_size.h);
    }
    _REFRESH_DEBUG(1,(__F,
    "\r\nel->el_refresh.r_cursor.v=[i],el->el_refresh.r_oldcv=[i] i=[i]\r\n",
	 el->el_refresh.r_cursor.v, el->el_refresh.r_oldcv, i),);

    if (el->el_refresh.r_oldcv > el->el_refresh.r_newcv)
	for (; i <= el->el_refresh.r_oldcv; i++) {
	    term_move_to_line(el, i);
	    term_move_to_char(el, 0);
	    term_clear_EOL(el, strlen(el->el_display[i]));
#ifdef DEBUG_REFRESH
	    term_overwrite(el, "C\b", 2);
#endif /* DEBUG_REFRESH */
	    *el->el_display[i] = '\0';
	}

    el->el_refresh.r_oldcv = el->el_refresh.r_newcv;	/* set for next time */
    _REFRESH_DEBUG(1,(__F,
    "\r\ncursor.h = [i], cursor.v = [i], cur.h = [i], cur.v = [i]\r\n",
		el->el_refresh.r_cursor.h, el->el_refresh.r_cursor.v,
		cur.h, cur.v),);
    term_move_to_line(el, cur.v);		/* go to where the cursor is */
    term_move_to_char(el, cur.h);
} /* end re_refresh */


/* re_goto_bottom():
 *	 used to go to last used screen line
 */
protected void
re_goto_bottom(el)
    EditLine *el;
{
    term_move_to_line(el, el->el_refresh.r_oldcv);
    term__putc('\r');
    term__putc('\n');
    re_clear_display(el);
    term__flush();
} /* end re_goto_bottom */


/* re_insert():
 *	insert num characters of s into d (in front of the character)
 *	at dat, maximum length of d is dlen
 */
private void
/*ARGSUSED*/
re_insert(el, d, dat, dlen, s, num)
    EditLine *el;
    char *d;
    int dat, dlen;
    char *s;
    int num;
{
    char *a, *b;

    if (num <= 0)
	return;
    if (num > dlen - dat)
	num = dlen - dat;

    _REFRESH_DEBUG(1,(__F,
        "re_insert() starting: [i] at [i] max [i], d == \"[s]\"\n",
	    num, dat, dlen, d),);
    _REFRESH_DEBUG(1,(__F, "s == \"[s]\"n", s),);

    /* open up the space for num chars */
    if (num > 0) {
	b = d + dlen - 1;
	a = b - num;
	while (a >= &d[dat])
	    *b-- = *a--;
	d[dlen] = '\0';		/* just in case */
    }
    _REFRESH_DEBUG(1,(__F,
		"re_insert() after insert: [i] at [i] max [i], d == \"[s]\"\n",
		num, dat, dlen, d),);
    _REFRESH_DEBUG(1,(__F, "s == \"[s]\"n", s),);

    /* copy the characters */
    for (a = d + dat; (a < d + dlen) && (num > 0); num--)
	*a++ = *s++;

    _REFRESH_DEBUG(1,(__F,
    "re_insert() after copy: [i] at [i] max [i], [s] == \"[s]\"\n",
	     num, dat, dlen, d, s),);
    _REFRESH_DEBUG(1,(__F, "s == \"[s]\"n", s),);
} /* end re_insert */


/* re_delete():
 *	delete num characters d at dat, maximum length of d is dlen
 */
private void
/*ARGSUSED*/
re_delete(el, d, dat, dlen, num)
    EditLine *el;
    char *d;
    int dat, dlen, num;
{
    char *a, *b;

    if (num <= 0)
	return;
    if (dat + num >= dlen) {
	d[dat] = '\0';
	return;
    }

    _REFRESH_DEBUG(1,(__F,
    "re_delete() starting: [i] at [i] max [i], d == \"[s]\"\n",
	    num, dat, dlen, d),);

    /* open up the space for num chars */
    if (num > 0) {
	b = d + dat;
	a = b + num;
	while (a < &d[dlen])
	    *b++ = *a++;
	d[dlen] = '\0';		/* just in case */
    }
    _REFRESH_DEBUG(1,(__F,
    "re_delete() after delete: [i] at [i] max [i], d == \"[s]\"\n",
	    num, dat, dlen, d),);
} /* end re_delete */


/* re__strncopy():
 *	Like strncpy without padding.
 */
private void
re__strncopy(a, b, n)
    char *a, *b;
    size_t n;
{
    while (n-- && *b)
	*a++ = *b++;
} /* end re__strncopy */


/* ****************************************************************
    re_update_line() is based on finding the middle difference of each line
    on the screen; vis:

			     /old first difference
	/beginning of line   |              /old last same       /old EOL
	v		     v              v                    v
old:	eddie> Oh, my little gruntle-buggy is to me, as lurgid as
new:	eddie> Oh, my little buggy says to me, as lurgid as
	^		     ^        ^			   ^
	\beginning of line   |        \new last same	   \new end of line
			     \new first difference

    all are character pointers for the sake of speed.  Special cases for
    no differences, as well as for end of line additions must be handled.
**************************************************************** */

/* Minimum at which doing an insert it "worth it".  This should be about
 * half the "cost" of going into insert mode, inserting a character, and
 * going back out.  This should really be calculated from the termcap
 * data...  For the moment, a good number for ANSI terminals.
 */
#define MIN_END_KEEP	4

private void
re_update_line(el, old, new, i)
    EditLine *el;
    char *old, *new;
    int     i;
{
    char *o, *n, *p, c;
    char   *ofd, *ols, *oe, *nfd, *nls, *ne;
    char   *osb, *ose, *nsb, *nse;
    int     fx, sx;

    /*
     * find first diff
     */
    for (o = old, n = new; *o && (*o == *n); o++, n++)
	continue;
    ofd = o;
    nfd = n;

    /*
     * Find the end of both old and new
     */
    while (*o)
	o++;
    /*
     * Remove any trailing blanks off of the end, being careful not to
     * back up past the beginning.
     */
    while (ofd < o) {
	if (o[-1] != ' ')
	    break;
	o--;
    }
    oe = o;
    *oe = '\0';

    while (*n)
	n++;

    /* remove blanks from end of new */
    while (nfd < n) {
	if (n[-1] != ' ')
	    break;
	n--;
    }
    ne = n;
    *ne = '\0';

    /*
     * if no diff, continue to next line of redraw
     */
    if (*ofd == '\0' && *nfd == '\0') {
	_REFRESH_DEBUG(1,(__F, "no difference.\r\n"),);
	return;
    }

    /*
     * find last same pointer
     */
    while ((o > ofd) && (n > nfd) && (*--o == *--n))
	continue;
    ols = ++o;
    nls = ++n;

    /*
     * find same begining and same end
     */
    osb = ols;
    nsb = nls;
    ose = ols;
    nse = nls;

    /*
     * case 1: insert: scan from nfd to nls looking for *ofd
     */
    if (*ofd) {
	for (c = *ofd, n = nfd; n < nls; n++) {
	    if (c == *n) {
		for (o = ofd, p = n; p < nls && o < ols && *o == *p; o++, p++)
		    continue;
		/*
		 * if the new match is longer and it's worth keeping, then we
		 * take it
		 */
		if (((nse - nsb) < (p - n)) && (2 * (p - n) > n - nfd)) {
		    nsb = n;
		    nse = p;
		    osb = ofd;
		    ose = o;
		}
	    }
	}
    }

    /*
     * case 2: delete: scan from ofd to ols looking for *nfd
     */
    if (*nfd) {
	for (c = *nfd, o = ofd; o < ols; o++) {
	    if (c == *o) {
		for (n = nfd, p = o; p < ols && n < nls && *p == *n; p++, n++)
		    continue;
		/*
		 * if the new match is longer and it's worth keeping, then we
		 * take it
		 */
		if (((ose - osb) < (p - o)) && (2 * (p - o) > o - ofd)) {
		    nsb = nfd;
		    nse = n;
		    osb = o;
		    ose = p;
		}
	    }
	}
    }

    /*
     * Pragmatics I: If old trailing whitespace or not enough characters to
     * save to be worth it, then don't save the last same info.
     */
    if ((oe - ols) < MIN_END_KEEP) {
	ols = oe;
	nls = ne;
    }

    /*
     * Pragmatics II: if the terminal isn't smart enough, make the data dumber
     * so the smart update doesn't try anything fancy
     */

    /*
     * fx is the number of characters we need to insert/delete: in the
     * beginning to bring the two same begins together
     */
    fx = (nsb - nfd) - (osb - ofd);
    /*
     * sx is the number of characters we need to insert/delete: in the end to
     * bring the two same last parts together
     */
    sx = (nls - nse) - (ols - ose);

    if (!EL_CAN_INSERT) {
	if (fx > 0) {
	    osb = ols;
	    ose = ols;
	    nsb = nls;
	    nse = nls;
	}
	if (sx > 0) {
	    ols = oe;
	    nls = ne;
	}
	if ((ols - ofd) < (nls - nfd)) {
	    ols = oe;
	    nls = ne;
	}
    }
    if (!EL_CAN_DELETE) {
	if (fx < 0) {
	    osb = ols;
	    ose = ols;
	    nsb = nls;
	    nse = nls;
	}
	if (sx < 0) {
	    ols = oe;
	    nls = ne;
	}
	if ((ols - ofd) > (nls - nfd)) {
	    ols = oe;
	    nls = ne;
	}
    }

    /*
     * Pragmatics III: make sure the middle shifted pointers are correct if
     * they don't point to anything (we may have moved ols or nls).
     */
    /* if the change isn't worth it, don't bother */
    /* was: if (osb == ose) */
    if ((ose - osb) < MIN_END_KEEP) {
	osb = ols;
	ose = ols;
	nsb = nls;
	nse = nls;
    }

    /*
     * Now that we are done with pragmatics we recompute fx, sx
     */
    fx = (nsb - nfd) - (osb - ofd);
    sx = (nls - nse) - (ols - ose);

    _REFRESH_DEBUG(1,(__F, "\n"),);
    _REFRESH_DEBUG(1,(__F, "ofd [i], osb [i], ose [i], ols [i], oe [i]\n",
	    ofd - old, osb - old, ose - old, ols - old, oe - old),);
    _REFRESH_DEBUG(1,(__F, "nfd [i], nsb [i], nse [i], nls [i], ne [i]\n",
	    nfd - new, nsb - new, nse - new, nls - new, ne - new),);
    _REFRESH_DEBUG(1,(__F,
		"xxx-xxx:\"00000000001111111111222222222233333333334\"\r\n"),);
    _REFRESH_DEBUG(1,(__F,
		"xxx-xxx:\"01234567890123456789012345678901234567890\"\r\n"),);
#ifdef DEBUG_REFRESH
    re_printstr(el, "old- oe", old, oe);
    re_printstr(el, "new- ne", new, ne);
    re_printstr(el, "old-ofd", old, ofd);
    re_printstr(el, "new-nfd", new, nfd);
    re_printstr(el, "ofd-osb", ofd, osb);
    re_printstr(el, "nfd-nsb", nfd, nsb);
    re_printstr(el, "osb-ose", osb, ose);
    re_printstr(el, "nsb-nse", nsb, nse);
    re_printstr(el, "ose-ols", ose, ols);
    re_printstr(el, "nse-nls", nse, nls);
    re_printstr(el, "ols- oe", ols, oe);
    re_printstr(el, "nls- ne", nls, ne);
#endif /* DEBUG_REFRESH */

    /*
     * el_cursor.v to this line i MUST be in this routine so that if we
     * don't have to change the line, we don't move to it. el_cursor.h to first
     * diff char
     */
    term_move_to_line(el, i);

    /*
     * at this point we have something like this:
     *
     * /old                  /ofd    /osb               /ose    /ols     /oe
     * v.....................v       v..................v       v........v
     * eddie> Oh, my fredded gruntle-buggy is to me, as foo var lurgid as
     * eddie> Oh, my fredded quiux buggy is to me, as gruntle-lurgid as
     * ^.....................^     ^..................^       ^........^
     * \new                  \nfd  \nsb               \nse     \nls    \ne
     *
     * fx is the difference in length between the the chars between nfd and
     * nsb, and the chars between ofd and osb, and is thus the number of
     * characters to delete if < 0 (new is shorter than old, as above),
     * or insert (new is longer than short).
     *
     * sx is the same for the second differences.
     */

    /*
     * if we have a net insert on the first difference, AND inserting the net
     * amount ((nsb-nfd) - (osb-ofd)) won't push the last useful character
     * (which is ne if nls != ne, otherwise is nse) off the edge of the screen
     * (el->el_term.t_size.h) else we do the deletes first so that we keep everything we need
     * to.
     */

    /*
     * if the last same is the same like the end, there is no last same part,
     * otherwise we want to keep the last same part set p to the last useful
     * old character
     */
    p = (ols != oe) ? oe : ose;

    /*
     * if (There is a diffence in the beginning) && (we need to insert
     * characters) && (the number of characters to insert is less than the term
     * width) We need to do an insert! else if (we need to delete characters)
     * We need to delete characters! else No insert or delete
     */
    if ((nsb != nfd) && fx > 0 && ((p - old) + fx <= el->el_term.t_size.h)) {
	_REFRESH_DEBUG(1,(__F, "first diff insert at [i]...\r\n", nfd - new),);
	/*
	 * Move to the first char to insert, where the first diff is.
	 */
	term_move_to_char(el, nfd - new);
	/*
	 * Check if we have stuff to keep at end
	 */
	if (nsb != ne) {
	    _REFRESH_DEBUG(1,(__F, "with stuff to keep at end\r\n"),);
	    /*
	     * insert fx chars of new starting at nfd
	     */
	    if (fx > 0) {
		_REFRESH_DEBUG(!EL_CAN_INSERT,
			 (__F, "ERROR: cannot insert in early first diff\n"),);
		term_insertwrite(el, nfd, fx);
		re_insert(el, old, ofd - old, el->el_term.t_size.h, nfd, fx);
	    }
	    /*
	     * write (nsb-nfd) - fx chars of new starting at (nfd + fx)
	     */
	    term_overwrite(el, nfd + fx, (nsb - nfd) - fx);
	    re__strncopy(ofd + fx, nfd + fx, (nsb - nfd) - fx);
	}
	else {
	    _REFRESH_DEBUG(1,(__F, "without anything to save\r\n"),);
	    term_overwrite(el, nfd, (nsb - nfd));
	    re__strncopy(ofd, nfd, (nsb - nfd));
	    /*
	     * Done
	     */
	    return;
	}
    }
    else if (fx < 0) {
	_REFRESH_DEBUG(1,(__F, "first diff delete at [i]...\r\n", ofd - old),);
	/*
	 * move to the first char to delete where the first diff is
	 */
	term_move_to_char(el, ofd - old);
	/*
	 * Check if we have stuff to save
	 */
	if (osb != oe) {
	    _REFRESH_DEBUG(1,(__F, "with stuff to save at end\r\n"),);
	    /*
	     * fx is less than zero *always* here but we check for code
	     * symmetry
	     */
	    if (fx < 0) {
		_REFRESH_DEBUG(!EL_CAN_DELETE,
			 (__F, "ERROR: cannot delete in first diff\n"),);
		term_deletechars(el, -fx);
		re_delete(el, old, ofd - old, el->el_term.t_size.h, -fx);
	    }
	    /*
	     * write (nsb-nfd) chars of new starting at nfd
	     */
	    term_overwrite(el, nfd, (nsb - nfd));
	    re__strncopy(ofd, nfd, (nsb - nfd));

	}
	else {
	    _REFRESH_DEBUG(1,(__F, "but with nothing left to save\r\n"),);
	    /*
	     * write (nsb-nfd) chars of new starting at nfd
	     */
	    term_overwrite(el, nfd, (nsb - nfd));
	    _REFRESH_DEBUG(1,(__F, "cleareol [i]\n", (oe - old) - (ne - new)),);
	    term_clear_EOL(el, (oe - old) - (ne - new));
	    /*
	     * Done
	     */
	    return;
	}
    }
    else
	fx = 0;

    if (sx < 0) {
	_REFRESH_DEBUG(1,(__F, "second diff delete at [i]...\r\n", (ose - old) +
	    fx),);
	/*
	 * Check if we have stuff to delete
	 */
	/*
	 * fx is the number of characters inserted (+) or deleted (-)
	 */

	term_move_to_char(el, (ose - old) + fx);
	/*
	 * Check if we have stuff to save
	 */
	if (ols != oe) {
	    _REFRESH_DEBUG(1,(__F, "with stuff to save at end\r\n"),);
	    /*
	     * Again a duplicate test.
	     */
	    if (sx < 0) {
		_REFRESH_DEBUG(!EL_CAN_DELETE,
			 (__F, "ERROR: cannot delete in second diff\n"),);
		term_deletechars(el, -sx);
	    }

	    /*
	     * write (nls-nse) chars of new starting at nse
	     */
	    term_overwrite(el, nse, (nls - nse));
	}
	else {
	    _REFRESH_DEBUG(1,(__F, "but with nothing left to save\r\n"),);
	    term_overwrite(el, nse, (nls - nse));
	    _REFRESH_DEBUG(1,(__F, "cleareol [i]\n", (oe - old) - (ne - new)),);
	    term_clear_EOL(el, (oe - old) - (ne - new));
	}
    }

    /*
     * if we have a first insert AND WE HAVEN'T ALREADY DONE IT...
     */
    if ((nsb != nfd) && (osb - ofd) <= (nsb - nfd) && (fx == 0)) {
	_REFRESH_DEBUG(1,(__F, "late first diff insert at [i]...\r\n", nfd -
	    new),);

	term_move_to_char(el, nfd - new);
	/*
	 * Check if we have stuff to keep at the end
	 */
	if (nsb != ne) {
	    _REFRESH_DEBUG(1,(__F, "with stuff to keep at end\r\n"),);
	    /*
	     * We have to recalculate fx here because we set it
	     * to zero above as a flag saying that we hadn't done
	     * an early first insert.
	     */
	    fx = (nsb - nfd) - (osb - ofd);
	    if (fx > 0) {
		/*
		 * insert fx chars of new starting at nfd
		 */
		_REFRESH_DEBUG(!EL_CAN_INSERT,
			 (__F, "ERROR: cannot insert in late first diff\n"),);
		term_insertwrite(el, nfd, fx);
		re_insert(el, old, ofd - old, el->el_term.t_size.h, nfd, fx);
	    }

	    /*
	     * write (nsb-nfd) - fx chars of new starting at (nfd + fx)
	     */
	    term_overwrite(el, nfd + fx, (nsb - nfd) - fx);
	    re__strncopy(ofd + fx, nfd + fx, (nsb - nfd) - fx);
	}
	else {
	    _REFRESH_DEBUG(1,(__F, "without anything to save\r\n"),);
	    term_overwrite(el, nfd, (nsb - nfd));
	    re__strncopy(ofd, nfd, (nsb - nfd));
	}
    }

    /*
     * line is now NEW up to nse
     */
    if (sx >= 0) {
	_REFRESH_DEBUG(1,(__F, "second diff insert at [i]...\r\n", nse - new),);
	term_move_to_char(el, nse - new);
	if (ols != oe) {
	    _REFRESH_DEBUG(1,(__F, "with stuff to keep at end\r\n"),);
	    if (sx > 0) {
		/* insert sx chars of new starting at nse */
		_REFRESH_DEBUG(!EL_CAN_INSERT,
		         (__F, "ERROR: cannot insert in second diff\n"),);
		term_insertwrite(el, nse, sx);
	    }

	    /*
	     * write (nls-nse) - sx chars of new starting at (nse + sx)
	     */
	    term_overwrite(el, nse + sx, (nls - nse) - sx);
	}
	else {
	    _REFRESH_DEBUG(1,(__F, "without anything to save\r\n"),);
	    term_overwrite(el, nse, (nls - nse));

	    /*
             * No need to do a clear-to-end here because we were doing
	     * a second insert, so we will have over written all of the
	     * old string.
	     */
	}
    }
    _REFRESH_DEBUG(1,(__F, "done.\r\n"),);
} /* re_update_line */


/* re__copy_and_pad():
 *	Copy string and pad with spaces
 */
private void
re__copy_and_pad(dst, src, width)
    char *dst, *src;
    size_t width;
{
    int i;

    for (i = 0; i < width; i++) {
	if (*src == '\0')
	    break;
	*dst++ = *src++;
    }

    while (i < width) {
	*dst++ = ' ';
	i++;
    }
    *dst = '\0';
} /* end re__copy_and_pad */


/* re_refresh_cursor():
 *	Move to the new cursor position
 */
protected void
re_refresh_cursor(el)
    EditLine *el;
{
    char *cp;
    int	c;
    int h, v, th;

    /* first we must find where the cursor is... */
    h  = el->el_prompt.p_pos.h;
    v  = el->el_prompt.p_pos.v;
    th = el->el_term.t_size.h;		/* optimize for speed 		*/

    /* do input buffer to el->el_line.cursor */
    for (cp = el->el_line.buffer; cp < el->el_line.cursor; cp++) {
	c = (unsigned char)*cp;
	h++;			/* all chars at least this long */

	if (c == '\n') {	/* handle newline in data part too */
	    h = 0;
	    v++;
	}
	else {
	    if (c == '\t') {	/* if a tab, to next tab stop */
		while (h & 07) {
		    h++;
		}
	    }
	    else if (iscntrl(c)) {	/* if control char */
		h++;
		if (h > th) {	/* if overflow, compensate */
		    h = 1;
		    v++;
		}
	    }
	    else if (!isprint(c)) {
		h += 3;
		if (h > th) {	/* if overflow, compensate */
		    h = h - th;
		    v++;
		}
	    }
	}

	if (h >= th) {		/* check, extra long tabs picked up here also */
	    h = 0;
	    v++;
	}
    }

    /* now go there */
    term_move_to_line(el, v);
    term_move_to_char(el, h);
    term__flush();
} /* re_refresh_cursor */


/* re_fastputc():
 *	Add a character fast.
 */
private void
re_fastputc(el, c)
    EditLine *el;
    int    c;
{
    term__putc(c);
    el->el_display[el->el_cursor.v][el->el_cursor.h++] = c;
    if (el->el_cursor.h >= el->el_term.t_size.h) {
	/* if we must overflow */
	el->el_cursor.h = 0;
	el->el_cursor.v++;
	el->el_refresh.r_oldcv++;
	term__putc('\r');
	term__putc('\n');
    }
} /* end re_fastputc */


/* re_fastaddc():
 *	we added just one char, handle it fast.
 *	Assumes that screen cursor == real cursor
 */
protected void
re_fastaddc(el)
    EditLine *el;
{
    int c;

    c = (unsigned char)el->el_line.cursor[-1];

    if (c == '\t' || el->el_line.cursor != el->el_line.lastchar) {
	re_refresh(el);		/* too hard to handle */
	return;
    }				/* else (only do at end of line, no TAB) */

    if (iscntrl(c)) {		/* if control char, do caret */
	char mc = (c == 0177) ? '?' : (toascii(c) | 0100);
	re_fastputc(el, '^');
	re_fastputc(el, mc);
    }
    else if (isprint(c)) {	/* normal char */
	re_fastputc(el, c);
    }
    else {
	re_fastputc(el, '\\');
	re_fastputc(el, ((c >> 6) & 7) + '0');
	re_fastputc(el, ((c >> 3) & 7) + '0');
	re_fastputc(el, (c & 7) + '0');
    }
    term__flush();
} /* end re_fastaddc */


/* re_clear_display():
 *	clear the screen buffers so that new new prompt starts fresh.
 */
protected void
re_clear_display(el)
    EditLine *el;
{
    int i;

    el->el_cursor.v = 0;
    el->el_cursor.h = 0;
    for (i = 0; i < el->el_term.t_size.v; i++)
	el->el_display[i][0] = '\0';
    el->el_refresh.r_oldcv = 0;
} /* end re_clear_display */


/* re_clear_lines():
 *	Make sure all lines are *really* blank
 */
protected void
re_clear_lines(el)
    EditLine *el;
{
    if (EL_CAN_CEOL) {
	int i;
	term_move_to_char(el, 0);
	for (i = 0; i <= el->el_refresh.r_oldcv; i++) {
	    /* for each line on the screen */
	    term_move_to_line(el, i);
	    term_clear_EOL(el, el->el_term.t_size.h);
	}
	term_move_to_line(el, 0);
    }
    else {
	term_move_to_line(el, el->el_refresh.r_oldcv);	/* go to last line */
	term__putc('\r');				/* go to BOL */
	term__putc('\n');				/* go to new line */
    }
} /* end re_clear_lines */
