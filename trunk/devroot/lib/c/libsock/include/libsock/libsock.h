/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Master header file for libsock.
 *
 ****************************************************************************/

#ifdef __cplusplus
extern  "C" {
#endif

#ifndef _LIBSOCK_H_
#define _LIBSOCK_H_

#define _LIBSOCK_VERSION "<Version>"

/* Causes verbose I/O-related output. */
/* #define _LIBSOCK_CONFESS */

/* Project headers to always be included. */

#include "libsock_incs.h"

/* Opaque type. */
typedef struct cw_libsock_s cw_libsock_t;

/* Global variable. */
extern cw_libsock_t	*g_libsock;

cw_bool_t	libsock_init(cw_uint32_t a_max_fds, cw_uint32_t a_bufc_size,
    cw_uint32_t a_max_spare_bufcs);
void		libsock_shutdown(void);
cw_bufc_t	*libsock_get_spare_bufc(void);
cw_bool_t	libsock_in_notify(cw_mq_t *a_mq, int a_sockfd);
	
#endif /* _LIBSOCK_H_ */

#ifdef __cplusplus
};

#endif
