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
	ENTRY(period),
	ENTRY(setactive),
	ENTRY(setperiod),
	ENTRY(setthreshold),
	ENTRY(stats),
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
	cw_nx_t		*nx;

#define	NEXTRA	0
#define NENTRIES							\
	(sizeof(gcdict_ops) / sizeof(struct cw_gcdict_entry))

	nx = nxa_l_nx_get(a_nxa);
	nxo_dict_new(a_dict, nx, FALSE, NENTRIES + NEXTRA);

	for (i = 0; i < NENTRIES; i++) {
		nxo_name_new(&name, nx, nxn_str(gcdict_ops[i].nxn),
		    nxn_len(gcdict_ops[i].nxn), TRUE);
		nxo_operator_new(&value, gcdict_ops[i].op_f, gcdict_ops[i].nxn);
		nxo_attr_set(&value, NXOA_EXECUTABLE);

		nxo_dict_def(a_dict, nx, &name, &value);
	}

#ifdef _CW_DBG
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

/*
 * Create a stats array:
 *
 * stats -->   [
 *               collections
 *               new
 * current -->   [ mark sweep ]
 * maximum -->   [ mark sweep ]
 *     sum -->   [ mark sweep ]
 *             ]
 */
void
gcdict_stats(cw_nxo_t *a_thread)
{
	cw_nx_t		*nx;
	cw_nxa_t	*nxa;
	cw_bool_t	currentlocking;
	cw_nxo_t	*ostack, *tstack;
	cw_nxo_t	*stats, *nxo, *tnxo;
	cw_nxoi_t	collections, new;
	cw_nxoi_t	cmark, csweep;
	cw_nxoi_t	mmark, msweep;
	cw_nxoi_t	smark, ssweep;

	nx = nxo_thread_nx_get(a_thread);
	nxa = nx_nxa_get(nx);
	currentlocking = nxo_thread_currentlocking(a_thread);
	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);

	nxo = nxo_stack_push(tstack);
	tnxo = nxo_stack_push(tstack);

	/* Get stats. */
	nxa_stats_get(nxa, &collections, &new,
	    &cmark, &csweep,
	    &mmark, &msweep,
	    &smark, &ssweep);

	/* Create the main array. */
	stats = nxo_stack_push(ostack);
	nxo_array_new(stats, nx, currentlocking, 5);

	/* collections. */
	nxo_integer_new(nxo, collections);
	nxo_array_el_set(stats, nxo, 0);

	/* new. */
	nxo_integer_new(nxo, new);
	nxo_array_el_set(stats, nxo, 1);

	/* current. */
	nxo_array_new(nxo, nx, currentlocking, 2);
	nxo_integer_new(tnxo, cmark);
	nxo_array_el_set(nxo, tnxo, 0);
	nxo_integer_new(tnxo, csweep);
	nxo_array_el_set(nxo, tnxo, 1);
	nxo_array_el_set(stats, nxo, 2);

	/* maximum. */
	nxo_array_new(nxo, nx, currentlocking, 2);
	nxo_integer_new(tnxo, mmark);
	nxo_array_el_set(nxo, tnxo, 0);
	nxo_integer_new(tnxo, msweep);
	nxo_array_el_set(nxo, tnxo, 1);
	nxo_array_el_set(stats, nxo, 3);

	/* current. */
	nxo_array_new(nxo, nx, currentlocking, 2);
	nxo_integer_new(tnxo, smark);
	nxo_array_el_set(nxo, tnxo, 0);
	nxo_integer_new(tnxo, ssweep);
	nxo_array_el_set(nxo, tnxo, 1);
	nxo_array_el_set(stats, nxo, 4);

	nxo_stack_npop(tstack, 2);
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