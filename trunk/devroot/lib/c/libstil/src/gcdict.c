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
gcdict_active(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*active;

	ostack = stilt_ostack_get(a_stilt);
	stila = stil_stila_get(stilt_stil_get(a_stilt));
	active = stils_push(ostack);
	stilo_boolean_new(active, stila_active_get(stila));
}

void
gcdict_collect(cw_stilt_t *a_stilt)
{
	stila_collect(stil_stila_get(stilt_stil_get(a_stilt)));
}

void
gcdict_collections(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*collections;

	ostack = stilt_ostack_get(a_stilt);
	stila = stil_stila_get(stilt_stil_get(a_stilt));
	collections = stils_push(ostack);
	stilo_integer_new(collections, stila_collections_get(stila));
}

void
gcdict_current(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*current, tstilo;
	cw_stiloi_t	count, mark, sweep;

	ostack = stilt_ostack_get(a_stilt);
	stila = stil_stila_get(stilt_stil_get(a_stilt));
	stila_current_get(stila, &count, &mark, &sweep);
	current = stils_push(ostack);
	stilo_array_new(current, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt), 3);

	stilo_integer_new(&tstilo, count);
	stilo_array_el_set(current, &tstilo, 0);
	stilo_integer_set(&tstilo, mark);
	stilo_array_el_set(current, &tstilo, 1);
	stilo_integer_set(&tstilo, sweep);
	stilo_array_el_set(current, &tstilo, 2);
}

void
gcdict_dump(cw_stilt_t *a_stilt)
{
	stila_dump(stil_stila_get(stilt_stil_get(a_stilt)), a_stilt);
}

void
gcdict_maximum(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*maximum, tstilo;
	cw_stiloi_t	count, mark, sweep;

	ostack = stilt_ostack_get(a_stilt);
	stila = stil_stila_get(stilt_stil_get(a_stilt));
	stila_maximum_get(stila, &count, &mark, &sweep);
	maximum = stils_push(ostack);
	stilo_array_new(maximum, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt), 3);

	stilo_integer_new(&tstilo, count);
	stilo_array_el_set(maximum, &tstilo, 0);
	stilo_integer_set(&tstilo, mark);
	stilo_array_el_set(maximum, &tstilo, 1);
	stilo_integer_set(&tstilo, sweep);
	stilo_array_el_set(maximum, &tstilo, 2);
}

void
gcdict_new(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*new;

	ostack = stilt_ostack_get(a_stilt);
	stila = stil_stila_get(stilt_stil_get(a_stilt));
	new = stils_push(ostack);
	stilo_integer_new(new, stila_new_get(stila));
}

void
gcdict_period(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*period;

	ostack = stilt_ostack_get(a_stilt);
	stila = stil_stila_get(stilt_stil_get(a_stilt));
	period = stils_push(ostack);
	stilo_integer_new(period, stila_period_get(stila));
}

void
gcdict_setactive(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*active;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(active, ostack, a_stilt);
	if (stilo_type_get(active) != STILOT_BOOLEAN) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stila_active_set(stil_stila_get(stilt_stil_get(a_stilt)),
	    stilo_boolean_get(active));

	stils_pop(ostack);
}

void
gcdict_setperiod(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*period;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(period, ostack, a_stilt);
	if (stilo_type_get(period) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	if (stilo_integer_get(period) < 0 || stilo_integer_get(period) >
	    UINT_MAX) {
		stilt_error(a_stilt, STILTE_RANGECHECK);
		    return;
	}

	stila_period_set(stil_stila_get(stilt_stil_get(a_stilt)),
	    stilo_integer_get(period));

	stils_pop(ostack);
}

void
gcdict_setthreshold(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*threshold;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(threshold, ostack, a_stilt);
	if (stilo_type_get(threshold) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	if (stilo_integer_get(threshold) < 0 || stilo_integer_get(threshold) >
	    UINT_MAX) {
		stilt_error(a_stilt, STILTE_RANGECHECK);
		    return;
	}

	stila_threshold_set(stil_stila_get(stilt_stil_get(a_stilt)),
	    stilo_integer_get(threshold));

	stils_pop(ostack);
}

void
gcdict_sum(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*sum, tstilo;
	cw_stiloi_t	count, mark, sweep;

	ostack = stilt_ostack_get(a_stilt);
	stila = stil_stila_get(stilt_stil_get(a_stilt));
	stila_sum_get(stila, &count, &mark, &sweep);
	sum = stils_push(ostack);
	stilo_array_new(sum, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt), 3);

	stilo_integer_new(&tstilo, count);
	stilo_array_el_set(sum, &tstilo, 0);
	stilo_integer_set(&tstilo, mark);
	stilo_array_el_set(sum, &tstilo, 1);
	stilo_integer_set(&tstilo, sweep);
	stilo_array_el_set(sum, &tstilo, 2);
}

void
gcdict_threshold(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stila_t	*stila;
	cw_stilo_t	*threshold;

	ostack = stilt_ostack_get(a_stilt);
	stila = stil_stila_get(stilt_stil_get(a_stilt));
	threshold = stils_push(ostack);
	stilo_integer_new(threshold, stila_threshold_get(stila));
}
