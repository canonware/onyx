/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

/*
 * Stack definitions.
 */
#define qs_head(a_type)							\
struct {								\
	a_type *qsh_top;		/* Top element. */		\
}

#define qs_head_initializer(a_head)					\
	{NULL}

#define qs_elm(a_type)							\
struct {								\
	a_type *qse_down;						\
}
 
/*
 * Stack functions.
 */
#define qs_new(a_head) do {						\
	(a_head)->qsh_top = NULL;					\
} while (0)

#define qs_elm_new(a_elm, a_field)

#define qs_top(a_head)	((a_head)->qsh_top)

#define qs_down(a_elm, a_field)	((a_elm)->a_field.qse_down)

#define qs_push(a_head, a_elm, a_field) do {				\
	qs_down((a_elm), a_field) = qs_top(a_head);			\
	qs_top(a_head) = (a_elm);					\
} while (0)

#define qs_under_push(a_qselm, a_elm, a_field) do {			\
	qs_down((a_elm), a_field) = qs_down((a_qselm), a_field);	\
	qs_down((a_qselm), a_field) = (a_elm);				\
} while (0)

#define qs_pop(a_head, a_field) do {					\
	qs_top(a_head) = qs_down(qs_top(a_head), a_field);		\
} while (0)

#define qs_foreach(a_var, a_head, a_field)				\
	for ((a_var) = qs_top(a_head); (a_var) != NULL;			\
	    (a_var) = qs_down((a_var), a_field))
