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

#ifdef _LIBONYX_DBG
#define	nxo_integer_new(a_nxo, a_val) do {				\
	memset((a_nxo), 0, sizeof(cw_nxo_t));				\
	(a_nxo)->magic = _CW_NXO_MAGIC;					\
	(a_nxo)->o.integer.i = (a_val);					\
	(a_nxo)->type = NXOT_INTEGER;					\
} while (0)
#else
#define	nxo_integer_new(a_nxo, a_val) do {				\
	memset((a_nxo), 0, sizeof(cw_nxo_t));				\
	(a_nxo)->o.integer.i = (a_val);					\
	(a_nxo)->type = NXOT_INTEGER;					\
} while (0)
#endif

#define	nxo_integer_get(a_nxo) (a_nxo)->o.integer.i
#define	nxo_integer_set(a_nxo, a_val) do {				\
	(a_nxo)->o.integer.i = (a_val);					\
} while (0)
