/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *
 *	@(#)queue.h	8.5 (Berkeley) 8/20/94
 * $FreeBSD: src/sys/sys/queue.h,v 1.34 2000/04/27 22:50:12 archie Exp $
 */

/*
 * This file defines six types of data structures: singly-linked lists,
 * singly-linked tail queues, lists, tail queues, circular queues, and rings.
 *
 * A singly-linked list is headed by a single forward pointer. The elements
 * are singly linked for minimum space and pointer manipulation overhead at
 * the expense of O(n) removal for arbitrary elements. New elements can be
 * added to the list after an existing element or at the head of the list.
 * Elements being removed from the head of the list should use the explicit
 * macro for this purpose for optimum efficiency. A singly-linked list may
 * only be traversed in the forward direction.  Singly-linked lists are ideal
 * for applications with large datasets and few or no removals or for
 * implementing a LIFO queue.
 *
 * A singly-linked tail queue is headed by a pair of pointers, one to the
 * head of the list and the other to the tail of the list. The elements are
 * singly linked for minimum space and pointer manipulation overhead at the
 * expense of O(n) removal for arbitrary elements. New elements can be added
 * to the list after an existing element, at the head of the list, or at the
 * end of the list. Elements being removed from the head of the tail queue
 * should use the explicit macro for this purpose for optimum efficiency.
 * A singly-linked tail queue may only be traversed in the forward direction.
 * Singly-linked tail queues are ideal for applications with large datasets
 * and few or no removals or for implementing a FIFO queue.
 *
 * A list is headed by a single forward pointer (or an array of forward
 * pointers for a hash table header). The elements are doubly linked
 * so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before
 * or after an existing element or at the head of the list. A list
 * may only be traversed in the forward direction.
 *
 * A tail queue is headed by a pair of pointers, one to the head of the
 * list and the other to the tail of the list. The elements are doubly
 * linked so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before or
 * after an existing element, at the head of the list, or at the end of
 * the list. A tail queue may only be traversed in the forward direction.
 *
 * A circle queue is headed by a pair of pointers, one to the head of the
 * list and the other to the tail of the list. The elements are doubly
 * linked so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before or after
 * an existing element, at the head of the list, or at the end of the list.
 * A circle queue may be traversed in either direction, but has a more
 * complex end of list detection.
 *
 * A ring has no header.  All elments are equal.  Rings are doubly linked, and
 * can be traversed and operated on in both directions.  Rings can be cut,
 * given two elements in the same ring, or spliced, given two separate rings.
 *
 *			qsl	ql	qst	qt	qc	qr
 * _head		+	+	+	+	+	-
 * _entry		+	+	+	+	+	+
 * _init		+	+	+	+	+	+
 * _empty		+	+	+	+	+	-
 * _first		+	+	+	+	+	-
 * _next		+	+	+	+	+	+
 * _prev		-	-	-	+	+	+
 * _last		-	-	+	+	+	-
 * _foreach		+	+	+	+	+	+
 * _insert_head		+	+	+	+	+	-
 * _insert_before	-	+	-	+	+	+
 * _insert_after	+	+	+	+	+	+
 * _insert_tail		-	-	+	+	+	-
 * _remove_head		+	-	+	-	-	-
 * _remove		+	+	+	+	+	+
 * _meld		-	-	-	-	-	+
 * _split		-	-	-	-	-	+
 *
 */

/*
 * Singly-linked List definitions.
 */
#define qsl_head(name, type)						\
struct name {								\
	type *slh_first;	/* first element */			\
}

#define qsl_head_initializer(head)					\
	{ NULL }
 
#define qsl_entry(type)							\
struct {								\
	type *sle_next;		/* next element */			\
}
 
/*
 * Singly-linked List functions.
 */
#define	qsl_empty(head)	((head)->slh_first == NULL)

#define	qsl_first(head)	((head)->slh_first)

#define qsl_foreach(var, head, field)					\
	for((var) = (head)->slh_first; (var); (var) = (var)->field.sle_next)

#define qsl_init(head) {						\
	(head)->slh_first = NULL;					\
}

#define qsl_insert_after(slistelm, elm, field) do {			\
	(elm)->field.sle_next = (slistelm)->field.sle_next;		\
	(slistelm)->field.sle_next = (elm);				\
} while (0)

#define qsl_insert_head(head, elm, field) do {				\
	(elm)->field.sle_next = (head)->slh_first;			\
	(head)->slh_first = (elm);					\
} while (0)

#define qsl_next(elm, field)	((elm)->field.sle_next)

#define qsl_remove_head(head, field) do {				\
	(head)->slh_first = (head)->slh_first->field.sle_next;		\
} while (0)

#define qsl_remove(head, elm, type, field) do {				\
	if ((head)->slh_first == (elm)) {				\
		qsl_remove_head((head), field);				\
	}								\
	else {								\
		type *curelm = (head)->slh_first;			\
		while( curelm->field.sle_next != (elm) )		\
			curelm = curelm->field.sle_next;		\
		curelm->field.sle_next =				\
		    curelm->field.sle_next->field.sle_next;		\
	}								\
} while (0)

/*
 * Singly-linked Tail queue definitions.
 */
#define qst_head(name, type)						\
struct name {								\
	type *stqh_first;	/* first element */			\
	type **stqh_last;	/* addr of last next element */		\
}

#define qst_head_initializer(head)					\
	{ NULL, &(head).stqh_first }

#define qst_entry(type)							\
struct {								\
	type *stqe_next;	/* next element */			\
}

/*
 * Singly-linked Tail queue functions.
 */
#define qst_empty(head) ((head)->stqh_first == NULL)

#define	qst_init(head) do {						\
	(head)->stqh_first = NULL;					\
	(head)->stqh_last = &(head)->stqh_first;			\
} while (0)

#define qst_first(head)	((head)->stqh_first)
#define qst_last(head)	(*(head)->stqh_last)

#define qst_foreach(var, head, field)					\
	for((var) = (head)->stqh_first; (var); (var) = (var)->field.stqe_next)

#define qst_insert_head(head, elm, field) do {				\
	if (((elm)->field.stqe_next = (head)->stqh_first) == NULL)	\
		(head)->stqh_last = &(elm)->field.stqe_next;		\
	(head)->stqh_first = (elm);					\
} while (0)

#define qst_insert_tail(head, elm, field) do {				\
	(elm)->field.stqe_next = NULL;					\
	*(head)->stqh_last = (elm);					\
	(head)->stqh_last = &(elm)->field.stqe_next;			\
} while (0)

#define qst_insert_after(head, tqelm, elm, field) do {			\
	if (((elm)->field.stqe_next = (tqelm)->field.stqe_next) == NULL)\
		(head)->stqh_last = &(elm)->field.stqe_next;		\
	(tqelm)->field.stqe_next = (elm);				\
} while (0)

#define qst_next(elm, field)	((elm)->field.stqe_next)

#define qst_remove_head(head, field) do {				\
	if (((head)->stqh_first =					\
	     (head)->stqh_first->field.stqe_next) == NULL)		\
		(head)->stqh_last = &(head)->stqh_first;		\
} while (0)

#define qst_remove_head_UNTIL(head, elm, field) do {			\
	if (((head)->stqh_first = (elm)->field.stqe_next) == NULL)	\
		(head)->stqh_last = &(head)->stqh_first;		\
} while (0)

#define qst_remove(head, elm, type, field) do {				\
	if ((head)->stqh_first == (elm)) {				\
		qst_remove_head(head, field);				\
	}								\
	else {								\
		type *curelm = (head)->stqh_first;			\
		while( curelm->field.stqe_next != (elm) )		\
			curelm = curelm->field.stqe_next;		\
		if((curelm->field.stqe_next =				\
		    curelm->field.stqe_next->field.stqe_next) == NULL)	\
			(head)->stqh_last = &(curelm)->field.stqe_next;	\
	}								\
} while (0)

/*
 * List definitions.
 */
#define ql_head(name, type)						\
struct name {								\
	type *lh_first;		/* first element */			\
}

#define ql_head_initializer(head)					\
	{ NULL }

#define ql_entry(type)							\
struct {								\
	type *le_next;		/* next element */			\
	type **le_prev;		/* address of previous next element */	\
}

/*
 * List functions.
 */

#define	ql_empty(head) ((head)->lh_first == NULL)

#define ql_first(head)	((head)->lh_first)

#define ql_foreach(var, head, field)					\
	for((var) = (head)->lh_first; (var); (var) = (var)->field.le_next)

#define	ql_init(head) do {						\
	(head)->lh_first = NULL;					\
} while (0)

#define ql_insert_after(listelm, elm, field) do {			\
	if (((elm)->field.le_next = (listelm)->field.le_next) != NULL)	\
		(listelm)->field.le_next->field.le_prev =		\
		    &(elm)->field.le_next;				\
	(listelm)->field.le_next = (elm);				\
	(elm)->field.le_prev = &(listelm)->field.le_next;		\
} while (0)

#define ql_insert_before(listelm, elm, field) do {			\
	(elm)->field.le_prev = (listelm)->field.le_prev;		\
	(elm)->field.le_next = (listelm);				\
	*(listelm)->field.le_prev = (elm);				\
	(listelm)->field.le_prev = &(elm)->field.le_next;		\
} while (0)

#define ql_insert_head(head, elm, field) do {				\
	if (((elm)->field.le_next = (head)->lh_first) != NULL)		\
		(head)->lh_first->field.le_prev = &(elm)->field.le_next;\
	(head)->lh_first = (elm);					\
	(elm)->field.le_prev = &(head)->lh_first;			\
} while (0)

#define ql_next(elm, field)	((elm)->field.le_next)

#define ql_remove(elm, field) do {					\
	if ((elm)->field.le_next != NULL)				\
		(elm)->field.le_next->field.le_prev = 			\
		    (elm)->field.le_prev;				\
	*(elm)->field.le_prev = (elm)->field.le_next;			\
} while (0)

/*
 * Tail queue definitions.
 */
#define qt_head(name, type)						\
struct name {								\
	type *tqh_first;	/* first element */			\
	type **tqh_last;	/* addr of last next element */		\
}

#define qt_head_initializer(head)					\
	{ NULL, &(head).tqh_first }

#define qt_entry(type)							\
struct {								\
	type *tqe_next;		/* next element */			\
	type **tqe_prev;	/* address of previous next element */	\
}

/*
 * Tail queue functions.
 */
#define	qt_empty(head) ((head)->tqh_first == NULL)

#define qt_foreach(var, head, field)					\
	for (var = qt_first(head); var; var = TAILQ_next(var, field))

#define	qt_first(head) ((head)->tqh_first)

#define	qt_last(head, headname)						\
	(*(((struct headname *)((head)->tqh_last))->tqh_last))

#define	qt_next(elm, field) ((elm)->field.tqe_next)

#define qt_prev(elm, headname, field)					\
	(*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))

#define	qt_init(head) do {						\
	(head)->tqh_first = NULL;					\
	(head)->tqh_last = &(head)->tqh_first;				\
} while (0)

#define qt_insert_head(head, elm, field) do {				\
	if (((elm)->field.tqe_next = (head)->tqh_first) != NULL)	\
		(head)->tqh_first->field.tqe_prev =			\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(head)->tqh_first = (elm);					\
	(elm)->field.tqe_prev = &(head)->tqh_first;			\
} while (0)

#define qt_insert_tail(head, elm, field) do {				\
	(elm)->field.tqe_next = NULL;					\
	(elm)->field.tqe_prev = (head)->tqh_last;			\
	*(head)->tqh_last = (elm);					\
	(head)->tqh_last = &(elm)->field.tqe_next;			\
} while (0)

#define qt_insert_after(head, listelm, elm, field) do {			\
	if (((elm)->field.tqe_next = (listelm)->field.tqe_next) != NULL)\
		(elm)->field.tqe_next->field.tqe_prev = 		\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(listelm)->field.tqe_next = (elm);				\
	(elm)->field.tqe_prev = &(listelm)->field.tqe_next;		\
} while (0)

#define qt_insert_before(listelm, elm, field) do {			\
	(elm)->field.tqe_prev = (listelm)->field.tqe_prev;		\
	(elm)->field.tqe_next = (listelm);				\
	*(listelm)->field.tqe_prev = (elm);				\
	(listelm)->field.tqe_prev = &(elm)->field.tqe_next;		\
} while (0)

#define qt_remove(head, elm, field) do {				\
	if (((elm)->field.tqe_next) != NULL)				\
		(elm)->field.tqe_next->field.tqe_prev = 		\
		    (elm)->field.tqe_prev;				\
	else								\
		(head)->tqh_last = (elm)->field.tqe_prev;		\
	*(elm)->field.tqe_prev = (elm)->field.tqe_next;			\
} while (0)

/*
 * Circular queue definitions.
 */
#define qc_head(name, type)						\
struct name {								\
	type *cqh_first;		/* first element */		\
	type *cqh_last;			/* last element */		\
}

#define qc_entry(type)							\
struct {								\
	type *cqe_next;			/* next element */		\
	type *cqe_prev;			/* previous element */		\
}

/*
 * Circular queue functions.
 */
#define qc_empty(head) ((head)->cqh_first == (void *)(head))

#define qc_first(head) ((head)->cqh_first)

#define qc_foreach(var, head, field)					\
	for((var) = (head)->cqh_first;					\
	    (var) != (void *)(head);					\
	    (var) = (var)->field.cqe_next)

#define qc_foreach_reverse(var, head, field)				\
	for((var) = (head)->cqh_last;					\
	    (var) != (void *)(head);					\
	    (var) = (var)->field.cqe_prev)

#define	qc_init(head) do {						\
	(head)->cqh_first = (void *)(head);				\
	(head)->cqh_last = (void *)(head);				\
} while (0)

#define qc_insert_after(head, listelm, elm, field) do {	\
	(elm)->field.cqe_next = (listelm)->field.cqe_next;		\
	(elm)->field.cqe_prev = (listelm);				\
	if ((listelm)->field.cqe_next == (void *)(head))		\
		(head)->cqh_last = (elm);				\
	else								\
		(listelm)->field.cqe_next->field.cqe_prev = (elm);	\
	(listelm)->field.cqe_next = (elm);				\
} while (0)

#define qc_insert_before(head, listelm, elm, field) do {		\
	(elm)->field.cqe_next = (listelm);				\
	(elm)->field.cqe_prev = (listelm)->field.cqe_prev;		\
	if ((listelm)->field.cqe_prev == (void *)(head))		\
		(head)->cqh_first = (elm);				\
	else								\
		(listelm)->field.cqe_prev->field.cqe_next = (elm);	\
	(listelm)->field.cqe_prev = (elm);				\
} while (0)

#define qc_insert_head(head, elm, field) do {				\
	(elm)->field.cqe_next = (head)->cqh_first;			\
	(elm)->field.cqe_prev = (void *)(head);				\
	if ((head)->cqh_last == (void *)(head))				\
		(head)->cqh_last = (elm);				\
	else								\
		(head)->cqh_first->field.cqe_prev = (elm);		\
	(head)->cqh_first = (elm);					\
} while (0)

#define qc_insert_tail(head, elm, field) do {				\
	(elm)->field.cqe_next = (void *)(head);				\
	(elm)->field.cqe_prev = (head)->cqh_last;			\
	if ((head)->cqh_first == (void *)(head))			\
		(head)->cqh_first = (elm);				\
	else								\
		(head)->cqh_last->field.cqe_next = (elm);		\
	(head)->cqh_last = (elm);					\
} while (0)

#define qc_last(head) ((head)->cqh_last)

#define qc_next(elm,field) ((elm)->field.cqe_next)

#define qc_prev(elm,field) ((elm)->field.cqe_prev)

#define	qc_remove(head, elm, field) do {				\
	if ((elm)->field.cqe_next == (void *)(head))			\
		(head)->cqh_last = (elm)->field.cqe_prev;		\
	else								\
		(elm)->field.cqe_next->field.cqe_prev =			\
		    (elm)->field.cqe_prev;				\
	if ((elm)->field.cqe_prev == (void *)(head))			\
		(head)->cqh_first = (elm)->field.cqe_next;		\
	else								\
		(elm)->field.cqe_prev->field.cqe_next =			\
		    (elm)->field.cqe_next;				\
} while (0)

/*
 * Ring definitions.
 */
#define qr_entry(type)							\
struct {								\
	type *r_next;			/* Next element. */		\
	type *r_prev;			/* Previous element. */		\
}

/*
 * Ring functions.
 */
#define qr_init(elm, field) do {					\
	(elm)->field.r_next = (elm);					\
	(elm)->field.r_prev = (elm);					\
} while (0)

#define qr_next(elm, field) ((elm)->field.r_next)

#define qr_prev(elm, field) ((elm)->field.r_prev)

#define qr_foreach(var, elm, field)					\
	for ((var) = (elm);						\
	    (var) != NULL;						\
	    (var) = (((var)->field.r_next != (elm))			\
	    ? (elm)->field.r_next : NULL)))

#define qr_foreach_reverse(var, elm, field)				\
	for ((var) = (elm);						\
	    (var) != NULL;						\
	    (var) = (((var)->field.r_prev != (elm))			\
	    ? (elm)->field.r_prev : NULL)))

#define qr_insert_before(ringelm, elm, field) do {			\
	(elm)->field.r_prev = (ringelm)->field.r_prev;			\
	(elm)->field.r_next = (ringelm);				\
	(elm)->field.r_prev->field.r_next = (elm);			\
	(ringelm)->field.r_prev = (elm);				\
} while (0)

#define qr_insert_after(ringelm, elm, field) do {			\
	(elm)->field.r_next = (ringelm)->field.r_next;			\
	(elm)->field.r_prev = (ringelm);				\
	(elm)->field.r_next->field.r_prev = (elm);			\
	(ringelm)->field.r_next = (elm);				\
} while (0)

#define qr_meld(elm_a, elm_b, field) do {				\
	void	*t;							\
	(elm_a)->field.r_prev->field.r_next = (elm_b);			\
	(elm_b)->field.r_prev->field.r_next = (elm_a);			\
	t = (elm_a)->field.r_prev;					\
	(elm_a)->field.r_prev = (elm_b)->field.r_prev;			\
	(elm_b)->field.r_prev = t;					\
} while (0)

#define qr_remove(elm, field) do {					\
	(elm)->field.r_prev->field.r_next = (elm)->field.r_next;	\
	(elm)->field.r_next->field.r_prev = (elm)->field.r_prev;	\
	(elm)->field.r_next = (elm);					\
	(elm)->field.r_prev = (elm);					\
} while (0)

#define qr_split(elm_a, elm_b, field) do {				\
	void	*t = (elm_a)->field.r_prev;				\
	(elm_a)->field.r_prev = (elm_b)->field.r_prev;			\
	(elm_b)->field.r_prev = t;					\
	(elm_a)->field.r_prev->field.r_next = (elm_a);			\
	(elm_b)->field.r_prev->field.r_next = (elm_b);			\
} while (0)
