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
 * Stack definitions.
 */
#define qs_head(type)							\
struct {								\
	type *qsh_top;		/* Top element. */			\
}

#define qs_head_initializer(head)					\
	{NULL}

#define qs_elm(type)							\
struct {								\
	type *qse_down;							\
}
 
/*
 * Stack functions.
 */
#define qs_new(head) do {						\
	(head)->qsh_top = NULL;						\
} while (0)

#define qs_top(head)	((head)->qsh_top)

#define qs_down(elm, field)	((elm)->field.qse_down)

#define qs_push(head, elm, field) do {					\
	qs_down((elm), field) = qs_top(head);				\
	qs_top(head) = (elm);						\
} while (0)

#define qs_under_push(qselm, elm, field) do {				\
	qs_down((elm), field) = qs_down((qselm), field);		\
	qs_down((qselm), field) = (elm);				\
} while (0)

#define qs_pop(head, field) do {					\
	qs_top(head) = qs_down(qs_top(head), field);			\
} while (0)

#define qs_foreach(var, head, field)					\
	for ((var) = qs_top(head); (var) != NULL;			\
	    (var) = qs_down((var), field))
