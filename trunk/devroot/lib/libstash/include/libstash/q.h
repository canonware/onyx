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
 * This file defines three types of data structures: stacks, queues, and rings.
 *
 * A stack is headed by a single forward pointer. The elements are singly linked
 * for minimum space and pointer manipulation overhead at the expense of O(n)
 * removal for arbitrary elements. New elements can be added to the stack after
 * an existing element or at the top of the stack.  Elements being removed from
 * the top of the stack should use the explicit macro for this purpose for
 * optimum efficiency. A stack may only be traversed in the forward direction.
 * Stacks are ideal for applications with large datasets and few or no removals
 * or for implementing a LIFO queue.
 *
 * A queue is headed by a pair of pointers, one to the head of the queue and the
 * other to the tail of the queue. The elements are singly linked for minimum
 * space and pointer manipulation overhead at the expense of O(n) removal for
 * arbitrary elements. New elements can be added to the queue after an existing
 * element, at the head of the queue, or at the end of the queue. Elements being
 * removed from the head of the queue should use the explicit macro for this
 * purpose for optimum efficiency.  A queue may only be traversed in the forward
 * direction.  Queues are ideal for applications with large datasets and few or
 * no removals or for implementing a FIFO queue.
 *
 * A ring has no header.  All elments are equal.  Rings are doubly linked, and
 * can be traversed and operated on in both directions.  Rings can be cut, given
 * two elements in the same ring, or spliced, given two separate rings.  Rings
 * are very efficient and versatile and can be used in virtually every situation
 * that require doubly linked lists.
 *
 *			qs	qq	qr
 * _head		+	+	-
 * _head_initializer	+	+	-
 * _entry		+	+	+
 * _new			+	+	+
 * _empty		+	+	-
 * _first		+	+	-
 * _next		+	+	+
 * _prev		-	-	+
 * _last		-	+	-
 * _foreach		+	+	+
 * _insert_head		+	+	-
 * _insert_before	-	-	+
 * _insert_after	+	+	+
 * _insert_tail		-	+	-
 * _remove_head		+	+	-
 * _remove		+	+	+
 * _meld		-	-	+
 * _split		-	-	+
 *
 */

/*
 * Stack definitions.
 */
#define qs_head(type)							\
struct {								\
	type *qsh_first;	/* first element */			\
}

#define qs_head_initializer(head)					\
	{ NULL }
 
#define qs_entry(type)							\
struct {								\
	type *qse_next;		/* next element */			\
}
 
/*
 * Stack functions.
 */
#define	qs_empty(head)	((head)->qsh_first == NULL)

#define	qs_first(head)	((head)->qsh_first)

#define qs_foreach(var, head, field)					\
	for((var) = (head)->qsh_first; (var); (var) = (var)->field.qse_next)

#define qs_new(head) {							\
	(head)->qsh_first = NULL;					\
}

#define qs_insert_after(qselm, elm, field) do {				\
	(elm)->field.qse_next = (qselm)->field.qse_next;		\
	(qselm)->field.qse_next = (elm);				\
} while (0)

#define qs_insert_head(head, elm, field) do {				\
	(elm)->field.qse_next = (head)->qsh_first;			\
	(head)->qsh_first = (elm);					\
} while (0)

#define qs_next(elm, field)	((elm)->field.qse_next)

#define qs_remove_head(head, field) do {				\
	(head)->qsh_first = (head)->qsh_first->field.qse_next;		\
} while (0)

#define qs_remove(head, elm, type, field) do {				\
	if ((head)->qsh_first == (elm)) {				\
		qs_remove_head((head), field);				\
	}								\
	else {								\
		type *curelm = (head)->qsh_first;			\
		while( curelm->field.qse_next != (elm) )		\
			curelm = curelm->field.qse_next;		\
		curelm->field.qse_next =				\
		    curelm->field.qse_next->field.qse_next;		\
	}								\
} while (0)

/*
 * Queue definitions.
 */
#define qq_head(type)							\
struct {								\
	type *qqh_first;	/* first element */			\
	type **qqh_last;	/* addr of last next element */		\
}

#define qq_head_initializer(head)					\
	{ NULL, &(head).qqh_first }

#define qq_entry(type)							\
struct {								\
	type *qqe_next;	/* next element */				\
}

/*
 * Queue functions.
 */
#define qq_empty(head) ((head)->qqh_first == NULL)

#define	qq_init(head) do {						\
	(head)->qqh_first = NULL;					\
	(head)->qqh_last = &(head)->qqh_first;				\
} while (0)

#define qq_first(head)	((head)->qqh_first)
#define qq_last(head)	(*(head)->qqh_last)

#define qq_foreach(var, head, field)					\
	for((var) = (head)->qqh_first; (var); (var) = (var)->field.qqe_next)

#define qq_insert_head(head, elm, field) do {				\
	if (((elm)->field.qqe_next = (head)->qqh_first) == NULL)	\
		(head)->qqh_last = &(elm)->field.qqe_next;		\
	(head)->qqh_first = (elm);					\
} while (0)

#define qq_insert_tail(head, elm, field) do {				\
	(elm)->field.qqe_next = NULL;					\
	*(head)->qqh_last = (elm);					\
	(head)->qqh_last = &(elm)->field.qqe_next;			\
} while (0)

#define qq_insert_after(head, qqelm, elm, field) do {			\
	if (((elm)->field.qqe_next = (qqelm)->field.qqe_next) == NULL)	\
		(head)->qqh_last = &(elm)->field.qqe_next;		\
	(qqelmy)->field.qqe_next = (elm);				\
} while (0)

#define qq_next(elm, field)	((elm)->field.qqe_next)

#define qq_remove_head(head, field) do {				\
	if (((head)->qqh_first =					\
	     (head)->qqh_first->field.qqe_next) == NULL)		\
		(head)->qqh_last = &(head)->qqh_first;			\
} while (0)

#define qq_remove_head_until(head, elm, field) do {			\
	if (((head)->qqh_first = (elm)->field.qqe_next) == NULL)	\
		(head)->qqh_last = &(head)->qqh_first;			\
} while (0)

#define qq_remove(head, elm, type, field) do {				\
	if ((head)->qqh_first == (elm)) {				\
		qq_remove_head(head, field);				\
	} else {							\
		type *curelm = (head)->qqh_first;			\
		while( curelm->field.qqe_next != (elm) )		\
			curelm = curelm->field.qqe_next;		\
		if((curelm->field.qqe_next =				\
		    curelm->field.qqe_next->field.qqe_next) == NULL)	\
			(head)->qqh_last = &(curelm)->field.qqe_next;	\
	}								\
} while (0)

/*
 * Ring definitions.
 */
#define qr_entry(type)							\
struct {								\
	type *qre_next;			/* Next element. */		\
	type *qre_prev;			/* Previous element. */		\
}

/*
 * Ring functions.
 */
#define qr_new(elm, field) do {						\
	(elm)->field.qre_next = (elm);					\
	(elm)->field.qre_prev = (elm);					\
} while (0)

#define qr_next(elm, field) ((elm)->field.qre_next)

#define qr_prev(elm, field) ((elm)->field.qre_prev)

#define qr_foreach(var, elm, field)					\
	for ((var) = (elm);						\
	    (var) != NULL;						\
	    (var) = (((var)->field.qre_next != (elm))			\
	    ? (elm)->field.qre_next : NULL)))

#define qr_foreach_reverse(var, elm, field)				\
	for ((var) = (elm);						\
	    (var) != NULL;						\
	    (var) = (((var)->field.qre_prev != (elm))			\
	    ? (elm)->field.qre_prev : NULL)))

#define qr_insert_before(qrelm, elm, field) do {			\
	(elm)->field.qre_prev = (qrelm)->field.qre_prev;		\
	(elm)->field.qre_next = (qrelm);				\
	(elm)->field.qre_prev->field.qre_next = (elm);			\
	(qrelm)->field.qre_prev = (elm);				\
} while (0)

#define qr_insert_after(qrelm, elm, field) do {				\
	(elm)->field.qre_next = (qrelm)->field.qre_next;		\
	(elm)->field.qre_prev = (qrelm);				\
	(elm)->field.qre_next->field.qre_prev = (elm);			\
	(qrelm)->field.qre_next = (elm);				\
} while (0)

#define qr_meld(elm_a, elm_b, field) do {				\
	void	*t;							\
	(elm_a)->field.qre_prev->field.qre_next = (elm_b);		\
	(elm_b)->field.qre_prev->field.qre_next = (elm_a);		\
	t = (elm_a)->field.qre_prev;					\
	(elm_a)->field.qre_prev = (elm_b)->field.qre_prev;		\
	(elm_b)->field.qre_prev = t;					\
} while (0)

#define qr_remove(elm, field) do {					\
	(elm)->field.qre_prev->field.qre_next = (elm)->field.qre_next;	\
	(elm)->field.qre_next->field.qre_prev = (elm)->field.qre_prev;	\
	(elm)->field.qre_next = (elm);					\
	(elm)->field.qre_prev = (elm);					\
} while (0)

#define qr_split(elm_a, elm_b, field) do {				\
	void	*t = (elm_a)->field.qre_prev;				\
	(elm_a)->field.qre_prev = (elm_b)->field.qre_prev;		\
	(elm_b)->field.qre_prev = t;					\
	(elm_a)->field.qre_prev->field.qre_next = (elm_a);		\
	(elm_b)->field.qre_prev->field.qre_next = (elm_b);		\
} while (0)
