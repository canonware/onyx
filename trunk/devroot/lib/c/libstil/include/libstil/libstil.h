/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Master header file for libstil.
 *
 ******************************************************************************/

#ifdef __cplusplus
extern  "C" {
#endif

#ifndef _LIBSTIL_H_
#define _LIBSTIL_H_

#define _LIBSTIL_VERSION "<Version>"

/* Exception numbers.  libstil reserves -128 to -255. */
#define	_CW_STILX_MAX			-128
#define	_CW_STILX_MIN			-255

/* errordict not found, or handler not found in errordict. */
#define	_CW_STILX_ERRORDICT		-128
/* Error accessing $error. */
#define	_CW_STILX_DERROR		-129

/* Internal use, for the exit operator. */
#define	_CW_STILX_EXIT			-129
/* Internal use, for the stop operator. */
#define	_CW_STILX_STOP			-130
/* Internal use, for the quit operator, caught by the start operator. */
#define	_CW_STILX_QUIT			-131

#include "libstil_defs.h"
#include "libstil_incs.h"

/* Convenience macro for static embedded stil code. */
#define	_cw_stil_code(a_stilt, a_code) do {				\
	cw_stilts_t		stilts;					\
	static const cw_uint8_t	code[] = (a_code);			\
									\
	stilts_new(&stilts);						\
	stilt_interpret((a_stilt), &stilts, code, sizeof(code) - 1);	\
	stilt_flush((a_stilt), &stilts);				\
	stilts_delete(&stilts, (a_stilt));				\
} while (0)

#endif	/* _LIBSTIL_H_ */

#ifdef __cplusplus
};
#endif
