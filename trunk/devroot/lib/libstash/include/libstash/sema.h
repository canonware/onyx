/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_sema_s cw_sema_t;

struct cw_sema_s {
	cw_mtx_t	lock;
	cw_cnd_t	gtzero;
	cw_sint32_t	count;
	cw_uint32_t	waiters;
};

void		sema_new(cw_sema_t *a_sema, cw_sint32_t a_count);
void		sema_delete(cw_sema_t *a_sema);
void		sema_post(cw_sema_t *a_sema);
void		sema_wait(cw_sema_t *a_sema);
cw_bool_t	sema_timedwait(cw_sema_t *a_sema, struct timespec *a_timeout);
cw_bool_t	sema_trywait(cw_sema_t *a_sema);
cw_sint32_t	sema_getvalue(cw_sema_t *a_sema);
void		sema_adjust(cw_sema_t *a_sema, cw_sint32_t a_adjust);
