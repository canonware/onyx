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

/* Opaque type. */
typedef struct cw_socks_s cw_socks_t;

cw_socks_t	*socks_new(void);
void		socks_delete(cw_socks_t *a_socks);
cw_bool_t	socks_listen(cw_socks_t *a_socks, cw_uint32_t a_mask, int
    *r_port);
cw_sock_t	*socks_accept(cw_socks_t *a_socks, struct timespec *a_timeout,
    cw_sock_t *r_sock);
