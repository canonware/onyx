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

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxa_l.h"

struct cw_gcdict_entry {
	cw_nxn_t	nxn;
	cw_op_t		*op_f;
};

#define ENTRY(name)	{NXN_##name, gcdict_##name}

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
gcdict_l_populate(cw_nxo_t *a_dict, cw_nxa_t *a_nxa)
{
	cw_uint32_t	i;
	cw_nxo_t	name, value;

#define	NEXTRA	0
#define NENTRIES							\
	(sizeof(gcdict_ops) / sizeof(struct cw_gcdict_entry))

	nxo_dict_new(a_dict, nxa_l_nx_get(a_nxa), FALSE, NENTRIES + NEXTRA);

	for (i = 0; i < NENTRIES; i++) {
		nxo_name_new(&name, nxa_l_nx_get(a_nxa),
		    nxn_str(gcdict_ops[i].nxn), nxn_len(gcdict_ops[i].nxn),
		    TRUE);
		nxo_operator_new(&value, gcdict_ops[i].op_f, gcdict_ops[i].nxn);
		nxo_attrs_set(&value, NXOA_EXECUTABLE);

		nxo_dict_def(a_dict, nxa_l_nx_get(a_nxa), &name, &value);
	}

#ifdef _LIBONYX_DBG
	if (nxo_dict_count(a_dict) != NENTRIES + NEXTRA) {
		_cw_out_put_e("nxo_dict_count(a_dict) != NENTRIES"
		    " ([i] != [i])\n", nxo_dict_count(a_dict), NENTRIES +
		    NEXTRA);
		_cw_error("Adjust NENTRIES");
	}
#endif
#undef NENTRIES
}

void
gcdict_active(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxa_t	*nxa;
	cw_nxo_t	*active;

	ostack = nxo_thread_ostack_get(a_thread);
	nxa = nx_nxa_get(nxo_thread_nx_get(a_thread));
	active = nxo_stack_push(ostack);
	nxo_boolean_new(active, nxa_active_get(nxa));
}

void
gcdict_collect(cw_nxo_t *a_thread)
{
	nxa_collect(nx_nxa_get(nxo_thread_nx_get(a_thread)));
}

void
gcdict_collections(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxa_t	*nxa;
	cw_nxo_t	*collections;

	ostack = nxo_thread_ostack_get(a_thread);
	nxa = nx_nxa_get(nxo_thread_nx_get(a_thread));
	collections = nxo_stack_push(ostack);
	nxo_integer_new(collections, nxa_collections_get(nxa));
}

void
gcdict_current(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxa_t	*nxa;
	cw_nxo_t	*current, tnxo;
	cw_nxoi_t	count, mark, sweep;

	ostack = nxo_thread_ostack_get(a_thread);
	nxa = nx_nxa_get(nxo_thread_nx_get(a_thread));
	nxa_current_get(nxa, &count, &mark, &sweep);
	current = nxo_stack_push(ostack);
	nxo_array_new(current, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), 3);

	nxo_integer_new(&tnxo, count);
	nxo_array_el_set(current, &tnxo, 0);
	nxo_integer_set(&tnxo, mark);
	nxo_array_el_set(current, &tnxo, 1);
	nxo_integer_set(&tnxo, sweep);
	nxo_array_el_set(current, &tnxo, 2);
}

void
gcdict_dump(cw_nxo_t *a_thread)
{
	nxa_dump(nx_nxa_get(nxo_thread_nx_get(a_thread)), a_thread);
}

void
gcdict_maximum(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxa_t	*nxa;
	cw_nxo_t	*maximum, tnxo;
	cw_nxoi_t	count, mark, sweep;

	ostack = nxo_thread_ostack_get(a_thread);
	nxa = nx_nxa_get(nxo_thread_nx_get(a_thread));
	nxa_maximum_get(nxa, &count, &mark, &sweep);
	maximum = nxo_stack_push(ostack);
	nxo_array_new(maximum, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), 3);

	nxo_integer_new(&tnxo, count);
	nxo_array_el_set(maximum, &tnxo, 0);
	nxo_integer_set(&tnxo, mark);
	nxo_array_el_set(maximum, &tnxo, 1);
	nxo_integer_set(&tnxo, sweep);
	nxo_array_el_set(maximum, &tnxo, 2);
}

void
gcdict_new(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxa_t	*nxa;
	cw_nxo_t	*new;

	ostack = nxo_thread_ostack_get(a_thread);
	nxa = nx_nxa_get(nxo_thread_nx_get(a_thread));
	new = nxo_stack_push(ostack);
	nxo_integer_new(new, nxa_new_get(nxa));
}

void
gcdict_period(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxa_t	*nxa;
	cw_nxo_t	*period;

	ostack = nxo_thread_ostack_get(a_thread);
	nxa = nx_nxa_get(nxo_thread_nx_get(a_thread));
	period = nxo_stack_push(ostack);
	nxo_integer_new(period, nxa_period_get(nxa));
}

void
gcdict_setactive(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*active;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(active, ostack, a_thread);
	if (nxo_type_get(active) != NXOT_BOOLEAN) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxa_active_set(nx_nxa_get(nxo_thread_nx_get(a_thread)),
	    nxo_boolean_get(active));

	nxo_stack_pop(ostack);
}

void
gcdict_setperiod(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*period;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(period, ostack, a_thread);
	if (nxo_type_get(period) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	if (nxo_integer_get(period) < 0 || nxo_integer_get(period) > UINT_MAX) {
		nxo_thread_error(a_thread, NXO_THREADE_LIMITCHECK);
		return;
	}

	nxa_period_set(nx_nxa_get(nxo_thread_nx_get(a_thread)),
	    nxo_integer_get(period));

	nxo_stack_pop(ostack);
}

void
gcdict_setthreshold(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*threshold;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(threshold, ostack, a_thread);
	if (nxo_type_get(threshold) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	if (nxo_integer_get(threshold) < 0 || nxo_integer_get(threshold) >
	    UINT_MAX) {
		nxo_thread_error(a_thread, NXO_THREADE_LIMITCHECK);
		return;
	}

	nxa_threshold_set(nx_nxa_get(nxo_thread_nx_get(a_thread)),
	    nxo_integer_get(threshold));

	nxo_stack_pop(ostack);
}

void
gcdict_sum(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxa_t	*nxa;
	cw_nxo_t	*sum, tnxo;
	cw_nxoi_t	count, mark, sweep;

	ostack = nxo_thread_ostack_get(a_thread);
	nxa = nx_nxa_get(nxo_thread_nx_get(a_thread));
	nxa_sum_get(nxa, &count, &mark, &sweep);
	sum = nxo_stack_push(ostack);
	nxo_array_new(sum, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), 3);

	nxo_integer_new(&tnxo, count);
	nxo_array_el_set(sum, &tnxo, 0);
	nxo_integer_set(&tnxo, mark);
	nxo_array_el_set(sum, &tnxo, 1);
	nxo_integer_set(&tnxo, sweep);
	nxo_array_el_set(sum, &tnxo, 2);
}

void
gcdict_threshold(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxa_t	*nxa;
	cw_nxo_t	*threshold;

	ostack = nxo_thread_ostack_get(a_thread);
	nxa = nx_nxa_get(nxo_thread_nx_get(a_thread));
	threshold = nxo_stack_push(ostack);
	nxo_integer_new(threshold, nxa_threshold_get(nxa));
}
