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

#ifdef _LIBSTIL_DBG
#define	stilo_null_new(a_stilo) do {					\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->magic = _CW_STILO_MAGIC;				\
	(a_stilo)->type = STILOT_NULL;					\
} while (0)
#else
#define	stilo_null_new(a_stilo) do {					\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->type = STILOT_NULL;					\
} while (0)
#endif
