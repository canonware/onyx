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

#define qs_push(head, elm, field) do {					\
	(elm)->field.qse_down = (head)->qsh_top;			\
	(head)->qsh_top = (elm);					\
} while (0)

#define qs_under_push(qselm, elm, field) do {				\
	(elm)->field.qse_down = (qselm)->field.qse_down;		\
	(qselm)->field.qse_down = (elm);				\
} while (0)

#define qs_pop(head, field) do {					\
	(head)->qsh_top = (head)->qsh_top->field.qse_down;		\
} while (0)

#define qs_top(head)	((head)->qsh_top)

#define qs_down(elm, field)	((elm)->field.qse_down)

#define qs_foreach(var, head, field)					\
	for ((var) = (head)->qsh_top; (var) != NULL;			\
	    (var) = (var)->field.qse_down)
