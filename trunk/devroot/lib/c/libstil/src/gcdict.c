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
	ENTRY(collect),
	ENTRY(dump),
	ENTRY(setactive),
	ENTRY(setperiod),
	ENTRY(setthreshold)
};

void
gcdict_l_populate(cw_stilo_t *a_dict, cw_stila_t *a_stila)
{
	cw_uint32_t	i;
	cw_stilo_t	name, value, el;

#define	NEXTRA	8
#define NENTRIES							\
	(sizeof(gcdict_ops) / sizeof(struct cw_gcdict_entry))

	stilo_dict_new(a_dict, stila_l_stil_get(a_stila), NENTRIES + NEXTRA);

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

	/*
	 * Initialize entries that are not operators.
	 */
	stilo_integer_new(&el, 0);

	/* collections. */
	stilo_name_new(&name, stila_l_stil_get(a_stila),
	    stiln_str(STILN_collections), stiln_len(STILN_collections), TRUE);
	stilo_integer_new(&value, 0);
	stilo_dict_def(a_dict, stila_l_stil_get(a_stila), &name, &value);

	/* new. */
	stilo_name_new(&name, stila_l_stil_get(a_stila), stiln_str(STILN_new),
	    stiln_len(STILN_new), TRUE);
	stilo_integer_new(&value, 0);
	stilo_dict_def(a_dict, stila_l_stil_get(a_stila), &name, &value);

	/* current. */
	stilo_name_new(&name, stila_l_stil_get(a_stila),
	    stiln_str(STILN_current), stiln_len(STILN_current), TRUE);
	stilo_array_new(&value, stila_l_stil_get(a_stila), 3);
	for (i = 0; i < 3; i++)
		stilo_array_el_set(&value, &el, i);
	stilo_dict_def(a_dict, stila_l_stil_get(a_stila), &name, &value);

	/* maximum. */
	stilo_name_new(&name, stila_l_stil_get(a_stila),
	    stiln_str(STILN_maximum), stiln_len(STILN_maximum), TRUE);
	stilo_array_new(&value, stila_l_stil_get(a_stila), 3);
	for (i = 0; i < 3; i++)
		stilo_array_el_set(&value, &el, i);
	stilo_dict_def(a_dict, stila_l_stil_get(a_stila), &name, &value);

	/* sum. */
	stilo_name_new(&name, stila_l_stil_get(a_stila), stiln_str(STILN_sum),
	    stiln_len(STILN_sum), TRUE);
	stilo_array_new(&value, stila_l_stil_get(a_stila), 3);
	for (i = 0; i < 3; i++)
		stilo_array_el_set(&value, &el, i);
	stilo_dict_def(a_dict, stila_l_stil_get(a_stila), &name, &value);

	/* active. */
	stilo_name_new(&name, stila_l_stil_get(a_stila),
	    stiln_str(STILN_active), stiln_len(STILN_active), TRUE);
	stilo_boolean_new(&value, FALSE);
	stilo_dict_def(a_dict, stila_l_stil_get(a_stila), &name, &value);

	/* threshold. */
	stilo_name_new(&name, stila_l_stil_get(a_stila),
	    stiln_str(STILN_threshold), stiln_len(STILN_threshold), TRUE);
	stilo_integer_new(&value, _LIBSTIL_GCDICT_THRESHOLD);
	stilo_dict_def(a_dict, stila_l_stil_get(a_stila), &name, &value);

	/* period. */
	stilo_name_new(&name, stila_l_stil_get(a_stila),
	    stiln_str(STILN_period), stiln_len(STILN_period), TRUE);
	stilo_integer_new(&value, _LIBSTIL_GCDICT_PERIOD);
	stilo_dict_def(a_dict, stila_l_stil_get(a_stila), &name, &value);

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
gcdict_collect(cw_stilt_t *a_stilt)
{
	stila_collect(stil_stila_get(stilt_stil_get(a_stilt)));
}

void
gcdict_dump(cw_stilt_t *a_stilt)
{
	stila_dump(stil_stila_get(stilt_stil_get(a_stilt)), a_stilt);
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
