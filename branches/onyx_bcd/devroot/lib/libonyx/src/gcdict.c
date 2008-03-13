/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
******************************************************************************
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
#ifdef CW_THREADS
    ENTRY(period),
#endif
    ENTRY(setactive),
#ifdef CW_THREADS
    ENTRY(setperiod),
#endif
    ENTRY(setthreshold),
    ENTRY(stats),
    ENTRY(threshold)
};

/* This is a global dictionary, but it should never be written to except by the
 * GC thread, so don't bother locking it. */
void
gcdict_l_populate(cw_nxo_t *a_dict, cw_nxa_t *a_nxa)
{
    cw_uint32_t i;
    cw_nxo_t name, value;
    cw_nx_t *nx;

#define	NEXTRA	0
#define NENTRIES (sizeof(gcdict_ops) / sizeof(struct cw_gcdict_entry))

    nx = nxa_nx_get(a_nxa);
    nxo_dict_new(a_dict, nx, FALSE, NENTRIES + NEXTRA);

    for (i = 0; i < NENTRIES; i++)
    {
	nxo_name_new(&name, nx, nxn_str(gcdict_ops[i].nxn),
		     nxn_len(gcdict_ops[i].nxn), TRUE);
	nxo_operator_new(&value, gcdict_ops[i].op_f, gcdict_ops[i].nxn);
	nxo_attr_set(&value, NXOA_EXECUTABLE);

	nxo_dict_def(a_dict, nx, &name, &value);
    }

    cw_assert(nxo_dict_count(a_dict) == NENTRIES + NEXTRA);
#undef NENTRIES
}

void
gcdict_active(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxa_t *nxa;
    cw_nxo_t *active;

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

#ifdef CW_THREADS
void
gcdict_period(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxa_t *nxa;
    cw_nxo_t *period;

    ostack = nxo_thread_ostack_get(a_thread);
    nxa = nx_nxa_get(nxo_thread_nx_get(a_thread));
    period = nxo_stack_push(ostack);
    nxo_dup(period, nxa_period_get(nxa));
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

    nxa_active_set(nx_nxa_get(nxo_thread_nx_get(a_thread)),
		   nxo_boolean_get(active));

    nxo_stack_pop(ostack);
}

#ifdef CW_THREADS
void
gcdict_setperiod(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *period;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(period, ostack, a_thread);
    if (nxo_type_get(period) != NXOT_NUMBER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    /* Make sure period is a non-negative integer. */
    if (nxo_number_negative(period) || nxo_number_integer(period) == FALSE)
    {
	nxo_thread_nerror(a_thread, NXN_limitcheck);
	return;
    }

    nxa_period_set(nx_nxa_get(nxo_thread_nx_get(a_thread)), period);

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
    if (nxo_type_get(threshold) != NXOT_NUMBER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    /* Make sure period is a non-negative integer. */
    if (nxo_number_negative(threshold)
	|| nxo_number_integer(threshold) == FALSE)
    {
	nxo_thread_nerror(a_thread, NXN_limitcheck);
	return;
    }

    nxa_threshold_set(nx_nxa_get(nxo_thread_nx_get(a_thread)), threshold);

    nxo_stack_pop(ostack);
}

/* Create a stats array:
 *
 * stats -->   [
 *               collections
 *               count
 * current -->   [ count mark sweep ]
 * maximum -->   [ count mark sweep ]
 *     sum -->   [ count mark sweep ]
 *             ]
 */
void
gcdict_stats(cw_nxo_t *a_thread)
{
    cw_nx_t *nx;
    cw_nxa_t *nxa;
    cw_bool_t currentlocking;
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *stats, *nxo;
    cw_nxo_t *collections, *count;
    cw_nxo_t *ccount, *cmark, *csweep;
    cw_nxo_t *mcount, *mmark, *msweep;
    cw_nxo_t *scount, *smark, *ssweep;

    nx = nxo_thread_nx_get(a_thread);
    nxa = nx_nxa_get(nx);
    currentlocking = nxo_thread_currentlocking(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    nxo = nxo_stack_push(tstack);

    /* Get stats. */
    nxa_stats_get(nxa, &collections, &count,
		  &ccount, &cmark, &csweep,
		  &mcount, &mmark, &msweep,
		  &scount, &smark, &ssweep);

    /* Create the main array. */
    stats = nxo_stack_push(ostack);
    nxo_array_new(stats, nx, currentlocking, 5);

    /* collections. */
    nxo_array_el_set(stats, collections, 0);

    /* count. */
    nxo_array_el_set(stats, count, 1);

    /* current. */
    nxo_array_new(nxo, nx, currentlocking, 3);
    nxo_array_el_set(nxo, ccount, 0);
    nxo_array_el_set(nxo, cmark, 1);
    nxo_array_el_set(nxo, csweep, 2);
    nxo_array_el_set(stats, nxo, 2);

    /* maximum. */
    nxo_array_new(nxo, nx, currentlocking, 3);
    nxo_array_el_set(nxo, mcount, 0);
    nxo_array_el_set(nxo, mmark, 1);
    nxo_array_el_set(nxo, msweep, 2);
    nxo_array_el_set(stats, nxo, 3);

    /* current. */
    nxo_array_new(nxo, nx, currentlocking, 3);
    nxo_array_el_set(nxo, scount, 0);
    nxo_array_el_set(nxo, smark, 1);
    nxo_array_el_set(nxo, ssweep, 2);
    nxo_array_el_set(stats, nxo, 4);

    nxo_stack_pop(tstack);
}

void
gcdict_threshold(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxa_t *nxa;
    cw_nxo_t *threshold;

    ostack = nxo_thread_ostack_get(a_thread);
    nxa = nx_nxa_get(nxo_thread_nx_get(a_thread));
    threshold = nxo_stack_push(ostack);
    nxo_dup(threshold, nxa_threshold_get(nxa));
}
