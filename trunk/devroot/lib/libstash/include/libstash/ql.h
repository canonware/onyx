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
 * List definitions.
 */
#define ql_head(type)							\
struct {								\
	type *qlh_first;						\
}

#define ql_head_initializer(head)					\
	{NULL}

#define ql_elm(type)	qr_elm(type)

/*
 * List functions.
 */
#define ql_new(head) do {						\
	(head)->qlh_first = NULL;					\
} while (0)

#define ql_elm_new(elm, field)	qr_new((elm), field)

#define ql_first(head) ((head)->qlh_first)

#define ql_last(head, field)						\
	((ql_first(head) != NULL) ? qr_prev(ql_first(head), field) : NULL)

#define ql_next(head, elm, field)					\
	((ql_last(head, field) != (elm)) ? qr_next((elm), field) : NULL)

#define ql_prev(head, elm, field)					\
	((ql_first(head) != (elm)) ? qr_prev((elm), field) : NULL)

#define ql_before_insert(head, qlelm, elm, field) do {			\
	qr_before_insert((qlelm), (elm), field);			\
	if (ql_first(head) == (qlelm))					\
		ql_first(head) = (elm);					\
} while (0)

#define ql_after_insert(qlelm, elm, field)				\
	qr_after_insert((qlelm), (elm), field)

#define ql_head_insert(head, elm, field) do {				\
	if (ql_first(head) != NULL)					\
		qr_before_insert(ql_first(head), (elm), field);		\
	ql_first(head) = (elm);						\
} while (0)

#define ql_tail_insert(head, elm, field) do {				\
	if (ql_first(head) != NULL) {					\
		qr_before_insert(ql_first(head), (elm), field);		\
	}								\
	ql_first(head) = qr_next((elm), field);				\
} while (0)

#define ql_remove(head, elm, field) do {				\
	if (ql_first(head) == (elm)) {					\
		ql_first(head) = qr_next(ql_first(head), field);	\
	}								\
	if (ql_first(head) != (elm))					\
		qr_remove((elm), field);				\
	else								\
		ql_first(head) = NULL;					\
} while (0)

#define ql_head_remove(head, type, field) do {				\
	type	*t = ql_first(head);					\
	ql_remove((head), t, field);					\
} while (0)

#define ql_tail_remove(head, type, field) do {				\
	type	*t = ql_last(head, field);				\
	ql_remove((head), t, field);					\
} while (0)

#define ql_foreach(var, head, field)					\
	qr_foreach((var), ql_first(head), field)

#define ql_reverse_foreach(var, head, field)				\
	qr_reverse_foreach((var), ql_first(head), field)
