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

#ifdef _CW_THREADS
/* Enumeration of message types for the GC thread event loop. */
typedef enum {
	NXAM_NONE,
	NXAM_COLLECT,
	NXAM_RECONFIGURE,
	NXAM_SHUTDOWN
} cw_nxam_t;
#endif

#ifndef _CW_THREADS
#endif

void	nxa_l_shutdown(cw_nxa_t *a_nxa);
void	nxa_l_gc_register(cw_nxa_t *a_nxa, cw_nxoe_t *a_nxoe);
cw_bool_t nxa_l_white_get(cw_nxa_t *a_nxa);
