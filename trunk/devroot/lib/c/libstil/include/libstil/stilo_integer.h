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
#define	stilo_integer_new(a_stilo, a_val) do {				\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->magic = _CW_STILO_MAGIC;				\
	(a_stilo)->o.integer.i = (a_val);				\
	(a_stilo)->type = STILOT_INTEGER;				\
} while (0)
#else
#define	stilo_integer_new(a_stilo, a_val) do {				\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->o.integer.i = (a_val);				\
	(a_stilo)->type = STILOT_INTEGER;				\
} while (0)
#endif

#define	stilo_integer_get(a_stilo) (a_stilo)->o.integer.i
#define	stilo_integer_set(a_stilo, a_val) do {				\
	(a_stilo)->o.integer.i = (a_val);				\
} while (0)
