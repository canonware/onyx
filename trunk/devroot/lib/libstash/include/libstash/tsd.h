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

/* Pseudo-opaque type. */
typedef struct cw_tsd_s cw_tsd_t;

struct cw_tsd_s {
	pthread_key_t	key;
};

void	tsd_new(cw_tsd_t *a_tsd, void (*a_func)(void *));
void	tsd_delete(cw_tsd_t *a_tsd);
void	*tsd_get(cw_tsd_t *a_tsd);
void	tsd_set(cw_tsd_t *a_tsd, void *a_val);
