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

#define	nxo_boolean_new(a_nxo, a_val) do {				\
	nxo_p_new(a_nxo, NXOT_BOOLEAN);					\
	(a_nxo)->o.boolean.val = (a_val);				\
} while (0)

cw_bool_t nxo_boolean_get(cw_nxo_t *a_nxo);
void	nxo_boolean_set(cw_nxo_t *a_nxo, cw_bool_t a_val);
