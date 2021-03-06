/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxa_l.h"

struct cw_gcdict_entry
{
    cw_nxn_t nxn;
    cw_op_t *op_f;
};

#define ENTRY(name) {NXN_##name, gcdict_##name}

/* Array of operators in gcdict. */
static const struct cw_gcdict_entry gcdict_ops[] = {
    ENTRY(active),
    ENTRY(collect),
#ifdef CW_PTHREADS
    ENTRY(period),
#endif
    ENTRY(setactive),
#ifdef CW_PTHREADS
    ENTRY(setperiod),
#endif
    ENTRY(setthreshold),
    ENTRY(stats),
    ENTRY(threshold)
};

void
gcdict_l_populate(cw_nxo_t *a_dict, cw_nxo_t *a_tname, cw_nxo_t *a_tvalue)
{
    uint32_t i;

#define	NEXTRA	0
#define NENTRIES (sizeof(gcdict_ops) / sizeof(struct cw_gcdict_entry))

    nxo_dict_new(a_dict, true, NENTRIES + NEXTRA);

    for (i = 0; i < NENTRIES; i++)
    {
	nxo_name_new(a_tname, nxn_str(gcdict_ops[i].nxn),
		     nxn_len(gcdict_ops[i].nxn), true);
	nxo_operator_new(a_tvalue, gcdict_ops[i].op_f, gcdict_ops[i].nxn);
	nxo_attr_set(a_tvalue, NXOA_EXECUTABLE);

	nxo_dict_def(a_dict, a_tname, a_tvalue);
    }

    cw_assert(nxo_dict_count(a_dict) == NENTRIES + NEXTRA);
#undef NENTRIES
}

void
gcdict_active(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *active;

    ostack = nxo_thread_ostack_get(a_thread);
    active = nxo_stack_push(ostack);
    nxo_boolean_new(active, nxa_active_get());
}

void
gcdict_collect(cw_nxo_t *a_thread)
{
    nxa_collect();
}

#ifdef CW_PTHREADS
void
gcdict_period(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *period;

    ostack = nxo_thread_ostack_get(a_thread);
    period = nxo_stack_push(ostack);
    nxo_integer_new(period, nxa_period_get());
}
#endif

void
gcdict_setactive(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *active;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(active, ostack, a_thread);
    if (nxo_type_get(active) != NXOT_BOOLEAN)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxa_active_set(nxo_boolean_get(active));

    nxo_stack_pop(ostack);
}

#ifdef CW_PTHREADS
void
gcdict_setperiod(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *period;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(period, ostack, a_thread);
    if (nxo_type_get(period) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    if (nxo_integer_get(period) < 0 || nxo_integer_get(period) > UINT_MAX)
    {
	nxo_thread_nerror(a_thread, NXN_limitcheck);
	return;
    }

    nxa_period_set(nxo_integer_get(period));

    nxo_stack_pop(ostack);
}
#endif

void
gcdict_setthreshold(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *threshold;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(threshold, ostack, a_thread);
    if (nxo_type_get(threshold) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    if (nxo_integer_get(threshold) < 0 || nxo_integer_get(threshold) > UINT_MAX)
    {
	nxo_thread_nerror(a_thread, NXN_limitcheck);
	return;
    }

    nxa_threshold_set(nxo_integer_get(threshold));

    nxo_stack_pop(ostack);
}

/* Create a stats array:
 *
 * stats -->   [
 *               collections
 *               count
 * current -->   [ count mark ]
 * maximum -->   [ count mark ]
 *     sum -->   [ count mark ]
 *             ]
 */
void
gcdict_stats(cw_nxo_t *a_thread)
{
    bool currentlocking;
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *stats, *nxo, *tnxo;
    cw_nxoi_t collections, count;
    cw_nxoi_t ccount, cmark;
    cw_nxoi_t mcount, mmark;
    cw_nxoi_t scount, smark;

    currentlocking = nxo_thread_currentlocking(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    nxo = nxo_stack_push(tstack);
    tnxo = nxo_stack_push(tstack);

    /* Get stats. */
    nxa_stats_get(&collections, &count,
		  &ccount, &cmark,
		  &mcount, &mmark,
		  &scount, &smark);

    /* Create the main array. */
    stats = nxo_stack_push(ostack);
    nxo_array_new(stats, currentlocking, 5);

    /* collections. */
    nxo_integer_new(nxo, collections);
    nxo_array_el_set(stats, nxo, 0);

    /* count. */
    nxo_integer_new(nxo, count);
    nxo_array_el_set(stats, nxo, 1);

    /* current. */
    nxo_array_new(nxo, currentlocking, 2);
    nxo_integer_new(tnxo, ccount);
    nxo_array_el_set(nxo, tnxo, 0);
    nxo_integer_new(tnxo, cmark);
    nxo_array_el_set(nxo, tnxo, 1);
    nxo_array_el_set(stats, nxo, 2);

    /* maximum. */
    nxo_array_new(nxo, currentlocking, 2);
    nxo_integer_new(tnxo, mcount);
    nxo_array_el_set(nxo, tnxo, 0);
    nxo_integer_new(tnxo, mmark);
    nxo_array_el_set(nxo, tnxo, 1);
    nxo_array_el_set(stats, nxo, 3);

    /* current. */
    nxo_array_new(nxo, currentlocking, 2);
    nxo_integer_new(tnxo, scount);
    nxo_array_el_set(nxo, tnxo, 0);
    nxo_integer_new(tnxo, smark);
    nxo_array_el_set(nxo, tnxo, 1);
    nxo_array_el_set(stats, nxo, 4);

    nxo_stack_npop(tstack, 2);
}

void
gcdict_threshold(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *threshold;

    ostack = nxo_thread_ostack_get(a_thread);
    threshold = nxo_stack_push(ostack);
    nxo_integer_new(threshold, nxa_threshold_get());
}
