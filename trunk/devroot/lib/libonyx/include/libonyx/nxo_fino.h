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
#define	nxo_fino_new(a_nxo) do {					\
	memset((a_nxo), 0, sizeof(cw_nxo_t));				\
	(a_nxo)->magic = _CW_NXO_MAGIC;					\
	(a_nxo)->type = NXOT_FINO;					\
} while (0)
#else
#define	nxo_fino_new(a_nxo) do {					\
	memset((a_nxo), 0, sizeof(cw_nxo_t));				\
	(a_nxo)->type = NXOT_FINO;					\
} while (0)
#endif
