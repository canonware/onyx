/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Master header file for libsock.
 *
 ******************************************************************************/

#ifdef __cplusplus
extern  "C" {
#endif

#ifndef _LIBSOCK_H_
#define _LIBSOCK_H_

#define _CW_LIBSOCK_VERSION "<Version>"

/* Causes verbose I/O-related output. */
/*  #define _CW_LIBSOCK_CONFESS */

/* Project headers to always be included. */

#include "libsock_incs.h"

/* Opaque type. */
typedef struct cw_libsock_s cw_libsock_t;

/* Global variable. */
extern cw_libsock_t	*g_libsock;

void		libsock_init(cw_uint32_t a_max_fds, cw_uint32_t a_bufc_size,
    cw_uint32_t a_max_spare_bufcs);
void		libsock_shutdown(void);
cw_bufc_t	*libsock_spare_bufc_get(void);
void		libsock_in_notify(cw_mq_t *a_mq, cw_sock_t *a_sock, void *a_val);
	
#endif /* _LIBSOCK_H_ */

#ifdef __cplusplus
};

#endif
