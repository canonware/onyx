/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libstil/libstil.h"
#include "../include/libstil/stila_l.h"

struct cw_gcdict_entry {
	cw_stiln_t	stiln;
	cw_op_t		*op_f;
};

#define ENTRY(name)	{STILN_##name, gcdict_##name}

/*
 * Array of operators in gcdict.
 */
static const struct cw_gcdict_entry gcdict_ops[] = {
	ENTRY(active),
	ENTRY(collect),
	ENTRY(collections),
	ENTRY(current),
	ENTRY(dump),
	ENTRY(maximum),
	ENTRY(new),
	ENTRY(period),
	ENTRY(setactive),
	ENTRY(setperiod),
	ENTRY(setthreshold),
	ENTRY(sum),
	ENTRY(threshold)
};

/*
 * This is a global dictionary, but it should never be written to except by the
 * GC thread, so don't bother locking it.
 */
void
gcdict_l_populate(cw_stilo_t *a_dict, cw_stila_t *a_stila)
{
	cw_uint32_t	i;
	cw_stilo_t	name, value;

#define	NEXTRA	0
#define NENTRIES							\
	(sizeof(gcdict_ops) / sizeof(struct cw_gcdict_entry))

	stilo_dict_new(a_dict, stila_l_stil_get(a_stila), FALSE, NENTRIES +
	    NEXTRA);

	for (i = 0; i < NENTRIES; i++) {
		stilo_name_new(&name, stila_l_stil_get(a_stila),
		    stiln_str(gcdict_ops[i].stiln),
		    stiln_len(gcdict_ops[i].stiln), TRUE);
		stilo_operator_new(&value, gcdict_ops[i].op_f,
		    gcdict_ops[i].stiln);
		stilo_attrs_set(&value, STILOA_EXECUTABLE);

		stilo_dict_def(a_dict, stila_l_stil_get(a_stila), &name,
		    &value);
	}

#ifdef _LIBSTIL_DBG
	if (stilo_dict_count(a_dict) != NENTRIES + NEXTRA) {
		_cw_out_put_e("stilo_dict_count(a_dict) != NENTRIES"
		    " ([i] != [i])\n", stilo_dict_count(a_dict), NENTRIES +
		    NEXTRA);
		_cw_error("Adjust NENTRIES");
	}
#endif
#undef NENTRIES
}

void
gcdict_active(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*active;

	ostack = stilo_thread_ostack_get(a_thread);
	stila = stil_stila_get(stilo_thread_stil_get(a_thread));
	active = stilo_stack_push(ostack);
	stilo_boolean_new(active, stila_active_get(stila));
}

void
gcdict_collect(cw_stilo_t *a_thread)
{
	stila_collect(stil_stila_get(stilo_thread_stil_get(a_thread)));
}

void
gcdict_collections(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*collections;

	ostack = stilo_thread_ostack_get(a_thread);
	stila = stil_stila_get(stilo_thread_stil_get(a_thread));
	collections = stilo_stack_push(ostack);
	stilo_integer_new(collections, stila_collections_get(stila));
}

void
gcdict_current(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*current, tstilo;
	cw_stiloi_t	count, mark, sweep;

	ostack = stilo_thread_ostack_get(a_thread);
	stila = stil_stila_get(stilo_thread_stil_get(a_thread));
	stila_current_get(stila, &count, &mark, &sweep);
	current = stilo_stack_push(ostack);
	stilo_array_new(current, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread), 3);

	stilo_integer_new(&tstilo, count);
	stilo_array_el_set(current, &tstilo, 0);
	stilo_integer_set(&tstilo, mark);
	stilo_array_el_set(current, &tstilo, 1);
	stilo_integer_set(&tstilo, sweep);
	stilo_array_el_set(current, &tstilo, 2);
}

void
gcdict_dump(cw_stilo_t *a_thread)
{
	stila_dump(stil_stila_get(stilo_thread_stil_get(a_thread)), a_thread);
}

void
gcdict_maximum(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*maximum, tstilo;
	cw_stiloi_t	count, mark, sweep;

	ostack = stilo_thread_ostack_get(a_thread);
	stila = stil_stila_get(stilo_thread_stil_get(a_thread));
	stila_maximum_get(stila, &count, &mark, &sweep);
	maximum = stilo_stack_push(ostack);
	stilo_array_new(maximum, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread), 3);

	stilo_integer_new(&tstilo, count);
	stilo_array_el_set(maximum, &tstilo, 0);
	stilo_integer_set(&tstilo, mark);
	stilo_array_el_set(maximum, &tstilo, 1);
	stilo_integer_set(&tstilo, sweep);
	stilo_array_el_set(maximum, &tstilo, 2);
}

void
gcdict_new(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*new;

	ostack = stilo_thread_ostack_get(a_thread);
	stila = stil_stila_get(stilo_thread_stil_get(a_thread));
	new = stilo_stack_push(ostack);
	stilo_integer_new(new, stila_new_get(stila));
}

void
gcdict_period(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*period;

	ostack = stilo_thread_ostack_get(a_thread);
	stila = stil_stila_get(stilo_thread_stil_get(a_thread));
	period = stilo_stack_push(ostack);
	stilo_integer_new(period, stila_period_get(stila));
}

void
gcdict_setactive(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*active;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(active, ostack, a_thread);
	if (stilo_type_get(active) != STILOT_BOOLEAN) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stila_active_set(stil_stila_get(stilo_thread_stil_get(a_thread)),
	    stilo_boolean_get(active));

	stilo_stack_pop(ostack);
}

void
gcdict_setperiod(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*period;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(period, ostack, a_thread);
	if (stilo_type_get(period) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	if (stilo_integer_get(period) < 0 || stilo_integer_get(period) >
	    UINT_MAX) {
		stilo_thread_error(a_thread, STILO_THREADE_LIMITCHECK);
		    return;
	}

	stila_period_set(stil_stila_get(stilo_thread_stil_get(a_thread)),
	    stilo_integer_get(period));

	stilo_stack_pop(ostack);
}

void
gcdict_setthreshold(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*threshold;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(threshold, ostack, a_thread);
	if (stilo_type_get(threshold) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	if (stilo_integer_get(threshold) < 0 || stilo_integer_get(threshold) >
	    UINT_MAX) {
		stilo_thread_error(a_thread, STILO_THREADE_LIMITCHECK);
		    return;
	}

	stila_threshold_set(stil_stila_get(stilo_thread_stil_get(a_thread)),
	    stilo_integer_get(threshold));

	stilo_stack_pop(ostack);
}

void
gcdict_sum(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*sum, tstilo;
	cw_stiloi_t	count, mark, sweep;

	ostack = stilo_thread_ostack_get(a_thread);
	stila = stil_stila_get(stilo_thread_stil_get(a_thread));
	stila_sum_get(stila, &count, &mark, &sweep);
	sum = stilo_stack_push(ostack);
	stilo_array_new(sum, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread), 3);

	stilo_integer_new(&tstilo, count);
	stilo_array_el_set(sum, &tstilo, 0);
	stilo_integer_set(&tstilo, mark);
	stilo_array_el_set(sum, &tstilo, 1);
	stilo_integer_set(&tstilo, sweep);
	stilo_array_el_set(sum, &tstilo, 2);
}

void
gcdict_threshold(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*threshold;

	ostack = stilo_thread_ostack_get(a_thread);
	stila = stil_stila_get(stilo_thread_stil_get(a_thread));
	threshold = stilo_stack_push(ostack);
	stilo_integer_new(threshold, stila_threshold_get(stila));
}
