/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

/*
 * Ring definitions.
 */
#define qr_elm(type)							\
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

#define qr_before_insert(qrelm, elm, field) do {			\
	(elm)->field.qre_prev = (qrelm)->field.qre_prev;		\
	(elm)->field.qre_next = (qrelm);				\
	(elm)->field.qre_prev->field.qre_next = (elm);			\
	(qrelm)->field.qre_prev = (elm);				\
} while (0)

#define qr_after_insert(qrelm, elm, field) do {				\
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

/*
 * qr_meld() and qr_split() are functionally equivalent, so there's no need to
 * have two copies of the code.
 */
#define qr_split(elm_a, elm_b, field)					\
	qr_meld((elm_a), (elm_b), field)

#define qr_remove(elm, field) do {					\
	(elm)->field.qre_prev->field.qre_next = (elm)->field.qre_next;	\
	(elm)->field.qre_next->field.qre_prev = (elm)->field.qre_prev;	\
	(elm)->field.qre_next = (elm);					\
	(elm)->field.qre_prev = (elm);					\
} while (0)

#define qr_foreach(var, elm, field)					\
	for ((var) = (elm);						\
	    (var) != NULL;						\
	    (var) = (((var)->field.qre_next != (elm))			\
	    ? (var)->field.qre_next : NULL))

#define qr_reverse_foreach(var, elm, field)				\
	for ((var) = ((elm) != NULL) ? qr_prev(elm, field) : NULL;	\
	    (var) != NULL;						\
	    (var) = (((var) != (elm))					\
	    ? (var)->field.qre_prev : NULL))
