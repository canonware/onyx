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

#define	nxo_integer_new(a_nxo, a_val) do {				\
	nxo_p_new(a_nxo, NXOT_INTEGER);					\
	(a_nxo)->o.integer.i = (a_val);					\
} while (0)

#define	nxo_integer_get(a_nxo) (a_nxo)->o.integer.i
#define	nxo_integer_set(a_nxo, a_val) do {				\
	(a_nxo)->o.integer.i = (a_val);					\
} while (0)
