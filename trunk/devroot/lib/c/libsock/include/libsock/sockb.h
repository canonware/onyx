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

/* Opaque type. */
typedef struct cw_sockb_s cw_sockb_t;

/* Global variable. */
extern cw_sockb_t	*g_sockb;

cw_bool_t	sockb_init(cw_uint32_t a_max_fds, cw_uint32_t a_bufc_size,
    cw_uint32_t a_max_spare_bufcs);

void		sockb_shutdown(void);

cw_bufc_t	*sockb_get_spare_bufc(void);

cw_bool_t	sockb_in_notify(cw_mq_t *a_mq, int a_sockfd);
