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
typedef struct cw_sma_s cw_sma_t;

struct cw_sma_s {
	cw_mtx_t	lock;
	cw_cnd_t	gtzero;
	cw_sint32_t	count;
	cw_uint32_t	waiters;
};

void		sma_new(cw_sma_t *a_sma, cw_sint32_t a_count);
void		sma_delete(cw_sma_t *a_sma);
void		sma_post(cw_sma_t *a_sma);
void		sma_wait(cw_sma_t *a_sma);
cw_bool_t	sma_timedwait(cw_sma_t *a_sma, struct timespec *a_timeout);
cw_bool_t	sma_trywait(cw_sma_t *a_sma);
cw_sint32_t	sma_getvalue(cw_sma_t *a_sma);
void		sma_adjust(cw_sma_t *a_sma, cw_sint32_t a_adjust);
